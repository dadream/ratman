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
#ifndef RATMAN_DECORATED_TERRAIN_VIEW_HPP
#define RATMAN_DECORATED_TERRAIN_VIEW_HPP

#include <vic/ratman/active_renderable.hpp>
#include <vic/ratman/terrain_renderable.hpp>
#include <vector>
#include <cassert>
#include <QGLWidget>


namespace ratman {
  class qgl_scene_view;
  class atmosphere;

  namespace detail {
    class decorated_terrain_view_update_thread;
  }
  
  /**
   * Association of a terrain tile with multiple
   * decoration layers and a camera
   */
  class decorated_terrain_view {
  protected:
    friend class detail::decorated_terrain_view_update_thread;
    
    mutable QMutex                        mutex_;
    detail::decorated_terrain_view_update_thread* update_thread_;
    float                                 camera_fovy_;
    int                                   viewport_width_;
    int                                   viewport_height_;
    rigid_body_map3d_t                    camera_V_;
    point3d_t                             camera_ground_target_;
    terrain_renderable*                   terrain_;
    active_renderable                     decorations_root_;
    
    QString                               elevation_copyright_;	// copyright for geometry data
    std::vector<QString>                  color_copyright_;	// copyrights for texture data

  public:

    decorated_terrain_view();
    virtual ~decorated_terrain_view();
    
    void set_terrain_layer(terrain_renderable* x);
    void insert_decoration_layer(active_renderable* x);
    const terrain_renderable* terrain_layer() const;
    terrain_renderable* terrain_layer();
    
    const active_renderable* decorations_root() const;
    active_renderable* decorations_root();

    void set_camera_fovy(float rad);
    void set_viewport(int width, int height);

    int viewport_width(){
      return viewport_width_;
    }

    int viewport_height(){
      return viewport_height_;
    }

    void set_cameraV(const rigid_body_map3d_t& V, 
		     const point3d_t& ground_target);
    rigid_body_map3d_t cameraV() const;
    projective_map3d_t cameraP() const;
    point3d_t camera_ground_target() const;

    void set_elevation_copyright(QString s) {
      elevation_copyright_=s;
    }

    const QString& elevation_copyright() const {
      return elevation_copyright_;
    }

    void add_color_copyright(QString s) {
      color_copyright_.push_back(s);
    }

    const QString& color_copyright(int i) const {
      assert(size_t(i) < color_copyright_.size());
      return color_copyright_[i];
    }


  public:
    
    virtual bool on_event(qgl_scene_view& qgl,
			  QEvent* e);

    virtual void render(qgl_scene_view& qgl);

  public:
    
    void async_update_start();
    void async_update_stop();

    void set_atmosphere(atmosphere* atm){
      atmosphere_ = atm;
    }

  protected:

    void async_update();
    
    atmosphere* atmosphere_;
  };
}

#endif
