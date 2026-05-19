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
#ifndef RATMAN_SNAPSHOTS_HPP
#define RATMAN_SNAPSHOTS_HPP

#include <vic/ratman/ratman.hpp>
#include <vic/ratman/active_renderable.hpp>

#include <vic/ratman/camera_controller.hpp>
#include <vic/ratman/camera_animation.hpp>

#include <GL/glew.h>
#include <qimage.h>
#include <list>
#include <algorithm>

#include <sl/clock.hpp>


namespace ratman {

  /**
   * Snapshot in the history
   */
  class snapshots_slot {
    
  public:
    snapshots_slot(const oriented_position &op,
		   sl::tuple2i &t,int width,int height);
    ~snapshots_slot();
 
    void draw_snapshot(const float alpha,const float tex_mapping);
    void init_gl_env();
  
    inline sl::tuple2i position(){
      return position_;
    }
    inline void set_position(const sl::tuple2i &t){
      position_=t;
    }

    inline int w()  const {
      return slot_w_;
    }

    inline int h() const {
      return slot_h_;
    }

    inline void set_w(int w){
      slot_w_=w;
    }
    inline void set_h(int h){
      slot_h_=h;
    }

    inline void set_image(const QImage &img){
      glBindTexture(GL_TEXTURE_2D, texid_);
      glTexImage2D(GL_TEXTURE_2D, 
		   0, 
		   (img.hasAlphaChannel() ? 4 : 3), 
		   img.width(), 
		   img.height(), 0,
		   GL_RGBA, 
		   GL_UNSIGNED_BYTE, 
		   img.bits());
    }
    inline void set_geo_pos(const point3d_t &p){
      geo_pos_ = p;
    }
    inline point3d_t geo_pos() const {
      return geo_pos_;
    }
    inline void set_oriented_pos(const oriented_position &op){
      oriented_pos_=op;
    }
    inline oriented_position oriented_pos(){
      return oriented_pos_;
    }
 
  protected:
    sl::tuple2i position_;
    int slot_w_;
    int slot_h_;
    GLuint texid_;
    point3d_t  geo_pos_;
    oriented_position oriented_pos_;
  };
  
  /**
   * Snapshots history
   */
  class snapshots: public active_renderable {
 
  public:

    snapshots(decorated_terrain_view* scene, const std::string& name);
 
    virtual ~snapshots();
    
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


    void build_slots_list(qgl_scene_view& qgl);
    void insert_snapshot(qgl_scene_view& qgl);
    void on_resize_window(qgl_scene_view& qgl);
 
    snapshots_slot* go_prev_slot();

    inline bool show_tape(){
      return show_tape_;
    }

    inline void set_show_tape(bool b){
      show_tape_=b;
    }
  
  protected:
    std::list<snapshots_slot*>::iterator current_slot_iterator;
    std::list<snapshots_slot*> slots_list;
    bool first_frame_;
    oriented_position last_oriented_position_;
    oriented_position last_snapped_position_;
    sl::real_time_clock clock_;
    snapshots_slot* slots_background_;
    QSize  old_window_size_;
    bool show_tape_;
  };
  
}

#endif
