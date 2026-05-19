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
#ifndef CBDAM_COORDINATE_TRANSFORM_HPP
#define CBDAM_COORDINATE_TRANSFORM_HPP

#include <vic/cbdam/base/grid_point.hpp>
#include <sl/rigid_body_map.hpp>
#include <sl/linear_map_factory.hpp>
#include <cassert>

// namespace vic {  namespace geo {
namespace cbdam {
  
  /**
   *
   */
  class coordinate_transform {
  public:
    typedef sl::point2d         point2d_t;
    typedef sl::point3d         point3d_t;
    typedef sl::column_vector3d vector3d_t;
    typedef sl::aabox2d	  aabox_t;
    typedef sl::rigid_body_map3d rigid_body_map3d_t;
  protected:
    aabox_t       bounding_rectangle_;
    bool          is_planar_;
    std::size_t   root_count_;

  public:
    coordinate_transform();

    virtual ~coordinate_transform() { }

    // true: 2d planar; false 2d spherical

  public: // Params

    bool is_planar() const;

    std::size_t root_count() const;
  
    const aabox_t& bounding_rectangle() const;

    virtual coordinate_transform* clone() const = 0;

  public: // 3D to 3D

    virtual point3d_t xyz_on_ground(const point3d_t&xyz) const = 0;

    rigid_body_map3d_t xyz_local_to_global_from_xyz(const point3d_t& xyz) const;

    virtual double altitude_from_xyz(const point3d_t& xyz) const = 0;

  public: // Parametric to 3D

    virtual point3d_t  xyz_from_uvh(const point3d_t& uvh) const = 0;

    virtual vector3d_t up_from_uvh(const point3d_t& uvh) const = 0;

    virtual vector3d_t north_from_uvh(const point3d_t& uvh) const = 0;

    virtual vector3d_t east_from_uvh(const point3d_t& uvh) const = 0;

  public: // 3D to parametric

    virtual point3d_t uvh_from_xyz(const point3d_t& xyz) const = 0;

    vector3d_t up_from_xyz(const point3d_t& uvh) const;

    vector3d_t north_from_xyz(const point3d_t& uvh) const;

    vector3d_t east_from_xyz(const point3d_t& uvh) const;

  public: // Grid to paramtric

    virtual point2d_t  uv_from_grid(const grid_point_t& gp) const = 0;

    point3d_t  uvh_from_grid(const grid_point_t& gp, double h=0) const;
  
    vector3d_t up_from_grid(const grid_point_t& gp) const;

    vector3d_t north_from_grid(const grid_point_t& gp) const;

    vector3d_t east_from_grid(const grid_point_t& gp) const;

  public: // Grid to xyz

    virtual point3d_t  xyz_from_grid(const grid_point_t& gp, double h=0.0) const = 0;

  public: // Distance / boxes

    virtual double uv_distance_between(const point2d_t& p0, const point2d_t& p1) const = 0;

    virtual void uv_box_containing(std::vector<aabox_t>& bv,
				   const point2d_t& p0, const point2d_t& p1, const point2d_t& p2, const point2d_t& p3) const = 0;

  };

  class planar_coordinate_transform: public coordinate_transform {
  public:
    typedef sl::point2d point2d_t;
    typedef sl::point3d point3d_t;
    typedef sl::aabox2d aabox_t;
    typedef sl::column_vector2d vector2d_t;
    typedef sl::column_vector3d vector3d_t;

  protected:
    sl::vector2d  scale_;

  public:
    planar_coordinate_transform(const aabox_t& b);

    virtual ~planar_coordinate_transform();

  public: // 3D to 3D

    virtual point3d_t xyz_on_ground(const point3d_t&xyz) const;

    virtual double altitude_from_xyz(const point3d_t& xyz) const;

  public: // Parametric to 3D

    virtual point3d_t  xyz_from_uvh(const point3d_t& uvh) const;

    virtual vector3d_t up_from_uvh(const point3d_t& uvh) const;

    virtual vector3d_t north_from_uvh(const point3d_t& uvh) const;

    virtual vector3d_t east_from_uvh(const point3d_t& uvh) const;

  public: // 3D to parametric

    virtual point3d_t uvh_from_xyz(const point3d_t& xyz) const;

    // FIXME orientation vectors?

  public: // Grid to paramtric
    virtual point2d_t uv_from_grid(const grid_point_t& gp) const;

    virtual point3d_t xyz_from_grid(const grid_point_t& gp, double h=0.0) const;

    virtual double uv_distance_between(const point2d_t& p0, const point2d_t& p1) const;

    virtual void uv_box_containing(std::vector<aabox_t>& bv,
				   const point2d_t& p0, const point2d_t& p1, const point2d_t& p2, const point2d_t& p3) const;

    virtual coordinate_transform* clone() const;
  };

  class spherical_coordinate_transform : public coordinate_transform {
  public:
    typedef sl::point2d point2d_t;
    typedef sl::point3d point3d_t;
    typedef sl::column_vector3d vector3d_t;
    typedef sl::aabox2d	  aabox_t;

  protected:
    double radius_;

  public:
    spherical_coordinate_transform(double radius = 1.0);

    virtual ~spherical_coordinate_transform();

  public: // 3D to 3D

    virtual point3d_t xyz_on_ground(const point3d_t&xyz) const;

    virtual double altitude_from_xyz(const point3d_t& xyz) const;

  public: // Parametric to 3D

    virtual point3d_t  xyz_from_uvh(const point3d_t& uvh) const;

    virtual vector3d_t up_from_uvh(const point3d_t& uvh) const;

    virtual vector3d_t north_from_uvh(const point3d_t& uvh) const;

    virtual vector3d_t east_from_uvh(const point3d_t& uvh) const;

  public: // 3D to parametric

    virtual point3d_t uvh_from_xyz(const point3d_t& xyz) const;

    // FIXME orientation vectors?

  public: // Grid to paramtric

     virtual point2d_t uv_from_grid(const grid_point_t& gp) const;

    virtual point3d_t xyz_from_grid(const grid_point_t& gp, double h=0.0) const;
 
    virtual double uv_distance_between(const point2d_t& p0, const point2d_t& p1) const;

    virtual void uv_box_containing(std::vector<aabox_t>& bv,
				   const point2d_t& p0, const point2d_t& p1, const point2d_t& p2, const point2d_t& p3) const;

    virtual coordinate_transform* clone() const;

    void set_radius(double x);
  
    double radius() const;

  };

  class cylindrical_coordinate_transform : public spherical_coordinate_transform {
  public:
    typedef sl::point2d point2d_t;
    typedef sl::point3d point3d_t;
    typedef sl::column_vector3d vector3d_t;
    typedef sl::aabox2d	aabox_t;

  public:
    cylindrical_coordinate_transform(double radius = 1.0);

    virtual ~cylindrical_coordinate_transform();

  public: // 3D to parametric

    // use same function of spherical

  public: // Grid to paramtric
    
    virtual point2d_t uv_from_grid(const grid_point_t& gp) const;

    virtual coordinate_transform* clone() const;

  };

} // namespace cbdam
  // } // namespace geo } // namespace vic

#endif // CBDAM_COORDINATE_TRANSFORM_HPP

#ifndef CBDAM_COORDINATE_TRANSFORM_IPP
#define CBDAM_COORDINATE_TRANSFORM_IPP

// namespace vic {  namespace geo {
namespace cbdam {  

  inline coordinate_transform::coordinate_transform() {
    bounding_rectangle_ = (aabox_t(point2d_t(0.0,0.0),
				   point2d_t(1.0,1.0)));
  }

  inline const coordinate_transform::aabox_t& coordinate_transform::bounding_rectangle() const {
    return bounding_rectangle_;
  }

  inline std::size_t coordinate_transform::root_count() const {
    return root_count_;
  }

  inline bool coordinate_transform::is_planar() const {
    return is_planar_;
  }

  inline coordinate_transform::rigid_body_map3d_t coordinate_transform::xyz_local_to_global_from_xyz(const point3d_t& xyz) const {
    point3d_t O = xyz_on_ground(xyz);
    vector3d_t X = east_from_xyz(O);
    vector3d_t Y = north_from_xyz(O);
    vector3d_t Z = up_from_xyz(O);
    return rigid_body_map3d_t(sl::matrix4d(X[0], Y[0], Z[0], O[0],
					   X[1], Y[1], Z[1], O[1],
					   X[2], Y[2], Z[2], O[2],
					    0.0,  0.0,  0.0, 1.0)); 
  }

  inline point3d_t  coordinate_transform::uvh_from_grid(const grid_point_t& gp, double h) const {
    point2d_t uv = uv_from_grid(gp);
    return point3d_t(uv[0], uv[1], h);
  }
  
  inline vector3d_t coordinate_transform::up_from_grid(const grid_point_t& gp) const {
    point3d_t uvh = uvh_from_grid(gp, 0);
    return up_from_uvh(uvh);
  }
  
  inline vector3d_t coordinate_transform::north_from_grid(const grid_point_t& gp) const {
    point3d_t uvh = uvh_from_grid(gp, 0);
    return north_from_uvh(uvh);
  }
  
  inline vector3d_t coordinate_transform::east_from_grid(const grid_point_t& gp) const {
    point3d_t uvh = uvh_from_grid(gp, 0);
    return east_from_uvh(uvh);
  }
 
 inline vector3d_t coordinate_transform::up_from_xyz(const point3d_t& xyz) const {
    point3d_t uvh = uvh_from_xyz(xyz);
    return up_from_uvh(uvh);
  }
  
  inline vector3d_t coordinate_transform::north_from_xyz(const point3d_t& xyz) const {
    point3d_t uvh = uvh_from_xyz(xyz);
    return north_from_uvh(uvh);
  }
  
  inline vector3d_t coordinate_transform::east_from_xyz(const point3d_t& xyz) const {
    point3d_t uvh = uvh_from_xyz(xyz);
    return east_from_uvh(uvh);
  }

  ///////////////////// planar //////////////////

  inline planar_coordinate_transform::planar_coordinate_transform(const aabox_t& x) {
    bounding_rectangle_ = x;
    vector2d_t d = bounding_rectangle_.diagonal();
    scale_ = vector2d_t(d[0]/double(max_grid_coord()-min_grid_coord()),
			d[1]/double(max_grid_coord()-min_grid_coord()));

    root_count_ = 1;
    is_planar_ = true;
  }

  inline planar_coordinate_transform::~planar_coordinate_transform() {

  }
  
  inline point3d_t planar_coordinate_transform::xyz_on_ground(const point3d_t&xyz) const {
    return point3d_t(xyz[0], xyz[1], 0.0);
  }

  inline double planar_coordinate_transform::altitude_from_xyz(const point3d_t& xyz) const {
    return xyz[2];
  }

  inline point3d_t  planar_coordinate_transform::xyz_from_uvh(const point3d_t& uvh) const {
    // identity
    return uvh;
  }

  inline vector3d_t planar_coordinate_transform::up_from_uvh(const point3d_t& /*uvh*/) const {
    return vector3d_t(0.0, 0.0, 1.0);
  }

  inline vector3d_t planar_coordinate_transform::north_from_uvh(const point3d_t& /*uvh*/) const {
    return vector3d_t(0.0, 1.0, 0.0);
  }

  inline vector3d_t planar_coordinate_transform::east_from_uvh(const point3d_t& /*uvh*/) const {
    return vector3d_t(1.0, 0.0, 0.0);
  }

  inline point3d_t planar_coordinate_transform::uvh_from_xyz(const point3d_t& xyz) const {
    // identity
    return xyz;
  }

  inline planar_coordinate_transform::point2d_t planar_coordinate_transform::uv_from_grid(const grid_point_t& gp) const {
    return point2d_t(bounding_rectangle_[0][0]+double(gp[0]-min_grid_coord())*scale_[0],
		     bounding_rectangle_[0][1]+double(gp[1]-min_grid_coord())*scale_[1]);
  }

  inline planar_coordinate_transform::point3d_t planar_coordinate_transform::xyz_from_grid(const grid_point_t& gp, double h) const {
    // FIXME centered!
#if 0
    return point3d_t(0.0 + double(gp[0])*scale_[0],
		     0.0 + double(gp[1])*scale_[1],
		     0.0 + h);
#else
    return point3d_t(bounding_rectangle_[0][0] + double(gp[0]-min_grid_coord())*scale_[0],
		     bounding_rectangle_[0][1] + double(gp[1]-min_grid_coord())*scale_[1],
		     0.0 + h);
#endif
  }

  inline double planar_coordinate_transform::uv_distance_between(const point2d_t& p0, const point2d_t& p1) const {
    return (p0-p1).two_norm();
  }
      
  inline void planar_coordinate_transform::uv_box_containing(std::vector<aabox_t>& bv,
							     const point2d_t& p0, const point2d_t& p1, const point2d_t& p2, const point2d_t& p3) const {
    aabox_t b;
    b.to(p0);
    b.merge(p1);
    b.merge(p2);
    b.merge(p3);

    bv.push_back(b);
  }

  inline coordinate_transform* planar_coordinate_transform::clone() const {
    return  new planar_coordinate_transform(bounding_rectangle_);
  }

  ///////////////////// spherical //////////////////
  inline spherical_coordinate_transform::spherical_coordinate_transform(double radius) :
    radius_(radius) {
    bounding_rectangle_ = aabox_t(point2d_t(-180.0, -90), point2d_t(180.0, 90.0));
    root_count_ = 6;
    is_planar_ = false;
  }

  inline spherical_coordinate_transform::~spherical_coordinate_transform() {

  }

  inline coordinate_transform* spherical_coordinate_transform::clone() const {
    spherical_coordinate_transform* x = new spherical_coordinate_transform;
    x->set_radius(this->radius_);
    return x;
  }

  inline double spherical_coordinate_transform::radius() const {
    return radius_;
  }

  inline void spherical_coordinate_transform::set_radius(double r) {
    radius_ = r;
  }

  inline point3d_t spherical_coordinate_transform::xyz_on_ground(const point3d_t&xyz) const {
    return point3d_t(0,0,0) + radius_ * as_vector(xyz).ok_normalized();
  }

  inline double spherical_coordinate_transform::altitude_from_xyz(const point3d_t& xyz) const {
    return as_vector(xyz).two_norm() - radius_;
  }

  inline point3d_t spherical_coordinate_transform::xyz_from_uvh(const point3d_t& uvh) const {
    return point3d_t(0,0,0) + (radius_ + uvh[2]) * up_from_uvh(uvh);
  }

  inline vector3d_t spherical_coordinate_transform::up_from_uvh(const point3d_t& uvh) const {
    // uv to sin cos: lon = uvh[0]; lat = uvh[1];
    static const double deg_2_rad = 0.01745329251994329576;
    double lon = uvh[0] * deg_2_rad;
    double lat = uvh[1] * deg_2_rad;

    const double cos_lat = std::cos(lat);
    return vector3d_t(std::sin(lon) * cos_lat, 
		      std::sin(lat),
		      std::cos(lon) * cos_lat);
  }

  inline vector3d_t spherical_coordinate_transform::north_from_uvh(const point3d_t& uvh) const {
    static const double deg_2_rad = 0.01745329251994329576;
    double lon = uvh[0] * deg_2_rad;
    double lat = uvh[1] * deg_2_rad;

    const double sin_lat = std::sin(lat);
    return vector3d_t(std::sin(lon) * (-sin_lat),
		      std::cos(lat),
		      std::cos(lon) * (-sin_lat));		      
  }

  inline vector3d_t spherical_coordinate_transform::east_from_uvh(const point3d_t& uvh) const {
    static const double deg_2_rad = 0.01745329251994329576;
    double lon = uvh[0] * deg_2_rad;

#if 0
    return north_from_uvh(uvh).cross(up_from_uvh(uvh));
#else
    // east vector normalized (/cos_lat)
    return vector3d_t(std::cos(lon),
		      0.0,
		      -std::sin(lon));
#endif
  }

  inline point3d_t spherical_coordinate_transform::uvh_from_xyz(const point3d_t& xyz) const {
    const double rad_2_deg = 57.29577951308232087679; 
    double d = std::sqrt(xyz[0]*xyz[0] + xyz[1]*xyz[1] + xyz[2]*xyz[2]);

    assert(d>0);
    if (d==0) d = 1;

    const double nx = xyz[0]/d;
    const double ny = xyz[1]/d;
    const double nz = xyz[2]/d;

    const double lon = std::atan2(nx, nz); 
    const double lat = std::atan2(ny, std::sqrt(nx*nx + nz*nz));

    return point3d_t(lon * rad_2_deg,          //u in [-180..180]
		     lat * rad_2_deg,          //v in [-90..90]
		     d - radius_);
  }

  inline spherical_coordinate_transform::point3d_t spherical_coordinate_transform::xyz_from_grid(const grid_point_t& gp, double h) const {
    const vector3d_t up = up_from_grid(gp);
    return point3d_t(0.0, 0.0, 0.0) + (radius_+h) * up;
  }

  inline spherical_coordinate_transform::point2d_t spherical_coordinate_transform::uv_from_grid(const grid_point_t& gp) const {
    const double rad_2_deg = 57.29577951308232087679; 
    // remap to normal
    const vector3d_t up = sl::column_vector3d(double(gp[0])/double(max_grid_coord()), 
					      double(gp[1])/double(max_grid_coord()), 
					      double(gp[2])/double(max_grid_coord())).ok_normalized();

    // compute lat lon
    const double& x = up[0];
    const double& y = up[1];
    const double& z = up[2];

    const double xz_dist = std::sqrt(x*x + z*z);

    const double lon = std::atan2(x, z); 
    const double lat = std::atan2(y, xz_dist);

    return point2d_t(lon * rad_2_deg,  //u in [-180..180]
		     lat * rad_2_deg); //v in [-90..90]
  }

  inline double spherical_coordinate_transform::uv_distance_between(const point2d_t& p0, const point2d_t& p1) const {
    vector2d_t v = p1 - p0;

    // manage longitude wraparound
    if (v[0] > 180.0) {
      v[0] = 360.0 - v[0];
    } else if (v[0] < -180.0) {
      v[0] = 360.0 + v[0];
    }

    return v.two_norm();
  }
      
  inline void spherical_coordinate_transform::uv_box_containing(std::vector<aabox_t>& bv,
								const point2d_t& p0, const point2d_t& p1, const point2d_t& p2, const point2d_t& p3) const {
    aabox_t b;
    b.to(p0);
    b.merge(p1);
    b.merge(p2);
    b.merge(p3);

    if (b.diagonal()[0] <= 180.0) {
      bv.push_back(b);
    } else {
      // wrap around
      // split box in 2 smaller boxes one near lon -180, one near lon 180
      point2d_t p_lo = b[0];
      point2d_t p_hi = b[1];

      // box from -180.0 to lo
      aabox_t b_lo;
      b_lo.to(point2d_t(-180.0,     p_lo[1]));	// lower left
      b_lo.merge(point2d_t(p_lo[0], p_hi[1]));	// upper right

      // box from hi to 180
      aabox_t b_hi;
      b_hi.to(point2d_t(p_hi[0], p_lo[1]));	// lower left
      b_hi.merge(point2d_t(180.0,   p_hi[1]));	// upper right

      // push only not empty boxes
      if (b_lo.diagonal()[0]*b_lo.diagonal()[1] != 0) {
	bv.push_back(b_lo);
      }
      if (b_hi.diagonal()[0]*b_hi.diagonal()[1] != 0) {
	bv.push_back(b_hi);
      }
    }
  }

  ///////////////////// cylindrical_coordinate_transform  //////////////////

  inline cylindrical_coordinate_transform::cylindrical_coordinate_transform(double radius) :
    spherical_coordinate_transform(radius) {
    root_count_ = 8;
  }
  
  inline cylindrical_coordinate_transform::~cylindrical_coordinate_transform() {

  }

  inline point2d_t cylindrical_coordinate_transform::uv_from_grid(const grid_point_t& gp) const {
    // return latlon in deg 

    // 8 quad roots with half_root_w

    //  | +z1 | +x1 | -z1 | -x1 |       +y
    //  ------------0------------
    //  | +z0 | +x0 | -z0 | -x0 |       -y

    // lat:  -90 at begin of -y    90 at end of +y
    // lon: -180 at begin of +z   180 at end of -x

    static const int root_w = max_grid_coord();
    static const int half_root_w = root_w / 2;
    static const double grid_to_deg = 90.0 / double(root_w);
    point2d_t lonlat_deg;

    // compute lat in deg
    lonlat_deg[1] = double(gp[1]) * grid_to_deg;

    // compute lon in grid unit unrolling zx boundary
    if (gp[2] == half_root_w) {
      // +Z
      lonlat_deg[0] = double(-2 * root_w) + double(gp[0]+half_root_w);
    } else if (gp[0] == half_root_w) {
      // +X
      lonlat_deg[0] = double(-1 * root_w) + double(-gp[2]+half_root_w);
    } else if (gp[2] == -half_root_w) {
      // -Z 
      lonlat_deg[0] = double(0 * root_w) + double(-gp[0]+half_root_w);
    } else if (gp[0] == -half_root_w) {
      // -X
      lonlat_deg[0] = double(1 * root_w) + double(gp[2]+half_root_w);
    } else {
      SL_TRACE_OUT(-1) << "coordinate not lying on a canonical root plane" << gp << std::endl;
    }
    // convert to rad
    lonlat_deg[0] *= grid_to_deg;

    //    std::cerr << "gp " << gp ", uv " << lonlat_deg;
#if 0
    if (!(lonlat_deg[0] >= -180.0 && lonlat_deg[0] <= 180.0)) {
      std::cerr << "lonlat_deg out of bounds 0: " << lonlat_deg[0] << " " << lonlat_deg[1] << " gp " << gp << " root_w " << root_w << std::endl;
    }
    if (!(lonlat_deg[1] >= -90.0 && lonlat_deg[1] <= 90.0)) {
      std::cerr << "lonlat_deg out of bounds 1: " << lonlat_deg[0] << " " << lonlat_deg[1] <<  " gp " << gp << " root_w " << root_w <<std::endl;
    }

    //    assert(lonlat_deg[0] >= -180.0 && lonlat_deg[0] <= 180.0);
    //    assert(lonlat_deg[1] >= -90.0 && lonlat_deg[1] <= 90.0);
#endif

    return lonlat_deg;
  }

  inline coordinate_transform* cylindrical_coordinate_transform::clone() const {
    return new cylindrical_coordinate_transform(radius_);
  }

} // namespace cbdam
  //  } // namespace geo } // namespace vic

#endif // CBDAM_COORDINATE_TRANSFORM_IPP
