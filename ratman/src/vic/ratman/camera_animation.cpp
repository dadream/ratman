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
#include <vic/ratman/camera_animation.hpp>
#include <cmath>

namespace ratman {

  camera_animation::camera_animation() {
    interpolation_track_ = new interpolation_track_t();
    interpolation_track_->set_continuity(1); // FIXME
    // interpolation_track_->set_is_interpolating(true); // missing from sl!!
  }

  camera_animation::~camera_animation() {
    delete interpolation_track_;
    interpolation_track_ = 0;
  }

  void camera_animation::compute_track_from_to(const oriented_position& from, const oriented_position& to, double to_millisec) {
    begin_track();
    {
      // Start
      add_track_point(to_millisec*0.00, from);
      
      // add middle steps with higher z if there is a difference in tilt between the 2 endpoints.
      double travel_dist               = from.position_xyz().distance_to(to.position_xyz());

      double from_altitude             = from.uvh_xyz_transform()->altitude_from_xyz(from.position_xyz());
      double target_altitude           = from.uvh_xyz_transform()->altitude_from_xyz(to.position_xyz());

      double desired_travel_altitude   = std::max(from_altitude,
						  std::max(target_altitude, 
							   0.6*travel_dist));
      point3d_t start_v = from.local_position_xyz();
      double start_d = std::sqrt(start_v[0]*start_v[0] + start_v[1]*start_v[1]);
      
      // Move vertically towards desired travel altitude
      oriented_position middle_0       = from;
      middle_0.set_tilt(deg2rad(90.0)-std::atan2(desired_travel_altitude,start_d)); // Look downward
      middle_0.set_distance_from_target(std::sqrt(desired_travel_altitude*desired_travel_altitude+
						  start_d*start_d));
      
      
      oriented_position middle_1       = from.lerp(to, 0.75);
      middle_1.set_tilt(to.tilt()); // == 0
      middle_1.set_distance_from_target(desired_travel_altitude);
      
      oriented_position middle_2       = to;
      middle_2.set_distance_from_target(target_altitude);
      
      double s0 = 1.0*from.position_xyz().distance_to(middle_0.position_xyz());
      double s1 = 1.0*middle_0.position_xyz().distance_to(middle_1.position_xyz());
      double s2 = 1.0*middle_1.position_xyz().distance_to(middle_2.position_xyz());
      double s3 = 1.0*middle_2.position_xyz().distance_to(to.position_xyz());
      double s = s0+s1+s2+s3;
      if (s) {
	double t0 = sl::median(0.0, 1.0, s0/s);
	double t1 = sl::median(t0+0.1, 1.0, (s0+s1)/s);
	double t2 = sl::median(t1+0.1, 1.0, (s0+s1+s2)/s);

	if (t0>0.0+0.1 && t0<1.0-0.1) add_track_point(to_millisec*t0, middle_0);
	if (t1>t0+0.1 && t1<1.0-0.1) add_track_point(to_millisec*t1, middle_1);
	if (t2>t1+0.1 && t2<1.0-0.1) add_track_point(to_millisec*t2, middle_2);
      }
      
      add_track_point(to_millisec*1.00, to);
    }
    end_track();
  }

void camera_animation::compute_fly_from_to(const oriented_position& from, const oriented_position& to, double to_millisec) {
  oriented_position start_0       = from;
  start_0.set_yaw(to.yaw());
  
  double travel_dist               = from.position_xyz().distance_to(to.position_xyz());
  double from_altitude             = from.uvh_xyz_transform()->altitude_from_xyz(from.position_xyz());
  double target_altitude           = from.uvh_xyz_transform()->altitude_from_xyz(to.position_xyz());
  double desired_travel_altitude   = std::max(from_altitude,std::max(target_altitude,0.5*travel_dist));
  point3d_t start_v = from.local_position_xyz();
  double start_d = std::sqrt(start_v[0]*start_v[0] + start_v[1]*start_v[1]);  

  begin_track();
  {
    // Start
    add_track_point(to_millisec*0.00, start_0);
      
    // Move vertically towards desired travel altitude
    oriented_position middle_0       = from;
    middle_0.set_yaw(to.yaw());
    middle_0.set_tilt(deg2rad(90.0)-std::atan2(desired_travel_altitude,start_d));// + 0.78);// Look forward
    middle_0.set_distance_from_target(std::sqrt(desired_travel_altitude*desired_travel_altitude+
						  start_d*start_d));
      
    oriented_position middle_1       = from.lerp(to, 0.75);
    middle_1.set_tilt(to.tilt()); // == 0
    middle_1.set_distance_from_target(desired_travel_altitude);
    middle_1.set_yaw(to.yaw());
    
    oriented_position middle_2       = to;
    middle_2.set_distance_from_target(target_altitude);
    middle_2.set_yaw(to.yaw());
      
      double s0 = 1.0*from.position_xyz().distance_to(middle_0.position_xyz());
      double s1 = 1.0*middle_0.position_xyz().distance_to(middle_1.position_xyz());
      double s2 = 1.0*middle_1.position_xyz().distance_to(middle_2.position_xyz());
      double s3 = 1.0*middle_2.position_xyz().distance_to(to.position_xyz());
      double s = s0+s1+s2+s3;
      if (s) {
	double t0 = sl::median(0.0, 1.0, s0/s);
	double t1 = sl::median(t0+0.1, 1.0, (s0+s1)/s);
	double t2 = sl::median(t1+0.1, 1.0, (s0+s1+s2)/s);

	if (t0>0.0+0.1 && t0<1.0-0.1) add_track_point(to_millisec*t0, middle_0);
	if (t1>t0+0.1 && t1<1.0-0.1) add_track_point(to_millisec*t1, middle_1);
	if (t2>t1+0.1 && t2<1.0-0.1) add_track_point(to_millisec*t2, middle_2);
      }

      add_track_point(to_millisec*1.00, to);
    }
    end_track();
  }

  void camera_animation::begin_track() {
    interpolation_track_->clear();
  }


  void camera_animation::add_track_point(double t_millisec, const oriented_position& p) {
    std::pair<interpolation_track_t::iterator, bool> ins = interpolation_track_->insert(std::make_pair(t_millisec, p));
    if (!ins.second) {
      // Was not inserted, replace previous!!!
      ins.first->second = p;
    } 
  }

  void camera_animation::end_track() {
    // Nothing to do
  }

  double camera_animation::end_time() const {
    double result = interpolation_track_->empty() ? 0.0 : interpolation_track_->end_time();
    return result;
  }

  bool camera_animation::is_active() {
    if (is_active_) {
      is_active_ = clock_.elapsed().as_milliseconds() < end_time();
    }
    return is_active_;
  }

  double camera_animation::current_time() const {
    return double(clock_.elapsed().as_milliseconds());
  }

  oriented_position camera_animation::current_position_xyz() const {
    // FIXME - SLOW IN SLOW OUT
    double t_end = end_time();

    double t = clock_.elapsed().as_milliseconds();
    double t_rescaled = std::min(1.0, t/t_end);
    double t_out;
    double t1 = (t_rescaled-1.0);
    t_out = (1.0 - t1*t1*t1*t1)* t_end;

    oriented_position result = interpolation_track_->value_at(sl::median(std::min(0.1, t_end), 
									 std::max(t_end-0.1, 0.0), 
									 t_out));
 
    return result;
  } 
}
