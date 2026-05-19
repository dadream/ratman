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
#include <vic/cbdam/base/camera_controller_flight.hpp>
#include <sl/quaternion.hpp>
#include <iostream>
#include <assert.h>

// The following is to remove macros coming from
// windows.h and getting access to template versions
#include <sl/utility.hpp> 
#undef min
#undef max

namespace cbdam {

  camera_controller_flight::camera_controller_flight(camera* camera_ptr) :
      camera_controller_base(camera_ptr) {
    reset();
  }

  camera_controller_flight::~camera_controller_flight(){

  }

  void camera_controller_flight::reset(){
    m_pitch_min = 0.0f;
    m_pitch_max = 3.141592653f;
    m_pitch = m_pitch_min;
    m_yaw = 0.0f;
    m_position = camera::vector3_t(0, 0, m_distance);
    camera_controller_base::reset();
  }

  void camera_controller_flight::set_distance(camera::value_t distance) {
    m_distance = distance;
    m_position = camera::vector3_t(0, 0, m_distance);
    update_camera_view();
  }
   
  void camera_controller_flight::reset_rotation() {
    m_yaw = 0.0f;
    m_pitch = m_pitch_min;
  }

  void camera_controller_flight::set_position(camera::vector3_t& x) {
    m_position = x;
  }

  void camera_controller_flight::idle_update() {
    // update dt: time elapsed from previous idle_update, to correctly
    // compute movements.
    camera::vector3_t projected_cursor = project_mouse( m_cursor_position );
    camera::vector3_t direction = projected_cursor - m_projected_center;
    camera::value_t dt = (camera::value_t)m_clock.elapsed().as_microseconds() * 0.0001;
    m_pitch = sl::median(m_pitch_min, m_pitch + delta_pitch(dt, direction), m_pitch_max);
    m_yaw += delta_yaw(dt, direction);
    m_position += delta_position(dt, direction);
    if (m_position[2] < 0.0001 * m_radius) {
      m_position[2] = 0.0001 * m_radius;
    }

    // update the camera
    update_camera_view();
    m_clock.restart();     
  }
  
  camera::vector3_t camera_controller_flight::delta_position(const camera::value_t& dt, const camera::vector3_t& direction) const {
    // move along z
    camera::value_t speed = sl::median(camera::value_t(0.35f), camera::value_t(3.0f) * m_position[2] / m_radius, camera::value_t(0.96));
    camera::value_t t_factor = 0.06f * dt;
    camera::vector3_t position(0.0f, 0.0f, 0.0f);
    if (m_keyboard_status & CC_K_FORWARD) {
      //      position[2] = -speed * dt;
      position[2] = -m_position[2] * speed * 0.3f* t_factor;
    } else if (m_keyboard_status & CC_K_BACKWARD) {
      //      position[2] = speed * dt;
      position[2] = m_position[2] * speed * 0.3f* t_factor;
    }

    // move on xy plane
    speed = m_position[2] * t_factor;
    if ( m_mouse_status & CC_MB_LEFT ) {
      position += (camera::linear_map_factory_t::rotation(2, m_yaw) *
                   camera::vector3_t(speed * direction[0], speed * direction[1], 0));
    }
    return position;
  }

  camera::value_t camera_controller_flight::delta_yaw(const camera::value_t& dt, const camera::vector3_t& direction) const {
    // dt is expressed as millisec
    static const camera::value_t angle_factor = 0.01;
    
    if (m_mouse_status & CC_MB_RIGHT) {
      return ( -direction[0] * angle_factor * dt );
    } else {
      return 0;
    }
  }

  camera::value_t camera_controller_flight::delta_pitch(const camera::value_t& dt, const camera::vector3_t& direction) const {
    // dt is expressed as millisec
    static const camera::value_t angle_factor = 0.01;
    if (m_mouse_status & CC_MB_RIGHT) {
      return direction[1] * angle_factor * dt;
    } else {
      return 0;
    }
  }

  camera::value_t camera_controller_flight::weight_from_height() const {
    //    return 0.005f * sl::max(m_position[2], 0.0001f * m_radius);
    return m_position[2] / m_radius;
  }

  camera::rigid_body_map_t camera_controller_flight::compute_camera_view() const {
    // Matrix : 1)translate -pos 2) rotate pitch, 3) rotate yaw
    return camera::rigid_body_map_t(camera::linear_map_factory_t::rotation(0, -m_pitch) * 
                                    camera::linear_map_factory_t::rotation(2, -m_yaw) *
                                    camera::linear_map_factory_t::translation(-m_position));
  }


} // namespace cbdam
