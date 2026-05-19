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
#include <vic/vfs/sl_repository.hpp>
#include <cassert>

namespace vic {
  namespace vfs {

    sl_repository::sl_repository() {
      data_repo_ = 0;
    }

    sl_repository::~sl_repository() {
      close();
    }

    void sl_repository::open_read(const std::string& filename) {
      if (is_open()) close();

      file_name_ = filename;
      write_mode_ = false;
      is_temporary_ = false;

      // read map
      std::string map_file = file_name_ + ".map";
      compact_map_fread(map_file.c_str());
      if (!io_operation_success()) {
	return;
      }

      // open data xarray
      data_repo_ = new data_xarray_t(file_name_ + ".repo", "r", 16*1024*1024);
      if (!data_repo_->is_open()) {
	delete data_repo_;
	data_repo_ = 0;
	std::cerr << "unable to open " << file_name_ << " for reading" << std::endl;
      }
    }

    void sl_repository::open_write(const std::string& filename, std::size_t /*expected_average_data_size*/, bool is_temporary) {
      if (is_open()) close();

      file_name_ = filename;
      write_mode_ = true;
      is_temporary_ = is_temporary;

      if (is_temporary_) {
	data_repo_ = new data_xarray_t(file_name_ + ".repo", "t", 16*1024*1024);
      } else {
	data_repo_ = new data_xarray_t(file_name_ + ".repo", "w", 16*1024*1024);
      }
    
      if (!data_repo_->is_open()) {
	delete data_repo_;
	data_repo_ = 0;
	std::cerr << "unable to open " << file_name_ << " for writing " << (is_temporary_ ? "temporary" : "permanent") << std::endl;
      } 
    }

    void sl_repository::close() {
      // close repo
      if (data_repo_) {
	data_repo_->close();
	delete data_repo_;
	data_repo_ = 0;
      } 

      // write out map only if in write mode not temporary and map is not empty:
      // we don't want that 2 subsequent calls of close erase the saved key map.
      key_data_handle_sorted_vector_.clear();
      if (write_mode_ &&				
	  !is_temporary_ &&				
	  !key_data_handle_sorted_map_.empty()) {
	std::string map_file = file_name_ + ".map";
	compact_map_fwrite(map_file.c_str());
	key_data_handle_sorted_map_.clear();
      }
      
      assert(key_data_handle_sorted_vector_.empty());
      assert(key_data_handle_sorted_map_.empty());
    }

    void sl_repository::set_data(const key_t& key, const uint8_t* data_buffer, uint32_t size) {
      assert(write_mode());
      assert(is_open());

      key_data_handle_sorted_map_t::iterator it = key_data_handle_sorted_map_.find(key);
      if (it != key_data_handle_sorted_map_.end() &&
	  it->second.second <= size) {
	// KEY already present and enough room for this new data: replace the old one
	uint32_t& old_size = it->second.second;
	uint64_t offset = it->second.first;
	uint8_t* data = data_repo_->range_page_in(offset, offset + old_size - 1);
     
	old_size = size;
	memcpy(data, data_buffer, size);
      } else {
	// APPEND
	uint64_t repo_size = data_repo_->size();
	key_data_handle_sorted_map_[key] = std::make_pair(repo_size, size);

	// copy to output
	uint64_t updated_size = repo_size + size;
	data_repo_->resize(updated_size);
	uint8_t* data = data_repo_->range_page_in(repo_size, updated_size);
	memcpy(data, data_buffer, size);
      }
    }

    const sl::uint8_t* sl_repository::get_data(const key_t& key, uint32_t& size) const {
      uint64_t offset = 0;
      size = 0;
      if (write_mode()) {
	key_data_handle_sorted_map_t::const_iterator it = key_data_handle_sorted_map_.find(key);
	if (it != key_data_handle_sorted_map_.end()) {
	  size = it->second.second;
	  offset = it->second.first;
	}
      } else {
	key_data_handle_sorted_vector_t::const_iterator it = sorted_vector_iterator(key);
	if (it != key_data_handle_sorted_vector_.end()) {
	  size = it->second.second;
	  offset = it->second.first;
	}
      }	  

      return 
	(size) 
	? 
	data_repo_->range_page_in(offset, offset + size - 1) 
	: 
	0;
    }

    void sl_repository::minimize_footprint() const {
      if (data_repo_) data_repo_->minimize_footprint();
    }

    sl::uint64_t sl_repository::size() const {
      const float gzcompress_factor = 0.30f; // FIXME Approx
      return (uint64_t)(gzcompress_factor * (number_of_elements() * (sizeof(key_t) + sizeof(data_handle_t))) +
			data_repo_->size());
    }
  
    void sl_repository::compact_map_fwrite(const char* filename) const {
      gzFile fp = gzopen(filename, "wb");
      if (fp == 0) {
	std::cerr << "unable to open map file " << filename << " for writing" << std::endl;
	io_operation_success_ = false;
	return;
      } 

      uint32_t bytes_written = 0;
      uint32_t key_bytes_written = 0;
      uint32_t data_bytes_written = 0;
      uint32_t count = 0;

      uint32_t x_size = key_data_handle_sorted_map_.size();
      bytes_written+= gzwrite(fp, &x_size, sizeof(uint32_t));
      key_data_handle_sorted_map_t::const_iterator it;
      for(it = key_data_handle_sorted_map_.begin(); it != key_data_handle_sorted_map_.end(); ++it) {
	// store key
	const key_t& x_id = it->first;
	key_bytes_written += gzwrite(fp, &x_id, sizeof(key_t));

	// store data
	uint64_t x_offset = it->second.first;
	data_bytes_written += gzwrite(fp, &(x_offset), sizeof(uint64_t));

	uint32_t x_size = it->second.second;
	data_bytes_written += gzwrite(fp, &(x_size), sizeof(uint32_t));

	++count;
      }

      bytes_written += key_bytes_written + data_bytes_written;
      gzclose(fp);
      io_operation_success_ = true;

      SL_TRACE_OUT(1) << "written: " << filename << "\n\tkeys " << count << "\n\tbytes " << bytes_written 
		      << "\n\tkey_bytes_written " << key_bytes_written << "\n\tdata_bytes_written " << data_bytes_written << std::endl;
    }
  
    void sl_repository::compact_map_fread(const char* filename) {
      key_data_handle_sorted_map_.clear();
      key_data_handle_sorted_vector_.clear();

      gzFile fp = gzopen(filename, "rb");
      if (fp == 0) {
	std::cerr << "unable to open map file " << filename << " for reading" << std::endl;
	io_operation_success_ = false;
	return;
      } 

      // FIXME - check for errors with ferror, feof
      uint32_t bytes_read = 0;
      uint32_t key_bytes_read = 0;
      uint32_t data_bytes_read = 0;
      uint32_t x_size;
      bytes_read+= gzread(fp, &x_size, sizeof(uint32_t));
      
      if (write_mode()) {
	SL_TRACE_OUT(1) << "fread: MAP SIZE = " << x_size << std::endl;
      } else {
	SL_TRACE_OUT(1) << "fread: VECTOR SIZE = " << x_size << std::endl;
	key_data_handle_sorted_vector_.resize(x_size);
      }
	
      for (uint32_t i=0; i<x_size; ++i) {
	key_t x_id;
	key_bytes_read+= gzread(fp, &x_id, sizeof(key_t));

	uint64_t x_offset;
	data_bytes_read+= gzread(fp, &x_offset, sizeof(uint64_t));
      
	uint32_t x_size;
	data_bytes_read+= gzread(fp, &x_size, sizeof(uint32_t));
	
	if (write_mode()) {
	  key_data_handle_sorted_map_[x_id] = std::make_pair(x_offset, x_size);
	} else {
	  key_data_handle_sorted_vector_[i] = std::make_pair(x_id, std::make_pair(x_offset, x_size));
	}
	if (i<10) SL_TRACE_OUT(1) << "x_id = " << x_id << std::endl;
      }

      bytes_read += key_bytes_read + data_bytes_read;
    
      gzclose(fp);
      io_operation_success_ = true;

      if (write_mode()) {
	assert(key_data_handle_sorted_vector_.empty());
	SL_TRACE_OUT(-1) << "fread: finished - MAP SIZE = " << key_data_handle_sorted_map_.size() << std::endl;
      } else {
	assert(key_data_handle_sorted_map_.empty());
	SL_TRACE_OUT(-1) << "fread: finished - VECTOR SIZE = " << key_data_handle_sorted_vector_.size() << 
	  " (" << key_data_handle_sorted_vector_.size()*sizeof(key_data_handle_pair_t)/1024 << "KB" << 
	  std::endl;
      }	
    }

  }
}
