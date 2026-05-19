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
#ifndef CBDAM_TERRAIN_SCENE_COMPILER_HPP
#define CBDAM_TERRAIN_SCENE_COMPILER_HPP

#include <vic/cbdam/base/config.hpp>
#include <vic/cbdam/base/color_rgb.hpp>
#include <vic/cbdam/geo/map_external_sampler.hpp>

namespace cbdam {

  /**
   * Encapsulate external samplers, one for height, and one for colors, with interface
   * in sync with opossum interfaces, and builds height and color multiresolution structures.
   * Needed for external use.
   */
  class terrain_scene_compiler {
  protected:
    vic::geo::map_height_int32_external_sampler	height_sampler_;
    vic::geo::map_rgb_int16_8_external_sampler	color_sampler_;
    uint32_t patch_dim_;
    uint32_t subsampling_level_;
    bool color_construction_enabled_;    

  public:
    terrain_scene_compiler();

    ~terrain_scene_compiler();

    void set_extent(double u_extent, double v_extent);

    void set_target_elevation_field_sampling_rate(double ds);

    void set_target_color_field_sampling_rate(double ds);

    void set_elevation_field_sampler(void * cb_context, vic::geo::elevation_field_sampler_t cb, double height_scale_factor);

    void set_color_field_sampler(void * cb_context, vic::geo::color_field_sampler_t cb);

    void set_patch_dimension(uint32_t patch_dim);

    void set_subsampling_level(uint32_t subsampling_level);
    
    void set_color_construction_enabled(bool x);    

    void scene_begin(const char* output_base_name);

    bool empty() const;

    const vic::geo::map_height_int32_external_sampler& height_sampler() const;
    
    const vic::geo::map_rgb_int16_8_external_sampler&  color_sampler() const;
  };


} // namespace cbdam 

#endif // CBDAM_TERRAIN_SCENE_COMPILER_HPP

#ifndef CBDAM_TERRAIN_SCENE_COMPILER_IPP
#define CBDAM_TERRAIN_SCENE_COMPILER_IPP

namespace cbdam {

  inline const vic::geo::map_height_int32_external_sampler& terrain_scene_compiler::height_sampler() const {
    return height_sampler_;
  }

  inline const vic::geo::map_rgb_int16_8_external_sampler& terrain_scene_compiler::color_sampler() const {
    return color_sampler_;
  }

} // namespace cbdam 

#endif // CBDAM_TERRAIN_SCENE_COMPILER_IPP
