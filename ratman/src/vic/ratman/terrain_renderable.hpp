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
#ifndef RATMAN_TERRAIN_RENDERABLE_HPP
#define RATMAN_TERRAIN_RENDERABLE_HPP

#include <vic/ratman/ratman.hpp>
#include <vic/ratman/active_renderable.hpp>

namespace cbdam {
  class terrain_model;
  class terrain_model_renderer;
}

namespace ratman {

  /**
   * The node of the active_renderable hierarchy which performs terrain rendering
   */
  class terrain_renderable: public active_renderable {
  protected:
    cbdam::terrain_model* model_;
    cbdam::terrain_model_renderer* renderer_;

    bool first_frame_; // Check

  public:

    terrain_renderable(decorated_terrain_view* scene, cbdam::terrain_model* model);
    
    ~terrain_renderable();

    /// the terrain model containing geometry and texture layers
    const cbdam::terrain_model* model() const;

    cbdam::terrain_model* model();

    const cbdam::terrain_model_renderer* renderer() const;

    cbdam::terrain_model_renderer* renderer();
   
  public:
    
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

    /// Enable/disable shading
    void set_shading_enabled(bool x);

    /// Is shading enabled ?
    bool is_shading_enabled() const;
  };
  
}

#endif
