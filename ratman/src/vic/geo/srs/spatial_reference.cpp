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
#include <math.h> // HUGE_VAL

#include <vic/geo/srs/spatial_reference.hpp>
#include <ogr_spatialref.h>

namespace vic {

  namespace geo {

    namespace srs {

      spatial_reference::spatial_reference(const char* txt) {
	srs_ = 0;
	to_WGS84_lonlat_ = 0;
	from_WGS84_lonlat_ = 0;
	reset(txt);
      }

      spatial_reference::~spatial_reference() {
	clear();
      }

      spatial_reference::spatial_reference(const spatial_reference& other) {
	srs_ = 0;
	to_WGS84_lonlat_ = 0;
	from_WGS84_lonlat_ = 0;

	if (other.srs_) srs_ = static_cast<OGRSpatialReference*>(other.srs_)->Clone();
      }

      spatial_reference& spatial_reference::operator=(const spatial_reference& other) {
	if (srs_ != other.srs_) {
	  clear();

	  if (other.srs_) srs_ = static_cast<OGRSpatialReference*>(other.srs_)->Clone();
	}
	return *this;
      }

      void spatial_reference::reset(const char* txt) {
	clear();
	if (txt && txt[0] != '\0') {
	  OGRSpatialReference* osrs = new OGRSpatialReference();
	  if (osrs->SetFromUserInput(txt) == OGRERR_NONE) {
	    srs_ = osrs;
	  } else {
	    delete osrs;
	  }	  
	}
      }

      void spatial_reference::clear() {
	if (from_WGS84_lonlat_) delete from_WGS84_lonlat_;
	from_WGS84_lonlat_ = 0;
	if (to_WGS84_lonlat_) delete to_WGS84_lonlat_;
	to_WGS84_lonlat_ = 0;
	if (srs_) delete static_cast<OGRSpatialReference*>(srs_);
	srs_ = 0;
      }

      bool spatial_reference::is_valid() const {
	bool result = false;
	result = (srs_ != 0);
	return result;	
      }
      
      std::string spatial_reference::description() const {
	if (srs_) {
	  char* wkt;
	  static_cast<OGRSpatialReference*>(srs_)->exportToWkt(&wkt);
	  std::string result = std::string(wkt);
	  OGRFree(wkt);
	  return result;
	} else {
	  return std::string("INVALID");
	}
      }

      bool spatial_reference::is_geographic() const {
	bool result = false;
	if (srs_) {
	  result = static_cast<OGRSpatialReference*>(srs_)->IsGeographic();
	}
	return result;	
      }

      bool spatial_reference::is_projected() const {
	bool result = false;
	if (srs_) {
	  result = static_cast<OGRSpatialReference*>(srs_)->IsProjected();
	}
	return result;	
      }
      
      bool spatial_reference::is_local() const {
	bool result = false;
	if (srs_) {
	  result = static_cast<OGRSpatialReference*>(srs_)->IsLocal();
	}
	return result;	
      }
	
      double spatial_reference::linear_units() const {
	double result = 1.0;
	if (srs_) {
	  char *name;
	  result = static_cast<OGRSpatialReference*>(srs_)->GetLinearUnits(&name);
	}
	return result;	
      }

      double spatial_reference::angular_units() const {
	double result = 1.0;
	if (srs_) {
	  char *name;
	  result = static_cast<OGRSpatialReference*>(srs_)->GetAngularUnits(&name);
	}
	return result;	
      }

      double spatial_reference::spheroid_semi_minor() const {
	double result = 1.0;
	if (srs_) {
	  result = static_cast<OGRSpatialReference*>(srs_)->GetSemiMinor();
	}
	return result;	
      }

      double spatial_reference::spheroid_semi_major() const {
	double result = 1.0;
	if (srs_) {
	  result = static_cast<OGRSpatialReference*>(srs_)->GetSemiMajor();
	}
	return result;	
      }

      double spatial_reference::spheroid_average_radius() const {
	return 0.5 * (spheroid_semi_major() + spheroid_semi_minor());
      }

      double spatial_reference::spheroid_inverse_flattening() const {
	double result = 1.0;
	if (srs_) {
	  result = static_cast<OGRSpatialReference*>(srs_)->GetInvFlattening();
	}
	return result;	
      }

      const spatial_reference& spatial_reference::WGS84_lonlat() {
	static spatial_reference* result = new spatial_reference("EPSG:4326");
	return *result;
      }

      const spatial_reference_transformation& spatial_reference::from_WGS84_lonlat() const {
	if (from_WGS84_lonlat_ == 0) {
	  from_WGS84_lonlat_ = new spatial_reference_transformation(&WGS84_lonlat(), this);
	}
	return *from_WGS84_lonlat_;
      }
	
      const spatial_reference_transformation& spatial_reference::to_WGS84_lonlat() const {
	if (to_WGS84_lonlat_ == 0) {
	  to_WGS84_lonlat_ = new spatial_reference_transformation(this, &WGS84_lonlat());
	}
	return *to_WGS84_lonlat_;
      }

      spatial_reference::point2d_t spatial_reference::to_WGS84_lonlat(const point2d_t& p) const {
	return to_WGS84_lonlat().transformed(p);
      }
      
      spatial_reference::point3d_t spatial_reference::to_WGS84_lonlat(const point3d_t& p) const {
	return to_WGS84_lonlat().transformed(p);
      }
      
      spatial_reference::point2d_t spatial_reference::from_WGS84_lonlat(const point2d_t& p) const {
	return from_WGS84_lonlat().transformed(p);
      }
      
      spatial_reference::point3d_t spatial_reference::from_WGS84_lonlat(const point3d_t& p) const {
	return from_WGS84_lonlat().transformed(p);
      }

    } // namespace srs
  } // namespace geo
} // namespace vic 


namespace vic {

  namespace geo {

    namespace srs {
      
      spatial_reference_transformation::spatial_reference_transformation(const spatial_reference* source,
									 const spatial_reference* target) :
	source_(0), target_(0), srt_(0)
      {
	reset(source, target);
      }

      void spatial_reference_transformation::clear() {
	source_ = 0;
	target_ = 0;
	if (srt_) delete static_cast<OGRCoordinateTransformation*>(srt_);
	srt_ = 0;
      }

      void spatial_reference_transformation::reset(const spatial_reference* source,
						   const spatial_reference* target) {
	clear();
	if (source && target && source->is_valid() && target->is_valid()) {
	  OGRCoordinateTransformation* osrt = OGRCreateCoordinateTransformation(static_cast<OGRSpatialReference*>(source->srs_),
										static_cast<OGRSpatialReference*>(target->srs_));
	  if (osrt) {
	    source_ = source;
	    target_ = target;
	    srt_ = osrt;
	  }
	}
      }

      spatial_reference_transformation::~spatial_reference_transformation() {
	clear();
      }

      
      spatial_reference_transformation::spatial_reference_transformation(const spatial_reference_transformation& other) {
	source_ = 0;
	target_ = 0;
	srt_ = 0;
	reset(other.source_, other.target_);
      }

      spatial_reference_transformation& spatial_reference_transformation::operator=(const spatial_reference_transformation& other) {
	clear();
	reset(other.source_, other.target_);
	return *this;
      }

      bool spatial_reference_transformation::is_valid() const {
	return source_ && target_ && srt_;
      }

      static const double INVALID_VALUE = HUGE_VAL;

      spatial_reference_transformation::point2d_t spatial_reference_transformation::transformed(const point2d_t& p) const {
	point2d_t result(p[0], p[1]);
	int ok = (srt_) && static_cast<OGRCoordinateTransformation*>(srt_)->Transform(1,
										      &(result[0]),
										      &(result[1]));
	if (!ok) {
	  result[0] = INVALID_VALUE;
	  result[1] = INVALID_VALUE;
	}
	return result;
      }

      spatial_reference_transformation::point3d_t spatial_reference_transformation::transformed(const point3d_t& p) const {
	point3d_t result(p[0], p[1], p[2]);
	int ok = (srt_) && static_cast<OGRCoordinateTransformation*>(srt_)->Transform(1,
										      &(result[0]),
										      &(result[1]),
										      &(result[2]));
	if (!ok) {
	  result[0] = INVALID_VALUE;
	  result[1] = INVALID_VALUE;
	  result[2] = INVALID_VALUE;
	}
	return result;
      }
      
      bool spatial_reference_transformation::is_valid(const point3d_t& p) {
	return (p[0] != INVALID_VALUE) && (p[1] != INVALID_VALUE) && (p[2] != INVALID_VALUE);
      }
      
      bool spatial_reference_transformation::is_valid(const point2d_t& p) {
	return (p[0] != INVALID_VALUE) && (p[1] != INVALID_VALUE);
      }
    
    } // namespace srs
  } // namespace geo
} // namespace vic 
