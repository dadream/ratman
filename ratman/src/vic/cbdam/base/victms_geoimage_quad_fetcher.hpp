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
#ifndef VICTMS_GEOIMAGE_QUAD_FETCHER_HPP
#define VICTMS_GEOIMAGE_QUAD_FETCHER_HPP

#include <vic/cbdam/base/geoimage_quad_fetcher.hpp>
#include <vic/geo/base/tms_tilemap_resource.hpp>

namespace cbdam {

  class victms_geoimage_quad_fetcher : public geoimage_quad_fetcher {
  public:
    typedef geoimage_quad_fetcher       super_t;
    typedef super_t::key_t              key_t;
    typedef super_t::status_t           status_t;
    typedef super_t::value_t            value_t;

  public:

    vic::geo::base::tms_tilemap_resource* tilemap_resource_;

  public:
    victms_geoimage_quad_fetcher(const std::string& url,
				 const std::string& desired_srs = "EPSG:4326",
				 const aabox2d_t&   desired_uv_box = aabox2d_t(point2d_t(-180.0,-90.0),
									       point2d_t( 180.0, 90.0)),
				 const std::size_t& desired_quad_width = 256,
				 const std::string& default_about = "");

    virtual ~victms_geoimage_quad_fetcher();

  public:
 
    int closest_quadtree_level(double res) const;
        
    key_t quadkey_from_request_box(const aabox2d_t& b) const;

    virtual bool is_out_of_bounds(const key_t& k, const aabox2d_t& k_uv_box) const;

  protected:

    virtual void direct_connect();

    virtual void direct_disconnect();

    virtual void direct_send_requests();

    virtual void direct_receive();

  protected:

    virtual void http_connect();

    virtual void http_disconnect();

    virtual std::string http_url_string(const key_t& k, const aabox2d_t& uv_box) const;

  };

} // namespace cbdam

#endif // VICTMS_GEOIMAGE_QUAD_FETCHER_HPP
