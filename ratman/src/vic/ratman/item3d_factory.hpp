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
#ifndef RATMAN_ITEM3D_FACTORY_HPP
#define RATMAN_ITEM3D_FACTORY_HPP

#include <vic/ratman/ratman.hpp>
#include <vic/ratman/terrain_item3d.hpp>

namespace ratman {

   /**
   *  The interface for a Factory for 3D terrain items
   */
  class item3d_factory {

  public:

    item3d_factory() {};

    virtual ~item3d_factory() {};

    /// Creates a 3d terrain item
    virtual terrain_item3d* create_item3d(decorated_terrain_view* scene, const std::string& name,
					  sl::point3d location, sl::point3d rotation, sl::point3d scale,
					  const std::string file) = 0;

    /// Managed render process
    virtual void render_managed() {};
  };

}

#endif
