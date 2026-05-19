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
#ifndef TILEMAP_CONFIG_HPP
#define TILEMAP_CONFIG_HPP

#include <string>

namespace vic {

  namespace xml {
    class node_iterator;
  }

  namespace geo {

    namespace base {

      class tilemap_config {

      protected:
	
	std::string name_;
	std::string profile_;
	std::string mime_;
	std::string extension_;
	int max_level_;
	std::string srs_;
	double bbox_lo_[2];
	double bbox_hi_[2];
	int nu_;
	int nv_;
	int img_width_;
	int img_height_;
	
      public:
	
	tilemap_config();
	~tilemap_config();
	
	inline const std::string& name() const {
	  return name_;
	}
	
	inline void set_name(const std::string& x) {
	  name_ = x;
	}

	inline const std::string& profile() const {
	  return profile_;
	}

	inline void set_profile(const std::string& x) {
	  profile_ = x;
	}
	
	inline const std::string& mime() const {
	  return mime_;
	}

	inline void set_mime(const std::string& x) {
	  mime_ = x;
	}
	
	inline const std::string& extension() const {
	  return extension_;
	}

	inline void set_extension(const std::string& x) {
	  extension_ = x;
	}
	
	inline int max_level() const {
	  return max_level_;
	}

	inline void set_max_level(const int& x) {
	  max_level_ = x;
	}
	
	inline const std::string& srs() const {
	  return srs_;
	}

	inline void set_srs(const std::string& x) {
	  srs_ = x;
	}
	
	inline double bbox_lo(int i) const {
	  return bbox_lo_[i];
	}
	
	inline double bbox_hi(int i) const {
	  return bbox_hi_[i];
	}

	inline void set_bbox(double l0, double l1, 
			     double u0, double u1) {
	  bbox_lo_[0] = l0;
	  bbox_lo_[1] = l1;
	  bbox_hi_[0] = u0;
	  bbox_hi_[1] = u1;
	}
	
	inline int nu() const {
	  return nu_;
	}
	
	inline int nv() const {
	  return nv_;
	}

	inline void set_nu_nv(int nu, int nv) {
	  nu_ = nu;
	  nv_ = nv;
	}

	inline int img_width() const {
	  return img_width_;
	}
	
	inline int img_height() const {
	  return img_height_;
	}

	inline void set_img_width_height(int w, int h) {
	  img_width_ = w;
	  img_height_ = h;
	}
	
	double units_per_pixel(std::size_t level) const;
	
	bool parse(vic::xml::node_iterator ptr);
	
	std::string description() const;
      };
    }
  }
}

#endif 

