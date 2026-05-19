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
#include <vic/fetcher/text_fetcher.hpp>

namespace vic {

  text_fetcher::value_t* text_fetcher::decoded(const sl::uint8_t* buf,
					       std::size_t buf_size) const {
    value_t* result = new value_t;
    result->resize(buf_size);
    for(std::size_t i=0; i<buf_size; ++i) {
      (*result)[i]=char(buf[i]);
    }
    return result;
  }
  


}
