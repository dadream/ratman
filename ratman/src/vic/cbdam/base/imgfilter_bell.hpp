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
#ifndef CBDAM_IMGFILTER_BELL_HPP
#define CBDAM_IMGFILTER_BELL_HPP

#include <vic/cbdam/base/imgfilter.hpp>

namespace cbdam {

  class imgfilter_bell : public imgfilter {
  public:
    typedef imgfilter                   super_t;
    typedef super_t::value_t            value_t;

  public:
    imgfilter_bell() {
      set_aperture(1.5f);
    }

    inline value_t value(value_t t) {
      value_t result=0.0f;
      if (t < 0.0) t=-t;
      if (t < 0.5) result=0.75-t*t;
      else if (t < 1.5) {
	t=t-1.5;
	result=0.5*t*t;
      }
      return result;
    }

  };

} // namespace cbdam

#endif // CBDAM_IMGFILTER_BELL_HPP
