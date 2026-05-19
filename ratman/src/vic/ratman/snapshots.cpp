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
#include <vic/ratman/snapshots.hpp>
#include <vic/ratman/qgl_scene_view.hpp>
#include <qfile.h>
#include <qtextstream.h>
#include <qstringlist.h>
#include <QMouseEvent>
#include "film_tile.xpm"

namespace ratman {
  static const int SIZE_STEP=50;
  static const int OFFSET_Y=10;
  static const int OFFSET_X=128;


  snapshots_slot::snapshots_slot(const oriented_position &op,
				 sl::tuple2i &t,int width,int height) :
    oriented_pos_(op)
  {
    set_position(t);
    set_w(width);
    set_h(height);
    init_gl_env();
  }
  snapshots_slot::~snapshots_slot(){   
    glDeleteTextures(1, &texid_);
  }

  void snapshots_slot::init_gl_env(){
    glGenTextures(1, &texid_);
    glBindTexture(GL_TEXTURE_2D, texid_);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE); 
  }
  
  void snapshots_slot::draw_snapshot(const float alpha,const float tex_mapping){
    glPushMatrix(); 
    {
      glLoadIdentity();
      glTranslatef((float(position()[0])), float(position()[1]), 0.0f);
      glScalef(float(w()), float(h()), 1.0f);
      glBindTexture(GL_TEXTURE_2D, texid_);
      glColor4f(3.0, 3.0, 3.0, alpha); // FIXME HACK    
      glBegin(GL_QUADS); {
	glTexCoord2f(0.0f, 0.0f); glVertex2f(-0.5f, -0.5f);
	glTexCoord2f(tex_mapping, 0.0f); glVertex2f( 0.5f, -0.5f);
	glTexCoord2f(tex_mapping, 1.0f); glVertex2f( 0.5f,  0.5f);
	glTexCoord2f(0.0f, 1.0f); glVertex2f(-0.5f,  0.5f);
      } glEnd();
      glBindTexture(GL_TEXTURE_2D, 0);	 
    }
    glPopMatrix();
  }

  //snapshots class
  snapshots::snapshots(decorated_terrain_view* scene, const std::string& name) :
    active_renderable(scene, name) {
    first_frame_=true;
    set_show_tape(false);
    current_slot_iterator =  slots_list.begin();
  }

  snapshots::~snapshots() {
  }
  
    
  bool snapshots::on_event_self(qgl_scene_view& qgl,
				QEvent* e,
				const projective_map3d_t& /*P*/,
				const rigid_body_map3d_t& /*V*/) {
    if (e->type() == QEvent::MouseButtonDblClick) {
      QMouseEvent *me = dynamic_cast<QMouseEvent *>(e);      
      int x = me->x();
      int y = qgl.height()-1-me->y();
      std::list<snapshots_slot*>::iterator p =  slots_list.begin();
      while (p != slots_list.end()){
	snapshots_slot*  temp_slot = *p;
	if (x >= temp_slot->position()[0]-(temp_slot->w()/2) && x <= temp_slot->position()[0]+(temp_slot->w()/2) &&
	    y >= temp_slot->position()[1]-(temp_slot->h()/2) && y <= temp_slot->position()[1]+(temp_slot->h()/2)) {
	  //std::cerr << "*******going to   " <<temp_slot->geo_pos()<< std::endl;

	  oriented_position start_position = qgl.camera_controller().get_oriented_position();
	  oriented_position end_position = temp_slot->oriented_pos();
	  qgl.fly_from_to_oriented_location(start_position,end_position);

	  return true;
	}
	p++;
      }
    }
    if (e->type() == QEvent::KeyPress) {
      QKeyEvent *ke = dynamic_cast<QKeyEvent *>(e);  
      if (ke->key() == Qt::Key_P){
	insert_snapshot(qgl);
	return true;
      } 
    }
    return false;
  }
  
  void snapshots::render_self(qgl_scene_view& qgl,
			      occupancy_map_t& /*occupancy_map*/,
			      const projective_map3d_t& /*P*/,
			      const rigid_body_map3d_t& V,
			      const point3d_t& /*C*/) {

    if (qgl.camera_controller().get_oriented_position() == last_oriented_position_) {
      if (clock_.elapsed().as_milliseconds() >= 6000) {
	if (last_oriented_position_ != last_snapped_position_) {
	  insert_snapshot(qgl);
	}
      }
    } else {
      last_oriented_position_ = qgl.camera_controller().get_oriented_position();
      clock_.restart();
    }
    
    if (first_frame_) {
      build_slots_list(qgl);
      first_frame_=false;
    }
    on_resize_window(qgl); 
    
    GLint xywh[4];
    glGetIntegerv(GL_VIEWPORT,xywh);
    matrix4x4d_t IV = V.inverse().as_matrix();
    mutex_.lock();
    glPushAttrib(GL_ENABLE_BIT);
    {
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

	if (show_tape()){
	  slots_background_->draw_snapshot(0.9,8.0);
	  std::list<snapshots_slot*>::iterator p =  slots_list.begin();
	  while (p != slots_list.end()){
	    snapshots_slot*  slots_it = *p;
	    slots_it->draw_snapshot(1.0,1.0);
	    p++;
	  }
	}

      } 
      glMatrixMode(GL_PROJECTION);
      glPopMatrix();
      glMatrixMode(GL_MODELVIEW);
    }
    glPopAttrib();
    mutex_.unlock();
  }
  

  void snapshots::build_slots_list(qgl_scene_view& qgl){
    int window_width = qgl.width();
    int window_height = qgl.height();

    slots_list.clear();
    int strip_height = int(SIZE_STEP*1.2);
    sl::tuple2i bg_pos;
    bg_pos[0] = window_width/2+OFFSET_X;
    bg_pos[1] = window_height-SIZE_STEP/2-OFFSET_Y;

    QImage background_image = QGLWidget::convertToGLFormat(QImage(film_tile_xpm));
    slots_background_ = new snapshots_slot(qgl.camera_controller().get_oriented_position(),
					   bg_pos,window_width,strip_height);
    slots_background_->set_image(background_image);
    old_window_size_ = QSize(window_width,window_height); 
  }
  
  void snapshots::insert_snapshot(qgl_scene_view& qgl){
    QImage snap_shot_full = qgl.grabFrameBuffer(false);
    sl::tuple2i start_pos;
    start_pos[0]=SIZE_STEP/2+OFFSET_X;
    start_pos[1]=qgl.height()-SIZE_STEP/2-OFFSET_Y;
    int icon_size = 32; // int(SIZE_STEP*0.85);
    QImage snap_shot_scaled = snap_shot_full.scaledToHeight(icon_size,Qt::SmoothTransformation);
    int img_cx = int ((snap_shot_scaled.width()-icon_size)/2);
    QImage snap_shot = snap_shot_scaled.copy(img_cx,0,icon_size,icon_size);
      
    snapshots_slot*  n_slot = new snapshots_slot(qgl.camera_controller().get_oriented_position(),
						 start_pos,snap_shot.width(),snap_shot.height());
    n_slot->set_image(QGLWidget::convertToGLFormat(snap_shot));
    n_slot->set_geo_pos(qgl.current_WGS84_lonlat_position());
    
    std::list<snapshots_slot*>::iterator p =  slots_list.begin();
    while (p != slots_list.end()){
      snapshots_slot*  slots_it = *p;
      sl::tuple2i new_pos;
      new_pos[0]=slots_it->position()[0]+SIZE_STEP;
      new_pos[1]=slots_it->position()[1];
      slots_it->set_position(new_pos);
      p++;
    }
    slots_list.push_back(n_slot);
    if (slots_list.front()->position()[0]>qgl.width()){
      slots_list.pop_front();
    }
    last_snapped_position_ = qgl.camera_controller().get_oriented_position(); 
    current_slot_iterator = slots_list.end();
  }

  void snapshots::on_resize_window(qgl_scene_view& qgl){
    if (old_window_size_ != QSize(qgl.size())){
      sl::tuple2i new_bg_pos;
      new_bg_pos[0]=qgl.width()/2+OFFSET_X;
      new_bg_pos[1]=qgl.height()-SIZE_STEP/2-OFFSET_Y;
      slots_background_->set_position(new_bg_pos);
      slots_background_->set_w(qgl.width());
      old_window_size_ = QSize(qgl.size());  

      std::list<snapshots_slot*>::iterator p =  slots_list.begin();
      while (p != slots_list.end()){
	snapshots_slot*  slots_it = *p;
	sl::tuple2i new_pos;
	new_pos[0]=slots_it->position()[0]+OFFSET_X;
	new_pos[1]=qgl.height()-SIZE_STEP/2-OFFSET_Y;
	slots_it->set_position(new_pos);
	p++;
      }
    }
  }

  snapshots_slot* snapshots::go_prev_slot(){
    if (current_slot_iterator != slots_list.begin()){
      current_slot_iterator--;
    }
    return *current_slot_iterator;
  }

}
