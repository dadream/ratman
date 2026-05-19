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
#ifndef RATMAN_ORIENTED_POSITION_HPP
#define RATMAN_ORIENTED_POSITION_HPP

#include <vic/ratman/ratman.hpp>
#include <vic/cbdam/base/coordinate_transform.hpp>
#include <sl/quaternion.hpp>
#include <cassert>

namespace ratman {

  /**
   * From:
   * - target position at ground given by local_to_global_xform_
   * - two angles: 
   *   - yaw:  angle with north
   *   - tilt: angle with vertical: 0 watch down, 90 watch horizontal
   * - distance from target position
   * oriented_position gives a matrix to place a camera, at dist
   * facing ground target, with yaw and tilt angles.
   * Used to place camera and for animations.
   */
  class oriented_position {
  public:
    typedef cbdam::coordinate_transform uvh_xyz_transform_t;
    typedef sl::quaternion<double>	quaternion_t;

  protected:
    const uvh_xyz_transform_t* uvh_xyz_transform_;
    //    point3d_t	ground_target_xyz_;
    rigid_body_map3d_t  local_to_global_xform_;
    double		distance_from_target_;
    double		yaw_;
    double		tilt_;			// angle between up and dir
    double		roll_;

  public:

    /// Init from parametric u,v ground position
    oriented_position(const uvh_xyz_transform_t* uvh_xyz_transform = 0,
		      const point2d_t& ground_target_uv = point2d_t(0.0, 0.0),
		      double distance_from_target = 1000.0,
		      double yaw = 0.0,
		      double tilt = 0.0); // 0 = vertical (on up vector)

    /// Init from x,y,z ground position
    oriented_position(const uvh_xyz_transform_t* uvh_xyz_transform,
		      const point3d_t& ground_target_xyz,
		      double distance_from_target = 1000.0,
		      double yaw = 0.0,
		      double tilt = 0.0); // 0 = vertical (on up vector)

    oriented_position(const uvh_xyz_transform_t* uvh_xyz_transform,
		      const rigid_body_map3d_t& xform,
		      double distance_from_target = 1000.0,
		      double yaw = 0.0,
		      double tilt = 0.0); // 0 = vertical (on up vector)

    bool is_valid() const { return uvh_xyz_transform_ != 0; }

    bool operator == (const oriented_position& other) const;

    bool operator != (const oriented_position& other) const;

    template <class T_PARAMETER>
    oriented_position lerp(const oriented_position& other, T_PARAMETER t) const {
      assert(uvh_xyz_transform_ == other.uvh_xyz_transform_);
      assert(is_valid());

      oriented_position result(other);
      result.local_to_global_xform_= local_to_global_xform_.lerp(other.local_to_global_xform_, t);
      result.distance_from_target_ = distance_from_target_ * (1-t) + other.distance_from_target_ * t;
      result.yaw_                  = yaw_ * (1-t) + other.yaw_ * t;
      result.tilt_                 = tilt_ * (1-t) + other.tilt_ * t;
      result.roll_                 = roll_ * (1-t) + other.roll_ * t;
      return result;
    }
      
    const uvh_xyz_transform_t* uvh_xyz_transform() const { return uvh_xyz_transform_; }
    point3d_t ground_target_xyz() const {return local_to_global_xform_ * point3d_t(0.0, 0.0, 0.0);}
    const rigid_body_map3d_t& local_to_global_xform() const {return local_to_global_xform_;}
    double distance_from_target() const {return distance_from_target_;}
    double yaw() const {return yaw_;}
    double tilt() const {return tilt_;}
    double roll() const {return roll_;}

    vector3d_t local_direction_xyz() const;
    point3d_t  local_position_xyz() const;

    point3d_t position_xyz() const;

    rigid_body_map3d_t xyz_to_camera_transform() const;
 
    inline void set_ground_target_xyz(const point3d_t& xyz) {// ground_target_xyz_ = uvh_xyz_transform_->xyz_on_ground(xyz);}
      local_to_global_xform_ = uvh_xyz_transform_->xyz_local_to_global_from_xyz(xyz);
    }

    inline void set_local_to_global_xform(const rigid_body_map3d_t& xform) { local_to_global_xform_ = xform;}
    inline void set_distance_from_target(double x) { distance_from_target_ = x;}
    inline void set_yaw(double x) { yaw_ = x; }
    inline void set_tilt(double x) { tilt_ = x; }

    void set_position_xyz(const point3d_t& xyz);
    void move_ground_position_to_xyz(const point3d_t& xyz);
  };
}

inline std::ostream& operator<<(std::ostream& os, const ratman::oriented_position& op) {
  return os <<
    "Target: " << op.ground_target_xyz()[0] << " " << op.ground_target_xyz()[1] << " " << op.ground_target_xyz()[2] << std::endl <<
    "Dir   : " << op.local_direction_xyz()[0] << " " << op.local_direction_xyz()[1] << " " << op.local_direction_xyz()[2] << std::endl <<
    "Dist  : " << op.distance_from_target() << std::endl <<
    "Yaw   : " << op.yaw() << std::endl <<
    "Tilt  : " << op.tilt() << std::endl <<
    "Roll  : " << op.roll() << std::endl <<
    "=> POS: " << op.position_xyz()[0] << " " << op.position_xyz()[1] << " " << op.position_xyz()[2] << std::endl;
}

#endif
