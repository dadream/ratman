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
#ifndef CBDAM_TEXTURE_MANAGER_HPP
#define CBDAM_TEXTURE_MANAGER_HPP

#include <vic/cbdam/base/config.hpp>
#include <vic/cbdam/base/grid_diamond.hpp>
#include <vic/cbdam/base/grid_diamond_state.hpp>
#include <vic/cbdam/base/diamond_patch_accessor.hpp>
#include <GL/glew.h>

namespace cbdam {
  

  class  texture_descriptor {
  public:
    typedef diamond_patch_accessor::reference_counted_image_t	reference_counted_image_t;

  protected:
    GLuint id_;
    uint64_t time_stamp_;
    uint32_t last_access_frame_id_;

  public:
    texture_descriptor(GLuint id = 0, uint64_t time_stamp = 0, uint32_t last_access_frame_id = 0) :
      id_(id), time_stamp_(time_stamp), last_access_frame_id_(last_access_frame_id) {

    }

    CBDAM_RW_ACCESSOR(GLuint, id);
    CBDAM_RW_ACCESSOR(uint64_t, time_stamp);
    CBDAM_RW_ACCESSOR(uint32_t, last_access_frame_id);

    bool texture_up_to_date(const reference_counted_image_t* img) {
      return (time_stamp_ == img->global_time_stamp());
    }
  };

  /**
   *
   */
  class texture_manager {
    typedef diamond_patch_accessor				diamond_patch_accessor_t;    
    typedef std::pair<grid_point_t, int>			diamond_patch_id_t;
    typedef std::map<grid_point_t, texture_descriptor>		cache_t;
    typedef sl::fixed_size_point<4, uint8_t >			color4_t;
    typedef diamond_patch_accessor::reference_counted_image_t	reference_counted_image_t;
 
  protected:
    cache_t  cache_;
    uint32_t cache_capacity_;
    uint32_t current_cache_size_;
    uint32_t tile_width_;
    uint32_t texture_size_;

  public:
    texture_manager();

    ~texture_manager();

    void bind_texture(const diamond_patch_accessor_t* d, uint32_t current_frame);

    texture_descriptor get_texture_descriptor(const grid_point_t& id) const;

    void erase_texture(const grid_point_t& lxy_id);

    void set_tile_width(uint32_t x);

    uint32_t tile_width() const;

    void set_cache_capacity(uint32_t x_bytes);

    void cleanup(uint32_t current_frame);

    void clear();

    bool is_full() const;

    uint32_t current_cache_size() const;

    uint32_t cache_capacity() const;

  protected:

    void fill_texture(texture_descriptor& td, const reference_counted_image_t* ts_image);  
  };


} // namespace cbdam 

#endif // CBDAM_TEXTURE_MANAGER_HPP

#ifndef CBDAM_TEXTURE_MANAGER_IPP
#define CBDAM_TEXTURE_MANAGER_IPP

namespace cbdam {

  inline uint32_t texture_manager::current_cache_size() const {
    return current_cache_size_;
  }
   
  inline uint32_t texture_manager::cache_capacity() const {
    return cache_capacity_;
  }

  inline texture_descriptor texture_manager::get_texture_descriptor(const grid_point_t& id) const {
    cache_t::const_iterator it = cache_.find(id);
    if (it == cache_.end()) {
      return texture_descriptor(0,0,0);
    } else {
      return it->second;
    }
  }

  inline bool texture_manager::is_full() const {
    return current_cache_size_ + texture_size_ > cache_capacity_;
  }

  inline uint32_t texture_manager::tile_width() const {
    return tile_width_;
  }
} // namespace cbdam 

#endif // CBDAM_TEXTURE_MANAGER_IPP
