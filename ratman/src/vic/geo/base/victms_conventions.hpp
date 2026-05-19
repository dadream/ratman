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
#ifndef VIC_GEO_BASE_VICTMS_CONVENTIONS_HPP
#define VIC_GEO_BASE_VICTMS_CONVENTIONS_HPP

#include <iostream>
#include <sstream>
#include <string>
#include <stdio.h>
#include <cassert>

namespace vic {

  namespace geo {

    namespace base {

      class victms_conventions {
      public:

	static inline std::string quad_filename(const std::string& rootpath,
						int l,
						int x,
						int y,
						const std::string& ext="jpg") {
	  assert(l>=0);
	  assert(x>=0);
	  assert(y>=0);

#if 0
	  // RAW TMS CONVENTION 
	  // We assume that the file system deals perfectly fine
	  // with extremely large directories
	  std::ostringstream oss;
	  oss << rootpath << '/' << l << '/' << x << '/' << y << '.' << ext;
	  return oss.str();
#else
	  // VIC CONVENTION:
	  // Goal is to avoid directories with too many files and
	  // to ensure a bit of coherence for row major scans. 
	  // We assume world can be indexed with 32 bit (1cm resolution
	  // for the earth) and tiles are 256 pixel wide (8 bit). 
	  // This leaves 24 bits for the directory, which are
	  // split in 3 levels, 8 bit each. Thus, the scheme
	  // generates a max of 256 entries per directory. 
#ifdef _WIN32
#define SNPRINTF _snprintf
#else 
#define SNPRINTF snprintf
#endif	  
#if 0
	  int xhh = (x/65536);
	  int xlh = (x/256)%256;
	  int xll = (x%256);
	  int yhh = (y/65536);
	  int ylh = (y/256)%256;
	  int yll = (y%256);
	  char tmp[64];
	  SNPRINTF (tmp, sizeof(tmp), "%02d/%03d/%03d/%03d/%03d/%03d/%03d",
		    l,
		    yhh, ylh, yll,
		    xhh, xlh, xll);
#else
          int xh = x/4096;
          int xl = x%4096;
          int yh = y/4096;
          int yl = y%4096;
          char tmp[64];
          SNPRINTF (tmp, sizeof(tmp), "%02d/%04d/%04d/%04d/%04d",
                    l,
                    yh, yl,
                    xh, xl);
#endif
	  std::string result = rootpath;
	  result.append("/");
	  result.append(std::string(tmp));
	  result.append(".");
	  result.append(ext);
	  return result;
#endif
	}

      };
    }
  }
}

#endif

