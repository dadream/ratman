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
#include <vic/vfs/virtual_file_system_local.hpp>

namespace vic {
  namespace vfs {

    virtual_file_system_local::virtual_file_system_local() {
      max_opened_repositories_ = 10;
      current_request_time_ = 0;
    }

    virtual_file_system_local::~virtual_file_system_local() {
      for(map_iterator_t mi = read_repositories_.begin();
	  mi != read_repositories_.end();
	  ++mi) {
	mi->second.first->close();
	delete mi->second.first;
      }
      read_repositories_.clear();

      for(map_iterator_t mi = write_repositories_.begin();
	  mi != write_repositories_.end();
	  ++mi) {
	mi->second.first->close();
	delete mi->second.first;
      }
      write_repositories_.clear();
    }

    FILE* virtual_file_system_local::open(const std::string& url, const char* mode) {
      return fopen(url.c_str(), mode);
    }

    void virtual_file_system_local::close(FILE* fp) {
      fclose(fp);
    }
 
    void virtual_file_system_local::set_max_opened_repositories(uint32_t x) {
      max_opened_repositories_ = x;
    }

    void virtual_file_system_local::fetch(const std::string& url, const key_t& key, byte_array_t& byte_array) {
      mutex_.lock();
      {
	++current_request_time_;

	map_iterator_t mi = read_repositories_.find(url);
	if (mi == read_repositories_.end()) {
	  // open a new repository if not present in the map
	  clear_last_repository(read_repositories_);
	  repository_t* r = new repository_t;
	  r->open_read(url);
	  read_repositories_[url] = std::make_pair(r, current_request_time_);
	  mi = read_repositories_.find(url);
	} else {
	  // update last access time
	  mi->second.second = current_request_time_;
	}

	uint32_t size;
	const uint8_t* data = mi->second.first->get_data(key, size);
	if (data) {
	  // data present in repository, copy to buffer
	  byte_array.resize(size);
	  memcpy(&(byte_array[0]), data, size);
	} else {
	  byte_array.resize(0);
	}
	if (current_request_time_ == 0) {
	  clear_repository_access_times();
	}
      }
      mutex_.unlock();
    }

    void virtual_file_system_local::write(const std::string& url, const key_t& key, 
					  const byte_array_t& byte_array, uint32_t expected_average_data_size) {
      mutex_.lock();
      {
	++current_request_time_;

	map_iterator_t mi = write_repositories_.find(url);
	if (mi == write_repositories_.end()) {
	  // open a new repository if not present in the map
	  clear_last_repository(write_repositories_);
	  repository_t* r = new repository_t;
	  r->open_write(url, expected_average_data_size);
	  write_repositories_[url] = std::make_pair(r, current_request_time_);
	  mi = write_repositories_.find(url);
	} else {
	  // update last access time
	  mi->second.second = current_request_time_;
	}
     
	mi->second.first->set_data(key, &(byte_array[0]), byte_array.size());

	if (current_request_time_ == 0) {
	  clear_repository_access_times();
	}
      }
      mutex_.unlock();
    }

    void virtual_file_system_local::clear_repository_access_times() {
      // reset_repository_times after the 4G loop has been done
      for(map_iterator_t mi = read_repositories_.begin();
	  mi != read_repositories_.end();
	  ++mi) {
	mi->second.second = 0;
      }

      for(map_iterator_t mi = write_repositories_.begin();
	  mi != write_repositories_.end();
	  ++mi) {
	mi->second.second = 0;
      }
    }

    void virtual_file_system_local::clear_last_repository(map_repositories_t& map_repo) {
      if (map_repo.size() > max_opened_repositories_) {
	map_iterator_t delendo = map_repo.begin();
	uint32_t delendo_access_time = current_request_time_;
	for(map_iterator_t mi = map_repo.begin();
	    mi != map_repo.end();
	    ++mi) {
	  if (mi->second.second < delendo_access_time) {
	    delendo = mi;
	    delendo_access_time = mi->second.second;
	  }
	}      

	delete delendo->second.first;
	map_repo.erase(delendo);
      }    
    } 

  } //namespace vfs
} //namespace vic
