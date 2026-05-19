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
#include <vic/cbdam/base/diamond_operator.hpp>
#include <cstring>
#include <cassert>

namespace cbdam {
    
  void color_operator::compress_to_target_error(const array2_t& ycocg, data_buffer_t& compressed_delta,
						float tolerance, sl::array_codec* codec,
						bool use_amax_error) {
    int h_y = ycocg.extent()[0];
    int w_y = ycocg.extent()[1];
    int h_c = h_y;
    int w_c = w_y;

    sl::dense_array<int16_t,2,void> Y_r(h_y,w_y);
    sl::dense_array<int16_t,2,void> Co_r(h_c, w_c);
    sl::dense_array<int16_t,2,void> Cg_r(h_c, w_c);

    // split 3 components array ycocg into 3 arrays.
    for(int y = 0; y < h_y; ++y) {
      for(int x = 0; x < w_y; ++x) {
	// get transformed color
	const value_t& tc = ycocg(y,x);
	Y_r(y,x)  = tc[0];
	Co_r(y,x) = tc[1];
	Cg_r(y,x) = tc[2];
      }
    }

    std::size_t buf_size = ycocg.count() * sizeof(float) * 3 * 4;
    compressed_delta.resize(buf_size);
    
    // insert space for the size of the 2 compressed buf r,g
    uint32_t base_offset = 2*sizeof(int16_t);
    uint8_t* buf = &(compressed_delta[base_offset]);

    // compress the 3 arrays
    std::size_t actual_size0, actual_size1, actual_size2;

    const float eps = tolerance;
    const float chroma_dynamic_range_factor = 2.0f;
    const float luminance_eps = eps;
    const float chroma_eps    = chroma_dynamic_range_factor*eps;

    buf_size -= base_offset;
    if (use_amax_error) {
      codec->compress_to_target_amax_error(Y_r, luminance_eps, buf, buf_size, &actual_size0, NULL);
    } else {
      codec->compress_to_target_rms_error(Y_r, luminance_eps, buf, buf_size, &actual_size0, NULL);
    }      
    buf = buf + actual_size0;
    buf_size -= actual_size0;
    
    if (use_amax_error) {
      codec->compress_to_target_amax_error(Co_r, chroma_eps, buf, buf_size, &actual_size1, NULL);
    } else {
      codec->compress_to_target_rms_error(Co_r, chroma_eps, buf, buf_size, &actual_size1, NULL);
    }      
      
    buf = buf + actual_size1;
    buf_size -= actual_size1;

    if (use_amax_error) {
      codec->compress_to_target_amax_error(Cg_r, chroma_eps, buf, buf_size, &actual_size2, NULL);
    } else {
      codec->compress_to_target_rms_error(Cg_r, chroma_eps, buf, buf_size, &actual_size2, NULL);
    }
    
    // copy actual sizes in the first bytes, third size is derived from the total size.
    uint16_t sizes[2] = {static_cast<uint16_t>(actual_size0), static_cast<uint16_t>(actual_size1)};
    memcpy(&(compressed_delta[0]), (uint8_t*)sizes, base_offset);
    
    // resize to target_size
    compressed_delta.resize(base_offset + actual_size0 + actual_size1 + actual_size2);
  }

  void color_operator::compress_lossless(const array2_t& delta, data_buffer_t& compressed_delta,
					 sl::array_codec* codec) {
    compress_to_target_error(delta, compressed_delta, 0.0f, codec, true); // amax error = 0
  }

  void color_operator::decompress_to(array2_t& ycocg, const uint8_t* data_buf, uint32_t data_buf_size,
                                     sl::array_codec* codec) {
    sl::dense_array<int16_t,2,void> Y_r;
    sl::dense_array<int16_t,2,void> Co_r;
    sl::dense_array<int16_t,2,void> Cg_r;

    // get sizes of the 3 arrays
    const uint32_t base_offset = 2 * sizeof(int16_t);
    uint16_t sizes[3];
    memcpy((uint8_t*)sizes, data_buf, base_offset);
    sizes[2] = data_buf_size - (base_offset + sizes[0] + sizes[1]);
    
    // decompress the 3 arrays
    const uint8_t* buf = data_buf + base_offset;
    codec->decompress(Y_r, buf, sizes[0]);
    buf += sizes[0];
    codec->decompress(Co_r, buf, sizes[1]);
    buf += sizes[1];
    codec->decompress(Cg_r, buf, sizes[2]);

    // combine the r,g,b into delta_rgb
    ycocg.resize(Y_r.extent());

    int h_y = ycocg.extent()[0];
    int w_y = ycocg.extent()[1];
    assert((int)Co_r.extent()[0] == h_y);
    assert((int)Co_r.extent()[1] == w_y);
    assert((int)Cg_r.extent()[0] == h_y);
    assert((int)Cg_r.extent()[1] == w_y);

    // recombine 3 arrays into a single 3 component array
    for(int y = 0; y < h_y; ++y) {
      for(int x = 0; x < w_y; ++x) {
	ycocg(y, x) = value_t(Y_r(y,x), Co_r(y,x), Cg_r(y,x));
      }
    }
  }

  void color_operator::decorrelate_channels(array2_t& ycocg, const array2_t& rgb) {
    int h_y = rgb.extent()[0];
    int w_y = rgb.extent()[1];

    // Sample Y, accumulate Co/Cg
    for(int y = 0; y < h_y; ++y) {
      for(int x = 0; x < w_y; ++x) {
	value_t c = rgb(y,x);
	ycocg(y, x) = ycocg_r_from_rgb(c[0], c[1], c[2]);
      }
    }    
  }
    
 void color_operator::recorrelate_channels(array2_t& rgb, const array2_t& ycocg) {
    int h_y = ycocg.extent()[0];
    int w_y = ycocg.extent()[1];

    for(int y = 0; y < h_y; ++y) {
      for(int x = 0; x < w_y; ++x) {
	const value_t& tc = ycocg(y, x);
	rgb(y,x) = rgb_from_ycocg_r(tc[0], tc[1], tc[2]);
      }
    }
  }

  double color_operator::amax_difference(const array2_t& a1, const array2_t& a2) {
    float result = 0.0;
    std::size_t h = a1.extent()[0];
    std::size_t w = a1.extent()[1];
    for (std::size_t i=0; i<h; ++i) {
      for (std::size_t j=0; j<w; ++j) {
        const delta_color3_t& c1 = a1(i,j);
        const delta_color3_t& c2 = a2(i,j);
        vector3_t delta(c1[0]-c2[0], c1[1]-c2[1], c1[2]-c2[2]);
	result = std::max(result, delta[delta.iamax()]);
      }
    }
    return (double)result;
  }

  double color_operator::rms(const array2_t& d0, const array2_t& d1) {
    assert(d0.extent() == d1.extent());
    double result = 0.0f;
    std::size_t h = d0.extent()[0];
    std::size_t w = d0.extent()[1];
    for (std::size_t i=0; i<h; ++i) {
      for (std::size_t j=0; j<w; ++j) {
        const delta_color3_t& c0 = d0(i,j);
        const delta_color3_t& c1 = d1(i,j);
        vector3_t delta(c0[0]-c1[0], c0[1]-c1[1], c0[2]-c1[2]);
        result += delta.two_norm_squared();
      }
    }
    if (result) {
      result = std::sqrt(result/(h*w));
    }
    return result;
  }
  
  double color_operator::amean(const array2_t& a1) {
    double mean_x = 0;
    double mean_y = 0;
    double mean_z = 0;
    std::size_t h = a1.extent()[0];
    std::size_t w = a1.extent()[1];
    for (std::size_t i=0; i<h; ++i) {
      for (std::size_t j=0; j<w; ++j) {
	value_t c = a1(i,j);
	mean_x += fabs((double)c[0]);
	mean_y += fabs((double)c[1]);
	mean_z += fabs((double)c[2]);
      }
    }
    mean_x /= double(w*h);
    mean_y /= double(w*h);
    mean_z /= double(w*h);

    return (mean_x + mean_y + mean_z) / 3.0;
  }

  double color_operator::amax(const array2_t& a1) {
    double max_x = 0;
    double max_y = 0;
    double max_z = 0;
    std::size_t h = a1.extent()[0];
    std::size_t w = a1.extent()[1];
    for (std::size_t i=0; i<h; ++i) {
      for (std::size_t j=0; j<w; ++j) {
	value_t c = a1(i,j);
	if (max_x < fabs((double)c[0])) max_x = fabs((double)c[0]);
	if (max_y < fabs((double)c[1])) max_y = fabs((double)c[1]);
	if (max_z < fabs((double)c[2])) max_z = fabs((double)c[2]);
      }
    }

    return sl::max(sl::max(max_x, max_y), max_z);
  }

  double color_operator::fraction_zero_values(const array2_t& a1, double tolerance) {
    uint32_t zero_count = 0;
    std::size_t h = a1.extent()[0];
    std::size_t w = a1.extent()[1];
    for (std::size_t i=0; i<h; ++i) {
      for (std::size_t j=0; j<w; ++j) {
	value_t c = a1(i,j);
	for (std::size_t x=0; x<3; ++x) {
	  if (-tolerance < c[x] && c[x] < tolerance) {
	    ++zero_count;
	  }
	}
      }
    }
   
    return (double)zero_count/(double)(a1.count() * 3);
  }

  ///////////////////////////////////////////////////////////////////////////////////
  // height_operator

  void height_operator::compress_to_target_error(const array2_t& delta, data_buffer_t& compressed_delta,
						 float tolerance, sl::array_codec* codec,
						 bool use_amax_error) {
    assert(delta.count());
    const int max_size = 4 * delta.count() * sizeof(value_t) + 1024;
    compressed_delta.resize(max_size);

    std::size_t actual_size;
    if (use_amax_error) {
      codec->compress_to_target_amax_error(delta, tolerance, &(compressed_delta[0]), max_size, &actual_size, NULL);
    } else {
      codec->compress_to_target_rms_error(delta, tolerance, &(compressed_delta[0]), max_size, &actual_size, NULL);
    }
    compressed_delta.resize(actual_size);
  }

  void height_operator::compress_lossless(const array2_t& delta, data_buffer_t& compressed_delta,
					  sl::array_codec* codec) {
    compress_to_target_error(delta, compressed_delta, 0.0f, codec, true); // amax error = 0
  }

  void height_operator::decompress_to(array2_t& delta, const uint8_t* data_buf, uint32_t data_buf_size,
                                      sl::array_codec* codec) {
    codec->decompress(delta, data_buf, data_buf_size);
  }

  double height_operator::amax_difference(const array2_t& a1, const array2_t& a2) {
    double result = 0.0;
    std::size_t h = a1.extent()[0];
    std::size_t w = a1.extent()[1];

    for (std::size_t i=0; i<h; ++i) {
      for (std::size_t j=0; j<w; ++j) {
	result = std::max(result, std::abs(double(a1(i,j))-double(a2(i,j))));
      }
    }

    return result;
  }

  double height_operator::rms(const array2_t& d0, const array2_t& d1) {
    return sl::rms(d0, d1);
  }

 double height_operator::amean(const array2_t& a1) {
    double mean = 0;
    std::size_t h = a1.extent()[0];
    std::size_t w = a1.extent()[1];
    for (std::size_t i=0; i<h; ++i) {
      for (std::size_t j=0; j<w; ++j) {
	mean += fabs((double)a1(i,j));
      }
    }
    mean /= double(w*h);

    return double(mean);
  }

  double height_operator::amax(const array2_t& a1) {
    double max = 0;
    std::size_t h = a1.extent()[0];
    std::size_t w = a1.extent()[1];
    for (std::size_t i=0; i<h; ++i) {
      for (std::size_t j=0; j<w; ++j) {
	if (max < fabs((double)a1(i,j))) {
	  max = fabs((double)a1(i,j));
	}
      }
    }

    return max;
  }

 double height_operator::fraction_zero_values(const array2_t& a1, double tolerance) {
   uint32_t zero_count = 0;
    std::size_t h = a1.extent()[0];
    std::size_t w = a1.extent()[1];
    for (std::size_t i=0; i<h; ++i) {
      for (std::size_t j=0; j<w; ++j) {
	value_t x = a1(i, j);
	if (-tolerance < x && x < tolerance) {
	  ++zero_count;
	}
      }
    }

    return (double)zero_count/(double)a1.count();
 }

  void height_operator::decorrelate_channels(array2_t& a, const array2_t& b) {
    uint32_t h = b.extent()[0];
    uint32_t w = b.extent()[1];
    for(uint32_t y = 0; y < h; ++y) {
      for(uint32_t x = 0; x < w; ++x) {
        a(y, x) = b(y, x);
      }
    }    
  }

  void height_operator::recorrelate_channels(array2_t& a, const array2_t& b) {
    uint32_t h = b.extent()[0];
    uint32_t w = b.extent()[1];
    for(uint32_t y = 0; y < h; ++y) {
      for(uint32_t x = 0; x < w; ++x) {
        a(y, x) = b(y, x);
      }
    }    
  }

}
