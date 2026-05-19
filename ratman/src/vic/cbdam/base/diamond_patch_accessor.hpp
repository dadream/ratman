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
#ifndef CBDAM_DIAMOND_PATCH_ACCESSOR_HPP
#define CBDAM_DIAMOND_PATCH_ACCESSOR_HPP

#include <vic/cbdam/base/reference_counted_cache.hpp>
#include <vic/cbdam/base/grid_diamond.hpp>
#include <vic/cbdam/base/diamond_vertices.hpp>
#include <vic/cbdam/base/grid_diamond_state.hpp>
#include <vic/img/gl_image.hpp>
#include <sl/fixed_size_point.hpp>
#include <vector>

namespace cbdam {

  /// used to exchange data with opengl_cached_data_renderer
  class diamond_patch_accessor {
  public:
    typedef grid_diamond                                grid_diamond_t;
    typedef grid_diamond_render_state                   grid_diamond_state_t;
    typedef vic::img::gl_image<>                        image_rgba_t;
    typedef reference_counted_object<image_rgba_t >     reference_counted_image_t;
    typedef diamond_vertices                            diamond_vertices_t;
    typedef sl::affine_map3d                            affine_map_t;
    
  protected:
    grid_diamond_state_t        diamond_state_;
    std::size_t			patch_id_;
    bool                        visible_;
    const reference_counted_image_t* texture_image_;
    grid_point_t		texture_level_xy_;
    affine_map_t                texture_matrix_;
       
  public:
    diamond_patch_accessor(const grid_diamond_state_t& ds,
			   std::size_t patch_id,
                           bool visible,
                           const reference_counted_image_t* ti,
			   const grid_point_t& texture_level_xy,
			   const affine_map_t& tm) :
        diamond_state_(ds), patch_id_(patch_id), visible_(visible), 
	texture_image_(ti), texture_level_xy_(texture_level_xy), texture_matrix_(tm) {

      // reference all reference counted data (diamond state has reference counted data)
      if (diamond_state_.reference_counted_patch_data(patch_id_)) diamond_state_.reference_counted_patch_data(patch_id_)->ref();
      if (texture_image_) texture_image_->ref();
    }

    ~diamond_patch_accessor() {
      if (diamond_state_.reference_counted_patch_data(patch_id_)) diamond_state_.reference_counted_patch_data(patch_id_)->deref();
      if (texture_image_) texture_image_->deref();
    }

    // FIXME ray caster

    const grid_diamond_state_t& diamond_state() const {
      return diamond_state_;
    }

    const diamond_vertices_t* vertices() const {
      return diamond_state_.reference_counted_patch_data(patch_id_);
    }

    const reference_counted_image_t* texture_image() const {
      return texture_image_;
    }

    const affine_map_t& texture_matrix() const {
      return texture_matrix_;
    }
    
    const grid_point_t& texture_level_xy() const {
      return texture_level_xy_;
    }

    std::size_t patch_id() const {
      return patch_id_;
    }

    bool visible() const {
      return visible_;
    }
 };
  
} // namespace cbdam 

#endif // CBDAM_DIAMOND_PATCH_ACCESSOR_HPP

#ifndef CBDAM_DIAMOND_PATCH_ACCESSOR_IPP
#define CBDAM_DIAMOND_PATCH_ACCESSOR_IPP

namespace cbdam {

} // namespace cbdam 

#endif // CBDAM_DIAMOND_PATCH_ACCESSOR_IPP
