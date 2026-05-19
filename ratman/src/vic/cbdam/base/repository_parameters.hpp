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
#ifndef CBDAM_REPOSITORY_PARAMETERS_HPP
#define CBDAM_REPOSITORY_PARAMETERS_HPP

#include <vic/cbdam/base/config.hpp>
#include <vic/cbdam/base/grid_point.hpp>
#include <vic/cbdam/base/coordinate_transform.hpp>
#include <vic/xml/document.hpp>

namespace cbdam {
  
  /**
   *
   */
  class repository_parameters {
  public:
    typedef grid_point_t        diamond_id_t;
    
  protected:
    uint32_t    patch_dim_;
    double	height_scale_factor_;
    coordinate_transform* geo_xform_;
    mutable bool last_operation_success_;
    std::string srs_;
    std::string about_;

  public:
    repository_parameters();

    repository_parameters& operator=(const repository_parameters& rhs);

    repository_parameters(const repository_parameters& rhs);

    repository_parameters(uint32_t patch_dim,
                          double height_scale_factor, 
			  const coordinate_transform* geo_xform,
			  const std::string& srs,
			  const std::string& about);

    ~repository_parameters();

    void read_from_file(const char* file_name, bool print = false);
    
    void write_to_file(const char* file_name, bool print = false) const;
    
    void set_coordinate_transform(const coordinate_transform* geo_xform);

    const coordinate_transform* get_coordinate_transform() const;

    bool is_planar() const;

    void print_parameters() const;

    bool last_operation_success() const;
    
    CBDAM_RW_ACCESSOR(uint32_t, patch_dim);
    CBDAM_RW_ACCESSOR(double,	height_scale_factor);
    CBDAM_RW_ACCESSOR(std::string, srs);
    CBDAM_RW_ACCESSOR(std::string, about);
    
  protected:
    void traverse_cbdam_xml(vic::xml::node_iterator ptr,std::string space, bool print);

    void read_fail(vic::xml::node_iterator ptr,std::string space);

    void read_coordinate_transform(vic::xml::node_iterator ptr,std::string space, bool print);

    void read_double_in(double& x, const char* tag, vic::xml::node_iterator ptr,std::string space, bool print);

    void read_int_in(int& x, const char* tag, vic::xml::node_iterator ptr,std::string space, bool print);
 
    void read_string_in(std::string& x, const char* tag, vic::xml::node_iterator ptr, std::string space, bool print);
  };


} // namespace cbdam 

#endif // CBDAM_REPOSITORY_PARAMETERS_HPP

#ifndef CBDAM_REPOSITORY_PARAMETERS_IPP
#define CBDAM_REPOSITORY_PARAMETERS_IPP

namespace cbdam {

  inline bool repository_parameters::last_operation_success() const {
    return last_operation_success_;
  }

  inline bool repository_parameters::is_planar() const {
    return geo_xform_->is_planar();
  }

  inline const coordinate_transform* repository_parameters::get_coordinate_transform() const {
    return geo_xform_;
  }

} // namespace cbdam 

#endif // CBDAM_REPOSITORY_PARAMETERS_IPP
