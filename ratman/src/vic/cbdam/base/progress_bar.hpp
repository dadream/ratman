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
#ifndef CBDAM_PROGRESS_BAR_HPP
#define CBDAM_PROGRESS_BAR_HPP

#include <vic/cbdam/base/config.hpp>

namespace cbdam {
  
  /**
   *
   */
  class progress_bar {
  protected:
#if 0
    float x_;
    float y_;
    float length_;
#endif
    bool is_vertical_;
    bool look3d_;

  public:
    /*
     * x,y,w,h: position and dimension of the progressbar in percent of current window size
     */
    //    progress_bar(float x, float y, float length, bool is_vertical = false, bool look3d = false);

    // always length 128, positioned on top right of the screen
    progress_bar(bool is_vertical = false, bool look3d = false);

    ~progress_bar();

    /**
     * value ranges in [0..1]
     */
    void draw(float value);

  protected:
  
  };


} // namespace cbdam 

#endif // CBDAM_PROGRESS_BAR_HPP

#ifndef CBDAM_PROGRESS_BAR_IPP
#define CBDAM_PROGRESS_BAR_IPP

namespace cbdam {

} // namespace cbdam 

#endif // CBDAM_PROGRESS_BAR_IPP
