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
#ifndef RATMAN_HPP
#define RATMAN_HPP

#include <sl/fixed_size_square_matrix.hpp>
#include <sl/fixed_size_point.hpp>
#include <sl/projective_map.hpp>
#include <sl/rigid_body_map.hpp>
#include <sl/linear_map_factory.hpp>
#include <sl/axis_aligned_box.hpp>
#include <sl/cstdint.hpp>
#include <cassert>

#ifdef _WIN32
#include <windows.h>
#undef min
#undef max
#endif


/**
 *  RATMAN - Rapidly Adaptive Terrain Models Available on the Net
 */
namespace ratman {

  // Basic data types
  typedef sl::uint8_t   uint8_t;
  typedef sl::int8_t    int8_t;
  typedef sl::uint16_t  uint16_t;
  typedef sl::int16_t   int16_t;
  typedef sl::uint32_t  uint32_t;
  typedef sl::int32_t   int32_t;
  typedef sl::uint64_t  uint64_t;
  typedef sl::int64_t   int64_t;

  typedef sl::point2f   point2f_t;
  typedef sl::vector2f  vector2f_t;
  typedef sl::point3f   point3f_t;
  typedef sl::vector3f  vector3f_t;
  typedef sl::point4f   point4f_t;
  typedef sl::vector4f  vector4f_t;
  typedef sl::matrix3f  matrix3x3f_t;
  typedef sl::matrix4f  matrix4x4f_t;

  typedef sl::point2d   point2d_t;
  typedef sl::vector2d  vector2d_t;
  typedef sl::point3d   point3d_t;
  typedef sl::vector3d  vector3d_t;
  typedef sl::point4f   point4d_t;
  typedef sl::vector4d  vector4d_t;
  typedef sl::matrix3d  matrix3x3d_t;
  typedef sl::matrix4d  matrix4x4d_t;

  typedef sl::fixed_size_point<2,int> point2i_t;


  typedef sl::aabox2d   aabox2d_t;
  typedef sl::aabox3d   aabox3d_t;

  typedef sl::projective_map3d projective_map3d_t;
  typedef sl::rigid_body_map3d rigid_body_map3d_t;
  typedef sl::projective_map3f projective_map3f_t;
  typedef sl::rigid_body_map3f rigid_body_map3f_t;

  static inline double rad2deg(double x) {
    const double RAD2DEG = 57.29577951308232077580;
    return RAD2DEG * x;
  }

  static inline double deg2rad(double x) {
    const double DEG2RAD =  0.01745329251994329580;
    return DEG2RAD * x;
  }
 
  template <class value_t>
  static inline sl::projective_map<3,value_t> perspective3D(value_t fov, 
							    value_t aspect, 
							    value_t v_near, 
							    value_t v_far) {
    return sl::linear_map_factory<3,value_t>::perspective(fov, aspect, v_near, v_far);
  }
  
  template <class value_t>
  static inline sl::rigid_body_map<3,value_t> lookat3D(const sl::fixed_size_point<3,value_t>& vp, 
						       const sl::fixed_size_point<3,value_t>& rp, 
						       value_t twist) {
    return sl::linear_map_factory<3,value_t>::lookat(vp, rp, twist);
  }

  template <class value_t>
  static inline sl::rigid_body_map<3,value_t> translation3D(const sl::fixed_size_vector<sl::column_orientation,3,value_t>& translation) {
    return sl::linear_map_factory<3,value_t>::translation(translation);
  }
  
}

#endif
