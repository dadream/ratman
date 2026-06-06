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
#include <vic/geo/base/tilemap_config.hpp>
#include <vic/xml/document.hpp>
#include <vic/curlstream/curlstream.hpp>
#include <sstream>

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

	std::cerr << "[DEBUG] http_connect: XML read:\n" << xml << std::endl;
	if (xml.find("<victms>") != std::string::npos) {
	  std::cerr << "[DEBUG] http_connect: found <victms>" << std::endl;
	  // Translate victms.xml to TileMap XML
	  std::istringstream in(xml);
	  vic::xml::document doc;
	  doc.parse(in);
	  std::cerr << "[DEBUG] http_connect: doc error: " << (doc.error() ? "YES" : "NO") << " msg: " << doc.error_msg() << std::endl;
	  vic::xml::node_iterator root = doc.first_root("victms");
	  std::cerr << "[DEBUG] http_connect: root tag: " << (root.is_null() ? "NULL" : root.tag()) << std::endl;
	  if (!root.is_null()) {
	    for (vic::xml::node_iterator it = root.down(); !it.is_null(); it = it.next()) {
	      std::cerr << "[DEBUG] http_connect: child tag: " << it.tag() << std::endl;
	      if (it.tag() == "tilemap") {
		vic::geo::base::tilemap_config tc;
		if (tc.parse(it)) {
		  std::cerr << "[DEBUG] http_connect: successfully parsed tilemap_config name=" << tc.name() << std::endl;
		  std::ostringstream out;
		  std::string tileset_url = base_url();
		  // Strip filename (victms.xml) from tileset_url to get base path
		  size_t last_slash = tileset_url.find_last_of('/');
		  if (last_slash != std::string::npos) {
		    tileset_url = tileset_url.substr(0, last_slash + 1);
		  } else {
		    tileset_url = "";
		  }
		  out << "<TileMap version=\"1.0.0\" tilemapservice=\"local\">" << std::endl;
		  out << "  <Title>" << tc.name() << "</Title>" << std::endl;
		  out << "  <Abstract>VICTMS server: TileMap Resource for " << tc.name() << "</Abstract>" << std::endl;
		  out << "  <SRS>" << tc.srs() << "</SRS>" << std::endl;
		  out << "  <BoundingBox minx=\"" << tc.bbox_lo(0) << "\" miny=\"" << tc.bbox_lo(1) << "\" maxx=\"" << tc.bbox_hi(0) << "\" maxy=\"" << tc.bbox_hi(1) << "\" />" << std::endl;
		  out << "  <Origin x=\"" << tc.bbox_lo(0) << "\" y=\"" << tc.bbox_lo(1) << "\" />" << std::endl;
		  out << "  <TileFormat width=\"" << tc.img_width() << "\" height=\"" << tc.img_height() << "\" mime-type=\"" << tc.mime() << "\" extension=\"" << tc.extension() << "\" />" << std::endl;
		  out << "  <TileSets profile=\"" << tc.profile() << "\">" << std::endl;
		  for (int l = 0; l <= tc.max_level(); ++l) {
		    out << "    <TileSet href=\"" << tileset_url << l << "/\" units-per-pixel=\"" << tc.units_per_pixel(l) << "\" order=\"" << l << "\" />" << std::endl;
		  }
		  out << "  </TileSets>" << std::endl;
		  out << "</TileMap>" << std::endl;
		  xml = out.str();
		  break;
		}
	      }
	    }
	  }
	}

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

    std::string base = base_url();
    size_t last_slash = base.find_last_of('/');
    if (last_slash != std::string::npos) {
      base = base.substr(0, last_slash); // Strip filename (e.g. victms.xml)
    }

    std::string ext = tilemap_resource_->img_extension();

    // Check if it is a local file or local URL
    bool is_local = true;
    if (base.find("http://") == 0 || base.find("https://") == 0) {
      is_local = false;
    }

    if (is_local) {
      // For local files/URLs, we must use the padded victms directory structure conventions
      std::string path = vic::geo::base::victms_conventions::quad_filename(base, kk[0], kk[1], kk[2], ext);
      if (path.find("file://") != 0 && path.find("/") == 0) {
        path = "file://" + path;
      }
      return path;
    } else {
      // For remote HTTP servers, use standard TMS URL format
      std::string result = base;
      result += "/";
      result += sl::to_string(kk[0]);
      result += "/";
      result += sl::to_string(kk[1]);
      result += "/";
      result += sl::to_string(kk[2]);
      result += ".";
      result += ext;
      return result;
    }
  }

} // namespace cbdam
