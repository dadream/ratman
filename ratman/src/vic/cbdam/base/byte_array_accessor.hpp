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
#ifndef CBDAM_BYTE_ARRAY_ACCESSOR_HPP
#define CBDAM_BYTE_ARRAY_ACCESSOR_HPP

#include <vic/cbdam/base/config.hpp>
#include <sl/encdec.hpp>
#include <vector>

namespace cbdam {
  
  /**
   * Byte array memory structure: 
   *  uint32_t first_patch_size
   *  uint8_t  first_patch_data[first_patch_size]
   *  uint8_t  second_patch_data[size-first_patch_size-sizeof(uint32_t)].
   * Set of helper functions to access byte array chunks
   */
  class byte_array_accessor {
  public:

    static bool sanity_check(const uint8_t* data, uint32_t size);

    static const uint8_t* first_patch_pointer(const uint8_t* data);

    static const uint8_t* second_patch_pointer(const uint8_t* data);

    static uint8_t* first_patch_pointer(uint8_t* data);

    static uint8_t* second_patch_pointer(uint8_t* data);

    static uint32_t first_patch_size(const uint8_t* data);

    static void set_first_patch_size(uint8_t* data, uint32_t size);

    static uint32_t second_patch_size(const uint8_t* data, uint32_t size);
  };


} // namespace cbdam 

#endif // CBDAM_BYTE_ARRAY_ACCESSOR_HPP

#ifndef CBDAM_BYTE_ARRAY_ACCESSOR_IPP
#define CBDAM_BYTE_ARRAY_ACCESSOR_IPP

namespace cbdam {
  
  inline const uint8_t* byte_array_accessor::first_patch_pointer(const uint8_t* data) {
    return &(data[sizeof(uint32_t)]);
  }

  inline const uint8_t* byte_array_accessor::second_patch_pointer(const uint8_t* data) {
    return &(data[sizeof(uint32_t) + first_patch_size(data)]);
  }

  inline uint8_t* byte_array_accessor::first_patch_pointer(uint8_t* data) {
    return &(data[sizeof(uint32_t)]);
  }

  inline uint8_t* byte_array_accessor::second_patch_pointer(uint8_t* data) {
    return &(data[sizeof(uint32_t) + first_patch_size(data)]);
  }

  inline uint32_t byte_array_accessor::first_patch_size(const uint8_t* data) {
    return sl::le_decode<uint32_t>(data);
  }

  inline void byte_array_accessor::set_first_patch_size(uint8_t* data, uint32_t size) {
    sl::le_encode(size, data);
  }
  
  inline uint32_t byte_array_accessor::second_patch_size(const uint8_t* data, uint32_t size) {
    return size - first_patch_size(data) - sizeof(uint32_t);;
  }

  inline bool byte_array_accessor::sanity_check(const uint8_t* data, uint32_t size) {
    return 
      (data) &&
      (size>=sizeof(uint32_t)) && 
      (first_patch_size(data)<=(size-sizeof(uint32_t)));
  }

} // namespace cbdam 

#endif // CBDAM_BYTE_ARRAY_ACCESSOR_IPP
