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
#ifndef CBDAM_CACHED_DATA_RENDERER_HPP
#define CBDAM_CACHED_DATA_RENDERER_HPP

#include <vic/cbdam/base/config.hpp>
#include <vic/cbdam/base/grid_point.hpp>
#include <vic/cbdam/base/coordinate_transform.hpp>
#include <vic/cbdam/base/diamond_patch_accessor.hpp>
#include <sl/fixed_size_point.hpp>
#include <sl/oriented_box.hpp>
#include <sl/projective_map.hpp>
#include <sl/rigid_body_map.hpp>
#include <utility>

namespace cbdam {

  typedef sl::fixed_size_point<4, float> point4_t;

  /**
   * Abstract interface for rendering cached terrain patches.
   * Enables decoupling of the terrain rendering algorithm from specific graphics backends.
   */
  class cached_data_renderer {
  public:
    typedef grid_point_t                                        diamond_id_t;
    typedef std::pair<diamond_id_t, uint32_t>                   diamond_patch_id_t;
    typedef diamond_patch_accessor                              diamond_patch_accessor_t;
    typedef sl::oriented_box<3, double>                         bounding_volume_t;
    typedef sl::projective_map3d                                projective_map_t;
    typedef sl::rigid_body_map3d                                rigid_body_map_t;

    enum rendering_pass_t {
      PASS_FILL,
      PASS_WIREFRAME
    };

    virtual ~cached_data_renderer() = default;

    virtual bool is_supported() const = 0;

    virtual void clear() = 0;
    virtual void clear_texture_cache() = 0;
    virtual void set_projection_parameters(const coordinate_transform* geo_xform) = 0;
    virtual void set_height_scale_factor(float /*scale*/) {}
    virtual void set_elevation_range(float min_h, float max_h) = 0;
    virtual void set_vbo_parameters(uint32_t height_patch_dim, uint32_t color_patch_dim) = 0;
    virtual void init_opengl() = 0;
    
    virtual void draw_begin() = 0;
    virtual void draw_end() = 0;
    virtual void set_rendering_pass(rendering_pass_t pass) = 0;
    virtual void set_matrices(const sl::projective_map3d& P, 
                              const sl::rigid_body_map3d& V, 
                              const point3d_t& center) = 0;
    virtual void draw_bounding_volume(const bounding_volume_t& bvol, const point3d_t& center) const = 0;
    virtual void draw_progress_bar(float progress) = 0;

    virtual void frame_initialize(const point4_t& surface_color) = 0;
    virtual void frame_finalize() = 0;
    virtual void bind_texture(const diamond_patch_accessor_t* x) = 0;
    virtual void render(const diamond_patch_id_t& node_idx, const diamond_patch_accessor_t* data_ptr) = 0;
    virtual bool has(const diamond_patch_id_t& node_idx) const = 0;
    virtual void set_texture_cache_capacity(uint32_t capacity_bytes) = 0;
    virtual void set_elevation_map_enabled(bool x) = 0;
    virtual bool elevation_map_enabled() const = 0;
    virtual void set_use_color(bool x) = 0;
    virtual bool use_color() const = 0;
    virtual void set_use_normal(bool x) = 0;
    virtual bool use_normal() const = 0;
    virtual uint32_t current_vbo_cache_count() const = 0;

    virtual void set_vbo_cache_capacity(uint32_t capacity) = 0;
    virtual const uint32_t& vbo_cache_capacity() const = 0;
    virtual void set_patch_color_enabled(bool enabled) = 0;
    virtual const bool& patch_color_enabled() const = 0;
    virtual void set_draw_enabled(bool enabled) = 0;
    virtual const bool& draw_enabled() const = 0;

    virtual const uint32_t& current_vbo_cache_size() const = 0;
    virtual const uint32_t& current_frame_id() const = 0;
    virtual const uint32_t& stat_frame_vbo_creation_count() const = 0;
    virtual const uint32_t& stat_frame_vbo_reuse_count() const = 0;
    virtual const uint32_t& stat_frame_direct_render_count() const = 0;
    virtual const uint32_t& stat_rendered_triangles() const = 0;
  };

} // namespace cbdam

#endif // CBDAM_CACHED_DATA_RENDERER_HPP
