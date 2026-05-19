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
#ifndef CBDAM_REPOSITORY_PARAMETERS_OLD_HPP
#define CBDAM_REPOSITORY_PARAMETERS_OLD_HPP

#include <vic/cbdam/base/config.hpp>
#include <vic/cbdam/base/grid_point.hpp>

namespace cbdam {
  
  /**
   *
   */
  class repository_parameters_old {
  public:
    typedef grid_point_t        diamond_id_t;
    
  protected:
    uint32_t    patch_dim_;
    bool        is_planar_;
    bool        is_mono_scale_;
    float       wavelet_alpha_;
    double      length_;
    uint32_t    root_count_;
    std::vector<diamond_id_t> roots_;

  public:
    repository_parameters_old();

    repository_parameters_old(uint32_t patch_dim, bool is_planar, bool is_mono_scale,
                          float wavelet_alpha, double length, uint32_t root_count,
                          const std::vector<diamond_id_t>& roots);

    ~repository_parameters_old();

    void read_from_file(FILE* fp, bool print = false);
    
    void write_to_file(FILE* fp, bool print = false) const;
    
    void set_roots(const std::vector<diamond_id_t>& roots);

    void add_root(const diamond_id_t& r);
    
    const std::vector<diamond_id_t>& roots() const;

    void print_parameters() const;

    CBDAM_RW_ACCESSOR(uint32_t, patch_dim);
    CBDAM_RW_ACCESSOR(bool,     is_planar);
    CBDAM_RW_ACCESSOR(bool,     is_mono_scale);
    CBDAM_RW_ACCESSOR(float,    wavelet_alpha);
    CBDAM_RW_ACCESSOR(double,   length);
    CBDAM_RW_ACCESSOR(uint32_t, root_count);

  protected:

  };


} // namespace cbdam 

#endif // CBDAM_REPOSITORY_PARAMETERS_OLD_HPP

#ifndef CBDAM_REPOSITORY_PARAMETERS_OLD_IPP
#define CBDAM_REPOSITORY_PARAMETERS_OLD_IPP

namespace cbdam {

} // namespace cbdam 

#endif // CBDAM_REPOSITORY_PARAMETERS_OLD_IPP
