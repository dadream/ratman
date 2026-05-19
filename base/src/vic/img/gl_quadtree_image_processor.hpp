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
#ifndef VIC_IMG_GL_IMAGE_PROCESSOR_HPP
#define VIC_IMG_GL_IMAGE_PROCESSOR_HPP

#include <vic/img/gl_image.hpp>

namespace vic {
  namespace img {

    class gl_quadtree_image_processor {
    public:
      typedef unsigned char      value_t;
      typedef gl_image<value_t>  quad_t;

    public:

      void magnify_in(quad_t& dst_quad, // fine
		      int dst_level, int dst_x, int dst_y,
		      const quad_t& src_quad, // coarse
		      int src_level, int src_x, int src_y);

      void blend_in(quad_t& dst_quad, 
		    const quad_t& src_quad);

      void blend_in(quad_t& dst_quad, // fine
		    int dst_level, int dst_x, int dst_y,
		    const quad_t& src_quad, // coarse
		    int src_level, int src_x, int src_y);
			     
    };
  }
}

#endif
