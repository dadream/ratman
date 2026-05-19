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
#ifndef GEO_UTILITY_HPP
#define GEO_UTILITY_HPP

#include <sl/fixed_size_square_matrix.hpp>
#include <string>
#include <vector>

namespace vic {
  namespace geo {

    class geo_utility {
    public:
      static std::string proj2srs(const std::string& proj);
      static sl::fixed_size_array<6,double> gdal_array(const sl::matrix3d& m);

      static bool mkpath(const std::string& path);
      static bool has_dir(const std::string& path);
    public:
      static std::string clean_path(const std::string& path);
    }; // geo_utility


    class geo_matrix {
    public:

    protected:
      sl::matrix3d mat_;
      sl::matrix3d mat_inv_;

    public:
      geo_matrix();
      geo_matrix(double x0, double y0, double rx, double ry);
      geo_matrix(double *mat);

      virtual ~geo_matrix();

    public: // Serialization
    
      inline void store_to(sl::output_serializer& s) const {
	mat_.store_to(s);
      }
    
      void retrieve_from(sl::input_serializer& s) {
	mat_.retrieve_from(s);
	mat_inv_ = ~mat_;
      }
      
    public: 

      inline const sl::matrix3d& mat() const {
	return mat_;
      }

      inline const sl::matrix3d& mat_inv() const {
	return mat_inv_;
      }

 
    }; // geo_matrix

  } // namespace geo
} // namespace vic

#endif
