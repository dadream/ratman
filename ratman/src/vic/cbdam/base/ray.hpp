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
#ifndef CBDAM_RAY_HPP
#define CBDAM_RAY_HPP

#include <vic/cbdam/base/config.hpp>
#include <sl/fixed_size_vector.hpp>

namespace cbdam {

  /**
   * Ray starts from a point and has a length and a direction.
   * Is used to find triangles intersection. 
   * Intersection is checked from t_near to t_far. t_near 0 means origin.
   * If a ray intersect a triangle, its t_far is set to 
   * origin-(triangle-intersection) distance.
   */
  class ray {
  public:
    /**
     * Ray starts from an origin and has a direction and a length.
     */
    ray(const point3d_t& origin, const vector3d_t& dir, double t_near = 0.0, double t_far = 1.0);

    ray(const point3d_t& origin, const point3d_t& extremity, double t_near = 0.0, double t_far = 1.0);

    ~ray();

    /**
     * Check if this ray intersect triangle identified by v0,v1,v2.
     * If ray intersects triangle, its new length is its distance from the triangle.
     * t is the value that inserted in the ray gives intersection
     * if normal != 0, it will contain the normal to the triangle at the intersection, if it exists.
     * u, v will contain barycentric coords of the intersection
     */
    void closest_triangle_intersection(const point3d_t& v0, const point3d_t& v1, const point3d_t& v2,
				        bool& hit, double& t,  double& u, double &v, normald_t* normal = 0);

    /**
     * Tests if ray intersect a sphere.Does not modify length of the ray.
     * t is the value that inserted in the ray gives intersection
     */
    bool sphere_intersection(const point3d_t& center, double radius, double& t) const;

    /** 
     * The point obtained from ray equation : r(t) = origin + t * direction
     */
    const point3d_t point_at(double t) const;

    CBDAM_RW_ACCESSOR(point3d_t, origin);
    CBDAM_RW_ACCESSOR(vector3d_t, direction);
    CBDAM_RW_ACCESSOR(double, t_near);
    CBDAM_RW_ACCESSOR(double, t_far);

  protected:
    point3d_t	origin_;
    vector3d_t	direction_;
    double	t_near_;
    double	t_far_;
  };


} // namespace cbdam 

#endif // CBDAM_RAY_HPP

#ifndef CBDAM_RAY_IPP
#define CBDAM_RAY_IPP

namespace cbdam {

  inline ray::ray(const point3d_t& origin, const vector3d_t& dir, double t_near, double t_far) :
    origin_(origin), direction_(dir), t_near_(t_near), t_far_(t_far) {

  }

  inline ray::ray(const point3d_t& origin, const point3d_t& extremity, double t_near, double t_far) :
    origin_(origin), t_near_(t_near), t_far_(t_far)  {
    direction_ = extremity - origin;
  }

  inline ray::~ray() {

  }

  static const double epsilon = 1E-05;

  inline  void ray::closest_triangle_intersection(const point3d_t& v0, const point3d_t& v1, const point3d_t& v2,
						  bool& hit, double& t, double& u, double &v, normald_t* normal) {
    // Moller Trumbore 97 algorithm
    // compute u ,v barycentric coordinates of the intersection.
    // ray		r(t)  = o + d*t
    // triangle point	p(u,v)= (1-u-v)*v0 + u*v1 + v*v2
    // intersection	o + d*t=(1-u-v)*v0 + u*v1 + v*v2
    // ( -d  | v1-v0 | v2-v0 ) * ( | t u v | ) = o - v0;	M = ( -d  | v1-v0 |  v2-v0 )
    // ( | t u v | ) = M^-1 * ( o - v0 )

    // compute M determinant
    vector3d_t e1 = v1 - v0;
    vector3d_t e2 = v2 - v0;
    vector3d_t p = direction_.cross( e2 );
    double det = e1.dot( p );
#define CULL_BACKFACE	0
#if CULL_BACKFACE
    if ( det < epsilon ) {
      hit = false;
      return;
    }

    // find and check first barycentric coordinate
    vector3d_t s = origin_ - v0;
    u = s.dot( p ) ;
    if ( u < 0.0f || u > det ) {
      hit = false;
      return;
    }

    // find and check second barycentric coordinate
    vector3d_t q = s.cross( e1 );
    v =  direction_.dot( q );
    if ( v < 0.0f || u + v > det ) {
      hit = false;
      return;
    }

    double inv_det = 1 / det;

#else
    // check if determinant is equal to 0
    if ( det > -epsilon && det < epsilon ) {
      hit = false;
      return;
    }

    double inv_det = 1 / det;

    // find and check first barycentric coordinate
    vector3d_t s = origin_ - v0;
    u = inv_det * s.dot( p ) ;
    if ( u < 0.0f || u > 1.0f ) {
      hit = false;
      return;
    }

    // find and check second barycentric coordinate
    vector3d_t q = s.cross( e1 );
    v = inv_det * direction_.dot( q );
    if ( v < 0.0f || u + v > 1.0f ) {
      hit = false;
      return;
    }

#endif

    // intersect: find third barycentric coordinate
    double t_check = inv_det * e2.dot( q );

    // intersection distance < old distance ?
    if ( t_check < t_near_ || t_check > t_far_) {
      // ray t_far does not arrive to the triangle
      hit = false;
    } else {
      // found new intersection: decrease t_far
      t_far_ = t_check;
      t = t_check;
      hit = true;

      // compute triangle normal only if requested
      if ( normal != 0 ) {
	sl::fixed_size_vector<sl::row_orientation,3, double> n =  e1.normal( e2 );
	*normal = normald_t( n[ 0 ], n[ 1 ], n[ 2 ] ).ok_normalized();
      }
    } 

  }

  inline bool ray::sphere_intersection(const point3d_t& center, double radius, double& t) const {
    // FIXME: IMPLEMENTATION DO NOT CONSIDER T_NEAR
    // vector center - ray-origin
    vector3d_t l = center - origin_;

    double d = l.dot( direction_ );
    double l_squared = l.dot( l );
    double r_squared = radius * radius;


    // ray goes away from the center and the point is outside the sphere
    if ( d < 0 && l_squared > r_squared )
      return false;

    // distance of the ray from the sphere center
    double m_squared = l_squared - d * d;

    // distance from the center bigger than radius ?
    if ( m_squared > r_squared )
      return false;

    // Intersect!
    // distance of nearest point of the ray from the center from the sphere
    double q = sqrt( r_squared - m_squared );

    if ( l_squared > r_squared ) {
      // origin is outside the sphere, take nearest intersection
      // t is d - q;
      // check if ray t_far does arrive to the sphere surface
      if ( t_far_ < d - q ) {
	return false;
      } else {
	t = d - q;
      }

    } else {
      // origin is inside
      t = d + q;
    }

    // ray t_far updated only on triangle intersection     
    return true;
  }  

  inline const point3d_t ray::point_at(double t) const {
    return origin_ + t * direction_;
  }

} // namespace cbdam 

#endif // CBDAM_RAY_IPP
