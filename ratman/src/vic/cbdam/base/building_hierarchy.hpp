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
#ifndef CBDAM_BUILDING_HIERARCHY_HPP
#define CBDAM_BUILDING_HIERARCHY_HPP

#include <vic/cbdam/base/config.hpp>
#include <vic/cbdam/base/color_rgb.hpp>
#include <sl/external_array.hpp>
#include <cassert>

namespace cbdam {

  // FIXME store object id or not in input_node and building_hierarchy_node ?
  // bbox is enough to identify an object ?

  class input_node {
  protected:
    aabox_t  bbox_;
    uint32_t first_point_index_;               /// index inside the vertices array
    uint32_t vertex_count_;
    color_rgba_t roof_color_;
    color_rgba_t facades_color_;
    
  public:
    input_node(const aabox_t&  bbox, uint32_t first_point_index, uint32_t vertex_count) :
        bbox_(bbox), first_point_index_(first_point_index), vertex_count_(vertex_count),
        roof_color_(255, 255, 255, 255), facades_color_(255, 255, 255, 255) {

    }

    CBDAM_RW_ACCESSOR(aabox_t, bbox);
    CBDAM_RW_ACCESSOR(uint32_t, first_point_index);
    CBDAM_RW_ACCESSOR(uint32_t, vertex_count);
    CBDAM_RW_ACCESSOR(color_rgba_t, roof_color);
    CBDAM_RW_ACCESSOR(color_rgba_t, facades_color);
  };

  typedef input_node input_node_t;
  
  class building_hierarchy_node {
  protected:
    aabox_t  bbox_;
    uint32_t parent_id_;
    uint32_t child_[2];
    uint32_t index_;            /// index inside the building_object_xarray_t for leafs, and split index for inner nodes
    uint32_t object_count_;
    uint32_t first_opengl_vertex_;
    uint32_t vertex_count_;
    uint32_t triangle_count_;
    
  public:
    building_hierarchy_node(const aabox_t&  bbox, uint32_t parent_id, uint32_t child_0 = uint32_t(-1),
                            uint32_t child_1 = uint32_t(-1), uint32_t index = uint32_t(-1), uint32_t object_count = uint32_t(-1),
                            uint32_t first_opengl_vertex = uint32_t(-1), uint32_t vertex_count = uint32_t(-1),
                            uint32_t triangle_count = uint32_t(-1)) :
        bbox_(bbox), parent_id_(parent_id), index_(index), object_count_(object_count),
    first_opengl_vertex_(first_opengl_vertex), vertex_count_(vertex_count), triangle_count_(triangle_count) {
      child_[0] = child_0;
      child_[1] = child_1;
    }

    CBDAM_RW_ACCESSOR(aabox_t, bbox);
    CBDAM_RW_ACCESSOR(uint32_t, parent_id);
    CBDAM_RW_ACCESSOR(uint32_t, index);
    CBDAM_RW_ACCESSOR(uint32_t, object_count);
    CBDAM_RW_ACCESSOR(uint32_t, first_opengl_vertex);
    CBDAM_RW_ACCESSOR(uint32_t, vertex_count);
    CBDAM_RW_ACCESSOR(uint32_t, triangle_count);

    uint32_t& child(uint32_t child_id) {
      return child_[child_id];
    }
    
    uint32_t child(uint32_t child_id) const {
      return child_[child_id];
    }
  };

  class building_object {
  protected:
    float bottom_;
    float top_;
    uint32_t first_point_index_;
    uint32_t vertex_count_;

  public:
    building_object(float bottom, float top, uint32_t first_point_index, uint32_t vertex_count) :
        bottom_(bottom), top_(top), first_point_index_(first_point_index), vertex_count_(vertex_count) {

    }

    CBDAM_RW_ACCESSOR(float, bottom);
    CBDAM_RW_ACCESSOR(float, top);
    CBDAM_RW_ACCESSOR(uint32_t, first_point_index);
    CBDAM_RW_ACCESSOR(uint32_t, vertex_count);
  };

  typedef sl::point2f   building_vertex_t;
  
  class building_hierarchy {
  public:
    typedef sl::fixed_size_vector<sl::row_orientation,3,int16_t> normal_i16_t;
    typedef sl::external_array1<input_node_t>            input_node_xarray_t;
    typedef sl::external_array1<building_hierarchy_node> building_hierarchy_node_xarray_t;
    typedef sl::external_array1<building_object>         building_object_xarray_t;
    typedef sl::external_array1<building_vertex_t>       building_vertex_xarray_t;
    typedef sl::external_array1<point3_t>                building_opengl_vertex_xarray_t;
    typedef sl::external_array1<normal_i16_t>            building_opengl_normal_xarray_t;
    typedef sl::external_array1<color_rgba_t>            building_opengl_color_xarray_t;
    
  protected:
    input_node_xarray_t*                input_nodes_;
    building_hierarchy_node_xarray_t*   building_hierarchy_;
    building_object_xarray_t*           building_objects_;
    building_vertex_xarray_t*           building_vertices_;
    building_opengl_vertex_xarray_t*    opengl_vertices_;
    building_opengl_normal_xarray_t*    opengl_normals_;
    building_opengl_color_xarray_t*     opengl_colors_;
    
    uint32_t max_node_vertex_count_;
    uint32_t max_depth_;
    uint32_t current_outline_;
    double u_extent_;
    double v_extent_;
    
    std::string base_name_;
    bool        last_operation_succes_;
    
  public:

    building_hierarchy();

    ~building_hierarchy();

    void set_extent(double u_extent,
                    double v_extent);

    void scene_begin(const char* file_name);

    void scene_end();

    /**
     * outline count ignored: insert only first outline
     */
    void building_begin(double umin, double vmin, double zmin,
                        double umax, double vmax, double zmax,
                        uint32_t outlines_count);

    void building_end();
    
    void building_color(const color_rgba_t& roof_color,
                        const color_rgba_t& facades_color);
    
    void building_outline_begin(uint32_t vertices_count);

    void building_outline_end();

    void building_vertex(double u, double v);

    void open_scene(const char* file_name);

    void close();

    bool last_operation_success() const;

    void print_bsp() const;

    //    uint32_t get_node_containing(const aabox_t& box);

    bool is_leaf(uint32_t node_idx) const;

    uint32_t size() const;

    building_hierarchy_node node(uint32_t node_idx) const;

    building_object object(uint32_t obj_idx) const;

    const building_vertex_t* vertices(uint32_t first_vertex, uint32_t& vertex_count) const;

    const point3_t* opengl_vertices(uint32_t first_vertex, uint32_t& vertex_count) const;

    const normal_i16_t* opengl_normals(uint32_t first_vertex, uint32_t& vertex_count) const;

    const color_rgba_t* opengl_colors(uint32_t first_vertex, uint32_t& vertex_count) const;

    CBDAM_RW_ACCESSOR(uint32_t, max_node_vertex_count);

    bool height_from_uv(double u, double v, double& top, double& bottom) const;

    bool height_from_xy(double x, double y, double& top, double& bottom) const;
    
    bool is_point_inside_current_input_building(double u, double v) const;

  protected:
    void build_bsp();
    
    void build_bsp(uint32_t parent_id, uint32_t child_id, 
                   input_node_xarray_t& node_array,
                   const aabox_t& bbox, uint32_t sum_vertex_count,
                   uint32_t depth);

    void split_xarray(input_node_xarray_t& samples, const aabox_t& bbox,
                      input_node_xarray_t& left_samples, aabox_t& left_box,
                      input_node_xarray_t& right_samples, aabox_t& right_box,
                      uint32_t& split_index, uint32_t& left_sum_vertex_count) const;

    void build_opengl_data(uint32_t node_idx,
                           const std::vector<color_rgba_t>& roof_colors,
                           const std::vector<color_rgba_t>& facades_color);
    
    bool is_point_inside_buildings(uint32_t node_idx, double x, double y, double& top, double& bottom) const;

    bool is_point_inside_polygon(uint32_t first_vertex, uint32_t vertex_count, double x, double y) const;

    void close_strip(const point3_t& v, const point3_t& old_v,
                     const color_rgba_t& c, const color_rgba_t& old_c,
                     uint32_t& strip_size);

    std::string tmp_xarray_name() const;

    building_vertex_t vertex_abs_coords(double u, double v) const;
  };
}

#endif  // CBDAM_BUILDING_HIERARCHY_HPP

#ifndef CBDAM_BUILDING_HIERARCHY_IPP
#define CBDAM_BUILDING_HIERARCHY_IPP

namespace cbdam {

  inline bool building_hierarchy::is_leaf(uint32_t node_idx) const {
    assert(node_idx < building_hierarchy_->size());
    const building_hierarchy_node& node = (*building_hierarchy_)[node_idx];
    return node.child(0) == uint32_t(-1);
  }

  inline building_hierarchy_node building_hierarchy::node(uint32_t node_idx) const {
    assert(building_hierarchy_ && node_idx < building_hierarchy_->size());
    return (*building_hierarchy_)[node_idx];
  }

  inline building_object building_hierarchy::object(uint32_t obj_idx) const {
    assert(obj_idx < building_objects_->size());
    return (*building_objects_)[obj_idx];
  }

  inline const sl::point2f* building_hierarchy::vertices(uint32_t first_vertex, uint32_t& vertex_count) const {
    assert(first_vertex + vertex_count <= building_vertices_->size());
    return building_vertices_->range_page_in(first_vertex, first_vertex + vertex_count - 1);
  }

  inline const point3_t* building_hierarchy::opengl_vertices(uint32_t first_vertex, uint32_t& vertex_count) const {
    assert(first_vertex + vertex_count <= opengl_vertices_->size());
    return opengl_vertices_->range_page_in(first_vertex, first_vertex + vertex_count - 1);
  }

  inline const building_hierarchy::normal_i16_t* building_hierarchy::opengl_normals(uint32_t first_vertex, uint32_t& vertex_count) const {
    assert(first_vertex + vertex_count <= opengl_normals_->size());
    return opengl_normals_->range_page_in(first_vertex, first_vertex + vertex_count - 1);
  }

  inline const color_rgba_t* building_hierarchy::opengl_colors(uint32_t first_vertex, uint32_t& vertex_count) const {
    assert(first_vertex + vertex_count <= opengl_colors_->size());
    return opengl_colors_->range_page_in(first_vertex, first_vertex + vertex_count - 1);
  }

  inline uint32_t building_hierarchy::size() const {
    assert(building_hierarchy_);
    return building_hierarchy_->size();
  }

  inline building_vertex_t building_hierarchy::vertex_abs_coords(double u, double v) const {
    // buildings centered at 0, between [-u_extent/2, u_extent/2] same for v
    return building_vertex_t((u - 0.5f)*u_extent_, (v - 0.5f)*v_extent_);
  }
}

#endif // CBDAM_BUILDING_HIERARCHY_IPP
