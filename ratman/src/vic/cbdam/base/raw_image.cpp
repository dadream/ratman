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
#include <vic/cbdam/base/raw_image.hpp>
#include <algorithm>
#include <cassert>
#include <stdlib.h>

namespace cbdam {

  const uint32_t raw_image::tile_width_ = (uint32_t)sqrt(4096.0f/sizeof(raw_image::value_t));
  const uint32_t raw_image::tile_size_ = tile_width_ * tile_width_;
  const uint32_t raw_image::header_size_ = 2 * sizeof(uint32_t) / sizeof(raw_image::value_t);
  
  raw_image::raw_image() {
    xarray_ = 0;
    width_ = 0;
    height_ = 0;
    y_tiles_ = 0;
    x_tiles_ = 0;
  }

  raw_image::raw_image(uint32_t h, uint32_t w, uint32_t cache_size) {
    // open temporary array of requested size
    xarray_ = new xarray_value_t(std::string()+"/tmp/raw_image"+sl::to_string(rand())+".tmp", "t", cache_size);
    if (xarray_->is_open()) {
      resize_array(h, w);    
      print_info();
    }
  }

  raw_image::~raw_image() {
    close();
  }

  void raw_image::open_to_write(uint32_t h, uint32_t w, std::string fname, uint32_t cache_size) {
    close();
    // open persistent array of requested size
    xarray_ = new xarray_value_t(fname, "w", cache_size);
    if (xarray_->is_open()) {
      resize_array(h, w);
      write_header();
      print_info();
    }
  }

  void raw_image::open_to_read(std::string fname, uint32_t cache_size) {
    close();
    xarray_ = new xarray_value_t(fname, "a", cache_size);
    if (xarray_->is_open()) {
      read_header();
      compute_tile_subdivision();
      print_info();
    }
  }

  void raw_image::close() {
    if (xarray_) {
      xarray_->close();
      delete xarray_;
      xarray_ = 0;
      width_ = 0;
      height_ = 0;
      x_tiles_ = 0;
      y_tiles_ = 0;
    }
  }

  void raw_image::resize_array(uint32_t h, uint32_t w) {
    assert(is_open());
    height_ = h;
    width_ = w;
    compute_tile_subdivision();
    xarray_->resize(x_tiles_ * y_tiles_ * tile_size_ + header_size_);
  }

  void raw_image::compute_tile_subdivision() {
    y_tiles_ = (height_ + tile_width_ - 1) / tile_width_;
    x_tiles_ = (width_ + tile_width_ - 1) / tile_width_;
  }

  void raw_image::read_header() {
    const uint32_t* h_ptr = reinterpret_cast<const uint32_t*>(xarray_->range_page_in(0, header_size_));
    height_ = h_ptr[0];
    width_ = h_ptr[1];
  }

  void raw_image::write_header() {
    uint32_t* h_ptr = reinterpret_cast<uint32_t*>(xarray_->range_page_in(0, header_size_));
    h_ptr[0] = height_;
    h_ptr[1] = width_;
  }
  
  void raw_image::print_info() const {
    if (is_open()) {
      std::cerr << "raw image: " << xarray_->file_name()
		<< " (" << height_ << ", " << width_ << ")"
		<< " tile width = " << tile_width_
		<< " tiles (" << y_tiles_ << ", " << x_tiles_ << ")" << std::endl;
    } else {
      std::cerr << "raw image: closed" << std::endl;
    }
  }
    
  void raw_image::set_value(value_t h) {
    assert(is_open());
    std::fill(xarray_->begin(), xarray_->end(), h);
  }
  
} // namespace cbdam
