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
#ifndef RATMAN_VERSION_HPP
#define RATMAN_VERSION_HPP

#include <sstream>
#include <string>

// RATMAN_VERSION is (major << 16) + (minor << 8) + patch.
#define RATMAN_VERSION 0x020000
#define RATMAN_DATE "22/08/2007"
#define RATMAN_AUTHOR "CRS4/ViC"

namespace ratman {

  /**
   * Application version.
   */ 
  class version {

  public:

    static int major_no() {
      return (RATMAN_VERSION >> 16) & 0xFF;
    }

    static int minor_no() {
      return (RATMAN_VERSION >> 8) & 0xFF;
    }

    static int patch_no() {
      return RATMAN_VERSION & 0xFF;
    }

    static std::string version_str() {
      std::ostringstream o;
      o <<  major_no() << "." <<  minor_no() << "." << patch_no();
      return o.str();
    }

    static std::string date_str() {
      return RATMAN_DATE;
    }

    static std::string author_str() {
      return RATMAN_AUTHOR;
    }
    
  };

}


#endif
