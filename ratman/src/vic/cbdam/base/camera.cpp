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
#include <vic/cbdam/base/camera.hpp>

namespace cbdam {

  camera::camera()
  {
    reset_view();
    set_projection( 1.0f, 1.0f,  1.0f, 100.0f); // dim, aspect, near, far
    assert( invariant() );
  }

  camera::~camera(){

  }

  void camera::reset_view(){
    m_projection_map = linear_map_factory_t::identity();
  }

  // set view frustum between [-dim,dim] [-dim/a,dim/a] [near,far] a = w / h 
  void camera::set_projection(float fovy, float aspect_ratio, float p_near, float p_far){
    assert( aspect_ratio != 0 );  // pre-condition

    //  m_projection_map = sl::linear_map_factory3d::frustum(-dim , dim, -dim/aspect_ratio, dim/aspect_ratio,  near, far);
    m_projection_map = linear_map_factory_t::perspective( fovy, aspect_ratio, p_near, p_far );

    assert( invariant() );   // post-condition
  }

} // namespace cbdam

