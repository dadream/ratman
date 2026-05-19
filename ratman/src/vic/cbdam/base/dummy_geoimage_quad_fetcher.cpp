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
#include <vic/cbdam/base/dummy_geoimage_quad_fetcher.hpp>
#include <vic/geo/base/victms_conventions.hpp>
#include <QGLWidget>
namespace cbdam {
  
  dummy_geoimage_quad_fetcher::dummy_geoimage_quad_fetcher(const std::string& desired_srs,
							   const aabox2d_t&   desired_uv_box,
							   const std::size_t& desired_quad_width) :
    super_t("dummy://", desired_srs, desired_uv_box, desired_quad_width, "diamond outline dummy layer") {
  }

  dummy_geoimage_quad_fetcher::~dummy_geoimage_quad_fetcher() {
    clear();
  }
        

  dummy_geoimage_quad_fetcher::value_t* dummy_geoimage_quad_fetcher::decoded(const uint8_t* /*buf*/,
									     std::size_t /*buf_size*/) const {
    
    vic::img::gl_image<> img(4, quad_width_, quad_width_);
    img.fill(0,0,0,0);
    for(std::size_t i = 0; i < quad_width_; ++i) {
      for (int c = 0; c < 4; ++c) {
	img(c, 0, i) = 255;
	img(c, quad_width_-1, i) = 255;
	img(c, i, 0) = 255;
	img(c, i, quad_width_-1) = 255;
      }
    }

    return new compressed_image_t(img);
  }

  void dummy_geoimage_quad_fetcher::direct_connect() {
    is_connected_ = true;
  }
 
 void dummy_geoimage_quad_fetcher::direct_disconnect() {
    is_connected_ = false;
  }

  void dummy_geoimage_quad_fetcher::direct_send_requests() {
  }

  void dummy_geoimage_quad_fetcher::direct_receive() {
    for(std::map<key_t, aabox2d_t>::iterator it = pending_requests_.begin();
        it != pending_requests_.end();
        ++it) {
      handle_data_response(it->first, 0, 0);
    }
    pending_requests_.clear();
  }

} // namespace cbdam
