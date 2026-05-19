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
#include <vic/cbdam/base/geodata_fetcher.hpp>
#include <cstring>

namespace cbdam {

  namespace detail {

    std::size_t geodata_fetcher_curl_callback_append_to_byte_array(void *ptr,
								   std::size_t size,
								   std::size_t nmemb,
								   void *data) {
      std::vector<uint8_t> *ba = (std::vector<uint8_t>*)data;    
      const char *chunk = (const char*)ptr;
      const std::size_t chunk_size = size * nmemb; 
      const std::size_t old_ba_size = ba->size();
      const std::size_t new_ba_size = old_ba_size + chunk_size;
      ba->resize(new_ba_size);
      memcpy(&((*ba)[old_ba_size]), chunk, chunk_size);

      return chunk_size;
    }
  }
}
