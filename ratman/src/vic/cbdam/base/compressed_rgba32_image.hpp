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
#ifndef VIC_CBDAM_COMPRESSED_RGBA32_IMAGE_HPP
#define VIC_CBDAM_COMPRESSED_RGBA32_IMAGE_HPP

#include <vic/img/gl_image.hpp>
#include <vector>

namespace cbdam {

  class compressed_rgba32_image {
  public:
    typedef vic::img::gl_image<> uncompressed_image_t;
    typedef sl::uint8_t		 uint8_t;

    enum genalpha_behavior_t { 
      ALPHA_KEEP_SRC, 
      ALPHA_FROM_CONSTANT, 
      ALPHA_FROM_SRC, 
      ALPHA_FROM_BLACK_IF_ABSENT,
      ALPHA_FROM_BLACK_FORCED,
      ALPHA_FROM_WHITE_IF_ABSENT,
      ALPHA_FROM_WHITE_FORCED,
      ALPHA_FROM_VALUE_IF_ABSENT,
      ALPHA_FROM_VALUE_FORCED
    };

  protected:
    std::size_t quad_width_;
    genalpha_behavior_t genalpha_behavior_;
    uint8_t     genalpha_constant_;
    uint8_t     genalpha_alphamin_;
    uint8_t     genalpha_alphamax_;
    bool is_uncompressed_;
    std::vector<uint8_t> data_;

  public:

    static void generate_alpha(uncompressed_image_t& img,
			       bool img_is_grayscale,
			       bool img_has_alpha,
			       genalpha_behavior_t genalpha_behavior,
			       uint8_t genalpha_constant,
			       uint8_t genalpha_alphamin = 0,
			       uint8_t genalpha_alphamax = 255);
  public:

    compressed_rgba32_image(const uint8_t* buf,
			    std::size_t buf_size,
			    std::size_t quad_width = 256,
			    genalpha_behavior_t genalpha_behavior = ALPHA_FROM_SRC, 
			    uint8_t genalpha_constant = 10,
			    uint8_t genalpha_alphamin = 0,
			    uint8_t genalpha_alphamax = 255);

    compressed_rgba32_image(const uncompressed_image_t& img);
    
    void uncompress_to(uncompressed_image_t& img) const;

  protected:

    static void color_remap(uncompressed_image_t& img,
			    uint8_t black_out, uint8_t black_in, 
			    uint8_t white_out, uint8_t white_in);
		     
  };


}

#endif
