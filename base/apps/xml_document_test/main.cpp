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
#include <vic/xml/document.hpp>
#include <string>
#include <iostream>
#include <fstream>

void pretty_print(vic::xml::node_iterator ptr, 
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
      pretty_print(child_it,child_space);
    }
    if(!ptr.is_text_node())
      std::cerr << space << "</" << ptr.tag() << ">" << std::endl;
  }
}

int main(int argc, char **argv) {

  if (argc != 2) {
    return 1;
  }
  
  std::string fname = std::string(argv[1]);

  std::ifstream in(fname.c_str(),std::ios::in | std::ios::binary);

  vic::xml::document doc;
  doc.parse(in);
  if(!doc.error()) {
    vic::xml::node_iterator ptr=doc.root();
    pretty_print(doc.root(),"");
  } else {
    std::cerr << doc.error_msg() << std::endl;
  }
}
