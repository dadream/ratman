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
#include <vic/ratman/copyright.hpp>
#include <vic/ratman/decorated_terrain_view.hpp>
#include <vic/cbdam/base/terrain_model.hpp>

namespace ratman {
  copyright::copyright(decorated_terrain_view* scene, 
		       const std::string& name,
		       const double label_size,
		       const vector4f_t& label_color,
		       const point2f_t& pos) :
    active_renderable(scene, name), label_(scene, name, label_size, label_color, pos) {
  }
    
  copyright::~copyright() {

  }
    
  /// Handle picking and return true if handled
  bool  copyright::on_event_self(qgl_scene_view& qgl,
				 QEvent* e,
				 const projective_map3d_t& P,
				 const rigid_body_map3d_t& V) {
    return false;
  }

  /// Render the hierarchy object rooted at this
  void  copyright::render_self(qgl_scene_view& qgl,
			       occupancy_map_t& occupancy_map,
			       const projective_map3d_t& P,
			       const rigid_body_map3d_t& V,
			       const point3d_t& C) {
    scene_->terrain_layer()->model()->lock_current_representation();
      std::vector<std::string> copyrights = scene_->terrain_layer()->model()->current_color_copyrights();
    scene_->terrain_layer()->model()->unlock_current_representation();

    label_.text().clear();
    for(std::size_t i = 0; i < copyrights.size(); ++i) {
      label_.text().push_back(copyrights[i].c_str());
    }
    label_.text().push_back(scene_->terrain_layer()->model()->current_elevation_copyright().c_str());
    label_.render_self(qgl, occupancy_map, P, V, C);
  }


}
