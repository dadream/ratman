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
#include <vic/ratman/string_utility.hpp>
#include <cctype>

namespace ratman {

  std::string string_utility::to_lower(const std::string& s) {
    std::string result = s;
    const std::size_t len = result.length();
    for(std::size_t i=0; i!=len; ++i) {
      result[i] = std::tolower(result[i]);
    }
    return result;
  }

  void string_utility::split(const std::string& str, const std::string& delim, std::vector<std::string>& output) {
    std::size_t offset = 0;
    std::size_t delimIndex = str.find(delim, offset);    
    while (delimIndex != std::string::npos) {
      output.push_back(str.substr(offset, delimIndex - offset));
      offset += delimIndex - offset + delim.length();
      delimIndex = str.find(delim, offset);
    }
    output.push_back(str.substr(offset));
  }

  

} // namespace ratman
