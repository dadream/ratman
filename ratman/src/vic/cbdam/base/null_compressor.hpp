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
#ifndef CBDAM_NULL_COMPRESSOR_HPP
#define CBDAM_NULL_COMPRESSOR_HPP

#include <vic/cbdam/base/config.hpp>
#include <sl/dense_array.hpp>
#include <cstring>

namespace cbdam {
  
  /**
   *
   */
  template<class T>
  class null_compressor {
  public:
    typedef sl::dense_array<T,2,void>   array2_t;
    typedef std::vector<uint8_t>        data_buffer_t;
    
  public:
    null_compressor();

    ~null_compressor();

    void compress_to(data_buffer_t& compressed, const array2_t& p);

    void decompress_to(array2_t& p, const uint8_t* compressed, uint32_t compressed_size);

  };


} // namespace cbdam 

#endif // CBDAM_NULL_COMPRESSOR_HPP

#ifndef CBDAM_NULL_COMPRESSOR_IPP
#define CBDAM_NULL_COMPRESSOR_IPP

namespace cbdam {

  template<class T>
  inline null_compressor<T>::null_compressor() {

  }

  template<class T>
  inline null_compressor<T>::~null_compressor() {

  }

  template<class T>
  inline void null_compressor<T>::compress_to(data_buffer_t& compressed, const array2_t& p) {
    compressed.resize(p.count() * sizeof(T) + 2 * sizeof(uint32_t));
    uint8_t* cursor = &(compressed[0]);
    uint32_t h =p.extent()[0];
    uint32_t w =p.extent()[1];
    
    std::memcpy(cursor, &h, sizeof(uint32_t));
    cursor += sizeof(uint32_t);
    std::memcpy(cursor, &w, sizeof(uint32_t));
    cursor += sizeof(uint32_t);
    
    for(uint32_t y = 0; y < h; ++y) {
      for(uint32_t x = 0; x < w; ++x) {
        T value = p(y,x);
        std::memcpy(cursor, &value, sizeof(T));
        cursor += sizeof(T);
      }
    }
    assert(cursor-&(compressed[0]) == (int)compressed.size());
  }

  template<class T>
  inline void null_compressor<T>::decompress_to(array2_t& p, const uint8_t* compressed, uint32_t compressed_size) {
    const uint8_t* cursor = compressed;
    uint32_t h, w;
    std::memcpy(&h, cursor, sizeof(uint32_t));
    cursor += sizeof(uint32_t);
    std::memcpy(&w, cursor, sizeof(uint32_t));
    cursor += sizeof(uint32_t);
    T value;
    p.resize(sl::index<2>(h,w));
    for(uint32_t y = 0; y < h; ++y) {
      for(uint32_t x = 0; x < w; ++x) {
        std::memcpy(&value, cursor, sizeof(T));
        p(y, x) = value;
        cursor += sizeof(T);
      }
    }
    assert(cursor-&(compressed[0]) == (int)compressed_size);
  }

} // namespace cbdam 

#endif // CBDAM_NULL_COMPRESSOR_IPP
