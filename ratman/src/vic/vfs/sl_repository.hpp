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
#ifndef VFS_SL_REPOSITORY_HPP
#define VFS_SL_REPOSITORY_HPP

#include <sl/external_array.hpp>
#include <sl/fixed_size_array.hpp>
#include <zlib.h>
#include <cstring>
#include <map>
#include <vector>
#include <algorithm>

namespace vic {

  namespace vfs {
  
    /**
     *
     */
    class sl_repository {
    public:
      // Basic data types 
      typedef sl::uint8_t                         uint8_t;
      typedef sl::uint16_t                        uint16_t;
      typedef sl::uint32_t                        uint32_t;
      typedef sl::uint64_t                        uint64_t;
      typedef sl::int32_t                         int32_t;
 
      typedef sl::fixed_size_array<3,int32_t>	  key_t;
      typedef std::pair<uint64_t, uint32_t>	  data_handle_t;
      typedef std::pair<key_t, data_handle_t>     key_data_handle_pair_t;
      typedef std::vector<key_data_handle_pair_t> key_data_handle_sorted_vector_t;
      typedef std::map<key_t, data_handle_t>      key_data_handle_sorted_map_t;
      typedef sl::external_array1<uint8_t>        data_xarray_t;

    protected:
      key_data_handle_sorted_map_t    key_data_handle_sorted_map_;
      key_data_handle_sorted_vector_t key_data_handle_sorted_vector_;

      data_xarray_t*      data_repo_;
      std::string         file_name_;
      bool                write_mode_;
      bool                is_temporary_;
      mutable bool        io_operation_success_;
    
    public:
      sl_repository();

      ~sl_repository();

      void open_read(const std::string& filename);

      void open_write(const std::string& filename, 
		      std::size_t expected_average_data_size, 
		      bool is_temporary = false);
    
      void close();

      bool is_open() const;

      const std::string& file_name() const;

      void set_data(const key_t& key, const uint8_t* data_buffer, uint32_t size);

      bool has_data(const key_t& key) const;
    
      const uint8_t* get_data(const key_t& key, uint32_t& size) const;

      void minimize_footprint() const;

      uint64_t size() const;

      uint32_t number_of_elements() const;

      bool write_mode() const;

      const key_data_handle_sorted_map_t& key_data_map() const;

      const key_data_handle_sorted_vector_t& key_data_vector() const;

    protected:
      bool io_operation_success() const;
    
      void compact_map_fwrite(const char* filename) const;

      void compact_map_fread(const char* filename);

      key_data_handle_sorted_vector_t::const_iterator sorted_vector_iterator(const key_t& key) const;

    };


  } // namespace vfs 
} // namespace vic

#endif // VFS_SL_REPOSITORY_HPP

#ifndef VFS_SL_REPOSITORY_IPP
#define VFS_SL_REPOSITORY_IPP

namespace vic {

  namespace vfs {

    inline bool sl_repository::is_open() const {
      return data_repo_ != 0 && data_repo_->is_open();
    }

    inline bool sl_repository::write_mode() const {
      return write_mode_;
    }

    inline const std::string& sl_repository::file_name() const {
      return file_name_;
    }

    inline bool sl_repository::has_data(const key_t& key) const {
      return 
	(write_mode()) 
	?
	(key_data_handle_sorted_map_.find(key) != key_data_handle_sorted_map_.end())
	:
	(sorted_vector_iterator(key) != key_data_handle_sorted_vector_.end());
    }
    
    inline sl::uint32_t sl_repository::number_of_elements() const {
      return
 	(write_mode()) 
	?
	(key_data_handle_sorted_map_.size())
	:
	(key_data_handle_sorted_vector_.size());
    }
  
    inline bool sl_repository::io_operation_success() const {
      return io_operation_success_;
    }

    inline const sl_repository::key_data_handle_sorted_map_t& sl_repository::key_data_map() const {
      return key_data_handle_sorted_map_;
    }

    inline const sl_repository::key_data_handle_sorted_vector_t& sl_repository::key_data_vector() const {
      return key_data_handle_sorted_vector_;
    }
    
    inline sl_repository::key_data_handle_sorted_vector_t::const_iterator sl_repository::sorted_vector_iterator(const key_t& key) const {
	key_data_handle_pair_t val = std::make_pair(key, data_handle_t(0,0));

	key_data_handle_sorted_vector_t::const_iterator it = 
	  std::lower_bound(key_data_handle_sorted_vector_.begin(),
			   key_data_handle_sorted_vector_.end(),
			   val);
	if ((it == key_data_handle_sorted_vector_.end()) || // not found
	    (it->first == key)) { // found
	  return it; 
	} else {
	  return key_data_handle_sorted_vector_.end(); // found something else
	}
      }

  } // namespace vfs 
}

#endif // VFS_SL_REPOSITORY_IPP
