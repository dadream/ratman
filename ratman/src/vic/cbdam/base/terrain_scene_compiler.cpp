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
#include <vic/cbdam/base/terrain_scene_compiler.hpp>
#include <vic/cbdam/base/diamond_operator.hpp>
#include <vic/cbdam/base/coordinate_transform.hpp>
#include <vic/cbdam/base/builder.hpp>

namespace cbdam {
  
  terrain_scene_compiler::terrain_scene_compiler() {
    patch_dim_ = 64;
    subsampling_level_ = 0;
    color_construction_enabled_ = false;
  }

  terrain_scene_compiler::~terrain_scene_compiler() {

  }

  void terrain_scene_compiler::set_extent(double u_extent, double v_extent) {
    height_sampler_.set_extent(u_extent, v_extent);
    color_sampler_.set_extent(u_extent, v_extent);
  }

  void terrain_scene_compiler::set_target_elevation_field_sampling_rate(double ds) {
    height_sampler_.set_target_sampling_rate(ds);
  }

  void terrain_scene_compiler::set_target_color_field_sampling_rate(double ds) {
    color_sampler_.set_target_sampling_rate(ds);    
  }

  void terrain_scene_compiler::set_elevation_field_sampler(void * cb_context,
                                                           vic::geo::elevation_field_sampler_t cb,
                                                           double height_scale_factor) {
    height_sampler_.set_callback(cb_context, cb);
    height_sampler_.set_height_scale_factor(height_scale_factor);
  }

  void terrain_scene_compiler::set_color_field_sampler(void * cb_context,
                                                       vic::geo::color_field_sampler_t cb) {
    color_sampler_.set_callback(cb_context, cb);
  }

  void terrain_scene_compiler::set_patch_dimension(uint32_t patch_dim) {
    patch_dim_ = patch_dim;
  }
  
  void terrain_scene_compiler::set_subsampling_level(uint32_t subsampling_level) {
    subsampling_level_  = subsampling_level;
  }

  void terrain_scene_compiler::set_color_construction_enabled(bool x) {
    color_construction_enabled_ = x;
  }

  void terrain_scene_compiler::scene_begin(const char* output_base_name) {
    if (!height_sampler_.is_empty()) {
      const std::string height_file_name = std::string(output_base_name) + ".height";
      std::cerr << "build " << height_file_name << std::endl;

      // height tolerance 1/20 sampling step: quite lossless compression
      planar_coordinate_transform geo_xform(height_sampler_.bounding_rectangle());

      builder<height_operator> height_builder;
      height_builder.arg_patch_dim() = patch_dim_;
      height_builder.arg_tolerance() = height_sampler_.target_sampling_rate() / 20.0f;
      height_builder.arg_use_amax_error() = false;
      height_builder.arg_data_scale_factor() = 1.0;

      height_builder.build(height_file_name,
			   &height_sampler_,
			   &geo_xform);
    }

    if (color_construction_enabled_ && !color_sampler_.is_empty()) {
      const std::string color_file_name = std::string(output_base_name) + ".color";
      std::cerr << "build " << color_file_name << std::endl;

      // color tolerance 5/255: quite lossless compression
      planar_coordinate_transform geo_xform(color_sampler_.bounding_rectangle());

      builder<color_operator> color_builder;
      color_builder.arg_patch_dim() = patch_dim_;
      color_builder.arg_tolerance() = 5;
      color_builder.arg_use_amax_error() = false;
      color_builder.build(color_file_name,
			  &color_sampler_,
			  &geo_xform);
    }
  }

  bool terrain_scene_compiler::empty() const {
    return height_sampler_.is_empty() && color_sampler_.is_empty();
  }

}
