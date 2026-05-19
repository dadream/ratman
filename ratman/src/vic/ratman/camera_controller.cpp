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
#include <vic/ratman/camera_controller.hpp>
#include <sl/linear_map_factory.hpp>
#include <sl/clock.hpp>

namespace ratman {

  rigid_body_map3d_t rotation_about_axis(const vector3d_t& axis, double rad_angle) {
    sl::quaternion<double> q;
    q.from_axis_angle(axis, rad_angle );
    return rigid_body_map3d_t(q, vector3d_t(0.0, 0.0, 0.0));    
  }

  camera_controller::camera_controller(const uvh_xyz_transform_t* uvh_xyz_transform) :
    uvh_xyz_transform_(uvh_xyz_transform),
    oriented_position_(uvh_xyz_transform),
    initial_oriented_position_(uvh_xyz_transform) {
    // FIXME default init position???
    reset();
    clock_ = new sl::real_time_clock;
  }

  camera_controller::~camera_controller() {
    delete clock_;
  }
  
  double camera_controller::camera_altitude() const {
    return uvh_xyz_transform_->altitude_from_xyz(camera_position_xyz());
  }


  void camera_controller::reset() {
    oriented_position_ = initial_oriented_position_;

    mouse_previous_position_ = point2d_t(0,0);
    mouse_current_position_ = point2d_t(0,0);
    mouse_status_ = MS_RELEASED;
    keyboard_status_ = K_RELEASED;
    current_speed_m_s_ = 0.0f;
    tilt_slider_factor_ = 1.0f;
    zoom_slider_factor_ = 1.0f;
    current_speed_m_s_ = 0.0f;
    mouse_delta_pos_ = vector3d_t(0,0,0);
    mouse_delta_xform_ = sl::linear_map_factory3d::identity();
  }

  void camera_controller::stop_movement() {
    mouse_delta_xform_  = sl::linear_map_factory3d::identity();
    mouse_delta_pos_ = vector3d_t(0,0,0);
  }

  rigid_body_map3d_t camera_controller::delta_xform(double dt, vector2d_t dir) const {
    // We don't want dir passed as const reference, because it can be modified.
    // FIXME PLANAR
    double k = 35.0;
    bool key_pressed = false;
    // handle key arrows as if there would have been a movement of dir along x or y 
    if (keyboard_status_ & K_FORWARD) {
      dir = vector2d_t(0.0, k);
      key_pressed = true;
    } else if (keyboard_status_ & K_BACKWARD) {
      dir = vector2d_t(0.0, -k);
      key_pressed = true;
    } else if (keyboard_status_ & K_LEFT) {
      dir = vector2d_t(k, 0.0);
      key_pressed = true;
    } else if (keyboard_status_ & K_RIGHT) {
      dir = vector2d_t(-k, 0.0);
      key_pressed = true;
    }

    // handle rotation of due to mouse move or arrow key pressed
    if (mouse_status_ & MS_LEFT || key_pressed) {
      vector3d_t rot_axis(-dir[1], -dir[0], 0.0);
      // FIXME Error in debug mode ... rot_axis is initialized to zero and from_axis_angle fails
      rot_axis = sl::linear_map_factory3d::rotation(2, oriented_position_.yaw()) * rot_axis;
      // FIXME double axis_norm  = rot_axis.two_norm();
      double axis_norm  = rot_axis.two_norm_squared();
      //      std::cerr << "AXIS_NORM= " << axis_norm << std::endl;
      const double rad_angle = axis_norm * dt * 0.0000001 * sl::max(camera_altitude() / 2000000.0, 0.0001);
      if (axis_norm != 0.0) {
	rot_axis /= sqrt(axis_norm);
	
	sl::quaternion<double> rotation_about_axis;
	rotation_about_axis.from_axis_angle(rot_axis, rad_angle);
	if (key_pressed) {
	  // return a temporary rotation, which will disappear when button is pressed
	  return rigid_body_map3d_t(rotation_about_axis, vector3d_t(0.0, 0.0, 0.0));
	} else {
	  // set rotation in mouse_delta_xform_ which will be valid also on mouse release.
	  mouse_delta_xform_ = rigid_body_map3d_t(rotation_about_axis, vector3d_t(0.0, 0.0, 0.0));
	}
      }
    }
    return mouse_delta_xform_;
  }

  void camera_controller::idle_update() {
    static const double PI = 3.141592653;

    point3d_t previous_position_xyz = oriented_position_.position_xyz();

    vector2d_t mouse_direction = mouse_current_position_ - mouse_previous_position_;
    double dt = clock_->elapsed().as_milliseconds();

    double tilt = oriented_position_.tilt() + delta_tilt(dt, mouse_direction);
    if (tilt < 0.0f) {tilt = 0.0f;}
    if (tilt > PI) {tilt = PI;}
    oriented_position_.set_tilt(tilt);

    double yaw = oriented_position_.yaw() + delta_yaw(dt, mouse_direction);
    if (yaw < 0) yaw += 2.0*PI;
    if (yaw > 2.0*PI) yaw -= 2.0*PI;
    oriented_position_.set_yaw(yaw);

    double distance_from_target = oriented_position_.distance_from_target() + delta_distance_from_target(dt);
    if (distance_from_target < 1.0) distance_from_target = 1.0;
    oriented_position_.set_distance_from_target(distance_from_target);

    // HANDLE ROTATION IN INCREMENTAL WAY
    // in LOCAL2GLOBAL
    // ROT = ROT * DELTA_ROT
    // LOCAL2GLOBAL is ROT*TR: remove TR and reapply: ROT = ROT * T(-1) * DELTA_ROT * T()
    rigid_body_map3d_t xform = delta_xform(dt, mouse_direction);
    double radius = 0.0;
    if (dynamic_cast<const cbdam::spherical_coordinate_transform*>(uvh_xyz_transform_) != 0) {
      radius = dynamic_cast<const cbdam::spherical_coordinate_transform*>(uvh_xyz_transform_)->radius();
    }
    rigid_body_map3d_t tr = sl::linear_map_factory3d::translation(0.0, 0.0, radius);
    rigid_body_map3d_t inv_tr = sl::linear_map_factory3d::translation(0.0, 0.0, -radius);
    rigid_body_map3d_t res = oriented_position_.local_to_global_xform() * inv_tr * xform * tr;
						   
    oriented_position_.set_local_to_global_xform(res);

    current_speed_m_s_ = (oriented_position_.position_xyz() - previous_position_xyz).two_norm() / (dt*0.001);

    mouse_previous_position_ = mouse_current_position_;
    clock_->restart();
  }

  double camera_controller::delta_yaw(double dt, const vector2d_t& mouse_dir) const {
    static const double yaw_conversion_factor = 0.00004f;
    double delta = 0;
    if (mouse_status_ & MS_RIGHT) {
      delta = -mouse_dir[0] * yaw_conversion_factor * dt;
    } else if (keyboard_status_ & K_YAW_CCW) {
      delta = -10 * yaw_conversion_factor * dt;
    } else if (keyboard_status_ & K_YAW_CW) {
      delta = 10 * yaw_conversion_factor * dt;
    }
    return delta;
  }

  double camera_controller::delta_tilt(double dt, const vector2d_t& mouse_dir) const {
    static const double tilt_conversion_factor = 0.00004f;
    double delta = 0;
    if (mouse_status_ & MS_RIGHT) {
      delta =  -mouse_dir[1] * tilt_conversion_factor * dt;
    } else if (keyboard_status_ & K_TILT_UP) {
      delta = 10 * tilt_conversion_factor * tilt_slider_factor_ * dt;
    } else if (keyboard_status_ & K_TILT_DOWN) {
      delta = -10 * tilt_conversion_factor * tilt_slider_factor_ * dt;
    }
    return delta;
  }

  double camera_controller::delta_distance_from_target(double dt) const {
    const double delta_z = 1.0 * (sl::max(camera_altitude() / 2000.0, 0.1))* dt;
    double delta_distance = 0.0f;

    if (keyboard_status_ & K_ZOOMIN) {
      delta_distance = -delta_z * zoom_slider_factor_;
    } else if (keyboard_status_ & K_ZOOMOUT) {
      delta_distance = delta_z * zoom_slider_factor_;
    }

    return delta_distance;
  }

  rigid_body_map3d_t camera_controller::camera_view() const {
    return oriented_position_.xyz_to_camera_transform();
  }

  void camera_controller::set_tilt_slider_factor(int x) {
    // x ranges in [-100,100]: map to [-3,3]
    tilt_slider_factor_ = fabs((double)x) * 3.0f / 100.0f;

    if (x > 0) {
      keyboard_status_ |= K_TILT_UP;
    } else if (x < 0) {
      keyboard_status_ |= K_TILT_DOWN;
    } else {
      keyboard_status_ &= !K_TILT_DOWN;
      keyboard_status_ &= !K_TILT_UP;
      tilt_slider_factor_ = 1.0f;
    } 
  }

  int camera_controller::tilt_slider_factor() const {
    int result = int(zoom_slider_factor_*100.0f/3.0f);
    if(keyboard_status_ & K_TILT_DOWN) result = -result;
    return result;
  }

  void camera_controller::set_zoom_slider_factor(int x) {
    // x ranges in [-100,100]: map to [-3,3]
    zoom_slider_factor_ = fabs((double)x) * 3.0f / 100.0f;

    if(x > 0) {
      keyboard_status_ |= K_ZOOMIN;
    } else if (x < 0) {
      keyboard_status_ |= K_ZOOMOUT;
    } else {
      keyboard_status_ &= !K_ZOOMIN;
      keyboard_status_ &= !K_ZOOMOUT;
      zoom_slider_factor_ = 1.0f;
    }    
  }

  int camera_controller::zoom_slider_factor() const {
    int result = int(zoom_slider_factor_*100.0f/3.0f);
    if(keyboard_status_ & K_ZOOMOUT) result = -result;
    return result;
  }

  void camera_controller::set_oriented_position(const oriented_position& op) {
    oriented_position_ = op;
    // restart the clock so that at next idle updte we don't find a huge dt.
    clock_->restart();

    // block previous movements
    //  mouse_delta_xform_ = sl::linear_map_factory3d::identity();
    //  mouse_delta_pos_ = vector3d_t(0.0, 0.0, 0.0);
  }

  const oriented_position& camera_controller::get_oriented_position() const {
    return oriented_position_;
  }

  void camera_controller::wheel_tick(int delta) {
    if (delta>0) {
      key_press(K_ZOOMIN);
    } else if (delta<0) {
      key_press(K_ZOOMOUT);
    }
    double dt = 200.0f;

    double distance_from_target = oriented_position_.distance_from_target() + delta_distance_from_target(dt);
    if (distance_from_target < 1.0) distance_from_target = 1.0;
    oriented_position_.set_distance_from_target(distance_from_target);

    key_release(K_ZOOMIN | K_ZOOMOUT);
  }

}




#if 0

  vector3d_t camera_controller::delta_target(double dt, const vector2d_t& dir) const {
    const double delta_z = (sl::max(camera_altitude() / 2000.0, 0.1))* dt;

    vector3d_t delta_pos(0,0,0);
    // update elevation
    double yaw = oriented_position_.yaw();
    if (keyboard_status_ & K_FORWARD) {
      vector3d_t v(0, delta_z, 0);
      delta_pos += (sl::linear_map_factory3d::rotation(2, yaw) * v);     
    } else if (keyboard_status_ & K_BACKWARD) {
      vector3d_t v(0, delta_z, 0);
      delta_pos -= (sl::linear_map_factory3d::rotation(2, yaw) * v);     
    } else if (keyboard_status_ & K_LEFT) {
      vector3d_t v(delta_z, 0, 0);
      delta_pos -= (sl::linear_map_factory3d::rotation(2, yaw) * v);     
    } else if (keyboard_status_ & K_RIGHT) {
      vector3d_t v(delta_z, 0, 0);
      delta_pos += (sl::linear_map_factory3d::rotation(2, yaw) * v);     
    }

    return delta_pos + delta_target_mouse(dt, dir);
  }

  vector3d_t camera_controller::delta_target_mouse(double /*dt*/, const vector2d_t& dir) const {
   // update xy position
    if (mouse_status_ & MS_LEFT) {
      // rotate v of yaw around z
      const double delta_xy = oriented_position_.distance_from_target() / 2000.0;
      vector3d_t v(-delta_xy*dir[0], delta_xy*dir[1], 0);
      mouse_delta_pos_ = (sl::linear_map_factory3d::rotation(2, oriented_position_.yaw()) * v);
    }

    return mouse_delta_pos_;
  }

#endif
