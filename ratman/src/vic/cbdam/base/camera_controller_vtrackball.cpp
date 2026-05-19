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
#include <vic/cbdam/base/camera_controller_vtrackball.hpp>
#include <sl/quaternion.hpp>
#include <iostream>
#include <assert.h>

// The following is to remove macros coming from
// windows.h and getting access to template versions
#include <sl/utility.hpp> 
#undef min
#undef max

namespace cbdam {

  camera_controller_vtrackball::camera_controller_vtrackball(camera* camera_ptr)
      : camera_controller_base(camera_ptr) {
    reset();
  }

  camera_controller_vtrackball::~camera_controller_vtrackball(){

  }

  void camera_controller_vtrackball::reset(){
    camera_controller_base::reset();

    m_rotation_factor = 4*1E-6 / 256; // FIXME
    
    m_acceleration = 0.005f;
    m_minimum_rotation_angle = 0.0f;
    m_tilt_angle = 0.0f; // when tilt is 0 user looks at the center of the sphere, when is 90 looks tangent.
    m_tilt_factor = 0.01f;
    m_tilt_angle_bound = -3.141592653f/2.0f;
    m_rotation_matrix = camera::linear_map_factory_t::identity();
  }

  void camera_controller_vtrackball::reset_rotation() {
    m_rotation_matrix = camera::linear_map_factory_t::identity();
    m_tilt_angle = 0.0f;
  }
  
  void camera_controller_vtrackball::idle_update() {
    // update dt: time elapsed from previous idle_update, to correctly
    // compute movements.
    camera::value_t dt = (camera::value_t)m_clock.elapsed().as_microseconds() * 0.0001;
    
    m_distance = next_distance( dt );
    m_rotation_matrix = next_rotation_matrix( dt );
    m_tilt_angle = next_tilt_angle( dt );
    
    // update the camera
    update_camera_view();
    m_clock.restart();     
  }

  void camera_controller_vtrackball::predict_position(camera* cam, const camera::value_t& dt_microseconds) const {
    camera::value_t dt = dt_microseconds * 0.0001;
    camera::value_t distance = next_distance( dt );
    camera::rigid_body_map_t rotation_matrix = next_rotation_matrix( dt );
    camera::value_t tilt_angle = next_tilt_angle( dt );

    // compute new camera view, relative to predicted position and set on cam
    cam->set_view( compute_camera_view( distance, tilt_angle, rotation_matrix ) );   
  }

  camera::rigid_body_map_t camera_controller_vtrackball::next_rotation_matrix(const camera::value_t & dt) const {
    camera::rigid_body_map_t next_rotation = m_rotation_matrix; 

    if ( m_keyboard_status & CC_K_LEFT ) {
      camera::value_t rad_rotation_angle = -dt * m_tilt_factor;
      // premultiplicate old rotation matrix with this new rotation
      next_rotation = camera::linear_map_factory_t::rotation( 2, rad_rotation_angle ) * next_rotation;
    } else if ( m_keyboard_status & CC_K_RIGHT ) {
      camera::value_t rad_rotation_angle = dt * m_tilt_factor;
      // premultiplicate old rotation matrix with this new rotation
      next_rotation = camera::linear_map_factory_t::rotation( 2, rad_rotation_angle ) * next_rotation;
    } 

    // manage mouse which can cohexist
    if ( m_mouse_status & CC_MB_LEFT ) {
      // perform trackball
      // project cursor into a unit sphere centered at the center of the window
      camera::vector3_t projected_cursor = project_mouse( m_cursor_position );

      // get direction from the projection of the point where click happened to the projected cursor
      camera::vector3_t direction = projected_cursor - m_projected_center;

      // compute a scaled angle proportional to the distance of the two points
      camera::value_t rad_rotation_angle = direction.two_norm() * m_rotation_factor * ( m_distance * 1.01 - m_radius ) * dt;
      if ( rad_rotation_angle > m_minimum_rotation_angle ) {
	// angle big enough to compute rotation, to skip rotation where direction isnot well defined

	// rot axis is orthogonal to the plane defined by the two previous vectord
	camera::vector3_t rotation_axis = ( projected_cursor.cross( m_projected_center ) ).ok_normalized();

	// compute rotation matrix on that vector using quaternions
	sl::quaternion<camera::value_t> rotation_about_axis;
	rotation_about_axis.from_axis_angle( rotation_axis, rad_rotation_angle );

	// premultiplicate old rotation matrix with this new rotation
	next_rotation = ( camera::rigid_body_map_t( rotation_about_axis, camera::vector3_t( 0.0, 0.0, 0.0 ) ) * 
                          next_rotation );
      }
    } 
    return next_rotation;
  }
  
  camera::value_t camera_controller_vtrackball::next_tilt_angle(const camera::value_t& dt) const {
    // dt is expressed as millisec
    // tilt on point of intersection between sphere and ray camera-center
    if ( m_mouse_status & CC_MB_RIGHT ) {
      camera::vector3_t projected_cursor = project_mouse( m_cursor_position );
    
      // get direction from the projection of the point where click happened to the projected cursor
      camera::vector3_t direction = projected_cursor - m_projected_center;
    
      // get distance along y to convert it into a rotation angle
      camera::value_t next_tilt_angle = m_tilt_angle + ( -direction[ 1 ] * m_tilt_factor * dt );
      
      // limit tilt to -89.0 deg
      if ( next_tilt_angle <  m_tilt_angle_bound )  
	next_tilt_angle =  m_tilt_angle_bound;
      else if (  next_tilt_angle > 0 )
	next_tilt_angle = 0;
      return next_tilt_angle;
    } else {
      return m_tilt_angle;
    }
    
  }

  camera::value_t camera_controller_vtrackball::next_distance(const camera::value_t& dt) const {
    camera::value_t dist_percent =  sl::min((float)(fabs( m_distance - m_radius + 1 ) / m_radius) , 4.0f );
    camera::value_t acceleration = dist_percent * m_acceleration;
    camera::value_t next_distance = m_distance;
    // manage keyboard
    if ( m_keyboard_status & CC_K_FORWARD ) {
      // decrease distance from planet center
      next_distance *= (1-acceleration * dt);
      //      if ( m_distance  < m_radius ) m_distance = m_radius;
    } else if ( m_keyboard_status & CC_K_BACKWARD ) {
      // increase distance from planet center
      next_distance *= (1+acceleration * dt);
    }  
    // manage mouse which can cohexist

    return next_distance;
  }

} // namespace cbdam
