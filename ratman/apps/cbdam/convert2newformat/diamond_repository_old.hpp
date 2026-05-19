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
#ifndef CBDAM_DIAMOND_REPOSITORY_OLD_HPP
#define CBDAM_DIAMOND_REPOSITORY_OLD_HPP

#include <vic/cbdam/base/config.hpp>
#include <vic/cbdam/base/grid_point.hpp>
#include <vic/cbdam/base/value_operator.hpp>
#include <sl/dense_array.hpp>

namespace cbdam {
  
  /**
   *
   */
  template<class OPERATOR_T>
  class diamond_repository_old {
  public:
    typedef grid_point_t                        diamond_id_t;
    typedef typename OPERATOR_T::value_t        value_t;
    typedef typename OPERATOR_T::delta_t        delta_t;
    typedef typename OPERATOR_T::diamond_data_t diamond_data_t;
    typedef sl::dense_array<value_t,2,void>     array2_t;
    typedef sl::dense_array<delta_t,2,void>     array2_delta_t;

  protected:
    OPERATOR_T          value_operator_;
    uint32_t		patch_dim_;
    bool		is_planar_;
    double		planar_terrain_root_side_length_;
    double		spherical_terrain_radius_;
    float		wavelet_alpha_;
    
  public:
    diamond_repository_old() {
      patch_dim_ = 0;
      is_planar_ = true;
      planar_terrain_root_side_length_ = 0;
      spherical_terrain_radius_ = 0;
      wavelet_alpha_ = 0.5f;
    }

    virtual ~diamond_repository_old() {

    }

    virtual void get_offsets(const diamond_id_t& dm_id,
                             float patch_edge_length, 
                             array2_delta_t& offset_control,
                             array2_delta_t& offset_new, 
			     uint32_t thread_idx = 0) const = 0;

    virtual void get_root_offsets(const diamond_id_t& dm_id,
                                  float patch_edge_length,
                                  array2_delta_t& offset, 
				  uint32_t thread_idx = 0) const = 0;

    virtual bool has_root_patches(const diamond_id_t& /*id*/) const {
      return true; 
    }
    
    virtual bool has(const diamond_id_t& /*id*/) const {
      return true;
    }

    virtual uint32_t compressed_diamond_size(const diamond_id_t& /*id*/) const {
      return 0;
    }

  public:

    /// The number of samples per patch side (pow2)
    uint32_t patch_dim() const {
      return patch_dim_;
    }

    /// True if the repository contains planar data
    bool is_planar() const {
      return is_planar_;
    }

    double planar_terrain_root_side_length() const {
      return planar_terrain_root_side_length_;
    }

    double spherical_terrain_radius() const {
      return spherical_terrain_radius_;
    }
    
    virtual float wavelet_alpha() const {
      return wavelet_alpha_;
    }

    virtual void set_patch_dim(const uint32_t& x) {
      patch_dim_ = x;
    }

    virtual void set_planar_with_side_length(double x) {
      is_planar_ = true;
      planar_terrain_root_side_length_ = x;
    }

    virtual void set_spherical_with_radius(double x) {
      is_planar_ = false;
      spherical_terrain_radius_ = x;
    }

    virtual void set_wavelet_alpha(const float& x) {
      wavelet_alpha_ = x;
    }
  };


} // namespace cbdam 

#endif // CBDAM_DIAMOND_REPOSITORY_OLD_HPP

#ifndef CBDAM_DIAMOND_REPOSITORY_OLD_IPP
#define CBDAM_DIAMOND_REPOSITORY_OLD_IPP

namespace cbdam {

} // namespace cbdam 

#endif // CBDAM_DIAMOND_REPOSITORY_OLD_IPP
