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
#ifndef CBDAM_DIAMOND_REPOSITORY_HPP
#define CBDAM_DIAMOND_REPOSITORY_HPP

#include <vic/cbdam/base/config.hpp>
#include <vic/cbdam/base/grid_point.hpp>
#include <vic/cbdam/base/diamond_operator.hpp>
#include <vic/cbdam/base/repository_parameters.hpp>
#include <sl/dense_array.hpp>

namespace cbdam {
  
  /**
   *
   */
  template<class OPERATOR_T>
  class diamond_repository {
  public:
    typedef grid_point_t                        diamond_id_t;
    typedef typename OPERATOR_T::value_t        value_t;
    typedef sl::dense_array<value_t,2,void>     array2_t;
    typedef OPERATOR_T				operator_t;

  protected:

    repository_parameters       repo_params_;
    
  public:
    diamond_repository() {

    }

    virtual ~diamond_repository() {

    }

    virtual void get_data(const diamond_id_t& dm_id,
                          array2_t& offset,
                          uint32_t thread_idx = 0,
                          float patch_edge_length = 1) const = 0;

    virtual void get_root_data(const diamond_id_t& dm_id,
                               array2_t& offset, 
                               uint32_t thread_idx = 0,
                               float patch_edge_length = 1) const = 0;

    virtual bool has_root_data(const diamond_id_t& /*id*/) const {
      return true; 
    }
    
    virtual bool has(const diamond_id_t& /*id*/) const {
      return true;
    }

  public:

    /// The number of samples per patch side (pow2)
    uint32_t patch_dim() const {
      return repo_params_.patch_dim();
    }

    /// True if the repository contains planar data
    bool is_planar() const {
      return repo_params_.is_planar();
    }

#if 0
    const coordinate_transform::aabox_t& bounding_rectangle() const {
      return dynamic_cast<const planar_coordinate_transform*>(repo_params_.coordinate_transform())->bounding_rectangle();
    }

    double spherical_terrain_radius() const {
      return dynamic_cast<const spherical_coordinate_transform*>(repo_params_.coordinate_transform())->radius();
    }
#endif
    void set_coordinate_transform(const coordinate_transform* x) {
      repo_params_.set_coordinate_transform(x);
    }

    const coordinate_transform& get_coordinate_transform() const {
      return *repo_params_.get_coordinate_transform();
    }

    virtual void set_patch_dim(const uint32_t& x) {
      repo_params_.patch_dim() = x;
    }

  };


} // namespace cbdam 

#endif // CBDAM_DIAMOND_REPOSITORY_HPP

#ifndef CBDAM_DIAMOND_REPOSITORY_IPP
#define CBDAM_DIAMOND_REPOSITORY_IPP

namespace cbdam {

} // namespace cbdam 

#endif // CBDAM_DIAMOND_REPOSITORY_IPP
