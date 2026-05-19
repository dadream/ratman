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
#ifndef CBDAM_DELTA_HEIGHT_CODEC_HPP
#define CBDAM_DELTA_HEIGHT_CODEC_HPP

#include <vic/cbdam/base/delta_codec.hpp>
#include <vic/cbdam/base/grid_diamond.hpp>
#include <vic/cbdam/base/diamond_vertices.hpp>
#include <vic/cbdam/base/coordinate_transform.hpp>

namespace cbdam {
  
  /**
   * T             : type of data encoded:              int / color
   * DIAMOND_DATA_T: type of data stored in diamond     point3_t / color
   * OPERATOR_T    : perform operations on data         height_operator / color_operator
   */
  class delta_height_codec : public delta_codec<height_operator, diamond_vertices> {
  public:
    typedef height_operator                             operator_t;
    typedef diamond_vertices                            diamond_data_t;
    typedef diamond_data_t::point3_t                    point3_t;
    typedef diamond_data_t::normal_t                    normal_t;
    typedef delta_codec<operator_t, diamond_vertices>   super_t;
    typedef grid_point_t                                diamond_id_t;
    typedef grid_diamond                                grid_diamond_t;
    typedef operator_t::value_t                         value_t;
    typedef sl::dense_array<value_t,2,void>             array2_t;
    
  protected:
    point3d_t*                  matrix_points_;
    normal_t*                   matrix_normals_;
    double                      scale_factor_;
    const coordinate_transform* geo_xform_;
    
  public:
    delta_height_codec();
    
    virtual ~delta_height_codec();

    virtual void init(int patch_dim, double scale_factor, const coordinate_transform* geo_xform_);

    virtual void distribute_data_to_root(const array2_t& offset, const grid_diamond_t& r,
                                         diamond_data_t* root0, diamond_data_t* root1);
                                 
    virtual void decode_values(const array2_t& offset,
                               const grid_diamond_t& r,
                               const diamond_data_t* d0,
                               const diamond_data_t* d1);

    void compute_patch_3dpoints(const grid_diamond_t& child_d, int patch_id, int child_i, int child_j);

    void fill_matrix_normal();

    void distribute_data_to_child(const diamond_id_t& id, diamond_data_t* dd, int child_i, int child_j);
  };


} // namespace cbdam 

#endif // CBDAM_DELTA_HEIGHT_CODEC_HPP

#ifndef CBDAM_DELTA_HEIGHT_CODEC_IPP
#define CBDAM_DELTA_HEIGHT_CODEC_IPP

namespace cbdam {

} // namespace cbdam 

#endif // CBDAM_DELTA_HEIGHT_CODEC_IPP
