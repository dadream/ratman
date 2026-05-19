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
#include <vic/cbdam/base/camera_controller_base.hpp>
#include <sl/quaternion.hpp>
#include <iostream>
#include <assert.h>

// The following is to remove macros coming from
// windows.h and getting access to template versions
#include <sl/utility.hpp> 
#undef min
#undef max

namespace cbdam {

  camera_controller_base::camera_controller_base(camera* camera_ptr)
      : m_camera_ptr(camera_ptr) {
    reset();
  }
  
  camera_controller_base::~camera_controller_base(){

  }

  void camera_controller_base::reset(){
    m_radius = 1.0f;
    m_rotation_factor = 4*1E-6;
    m_distance = 10.0;
    m_projected_center = camera::vector3_t( 0, 0, 0 ); // window dimension have to be set before mouse can be projected
    m_keyboard_status = CC_K_RELEASED;
    m_mouse_status = (int)CC_MB_RELEASED;

    // init camera with a simple translation
    m_camera_ptr->set_view( camera::linear_map_factory_t::translation( camera::vector3_t( 0.0, 0.0, -m_distance ) ) );
    m_clock.restart();
  }
  
} // namespace cbdam
