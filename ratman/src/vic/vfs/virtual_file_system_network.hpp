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
#ifndef VFS_VIRTUAL_FILE_SYSTEM_NETWORK_HPP
#define VFS_VIRTUAL_FILE_SYSTEM_NETWORK_HPP

#include <vic/vfs/virtual_file_system.hpp>
#include <vic/vfs/virtual_file_system_local.hpp>
#include <sl/utility.hpp>
#include <cstdio>
#include <curl/curl.h>

#if LIBCURL_VERSION_NUM >= 0x071000 
#  define HAVE_LIBCURL_HTTP_PIPELINING 1
#else 
#  define HAVE_LIBCURL_HTTP_PIPELINING 0
#endif

//#  define HAVE_LIBCURL_HTTP_PIPELINING 0

namespace vic {

  namespace vfs {
  
    /**
     * Virtual file system to access local/remote files and repositories.
     * Repository data is accessed through an index of type key_t
     */
    class virtual_file_system_network: public virtual_file_system {
    public:
      typedef virtual_file_system		  super_t;
      typedef super_t::key_t                      key_t;
      typedef sl::uint8_t                         uint8_t;
      typedef sl::uint32_t                        uint32_t;
      typedef std::vector<uint8_t>                byte_array_t;
      
    protected:
#if HAVE_LIBCURL_HTTP_PIPELINING
      enum { MAX_SIMULTANEOUS_TRANSFERS = 4 }; // FIXME !!!!
#else
      enum { MAX_SIMULTANEOUS_TRANSFERS = 1 }; // FIXME !!!!
#endif

      CURL  *curl_connection_[MAX_SIMULTANEOUS_TRANSFERS];
      CURLSH *curl_shared_data_;
      
      virtual_file_system_local			  vfs_local_;

    public:
      virtual_file_system_network();
    
      virtual ~virtual_file_system_network();

      virtual FILE* open(const std::string& url, const char* mode);

      virtual void close(FILE* fp);

      virtual void fetch(const std::string& url, const key_t& key, byte_array_t& byte_array);

      virtual void multiple_fetch(const std::vector<std::string>& urls, 
				  const std::vector<key_t>& keys, 
				  std::vector<byte_array_t>& byte_arrays);

      virtual void write(const std::string& url, const key_t& key, 
			 const byte_array_t& byte_array, uint32_t expected_average_data_size);

    protected:

      virtual void group_fetch(std::size_t i0,
                               const std::vector<std::string>& urls, 
                               const std::vector<key_t>& keys, 
                               std::vector<byte_array_t>& byte_arrays);
  
      static std::size_t curl_callback_append_to_byte_array(void *ptr,
							    std::size_t size,
							    std::size_t nmemb,
							    void *data); 
    };
  
  } // namespace vfs 

} // namespace vfs

#endif // VFS_VIRTUAL_FILE_SYSTEM_NETWORK_HPP

#ifndef VFS_VIRTUAL_FILE_SYSTEM_NETWORK_IPP
#define VFS_VIRTUAL_FILE_SYSTEM_NETWORK_IPP

namespace vic {
  namespace vfs {

  
  } // namespace vfs 
} // namespace vic

#endif // VFS_VIRTUAL_FILE_SYSTEM_NETWORK_IPP
