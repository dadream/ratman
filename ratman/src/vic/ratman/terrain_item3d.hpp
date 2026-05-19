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
#ifndef RATMAN_TERRAIN_ITEM3D_HPP
#define RATMAN_TERRAIN_ITEM3D_HPP

#include <vic/ratman/ratman.hpp>
#include <vic/ratman/active_renderable.hpp>
#include <sl/fixed_size_point.hpp>
//#include <istream>

namespace ratman {

   /**
   *  An interface for 3D terrain items
   */
  class terrain_item3d: public active_renderable {
  protected:
    point3d_t   location_;    /// longitude, latitude, elevation
    point3f_t   rotation_;    /// heading, pitch, roll
    float       scale_;       /// scale
    std::string id_;          /// id, name, etc.
    std::string url_file_;  /// url of the item3d file

  public:

    terrain_item3d(decorated_terrain_view* scene, const std::string& id, const std::string& url_file);

    ~terrain_item3d();

  public:

    /// Activate/deactivate the object
    virtual void set_active(bool x);

    /// Handle picking and return true if handled
    virtual bool on_select_self(const projective_map3d_t& P,
                                const rigid_body_map3d_t& V,
                                const point2d_t& xy);

    /// Render the hierarchy object rooted at this
    virtual void render_self(qgl_scene_view& qgl,
                             occupancy_map_t& occupancy_map,
			     const projective_map3d_t& P,
                             const rigid_body_map3d_t& V,
			     const point3d_t& C);


    /// Asynchronously update the object. Called in a separate thread.
    virtual void async_update_self(const projective_map3d_t& P,
                                   const rigid_body_map3d_t& V,
				   const point3d_t& G);

  };

}

#endif
