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
#ifndef TILEINFO_HPP
#define TILEINFO_HPP
#include <string>
#include <vector>
#include <sl/axis_aligned_box.hpp>
#include <sl/fixed_size_point.hpp>
#include <gdal_priv.h>

namespace vic {
  namespace geo {

    class tileinfo {
    protected:
      typedef sl::axis_aligned_box<2,double> aabox2d_t;
      typedef aabox2d_t::point_t             point2d_t;
      
    protected:
      std::vector<std::string> fnames_;
      std::vector<aabox2d_t> boxes_;

    public:
      tileinfo(const std::string& dirname, const std::string& pattern);
      ~tileinfo();

      std::size_t search(double x, double y);
      
      inline const std::string& fname(std::size_t i) const {
	return fnames_[i];
      }

      inline const aabox2d_t& box(std::size_t i) const {
	return boxes_[i];
      }

    protected:
      void process_tile(const std::string& fname);
      void process_directory(const std::string& dirname, const std::string& pattern);
      sl::point2d get_corner(GDALDataset* tile, double x, double y) const;

    };
    
  } // namespace geo
} // namespace vic

#endif
