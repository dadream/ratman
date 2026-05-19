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
#ifndef CBDAM_CONFIG_HPP
#define CBDAM_CONFIG_HPP

#include <sl/fixed_size_point.hpp>
#include <sl/fixed_size_vector.hpp>
#include <sl/fixed_size_square_matrix.hpp>
#include <sl/axis_aligned_box.hpp>
#include <sl/clock.hpp>
#include <sl/cstdint.hpp>

namespace cbdam {

  // Basic data types 
  typedef sl::uint8_t         uint8_t;
  typedef sl::int8_t          int8_t;
  typedef sl::uint16_t        uint16_t;
  typedef sl::int16_t         int16_t;
  typedef sl::uint32_t        uint32_t;
  typedef sl::int32_t         int32_t;
  typedef sl::uint64_t        uint64_t;
  typedef sl::int64_t         int64_t;
  
  typedef sl::point2f  point2_t;
  typedef sl::vector2f vector2_t;
  typedef sl::point3f  point3_t;
  typedef sl::vector3f vector3_t;
  typedef sl::point4f  point4_t;
  typedef sl::vector4f vector4_t;
  typedef sl::matrix4f matrix44_t;
  typedef sl::aabox3f  aabox_t;
  typedef sl::row_vector3f normal_t;

  typedef sl::point2d  point2d_t;
  typedef sl::vector2d vector2d_t;
  typedef sl::point3d  point3d_t;
  typedef sl::vector3d vector3d_t;
  typedef sl::point4d  point4d_t;
  typedef sl::vector4d vector4d_t;
  typedef sl::matrix4d matrix44d_t;
  typedef sl::aabox2d  aabox2d_t;
  typedef sl::aabox3d  aabox3d_t;
  typedef sl::row_vector3d normald_t;
  
#define CBDAM_R_ACCESSOR(__TP__,__ID__) \
  inline const __TP__& __ID__ () const { return __ID__##_; }

#define CBDAM_W_ACCESSOR(__TP__,__ID__) \
  inline __TP__& __ID__ () { return __ID__##_; }

#define CBDAM_RW_ACCESSOR(__TP__,__ID__) \
  inline const __TP__& __ID__ () const { return __ID__##_; } \
  inline __TP__& __ID__ () { return __ID__##_; }

  /// Set to 1 for single cpu non threaded implementation
  
#ifdef _WIN32
#define CBDAM_BUILD_MAX_THREAD_COUNT 1
#else
#define CBDAM_BUILD_MAX_THREAD_COUNT 8
#endif

#if CBDAM_BUILD_MAX_THREAD_COUNT>1
#  define CBDAM_BUILD_MULTITHREADED 1
#else 
#  define CBDAM_BUILD_MULTITHREADED 0
#endif

  template<class T>
  inline sl::fixed_size_vector<sl::column_orientation, 3, T> rgb_from_ycocg_r(const T& Y, const T& Co, const T& Cg) {
    T tmp = Y - (Cg>>1);
    T G = Cg + tmp;
    T B = tmp - (Co>>1);
    T R = B + Co;
    return sl::fixed_size_vector<sl::column_orientation, 3, T>(R, G, B);
  }

  template<class T>
  inline sl::fixed_size_vector<sl::column_orientation, 3, T> ycocg_r_from_rgb(const T& r, const T& g, const T& b) {
    T Co  = r - b;
    T tmp = b + (Co>>1);
    T Cg  = g - tmp;
    T Y   = tmp + (Cg>>1); 
    return sl::fixed_size_vector<sl::column_orientation, 3, T>(Y, Co, Cg);
  }

} // namespace cbdam

#endif
