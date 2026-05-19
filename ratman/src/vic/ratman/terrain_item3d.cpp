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
#include <vic/ratman/terrain_item3d.hpp>
#include <vic/ratman/qgl_scene_view.hpp>
#include <vic/cbdam/base/terrain_model.hpp>
#include <vic/cbdam/base/terrain_model_renderer.hpp>

#include <GL/glew.h>
#include <QApplication>
#include <QMessageBox>
#include <cassert>

namespace ratman {
  
    terrain_item3d::terrain_item3d(decorated_terrain_view* scene, const std::string& id, const std::string& url_file) :
	active_renderable(scene, id) {
    url_file_ = url_file;
  }
      
  terrain_item3d::~terrain_item3d() {
  }

  void terrain_item3d::set_active(bool x) {
    active_renderable::set_active(x);
    for (std::vector<this_t*>::iterator it=children_.begin();
         it!= children_.end();
         ++it) {
        (*it)->set_active(x);
    }
  }

  bool terrain_item3d::on_select_self(const projective_map3d_t& /*P*/,
				      const rigid_body_map3d_t& /*V*/,
				      const point2d_t& /*xy*/) {
    bool result;
    //   mutex_.lock();
    {
      result = false;
    }
    // mutex_.unlock();
    return result;
  }
    
  void terrain_item3d::render_self(qgl_scene_view& /*qgl*/,
				   occupancy_map_t& /*occupancy_map*/,
				   const projective_map3d_t& /*P*/,
				   const rigid_body_map3d_t& /*V*/,
				   const point3d_t& /*C*/) {
    // Nothing to do
  }
            
  void terrain_item3d::async_update_self(const projective_map3d_t& /*P*/,
					     const rigid_body_map3d_t& /*V*/,
					     const point3d_t& /*G*/) {
    // Nothing to do
  }
 
}
