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
#ifndef CBDAM_GRID_POINT_HPP
#define CBDAM_GRID_POINT_HPP

#include <vic/cbdam/base/config.hpp>
#include <sl/bitops.hpp>

#ifdef _WIN32
  // FIXME WARNINGS DISABLED
  #pragma warning(disable: 4244)  // possible loss of data 
  #pragma warning(disable: 4390)  // empty controlled statement
  #pragma warning(disable: 4522)  // multiple assignment operator specified
#endif

namespace cbdam {
  
  typedef sl::int32_t                                                         grid_value_t;
  typedef sl::fixed_size_point<3,grid_value_t>                                grid_point_t;
  typedef sl::fixed_size_vector<sl::column_orientation, 3, grid_value_t>      grid_vector_t;
  
  
  extern const sl::uint32_t  grid_sub_max;
  extern const grid_value_t grid_coord_max;
  extern const grid_value_t grid_coord_min;

  
  inline double grid_length_squared(const grid_point_t& p0,
                                    const grid_point_t& p1) {
    double dx = p0[0]-p1[0];
    double dy = p0[1]-p1[1];
    double dz = p0[2]-p1[2];
    return dx*dx+dy*dy+dz*dz;
  }
  
  inline grid_point_t midpoint(const grid_point_t& p0,
                               const grid_point_t& p1) {
    grid_point_t result = sl::tags::not_initialized();
    result[0] = (p0[0]+p1[0]) >> 1;
    result[1] = (p0[1]+p1[1]) >> 1;
    result[2] = (p0[2]+p1[2]) >> 1;
    return result;
  }

  inline double to_double(grid_value_t i0, double l, double u) {
    return l+(u-l)*double(i0)/double(grid_coord_max-grid_coord_min);
  }

  inline float to_float(grid_value_t i0, float l, float u) {
    return float(to_double(i0, double(l), double(u)));
  }

  inline sl::point3d to_point_double(const grid_point_t& p, float l, float u) {
    return sl::point3d(to_double(p[0], l, u), to_double(p[1], l, u), to_double(p[2], l, u));
  }

  inline sl::point3d to_point_on_z_ground_double(const grid_point_t& p, double l, double u) {
    return sl::point3d(to_double(p[0], l, u), to_double(p[1], l, u), 0.0f);
  }

  inline point3_t to_point_float(const grid_point_t& p, float l, float u) {
    return point3_t(to_float(p[0], l, u), to_float(p[1], l, u), to_float(p[2], l, u));
  }

  inline point3_t to_point_on_z_ground_float(const grid_point_t& p, float l, float u) {
    return point3_t(to_float(p[0], l, u), to_float(p[1], l, u), 0.0f);
  }

  inline normal_t to_normal(const grid_point_t& p) {
    return (normal_t(p[0], p[1], p[2])).ok_normalized();
  }
  
  inline grid_value_t min_grid_coord() {
    return -grid_coord_max;
  }

  inline grid_value_t max_grid_coord() {
    return grid_coord_max;
  }

  inline grid_point_t min_grid_point() {
    const grid_value_t L = min_grid_coord();
    return grid_point_t(L,L,L);
  }

  inline grid_point_t max_grid_point() {
    const grid_value_t H = max_grid_coord();
    return grid_point_t(H,H,H);
  }

  inline grid_point_t invalid_grid_point() {
    const grid_value_t V = -(grid_value_t(1) << (grid_sub_max+1));
    return grid_point_t(V,V,V);
  }

  inline grid_point_t grid_canonical_point(std::size_t i) {
    const grid_value_t L = -grid_coord_max;
    const grid_value_t H = grid_coord_max;
    switch(i) {
    case 0: return grid_point_t(L,L,H); break;
    case 1: return grid_point_t(L,H,H); break;
    case 2: return grid_point_t(H,H,H); break;
    case 3: return grid_point_t(H,L,H); break;
    case 4: return grid_point_t(L,L,L); break;
    case 5: return grid_point_t(L,H,L); break;
    case 6: return grid_point_t(H,H,L); break;
    case 7: return grid_point_t(H,L,L); break;
    default:
      SL_TRACE_OUT(-1) << "requested invalid canonical root " << i << std::endl;
      return grid_point_t(0,0,0); break;
    }
  }
  
  inline grid_point_t grid_cylindrical_canonical_point(std::size_t i) {
    const grid_value_t Ly = -grid_coord_max;
    const grid_value_t Hy = grid_coord_max;
    const grid_value_t L = Ly / 2;
    const grid_value_t H = Hy / 2;
    
    switch(i) {
    case 0:  return grid_point_t(L,Ly,H); break;
    case 1:  return grid_point_t(H,Ly,H); break;
    case 2:  return grid_point_t(L,0 ,H); break;
    case 3:  return grid_point_t(H,0 ,H); break;
    case 4:  return grid_point_t(L,Hy,H); break;
    case 5:  return grid_point_t(H,Hy,H); break;
    case 6:  return grid_point_t(L,Ly,L); break;
    case 7:  return grid_point_t(H,Ly,L); break;
    case 8:  return grid_point_t(L,0 ,L); break;
    case 9:  return grid_point_t(H,0 ,L); break;
    case 10: return grid_point_t(L,Hy,L); break;
    case 11: return grid_point_t(H,Hy,L); break;
    default:
      SL_TRACE_OUT(-1) << "requested invalid canonical root " << i << std::endl;
      return grid_point_t(0,0,0); break;
    }
  }

  inline std::ostream& operator<<(std::ostream& os, const grid_point_t& rhs) {
    os << "(" << rhs[0] << ", " << rhs[1] << ", " << rhs[2] << ") ";
    return os;
  }

  struct grid_point_hasher { // FIXME - define point hashing in sl
    inline uint32_t operator()(const grid_point_t& x) const {
      return uint32_t(sl::hash_bytes<sizeof(x)>((const unsigned char*)&x));
    }
  };

  struct grid_point_morton_cmp {
    // NOTE: log2(grid_coord_max) > 64/3, it is thus not possible to
    // just use a simple 64 bit morton encoding on the full range of coords!
    
    static inline sl::uint64_t hi_morton(const grid_point_t& gp) {
      uint32_t x = (-grid_coord_min + gp[0]); // Positive
      uint32_t y = (-grid_coord_min + gp[1]); // Positive
      uint32_t z = (-grid_coord_min + gp[2]); // Positive
      return
        sl::morton_bitops<uint64_t,3>::encoded(x>>16, 0) |
        sl::morton_bitops<uint64_t,3>::encoded(y>>16, 1) |
        sl::morton_bitops<uint64_t,3>::encoded(z>>16, 2);
    }

    static inline sl::uint64_t lo_morton(const grid_point_t& gp) {
      uint32_t x = (-grid_coord_min + gp[0]); // Positive
      uint32_t y = (-grid_coord_min + gp[1]); // Positive
      uint32_t z = (-grid_coord_min + gp[2]); // Positive
      return
        sl::morton_bitops<uint64_t,3>::encoded(x&((1<<16)-1), 0) |
        sl::morton_bitops<uint64_t,3>::encoded(y&((1<<16)-1), 1) |
        sl::morton_bitops<uint64_t,3>::encoded(z&((1<<16)-1), 2);
    }

    inline bool operator()(const grid_point_t& x, const grid_point_t& y) const {
      bool result = false;

      const sl::uint64_t hi_x = hi_morton(x);
      const sl::uint64_t hi_y = hi_morton(y);
      if (hi_x == hi_y) {
        result = lo_morton(x)<lo_morton(y);
      } else {
        result = hi_x<hi_y;
      }
      return result;
    }
  };
  
} // namespace cbdam 

#endif // CBDAM_GRID_POINT_HPP

#ifndef CBDAM_GRID_POINT_IPP
#define CBDAM_GRID_POINT_IPP

namespace cbdam {

} // namespace cbdam 

#endif // CBDAM_GRID_POINT_IPP
