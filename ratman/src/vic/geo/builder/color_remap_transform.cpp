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
#include <vic/geo/builder/color_remap_transform.hpp>
#include <sl/utility.hpp>

// GDAL include
#include <gdal_priv.h>
#include <cpl_string.h>
#include <gdalwarper.h>

namespace vic {
  namespace geo {

    color_remap_transform::color_remap_transform(int black_out, int black_in, 
						 int white_out, int white_in, 
						 int below_to_black, 
						 int above_to_black) :
      black_out_(black_out),
      black_in_(black_in),
      white_out_(white_out),
      white_in_(white_in),
      below_to_black_(below_to_black),
      above_to_black_(above_to_black) {

      lut_rebuild();
    }

    color_remap_transform::~color_remap_transform() {
    }

    void color_remap_transform::lut_rebuild() {
      for (int i = 0; i < black_in_; ++i) {
	lut_[i] = black_out_;
      }
      for (int i = black_in_; i < white_in_; ++i) {
	float lf_i = float(black_out_) + (float(white_out_ - black_out_) * float(i - black_in_)) / float(white_in_ - black_in_);
	int li_i = sl::median(int(lf_i+0.5f), black_out_, white_out_-1);
	lut_[i] = li_i;
      }

      for (int i = white_in_; i < 256; ++i) {
	lut_[i] = white_out_;
      }
      for (int i = 0; i < below_to_black_; ++i) {
	lut_[i] = 0; 
      }
    }


    void color_remap_transform::store_to(sl::output_serializer& s) const {
      s.write_simple(black_out_);
      s.write_simple(black_in_);
      s.write_simple(white_out_);
      s.write_simple(white_in_);
      s.write_simple(below_to_black_);
      s.write_simple(above_to_black_);
    }

    void color_remap_transform::retrieve_from(sl::input_serializer& s) {
      s.read_simple(black_out_);
      s.read_simple(black_in_);
      s.read_simple(white_out_);
      s.read_simple(white_in_);
      s.read_simple(below_to_black_);
      s.read_simple(above_to_black_);

      lut_rebuild();
    }
    
    CPLErr color_remap_transform::gdal_color_transform_callback(void *p_kern, void *p_arg) {
      this_t* self = static_cast<this_t*>(p_arg);
      return self->gdal_color_transform(p_kern);
    }

    CPLErr color_remap_transform::gdal_color_transform(void *p_kern) const {
      GDALWarpKernel *kernel = static_cast<GDALWarpKernel *>(p_kern);
      GByte **img = kernel->papabySrcImage;
      GDALDataType data_type = kernel->eWorkingDataType;
      if(data_type != GDT_Byte) {
	SL_TRACE_OUT(-1) << "Cannot handle GDAL data_type" << std::endl;
	return CE_None; // FIXME
      }

      int sx = kernel->nSrcXSize;
      int sy = kernel->nSrcYSize;
      int bands = kernel->nBands;
  
      if (above_to_black_ > 0 && above_to_black_ <= 255) {
	bool *is_white = new bool[sx+2];
	for (int y = 0; y < sy; ++y) {
	  is_white[0] = false;
	  is_white[sx+1] = false;
	  for (int x = 0; x < sx; ++x) {
	    int value=0;
	    for(int b=0; b<bands; ++b) value += static_cast<int>(img[b][x+y*sx]);
	    is_white[x+1] = (value >= above_to_black_*bands);
	  }
	  for (int x = 0; x < sx; ++x) {
	    if (is_white[x+1] && (is_white[x+1-1] || is_white[x+1+1])) {
	      // White to black
	      for (int b=0; b<bands; ++b) img[b][x+y*sx] = 0;
	    } else {
	      // Normal 
	      for (int b=0; b<bands; ++b) img[b][x+y*sx] = lut_[img[b][x+y*sx]];
	    }
	  }
	}
	delete [] is_white;
      } else {
	for (int y = 0; y < sy; ++y) {
	  for (int x = 0; x < sx; ++x) {
	    for(int b=0; b<bands; ++b) {
	      img[b][x+y*sx] = lut_[img[b][x+y*sx]];
	    }
	  }
	}
      }

      return CE_None;
    }

  } // namespace geo
} // namespace vic
