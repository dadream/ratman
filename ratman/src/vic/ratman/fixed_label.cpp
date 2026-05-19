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
#include <vic/ratman/fixed_label.hpp>
#include <vic/ratman/qgl_scene_view.hpp>

namespace ratman {

  fixed_label::fixed_label(decorated_terrain_view* scene,
			   const std::string& name,
			   const double label_size,
			   const vector4f_t& label_color,
			   const point2f_t& pos) :
    active_renderable(scene, name),
    label_size_(label_size),
    label_color_(label_color),
    pos_(pos) {
  }

  fixed_label::~fixed_label() {
  }


  void fixed_label::render_self(qgl_scene_view& /*qgl*/,
				occupancy_map_t& /*occupancy_map*/,
				const projective_map3d_t& /*P*/,
				const rigid_body_map3d_t& /*V*/,
				const point3d_t& /*C*/) {

    if(text_.isEmpty()) return;

    double default_font_scale =  shared_3dfont().scale_x(); 

    GLint xywh[4];
    glGetIntegerv(GL_VIEWPORT,xywh);

    mutex_.lock();
    glPushAttrib(GL_ENABLE_BIT);
    {
      glDisable(GL_LIGHTING);    
      glDisable(GL_DEPTH_TEST);
      
      glMatrixMode(GL_PROJECTION);
      glPushMatrix(); 
      {
	glLoadIdentity();
	glOrtho(0.0,xywh[2],0.0,xywh[3], -1.0, 1.0);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix(); 
	{
	  glLoadIdentity();

          shared_3dfont().begin();
          {
            glColor4f(label_color_[0], label_color_[1], label_color_[2], 1.0f);
            glTranslatef(0.5f*xywh[2], pos_[1], 0.0f); // Center x
            glScalef(label_size_/default_font_scale,label_size_/default_font_scale,1.0f);
            for(int i=0; i<text_.size(); ++i) {
	      float lo_x, lo_y, hi_x, hi_y;
	      shared_3dfont().string_bbox(text_.at(i).toLatin1().data(), lo_x, lo_y, hi_x, hi_y);
	      const float dx = hi_x-lo_x;
	      glTranslatef(-0.5*dx, 0.0f, 0.0f);
              shared_3dfont().write(text_.at(i).toLatin1().data());
              glTranslatef(0.5f*dx,-(1.2f*default_font_scale), 0.0f);
            }
          }
          shared_3dfont().end();
	} 
	glPopMatrix();
      } 
      glMatrixMode(GL_PROJECTION);
      glPopMatrix();

      glMatrixMode(GL_MODELVIEW);
    }
    glPopAttrib();
    mutex_.unlock();
  }

}
