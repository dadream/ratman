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
#ifndef GEO_TRANSFORM_HPP
#define GEO_TRANSFORM_HPP

#include <vic/geo/builder/geo_utility.hpp>
#include <sl/serializer.hpp>
#include <gdal_alg.h>
#include <string>

namespace vic {
  namespace geo {
    class geo_transform {
    public:
      typedef geo_transform this_t;

    protected:
      std::string src_srs_;
      geo_matrix  src_matrix_;
      std::string dst_srs_;
      geo_matrix  dst_matrix_;
      double      max_error_;

    protected:
      void *precise_reprojector_;
      void *approximate_reprojector_;

    public:
      geo_transform();
      geo_transform(const std::string& src_proj, 
		    const std::string& dst_proj, 
		    double max_error = 0.25);

      geo_transform(const geo_transform& other);

      geo_transform& operator= (const geo_transform& other);

      virtual ~geo_transform();
  
    public: // Serialization
    
      void store_to(sl::output_serializer& s) const;
    
      void retrieve_from(sl::input_serializer& s);
  
    public: // Accessors

      const std::string& src_srs() const { return src_srs_; }
      const geo_matrix&  src_matrix() const { return src_matrix_; }
      const std::string& dst_srs() const { return dst_srs_; }
      const geo_matrix&  dst_matrix() const { return dst_matrix_; }
      const double&      max_error() const { return max_error_; }

    public:

      inline GDALTransformerFunc get_trasformation() const { 
	return transform; 
      }

      inline this_t* to_pointer() const { 
	return const_cast<this_t*>(this);
      }

      void create(const std::string& src_proj, const std::string& dst_proj, double max_error = 0.25);
      void destroy();

      inline void set_src_geo_matrix(const geo_matrix& gt) {
	src_matrix_=gt;
      }

      inline void set_dst_geo_matrix(const geo_matrix& gt) {
	dst_matrix_=gt;
      }

    protected:
      static int transform(void *arg, int dts, int n, double *x, double *y, double *z, int *ps);
      

    };

  } // namespace geo
} // namespace vic

#endif
