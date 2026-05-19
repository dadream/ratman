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
#include <vic/geo/builder/geo_transform.hpp>
// GDAL include
#include <gdal_priv.h>
#include <cpl_string.h>

namespace vic {
  namespace geo {

    geo_transform::geo_transform() :
      precise_reprojector_(0),
      approximate_reprojector_(0) {
    }

    geo_transform::geo_transform(const std::string& src_proj, const std::string& dst_proj, double max_error) :
      precise_reprojector_(0),
      approximate_reprojector_(0) {
      create(src_proj, dst_proj, max_error);
    }

    geo_transform::~geo_transform() {
      destroy();
    }

    geo_transform::geo_transform(const geo_transform& other) :
      precise_reprojector_(0),
      approximate_reprojector_(0) {
      src_srs_ = other.src_srs_;
      src_matrix_ = other.src_matrix_;
      dst_srs_ = other.dst_srs_;
      dst_matrix_ = other.dst_matrix_;
      max_error_ = other.max_error_;
      if (!src_srs_.empty() || !dst_srs_.empty()) {
	create(src_srs_, dst_srs_, max_error_);
      }
    }

    geo_transform& geo_transform::operator= (const geo_transform& other) {
      src_srs_ = other.src_srs_;
      src_matrix_ = other.src_matrix_;
      dst_srs_ = other.dst_srs_;
      dst_matrix_ = other.dst_matrix_;
      max_error_ = other.max_error_;
      destroy();
      create(src_srs_, dst_srs_, max_error_);
      return *this;
    }

    void geo_transform::store_to(sl::output_serializer& s) const {
      s.write_simple(sl::uint32_t(src_srs_.size()));
      if (src_srs_.size()) s.write_array(src_srs_.size(), (const sl::uint8_t*)src_srs_.c_str());
      src_matrix_.store_to(s);
      s.write_simple(sl::uint32_t(dst_srs_.size()));
      if (dst_srs_.size()) s.write_array(dst_srs_.size(), (const sl::uint8_t*)dst_srs_.c_str());
      dst_matrix_.store_to(s);
      s.write_simple(max_error_);
    }

    void geo_transform::retrieve_from(sl::input_serializer& s) {
      sl::uint32_t sz; 
      s.read_simple(sz);
      src_srs_.resize(sz);
      if (sz) s.read_array(sz, (sl::uint8_t*)src_srs_.c_str());
      src_matrix_.retrieve_from(s);
      s.read_simple(sz);
      dst_srs_.resize(sz);
      if (sz) s.read_array(sz, (sl::uint8_t*)dst_srs_.c_str());
      dst_matrix_.retrieve_from(s);
      s.read_simple(max_error_);
      
      destroy();
      create(src_srs_, dst_srs_, max_error_);
    }
    
    void geo_transform::create(const std::string& src_proj, const std::string& dst_proj, double max_error) {
      src_srs_ = geo_utility::proj2srs(src_proj);
      dst_srs_ = geo_utility::proj2srs(dst_proj);
      max_error_ = max_error;

      if(approximate_reprojector_) GDALDestroyApproxTransformer(approximate_reprojector_);
      approximate_reprojector_=0;

      if(precise_reprojector_) GDALDestroyReprojectionTransformer(precise_reprojector_);
      precise_reprojector_=0;

      precise_reprojector_ = GDALCreateReprojectionTransformer(src_srs_.c_str(), dst_srs_.c_str());
      approximate_reprojector_=0;
      if (max_error_ >= 0.001) approximate_reprojector_ = GDALCreateApproxTransformer(GDALReprojectionTransform, precise_reprojector_, max_error_);
    }

    void geo_transform::destroy() {
      if(approximate_reprojector_) GDALDestroyApproxTransformer(approximate_reprojector_);
      approximate_reprojector_=0;
      if(precise_reprojector_) GDALDestroyReprojectionTransformer(precise_reprojector_);
      precise_reprojector_=0;
    }

    int geo_transform::transform(void *arg, int dts, int n, double *x, double *y, double *z, int *ps) {
      this_t *t = reinterpret_cast<this_t*>(arg);
      sl::fixed_size_array<6,double> src_m;
      sl::fixed_size_array<6,double> dst_m;
      if (dts) {
	src_m=geo_utility::gdal_array(t->dst_matrix_.mat());
	dst_m=geo_utility::gdal_array(t->src_matrix_.mat_inv());
      } else {
	src_m=geo_utility::gdal_array(t->src_matrix_.mat());
	dst_m=geo_utility::gdal_array(t->dst_matrix_.mat_inv());
      }
      
      for (int i=0; i<n; ++i) {
        double tx = x[i]; 
	double ty = y[i];
        x[i] = src_m[0] + tx * src_m[1] + ty * src_m[2];
        y[i] = src_m[3] + tx * src_m[4] + ty * src_m[5];
      }

      if (t->approximate_reprojector_) {
	GDALApproxTransform(t->approximate_reprojector_, dts, n, x, y, z, ps);
      } else if (t->precise_reprojector_) {
	GDALReprojectionTransform(t->precise_reprojector_, dts, n, x, y, z, ps);
      }

      for (int i=0; i<n; ++i) {
        double tx = x[i]; 
	double ty = y[i];
        x[i] = dst_m[0] + tx * dst_m[1] + ty * dst_m[2];
        y[i] = dst_m[3] + tx * dst_m[4] + ty * dst_m[5];
      }
      return 1;
    }
      

  } // namespace geo
} // namespace vic
