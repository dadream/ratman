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
#include <sl/clock.hpp>

#ifndef CBDAM_NO_OPENGL
#include <vic/cbdam/base/opengl_cached_data_renderer.hpp>
#endif

#ifdef _WIN32
#undef min
#undef max
#endif

namespace cbdam {

  terrain_model_renderer::terrain_model_renderer(terrain_model* tm) :
    cached_data_renderer_(0),
    owns_renderer_(false),
    terrain_model_(tm) {
#ifndef CBDAM_NO_OPENGL
    cached_data_renderer_ = new opengl_cached_data_renderer();
    owns_renderer_ = true;
#else
    cached_data_renderer_ = 0;
    owns_renderer_ = false;
#endif
    focus_fraction_ = 1.0;
    screen_tolerance_ = 0.1;
    wireframe_enabled_ = false;
    draw_bounding_volumes_enabled_ = false;
    is_opengl_supported_ = false;
    adaptive_tolerance_enabled_ = true;
    current_center_ = point3d_t(0.0, 0.0, 0.0);
  }

  terrain_model_renderer::terrain_model_renderer(terrain_model* tm, cached_data_renderer* r) :
    cached_data_renderer_(r), owns_renderer_(false), terrain_model_(tm) {
    focus_fraction_ = 1.0;
    screen_tolerance_ = 0.1;
    wireframe_enabled_ = false;
    draw_bounding_volumes_enabled_ = false;
    is_opengl_supported_ = false;
    adaptive_tolerance_enabled_ = true;
    current_center_ = point3d_t(0.0, 0.0, 0.0);
  }

  terrain_model_renderer::~terrain_model_renderer() {
    if (owns_renderer_) {
      delete cached_data_renderer_;
    }
  }

  //opengl_cached_data_renderer::rendering_mode_t terrain_model_renderer::gl_rendering_mode() const {
  //  return opengl_cached_data_renderer_.gl_rendering_mode();
  //}

  
  void terrain_model_renderer::init_opengl() {
    cached_data_renderer_->set_projection_parameters(terrain_model_->uvh_xyz_transform());
    cached_data_renderer_->set_vbo_parameters(terrain_model_->height_patch_dim(),
                                                    terrain_model_->texture_quad_width());
    cached_data_renderer_->init_opengl();
    
    is_opengl_supported_ = cached_data_renderer_ && cached_data_renderer_->is_supported();

    std::pair<float, float> h_range = terrain_model_->estimated_elevation_range();
    double height_scale = terrain_model_->height_scale_factor();
    if (height_scale == 0) height_scale = 1.0;
    cached_data_renderer_->set_height_scale_factor(height_scale);
    cached_data_renderer_->set_elevation_range(0, h_range.second / height_scale);
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
      cached_data_renderer_->set_matrices(P, V, draw_origin);
      cached_data_renderer_->set_rendering_pass(cached_data_renderer::PASS_FILL);
      draw_begin();
      draw_diamonds(cut, is_cut_diamond_visible);
      
      if (wireframe_enabled_) {
        cached_data_renderer_->set_rendering_pass(cached_data_renderer::PASS_WIREFRAME);
        draw_begin();
        draw_diamonds(cut, is_cut_diamond_visible);
      }
      
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
    cached_data_renderer_->draw_begin();
  }

  void terrain_model_renderer::draw_end() {
    cached_data_renderer_->draw_end();
  }

  void terrain_model_renderer::draw_diamonds(const diamond_data_map_t& cut,
                                             const std::vector<bool>& is_cut_diamond_visible) {
    uint32_t count = 0;
    bool use_color = cached_data_renderer_->use_color();
    grid_point_t previous_texture_level_xy(-1,-1,-1);

    for(diamond_data_map_t::const_iterator it = cut.begin(); it != cut.end(); ++it) {
      if (is_cut_diamond_visible[count]) {
	if (use_color) {
	  grid_point_t current_texture_level_xy = it->second->texture_level_xy();
	  if (current_texture_level_xy != previous_texture_level_xy &&
	      it->second->texture_image() != 0) {
	    cached_data_renderer_->bind_texture(it->second);
	    previous_texture_level_xy = current_texture_level_xy;
	  }
	}
        cached_data_renderer_->render(it->first, it->second);
      }
      ++count;
    }
  }

  void terrain_model_renderer::draw_diamonds_wireframe(const diamond_data_map_t& /*cut*/,
                                                       const std::vector<bool>& /*is_cut_diamond_visible*/) {
    // No-op: handled directly in draw() pass switches
  }

  void terrain_model_renderer::draw_overlays(const diamond_data_map_t& cut,
                                             const std::vector<bool>& is_cut_diamond_visible) {
    cached_data_renderer_->frame_finalize();

    if (draw_bounding_volumes_enabled_) {
      draw_bounding_volumes(cut, is_cut_diamond_visible);
    }

    float dmf = 1.0f - terrain_model_->data_missing_fraction();
    if (dmf < 0) dmf = 0;
    if (dmf < 0.9999) {
      cached_data_renderer_->draw_progress_bar(dmf);
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
    cached_data_renderer_->draw_bounding_volume(bvol, current_center_);
  }
  
  void terrain_model_renderer::set_wireframe_enabled(bool x) {
    wireframe_enabled_ = x;
  }

  bool terrain_model_renderer::is_wireframe_enabled() const {
    return wireframe_enabled_;
  }

  void terrain_model_renderer::set_draw_enabled(bool x) {
    cached_data_renderer_->set_draw_enabled(x);
  }

  bool terrain_model_renderer::is_draw_enabled() const {
    return cached_data_renderer_->draw_enabled();
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
    cached_data_renderer_->set_patch_color_enabled(x);
  }

  bool terrain_model_renderer::is_patch_color_enabled() const {
    return cached_data_renderer_->patch_color_enabled();
  }

  void terrain_model_renderer::set_elevation_map_enabled(bool x) {
    cached_data_renderer_->set_elevation_map_enabled(x);
  }

  void terrain_model_renderer::set_color_enabled(bool x) {
    cached_data_renderer_->set_use_color(x);
  }

  bool terrain_model_renderer::is_color_enabled() const {
    return cached_data_renderer_->use_color();
  }

  void terrain_model_renderer::set_shading_enabled(bool x) {
    cached_data_renderer_->set_use_normal(x);
  }

  bool terrain_model_renderer::is_shading_enabled() const {
    return cached_data_renderer_->use_normal();
  }

  bool terrain_model_renderer::is_elevation_map_enabled() const {
    return cached_data_renderer_->elevation_map_enabled();
  }

  void terrain_model_renderer::set_draw_bounding_volumes_enabled(bool x) {
    draw_bounding_volumes_enabled_ = x;
  }

  bool terrain_model_renderer::is_draw_bounding_volumes_enabled() const {
    return draw_bounding_volumes_enabled_;
  }

  void terrain_model_renderer::set_geometry_cache_capacity(uint32_t x) {
    cached_data_renderer_->set_vbo_cache_capacity(x);
  }

  void terrain_model_renderer::set_texture_cache_capacity(uint32_t x_bytes) {
    cached_data_renderer_->set_texture_cache_capacity(x_bytes);
  }

  void terrain_model_renderer::release_graphics() {
    cached_data_renderer_->clear();
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
    cached_data_renderer_->clear_texture_cache();
  }

} // namespace cbdam 


