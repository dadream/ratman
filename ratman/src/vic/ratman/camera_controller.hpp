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
#ifndef RATMAN_CAMERA_CONTOLLER_HPP
#define RATMAN_CAMERA_CONTOLLER_HPP

#include <vic/ratman/ratman.hpp>
#include <vic/ratman/oriented_position.hpp>

namespace sl {
  class real_time_clock;
}

namespace ratman {

  class camera_controller {
  public:
    typedef cbdam::coordinate_transform uvh_xyz_transform_t;

    typedef unsigned int MOUSE_STATUS;
    static const MOUSE_STATUS MS_RELEASED = 0;
    static const MOUSE_STATUS MS_LEFT = 1;
    static const MOUSE_STATUS MS_MIDDLE = 2;
    static const MOUSE_STATUS MS_RIGHT = 4;

    typedef unsigned int KEYBOARD_STATUS;
    static const KEYBOARD_STATUS K_RELEASED = 0;
    static const KEYBOARD_STATUS K_ZOOMIN = 1;
    static const KEYBOARD_STATUS K_ZOOMOUT = 2;
    static const KEYBOARD_STATUS K_FORWARD = 4;
    static const KEYBOARD_STATUS K_BACKWARD = 8;
    static const KEYBOARD_STATUS K_LEFT = 16;
    static const KEYBOARD_STATUS K_RIGHT = 32; 
    static const KEYBOARD_STATUS K_TILT_UP = 64;
    static const KEYBOARD_STATUS K_TILT_DOWN = 128; 
    static const KEYBOARD_STATUS K_YAW_CW = 256;
    static const KEYBOARD_STATUS K_YAW_CCW = 512;

  protected:
    const uvh_xyz_transform_t* uvh_xyz_transform_;

    sl::real_time_clock*clock_;			//< used to compute increments independent from cpu speed
    point2d_t		mouse_previous_position_;
    point2d_t		mouse_current_position_;
    MOUSE_STATUS 	mouse_status_;		//< mouse status used by idle function 
    KEYBOARD_STATUS	keyboard_status_;	//< keyboard status used by idle function

    double		tilt_slider_factor_;
    double		zoom_slider_factor_;
    double		current_speed_m_s_;
    oriented_position	oriented_position_;
    mutable vector3d_t	mouse_delta_pos_;		
    mutable rigid_body_map3d_t mouse_delta_xform_;

    oriented_position   initial_oriented_position_;
  public:

    camera_controller(const uvh_xyz_transform_t* uvh_xyz_transform);

    ~camera_controller();

    void set_initial_oriented_position(const oriented_position& op);
    void reset();

    void idle_update();

    void stop_movement();

    const point3d_t camera_position_xyz() const;
    double camera_altitude() const;

    rigid_body_map3d_t camera_view() const;

    void set_oriented_position(const oriented_position& op);

    const oriented_position& get_oriented_position() const;

    double current_speed_m_s() const;
    
    //////////////////////////////////////////////////////////////////
    // User Input

    void set_tilt_slider_factor(int x);
    int tilt_slider_factor() const;

    void reset_tilt_slider_factor();

    void set_zoom_slider_factor(int x);
    int zoom_slider_factor() const;

    void reset_zoom_slider_factor();

    void mouse_move(int x, int y);

    MOUSE_STATUS mouse_status() const;
    
    void mouse_press(int x, int y, MOUSE_STATUS button);
  
    void mouse_release(MOUSE_STATUS button);

    KEYBOARD_STATUS keyboard_status() const;
    
    void key_press(KEYBOARD_STATUS key_status);

    void key_release(KEYBOARD_STATUS key_status);

    void wheel_tick(int delta);

  protected:
    double delta_yaw(double dt, const vector2d_t& dir) const;

    double delta_tilt(double dt, const vector2d_t& dir) const;

    double delta_distance_from_target(double dt) const;

    rigid_body_map3d_t delta_xform(double dt, vector2d_t dir) const;

  };

}
#endif // RATMAN_CAMERA_CONTOLLER_HPP

#ifndef RATMAN_CAMERA_CONTOLLER_IPP
#define RATMAN_CAMERA_CONTOLLER_IPP

namespace ratman {

  inline void camera_controller::set_initial_oriented_position(const oriented_position& op) {
    initial_oriented_position_ = op;
  }

  inline camera_controller::MOUSE_STATUS camera_controller::mouse_status() const {
    return mouse_status_;
  }

  inline camera_controller::KEYBOARD_STATUS camera_controller::keyboard_status() const {
    return keyboard_status_;
  }
  
  inline void camera_controller::mouse_move(int x, int y) {
    mouse_current_position_ = point2d_t(x,y);
  }

  inline void camera_controller::mouse_press(int x, int y, MOUSE_STATUS button) { 
    mouse_previous_position_ = point2d_t(x,y);
    mouse_current_position_ = point2d_t(x,y);
    stop_movement(); // Added
    mouse_status_ |= button;
  }
  
  inline void camera_controller::mouse_release(MOUSE_STATUS button) {
    mouse_status_ &= !button;
  }

  inline void camera_controller::key_press(KEYBOARD_STATUS key_status) { 
    keyboard_status_ |= key_status;
  }

  inline void camera_controller::key_release(KEYBOARD_STATUS key_status) { 
    keyboard_status_ &= !key_status;
  }

  inline const point3d_t camera_controller::camera_position_xyz() const {
    return oriented_position_.position_xyz();
  }

  inline double camera_controller::current_speed_m_s() const {
    return current_speed_m_s_;
  }

  inline void camera_controller::reset_tilt_slider_factor() {
    tilt_slider_factor_ = 1.0f;
    keyboard_status_ &= !K_TILT_DOWN;
    keyboard_status_ &= !K_TILT_UP;
  }

  inline void camera_controller::reset_zoom_slider_factor() {
    zoom_slider_factor_ = 1.0f;
    keyboard_status_ &= !K_ZOOMIN;
    keyboard_status_ &= !K_ZOOMOUT;
  }

 
}
#endif // RATMAN_CAMERA_CONTOLLER_IPP
