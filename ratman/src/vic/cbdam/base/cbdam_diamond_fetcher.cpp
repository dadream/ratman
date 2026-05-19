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
#include <vic/cbdam/base/cbdam_diamond_fetcher.hpp>
#include <vic/cbdam/base/diamond_operator.hpp>
#include <vic/cbdam/base/byte_array_accessor.hpp>

namespace cbdam {

  cbdam_diamond_fetcher::cbdam_diamond_fetcher(const std::string& url,
					       const std::string& default_about) :
    super_t(url, 
	    "EPSG:4326", 
	    aabox2d_t(point2d_t(-180.0,-90.0),
		      point2d_t( 180.0, 90.0)),
	    default_about) {
    codec_.set_is_compressing_header(true);
  }

  cbdam_diamond_fetcher::~cbdam_diamond_fetcher() { 
    disconnect();
  }

  void cbdam_diamond_fetcher::direct_connect() {
    // base url is something like cbdamrepo.data or cbdamrepo.root, while params are in cbdamrepo.xml
    std::string file_name = sl::pathname_without_extension(base_url()) + ".xml";
    repository_parameters_.read_from_file(file_name.c_str(), true);
    is_connected_ = repository_parameters_.last_operation_success();

    srs_ = repository_parameters_.srs();
    about_ = repository_parameters_.about();
    uv_box_ = repository_parameters_.get_coordinate_transform()->bounding_rectangle();

    if (is_connected_) {
      data_repo_.open_read(base_url());
      if (!data_repo_.is_open()) {
	SL_TRACE_OUT(-1) << "LOCAL: Failed to open " << base_url() << " for reading" << std::endl;
	is_connected_ = false;
      } else {
	SL_TRACE_OUT(-1) << "LOCAL: " << base_url() << " open for reading" << std::endl;
	is_connected_ = true;
      }
    }
  }

  void cbdam_diamond_fetcher::direct_disconnect() {
    if (data_repo_.is_open()) {
      data_repo_.close();
    }
    is_connected_ = false;
  }
    
  cbdam_diamond_fetcher::value_t* cbdam_diamond_fetcher::decoded(const uint8_t* buf,
								 std::size_t buf_size) const {
    value_t* result = 0;

    if (byte_array_accessor::sanity_check(buf, buf_size)) {
      result = new value_t();
      height_operator::decompress_to(*result,
				     byte_array_accessor::first_patch_pointer(buf),
				     byte_array_accessor::first_patch_size(buf),
				     &codec_);
    }
    return result;
  }

  void cbdam_diamond_fetcher::http_connect() {
    std::string file_name = sl::pathname_without_extension(base_url()) + ".xml";
    SL_TRACE_OUT(1) << "reading parameters from " << file_name << std::endl;
    repository_parameters_.read_from_file(file_name.c_str(), true);
    is_connected_ = repository_parameters_.last_operation_success();

    srs_ = repository_parameters_.srs();
    about_ = repository_parameters_.about();
    uv_box_ = repository_parameters_.get_coordinate_transform()->bounding_rectangle();
  }

  void cbdam_diamond_fetcher::http_disconnect() {
    // Nothing to do
    is_connected_ = false;
  }

  std::string cbdam_diamond_fetcher::http_url_string(const key_t& k, const aabox2d_t& /*uv_box*/) const {
    std::string result = base_url();
    result += ".vicrepo";
    result += "?i=" + sl::to_string(k[0]);
    result += "&j=" + sl::to_string(k[1]);
    result += "&k=" + sl::to_string(k[2]);
    return result;
  }
    
  void cbdam_diamond_fetcher::direct_send_requests() {
    // Do nothing
  }

  void cbdam_diamond_fetcher::direct_receive() {
    if (!data_repo_.is_open()) {
      SL_TRACE_OUT(-1) << "DATA REPO NOT OPEN!" << std::endl;
      pending_requests_.clear();
    } else {
      const int max_served_request_per_receive = 2;
      int received_count = 0;

      for(std::map<key_t, aabox2d_t >::iterator it = pending_requests_.begin();
	  it != pending_requests_.end() && received_count < max_served_request_per_receive;
	  ) {
	// read data from db and decompress to map_received_data_
	const grid_point_t& gp = it->first; // it->second is the uv_bbox
	sl::fixed_size_array<3, int> key(gp[0], gp[1], gp[2]);
      
	sl::uint32_t   data_sz=0;
	const uint8_t* data = data_repo_.get_data(key, data_sz);
	
	if (data) {
	  SL_TRACE_OUT(1) << gp << " DATA!" << std::endl;
	  handle_data_response(gp, data, data_sz);
	} else {
	  SL_TRACE_OUT(1) << gp << " NO DATA!" << std::endl;
	  handle_nodata_response(gp);
	}

	++received_count;
	
	// remove from pending
	std::map<key_t, aabox2d_t>::iterator del_it = it;
	++it;
	pending_requests_.erase(del_it);
      }
    }
  }

}
