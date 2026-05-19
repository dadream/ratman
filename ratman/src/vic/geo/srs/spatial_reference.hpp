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
#ifndef VIC_GEO_SRS_SPATIAL_REFERENCE_HPP
#define VIC_GEO_SRS_SPATIAL_REFERENCE_HPP

#include <sl/fixed_size_point.hpp>
#include <string>

namespace vic {

  namespace geo {
    
    namespace srs {

      class spatial_reference_transformation;

      /** 
       *  A OpenGIS Spatial Reference System
       *  currently, just a wrapper to OGRSpatialReference
       */
      class spatial_reference {
      public:
	typedef sl::fixed_size_point<2, double> point2d_t;
	typedef sl::fixed_size_point<3, double> point3d_t;
      protected:
	friend class spatial_reference_transformation;
	void * srs_;
	mutable spatial_reference_transformation* from_WGS84_lonlat_;
	mutable spatial_reference_transformation* to_WGS84_lonlat_;
      public:
	
	spatial_reference(const char* txt=0);

	~spatial_reference();

	spatial_reference(const spatial_reference& other);

	spatial_reference& operator=(const spatial_reference& other);

	void reset(const char* txt);

	void clear();

	bool is_valid() const;

	std::string description() const;

      public:

	/**
	 *  True iff this is a coordinate system expressed in unprojected coordinates (angular
	 *  units).
	 */
	bool is_geographic() const;

	/**
	 *  True iff this is a coordinate system expressed in projected coordinates (linear
	 *  units).
	 */
	bool is_projected() const;

	/**
	 *  True iff this is a coordinate system that can't be related to world coordinates.
	 */
	bool is_local() const;

      public:

	/// the value to multiply by linear distances to transform them to meters or 1.0 if not found
	double linear_units() const;

	/// the value to multiply the value to multiply by angular distances to transform them to radians or 180/Pi if not found
	double angular_units() const;

	/// The spheroid's semi-minor axis, or WGS84 value if it can't be found.
	double spheroid_semi_minor() const;

	/// The spheroid's semi-major axis, or WGS84 value if it can't be found.
	double spheroid_semi_major() const;

	/// The spheroid's inverse flattening, or WGS84 value if it can't be found.
	double spheroid_inverse_flattening() const;

	/// The spheroid's average radius, or WGS84 value if it can't be found.
	double spheroid_average_radius() const;

      public: // Conversions to/from WGS84
	
	/// The WGS84 global geodetic reference system (lonlat)
	static const spatial_reference& WGS84_lonlat();

	const spatial_reference_transformation& from_WGS84_lonlat() const;

	const spatial_reference_transformation& to_WGS84_lonlat() const;

	/**
	 *  Transform coordinates from this to WGS84 reference system.
	 *  If the transformation is invalid or impossible for the given
	 *  point, the returned point is set to an invalid value
	 */
	point2d_t to_WGS84_lonlat(const point2d_t& p) const;

	/**
	 *  Transform coordinates from this to WGS84 reference system.
	 *  If the transformation is invalid or impossible for the given
	 *  point, the returned point is set to an invalid value
	 */
	point3d_t to_WGS84_lonlat(const point3d_t& p) const;

	/**
	 *  Transform coordinates from WGS84 to this reference system.
	 *  If the transformation is invalid or impossible for the given
	 *  point, the returned point is set to an invalid value
	 */
	point2d_t from_WGS84_lonlat(const point2d_t& p) const;

	/**
	 *  Transform coordinates from WGS84 to this reference system.
	 *  If the transformation is invalid or impossible for the given
	 *  point, the returned point is set to an invalid value
	 */
	point3d_t from_WGS84_lonlat(const point3d_t& p) const;
	
      };

      /**
       *  Objects for transforming between 
       *  coordinate systems.
       */
      class spatial_reference_transformation {
      public:
	typedef sl::fixed_size_point<2, double> point2d_t;
	typedef sl::fixed_size_point<3, double> point3d_t;
      protected:
	const spatial_reference* source_;
	const spatial_reference* target_;
	void* srt_;
      public:

	/// Create a transformation from source to target. If successful, is_valid() is true
	spatial_reference_transformation(const spatial_reference* source=0,
					 const spatial_reference* target=0);

	/// Destroy transformation
	~spatial_reference_transformation();

	
	/// Create a copy of given transformation
	spatial_reference_transformation(const spatial_reference_transformation& other);

	/// Assign to a copy of given transformation
	spatial_reference_transformation& operator=(const spatial_reference_transformation& other);

	
	/// Assign to a transformation from source to target. If successful, is_valid() is true
	void reset(const spatial_reference* source,
		   const spatial_reference* target); 

	/// Clear transformation, resetting to an invalid state
	void clear();

	/// True iff current transformation is valid
	bool is_valid() const;

	/**
	 *  Transform coordinates from source to target reference system.
	 *  If the transformation is invalid or impossible for the given
	 *  point, the returned point is set to an invalid value
	 */
	point2d_t transformed(const point2d_t& p) const;

	/**
	 *  Transform coordinates from source to target reference system.
	 *  If the transformation is invalid or impossible for the given
	 *  point, the returned point is set to an invalid value
	 */
	point3d_t transformed(const point3d_t& p) const;

	/// True iff p is not the result of an invalid transformation
	static bool is_valid(const point2d_t& p);

	/// True iff p is not the result of an invalid transformation
	static bool is_valid(const point3d_t& p);
      };

    }

  }

}

#endif
