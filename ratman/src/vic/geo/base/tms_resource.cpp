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
#include <vic/geo/base/tms_resource.hpp>
#include <iostream>
#include <sstream>
#include <cctype>       // std::tolower

namespace vic {

  namespace geo {

    namespace base {

      std::string tms_resource::to_lower(const std::string& s) {
	std::string result = s;
	const std::size_t len = result.length();
	for(std::size_t i=0; i!=len; ++i) {
	  result[i] = std::tolower(result[i]);
	}
	return result;
      }

      bool tms_resource::lc_equal(const std::string& s1, const std::string& s2) {
	return to_lower(s1) == to_lower(s2);
      }

      static void xml_pretty_print(vic::xml::node_iterator ptr,
				   std::string space) {
	if (!ptr.is_null()) {
	  if(!ptr.is_text_node()) {
	    std::cerr << space << "<" << ptr.tag() << ">" << std::endl;
	  } else {
	    std::cerr << space << ptr.text() << std::endl;
	  }
	  std::string child_space = "  " + space;
	  for(vic::xml::node_iterator child_it=ptr.down();
	      !child_it.is_null();
	      child_it = child_it.next()) {
	    xml_pretty_print(child_it,child_space);
	  }
	  if(!ptr.is_text_node())
	    std::cerr << space << "</" << ptr.tag() << ">" << std::endl;
	}
      }

      void tms_resource::xml_parse_string(const std::string& xml_description, const std::string& root_tag) {
	reset_error();
	std::istringstream in(xml_description);
	vic::xml::document doc;
	doc.parse(in);
	if(!doc.error()) {
	  vic::xml::node_iterator ptr=doc.first_root(root_tag);
	  if (ptr.is_null()) {
	    fail(std::string("Cannot find root tag: ")+root_tag); 
	  } else {
	    xml_parse(ptr);
	  }	  
	} else {
	  fail(doc.error_msg());
	}
      }
    }
  }
}
 
