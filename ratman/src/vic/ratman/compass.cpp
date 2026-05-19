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
#include <vic/ratman/compass.hpp>
#include <vic/ratman/qgl_scene_view.hpp>
#include <vic/cbdam/base/terrain_model.hpp>
#include <qfile.h>
#include <qtextstream.h>
#include <qstringlist.h>
#include <QMouseEvent>
#include "compass.xpm"
#include "compass_icon.xpm"

namespace ratman {

  compass::compass(decorated_terrain_view* scene, const std::string& name) :
    active_renderable(scene,name) { 
    gl_img_=QGLWidget::convertToGLFormat(QImage(compass_xpm));
    icon_half_size_ = gl_img_.width()/2;
    icon_xc_ = icon_yc_ = icon_half_size_ + 10;
    gl_texid_= 0;
    icon_= new QImage(compass_icon_xpm);
  }

  compass::~compass() {
  }
  

  bool compass::on_event_self(qgl_scene_view& qgl,
			      QEvent* e,
			      const projective_map3d_t& /*P*/,
			      const rigid_body_map3d_t& /*V*/) {

    if (e->type() == QEvent::MouseButtonDblClick) {
      QMouseEvent *me = dynamic_cast<QMouseEvent *>(e);      
      int x = me->x();
      int y = qgl.height()-1-me->y();
      if (x >= icon_xc()-icon_half_size() && x <= icon_xc()+icon_half_size() &&
	  y >= icon_yc()-icon_half_size() && y <= icon_yc()+icon_half_size()) {
	qgl.reset_north_pressed();
	return true;
      }
    }
    return false;
  }

  void compass::render_self(qgl_scene_view& qgl,
                            occupancy_map_t& /*occupancy_map*/,
			    const projective_map3d_t& /*P*/,
			    const rigid_body_map3d_t& V,
			    const point3d_t& /*C*/) {
    double       north_angle   = qgl.north_angle(V);

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
	glPushMatrix(); 
	{
	  glLoadIdentity();
	  glTranslatef(float(icon_xc()), float(icon_yc()), 0.0f);
	  glRotatef(-90.0f-north_angle/3.14159265*180.0, 0.0, 0.0, 1.0);
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
      glMatrixMode(GL_PROJECTION);
      glPopMatrix();

      glMatrixMode(GL_MODELVIEW);
    }
    glPopAttrib();
    mutex_.unlock();
  }

}
