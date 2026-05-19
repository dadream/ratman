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
#ifndef CBDAM_GRID_DIAMOND_GRAPH_INCORE_HPP
#define CBDAM_GRID_DIAMOND_GRAPH_INCORE_HPP

#ifdef _WIN32
#undef min
#undef max
#endif

#include <vic/cbdam/base/grid_diamond_graph.hpp>
#include <vic/cbdam/base/reference_counted_cache.hpp>
#include <vic/cbdam/base/delta_height_codec.hpp>
#include <vic/cbdam/base/priority_diamond.hpp>
#include <vic/cbdam/base/repository_parameters.hpp>
#include <vic/cbdam/base/cbdam_diamond_fetcher.hpp>
#include <vic/cbdam/base/ray.hpp>
#include <sl/dense_array.hpp>
#include <sl/keyed_heap.hpp>
#include <sl/oriented_box.hpp>
#include <sl/projective_map.hpp>
#include <sl/rigid_body_map.hpp>
#include <sl/clock.hpp>
#include <cassert>

namespace cbdam {
  
  /**
   *
   */
  class grid_diamond_graph_incore : public grid_diamond_graph<grid_diamond_render_state> {
  public:
    typedef grid_point_t                                                        diamond_id_t;
    typedef grid_diamond                                                        grid_diamond_t;
    typedef grid_diamond_render_state                                           grid_diamond_state_t;
    typedef grid_diamond_graph<grid_diamond_state_t>                            super_t;
    typedef grid_diamond_state_t::bounding_volume_t                             bounding_volume_t;
    typedef std::map<grid_diamond_t, grid_diamond_state_t >                     grid_diamond_map_t;
    typedef grid_diamond_map_t::iterator                                        grid_diamond_map_iterator_t;
    typedef grid_diamond_map_t::const_iterator                                  grid_diamond_map_const_iterator_t;
    typedef std::pair<diamond_id_t, int>                                        diamond_patch_id_t;
    typedef reference_counted_cache_base<diamond_patch_id_t, diamond_vertices>  geometry_cache_t;
    typedef sl::dense_array<int32_t,2,void>                                     array2_height_t;
    typedef reference_counted_object<array2_height_t >                          geometry_reference_counted_array2_t;
    typedef cbdam_diamond_fetcher						geometry_fetcher_t;
    
    typedef sl::keyed_heap<diamond_id_t,  priority_diamond, coarsen_less>	coarsening_heap_t; 
    typedef sl::keyed_heap<diamond_id_t,  priority_diamond, refine_less>	refinement_heap_t;
    typedef sl::projective_map3d                                                projective_map_t;
    typedef sl::rigid_body_map3d                                                rigid_body_map_t;

  protected:
    geometry_cache_t                    geometry_cache_;
    const repository_parameters*        height_repository_parameters_;
    geometry_fetcher_t*			geometry_fetcher_;

    delta_height_codec                  delta_height_codec_;
    uint32_t                            decoded_diamond_count_;
    uint32_t                            decoded_diamond_budget_;
    double				data_missing_fraction_;
    uint32_t				texture_tile_width_;
    array2_height_t                     procedural_height_;
    
    refinement_heap_t                   refinement_heap_;
    coarsening_heap_t                   coarsening_heap_;    
    projective_map_t                    camera_pv_;
    point3d_t                           view_point_;
    float				previous_threshold_;
    bool                                is_open_;

    
  public:
    grid_diamond_graph_incore();

    virtual ~grid_diamond_graph_incore();

    virtual void clear();

    void open(geometry_fetcher_t* geometry_fetcher);

    bool is_open() const;
    
    void set_cache_capacity(std::size_t cc);
    
    void set_texture_tile_width(uint32_t texture_tile_width);

    void build_canonical_root(const grid_diamond_t& r,
                              const array2_height_t* offset_height);
    
    void time_critical_refine(std::size_t level,
                              diamond_id_t id, // recursive, pass by value!
			      const sl::real_time_clock& timeout_clock,
			      sl::uint64_t max_refine_time);

    void coarsen(std::size_t level, const diamond_id_t id); // recursive, pass by value!

    double patch_edge_length(const grid_diamond_t& d) const;

    uint32_t height_patch_dim() const;
    uint32_t patch_vertices_count() const;
    bool is_planar() const;
    const coordinate_transform* uvh_xyz_transform() const;
    double height_scale_factor() const;
    double data_missing_fraction() const;

    std::pair<float,float> estimated_elevation_range() const;
    
    void init_heaps();

    void extract_cut(float threshold, const projective_map_t& P, const rigid_body_map_t& V, int& incremental_updates_count);

    const projective_map_t& current_camera_pv() const;

    void set_decoded_diamond_budget(uint32_t x);

    bool is_visible(const bounding_volume_t& bv, const projective_map_t& camera_pv) const;

    bool is_visible(const bounding_volume_t& bv) const;

    double diamond_projected_error(const bounding_volume_t& bv) const;

    aabox2d_t aabox(const grid_diamond_t& d) const;

    void print() const;

  public:
   
    std::pair<bool,grid_diamond_map_const_iterator_t> diamond_at(std::size_t level, const diamond_id_t& id) const;

    const refinement_heap_t& refinement_heap() const;

    const geometry_cache_t& geometry_cache() const;

  protected:
    void prefetch_refine_data(float threshold);

    // recursive: pass by value
    void prefetch_refine_data(std::size_t level, diamond_id_t id);

    void recompute_heap_priority();

    void refine_update_heap(std::size_t level, const grid_diamond_map_const_iterator_t& it);

    void coarsen_update_heap(std::size_t level, const grid_diamond_map_const_iterator_t& it);

    priority_diamond get_priority_diamond(std::size_t level, const grid_diamond_map_const_iterator_t& it) const;

    void read_roots(uint32_t timeout_s);

  protected:
    bool check_all_children_are_leafs(std::size_t level, const grid_diamond_map_const_iterator_t& it) const;

    bool check_heap_consistency() const;

    grid_diamond_map_iterator_t get_or_create_child(std::size_t level, const grid_diamond_t& d_child,
                                                    std::size_t child_fragment_id, bool is_procedural);

    void refine_if_present(std::size_t level, diamond_id_t id);
     
    void refine(std::size_t level, grid_diamond_map_iterator_t& di);

    void set_bounding_volume(grid_diamond_map_iterator_t& it);


  };


} // namespace cbdam 

#endif // CBDAM_GRID_DIAMOND_GRAPH_INCORE_HPP

#ifndef CBDAM_GRID_DIAMOND_GRAPH_INCORE_IPP
#define CBDAM_GRID_DIAMOND_GRAPH_INCORE_IPP

namespace cbdam {

  inline bool grid_diamond_graph_incore::is_open() const {
    return is_open_;
  }
    
  inline const grid_diamond_graph_incore::projective_map_t& grid_diamond_graph_incore::current_camera_pv() const {
    return camera_pv_;
  }
  
  inline bool grid_diamond_graph_incore::is_visible(const bounding_volume_t& bv, const projective_map_t& camera_pv) const {
    bool visible = true;
    for(int i = 0; i < 6 && visible; ++i) {
      aabox3d_t::plane_t pl = camera_pv.clip_plane(i);
      if (bv.is_fully_below(pl)) visible = false;
    }
    return visible;    
  }

  inline bool grid_diamond_graph_incore::is_visible(const bounding_volume_t& bv) const {
    return is_visible(bv, current_camera_pv());
  }

  inline double grid_diamond_graph_incore::diamond_projected_error(const bounding_volume_t& bv) const {
    uint32_t patch_dim = height_patch_dim();
    double dist = bv.distance_to(view_point_);
    if (dist == 0) {
      return (double)(0x1<<30);
    } else {
      // FIXME: BAD Approx for viewpoint near box but far from center.
      vector3d_t dir = (bv.center() - view_point_).ok_normalized();
      double triangle_area = bv.projected_area(dir) / (patch_dim*patch_dim*2);
      double triangle_edge_length = sqrt(2*triangle_area);
      return triangle_edge_length / dist;
    }
  }

  inline uint32_t grid_diamond_graph_incore::height_patch_dim() const {
    return height_repository_parameters_->patch_dim();
  }
  
  inline uint32_t grid_diamond_graph_incore::patch_vertices_count() const {
    uint32_t N=height_patch_dim();
    return (N+1)*(N+2)/2;
  }

  inline bool grid_diamond_graph_incore::is_planar() const {
    return height_repository_parameters_->is_planar();
  }
  
  inline const coordinate_transform* grid_diamond_graph_incore::uvh_xyz_transform() const {
    return height_repository_parameters_->get_coordinate_transform();
  }

  inline double grid_diamond_graph_incore::data_missing_fraction() const {
    return data_missing_fraction_;
  }

  inline double grid_diamond_graph_incore::height_scale_factor() const {
    return height_repository_parameters_->height_scale_factor();
  }

  inline std::pair<bool, grid_diamond_graph_incore::grid_diamond_map_const_iterator_t> grid_diamond_graph_incore::diamond_at(std::size_t level, const diamond_id_t& id) const {
    std::pair<bool, grid_diamond_map_const_iterator_t> result = std::make_pair(false, diamond_map_by_level_[0]->begin());//grid_diamond_map_const_iterator_t());

    if (level < level_count()) {
      grid_diamond_t key = grid_diamond_t(id,id,id,id);
      result.second = diamond_map_by_level_[level]->find(key);
      result.first = (result.second != diamond_map_by_level_[level]->end());
    }
    return result;
  }

  inline const grid_diamond_graph_incore::refinement_heap_t& grid_diamond_graph_incore::refinement_heap() const {
    return refinement_heap_;
  }

  inline const grid_diamond_graph_incore::geometry_cache_t& grid_diamond_graph_incore::geometry_cache() const {
    return geometry_cache_;
  }

  inline aabox2d_t grid_diamond_graph_incore::aabox(const grid_diamond_t& d) const {
    const coordinate_transform* geo_xform = uvh_xyz_transform();
    point2d_t uv0 = geo_xform->uv_from_grid(d.corner(0));
    point2d_t uv1 = geo_xform->uv_from_grid(d.corner(1));
    point2d_t uv2 = geo_xform->uv_from_grid(d.corner(2));
    point2d_t uv3 = geo_xform->uv_from_grid(d.corner(3));

    aabox2d_t result;
    result.to(uv0);
    result.merge(uv1);
    result.merge(uv2);
    result.merge(uv3);
    return result;
  }

  inline priority_diamond grid_diamond_graph_incore::get_priority_diamond(std::size_t level, const grid_diamond_map_const_iterator_t& it) const {
    float error = 0.0f;
    const grid_diamond_state_t& ds = it->second;
    const bounding_volume_t& bv = ds.bounding_volume();
    const bool  visible = is_visible(bv, camera_pv_);

    // bound due to max supported level
    if (level < 40) { // FIXME
      if (visible) {
	error = diamond_projected_error(bv);
      }
    }

    return priority_diamond(error, visible, level);
  }

} // namespace cbdam 

#endif // CBDAM_GRID_DIAMOND_GRAPH_INCORE_IPP
