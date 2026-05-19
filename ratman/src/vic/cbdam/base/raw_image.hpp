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
#ifndef CBDAM_RAW_IMAGE_HPP
#define CBDAM_RAW_IMAGE_HPP

#include <vic/cbdam/base/config.hpp>
#include <sl/external_array.hpp>
#include <cassert>

namespace cbdam {
  
  /**
   *
   */
  class raw_image {
  public:
    typedef int16_t                             value_t;
    typedef sl::external_array1<value_t>        xarray_value_t;

  protected:
    xarray_value_t*     xarray_;

  protected:
    static const uint32_t tile_width_;
    static const uint32_t tile_size_;
    static const uint32_t header_size_;
    uint32_t height_;
    uint32_t width_;
    uint32_t y_tiles_;
    uint32_t x_tiles_;

  public:
    raw_image();

    /**
     * create temporary image
     */
    raw_image(uint32_t h, uint32_t w, uint32_t cache_size = 16*1024*1024);

    ~raw_image();
    
    void open_to_write(uint32_t h, uint32_t w, std::string fname, uint32_t cache_size = 16*1024*1024);

    void open_to_read(std::string fname, uint32_t cache_size = 16*1024*1024);

    bool is_open() const;

    void close();
    
    uint32_t width() const;

    uint32_t height() const;

    value_t operator()(uint32_t y, uint32_t x) const;

    value_t value_at_parametric_coords(float y_param, float x_param) const;

    void set_value_at(uint32_t y, uint32_t x, value_t h);

    void set_value(value_t h);

    void print_info() const;
    
  protected:
    void read_header();

    void write_header();
    
    void resize_array(uint32_t h, uint32_t w);

    void compute_tile_subdivision();

    uint32_t offset(uint32_t y, uint32_t x) const;
  };


} // namespace cbdam 

#endif // CBDAM_RAW_IMAGE_HPP

#ifndef CBDAM_RAW_IMAGE_IPP
#define CBDAM_RAW_IMAGE_IPP

namespace cbdam {

  inline bool raw_image::is_open() const {
    return xarray_ && xarray_->is_open();
  }
  
  inline uint32_t raw_image::width() const {
    return width_;
  }

  inline uint32_t raw_image::height() const {
    return height_;
  }

  inline raw_image::value_t raw_image::operator()(uint32_t y, uint32_t x) const {
    assert(is_open());
    return (*xarray_)[offset(y, x)];
  }

  inline void raw_image::set_value_at(uint32_t y, uint32_t x, value_t h) {
    assert(is_open());
    (*xarray_)[offset(y, x)] = h;
  }

  inline raw_image::value_t raw_image::value_at_parametric_coords(float y_param, float x_param) const {
    assert(is_open());
    assert(0.0f <= y_param && y_param <= 1.0f);
    assert(0.0f <= x_param && x_param <= 1.0f);

    float x = x_param * (width_ - 1);
    float y = y_param * (height_ - 1);
    uint32_t ix = (uint32_t)x;
    uint32_t iy = (uint32_t)y;
    float fx = x - ix;
    float fy = y - iy;

    if (x_param > 0.0f && x_param < 1.0f &&
        y_param > 0.0f && y_param < 1.0f) {
      return (value_t)((1-fx) * ((1-fy) * (*this)(iy, ix) + fy * (*this)(iy+1, ix)) +
                       fx * ((1-fy) * (*this)(iy, ix+1) + fy * (*this)(iy+1, ix+1)));
    } else {
      return (*this)(iy, ix);
    }
  }

  inline uint32_t raw_image::offset(uint32_t y, uint32_t x) const {
    assert(y < height_);
    assert(x < width_);
    
    uint32_t ty = y / tile_width_;
    uint32_t tx = x / tile_width_;
    uint32_t iy = y - ty * tile_width_;
    uint32_t ix = x - tx * tile_width_;
    uint32_t offset = (ty * x_tiles_ + tx) * tile_size_ + iy * tile_width_ + ix + header_size_;
    assert(offset < xarray_->size());

    return offset;
  }

} // namespace cbdam 

#endif // CBDAM_RAW_IMAGE_IPP
