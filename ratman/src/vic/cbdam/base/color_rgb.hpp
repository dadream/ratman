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
#ifndef CBDAM_COLOR_RGB_HPP
#define CBDAM_COLOR_RGB_HPP

#include <vic/cbdam/base/config.hpp>

namespace cbdam {
  
  typedef sl::fixed_size_vector<sl::column_orientation, 4, int16_t>     delta_color4_t;
  typedef sl::fixed_size_vector<sl::column_orientation, 3, int16_t>     delta_color3_t;
  typedef sl::fixed_size_point<4, uint8_t>                              color_rgba_t;
  
  /**
   *
   */
  class color_rgb  : public sl::fixed_size_point<3, uint8_t> {
  public:
    typedef sl::fixed_size_point<3, uint8_t>	super_t;

  public:
    color_rgb();

    ~color_rgb();

    color_rgb(uint8_t v);

    color_rgb(uint8_t r, uint8_t g, uint8_t b);

    color_rgb(const super_t& x);

    color_rgb(const color_rgba_t& x);

    /**
     * Conversion operator to delta_color3_t to
     * have lighter code in diamond_operator
     */
    operator delta_color3_t() const {
      const color_rgb& rthis = *this;
      return delta_color3_t(rthis[0], rthis[1], rthis[2]);
    }
    
    /**
     * Conversion operator to color_rgba_t to
     * have lighter code in diamond_operator
     */
    operator color_rgba_t() const {
      const color_rgb& rthis = *this;
      return color_rgba_t(rthis[0], rthis[1], rthis[2], 255);
    }
    
    
    /**
     * Sum two color_rgbs, clamp values in 0..255
     */
    color_rgb operator+(const delta_color3_t& delta) const;

    /**
     * Sum two color_rgbs, clamp values in 0..255
     */
    color_rgb operator+(const color_rgb& rhs) const;

    delta_color3_t operator-(const color_rgb& rhs) const;

    color_rgb operator-(const delta_color3_t& rhs) const;

    color_rgb& operator+=(const delta_color3_t& delta);

    color_rgb& operator+=(const color_rgb& rhs);

    color_rgb operator*(float x) const;

  protected:
  
  };

  std::ostream& operator<<(std::ostream& os, const delta_color3_t& rhs);

  std::ostream& operator<<(std::ostream& os, const color_rgb& rhs);  
 
} // namespace cbdam 

#endif // CBDAM_COLOR_RGB_HPP



#ifndef CBDAM_COLOR_RGB_IPP
#define CBDAM_COLOR_RGB_IPP

namespace cbdam {

  inline color_rgb::color_rgb() : 
      super_t() {

  }

  inline color_rgb::~color_rgb() {

  }

  inline color_rgb::color_rgb(uint8_t v) :
      super_t(v, v, v) {

  }

  inline color_rgb::color_rgb(uint8_t r, uint8_t g, uint8_t b) :
      super_t(r, g, b) {

  }

  inline color_rgb::color_rgb(const super_t& x) :
      super_t(x) {

  }

  inline color_rgb::color_rgb(const color_rgba_t& x) :
      super_t(x[0], x[1], x[2]) {
    
  }
  
  inline color_rgb color_rgb::operator+(const delta_color3_t& delta) const {
    const color_rgb& rthis = *this;
    return color_rgb((uint8_t)sl::median(0, ((int16_t)rthis[0]) + delta[0], 255),
                     (uint8_t)sl::median(0, ((int16_t)rthis[1]) + delta[1], 255),
                     (uint8_t)sl::median(0, ((int16_t)rthis[2]) + delta[2], 255));
  }
     
  inline color_rgb color_rgb::operator-(const delta_color3_t& delta) const {
    const color_rgb& rthis = *this;
    for(int i = 0; i < 3; ++i) { 
      if ((int16_t)(rthis[i]) - delta[i]<0 || (int16_t)(rthis[i]) - delta[i]>255) {
	std::cerr << "warning corrupted color " << i << " component" << (int16_t)(rthis[i]) - delta[i] << std::endl;
      }
    }
    return color_rgb((uint8_t)sl::median(0, ((int16_t)rthis[0]) - delta[0], 255),
                     (uint8_t)sl::median(0, ((int16_t)rthis[1]) - delta[1], 255),
                     (uint8_t)sl::median(0, ((int16_t)rthis[2]) - delta[2], 255));
  }

  inline color_rgb color_rgb::operator+(const color_rgb& rhs) const {
    const color_rgb& rthis = *this;
    return color_rgb((uint8_t)sl::median(0, ((int16_t)rthis[0]) + (int16_t)rhs[0], 255),
                     (uint8_t)sl::median(0, ((int16_t)rthis[1]) + (int16_t)rhs[1], 255),
                     (uint8_t)sl::median(0, ((int16_t)rthis[2]) + (int16_t)rhs[2], 255));    
  }

  inline delta_color3_t color_rgb::operator-(const color_rgb& rhs) const {
    const color_rgb& rthis = *this;
    return delta_color3_t(((int16_t)rthis[0]) - ((int16_t)rhs[0]),
                             ((int16_t)rthis[1]) - ((int16_t)rhs[1]),
                             ((int16_t)rthis[2]) - ((int16_t)rhs[2]));
  }

  inline color_rgb& color_rgb::operator+=(const delta_color3_t& delta) {
    *this = *this + delta;
    return *this;
  }
  
  inline color_rgb& color_rgb::operator+=(const color_rgb& rhs) {
    *this = *this + rhs;
    return *this;
  }

  inline color_rgb color_rgb::operator*(float x) const {
    const color_rgb& rthis = *this;
    return color_rgb((uint8_t)sl::median(0, (int)(rthis[0] * x), 255),
                     (uint8_t)sl::median(0, (int)(rthis[1] * x), 255),
                     (uint8_t)sl::median(0, (int)(rthis[2] * x), 255));    
  
  }

  inline std::ostream& operator<<(std::ostream& os, const delta_color3_t& rhs) {
    os << "(" << (int)rhs[0] << ", " << (int)rhs[1] << ", "  << (int)rhs[2] << ") ";
    return os;
  }

  inline std::ostream& operator<<(std::ostream& os, const color_rgb& rhs) {
    os << "(" << (int)rhs[0] << ", " << (int)rhs[1] << ", "  << (int)rhs[2] << ") ";
    return os;
  }
} // namespace cbdam 

#endif // CBDAM_COLOR_RGB_IPP
