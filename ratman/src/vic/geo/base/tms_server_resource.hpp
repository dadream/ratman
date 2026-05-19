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
#ifndef VIC_GEO_BASE_TMS_SERVER_RESOURCE_HPP
#define VIC_GEO_BASE_TMS_SERVER_RESOURCE_HPP

#include <string>

namespace vic {

  namespace geo {

    namespace base {

      class tms_resource {
      protected:
	bool is_valid_;
      public:
	tms_resource() {
	  is_valid_ = false;
	}

	virtual ~tms_resource() {
	}

	const bool is_valid() const {
	  return is_valid_;
	}
      };

    }
  }
}

#endif
