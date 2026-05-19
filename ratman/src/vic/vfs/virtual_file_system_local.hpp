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
#ifndef VFS_VIRTUAL_FILE_SYSTEM_LOCAL_HPP
#define VFS_VIRTUAL_FILE_SYSTEM_LOCAL_HPP

#include <vic/vfs/virtual_file_system.hpp>
#include <vic/vfs/repository.hpp>
#include <QMutex>

namespace vic {
  namespace vfs {
  
    /**
     *
     */
    class virtual_file_system_local : public virtual_file_system {
    public:
      typedef virtual_file_system	        super_t;
      typedef super_t::key_t                    key_t;
      typedef super_t::byte_array_t		byte_array_t;
      typedef super_t::uint8_t			uint8_t;
      typedef super_t::uint32_t			uint32_t;

      typedef repository	                                  repository_t;
      typedef std::pair<repository_t*, uint32_t>		  repository_time_pair_t;
      typedef std::map<std::string, repository_time_pair_t>	  map_repositories_t;
      typedef map_repositories_t::iterator			  map_iterator_t;
      typedef map_repositories_t::const_iterator		  map_const_iterator_t;

    protected:
      map_repositories_t	read_repositories_;
      map_repositories_t	write_repositories_;
      uint32_t		max_opened_repositories_;
      uint32_t		current_request_time_;
      QMutex              mutex_;
    
    public:
      virtual_file_system_local();

      virtual ~virtual_file_system_local();

      virtual FILE* open(const std::string& url, const char* mode);

      virtual void close(FILE* fp);

      virtual void fetch(const std::string& url, const key_t& key, byte_array_t& byte_array);

      virtual void write(const std::string& url, const key_t& key, const 
			 byte_array_t& byte_array, uint32_t expected_average_data_size);

      void set_max_opened_repositories(uint32_t x);

    protected:
      void clear_last_repository(map_repositories_t& map_repo);

      void clear_repository_access_times();
    };


  } // namespace vfs 
} // namespace vic

#endif // VFS_VIRTUAL_FILE_SYSTEM_LOCAL_HPP

#ifndef VFS_VIRTUAL_FILE_SYSTEM_LOCAL_IPP
#define VFS_VIRTUAL_FILE_SYSTEM_LOCAL_IPP
namespace vic {
  namespace vfs {

  } // namespace vfs 
} // namespace vic 

#endif // VFS_VIRTUAL_FILE_SYSTEM_LOCAL_IPP
