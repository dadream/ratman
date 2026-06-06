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
#ifndef CBDAM_TERRAIN_MODEL_RENDERER_HPP
#define CBDAM_TERRAIN_MODEL_RENDERER_HPP

#include <vic/cbdam/base/config.hpp>
#include <vic/cbdam/base/terrain_model.hpp>
#include <vic/cbdam/base/cached_data_renderer.hpp>
#include <sl/oriented_box.hpp>
#include <sl/projective_map.hpp>
#include <sl/rigid_body_map.hpp>

namespace cbdam {
  
  /**
   *
   */
  class terrain_model_renderer {
  public:
    typedef grid_point_t                        diamond_id_t;
    typedef std::pair<grid_point_t, int>        diamond_patch_id_t;
    typedef terrain_model::diamond_data_map_t   diamond_data_map_t;
    typedef sl::oriented_box<3, double>         bounding_volume_t;
    typedef sl::projective_map3d                projective_map_t;
    typedef sl::rigid_body_map3d                rigid_body_map_t;
    
  protected:
    cached_data_renderer *      cached_data_renderer_;
    bool                        owns_renderer_;
    terrain_model *             terrain_model_;
    
    sl::projective_map3d        current_projection_;
    sl::rigid_body_map3d        current_view_;
    point3d_t			current_center_;

    float                       focus_fraction_;
    float                       screen_tolerance_;

    bool                        wireframe_enabled_;
    bool                        draw_bounding_volumes_enabled_;

    bool			is_opengl_supported_;
    bool			is_opengl_vbo_supported_;

    bool			adaptive_tolerance_enabled_;    
 
  public:

    terrain_model_renderer(terrain_model* tm);
    terrain_model_renderer(terrain_model* tm, cached_data_renderer* r);

    ~terrain_model_renderer();

    void set_terrain_model(terrain_model* tm);

    bool is_terrain_model_attached() const;

  public: // OpenGL init/finalization

    void init_opengl();

    bool is_opengl_supported() const;

    bool is_opengl_vbo_supported() const;

    void release_graphics();    
 
    void clear_texture_cache();    

  public: // On board caches

    void set_geometry_cache_capacity(uint32_t x);
    
    void set_texture_cache_capacity(uint32_t x_bytes);

  public: // Rendering

    void draw(const projective_map_t& P, 
	      const rigid_body_map_t& V,
	      const point3d_t& draw_origin);


    void set_adaptive_tolerance_enabled(bool x);

    bool is_adaptive_tolerance_enabled() const;


    void set_focus_fraction(float x);

    void set_screen_tolerance(float x);


    void set_draw_enabled(bool x);

    bool is_draw_enabled() const;


    void set_wireframe_enabled(bool x);

    bool is_wireframe_enabled() const;


    void set_patch_color_enabled(bool x);

    bool is_patch_color_enabled() const;


    void set_color_enabled(bool x);

    bool is_color_enabled() const;


    void set_elevation_map_enabled(bool x);

    bool is_elevation_map_enabled() const;


    void set_draw_bounding_volumes_enabled(bool x);

    bool is_draw_bounding_volumes_enabled() const;

    // opengl_cached_data_renderer::rendering_mode_t gl_rendering_mode() const;

  public: // 

    void set_shading_enabled(bool x);

    bool is_shading_enabled() const;

  public: // stats

    uint32_t stat_frame_vbo_creation_count();

    uint32_t stat_frame_vbo_reuse_count();

    uint32_t stat_frame_direct_render_count();

    uint32_t stat_rendered_triangles();

  protected:

    void draw_begin();

    void draw_end();

    void draw_diamonds(const diamond_data_map_t& cut, const std::vector<bool>& is_cut_diamond_visible);

    void draw_diamonds_wireframe(const diamond_data_map_t& cut, const std::vector<bool>& is_cut_diamond_visible);

    void draw_overlays(const diamond_data_map_t& cut, const std::vector<bool>& is_cut_diamond_visible);
    
    void draw_bounding_volumes(const diamond_data_map_t& cut, const std::vector<bool>& is_cut_diamond_visible) const;

    void draw_bounding_volume(const bounding_volume_t& bvol) const;

    void recompute_visibility(const diamond_data_map_t& cut,
                              std::vector<bool>& is_cut_diamond_visible) const;

    float angle_with_up_vector() const;
  };

} // namespace cbdam 

#endif // CBDAM_TERRAIN_MODEL_RENDERER_HPP

#ifndef CBDAM_TERRAIN_MODEL_RENDERER_IPP
#define CBDAM_TERRAIN_MODEL_RENDERER_IPP

namespace cbdam {

  inline uint32_t terrain_model_renderer::stat_frame_vbo_creation_count() {
    return cached_data_renderer_->stat_frame_vbo_creation_count();
  }

  inline uint32_t terrain_model_renderer::stat_frame_vbo_reuse_count() {
    return cached_data_renderer_->stat_frame_vbo_reuse_count();
  }

  inline uint32_t terrain_model_renderer::stat_frame_direct_render_count() {
    return cached_data_renderer_->stat_frame_direct_render_count();
  }

  inline uint32_t terrain_model_renderer::stat_rendered_triangles() {
    return cached_data_renderer_->stat_rendered_triangles();
  }

  inline bool terrain_model_renderer::is_terrain_model_attached() const {
    return terrain_model_ != 0;
  }

  inline bool terrain_model_renderer::is_opengl_supported() const {
    return is_opengl_supported_;
  }

  inline bool terrain_model_renderer::is_opengl_vbo_supported() const {
    return is_opengl_vbo_supported_;
  }

} // namespace cbdam 

#endif // CBDAM_TERRAIN_MODEL_RENDERER_IPP
