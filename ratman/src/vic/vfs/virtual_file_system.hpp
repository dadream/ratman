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
#ifndef VFS_VIRTUAL_FILE_SYSTEM_HPP
#define VFS_VIRTUAL_FILE_SYSTEM_HPP

#include <sl/cstdint.hpp>
#include <sl/fixed_size_array.hpp>
#include <QThread>

namespace vic {

  namespace vfs {
  
    /**
     * Virtual file system to access local/remote files and repositories.
     * Repository data is accessed through an index of type key_t
     */
    class virtual_file_system {
    public:
      typedef sl::uint8_t                         uint8_t;
      typedef sl::uint32_t                        uint32_t;
      typedef sl::int32_t			  int32_t;
      typedef sl::fixed_size_array<3,int32_t>	  key_t;
      typedef std::vector<uint8_t>                byte_array_t;

    public:
      virtual_file_system() { }
    
      virtual ~virtual_file_system() { }

      virtual FILE* open(const std::string& url, const char* mode) = 0;

      virtual void close(FILE* fp) = 0;

      virtual void fetch(const std::string& url, const key_t& key, byte_array_t& byte_array) = 0;

      virtual void multiple_fetch(const std::vector<std::string>& urls, 
				  const std::vector<key_t>& keys, 
				  std::vector<byte_array_t>& byte_arrays);

      virtual void write(const std::string& url, const key_t& key, const byte_array_t& byte_array, uint32_t expected_average_data_size) = 0;

    protected:
  
    };
  
  } // namespace vfs 

} // namespace vfs

#endif // VFS_VIRTUAL_FILE_SYSTEM_HPP

#ifndef VFS_VIRTUAL_FILE_SYSTEM_IPP
#define VFS_VIRTUAL_FILE_SYSTEM_IPP

namespace vic {
  namespace vfs {

    inline void virtual_file_system::multiple_fetch(const std::vector<std::string>& urls, 
						    const std::vector<key_t>& keys, 
						    std::vector<byte_array_t>& byte_arrays) {
      uint32_t size = urls.size();
      for(uint32_t i = 0; i < size; ++i) {
	fetch(urls[i], keys[i], byte_arrays[i]);
      }
    }

  } // namespace vfs 
} // namespace vic

#endif // VFS_VIRTUAL_FILE_SYSTEM_IPP
