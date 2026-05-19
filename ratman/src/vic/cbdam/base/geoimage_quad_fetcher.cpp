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
#include <vic/cbdam/base/geoimage_quad_fetcher.hpp>

namespace cbdam {

  geoimage_quad_fetcher::geoimage_quad_fetcher(const std::string& url,
					       const std::string& desired_srs,
					       const aabox2d_t&   desired_uv_box,
					       const std::size_t& desired_quad_width,
					       const std::string& default_about) :
    super_t(url, desired_srs, desired_uv_box, default_about), quad_width_(desired_quad_width) {

    genalpha_behavior_ = compressed_image_t::ALPHA_FROM_SRC;
    genalpha_constant_ = 10;
    genalpha_alphamin_ = 0;
    genalpha_alphamax_ = 255;
  }


  geoimage_quad_fetcher::~geoimage_quad_fetcher() {
  }

  
  geoimage_quad_fetcher::value_t* geoimage_quad_fetcher::decoded(const uint8_t* buf,
								std::size_t buf_size) const {
    return new compressed_image_t(buf, buf_size, quad_width_, genalpha_behavior_, genalpha_constant_, genalpha_alphamin_, genalpha_alphamax_);
  }
    
} // namespace cbdam
