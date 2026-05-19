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
#ifndef CBDAM_TEXTURE_LAYER_HPP
#define CBDAM_TEXTURE_LAYER_HPP

#include <vic/cbdam/base/config.hpp>
#include <vic/cbdam/base/geoimage_quad_fetcher.hpp>
#include <vic/cbdam/base/grid_texture_quadtree.hpp>
#include <vic/cbdam/base/texture_refiner.hpp>
#include <vic/cbdam/base/grid_diamond_graph_incore.hpp>
#include <vic/img/gl_image.hpp>
#include <vic/img/gl_quadtree_image_processor.hpp>
#include <sl/affine_map.hpp>
#include <sl/linear_map_factory.hpp>

namespace cbdam {
  
  /**
   * Simple accessor to a tile: level_xy, time_stamp, image*
   */
  class texture_tile_descriptor {
  public:
    typedef compressed_rgba32_image compressed_rgba_image_t;

  protected:
    grid_point_t		   level_xy_;
    uint64_t			   global_time_stamp_;
    const compressed_rgba_image_t* gl_image_;
    
  public:
    texture_tile_descriptor();
    
    texture_tile_descriptor(const grid_point_t& level_xy, uint64_t global_time_stamp, const compressed_rgba_image_t* gl_image);

    ~texture_tile_descriptor();

    grid_point_t& level_xy();

    const grid_point_t& level_xy() const;

    uint64_t& global_time_stamp();

    uint64_t global_time_stamp() const;

     void set_image(const compressed_rgba_image_t* img);
 
    const compressed_rgba_image_t* image() const;
 };

  /**
   * Stack of texture descriptor, able to build a unique id from the stack
   * and a unique representation.
   */
  class texture_tile_stack {
  public:
    typedef texture_tile_descriptor::compressed_rgba_image_t compressed_rgba_image_t;
    typedef vic::img::gl_image<>			uncompressed_rgba_image_t;
    typedef sl::affine_map<3, double>                   affine_map_t;
    typedef sl::fixed_size_point<4, uint8_t >           color4_t;
    typedef grid_diamond                                grid_diamond_t;
    
  protected:
    std::vector<texture_tile_descriptor>        tile_stack_;
    grid_point_t level_xy_;
    uint64_t global_time_stamp_;
    
  public:
    texture_tile_stack();

    ~texture_tile_stack();

    void insert_tile(const texture_tile_descriptor& x);

    void build_representation_in(uncompressed_rgba_image_t* img, uint8_t bg_r, uint8_t bg_g, uint8_t bg_b, uint8_t bg_a, bool add_boundaries = false);

    grid_point_t stack_level_xy();

    uint64_t stack_global_time_stamp();

    /// texture_matrix which map texture coordinates from diamond to stack tile
    affine_map_t texture_matrix(const grid_point_t& covering_d_level_xy, bool odd,
                                const grid_diamond_t& d, int patch_id, std::size_t root_count) const;
    
  };

  /**
   * Accessor to texture quadtree, instantiated with an already created texture fetcher
   */
  class texture_layer {
  public:
    typedef geoimage_quad_fetcher	texture_fetcher_t;

  protected:
    std::string                 id_;
    texture_fetcher_t*	        texture_fetcher_;
    texture_refiner*            texture_refiner_;
    grid_texture_quadtree*      quadtree_;
    bool                        is_active_;
    double			min_altitude_;
    double			max_altitude_;

  public:
    texture_layer(const std::string& id, 
		  texture_fetcher_t* tf, texture_refiner* tr, const coordinate_transform* geo_xform, 
		  std::size_t first_level, std::size_t last_level, 
		  double min_altitude = -10e30, double max_altitude = 10e30);
   
    ~texture_layer();

    grid_texture_quadtree& quadtree();

    const grid_texture_quadtree& quadtree() const;

    const std::string& id() const;
 
    texture_fetcher_t* fetcher();
    
    const texture_fetcher_t* fetcher() const;

    void set_active(bool x);

    bool is_active() const;

    bool is_camera_in_range(double camera_h) const;
    
  protected:
  
  };

} // namespace cbdam 

#endif // CBDAM_TEXTURE_LAYER_HPP

#ifndef CBDAM_TEXTURE_LAYER_IPP
#define CBDAM_TEXTURE_LAYER_IPP

namespace cbdam {

  ///////////////////////////////////////////// texture_tile_descriptor ////////////////////////////////////////////////

  inline texture_tile_descriptor::texture_tile_descriptor() :
      level_xy_(0,0,0), global_time_stamp_(0), gl_image_(0) {
    
  }
  
  inline texture_tile_descriptor::texture_tile_descriptor(const grid_point_t& level_xy, uint64_t global_time_stamp, const compressed_rgba_image_t* gl_image) :
      level_xy_(level_xy), global_time_stamp_(global_time_stamp), gl_image_(gl_image) {

  }

  inline texture_tile_descriptor::~texture_tile_descriptor() {

  }

  inline grid_point_t& texture_tile_descriptor::level_xy() {
    return level_xy_;
  }

  inline const grid_point_t& texture_tile_descriptor::level_xy() const {
    return level_xy_;
  }

  inline uint64_t& texture_tile_descriptor::global_time_stamp() {
    return global_time_stamp_;
  }

  inline uint64_t texture_tile_descriptor::global_time_stamp() const {
    return global_time_stamp_;
  }

  inline void texture_tile_descriptor::set_image(const compressed_rgba_image_t* img) {
    gl_image_ = img;
  }
  
  inline const texture_tile_descriptor::compressed_rgba_image_t* texture_tile_descriptor::image() const {
    return gl_image_;
  }

  ///////////////////////////////////////////// texture_tile_stack ////////////////////////////////////////////////

  inline texture_tile_stack::texture_tile_stack() {
    level_xy_ = grid_point_t(-1,0,0);
    global_time_stamp_ = 0;
  }

  inline texture_tile_stack::~texture_tile_stack() {
    tile_stack_.clear();
  }

  inline  void texture_tile_stack::insert_tile(const texture_tile_descriptor& x) {
    tile_stack_.push_back(x); 

    if (level_xy_[0] < x.level_xy()[0]) {
      level_xy_ = x.level_xy();
    }
#if 0
    if (global_time_stamp_ < x.global_time_stamp()) {
      global_time_stamp_ = x.global_time_stamp();
    }
#else
    // simple hash
    global_time_stamp_ +=  x.global_time_stamp();
#endif    
  }


  inline grid_point_t texture_tile_stack::stack_level_xy() {
    return level_xy_;
  }

  inline uint64_t texture_tile_stack::stack_global_time_stamp() {
    return global_time_stamp_;
  }

  ///////////////////////////////////////////// texture_layer ////////////////////////////////////////////////

  inline grid_texture_quadtree& texture_layer::quadtree() {
    assert(quadtree_);
    return *quadtree_;
  }

  inline const grid_texture_quadtree& texture_layer::quadtree() const {
    assert(quadtree_);
    return *quadtree_;
  }
  
  inline void texture_layer::set_active(bool x) {
    is_active_ = x;
  }

  inline bool texture_layer::is_active() const {
    return is_active_;
  }

  inline bool texture_layer::is_camera_in_range(double camera_h) const {
    return min_altitude_ < camera_h && camera_h < max_altitude_;
  }

  inline const std::string&  texture_layer::id() const {
    return id_;
  }

  inline texture_layer::texture_fetcher_t* texture_layer::fetcher() {
    return texture_fetcher_;
  }

  inline const texture_layer::texture_fetcher_t* texture_layer::fetcher() const {
    return texture_fetcher_;
  }

} // namespace cbdam 

#endif // CBDAM_TEXTURE_LAYER_IPP
