//+++HDR+++
//======================================================================
//   This file is part of the RATMAN software framework.
//   Copyright (C) 2009 by CRS4, Pula, Italy.
//
//   For more information, visit the CRS4 Visual Computing Group
//   web pages at http://www.crs4.it/vic/
//
//   This file may be used under the terms of the GNU General Public
//   License as published by the Free Software Foundation and appearing
//   in the file LICENSE included in the packaging of this file.
//
//   CRS4 reserves all rights not expressly granted herein.
//  
//   This file is provided AS IS with NO WARRANTY OF ANY KIND, 
//   INCLUDING THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS 
//   FOR A PARTICULAR PURPOSE.
//
//======================================================================
//---HDR---//
#include <vic/cbdam/base/compressed_rgba32_image.hpp>
#include <sl/utility.hpp>
#include <QGLWidget>
#include <QImage>

namespace cbdam {

  compressed_rgba32_image::compressed_rgba32_image(const uint8_t* buf,
						   std::size_t buf_size,
						   std::size_t quad_width,
						   genalpha_behavior_t genalpha_behavior, 
						   uint8_t genalpha_constant, 
						   uint8_t genalpha_alphamin,
						   uint8_t genalpha_alphamax) :
    quad_width_(quad_width), genalpha_behavior_(genalpha_behavior), genalpha_constant_(genalpha_constant),
    genalpha_alphamin_(genalpha_alphamin), genalpha_alphamax_(genalpha_alphamax) {
    assert(buf);
    assert(buf_size > 0);

    data_.resize(buf_size);
    memcpy(&(data_[0]), buf, buf_size);

    is_uncompressed_ = false;
  }

  compressed_rgba32_image::compressed_rgba32_image(const uncompressed_image_t& img) {
    assert(img.extent(0) == 4);
    assert(img.extent(1) > 0);
    assert(img.extent(1) == img.extent(2));
    
    quad_width_ = img.extent(1);
    data_.resize(img.size());
    memcpy(&(data_[0]), img.to_pointer(), img.size());

    is_uncompressed_ = true;
    // next parameters will not be used
    genalpha_behavior_ =  ALPHA_FROM_SRC;
    genalpha_constant_ = 10;
  }
    
  void compressed_rgba32_image::generate_alpha(uncompressed_image_t& img,
					       bool img_is_grayscale,
					       bool img_has_alpha,
					       genalpha_behavior_t genalpha_behavior,
					       uint8_t genalpha_constant,
					       uint8_t genalpha_alphamin,
					       uint8_t genalpha_alphamax) {
    if (genalpha_behavior == ALPHA_FROM_CONSTANT) {
      // Force fixed opacity
      img.fill_channel(3, genalpha_constant);
    } else if ((genalpha_behavior == ALPHA_FROM_SRC) &&
	       (!img_has_alpha)) {
      // Color images get alpha from black, others from
      // intensity
      if (img_is_grayscale) {
	img.set_alpha_from_value();
      } else {
	img.set_alpha_from_black(genalpha_constant);

	// Increase color range, recovering bits used for alpha coding
	uint8_t black_in = uint8_t(sl::median(1.5f*genalpha_constant, 0.0f, 254.0f));
	if (black_in>0) {
	  color_remap(img, 
		      0, black_in,
		      255, 255);
	}
      }	  
    } else if ((genalpha_behavior == ALPHA_FROM_BLACK_FORCED) ||
	       ((!img_has_alpha) && genalpha_behavior == ALPHA_FROM_BLACK_IF_ABSENT)) {
      // Force alpha from black 
      img.set_alpha_from_black(genalpha_constant);
      
      // Increase color range, recovering bits used for alpha coding
      uint8_t black_in = uint8_t(sl::median(1.5f*genalpha_constant, 0.0f, 254.0f));
      if (black_in>0) {
	color_remap(img, 
		    0, black_in,
		    255, 255);
      }
    } else if ((genalpha_behavior == ALPHA_FROM_WHITE_FORCED) ||
	       ((!img_has_alpha) && genalpha_behavior == ALPHA_FROM_WHITE_IF_ABSENT)) {
      // Force alpha from white 
      img.set_alpha_from_white(genalpha_constant);
    } else if ((genalpha_behavior == ALPHA_FROM_VALUE_FORCED) ||
	       ((!img_has_alpha) && genalpha_behavior == ALPHA_FROM_VALUE_IF_ABSENT)) {
      // Force alpha from value
      img.set_alpha_from_value();
    } else {
      // Keep source's alpha
    }

    // Alpha rescale
    if (genalpha_alphamin != 0 || genalpha_alphamax != 255) {
      img.channel_rescale(3,genalpha_alphamin,genalpha_alphamax);
    }
  }
  

  void compressed_rgba32_image::uncompress_to(uncompressed_image_t& img) const {
    if (is_uncompressed_) {
      // directly copy decompressed data buf into img
      img.assign(&(data_[0]), 4, quad_width_, quad_width_);
    } else {
      // decompress and properly assign alpha channel
      QImage qi;
      if (qi.loadFromData(&(data_[0]), data_.size()) && (qi.width()>0) && (qi.height()>0)) {
	img.reshape(4, quad_width_, quad_width_);
	// Rescale image if of wrong size 
	if ((qi.width() != int(quad_width_)) ||
	    (qi.height() != int(quad_width_))) {
	  qi = qi.scaled(quad_width_, quad_width_,
			 Qt::IgnoreAspectRatio,
			 Qt::SmoothTransformation);
	}

	// Convert to opengl
	QImage qi_argb32 = qi.convertToFormat(QImage::Format_ARGB32);
	QImage qi_gl = QGLWidget::convertToGLFormat(qi_argb32);
	
	// Create result and assign alpha channel
	img.assign(qi_gl.bits(), 4, qi_gl.width(), qi_gl.height());

	generate_alpha(img,
		       qi.isGrayscale(),
		       qi.hasAlphaChannel(),
		       genalpha_behavior_,
		       genalpha_constant_,
		       genalpha_alphamin_,
		       genalpha_alphamax_);
      } else {
	// Mark image as invalid with red fill
	img.reshape(4, quad_width_, quad_width_);
	img.fill(255, 0, 0, 255);
      }
    }
  }

  void compressed_rgba32_image::color_remap(uncompressed_image_t& img,
					    uint8_t black_out, uint8_t black_in, 
					    uint8_t white_out, uint8_t white_in) {
    assert(img.channels() == 4);
    // --- Compute lookup table
    uint8_t lut[256];
    for (int i = 0; i < black_in; ++i) {
      lut[i] = black_out;
    }
    const float scale = float(white_out - black_out)/float(white_in - black_in);
    for (int i = black_in; i < white_in; ++i) {
      float lf_i = float(black_out) + scale * float(i - black_in);
      lut[i] = uint8_t(sl::median(int(lf_i+0.5f), int(black_out), int(white_out)));
    }
    for (int i = white_in; i < 256; ++i) {
      lut[i] = white_out;
    }

    // --- Remap color channels
    const::size_t nc=img.channels();

    uint8_t *ptr, *ptr_end = img.end();
    for (ptr=img.begin(); ptr<ptr_end; ptr+=nc) {
      ptr[0] = lut[ptr[0]];
      ptr[1] = lut[ptr[1]];
      ptr[2] = lut[ptr[2]];
    }
  }


} // namespace cbdam
