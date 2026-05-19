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
#ifndef CBDAM_DIAMOND_REPOSITORY_PROCEDURAL_HPP
#define CBDAM_DIAMOND_REPOSITORY_PROCEDURAL_HPP

#include <vic/cbdam/base/diamond_repository.hpp>
#include <cstdlib>

namespace cbdam {
  
  /**
   *
   */
  template<class OPERATOR_T>
  class diamond_repository_procedural : public diamond_repository<OPERATOR_T> {
    typedef grid_point_t                        diamond_id_t;
    typedef diamond_repository<OPERATOR_T>      super_t;
    typedef typename super_t::value_t           value_t;
    typedef typename super_t::diamond_data_t    diamond_data_t;
    typedef typename super_t::operator_t        operator_t;
    typedef sl::dense_array<value_t,2,void>     array2_t;
   
  public:
    diamond_repository_procedural() {

    }

    virtual ~diamond_repository_procedural() {

    }
    
    virtual void get_data(const diamond_id_t& dm_id,
			  array2_t& offset_control,
			  array2_t& offset_new, 
			  uint32_t thread_idx = 0,
			  float patch_edge_length = 1) const;

    virtual void get_root_data(const diamond_id_t& dm_id,
			       array2_t& offset, 
			       uint32_t thread_idx = 0,
			       float patch_edge_length = 1) const;
    
  protected:
  
  };


} // namespace cbdam 

#endif // CBDAM_DIAMOND_REPOSITORY_PROCEDURAL_HPP

#ifndef CBDAM_DIAMOND_REPOSITORY_PROCEDURAL_IPP
#define CBDAM_DIAMOND_REPOSITORY_PROCEDURAL_IPP

namespace cbdam {

  template<class OPERATOR_T>
  inline void diamond_repository_procedural<OPERATOR_T>::get_data(const diamond_id_t& dm_id, 
								  array2_t& offset_control,
								  array2_t& offset_new, 
								  uint32_t /*thread_idx*/,
								  float patch_edge_length) const {
    assert(this->patch_dim());
    operator_t::get_procedural_offsets(dm_id, patch_edge_length, offset_control, offset_new, this->patch_dim());
  }

  template<class OPERATOR_T>
  inline void diamond_repository_procedural<OPERATOR_T>::get_root_data(const diamond_id_t& dm_id, 
								       array2_t& offset, 
								       uint32_t /*thread_idx*/,
								       float patch_edge_length) const {
    assert(this->patch_dim());
    operator_t::get_procedural_root_offsets(dm_id, patch_edge_length, offset, this->patch_dim());
  }

} // namespace cbdam 

#endif // CBDAM_DIAMOND_REPOSITORY_PROCEDURAL_IPP
