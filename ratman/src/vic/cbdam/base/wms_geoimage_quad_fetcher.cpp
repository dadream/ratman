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
#include <vic/cbdam/base/wms_geoimage_quad_fetcher.hpp>
#include <vic/curlstream/curlstream.hpp>
#include <QDebug>
#include <QString>
namespace cbdam {
  
  wms_geoimage_quad_fetcher::wms_geoimage_quad_fetcher(const std::string& url,
						       const std::string& layers,
						       const std::string& image_format,
						       const std::string& extra_arguments,
						       const std::string& desired_srs,
						       const aabox2d_t&   desired_uv_box,
						       const std::size_t& desired_quad_width,
						       const std::string& default_about) :
    super_t(url, desired_srs, desired_uv_box, desired_quad_width, default_about) {
    
    layers_ = layers;
    image_format_ = image_format;
    extra_arguments_ = extra_arguments;
  }

  wms_geoimage_quad_fetcher::~wms_geoimage_quad_fetcher() {
    clear();
  }

  void wms_geoimage_quad_fetcher::http_connect() {
    // FIXME: perform a getcapabilities and check
    SL_TRACE_OUT(-1) << "WMS connect not implemented " << base_url() << std::endl;
    
    is_connected_ = true;
  }

  void wms_geoimage_quad_fetcher::http_disconnect() {
    is_connected_ = false;
  }

  std::string wms_geoimage_quad_fetcher::http_url_string(const key_t& /*k*/, const aabox2d_t& uv_box) const {
    
    std::string result = base_url();
    //    result += "?Service=WMS&Version=1.1.0&Request=GetMap";
    result += "?SERVICE=WMS&REQUEST=GETMAP";
    result += "&BBOX="+sl::to_string(uv_box[0][0])+","+sl::to_string(uv_box[0][1])+","+sl::to_string(uv_box[1][0])+","+sl::to_string(uv_box[1][1]);
    result += "&SRS="+srs();
    result += "&WIDTH="+sl::to_string(quad_width())+"&HEIGHT="+sl::to_string(quad_width());
    result += "&LAYERS="+layers_;
    result += "&FORMAT="+image_format_;
    if (extra_arguments_ != "") {
      result += "&"+extra_arguments_;
    }

    SL_TRACE_OUT(1) << result << std::endl;
		
    return result;
  }

} // namespace cbdam
