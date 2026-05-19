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
#ifndef RATMAN_ITEM3D_XML_READER_HPP
#define RATMAN_ITEM3D_XML_READER_HPP

#include <vic/ratman/ratman.hpp>
#include <vic/ratman/terrain_item3d.hpp>
#include <vic/xml/document.hpp>

using namespace vic::xml;
   
namespace ratman {

     /**
     *  A document subclass to parse items3d xml files
     */
    class item3d_xml_reader : public document {

    protected:
      std::vector<terrain_item3d> items3d_;

    public:

      item3d_xml_reader() {};

      virtual ~item3d_xml_reader() {};

      virtual void read_node(node_iterator node);

    };

}

#endif
