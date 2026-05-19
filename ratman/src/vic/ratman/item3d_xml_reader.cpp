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
#include <vic/ratman/item3d_xml_reader.hpp>
#include <vic/ratman/item3d_manager.hpp>

namespace ratman {

    void parse_item3d(node_iterator node) {
      std::string id;
      sl::point3d location, rotation, scale;
      if (node.has_attribute("id")) {
	id = node.attribute("id");
      }
      if (node.has_attribute("location")) {
	vic::xml::node_iterator::point3d_t val;
	val = node.attributep("location");
	location = val.c[0], val.c[1], val.c[2];
      }
      if (node.has_attribute("rotation")) {
	vic::xml::node_iterator::point3d_t val;
	val = node.attributep("rotation");
	rotation = val.c[0], val.c[1], val.c[2];
      }
      if (node.has_attribute("scale")) {
	vic::xml::node_iterator::point3d_t val;
	val = node.attributep("scale");
	scale = val.c[0], val.c[1], val.c[2];
      }

      if (node.has_attribute("file")) {
	std::string file = node.attribute("file");
	item3d_manager::instance()->create_item3d(id, location, rotation, scale, file);
#if 0
	for (unsigned int i = 0; i < 50; i++) {
	    location[0]+=0.001*i;
	    location[1]+=0.01*i;
	    item3d_manager::instance()->create_item3d(id, location, rotation, scale, file);
	}
#endif
      }
    }

    void item3d_xml_reader::read_node(node_iterator node) {
      assert(node.is_element_node());
      if ( node.tag() == "item3d_group" ) {
	for ( node_iterator child = node.down(); !child.is_null() ; child = child.next() ) {
	  if ( child.tag() == "item3d" ) {
	    parse_item3d(child);
	  }
	}
      }
      if ( node.has_attribute("item3d") ) {
        parse_item3d(node);
      }
    }
}

