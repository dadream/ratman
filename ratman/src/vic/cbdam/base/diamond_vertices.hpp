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
#ifndef CBDAM_DIAMOND_VERTICES_HPP
#define CBDAM_DIAMOND_VERTICES_HPP

#include <vic/cbdam/base/reference_counted_cache.hpp>
#include <vic/cbdam/base/ray.hpp>
#include <sl/fixed_size_point.hpp>
#include <sl/fixed_size_vector.hpp>
#include <vector>
#include <limits.h>

namespace cbdam {
  
  class diamond_vertices: public  reference_counted {
  public:
    typedef sl::point3f                 point3_t;
    typedef sl::row_vector3f            normal_t;
    
  protected:
    std::vector<int32_t>       values_;
    std::vector<point3_t>      gl_points_;
    std::vector<normal_t>      gl_normals_;
    point3d_t		       diamond_center_;

  public:
    diamond_vertices(reference_counted_owner* owner = 0, int count = 0, bool use_normal = true) :
        reference_counted(owner) {
      values_.resize(count);
      gl_points_.resize(count);
      if (use_normal) {
        gl_normals_.resize(count);
      }
    }

    virtual ~diamond_vertices() {

    }


    inline std::vector<int32_t>& values() {
      return values_;
    }

    inline const std::vector<int32_t>& values() const {
      return values_;
    }  

    inline std::vector<point3_t>& gl_points() {
      return gl_points_;
    }

    inline const std::vector<point3_t>& gl_points() const {
      return gl_points_;
    }

    inline std::vector<normal_t>& gl_normals() {
      return gl_normals_;
    }

    inline const std::vector<normal_t>& gl_normals() const {
      return gl_normals_;
    }    

    inline point3d_t& diamond_center() {
      return diamond_center_;
    }

    inline const point3d_t& diamond_center() const {
      return diamond_center_;
    }

    std::pair<bool, double> patch_ray_intersection(const point3d_t& origin, const point3d_t& extremity, int patch_dim, normald_t *normal) const;
  };
  
} // namespace cbdam 

#endif // CBDAM_DIAMOND_VERTICES_HPP

#ifndef CBDAM_DIAMOND_VERTICES_IPP
#define CBDAM_DIAMOND_VERTICES_IPP

namespace cbdam {

  inline std::pair<bool, double> diamond_vertices::patch_ray_intersection(const point3d_t& origin, const point3d_t& extremity, int patch_dim, normald_t *normal) const {
    assert(gl_points_.size() == std::size_t((patch_dim+1)*(patch_dim+2)/2));
    ray r(as_point(origin-diamond_center_), as_point(extremity-diamond_center_));

    bool found = false;
    double t = 2.0; // returned t must be in 0..1
    
    int w = patch_dim;
    uint32_t count = 0;
    for(int y = 0; y < w; ++y) {
      for(int x = 0; x < w - y; ++x) {
	uint32_t count_next_row = count + w + 1 - y;
	point3_t p00 = gl_points_[count];
	point3_t p10 = gl_points_[count+1];
	point3_t p01 = gl_points_[count_next_row];

	// check upper triangle
	// *-*
	// |/
	// *
	//	r.closest_triangle_intersection(p00, p10, p01, hit, t, normal);
	bool ok = false;
	double u, v; // intersection barycentric coords
	point3d_t pd00(p00[0],p00[1],p00[2]);
	point3d_t pd10(p10[0],p10[1],p10[2]);
	point3d_t pd01(p01[0],p01[1],p01[2]);
	r.closest_triangle_intersection(pd00, pd01, pd10, ok, t, u, v, normal);	    
	found |= ok;
	if (x < w - y - 1) {
	  // check also the other triangle
	  //   *
	  //  /|
	  // *-*
	  point3_t p11 =gl_points_[count_next_row + 1];
	  point3d_t pd11(p11[0],p11[1],p11[2]);
	  
	  ok = false;
	  r.closest_triangle_intersection(pd11, pd01, pd10, ok, t, u, v, normal);	    	  
	  found |= ok;
	}
	++count;
      }
      ++count;
    }

    return std::make_pair(found, t);
  }

} // namespace cbdam 

#endif // CBDAM_DIAMOND_VERTICES_IPP
