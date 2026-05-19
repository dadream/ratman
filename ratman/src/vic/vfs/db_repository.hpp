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
#ifndef VFS_DB_REPOSITORY_HPP
#define VFS_DB_REPOSITORY_HPP

#include <sl/fixed_size_array.hpp>

typedef struct __db DB;

namespace vic {

  namespace vfs {

    /**
     *
     */
    class db_repository {
    public:
      // Basic data types 
      typedef sl::uint8_t                         uint8_t;
      typedef sl::uint16_t                        uint16_t;
      typedef sl::uint32_t                        uint32_t;
      typedef sl::uint64_t                        uint64_t;
      typedef sl::int32_t                         int32_t;
 
      typedef sl::fixed_size_array<3,int32_t>	  key_t;

    protected:
      DB*		db_;    
      std::string       file_name_;
      bool              write_mode_;
      bool              is_temporary_;
      mutable bool      io_operation_success_;
    
    public:
      db_repository();

      ~db_repository();

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

      uint64_t size() const;

      bool write_mode() const;

      void minimize_footprint() const;
    };


  } // namespace vfs 
} // namespace vic

#endif // VFS_DB_REPOSITORY_HPP

#ifndef VFS_DB_REPOSITORY_IPP
#define VFS_DB_REPOSITORY_IPP

namespace vic {

  namespace vfs {

    inline bool db_repository::is_open() const {
      return db_ != 0;
    }

    inline bool db_repository::write_mode() const {
      return write_mode_;
    }

    inline const std::string& db_repository::file_name() const {
      return file_name_;
    }

    inline void db_repository::minimize_footprint() const {
    }

  } // namespace vfs 
}

#endif // VFS_DB_REPOSITORY_IPP
