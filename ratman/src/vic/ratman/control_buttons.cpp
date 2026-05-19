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
#include <vic/ratman/control_buttons.hpp>
#include <vic/ratman/qgl_scene_view.hpp>
#include <vic/cbdam/base/terrain_model.hpp>
#include <vic/ratman/decorated_terrain_view.hpp>
#include <qfile.h>
#include <qtextstream.h>
#include <qstringlist.h>
#include <QMouseEvent>
#include "control_buttons.xpm"
#include "control_buttons_icon.xpm"

namespace ratman {

  control_buttons::control_buttons( decorated_terrain_view* scene, const std::string& name) :
    active_renderable(scene,name) { 
    gl_img_=QGLWidget::convertToGLFormat(QImage(control_buttons_xpm));
    icon_half_size_ = gl_img_.width()/2;
    icon_quarter_size_ = gl_img_.width()/4;
    icon_xc_ = scene->viewport_width();
    icon_yc_ = icon_quarter_size_ ;
    gl_texid_= 0;
    icon_= new QImage(control_buttons_icon_xpm);
    set_show(false);
  }

  control_buttons::~control_buttons() {
  }
  

  bool control_buttons::on_event_self(qgl_scene_view& qgl,
			      QEvent* e,
			      const projective_map3d_t& /*P*/,
			      const rigid_body_map3d_t& /*V*/) {

    if (e->type() == QEvent::MouseButtonPress ||  e->type() == QEvent::MouseButtonDblClick) {
      QMouseEvent *me = dynamic_cast<QMouseEvent *>(e);      
      int x = me->x();
      int y = qgl.height()-1-me->y();

      if (x >= icon_xc()-icon_quarter_size() && x <= icon_xc()+icon_quarter_size() &&
	  y >= icon_yc()-icon_quarter_size() && y <= icon_yc()+icon_quarter_size()) {
	qgl.backward_pressed();
	return true;
      }

      if (x >= icon_xc()-icon_half_size()+icon_quarter_size() && x <= icon_xc()+icon_half_size()-icon_quarter_size() &&
	  y >= icon_yc()+icon_quarter_size() && y <= icon_yc()+icon_half_size()) {
	qgl.forward_pressed();
	return true;
      }
      if (y >= icon_yc()-icon_half_size()+icon_quarter_size() && y <= icon_yc()+icon_half_size()-icon_quarter_size() &&
	  x >= icon_xc()+icon_quarter_size() && x <= icon_xc()+icon_half_size()) {
	qgl.right_pressed();
	return true;
      }
      if (y >= icon_yc()-icon_half_size()+icon_quarter_size() && y <= icon_yc()+icon_half_size()-icon_quarter_size() &&
	  x >= icon_xc()-icon_half_size() && x <= icon_xc()-icon_quarter_size()) {
	qgl.left_pressed();
	return true;
      }
      if (y >= icon_yc()+icon_quarter_size() && y <= icon_yc()+icon_half_size() &&
	  x >= icon_xc()-icon_half_size() && x <= icon_xc()-icon_quarter_size()) {
	qgl.yaw_ccw_pressed();
	return true;
      }
      if (y >= icon_yc()+icon_quarter_size() && y <= icon_yc()+icon_half_size() &&
	  x >= icon_xc()+icon_quarter_size() && x <= icon_xc()+icon_half_size()) {
	//std::cerr << "control panel CW " << std::endl;
	qgl.yaw_cw_pressed();
	return true;
      }
    }
    if (e->type() == QEvent::MouseButtonRelease) {
      QMouseEvent *me = dynamic_cast<QMouseEvent *>(e);      
      int x = me->x();
      int y = qgl.height()-1-me->y();
      
      if (y >= icon_yc()-icon_half_size() && y <= icon_yc()+icon_half_size() &&
	  x >= icon_xc()-icon_half_size() && x <= icon_xc()+icon_half_size()) {
      qgl.left_released();
      qgl.right_released();
      qgl.forward_released();
      qgl.backward_released();
      qgl.yaw_ccw_released();
      qgl.yaw_cw_released();
      }
    }
    return false;
  }


  void control_buttons::render_self(qgl_scene_view& qgl,
                            occupancy_map_t& /*occupancy_map*/,
			    const projective_map3d_t& /*P*/,
				    const rigid_body_map3d_t& /*V*/,
			    const point3d_t& /*C*/) {

    mutex_.lock();
    glPushAttrib(GL_ENABLE_BIT);
    {
      GLint xywh[4];
      glGetIntegerv(GL_VIEWPORT,xywh);

      glDisable(GL_LIGHTING);    
      glDisable(GL_DEPTH_TEST);
      
      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

      glEnable(GL_TEXTURE_2D);
      glMatrixMode(GL_PROJECTION);
      glPushMatrix(); 
      {
	glLoadIdentity();
	glOrtho(0.0,xywh[2],0.0,xywh[3], -1.0, 1.0);

	glMatrixMode(GL_MODELVIEW);
	if (show_){

	glPushMatrix(); 
	{
	  glLoadIdentity();
	  icon_xc_ = qgl.width()- icon_half_size();
	  glTranslatef(float(icon_xc()), float(icon_yc()), 0.0f);
	  {
	    if (gl_texid_ == 0) {
	      glGenTextures(1, &gl_texid_);
	      glBindTexture(GL_TEXTURE_2D, gl_texid_);
	      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	      glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	      glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	      glTexImage2D(GL_TEXTURE_2D, 
			   0, 
			   (gl_img_.hasAlphaChannel() ? 4 : 3), // FIXME 
			   gl_img_.width(), 
			   gl_img_.height(), 0,
			   GL_RGBA, 
			   GL_UNSIGNED_BYTE, 
			   gl_img_.bits());
	    } else {
	      glBindTexture(GL_TEXTURE_2D, gl_texid_);
	    }
	    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	    glColor4f(1.0, 1.0, 1.0, 1.0);    
	    glBegin(GL_QUADS); {
	      float ihs=float(icon_half_size());
	      glTexCoord2f(0.0f, 0.0f); glVertex2f(-ihs, -ihs);
	      glTexCoord2f(1.0f, 0.0f); glVertex2f( ihs, -ihs);
	      glTexCoord2f(1.0f, 1.0f); glVertex2f( ihs,  ihs);
	      glTexCoord2f(0.0f, 1.0f); glVertex2f(-ihs,  ihs);
	    } glEnd();
	    glBindTexture(GL_TEXTURE_2D, 0);	 
	  } 
	}
	glPopMatrix();
	}
      } 
      glMatrixMode(GL_PROJECTION);
      glPopMatrix();

      glMatrixMode(GL_MODELVIEW);
    }
    glPopAttrib();
    mutex_.unlock();
  }

}
