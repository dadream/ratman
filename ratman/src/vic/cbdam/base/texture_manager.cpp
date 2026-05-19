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

#include <vic/cbdam/base/texture_manager.hpp>  
#include <vic/cbdam/base/color_rgb.hpp>
#include <GL/glew.h>

namespace cbdam {

  texture_manager::texture_manager() {
    cache_capacity_ = 0;
    current_cache_size_ = 0;
    tile_width_ = 0;
    texture_size_ = 0;
  }

  texture_manager::~texture_manager() {
    clear();
  }

  void texture_manager::bind_texture(const diamond_patch_accessor_t* d, uint32_t current_frame) {
    const reference_counted_image_t* ts_image = d->texture_image();
    grid_point_t lxy_id = d->texture_level_xy();
    cache_t::iterator it = cache_.find(lxy_id);

    if (it == cache_.end()) {
      // create a new texture
      GLuint tex_id;
      glGenTextures(1, &tex_id);
      if (tex_id == 0) {
        std::cerr << "Unable to generate a new texture\n";
        return;
      }

      texture_descriptor td(tex_id, ts_image->global_time_stamp(), current_frame);
      fill_texture(td, ts_image);      
      cache_[lxy_id] = td;
      current_cache_size_ += texture_size_;
    } else if (!it->second.texture_up_to_date(ts_image)) {
      it->second.time_stamp() = ts_image->global_time_stamp();
      it->second.last_access_frame_id() = current_frame;

      // update texture
      fill_texture(it->second, ts_image);
    } else {
      it->second.last_access_frame_id() = current_frame;

      // simply bind
      glBindTexture(GL_TEXTURE_2D, it->second.id());
    }
  }

  void texture_manager::cleanup(uint32_t current_frame) {
    // uint32_t old_size = cache_.size();
    for (uint32_t texture_lifetime = 8;
         (texture_lifetime >=1) && (texture_lifetime == 8 || 2*current_cache_size() > cache_capacity());
         texture_lifetime /= 2) {
      for (cache_t::iterator texture_it = cache_.begin();
           texture_it != cache_.end() && (texture_lifetime == 8 || 2*current_cache_size() > cache_capacity());
           ) {
        cache_t::iterator texture_it_del = texture_it;
        ++texture_it;
        if (texture_it_del->second.last_access_frame_id()+texture_lifetime <= current_frame) {
          GLuint texture_id = texture_it_del->second.id();
          SL_TRACE_OUT(1) << "Cleanup: TEXTURE-recycle: " << texture_id << std::endl;
          glDeleteTextures(1, &texture_id);
          current_cache_size_ -= texture_size_;
          cache_.erase(texture_it_del);
        }
      }    
    }
    //    if (old_size != cache_.size()) {std::cerr << "Current texture cache size " << current_cache_size_ << " B, cache count " << cache_.size() << std::endl;}
  }

  void texture_manager::clear() {
    for (cache_t::iterator texture_it = cache_.begin();
         texture_it != cache_.end();
         ) {
      cache_t::iterator texture_it_del = texture_it;
      ++texture_it;
      GLuint texture_id = texture_it_del->second.id();
      SL_TRACE_OUT(1) << "Clear: TEXTURE-delete: " << texture_id << std::endl;
      glDeleteTextures(1, &texture_id);
      current_cache_size_ -= texture_size_;
      cache_.erase(texture_it_del);
    }

  }

  void texture_manager::fill_texture(texture_descriptor& td, const reference_counted_image_t* ts_image) {
    glBindTexture(GL_TEXTURE_2D, td.id());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
#if 0
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 
		 tile_width_, tile_width_, 0, GL_RGBA, GL_UNSIGNED_BYTE,
		 ts_image->object()->to_pointer());
#else
    gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA, 
		      tile_width_, tile_width_, GL_RGBA, GL_UNSIGNED_BYTE,
		      ts_image->object()->to_pointer());
#endif
  }

  void texture_manager::erase_texture(const grid_point_t& lxy_id) {
    cache_t::const_iterator it = cache_.find(lxy_id);
    if (it != cache_.end()) {
      glDeleteTextures(1, &(it->second.id()));
      cache_.erase(lxy_id);
      current_cache_size_ -= texture_size_;
    }
  }

  void texture_manager::set_tile_width(uint32_t x) {
    tile_width_ = x;
    texture_size_ = tile_width_ * tile_width_ * 4;
  }

 void texture_manager::set_cache_capacity(uint32_t x_bytes) {
    cache_capacity_ = x_bytes;
  }

}

