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
#include <vic/img/gl_quadtree_image_processor.hpp>
#include <sl/utility.hpp>

namespace vic {

  namespace img {

    void gl_quadtree_image_processor::magnify_in(quad_t& dst_quad, // fine
						 int dst_level, int dst_x, int dst_y,
						 const quad_t& src_quad, // coarse
						 int src_level, int src_x, int src_y) {
      // magnify only
      assert(src_level<=dst_level); 

      // square image
      assert(src_quad.width() == src_quad.height());
      assert(src_quad.depth() == 1);
      assert(src_quad.channels() > 0);
      // src shape = dst shape
      assert(dst_quad.width() == src_quad.width()); 
      assert(dst_quad.height() == src_quad.height());
      assert(dst_quad.depth() == src_quad.depth());
      assert(dst_quad.channels() == src_quad.channels());

      int delta_level = dst_level-src_level;
      if (delta_level==0) {
	// Copy
	assert(src_x == dst_x);
	assert(src_y == dst_y);

	dst_quad = src_quad;
      } else {
	// Zoom cropped version 
	
	int sz = int(src_quad.width());
	int nc = src_quad.channels();

	int zoom_factor = (1<<delta_level);

	int src_crop_size = sz/zoom_factor;
	int src_crop_x0 = sl::median(0, sz-1, (dst_x * sz) / zoom_factor - src_x * sz);
	int src_crop_y0 = sl::median(0, sz-1, (dst_y * sz) / zoom_factor - src_y * sz);
	//	int src_crop_y0 = sl::median(0, sz-1, sz - src_crop_size - ((dst_y * sz) / zoom_factor - src_y * sz));
	int src_crop_x1 = sl::median(src_crop_x0,  sz-1, src_crop_x0+src_crop_size-1);
	int src_crop_y1 = sl::median(src_crop_y0,  sz-1, src_crop_y0+src_crop_size-1);

#if 0
	std::cerr << "SRC: " <<
	  "(" << src_crop_x0 << " " << src_crop_y0 << ")" <<
	  "(" << src_crop_x1 << " " << src_crop_y1 << ")" << 
	  " zoom " << zoom_factor <<
	  std::endl;
#endif

	// FIXME STUPID NEAREST NEIGHBOR ZOOM - REPLACE
	for (int dst_y = 0; dst_y < sz; ++dst_y) {
	  // Find source row 
	  int src_y = sl::median(src_crop_y0, src_crop_y1, src_crop_y0 + dst_y / zoom_factor);
	  
	  for (int dst_x = 0; dst_x < sz; ++dst_x) {
	    int src_x = sl::median(src_crop_x0, src_crop_x1, src_crop_x0 + dst_x / zoom_factor);
	  
     	    const value_t* src_ptr = src_quad.to_pointer(src_x, src_y);
	    value_t* dst_ptr = dst_quad.to_pointer(dst_x, dst_y);

	    for (int c=0; c<nc; ++c) {
	      dst_ptr[c] = src_ptr[c];
	    }
	  }
	}
      }
    }

    void gl_quadtree_image_processor::blend_in(quad_t& dst_quad,
					       const quad_t& src_quad) {
      // Alpha blend!
      // dst_a' = 1 - (1 - src_a) * (1 - dst_a)
      // dst_r = (1 - src_a) * dst_r + src_a * src_r
      // dst_g = (1 - src_a) * dst_g + src_a * src_g
      // dst_b = (1 - src_a) * dst_b + src_a * src_b

      // square image with alpha
      assert(src_quad.width() == src_quad.height());
      assert(src_quad.depth() == 1);
      assert(src_quad.channels() > 1); // has alpha

      // src shape = dst shape
      assert(dst_quad.width() == src_quad.width()); 
      assert(dst_quad.height() == src_quad.height());
      assert(dst_quad.depth() == src_quad.depth());
      
      assert(dst_quad.channels() == src_quad.channels() ||
	     dst_quad.channels() == src_quad.channels()-1);
      
      int w = src_quad.width();
      int h = src_quad.height();
      int wh = w*h;
      int snc = src_quad.channels();
      int dnc = dst_quad.channels();
      int aidx = snc-1;

      if (dst_quad.channels() == src_quad.channels()) {
	// Destination has alpha
	const value_t* src_ptr = src_quad.to_pointer();
	value_t* dst_ptr = dst_quad.to_pointer();
	for (int i=0; i<wh; ++i) {
	  int src_a = src_ptr[aidx]+1;
	  for (int c=0; c<dnc; ++c) {
	    dst_ptr[c] += (((int(src_ptr[c])-int(dst_ptr[c])) * src_a + (1<<7)) >> 8);
	  }
	  dst_ptr[aidx] = (255 - ((256 - src_a) * (255 - dst_ptr[aidx]) + (1<<9)-1) >> 8);
	  src_ptr += snc;
	  dst_ptr += dnc;
	}
      } else {
	// Destination has no alpha
	const value_t* src_ptr = src_quad.to_pointer();
	value_t* dst_ptr = dst_quad.to_pointer();
	for (int i=0; i<wh; ++i) {
	  int src_a = src_ptr[aidx]+1;
	  for (int c=0; c<dnc; ++c) {
	    dst_ptr[c] += (((int(src_ptr[c])-int(dst_ptr[c])) * src_a + (1<<7)) >> 8);
	  }
	  src_ptr += snc;
	  dst_ptr += dnc;
	}
      }
    }

    void gl_quadtree_image_processor::blend_in(quad_t& dst_quad, // fine
					       int dst_level, int dst_x, int dst_y,
					       const quad_t& src_quad, // coarse
					       int src_level, int src_x, int src_y) {
      // magnify only
      assert(src_level<=dst_level); 
      // square image
      assert(src_quad.width() == src_quad.height());
      assert(src_quad.depth() == 1);
      assert(src_quad.channels() > 0);
      // src shape = dst shape
      assert(dst_quad.width() == src_quad.width()); 
      assert(dst_quad.height() == src_quad.height());
      assert(dst_quad.depth() == src_quad.depth());
      assert(dst_quad.channels() == src_quad.channels());


      assert(src_level<=dst_level); // magnify only

      int delta_level = dst_level-src_level;
      if (delta_level==0) {
	assert(src_x == dst_x);
	assert(src_y == dst_y);

	blend_in(dst_quad, src_quad);
      } else {
	quad_t magnified_src_quad(src_quad.channels(),
				  src_quad.width(),
				  src_quad.height());
	magnify_in(magnified_src_quad, 
		   dst_level, dst_x, dst_y,
		   src_quad, 
		   src_level, src_x, src_y);

	blend_in(dst_quad, magnified_src_quad);
      }
    }
  }

}
