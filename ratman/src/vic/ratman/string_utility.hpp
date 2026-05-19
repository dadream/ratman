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
#ifndef VIC_RATMAN_STRING_UTILITY_HPP
#define VIC_RATMAN_STRING_UTILITY_HPP

#include <vector>
#include <string>
#include <sstream>

namespace ratman {

  class string_utility {

  public:

    template<typename T>
    static T convert_into(const std::string& from_str) {
      T toT;
      std::istringstream(from_str)>>toT;
      return toT;
    }
    
    static std::string to_lower(const std::string& s);
    static void split(const std::string& str, const std::string& delim, std::vector<std::string>& output);

  };
  
} // namespace ratman

#endif


