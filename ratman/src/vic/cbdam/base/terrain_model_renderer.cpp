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
#ifdef _WIN32
#include <windows.h>
#undef min
#undef max
#endif

#include <vic/cbdam/base/terrain_model_renderer.hpp>
#include <vic/cbdam/base/progress_bar.hpp>
#include <sl/clock.hpp>
#include <GL/glew.h>

#ifdef _WIN32
#undef min
#undef max
#endif

namespace cbdam {

  terrain_model_renderer::terrain_model_renderer(terrain_model* tm) :
    terrain_model_(tm) {
    focus_fraction_ = 1.0;
    screen_tolerance_ = 0.1;
    wireframe_enabled_ = false;
    draw_bounding_volumes_enabled_ = false;
    is_opengl_supported_ = false;
    adaptive_tolerance_enabled_ = true;
    current_center_ = point3d_t(0.0, 0.0, 0.0);
  }

  terrain_model_renderer::~terrain_model_renderer() {
    
  }

  //opengl_cached_data_renderer::rendering_mode_t terrain_model_renderer::gl_rendering_mode() const {
  //  return opengl_cached_data_renderer_.gl_rendering_mode();
  //}

  
  void terrain_model_renderer::init_opengl() {
    opengl_cached_data_renderer_.set_projection_parameters(terrain_model_->uvh_xyz_transform());
    opengl_cached_data_renderer_.set_vbo_parameters(terrain_model_->height_patch_dim(),
                                                    terrain_model_->texture_quad_width());
    opengl_cached_data_renderer_.init_opengl();
    is_opengl_supported_ = (opengl_cached_data_renderer_.gl_rendering_mode() != 
			    opengl_cached_data_renderer::RENDERING_MODE_NONE);

    std::pair<float, float> h_range = terrain_model_->estimated_elevation_range();
    double height_scale = terrain_model_->height_scale_factor();
    if (height_scale == 0) height_scale = 1.0;
    opengl_cached_data_renderer_.set_elevation_range(0, h_range.second / height_scale);
  }

  void terrain_model_renderer::draw(const projective_map_t& P, 
				    const rigid_body_map_t& V,
				    const point3d_t& draw_origin) {
    sl::real_time_clock stat_clock_full;

    sl::real_time_clock stat_clock;
    double stat_lock_time;
    double stat_visibility_time;
    double stat_draw_time;
    double stat_full_time;

    stat_clock_full.restart();

    if (!is_opengl_supported()) {
      std::cerr << "opengl extensions not supported" << std::endl;
      return;
    }

    // Record position
    current_projection_ = P;
    current_view_ = V;
    current_center_ = draw_origin;

    // set threshold, P, V in the terrain_model
    float fovy = current_projection_.fov_from_std_3d_perspective();
    float threshold = screen_tolerance_ * 2 * tan(fovy/2);
    if (adaptive_tolerance_enabled_) {
      const float angle = angle_with_up_vector();
      const float k = 2.5f / (5.0f * 1.571f); // remap 0..1.57 in 0..1/2 //3/5
      threshold *= (1.0f + angle * k);
    }
    terrain_model_->set_refine_parameters(threshold, current_projection_, current_view_, focus_fraction_);
    
    stat_clock.restart();
    terrain_model_->lock_current_representation();
    {
      stat_lock_time = stat_clock.elapsed().as_milliseconds();

      const diamond_data_map_t& cut = terrain_model_->current_representation();

      stat_clock.restart();
      std::vector<bool> is_cut_diamond_visible;
      recompute_visibility(cut, is_cut_diamond_visible);
      stat_visibility_time = stat_clock.elapsed().as_milliseconds();

      stat_clock.restart();
      draw_begin();
      draw_diamonds(cut, is_cut_diamond_visible);
      draw_diamonds_wireframe(cut, is_cut_diamond_visible);
      draw_overlays(cut, is_cut_diamond_visible);
      draw_end();
      stat_draw_time = stat_clock.elapsed().as_milliseconds();
    }
    terrain_model_->unlock_current_representation();

    stat_full_time = stat_clock_full.elapsed().as_milliseconds();
    if (stat_full_time > 20) {
      SL_TRACE_OUT(1) << "FRAME: " << 
	" full: " << stat_full_time <<
	" full2: " << stat_lock_time + stat_visibility_time + stat_draw_time << 
	" lock: " << stat_lock_time <<
	" vis: " << stat_visibility_time <<
	" draw: " << stat_draw_time <<
	std::endl;
    }
  }

  void terrain_model_renderer::recompute_visibility(const diamond_data_map_t& cut,
						    std::vector<bool>& is_cut_diamond_visible) const {
    is_cut_diamond_visible.resize(cut.size());
    int count = 0;
    int visible_count = 0;
    projective_map_t camera_pv = current_projection_ * current_view_;
    for(diamond_data_map_t::const_iterator it = cut.begin();
	it != cut.end();
        ++it) {
      if (it->second->visible()) {
	is_cut_diamond_visible[count] = true;
      } else {
        is_cut_diamond_visible[count] = terrain_model_->is_visible(it->second->diamond_state().bounding_volume(), camera_pv);
      }
      if (is_cut_diamond_visible[count]) ++visible_count;
      ++count;
    }
  }

  void terrain_model_renderer::draw_begin() {
    // gl&co
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glEnable(GL_DEPTH_TEST);
    glShadeModel(GL_SMOOTH);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glEnable(GL_CULL_FACE);
    glFrontFace(GL_CW);
    
    // FIXME
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE); 

    if (wireframe_enabled_) {
      glEnable(GL_POLYGON_OFFSET_FILL);
      glPolygonOffset(1.0f, 4.0f);
    } else {
      glColor3f( 1.0, 1.0, 1.0 );
      glDisable(GL_POLYGON_OFFSET_FILL);
    }

    bool elevation_map_enabled = opengl_cached_data_renderer_.elevation_map_enabled();

    const point4_t surface_color = 
      (elevation_map_enabled || opengl_cached_data_renderer_.use_color()) ? point4_t(1.0f,1.0f,1.0f, 1.0f) : point4_t(0.87f, 0.73f, 0.58f, 1.0f);

    opengl_cached_data_renderer_.frame_initialize(surface_color);
  }

  void terrain_model_renderer::draw_end() {
    //    opengl_cached_data_renderer_.frame_finalize(); // MOVED BEFORE draw_overlays
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glPopAttrib();
 }

  void terrain_model_renderer::draw_diamonds(const diamond_data_map_t& cut,
                                             const std::vector<bool>& is_cut_diamond_visible) {
    uint32_t count = 0;
    grid_point_t previous_texture_level_xy(-1,-1,-1);
    bool use_color = opengl_cached_data_renderer_.use_color();
    vector3d_t previous_offset(1e+30,-1e+29,1e+28); // something strange for initialization
    //    std::cerr << "------------------------- NEW FRAME ----------------------------------------" << std::endl;
    glMatrixMode(GL_TEXTURE);
    glPushMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();

    sl::matrix4f VM;
    glGetFloatv(GL_MODELVIEW_MATRIX, VM.to_pointer());

    for(diamond_data_map_t::const_iterator it = cut.begin(); it != cut.end(); ++it) {
      if (is_cut_diamond_visible[count]) {
	if (use_color) {
	  grid_point_t current_texture_level_xy = it->second->texture_level_xy();
	  //	  std::cerr << "current_texture_level_xy " << current_texture_level_xy << std::endl;
	  if (current_texture_level_xy != previous_texture_level_xy &&
	      it->second->texture_image() != 0) {
	    opengl_cached_data_renderer_.bind_texture(it->second);
	    previous_texture_level_xy = current_texture_level_xy;
	  }
	  glMatrixMode(GL_TEXTURE);
	  glLoadMatrixd(it->second->texture_matrix().to_pointer());
	  glMatrixMode(GL_MODELVIEW);
	}
	
	// local draw
	const vector3d_t diamond_offset = it->second->vertices()->diamond_center() - current_center_;
	if (previous_offset != diamond_offset) {
	  previous_offset = diamond_offset;
	  sl::matrix4f VM_prime = VM;
	  VM_prime *= sl::matrix4f(1.0f, 0.0f, 0.0f, diamond_offset[0],
				   0.0f, 1.0f, 0.0f, diamond_offset[1],
				   0.0f, 0.0f, 1.0f, diamond_offset[2],
				   0.0f, 0.0f, 0.0f, 1.0f);
 
	  glLoadMatrixf(VM_prime.to_pointer());
	}
        opengl_cached_data_renderer_.render(it->first, it->second);
      }
      ++count;
    }

    glPopMatrix();
    glMatrixMode(GL_TEXTURE);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
  }

  void terrain_model_renderer::draw_diamonds_wireframe(const diamond_data_map_t& cut,
                                                       const std::vector<bool>& is_cut_diamond_visible) {
    if (wireframe_enabled_) {
      bool elevation_map_enabled = opengl_cached_data_renderer_.elevation_map_enabled();
      if (elevation_map_enabled) opengl_cached_data_renderer_.set_elevation_map_enabled(false);

      // set wireframe color
      glDisable(GL_POLYGON_OFFSET_FILL); 
      glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
      
      static const point4_t line_color(0.0f, 0.0f, 0.0f, 1.0f);
      opengl_cached_data_renderer_.frame_initialize(line_color);

      bool patch_color_enabled = opengl_cached_data_renderer_.patch_color_enabled();
      if (patch_color_enabled) opengl_cached_data_renderer_.patch_color_enabled() = false;

      draw_diamonds(cut, is_cut_diamond_visible);
      if (patch_color_enabled) opengl_cached_data_renderer_.patch_color_enabled() = true;     
      if (elevation_map_enabled) opengl_cached_data_renderer_.set_elevation_map_enabled(true); 
    }
  }

  void terrain_model_renderer::draw_overlays(const diamond_data_map_t& cut,
                                             const std::vector<bool>& is_cut_diamond_visible) {
    // disbale all opengl texture/vbos settings
    opengl_cached_data_renderer_.frame_finalize();

    if (draw_bounding_volumes_enabled_) {
      glColor3f( 1.0, 1.0, 1.0 );
      draw_bounding_volumes(cut, is_cut_diamond_visible);
    }

    float dmf = 1.0f - terrain_model_->data_missing_fraction();
    if (dmf<0) dmf = 0;
    if (dmf < 0.9999) {
      const bool is_vertical = false;
      const bool look3d = true;
      //      progress_bar pb(0.83, 0.96, 0.16, is_vertical, look3d);
      progress_bar pb(is_vertical, look3d); // fixed length and positioning.
      pb.draw(dmf);
    }
  }

  void terrain_model_renderer::draw_bounding_volumes(const diamond_data_map_t& cut,
                                                     const std::vector<bool>& is_cut_diamond_visible) const {
    uint32_t count = 0;
    for(diamond_data_map_t::const_iterator it = cut.begin();
        it != cut.end(); ++it) {
      if (is_cut_diamond_visible[count]) {
        draw_bounding_volume(it->second->diamond_state().bounding_volume());
      }
      ++count;
    }
  }
  
  void terrain_model_renderer::draw_bounding_volume(const bounding_volume_t& bvol) const {
    const point3d_t p000 = bvol.box_space_corner(0);
    const point3d_t p111 = bvol.box_space_corner(7);
    glPushMatrix();

    glTranslated(-current_center_[0], -current_center_[1], -current_center_[2]);
    glMultMatrixd(bvol.from_box_space_map().as_matrix().to_pointer());
    {
      glColor3f(1.0f, 0.0f, 0.0f);
      glLineWidth(2);
      glBegin(GL_LINES);
      glVertex3f(0.5f*(p000[0]+p111[0]), 0.5f*(p000[1]+p111[1]), p000[2]);
      glVertex3f(0.5f*(p000[0]+p111[0]), 0.5f*(p000[1]+p111[1]), p111[2]);
      glEnd();

      glColor3f(1.0f, 1.0f, 1.0f);
      glLineWidth(1);
      glBegin(GL_LINE_LOOP);
      glNormal3f( 0,0,-1 );
      glVertex3f(p111[0], p000[1], p000[2]);
      glVertex3f(p000[0], p000[1], p000[2]);
      glVertex3f(p000[0], p111[1], p000[2]);
      glVertex3f(p111[0], p111[1], p000[2]);
      glEnd();

      glBegin(GL_LINE_LOOP);
      glNormal3f( 0,1,0 );
      glVertex3f(p111[0], p111[1], p000[2]);
      glVertex3f(p000[0], p111[1], p000[2]);
      glVertex3f(p000[0], p111[1], p111[2]);
      glVertex3f(p111[0], p111[1], p111[2]);
      glEnd();

      glBegin(GL_LINE_LOOP);
      glNormal3f( 0,0,1 );
      glVertex3f(p111[0], p111[1], p111[2]);
      glVertex3f(p000[0], p111[1], p111[2]);
      glVertex3f(p000[0], p000[1], p111[2]);
      glVertex3f(p111[0], p000[1], p111[2]);
      glEnd();

      glBegin(GL_LINE_LOOP);
      glNormal3f( 0,-1,0 );
      glVertex3f(p111[0], p000[1], p111[2]);
      glVertex3f(p000[0], p000[1], p111[2]);
      glVertex3f(p000[0], p000[1], p000[2]);
      glVertex3f(p111[0], p000[1], p000[2]);
      glEnd();

      glBegin(GL_LINE_LOOP);
      glNormal3f( -1,0,0 );
      glVertex3f(p000[0], p000[1], p000[2]);
      glVertex3f(p000[0], p000[1], p111[2]);
      glVertex3f(p000[0], p111[1], p111[2]);
      glVertex3f(p000[0], p111[1], p000[2]);
      glEnd();

      glBegin(GL_LINE_LOOP);
      glNormal3f( -1,0,0 );
      glVertex3f(p111[0], p000[1], p000[2]);
      glVertex3f(p111[0], p111[1], p000[2]);
      glVertex3f(p111[0], p111[1], p111[2]);
      glVertex3f(p111[0], p000[1], p111[2]);
      glEnd();
    }
    glPopMatrix();
  }
  
  void terrain_model_renderer::set_wireframe_enabled(bool x) {
    wireframe_enabled_ = x;
  }

  bool terrain_model_renderer::is_wireframe_enabled() const {
    return wireframe_enabled_;
  }

  void terrain_model_renderer::set_draw_enabled(bool x) {
    opengl_cached_data_renderer_.draw_enabled() = x;
  }

  bool terrain_model_renderer::is_draw_enabled() const {
    return opengl_cached_data_renderer_.draw_enabled();
  }

  void terrain_model_renderer::set_focus_fraction(float x) {
    focus_fraction_ = x;
  }

  void terrain_model_renderer::set_screen_tolerance(float x) {
    screen_tolerance_ = x;
  }
 
  void terrain_model_renderer::set_terrain_model(terrain_model* tm) {
    terrain_model_ = tm;
  }

  void terrain_model_renderer::set_patch_color_enabled(bool x) {
    opengl_cached_data_renderer_.patch_color_enabled() = x;
  }

  bool terrain_model_renderer::is_patch_color_enabled() const {
    return opengl_cached_data_renderer_.patch_color_enabled();
  }

  void terrain_model_renderer::set_elevation_map_enabled(bool x) {
    opengl_cached_data_renderer_.set_elevation_map_enabled(x);
  }

  void terrain_model_renderer::set_color_enabled(bool x) {
    opengl_cached_data_renderer_.set_use_color(x);
  }

  bool terrain_model_renderer::is_color_enabled() const {
    return opengl_cached_data_renderer_.use_color();
  }

  void terrain_model_renderer::set_shading_enabled(bool x) {
    opengl_cached_data_renderer_.set_use_normal(x);
  }

  bool terrain_model_renderer::is_shading_enabled() const {
    return opengl_cached_data_renderer_.use_normal();
  }

  bool terrain_model_renderer::is_elevation_map_enabled() const {
    return opengl_cached_data_renderer_.elevation_map_enabled();
  }

  void terrain_model_renderer::set_draw_bounding_volumes_enabled(bool x) {
    draw_bounding_volumes_enabled_ = x;
  }

  bool terrain_model_renderer::is_draw_bounding_volumes_enabled() const {
    return draw_bounding_volumes_enabled_;
  }

  void terrain_model_renderer::set_geometry_cache_capacity(uint32_t x) {
    opengl_cached_data_renderer_.vbo_cache_capacity() = x;
  }

  void terrain_model_renderer::set_texture_cache_capacity(uint32_t x_bytes) {
    opengl_cached_data_renderer_.set_texture_cache_capacity(x_bytes);
  }

  void terrain_model_renderer::release_graphics() {
    opengl_cached_data_renderer_.clear();
  } 

  void terrain_model_renderer::set_adaptive_tolerance_enabled(bool x) {
    adaptive_tolerance_enabled_ = x;
  }
  
  bool terrain_model_renderer::is_adaptive_tolerance_enabled() const {
    return adaptive_tolerance_enabled_;
  }

  float terrain_model_renderer::angle_with_up_vector() const {
    sl::point3d eye = (current_projection_ * current_view_).eye();
    sl::vector3d up = terrain_model_->uvh_xyz_transform()->up_from_xyz(eye);
    sl::vector3d tx_up = current_view_ * up;
    return tx_up.angle(up);
  }

  void terrain_model_renderer::clear_texture_cache() {
    opengl_cached_data_renderer_.clear_texture_cache();
  }

} // namespace cbdam 


