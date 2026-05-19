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
#ifndef DUMMY_GEOIMAGE_QUAD_FETCHER_HPP
#define DUMMY_GEOIMAGE_QUAD_FETCHER_HPP

#include <vic/cbdam/base/geoimage_quad_fetcher.hpp>

namespace cbdam {

  class dummy_geoimage_quad_fetcher : public geoimage_quad_fetcher {
  public:
    typedef geoimage_quad_fetcher       super_t;
    typedef super_t::key_t              key_t;
    typedef super_t::status_t           status_t;
    typedef super_t::value_t            value_t;
  public:
    dummy_geoimage_quad_fetcher(const std::string& desired_srs = "EPSG:4326",
				const aabox2d_t&   desired_uv_box = aabox2d_t(point2d_t(-180.0,-90.0),
									      point2d_t( 180.0, 90.0)),
				const std::size_t& desired_quad_width = 256);

    virtual ~dummy_geoimage_quad_fetcher();

  protected: // Data decoding

    virtual value_t* decoded(const uint8_t* buf = 0,
			     std::size_t buf_size = 0) const;
  public:
        
    virtual void direct_connect();

    virtual void direct_disconnect();

    virtual void direct_send_requests();

    virtual void direct_receive();

  protected:

  };

} // namespace cbdam

#endif // DUMMY_GEOIMAGE_QUAD_FETCHER_HPP
