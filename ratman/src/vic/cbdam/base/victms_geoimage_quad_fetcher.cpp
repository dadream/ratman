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
#include <vic/cbdam/base/victms_geoimage_quad_fetcher.hpp>
#include <vic/geo/base/victms_conventions.hpp>
#include <vic/curlstream/curlstream.hpp>

#ifdef _WIN32
  #undef max
  #undef min
#endif

namespace cbdam {
  
  victms_geoimage_quad_fetcher::victms_geoimage_quad_fetcher(const std::string& url,
							     const std::string& desired_srs,
							     const aabox2d_t&   desired_uv_box,
							     const std::size_t& desired_quad_width,
							     const std::string& default_about) :
    super_t(url, desired_srs, desired_uv_box, desired_quad_width, default_about) {

    tilemap_resource_ = 0;
  }

  victms_geoimage_quad_fetcher::~victms_geoimage_quad_fetcher() {
    clear();
  }

  int victms_geoimage_quad_fetcher::closest_quadtree_level(double res) const {
    if (!tilemap_resource_) return -1;

    const int N = tilemap_resource_->tileset_count();

    int l=0;
    const double r_l = tilemap_resource_->tileset_units_per_pixel(l);
    const double d_l = (res>r_l)?(res-r_l):(r_l-res);

    int    l_best = l;
    double r_best = r_l;
    double d_best = d_l;

    for (int l=1; l<N; ++l) {
      const double r_l = tilemap_resource_->tileset_units_per_pixel(l);
      const double d_l = (res>r_l)?(res-r_l):(r_l-res);
      if (d_l<d_best) {
	l_best = l;
	r_best = r_l;
	d_best = d_l;
      }
    }

    if (l_best==N-1) {
      // Check if out of bounds, i.e., beyond last level
      const double r_l = 0.5*r_best;
      const double d_l = (res>r_l)?(res-r_l):(r_l-res);
      if (d_l<d_best) {
	l_best = -1;
      }
    }

    return l_best;      
  }
 

  victms_geoimage_quad_fetcher::key_t victms_geoimage_quad_fetcher::quadkey_from_request_box(const aabox2d_t& b) const {
    if (!tilemap_resource_) return key_t(-1,-1,-1); // not connected

    const double quad_width = double(tilemap_resource_->img_width());
    const double quad_height = double(tilemap_resource_->img_height());

    const double b_resolution = std::max(b.diagonal()[0]/double(quad_width),
					 b.diagonal()[1]/double(quad_height));

    int    qt_level = closest_quadtree_level(b_resolution);

    if (qt_level<0) return key_t(-1,-1,-1); // out of bounds

    const double qt_resolution = tilemap_resource_->tileset_units_per_pixel(qt_level);

    const double xc = (0.5*(b[0][0]+b[1][0]) - tilemap_resource_->bounding_box()[0][0]) / (qt_resolution * quad_width);
    const double yc = (0.5*(b[0][1]+b[1][1]) - tilemap_resource_->bounding_box()[0][1]) / (qt_resolution * quad_height);

    int qt_x = int(xc);
    int qt_y = int(yc);

    const int qt_nx = int((tilemap_resource_->bounding_box()[1][0] - tilemap_resource_->bounding_box()[0][0] - qt_resolution) / (qt_resolution * quad_width));
    const int qt_ny = int((tilemap_resource_->bounding_box()[1][1] - tilemap_resource_->bounding_box()[0][1] - qt_resolution) / (qt_resolution * quad_height));

    if (qt_x<0 || qt_x>qt_nx || qt_y<0 || qt_y>qt_ny) {
      qt_level = -1;
      qt_x = -1;
      qt_y = -1;
    } 
    
    return key_t(qt_level, qt_x, qt_y);
  }

  bool victms_geoimage_quad_fetcher::is_out_of_bounds(const key_t& /*k*/, const aabox2d_t& k_uv_box) const {
    return quadkey_from_request_box(k_uv_box)[0]<0;
  }

  void victms_geoimage_quad_fetcher::direct_connect() {
    SL_TRACE_OUT(-1) << "Not implemented" << std::endl;
    is_connected_ = false;
  }

  void victms_geoimage_quad_fetcher::direct_disconnect() {
    SL_TRACE_OUT(-1) << "Not implemented" << std::endl;
    is_connected_ = false;
  }

  void victms_geoimage_quad_fetcher::direct_send_requests() {
    SL_TRACE_OUT(-1) << "Not implemented" << std::endl;
    super_t::direct_send_requests();
  }

  void victms_geoimage_quad_fetcher::direct_receive() {
    SL_TRACE_OUT(-1) << "Not implemented" << std::endl;
    super_t::direct_receive();
  }

  void victms_geoimage_quad_fetcher::http_connect() {
    vic::icurlstream service_stream;
    service_stream.open(base_url().c_str());
    if (!service_stream) {
      SL_TRACE_OUT(-1) << "Unable to connect to " << base_url() << std::endl;
    } else {
      std::string xml;
      while (service_stream.good()) {
	std::string line;
	std::getline(service_stream,line);
	xml += line + '\n';
      }
      service_stream.close();
      if (xml.empty()) {
	SL_TRACE_OUT(-1) << "Error while streaming from " << base_url() << std::endl;
      } else {
	SL_TRACE_OUT(1) << "xml: " << xml << std::endl;	

	tilemap_resource_ = new vic::geo::base::tms_tilemap_resource(xml);
	
	// FIXME: Check SRS, etc.
	SL_TRACE_OUT(-1) << "Check Tilemap SRS, BOX, ..." << std::endl;

	is_connected_ = tilemap_resource_->last_operation_success();
	if (!is_connected_) {
	  std::cerr << "unable to connect to " << base_url() << " error: " << tilemap_resource_->last_error_message() << std::endl;
	}
      }
    }
  }

  void victms_geoimage_quad_fetcher::http_disconnect() {
    is_connected_ = false;
  }

  std::string victms_geoimage_quad_fetcher::http_url_string(const key_t& k, const aabox2d_t& uv_box) const {
    if (!tilemap_resource_) return "NULL://0/0/0";

    const key_t kk = quadkey_from_request_box(uv_box);
    
    SL_TRACE_OUT(1) << 
      "key_orig = [" << k[0] << " " << k[1] << " " << k[2] << "]" << std::endl <<
      "key_from_box = [" << kk[0] << " " << kk[1] << " " << kk[2] << "]" << std::endl <<
      "box = (" << uv_box[0][0] << " " << uv_box[0][1] << ")(" << uv_box[1][0] << " " << uv_box[1][1] << std::endl;

    std::string result = base_url();
    result += "/";
    result += sl::to_string(kk[0]);
    result += "/";
    result += sl::to_string(kk[1]);
    result += "/";
    result += sl::to_string(kk[2]);
    result += ".";
    result += tilemap_resource_->img_extension();
    return result;
  }

} // namespace cbdam
