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
#include <stdint.h>

#ifdef _WIN32
#include <windows.h>
#undef min
#undef max
#endif

#include <vic/cbdam/base/opengl_cached_data_renderer.hpp>
#include <vic/cbdam/base/color_rgb.hpp>
#include <vic/img/gl_image.hpp>
#include <sl/triangle_mesh_stripifier.hpp>
#include <sl/interpolation.hpp>
#include <sl/clock.hpp>
#include <cmath>

namespace cbdam {

  static uint8_t* color_map(uint32_t w, uint32_t h) {
    typedef sl::fixed_size_point<4,uint8_t> rgba_t;
    rgba_t* result = new rgba_t[w*h];
    
    sl::interpolation_track<float, point3_t> hmap;
    hmap.set_continuity(0);

#if 0
    // Mars
    hmap[-.1/6.0] = point3_t(0.50, 0.47, 0.42);
    hmap[0.0/6.0] = point3_t(0.50, 0.47, 0.42);
    hmap[1.0/6.0] = point3_t(0.76, 0.49, 0.36);
    hmap[2.0/6.0] = point3_t(0.94, 0.50, 0.29);
    hmap[3.0/6.0] = point3_t(0.98, 0.55, 0.30);
    hmap[4.0/6.0] = point3_t(0.99, 0.71, 0.43);
    hmap[5.0/6.0] = point3_t(1.00, 0.84, 0.61);
    hmap[6.0/6.0] = point3_t(1.00, 0.91, 0.75);
    hmap[6.1/6.0] = point3_t(1.00, 0.91, 0.75);
#elif 0
    // Earth
    hmap[-.1/8192.0]    = point3_t(0.0, 0.0, 1.0);
    hmap[16.0/8192.0]    = point3_t(0.0, 0.0, 1.0);
    hmap[100.0/8192.0]  = point3_t(0.0, 1.0, 0.0);
    hmap[1024.0/8192.0] = point3_t(0.90, 0.40, 0.30);
    hmap[2048.0/8192.0] = point3_t(1.00, 0.55, 0.30);
    hmap[4096.0/8192.0] = point3_t(1.00, 0.84, 0.61);
    hmap[8192.0/8192.0] = point3_t(1.00, 0.91, 0.75);
    hmap[8192.1/8192.0] = point3_t(1.00, 0.91, 0.75);
#else
    // Earth
    hmap[-16/5436.0]    = point3_t(0.3, 0.3, 0.7);
    hmap[0/5436.0]      = point3_t(0.3, 0.3, 0.7);
    //    hmap[16.0/5436.0]   = point3_t(0.3, 0.3, 0.7);
    //    hmap[17.0/5436.0]   = point3_t(0.02, 0.50, 0.30);
    hmap[0.5/5436.0]   = point3_t(0.3, 0.3, 0.7);
    hmap[1.0/5436.0]   = point3_t(0.02, 0.50, 0.30);
    hmap[400.0/5436.0]  = point3_t(0.53, 0.73, 0.58);
    hmap[800.0/5436.0] = point3_t(0.63, 0.75, 0.60);
    hmap[1200.0/5436.0] = point3_t(0.77, 0.76, 0.74);
    hmap[1600.0/5436.0] = point3_t(0.85, 0.78, 0.68);
    hmap[2000.0/5436.0] = point3_t(0.94, 0.75, 0.45);
    hmap[2400.0/5436.0] = point3_t(0.86, 0.62, 0.42);
    hmap[2800.0/5436.0] = point3_t(0.80, 0.59, 0.44);
    hmap[3200.0/5436.0] = point3_t(1.00, 0.82, 0.66);
    hmap[3600.0/5436.0] = point3_t(0.99, 0.93, 0.83);
    hmap[4000.0/5436.0] = point3_t(1.00, 1.00, 0.88);
    hmap[4400.0/5436.0] = point3_t(1.00, 1.00, 1.00);
    hmap[4800.0/5436.0] = point3_t(1.00, 1.00, 1.00);
    hmap[5200.0/5436.0] = point3_t(1.00, 1.00, 1.00);
    hmap[5600.0/5436.0] = point3_t(1.00, 1.00, 1.00);
#endif

    for (std::size_t i=0; i<w; ++i) {
      float    ii   = float(i)/float(w-1);
      point3_t v_h = hmap.value_at(ii);
      for (std::size_t j=0; j<h; ++j) {
	// Modify based on j = latitude
        float    jj = float(j)/float(h-1);
        float     l = 2.0f*sl::abs(jj-0.5f);
        point3_t v_pole =  point3_t(1.00, 1.00, 1.00);
        point3_t v = (l<0.75f) ? v_h : v_h.lerp(v_pole, (l-0.75f));
	result[i+j*w] = rgba_t(uint8_t(sl::median(0.0f, 255.0f, v[0]*255.0f)), 
			       uint8_t(sl::median(0.0f, 255.0f, v[1]*255.0f)), 
			       uint8_t(sl::median(0.0f, 255.0f, v[2]*255.0f)),
			       uint8_t(255));
      }
    }
     
    return (uint8_t*)result; 
  }
 
  void opengl_cached_data_renderer::vbo_gen_buffers(GLsizei n, GLuint *buffers) const {
    if (gl_rendering_mode_ == RENDERING_MODE_VBO) {
      glGenBuffers(n, buffers);
    } else if (gl_rendering_mode_ == RENDERING_MODE_VBO_ARB) {
      glGenBuffersARB(n, buffers);
    }
  }

  void opengl_cached_data_renderer::vbo_bind_buffer(GLuint buffer) const {
    if (gl_rendering_mode_ == RENDERING_MODE_VBO) {
      glBindBuffer(GL_ARRAY_BUFFER, buffer);
    } else if (gl_rendering_mode_ == RENDERING_MODE_VBO_ARB) {
      glBindBufferARB(GL_ARRAY_BUFFER_ARB, buffer);
    }
  }

  void opengl_cached_data_renderer::vbo_bind_element_buffer(GLuint buffer) const {
    if (gl_rendering_mode_ == RENDERING_MODE_VBO) {
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer);
    } else if (gl_rendering_mode_ == RENDERING_MODE_VBO_ARB) {
      glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, buffer);
    }
  }
  void opengl_cached_data_renderer::vbo_delete_buffers(GLsizei n, const GLuint *buffers) const {
    if (gl_rendering_mode_ == RENDERING_MODE_VBO) {
      glDeleteBuffers(n, buffers);
    } else if (gl_rendering_mode_ == RENDERING_MODE_VBO_ARB) {
      glDeleteBuffersARB(n, buffers);
    }
  }

  void opengl_cached_data_renderer::vbo_element_buffer_data(GLsizeiptr size, const GLvoid *data, GLenum usage) const {
    if (gl_rendering_mode_ == RENDERING_MODE_VBO) {
      glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, data, usage);
    } else if (gl_rendering_mode_ == RENDERING_MODE_VBO_ARB) {
      glBufferDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB, size, data, usage);
    }
  }

  void opengl_cached_data_renderer::vbo_buffer_data(GLsizeiptr size, const GLvoid *data, GLenum usage) const {
    if (gl_rendering_mode_ == RENDERING_MODE_VBO) {
      glBufferData(GL_ARRAY_BUFFER, size, data, usage);
    } else if (gl_rendering_mode_ == RENDERING_MODE_VBO_ARB) {
      glBufferDataARB(GL_ARRAY_BUFFER_ARB, size, data, usage);
    }
  }

  void opengl_cached_data_renderer::vbo_buffer_sub_data(GLintptr offset, 
							GLsizeiptr size, 
							const GLvoid *data) const {
    if (gl_rendering_mode_ == RENDERING_MODE_VBO) {
      glBufferSubData(GL_ARRAY_BUFFER, offset, size, data);
    } else if (gl_rendering_mode_ == RENDERING_MODE_VBO_ARB) {
      glBufferSubDataARB(GL_ARRAY_BUFFER_ARB, offset, size, data);
    }
  }


  opengl_cached_data_renderer::opengl_cached_data_renderer(std::size_t vbo_capacity) {
    current_frame_id_ = 0;
    vbo_cache_capacity_ = vbo_capacity;
    current_vbo_cache_size_ = 0;
    geo_xform_ = 0;
    patch_triangle_count_ = 0;
    data_size_ = 0;
    normal_size_ = 0;
    patch_color_enabled_ = false;
    elevation_map_enabled_ = true;
    draw_enabled_ = true;
    use_color_ = false;
    use_normal_ = true;

    tex_id_elevation_map_ = 0;
    height_scale_factor_ = 1.0f;

    vbo_id_indices_ = 0;
    vbo_id_uv_coordinates_patch0_ = 0;
    vbo_id_uv_coordinates_patch1_ = 0;

    gl_rendering_mode_ = RENDERING_MODE_NONE; 
    current_pass_ = PASS_FILL;
  }

  opengl_cached_data_renderer::~opengl_cached_data_renderer() {
    clear();
  }
 
  void opengl_cached_data_renderer::clear() {
    vbo_cache_clear();
    texture_manager_.clear();

    if (vbo_id_indices_ != 0)          {vbo_delete_buffers(1, &vbo_id_indices_); vbo_id_indices_ = 0;}
    if (vbo_id_uv_coordinates_patch0_) {vbo_delete_buffers(1, &vbo_id_uv_coordinates_patch0_); vbo_id_uv_coordinates_patch0_ = 0;}
    if (vbo_id_uv_coordinates_patch1_) {vbo_delete_buffers(1, &vbo_id_uv_coordinates_patch1_); vbo_id_uv_coordinates_patch1_ = 0;}

    if (tex_id_elevation_map_ != 0) {
      glDeleteTextures(1, &tex_id_elevation_map_);
    }
  }

  opengl_cached_data_renderer::rendering_mode_t opengl_cached_data_renderer::gl_rendering_mode() const {
    return gl_rendering_mode_;
  }

  void opengl_cached_data_renderer::set_height_scale_factor(float scale) {
    height_scale_factor_ = scale;
  }

  void opengl_cached_data_renderer::set_elevation_range(float min_h, float max_h) {
    min_h_ = min_h;
    // FIXME WIN32 GL_CLAMP doesn't work properly for elevation map..
    max_h_ = max_h * 1.1;
  }

  void opengl_cached_data_renderer::set_vbo_parameters(uint32_t height_patch_dim, uint32_t texture_tile_width) {
    SL_TRACE_OUT(1) << "HEIGHT_PATCH_DIM " << height_patch_dim << ", TEXTURE_TILE_WIDTH " << texture_tile_width << std::endl;
    texture_manager_.set_tile_width(texture_tile_width);
    height_patch_dim_		= height_patch_dim;

    height_vertex_count_	= ((height_patch_dim_ + 2) * (height_patch_dim_ + 1) / 2);
    patch_triangle_count_	= height_patch_dim_ * height_patch_dim_;
    data_size_			= height_vertex_count_ * sizeof(point3_t);
    normal_size_		= height_vertex_count_ * sizeof(normal_t);
    spherical_tex_size_		= height_vertex_count_ * sizeof(float);
    tmp_vbo_spherical_tex_coordinates_.resize(height_vertex_count_);

    color_available_ = (texture_tile_width != 0);

    elevation_map_enabled_ = !color_available_;

    set_use_color(color_available_);
  }

  void opengl_cached_data_renderer::init_opengl() {
    // Define rendering mode
    GLenum err = glewInit();
    if (err != GLEW_OK) {
      std::cerr  << "GLEW init error: "  << glewGetErrorString(err) << std::endl;
      gl_rendering_mode_ = RENDERING_MODE_NONE;
    } else if (GLEW_VERSION_1_5) {
      gl_rendering_mode_ = RENDERING_MODE_VBO;
    } else if (GLEW_ARB_vertex_buffer_object) {
      gl_rendering_mode_ = RENDERING_MODE_VBO_ARB;
    } else if (GLEW_VERSION_1_1) {
      gl_rendering_mode_ = RENDERING_MODE_BASIC;
    } else {
      gl_rendering_mode_ = RENDERING_MODE_NONE;
    }

    // Init
    create_vbo_uv_coordinates();
    create_vbo_indices();
  }
  
  void opengl_cached_data_renderer::set_projection_parameters(const coordinate_transform* geo_xform) {
    geo_xform_ = geo_xform;
  }

  void opengl_cached_data_renderer::set_texture_cache_capacity(uint32_t capacity_bytes) {
    texture_manager_.set_cache_capacity(capacity_bytes);
  }

  void opengl_cached_data_renderer::bind_elevation_texture() {
    // when using vbo rendering, texture rectangle is active, remove it
    glEnable(GL_TEXTURE_2D);
    if (tex_id_elevation_map_ == 0) {
      const uint32_t data_width  = 256;
      const uint32_t data_height = 256;
      uint8_t       *data = color_map(data_width, data_height);
      glGenTextures(1, &tex_id_elevation_map_);
      glBindTexture(GL_TEXTURE_2D, tex_id_elevation_map_);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 
                   data_width, 
                   data_height, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                   data);
      delete[] data; data=0;
    } else {
      glBindTexture(GL_TEXTURE_2D, tex_id_elevation_map_);
    }
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    if (geo_xform_->is_planar()) {
      // enable texgen to get texture coordinates from heights
      float param_s[4]= { 0, 0, 1.0f / (height_scale_factor_ * (max_h_ - min_h_)), 0.0f };
      glEnable(GL_TEXTURE_GEN_S);
      glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
      glTexGenfv( GL_S, GL_OBJECT_PLANE, param_s);
    } else {
      // spherical use texture coord array
    }
  }

  void opengl_cached_data_renderer::frame_initialize(const point4_t& surface_color) {
    // Free unused memory of old frames. This is done before starting a
    // new frame instead of at the end of a frame since it is more stable
    // and possibly more efficient since the pipe-line is hopefully on
    // idle

    vbo_cache_cleanup();
    texture_manager_.cleanup(current_frame_id_);

    ++current_frame_id_;
    stat_frame_vbo_creation_count_ = 0;
    stat_frame_vbo_reuse_count_ = 0;
    stat_frame_direct_render_count_ =0;
    stat_rendered_triangles_ = 0;

    if (draw_enabled_) {
      glPushAttrib(GL_ENABLE_BIT);
      
      glColor4fv(surface_color.to_pointer());
      glShadeModel(GL_SMOOTH);
      glEnable(GL_CULL_FACE);

      // enable vertex arrays for vbos
      glEnableClientState(GL_VERTEX_ARRAY);
      if (use_color_ || (!geo_xform_->is_planar() && elevation_map_enabled_)) {
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
      } else {
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
      }
      if (use_normal_) {
        glEnableClientState(GL_NORMAL_ARRAY);
      } else {
        glDisableClientState(GL_NORMAL_ARRAY);
      }

      if (use_color_) {
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
        glEnable(GL_TEXTURE_2D);
      } else {
        glDisable(GL_TEXTURE_2D);
      }

      if (elevation_map_enabled()) {
        bind_elevation_texture();
      } 
    }
  }
    
  void opengl_cached_data_renderer::frame_finalize() {
    if (draw_enabled_) {
      glPopAttrib();    // disable vertex arrays & co

      glDisableClientState(GL_VERTEX_ARRAY);
      if (use_color_ || (!geo_xform_->is_planar() && elevation_map_enabled_))  {glDisableClientState(GL_TEXTURE_COORD_ARRAY);}
      if (use_normal_) {glDisableClientState(GL_NORMAL_ARRAY);}
      
	vbo_bind_element_buffer(0);
	vbo_bind_buffer(0);
    }
    
    SL_TRACE_OUT(1) << "Frame end: Allocated " << current_vbo_cache_count() << " VBOS: " << current_vbo_cache_size() << "/" << vbo_cache_capacity() << " bytes" << std::endl; 
  }

  void opengl_cached_data_renderer::render(const diamond_patch_id_t& node_idx, const diamond_patch_accessor_t* data_ptr) {
    if (gl_rendering_mode_ == RENDERING_MODE_NONE) return;

    // Load texture matrix
    if (use_color_) {
      glMatrixMode(GL_TEXTURE);
      glLoadMatrixd(data_ptr->texture_matrix().to_pointer());
      glMatrixMode(GL_MODELVIEW);
    }

    // Load modelview matrix
    const vector3d_t diamond_offset = data_ptr->vertices()->diamond_center() - current_center_;
    sl::matrix4f VM_prime = base_modelview_;
    VM_prime *= sl::matrix4f(1.0f, 0.0f, 0.0f, static_cast<float>(diamond_offset[0]),
                             0.0f, 1.0f, 0.0f, static_cast<float>(diamond_offset[1]),
                             0.0f, 0.0f, 1.0f, static_cast<float>(diamond_offset[2]),
                             0.0f, 0.0f, 0.0f, 1.0f);
    glLoadMatrixf(VM_prime.to_pointer());

    vbo_cache_t::iterator cache_it = vbo_cache_.find(node_idx);
    if (gl_rendering_mode_ == RENDERING_MODE_BASIC) {
      // vertex arrays are faster then display list: do not use cache/display lists for direct rendering.
      direct_render(data_ptr, node_idx);
    } else {
      if (cache_it == vbo_cache_.end()) {
        cache_it = create_vbo_diamond_patch(data_ptr, node_idx);
      }
      if (cache_it != vbo_cache_.end()) {
        vbo_render(cache_it->second);
        cache_it->second.last_access_frame_id() = current_frame_id_;
      } else {
        // NO VBO: we couldn't find one, and the cache is full!
        assert( data_ptr != 0 );
        direct_render(data_ptr, node_idx);
      }
    }
  }
  
  ///////////////////////////////////// vbo stuff ///////////////////////////////

  void opengl_cached_data_renderer::vbo_cache_cleanup() {
    sl::real_time_clock cleanup_clock;
    cleanup_clock.restart();
    std::size_t cleanup_count = 0;
    for (uint32_t vbo_lifetime = 8;
         (vbo_lifetime >=2) && ((vbo_lifetime == 8) || (2*current_vbo_cache_size() > vbo_cache_capacity()));
         vbo_lifetime /= 2) {
      for (vbo_cache_t::iterator vbo_it = vbo_cache_.begin();
           (vbo_it != vbo_cache_.end()) && (((vbo_lifetime == 8) || (2*current_vbo_cache_size() > vbo_cache_capacity()))) ;
           ) {
        vbo_cache_t::iterator vbo_it_del = vbo_it;
        ++vbo_it;
        if (vbo_it_del->second.last_access_frame_id()+vbo_lifetime <= current_frame_id()) {
	  ++cleanup_count;
          GLuint vbo_id = vbo_it_del->second.id();
          SL_TRACE_OUT(0) << "Cleanup: VBO-recycle: " << vbo_id << std::endl;
          if (gl_rendering_mode_ == RENDERING_MODE_BASIC) {
            glDeleteLists(vbo_id, 1);
          } else {
	    vbo_delete_buffers(1, &vbo_id);
          }
          current_vbo_cache_size_ -= patch_size();
          vbo_cache_.erase(vbo_it_del);
        }
      }    
    }
    int cleanup_ms = int(cleanup_clock.elapsed().as_microseconds()*1e-3);
    if (cleanup_count) {
      SL_TRACE_OUT(1) << "VBO Cleanup: " << 
	" deleted " << cleanup_count << " VBOs " <<
	" in " << cleanup_ms << " ms " <<
	std::endl;
    }
  }

  void opengl_cached_data_renderer::vbo_cache_clear() {
    for (vbo_cache_t::iterator vbo_it = vbo_cache_.begin();
         vbo_it != vbo_cache_.end();
         ) {
      vbo_cache_t::iterator vbo_it_del = vbo_it;
      ++vbo_it;
      GLuint vbo_id = vbo_it_del->second.id();
      SL_TRACE_OUT(1) << "Clear: VBO-delete: " << vbo_id << std::endl;
      if (gl_rendering_mode_ == RENDERING_MODE_BASIC) {
        glDeleteLists(vbo_id, 1);
      } else {
	  vbo_delete_buffers(1, &vbo_id);
      }
      current_vbo_cache_size_ -= patch_size();
      vbo_cache_.erase(vbo_it_del);
    }
  }

  void opengl_cached_data_renderer::create_vbo_indices() {
    // triangulate
    sl::triangle_mesh_greedy_stripifier tms;
    tms.set_cache_size(16);
    tms.begin_input();
    uint32_t count = 0;
    for(uint32_t y = 0; y < height_patch_dim_; ++y) {
      uint32_t count_next_row = count + height_patch_dim_ + 1 - y;
      for(uint32_t x = 0; x < height_patch_dim_ - y; ++x) {
        count_next_row = count + height_patch_dim_ + 1 - y;
        tms.insert_input_triangle(count, count+1, count_next_row);
        if (x < height_patch_dim_ - y - 1) {
          tms.insert_input_triangle(count_next_row+1, count_next_row , count+1);
        }
        ++count;
      }
      ++count;
    }
    tms.end_input();

    // stripify and stitch strips
    indices_stitched_strip_.clear();
    tms.begin_output();
    while (tms.has_output_strip()) {
      tms.begin_output_strip();
      std::size_t v0 = tms.get_output_strip_vertex();
      if (indices_stitched_strip_.size()>0) {
        // close old strip
        indices_stitched_strip_.push_back(indices_stitched_strip_[indices_stitched_strip_.size()-1]);

        indices_stitched_strip_.push_back(v0);
        if (indices_stitched_strip_.size()%2==1) {
          indices_stitched_strip_.push_back(v0);
        }
      }
      assert(indices_stitched_strip_.size() % 2 == 0);
      indices_stitched_strip_.push_back(v0);

      while(tms.has_output_vertex()) {
        indices_stitched_strip_.push_back(tms.get_output_strip_vertex());
      }
      tms.end_output_strip();
    }
    tms.end_output();

    // now indices_stitched_strip_ should contain a proper single strip.

      // set indices in the vbo
      if (vbo_id_indices_ != 0) {
	vbo_delete_buffers(1, &vbo_id_indices_);
        vbo_id_indices_ = 0;
      }
      number_of_indices_ = indices_stitched_strip_.size();
      vbo_gen_buffers(1, &vbo_id_indices_);
      assert(vbo_id_indices_);
      vbo_bind_element_buffer(vbo_id_indices_);
      vbo_element_buffer_data(number_of_indices_*sizeof(uint16_t), (void*)(&(indices_stitched_strip_[0])), GL_STATIC_DRAW);
      vbo_bind_buffer(0);
  }
  
  void opengl_cached_data_renderer::create_vbo_uv_coordinates() {
      if (vbo_id_uv_coordinates_patch0_ != 0) {
	vbo_delete_buffers(1, &vbo_id_uv_coordinates_patch0_);
	vbo_id_uv_coordinates_patch0_ = 0;
      }
      if (vbo_id_uv_coordinates_patch1_ != 0) {
	vbo_delete_buffers(1, &vbo_id_uv_coordinates_patch1_);
	vbo_id_uv_coordinates_patch1_ = 0;
      }

    // set uv coordinates of points of a triangle with patch_dim_+1 points x edge
    // create a uv_coordinates array for patch0 and patch1
    uv_coordinates_patch0_.resize((height_patch_dim_+2)*(height_patch_dim_+1)/2);
    uv_coordinates_patch1_.resize((height_patch_dim_+2)*(height_patch_dim_+1)/2);
    
    uint32_t count = 0;
    const uint32_t ymax = height_patch_dim_;
    //    float texture_tile_width = color_available_ ? texture_manager_.tile_width() : height_patch_dim_;
    //    const float color_height_ratio = texture_tile_width / ymax;
    for(uint32_t y = 0; y <= ymax; ++y) {
      for(uint32_t x = 0; x <= ymax - y; ++x) {
        float u = x / (float)ymax;
        float v = y / (float)ymax;

	uv_coordinates_patch0_[count] = point2_t(u, 1.0f - v);
	uv_coordinates_patch1_[count] = point2_t(1.0f - u, v); 
        ++count;
      }
    }

      vbo_gen_buffers(1, &vbo_id_uv_coordinates_patch0_);
      assert(vbo_id_uv_coordinates_patch0_);
      vbo_bind_buffer(vbo_id_uv_coordinates_patch0_);
      vbo_buffer_data(uv_coordinates_patch0_.size()*sizeof(point2_t), (void*)(&(uv_coordinates_patch0_[0])), GL_STATIC_DRAW);
      
      vbo_gen_buffers(1, &vbo_id_uv_coordinates_patch1_);
      assert(vbo_id_uv_coordinates_patch1_);
      vbo_bind_buffer(vbo_id_uv_coordinates_patch1_);
      vbo_buffer_data(uv_coordinates_patch1_.size()*sizeof(point2_t), (void*)(&(uv_coordinates_patch1_[0])), GL_STATIC_DRAW);
      
      vbo_bind_buffer(0);

    //    std::cerr << "generated tex coords buffers " << vbo_id_uv_coordinates_patch0_ << ", " << vbo_id_uv_coordinates_patch1_ << std::endl;
    //    std::cerr << "coords buffers sizes         " << uv_coordinates_patch0_.size() << ", " << uv_coordinates_patch1_.size() << std::endl;
  }

  opengl_cached_data_renderer::vbo_cache_t::iterator opengl_cached_data_renderer::create_vbo_diamond_patch(const diamond_patch_accessor_t* data_ptr,
                                                                                                           const diamond_patch_id_t& node_idx) {
    assert(gl_rendering_mode_ == RENDERING_MODE_VBO_ARB || gl_rendering_mode_ == RENDERING_MODE_VBO);

    //    std::cerr << "ocdr::create_vbo_diamond_patch " << node_idx.first << " " << node_idx.second << std::endl;
    uint32_t sz = patch_size();
    if (current_vbo_cache_size() + sz <= vbo_cache_capacity()) {
      GLuint vbo_id;
      vbo_gen_buffers(1, &vbo_id );

      //    std::cerr << "ocdr::create_vbo_diamond_patch vbo id " << vbo_id << std::endl;
      if (vbo_id != 0) {
        ++stat_frame_vbo_creation_count_;
        --stat_frame_vbo_reuse_count_; // To undo the increment of vbo_render

        create_vbo_patch(data_ptr, vbo_id);

        vbo_cache_[node_idx] = vbo_descriptor(vbo_id, node_idx.second, current_frame_id());

        current_vbo_cache_size_ += sz;
        return vbo_cache_.find(node_idx);
      } else {
        SL_TRACE_OUT(-1) << " unable to build new vbo" << std::endl;
      }
    } 
    return vbo_cache_.end();
  }

  void opengl_cached_data_renderer::create_vbo_patch(const diamond_patch_accessor_t* data_ptr, 
						     GLuint vbo_id) {
    const bool is_spherical = !geo_xform_->is_planar();
    if (elevation_map_enabled_ && is_spherical) {
      // sperical data needs texture coordinates for the elevation map
      const std::vector<int32_t>& dpv = data_ptr->vertices()->values();
      const float inv_delta_h = max_h_ > min_h_ ? 1.0f / (max_h_ - min_h_) : 1.0f;
      const int N = int(height_patch_dim_);
      int count = 0;
      for(int y = 0; y <= N; ++y) {
        for(int x = 0; x <= N - y; ++x) {
          tmp_vbo_spherical_tex_coordinates_[count] =  (dpv[count] - min_h_) * inv_delta_h;
	  ++count;
        }
      }
    }
        
    if (vbo_id != 0 ) {

	uint32_t offset = 0;
	vbo_bind_buffer(vbo_id);
	vbo_buffer_data(patch_size(), NULL, GL_STATIC_DRAW );
	vbo_buffer_sub_data(0, data_size_, data_ptr->vertices()->gl_points()[0].to_pointer()); // &(tmp_vbo_point_vector_[0]));
	offset += data_size_;
	if (use_normal_) {
	  vbo_buffer_sub_data(offset, normal_size_, data_ptr->vertices()->gl_normals()[0].to_pointer()); //tmp_vbo_normal_vector_[0]));
	  offset += normal_size_;
	}
	
	if (elevation_map_enabled_ && is_spherical) {
	  vbo_buffer_sub_data(offset, spherical_tex_size_, &(tmp_vbo_spherical_tex_coordinates_[0]));
	}
	vbo_bind_buffer(0);
    }
  }

  void opengl_cached_data_renderer::bind_texture(const diamond_patch_accessor_t* x) {
    texture_manager_.bind_texture(x, current_frame_id_);
  }

 
  void opengl_cached_data_renderer::vbo_render(const vbo_descriptor& vbo_desc) {
    if (draw_enabled_) {
      //      std::cerr << "ocdr::vbo_render: VBO-bind: " << vbo_desc.id() << std::endl;

      if (patch_color_enabled_) {
        srand(vbo_desc.id());
        glColor3f((float)(rand()%255)/255.0f, (float)(rand()%255)/255.0f, (float)(rand()%255)/255.0f);
      }

      // set vertex pointers
      vbo_bind_buffer(vbo_desc.id());
      glVertexPointer(3, GL_FLOAT, 0, (const GLvoid*)0);
      if (use_normal_) {
        glNormalPointer(GL_FLOAT, 0, reinterpret_cast<const GLvoid*>(static_cast<uintptr_t>(data_size_)));
      }
      if (!geo_xform_->is_planar() && elevation_map_enabled_) {
	uintptr_t offset = static_cast<uintptr_t>(data_size_) + (use_normal_? normal_size_ : 0);
        glTexCoordPointer(1, GL_FLOAT, 0,  reinterpret_cast<const GLvoid*>(offset));
      }
      
      if (use_color_) {
        // set texture coordinates
        if (vbo_desc.patch_id() == 0) {
	  vbo_bind_buffer(vbo_id_uv_coordinates_patch0_);
        } else {
	  vbo_bind_buffer(vbo_id_uv_coordinates_patch1_);
        }
        glTexCoordPointer(2, GL_FLOAT, 0,  (const GLvoid*)0);
      } 

      vbo_bind_element_buffer(vbo_id_indices_);
      glDrawElements(GL_TRIANGLE_STRIP, number_of_indices_, GL_UNSIGNED_SHORT, (const GLvoid*)0);

      vbo_bind_element_buffer(0);
      vbo_bind_buffer(0);
    }
    // update stats
    ++stat_frame_vbo_reuse_count_;
    stat_rendered_triangles_ += patch_triangle_count_;
  }
  
  void opengl_cached_data_renderer::direct_render(const diamond_patch_accessor_t* data_ptr, const diamond_patch_id_t& node_idx) {
    //    std::cerr << "direct_render " << node_idx.first << ", " << node_idx.second << std::endl;
    //std::cerr << "RENDER BASIC" << std::endl;
    if (draw_enabled_) {
	// deactivate vbo 
	vbo_bind_buffer(0);
	vbo_bind_element_buffer(0);

      // needed only to fill texture coords for spherical datasets
      create_vbo_patch(data_ptr, 0);

      if (patch_color_enabled_) {
        srand(node_idx.first[0] + node_idx.first[1] + node_idx.first[2] + node_idx.second);
        glColor3f((float)(rand()%255)/255.0f, (float)(rand()%255)/255.0f, (float)(rand()%255)/255.0f);
      }

      // set vertex pointers
      if (use_color_) {
        // set texture coordinates
        if (node_idx.second == 0) {
          glTexCoordPointer(2, GL_FLOAT, 0, (const GLvoid*)(&(uv_coordinates_patch0_[0])));
        } else {
          glTexCoordPointer(2, GL_FLOAT, 0, (const GLvoid*)(&(uv_coordinates_patch1_[0])));
        }
      }

      glVertexPointer(3, GL_FLOAT, 0, (const GLvoid*)data_ptr->vertices()->gl_points()[0].to_pointer());
      if (use_normal_) {
        glNormalPointer(GL_FLOAT, 0, (const GLvoid*)data_ptr->vertices()->gl_normals()[0].to_pointer()); 
      }
      if (!geo_xform_->is_planar() && elevation_map_enabled_) {
	glTexCoordPointer(1, GL_FLOAT, 0, (const GLvoid*)(&(tmp_vbo_spherical_tex_coordinates_[0])));
      }

      //std::cerr << "DRAW ELEMENTS" << std::endl;
      glDrawElements(GL_TRIANGLE_STRIP, indices_stitched_strip_.size(), GL_UNSIGNED_SHORT, (const GLvoid*)(&(indices_stitched_strip_[0])));
      //std::cerr << "DRAW ELEMENTS: Done" << std::endl;
    }

    // update stats
    ++stat_frame_direct_render_count_;
    stat_rendered_triangles_ += patch_triangle_count_;
  }

  /////////// state toggles //////////////////////////////////////
  
  void opengl_cached_data_renderer::set_use_color(bool x) {
    use_color_ = x;
    if (use_color_ && !color_available_) {
      std::cerr << "color data not available\n";
      use_color_ = false;
    }
    if (use_color_ && elevation_map_enabled_) {
      elevation_map_enabled_ = false;
    }
    
    if (!use_color_ && !use_normal_) {
      // use normal when there is no color
      set_use_normal(true);
    }
  }
  
  bool opengl_cached_data_renderer::use_color() const {
    return use_color_;
  }

  void opengl_cached_data_renderer::set_use_normal(bool x) {
    vbo_cache_clear();
    use_normal_ = x;
  }
  
  bool opengl_cached_data_renderer::use_normal() const {
    return use_normal_;
  }

  void opengl_cached_data_renderer::set_elevation_map_enabled(bool x) {
    elevation_map_enabled_ = x;
    if (elevation_map_enabled_) {
      use_color_ = false;
    }
    if (!geo_xform_->is_planar()) {
      // clear cache because spherical dataset vbos are different 
      // depending on elevation_map_enabled_
      vbo_cache_clear();
    }
  }

  bool opengl_cached_data_renderer::elevation_map_enabled() const {
    return elevation_map_enabled_;
  }

  void opengl_cached_data_renderer::clear_texture_cache() {
    texture_manager_.clear();
  }

  void opengl_cached_data_renderer::draw_begin() {
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glEnable(GL_DEPTH_TEST);
    glShadeModel(GL_SMOOTH);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glEnable(GL_CULL_FACE);
    glFrontFace(GL_CW);
    
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE); 

    if (current_pass_ == PASS_WIREFRAME) {
      glDisable(GL_POLYGON_OFFSET_FILL); 
      glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    } else {
      glColor3f( 1.0, 1.0, 1.0 );
      glDisable(GL_POLYGON_OFFSET_FILL);
    }

    bool elev_enabled = elevation_map_enabled();
    const point4_t surface_color = 
      (elev_enabled || use_color()) ? point4_t(1.0f,1.0f,1.0f, 1.0f) : point4_t(0.87f, 0.73f, 0.58f, 1.0f);

    if (current_pass_ == PASS_WIREFRAME) {
      static const point4_t line_color(0.0f, 0.0f, 0.0f, 1.0f);
      frame_initialize(line_color);
    } else {
      frame_initialize(surface_color);
    }

    glMatrixMode(GL_TEXTURE);
    glPushMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();

    glGetFloatv(GL_MODELVIEW_MATRIX, base_modelview_.to_pointer());
  }

  void opengl_cached_data_renderer::draw_end() {
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glMatrixMode(GL_TEXTURE);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glPopAttrib();
  }

  void opengl_cached_data_renderer::set_rendering_pass(rendering_pass_t pass) {
    current_pass_ = pass;
  }

  void opengl_cached_data_renderer::set_matrices(const sl::projective_map3d& P, 
                                                 const sl::rigid_body_map3d& V, 
                                                 const point3d_t& center) {
    current_projection_ = P;
    current_view_ = V;
    current_center_ = center;
  }

  void opengl_cached_data_renderer::draw_bounding_volume(const bounding_volume_t& bvol, const point3d_t& center) const {
    const point3d_t p000 = bvol.box_space_corner(0);
    const point3d_t p111 = bvol.box_space_corner(7);
    glPushMatrix();

    glTranslated(-center[0], -center[1], -center[2]);
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
      glNormal3f( 1,0,0 );
      glVertex3f(p111[0], p000[1], p000[2]);
      glVertex3f(p111[0], p111[1], p000[2]);
      glVertex3f(p111[0], p111[1], p111[2]);
      glVertex3f(p111[0], p000[1], p111[2]);
      glEnd();

      glBegin(GL_LINE_LOOP);
      glNormal3f( 0,0,1 );
      glVertex3f(p111[0], p000[1], p111[2]);
      glVertex3f(p111[0], p111[1], p111[2]);
      glVertex3f(p000[0], p111[1], p111[2]);
      glVertex3f(p000[0], p000[1], p111[2]);
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
      glVertex3f(p000[0], p111[1], p000[2]);
      glVertex3f(p000[0], p111[1], p111[2]);
      glVertex3f(p000[0], p000[1], p111[2]);
      glEnd();
    }
    glPopMatrix();
  }

  void opengl_cached_data_renderer::draw_progress_bar(float progress) {
    GLint xywh[4];
    glGetIntegerv(GL_VIEWPORT, xywh);
    float l = 128;
    float x_offset = 128+16;
    float y_offset = 24;
    float x_min = xywh[2] - x_offset;
    float y_min = xywh[3] - y_offset;
    float x_max = x_min + l;
    float y_max = y_min + l/8.0f;     
    
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, xywh[2], 0, xywh[3]);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glPushAttrib(GL_ENABLE_BIT);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // draw border
    glColor4f(0.8f, 0.8f, 0.8f, 0.5f);
    glLineWidth(2);
    glBegin(GL_LINE_STRIP);
    glVertex2f(x_min, y_min);    
    glVertex2f(x_min, y_max);
    glVertex2f(x_max, y_max);    
    glVertex2f(x_max, y_min);    
    glVertex2f(x_min, y_min);
    glEnd();
    glLineWidth(1);

    // draw progress quad
    const float border = 3;
    x_min += border;
    y_min += border;
    y_max -= border;
    x_max = x_min + ((l - 2*border) * progress);

    const uint8_t v0 = 130;
    const uint8_t v1 = 180;
    const uint8_t v2 = 200;
    const uint8_t v3 = 255;
    const uint8_t data[] = {v1, v1, v1, 255,
			    v2, v2, v2, 255,
			    v3, v3, v3, 255,
			    v3, v3, v3, 255,
			    v2, v2, v2, 255,
			    v1, v1, v1, 255,
			    v0, v0, v0, 255,
			    v0, v0, v0, 255};
			    
    glEnable(GL_TEXTURE_1D);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S,GL_CLAMP);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_T,GL_CLAMP);
    glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA, 
		 8, 0, GL_RGBA, GL_UNSIGNED_BYTE,
		 data);

    glColor4f(1.0f, 1.0f, 1.0f, 0.7f);
    glBegin(GL_QUADS);
    glTexCoord1f(1.0f);    glVertex2f(x_min, y_min);    
    glTexCoord1f(0.0f);    glVertex2f(x_min, y_max);
    glTexCoord1f(0.0f);    glVertex2f(x_max, y_max);    
    glTexCoord1f(1.0f);    glVertex2f(x_max, y_min);    
    glEnd();

    glPopAttrib();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
  }

}
