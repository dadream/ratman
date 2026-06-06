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
#ifndef CBDAM_OPENGL_CACHED_DATA_RENDERER_HPP
#define CBDAM_OPENGL_CACHED_DATA_RENDERER_HPP

#ifdef _WIN32
#include <windows.h>
#undef min
#undef max
#endif

#include <vic/cbdam/base/config.hpp>
#include <vic/cbdam/base/grid_point.hpp>
#include <vic/cbdam/base/coordinate_transform.hpp>
#include <vic/cbdam/base/texture_manager.hpp>
#include <vic/cbdam/base/cached_data_renderer.hpp>
#include <map>
#include <set>
#include <vector>
#include <GL/glew.h>

namespace cbdam {

  typedef uint32_t (*decore_texture_function_t)(void* context,
                                                const double* tm_g2l,
                                                const double* tm_l2g,
                                                double magnification);

  class vbo_descriptor {
  protected:
    GLuint   id_;
    uint32_t patch_id_;
    uint32_t texture_id_;
    uint32_t last_access_frame_id_;
 
  public:
     vbo_descriptor(GLuint id = 0, 
		    uint32_t patch_id = 0, 
		    uint32_t texture_id = 0, 
		    uint32_t last_access_frame_id = 0) :
         id_(id), patch_id_(patch_id), 
	 texture_id_(texture_id), last_access_frame_id_(last_access_frame_id) {
        
     }

    CBDAM_R_ACCESSOR(GLuint, id);
    CBDAM_R_ACCESSOR(uint32_t, patch_id);
    CBDAM_R_ACCESSOR(uint32_t, texture_id);
    CBDAM_RW_ACCESSOR(uint32_t, last_access_frame_id);
  };
  
  /**
   *
   */
  class opengl_cached_data_renderer : public cached_data_renderer {
  public:
    typedef grid_point_t                                        diamond_id_t;
    typedef grid_diamond                                        grid_diamond_t;    
    typedef std::pair<diamond_id_t, uint32_t>                   diamond_patch_id_t;
    typedef std::map<diamond_patch_id_t, vbo_descriptor>        vbo_cache_t;
    typedef diamond_patch_accessor                              diamond_patch_accessor_t;
    typedef std::pair<uint32_t,const diamond_patch_accessor_t*> frame_diamond_pair_t;
    typedef enum { 
      RENDERING_MODE_NONE,
      RENDERING_MODE_BASIC,
      RENDERING_MODE_VBO_ARB,
      RENDERING_MODE_VBO
    } rendering_mode_t;

  protected:
    // rendering mode
    rendering_mode_t            gl_rendering_mode_;

    // cache
    vbo_cache_t                 vbo_cache_;
    texture_manager             texture_manager_;
    uint32_t                    current_frame_id_;
    uint32_t                    vbo_cache_capacity_;
    uint32_t                    current_vbo_cache_size_;

    // other vbos
    GLuint                      vbo_id_uv_coordinates_patch0_;
    GLuint                      vbo_id_uv_coordinates_patch1_;
    GLuint                      vbo_id_indices_;
    uint32_t                    number_of_indices_;
    GLuint                      tex_id_elevation_map_;    // work on GL_TEXTURE0_ARB                                 

    // data needed for direct rendering and to not reduce dynamic allocation
    std::vector<point2_t>       uv_coordinates_patch0_;
    std::vector<point2_t>       uv_coordinates_patch1_;
    std::vector<uint16_t>       indices_stitched_strip_;
    std::vector<float>		tmp_vbo_spherical_tex_coordinates_;

    // data description
    const coordinate_transform* geo_xform_;
    float                       min_h_;
    float                       max_h_;
    float                       height_scale_factor_;
    uint32_t                    patch_triangle_count_;
    uint32_t                    height_vertex_count_;
    uint32_t                    data_size_;
    uint32_t                    normal_size_;
    uint32_t			spherical_tex_size_;
    uint32_t                    height_patch_dim_;
    uint32_t                    texture_tile_width_;

    // state
    bool                        elevation_map_enabled_;
    bool                        patch_color_enabled_;
    bool                        draw_enabled_;
    bool                        color_available_;
    bool                        use_color_;
    bool                        use_normal_;

    // stats
    uint32_t                    stat_frame_vbo_creation_count_;
    uint32_t                    stat_frame_vbo_reuse_count_;
    uint32_t                    stat_frame_direct_render_count_;
    uint32_t                    stat_rendered_triangles_;
    
    // Stored matrices for rendering
    sl::projective_map3d        current_projection_;
    sl::rigid_body_map3d        current_view_;
    point3d_t                   current_center_;
    sl::matrix4f                base_modelview_;
    rendering_pass_t            current_pass_;
    
  public:
    opengl_cached_data_renderer(std::size_t capacity = 96*1024*1024);

    ~opengl_cached_data_renderer();

    bool is_supported() const override { return gl_rendering_mode_ != RENDERING_MODE_NONE; }

    void clear() override;

    void clear_texture_cache() override;
    
    void set_projection_parameters(const coordinate_transform* geo_xform) override;

    void set_height_scale_factor(float scale) override;

    void set_elevation_range(float min_h, float max_h) override;

    void set_vbo_parameters(uint32_t height_patch_dim, uint32_t color_patch_dim) override;
    
    void init_opengl() override;

    void draw_begin() override;
    void draw_end() override;
    void set_rendering_pass(rendering_pass_t pass) override;
    void set_matrices(const sl::projective_map3d& P, 
                      const sl::rigid_body_map3d& V, 
                      const point3d_t& center) override;
    void draw_bounding_volume(const bounding_volume_t& bvol, const point3d_t& center) const override;
    void draw_progress_bar(float progress) override;
    
    void frame_initialize(const point4_t& surface_color) override;

    void frame_finalize() override;

    void bind_texture(const diamond_patch_accessor_t* x) override;
   
    void render(const diamond_patch_id_t& node_idx, const diamond_patch_accessor_t* data_ptr) override;

    bool has(const diamond_patch_id_t& node_idx) const override;

    void set_texture_cache_capacity(uint32_t capacity_bytes) override;

    void set_elevation_map_enabled(bool x) override;
    bool elevation_map_enabled() const override;

    rendering_mode_t gl_rendering_mode() const;

    void set_use_color(bool x) override;
    bool use_color() const override;
    void set_use_normal(bool x) override;
    bool use_normal() const override;
    uint32_t current_vbo_cache_count() const override;

    CBDAM_RW_ACCESSOR(uint32_t, vbo_cache_capacity);
    void set_vbo_cache_capacity(uint32_t capacity) override { vbo_cache_capacity_ = capacity; }
    CBDAM_RW_ACCESSOR(bool, patch_color_enabled);
    void set_patch_color_enabled(bool enabled) override { patch_color_enabled_ = enabled; }
    CBDAM_RW_ACCESSOR(bool, draw_enabled);
    void set_draw_enabled(bool enabled) override { draw_enabled_ = enabled; }

    CBDAM_R_ACCESSOR(uint32_t, current_vbo_cache_size);
    CBDAM_R_ACCESSOR(uint32_t, current_frame_id);
    CBDAM_R_ACCESSOR(uint32_t, stat_frame_vbo_creation_count);
    CBDAM_R_ACCESSOR(uint32_t, stat_frame_vbo_reuse_count);
    CBDAM_R_ACCESSOR(uint32_t, stat_frame_direct_render_count);
    CBDAM_R_ACCESSOR(uint32_t, stat_rendered_triangles);

    uint32_t patch_size() const;
  protected:
    vbo_cache_t::iterator create_vbo_diamond_patch(const diamond_patch_accessor_t* data_ptr, const diamond_patch_id_t& node_idx);
    void create_vbo_indices();
    void create_vbo_uv_coordinates();
    void create_vbo_patch(const diamond_patch_accessor_t* data_ptr, GLuint vbo_id);
    
    void vbo_cache_cleanup();
    void vbo_cache_clear();
    void vbo_render(const vbo_descriptor& vbo_desc);
    void direct_render(const diamond_patch_accessor_t* data_ptr, const diamond_patch_id_t& node_idx);
    void bind_elevation_texture();

  protected:
    void vbo_gen_buffers(GLsizei n, GLuint *buffers) const;

    void vbo_bind_buffer(GLuint buffer) const;

    void vbo_bind_element_buffer(GLuint buffer) const;
    
    void vbo_delete_buffers(GLsizei n, const GLuint *buffers) const;

    void vbo_element_buffer_data(GLsizeiptr size, const GLvoid *data, GLenum usage) const;

    void vbo_buffer_data(GLsizeiptr size, const GLvoid *data, GLenum usage) const;

    void vbo_buffer_sub_data(GLintptr offset, GLsizeiptr size, const GLvoid *data) const;

  };


} // namespace cbdam 

#endif // CBDAM_OPENGL_CACHED_DATA_RENDERER_HPP

#ifndef CBDAM_OPENGL_CACHED_DATA_RENDERER_IPP
#define CBDAM_OPENGL_CACHED_DATA_RENDERER_IPP

namespace cbdam {
 
  inline uint32_t opengl_cached_data_renderer::current_vbo_cache_count() const {
    return vbo_cache_.size();
  }

  inline bool opengl_cached_data_renderer::has(const diamond_patch_id_t& node_idx) const {
    return vbo_cache_.find(node_idx) != vbo_cache_.end();
  }

  inline uint32_t opengl_cached_data_renderer::patch_size() const {
    return (data_size_ + 
	    (use_normal_ ? normal_size_ : 0) +
	    ((!geo_xform_->is_planar() && elevation_map_enabled_) ? spherical_tex_size_ : 0));
  }
  
} // namespace cbdam 

#endif // CBDAM_OPENGL_CACHED_DATA_RENDERER_IPP
