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
#ifndef QUAD_WARPER_HPP
#define QUAD_WARPER_HPP

#include <vic/geo/builder/geo_transform.hpp>
#include <vic/geo/builder/color_remap_transform.hpp>

#include <vector>
#include <string>
#include <gdalwarper.h>


class GDALDataset;

namespace vic {
  namespace geo {

    class quad_warper {
    public:

    protected:
      mutable bool last_operation_success_;
      mutable std::string last_error_message_;
      bool need_destroy_;
      GDALWarpOptions *opts_;
      GDALWarpOperation *oper_;      
      color_remap_transform color_remap_transform_;
      geo_transform geo_transform_;

    public:
      quad_warper();
      virtual ~quad_warper();
      
      inline bool last_operation_success() const {
	return last_operation_success_;
      }
      
      inline const std::string& last_error_message() const {
	return last_error_message_;
      }

      inline void reset_error() {
	last_operation_success_=true;
	last_error_message_=std::string("");
      }

      void create(GDALDataset *src, GDALDataset *dst, const geo_transform& geo_xform, const color_remap_transform& color_xform);
      void set_src_geo_matrix(const geo_matrix& mat);
      void set_dst_geo_matrix(const geo_matrix& mat);      
      void destroy();
      void chunk_and_warp_image(int x, int y, int sx, int sy);
      
    };

  } // namespace geo
} // namespace vic

#endif
