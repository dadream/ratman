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
#ifndef CBDAM_IMG_OPERATIONS_HPP
#define CBDAM_IMG_OPERATIONS_HPP

#include <QImage>

#include <vector>
#include <utility>

namespace cbdam {

  class imgfilter;

  class img_operations {
  public:
    typedef std::pair<int, double>     contrib_item_t;

  public:

    static QImage zoom(const QImage& src, int dst_width, 
		       int dst_height, imgfilter* filter=0);


  };

} // namespace cbdam

#endif // CBDAM_IMG_OPERATIONS_HPP
