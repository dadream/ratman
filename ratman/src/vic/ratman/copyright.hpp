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
#ifndef RATMAN_COPYRIGHT_HPP
#define RATMAN_COPYRIGHT_HPP

#include <vic/ratman/ratman.hpp>
#include <vic/ratman/active_renderable.hpp>
#include <vic/ratman/fixed_label.hpp>
#include <GL/glew.h>
#include <qimage.h>

namespace ratman {
  
  class copyright: public active_renderable {
  protected:
    fixed_label label_;

  public:

    copyright(decorated_terrain_view* scene, const std::string& name,
	      const double label_size,
	      const vector4f_t& label_color,
	      const point2f_t& pos);
    
    virtual ~copyright();
    
  public:
    
    /// Handle picking and return true if handled
    virtual bool on_event_self(qgl_scene_view& qgl,
			       QEvent* e,
			       const projective_map3d_t& P,
			       const rigid_body_map3d_t& V);

    /// Render the hierarchy object rooted at this
    virtual void render_self(qgl_scene_view& qgl,
                             occupancy_map_t& occupancy_map,
			     const projective_map3d_t& P,
                             const rigid_body_map3d_t& V,
			     const point3d_t& C);


  };
  
}

#endif
