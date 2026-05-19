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
#include <vic/cbdam/base/loaded_geoimage_quad_fetcher.hpp>
#include <vic/cbdam/base/imgfilter_bell.hpp>
#include <vic/cbdam/base/img_operations.hpp>
#include <vic/geo/base/victms_conventions.hpp>
#include <vic/curlstream/curlstream.hpp>
#include <QByteArray>
#include <QGLWidget>

namespace cbdam {
  
  loaded_geoimage_quad_fetcher::loaded_geoimage_quad_fetcher(const std::string& url,
							     const std::string& desired_srs,
							     const aabox2d_t&   desired_uv_box,
							     const std::size_t& desired_quad_width,
							     const std::string& default_about) :
    super_t(url, desired_srs, desired_uv_box, desired_quad_width, default_about) {
    img_=0;
  }

  loaded_geoimage_quad_fetcher::~loaded_geoimage_quad_fetcher() {
    clear();
    if (img_) delete img_;
    img_=0;
  }

  void loaded_geoimage_quad_fetcher::http_send_requests() {
    if (!img_) {
      SL_TRACE_OUT(-1) << "Loading image from " << base_url() << std::endl;
      img_ = new QImage();

      vic::icurlstream* in_file = new vic::icurlstream();
      in_file->open(base_url().c_str());
      if (!in_file) {
	std::cerr << "ERROR unable to connect to " << base_url() << std::endl;
	// img_ will remain empty
      } else {
	// Copy to memory
	QByteArray bArray;
	while ( in_file->good() ) {
	  char c; in_file->get(c);
	  if (in_file->good()) {
	    bArray.append(c);
	  }
	}
	// Close in_file
	in_file->close();
	delete in_file;
	in_file=0;

	// Decode image format
	if (!img_->loadFromData(bArray)) {
	  std::cerr << "ERROR decoding image got from " << base_url() << std::endl;
	  *img_ = QImage(); // mark error
	} else {
	  // Convert to opengl format and generate alpha channel
	  bool is_grayscale = img_->isGrayscale();
	  bool has_alpha = img_->hasAlphaChannel();
	  *img_ = img_->convertToFormat(QImage::Format_ARGB32);
	  QImage qi_gl = QGLWidget::convertToGLFormat(*img_);
	  vic::img::gl_image<> vic_img;

	  vic_img.assign(qi_gl.bits(), 4, qi_gl.width(), qi_gl.height());
	  cbdam::compressed_rgba32_image::generate_alpha(vic_img,
							 is_grayscale,
							 has_alpha,
							 genalpha_behavior_,
							 genalpha_constant_);
	  
	  // Copy back to img
	  for (int y=0; y<img_->height(); ++y) {
	    for (int x=0; x<img_->width(); ++x) {
	      img_->setPixel(x, y, QColor(vic_img.at(0,x,y),
					  vic_img.at(1,x,y),
					  vic_img.at(2,x,y),
					  vic_img.at(3,x,y)).rgba());
					 
	    }
	  }
	}
      }
    }   

    http_receive();
  }

  void loaded_geoimage_quad_fetcher::http_receive() {
    for (key_box_map_t::iterator it = pending_requests_.begin();
	 it != pending_requests_.end();
	 ++it) {
      const key_t& k = it->first;
      const aabox2d_t& quad_box = it->second;
      if ((!img_) || (img_->isNull())) {
	// No image, cancel all requests
	handle_nodata_response(k);
      } else {

	double fx0_crop=(quad_box[0][0]-uv_box()[0][0])/(uv_box().half_side_lengths()[0]*2.0);
	double fy0_crop=(quad_box[0][1]-uv_box()[0][1])/(uv_box().half_side_lengths()[1]*2.0);
	double fx1_crop=(quad_box[1][0]-uv_box()[0][0])/(uv_box().half_side_lengths()[0]*2.0);
	double fy1_crop=(quad_box[1][1]-uv_box()[0][1])/(uv_box().half_side_lengths()[1]*2.0);

	int x0_crop=int(fx0_crop*(img_->width()-1));
	int y0_crop=int(fy0_crop*(img_->height()-1));
	int x1_crop=int(fx1_crop*(img_->width()-1));
	int y1_crop=int(fy1_crop*(img_->height()-1));

	if (x0_crop >=img_->width()  || x1_crop < 0 || 
	    y0_crop >=img_->height() || y1_crop < 0 ) {
	  handle_nodata_response(k);
	} else {

	  static const int npx=40;
	  double xscale = double(x1_crop-x0_crop)/quad_width_;
	  double yscale = double(y1_crop-y0_crop)/quad_width_;
	  int deltax = int(npx*xscale);
	  int deltay = int(npx*yscale);
	  int x0_crop_e=x0_crop-deltax;
	  int x1_crop_e=x1_crop+deltax;
	  int y0_crop_e=y0_crop-deltay;
	  int y1_crop_e=y1_crop+deltay;

	  QImage cropped_img = img_->copy(x0_crop_e,y0_crop_e,x1_crop_e-x0_crop_e,y1_crop_e-y0_crop_e);

#if 1	  
	  imgfilter_bell filter;
	  QImage result = img_operations::zoom(cropped_img, quad_width_+2*npx, quad_width_+2*npx, &filter).mirrored();

#else	  
	  QImage result = cropped_img.scaled(quad_width_+2*npx, quad_width_+2*npx,
					     Qt::IgnoreAspectRatio,
					     Qt::SmoothTransformation).mirrored();
#endif
	  result = result.copy(npx,npx,quad_width_,quad_width_);
	  handle_data_response(k, reinterpret_cast<const uint8_t*>(&result), 0);
	}
      }
    }
    pending_requests_.clear();
  }

  loaded_geoimage_quad_fetcher::value_t* loaded_geoimage_quad_fetcher::decoded(const uint8_t* buf,
									       std::size_t /*buf_size*/) const {
    const QImage *img = reinterpret_cast<const QImage*>(buf);
    QImage qi_gl = QGLWidget::convertToGLFormat(*img);
    vic::img::gl_image<> vic_img;
    vic_img.assign(qi_gl.bits(), 4, qi_gl.width(), qi_gl.height());
    return new value_t(vic_img);
  }
  


} // namespace cbdam
