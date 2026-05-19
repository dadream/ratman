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
#include <vic/persistent/map.hpp>
#include <map>
#include <iostream>
#include <iomanip>

///////////////////// FIXME MOVE TO SL

namespace sl {


  template <> struct serialization_traits<std::string> {
    static const bool is_array_base_type=false;
    static output_serializer& store(output_serializer& s, const std::string& x) {
      s.write_simple(x.size());
      if (x.size()) s.write_array(x.size(), (const uint8_t*)x.c_str()); return s;
    }
    static input_serializer& retrieve(input_serializer& s, std::string& x) {
      std::size_t sz;
      s.read_simple(sz);
      x.resize(sz);
      if (sz) s.read_array(sz, (uint8_t*)x.c_str());
      return s;
    }
  };

}
///////////////////// FIXME MOVE TO SL

int main() {
#if 0
  typedef  std::map<std::string, int> map_t;

  map_t months;
#else
  typedef  vic::persistent::map<std::string, int> map_t;

  map_t months("test_pmap.db", "t");
#endif
  months["january"] = 31;
  months["february"] = 28;
  months["march"] = 31;
  months["april"] = 30;
  months["may"] = 31;
  months["june"] = 30;
  months["july"] = 31;
  months["august"] = 31;
  months["september"] = 30;
  months["october"] = 31;
  months["november"] = 30;
  months["december"] = 31;

  std::cout << "Months: " << months.size() << std::endl;
  for (map_t::const_iterator it = months.begin();
       it != months.end();
       ++it) {
    std::cout << std::setw(20) << it->first << std::setw(4) << it->second << std::endl;
  }
  
  std::cout << "june -> " << months["june"] << std::endl;
  map_t::iterator cur  = months.find("june");
  map_t::iterator prev = cur;
  map_t::iterator next = cur;    
  ++next;
  --prev;
  std::cout << "Previous (in alphabetical order) is " << (*prev).first << std::endl;
  std::cout << "Next (in alphabetical order) is " << (*next).first << std::endl;
}



