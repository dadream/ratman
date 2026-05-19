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
#include <vic/ratman/oriented_position.hpp>
#include <sl/linear_map_factory.hpp>

namespace ratman {

  oriented_position::oriented_position(const uvh_xyz_transform_t* uvh_xyz_transform,
				       const point2d_t& ground_target_uv,
				       double distance_from_target,
				       double yaw,
				       double tilt) :
    uvh_xyz_transform_(uvh_xyz_transform),
    distance_from_target_(distance_from_target), 
    yaw_(yaw), 
    tilt_(tilt),
    roll_(0.0) {
    if (is_valid()) {
      point3d_t ground_target_xyz = uvh_xyz_transform_->xyz_from_uvh(point3d_t(ground_target_uv[0], ground_target_uv[1], 0.0));
      local_to_global_xform_ = uvh_xyz_transform_->xyz_local_to_global_from_xyz(ground_target_xyz);
    }
  }
  
  oriented_position::oriented_position(const uvh_xyz_transform_t* uvh_xyz_transform,
				       const point3d_t& ground_target_xyz,
				       double distance_from_target,
				       double yaw,
				       double tilt) :
    uvh_xyz_transform_(uvh_xyz_transform),
    distance_from_target_(distance_from_target), 
    yaw_(yaw), 
    tilt_(tilt),
    roll_(0.0) {
    if (is_valid()) {
      local_to_global_xform_ = uvh_xyz_transform_->xyz_local_to_global_from_xyz(ground_target_xyz);
    }
  }
    
  oriented_position::oriented_position(const uvh_xyz_transform_t* uvh_xyz_transform,
				       const rigid_body_map3d_t& xform,
				       double distance_from_target,
				       double yaw,
				       double tilt) :
    uvh_xyz_transform_(uvh_xyz_transform),
      local_to_global_xform_(xform),
      distance_from_target_(distance_from_target), 
    yaw_(yaw), 
    tilt_(tilt),
    roll_(0.0) {

    }

  bool oriented_position::operator == (const oriented_position& other) const {
    return 
      (uvh_xyz_transform_ == other.uvh_xyz_transform_) &&
      (local_to_global_xform_ == other.local_to_global_xform_) &&
      (distance_from_target_ == other.distance_from_target_) &&
      (yaw_ == other.yaw_) &&
      (tilt_ == other.tilt_) &&
      (roll_ == other.roll_);
  }

  bool oriented_position::operator != (const oriented_position& other) const {
    return !(*this == other);
  }

  vector3d_t oriented_position::local_direction_xyz() const {
    // Strange definition, keeping the one of ratman 1.0 
    return vector3d_t(std::sin(-tilt_) * std::sin(-yaw_), 
		      std::sin(-tilt_) * std::cos(-yaw_), 
		      std::cos(-tilt_));
  }

  point3d_t oriented_position::local_position_xyz() const {
    return as_point(distance_from_target_ * local_direction_xyz());
  }

  point3d_t oriented_position::position_xyz() const {
    assert(is_valid());
    return local_to_global_xform_ * local_position_xyz();
  }

   
  // Was view()
  rigid_body_map3d_t oriented_position::xyz_to_camera_transform() const {
    assert(is_valid());

    return (sl::linear_map_factory3d::translation(0.0, 0.0, -distance_from_target_) *
	    sl::linear_map_factory3d::rotation(0, -tilt_) * 
	    sl::linear_map_factory3d::rotation(2, -yaw_) * 
	    ~local_to_global_xform_);
  }

    
  void oriented_position::set_position_xyz(const point3d_t& xyz) {
    assert(is_valid());
    point3d_t xyz_local = inverse_transformation(local_to_global_xform_, xyz);
    //    std::cerr << "oriented_position::set_position_xyz::xyz_local " << xyz_local[0] << " " << xyz_local[1] << " " << xyz_local[2] << std::endl;
     
    // Invert local position_xyz
    double rho = as_vector(xyz_local).two_norm();
    double tilt = tilt_;
    double yaw = yaw_;
    if (rho) {
      tilt = std::acos(sl::median(-1.0, 1.0, xyz_local[2]/rho));
      if (tilt < -1e-02 || tilt > 1e-02) {  
	//	yaw  = M_PI-std::atan2(xyz_local[0], xyz_local[1]); // -PI ... PI
	//	std::cerr << "change yaw from " << yaw_ << " to " << yaw << std::endl;
      }
    } 
    distance_from_target_ = rho;
    tilt_ = tilt;
    yaw_ = yaw;
  }
  
  void oriented_position::move_ground_position_to_xyz(const point3d_t& ground_xyz) {
    assert(is_valid());
    point3d_t old_xyz = position_xyz();
    point3d_t old_ground = ground_target_xyz();
    vector3d_t old_dir = as_vector(old_ground).ok_normalized();
    vector3d_t new_dir = as_vector(ground_xyz).ok_normalized();
    // rotate global to local | old dir goes over new dir.
    // remove and readd translation before concateneting rotations.

    if (!old_dir.is_epsilon_equal(new_dir, 0.0001)) {
      local_to_global_xform_ = sl::linear_map_factory3d::rotation(old_dir, new_dir) * local_to_global_xform_;
      assert(ground_xyz.is_epsilon_equal(ground_target_xyz(), 0.1));
      
      // Invert local position_xyz
      //    set_position_xyz(old_xyz);
      point3d_t xyz_local = inverse_transformation(local_to_global_xform_, old_xyz);
      distance_from_target_ = as_vector(xyz_local).two_norm();
      if (distance_from_target_) {tilt_ = std::acos(sl::median(-1.0, 1.0, xyz_local[2]/distance_from_target_));}
    }
  }
}
