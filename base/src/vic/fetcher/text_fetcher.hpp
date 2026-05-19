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
#ifndef VIC_TEXT_FETCHER_HPP
#define VIC_TEXT_FETCHER_HPP

#include <vic/fetcher/fetcher.hpp>
#include <vector>

namespace vic {

  class text_fetcher: public fetcher<std::vector<char > > {
  public:
    typedef std::vector<char >            vector_t;
    typedef fetcher<vector_t >            super_t;
    typedef super_t::value_t              value_t;
    typedef super_t::status_data_pair_t   status_data_pair_t;
    
  protected:
    value_t* decoded(const sl::uint8_t* buf,
		     std::size_t buf_size) const;
    
  };
}

#endif // VIC_TEXT_FETCHER_HPP
