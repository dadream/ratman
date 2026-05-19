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
#ifndef GEOIMAGE_QUAD_FETCHER_HPP
#define GEOIMAGE_QUAD_FETCHER_HPP

#include <vic/cbdam/base/geodata_fetcher.hpp>
#include <vic/cbdam/base/compressed_rgba32_image.hpp>

namespace cbdam {

  class geoimage_quad_fetcher: public geodata_fetcher<compressed_rgba32_image> {
  public:
    typedef compressed_rgba32_image			compressed_image_t;
    typedef compressed_image_t::genalpha_behavior_t	genalpha_behavior_t;
    typedef geodata_fetcher<compressed_image_t>		super_t;
    typedef super_t::key_t				key_t;
    typedef super_t::status_t				status_t;
    typedef super_t::value_t				value_t;
 
  protected:
    std::size_t		quad_width_;
    genalpha_behavior_t genalpha_behavior_;
    uint8_t		genalpha_constant_;
    uint8_t             genalpha_alphamin_;
    uint8_t             genalpha_alphamax_;

  public:

    geoimage_quad_fetcher(const std::string& url,
			  const std::string& desired_srs = "EPSG:4326",
			  const aabox2d_t&   desired_uv_box = aabox2d_t(point2d_t(-180.0,-90.0),
									point2d_t( 180.0, 90.0)),
			  const std::size_t& desired_quad_width = 256,			    
			  const std::string& default_about = "");

    virtual ~geoimage_quad_fetcher();

    inline std::size_t quad_width() const {
      return quad_width_;
    }
    
    inline genalpha_behavior_t genalpha_behavior() const {
      return genalpha_behavior_;
    }

    inline uint8_t genalpha_constant() const {
      return genalpha_constant_;
    }

    inline void set_genalpha_behavior(genalpha_behavior_t x,
				      uint8_t genalpha_constant = 0,
				      uint8_t genalpha_alphamin = 0,
				      uint8_t genalpha_alphamax = 255) {
      genalpha_behavior_ = x;
      genalpha_constant_ = genalpha_constant;
      genalpha_alphamin_ = genalpha_alphamin;
      genalpha_alphamax_ = genalpha_alphamax;
    }

  protected:

    virtual value_t* decoded(const uint8_t* buf,
			     std::size_t buf_size) const;
  };

}

#endif // GEOIMAGE_QUAD_FETCHER_HPP
