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
#ifndef CBDAM_GRID_TEXTURE_QUADTREE_HPP
#define CBDAM_GRID_TEXTURE_QUADTREE_HPP

#include <vic/cbdam/base/config.hpp>
#include <vic/cbdam/base/grid_point.hpp>
#include <vic/cbdam/base/reference_counted_cache.hpp>
#include <vic/cbdam/base/texture_refiner.hpp>
#include <vic/cbdam/base/coordinate_transform.hpp>
#include <vic/cbdam/base/geoimage_quad_fetcher.hpp>
#include <vic/cbdam/base/priority_diamond.hpp>
#include <vic/img/gl_image.hpp>
#include <sl/clock.hpp>


namespace cbdam {



  
  class grid_texture_state {
  public:
    typedef compressed_rgba32_image				compressed_rgba_image_t;
    typedef reference_counted_object<compressed_rgba_image_t >  reference_counted_compressed_image_t;

  protected:
    grid_point_t level_xy_;
    bool is_leaf_;
    bool exist_children_;
    const reference_counted_compressed_image_t* reference_counted_image_;
    
  public:
    grid_texture_state() :
      level_xy_(0,0,0), is_leaf_(true), exist_children_(true), reference_counted_image_(0) {

    }

    grid_texture_state(const grid_point_t& level_xy, bool is_leaf, bool exist_children, const reference_counted_compressed_image_t* img) :
      level_xy_(level_xy), is_leaf_(is_leaf), exist_children_(exist_children), reference_counted_image_(img) {
      if (reference_counted_image_) reference_counted_image_->ref();
    }

    grid_texture_state(const grid_texture_state& x) :
      level_xy_(x.level_xy_id()), is_leaf_(x.is_leaf()), exist_children_(x.exist_children()), reference_counted_image_(x.reference_counted_image()) {
      if (reference_counted_image_) reference_counted_image_->ref();
    }

    grid_texture_state& operator=(const grid_texture_state& x) {
      level_xy_ = x.level_xy_id();
      is_leaf_ = x.is_leaf();
      exist_children_ = x.exist_children();
      reference_counted_image_ = x.reference_counted_image();
      if (reference_counted_image_) reference_counted_image_->ref();

      return *this;
    }

    ~grid_texture_state() {
      if (reference_counted_image_) {
	reference_counted_image_->deref();
      }
    }

    CBDAM_RW_ACCESSOR(bool, is_leaf);
    CBDAM_RW_ACCESSOR(bool, exist_children);

    const reference_counted_compressed_image_t* reference_counted_image() const {
      return reference_counted_image_;
    }

    inline const grid_point_t& level_xy_id() const {
      return level_xy_;
    }
  };
  
  /**
   *
   */
  class grid_texture_quadtree {
  public:
    typedef grid_point_t                                                        diamond_id_t;
    typedef diamond_id_t                                                        key_t;
    typedef grid_diamond                                                        grid_diamond_t;
    typedef grid_texture_state                                                  grid_texture_state_t;
    typedef std::map<grid_diamond_t, grid_texture_state_t >                     grid_texture_diamond_map_t;
    typedef grid_texture_diamond_map_t::iterator                                grid_texture_diamond_map_iterator_t;
    typedef grid_texture_diamond_map_t::const_iterator                          grid_texture_diamond_map_const_iterator_t;
    typedef vic::img::gl_image<>                                                image_rgba_t;
    typedef compressed_rgba32_image						compressed_rgba_image_t;
    typedef reference_counted_object<compressed_rgba_image_t >                  reference_counted_compressed_image_t;
    typedef reference_counted_cache_base<key_t, reference_counted_compressed_image_t >     texture_cache_t;
    typedef geoimage_quad_fetcher						texture_fetcher_t;
    typedef sl::keyed_heap<diamond_id_t,  priority_diamond, coarsen_less>	coarsening_heap_t; // use priority diamonds setting visible always true
    typedef sl::keyed_heap<diamond_id_t,  priority_diamond, refine_less>	refinement_heap_t;

  protected:
    std::vector<grid_texture_diamond_map_t*>    diamond_map_by_level_;
    texture_cache_t                             texture_cache_;
    texture_fetcher_t*				texture_fetcher_;
    const texture_refiner*                      refiner_;
    const coordinate_transform*			coordinate_transform_;
    refinement_heap_t                           refinement_heap_;
    coarsening_heap_t                           coarsening_heap_;    
    uint32_t                                    decoded_diamond_count_;
    uint32_t                                    decoded_diamond_budget_;
    double                                      data_missing_fraction_;
    bool					is_open_;
    std::size_t					first_level_;
    std::size_t					last_level_;

  public:
    grid_texture_quadtree();

    ~grid_texture_quadtree();

    void clear();

    void set_texture_refiner(const texture_refiner* ref);
    
    void open(texture_fetcher_t* texture_fetcher, const coordinate_transform* geo_xform, std::size_t first_level = 0, std::size_t last_level = 64);

    void read_roots(uint32_t timeout_s);

    bool is_open() const;

    void set_cache_capacity(std::size_t cc);

    void set_decoded_diamond_budget(uint32_t x);

    const texture_cache_t& texture_cache() const;

    void init_heaps();

    void extract_cut(int& incremental_updates_count, bool recompute_hp, sl::uint64_t max_refine_time = 10);
    
    std::pair<bool,grid_texture_diamond_map_const_iterator_t> diamond_at(std::size_t level, const diamond_id_t& id) const;

  protected:
    static uint64_t global_time_stamp();
    
  public: // Queries
    /// The total number of levels
    std::size_t level_count() const;

    /// The number of diamonds in the given level
    std::size_t level_diamond_count(std::size_t level) const;

    std::size_t root_count() const;
    
    /// The total number of diamonds in the graph
    std::size_t diamond_count() const;

    /// fraction of the cut which should be refined, but its data is still missing
    double data_missing_fraction() const;

    bool has(std::size_t level, const diamond_id_t& id) const;

    static grid_point_t level_xy_from_diamond(std::size_t level, const grid_diamond_t& d, std::size_t root_count);

    std::size_t first_level() const;    

    std::size_t last_level() const;    

  protected:
    /// Create a new level
    void push_level_map();

    /// Delete last level map
    void pop_level_map();

    void refine(std::size_t level, diamond_id_t id);

    void coarsen(std::size_t level, const diamond_id_t id); // recursive, pass by value!

    void recompute_heap_priority();

    void refine_update_heap(std::size_t level, const grid_texture_diamond_map_const_iterator_t& it);

    void coarsen_update_heap(std::size_t level, const grid_texture_diamond_map_const_iterator_t& it);

    bool check_all_children_are_leafs(std::size_t level, const grid_texture_diamond_map_const_iterator_t& it) const;

    aabox2d_t aabox(const grid_diamond_t& d) const;

    priority_diamond get_priority_diamond(std::size_t level,
                                          const grid_texture_diamond_map_const_iterator_t& it) const;
 };


} // namespace cbdam 

#endif // CBDAM_GRID_TEXTURE_QUADTREE_HPP

#ifndef CBDAM_GRID_TEXTURE_QUADTREE_IPP
#define CBDAM_GRID_TEXTURE_QUADTREE_IPP

namespace cbdam {

  inline std::size_t grid_texture_quadtree::level_count() const {
    return diamond_map_by_level_.size();
  }

  inline std::size_t grid_texture_quadtree::level_diamond_count(std::size_t level) const {
    assert(level<level_count());
    return diamond_map_by_level_[level]->size();
  }

  inline std::size_t grid_texture_quadtree::root_count() const {
    return (level_count() == 0) ? 0 : level_diamond_count(0);
  }
    
  inline std::size_t grid_texture_quadtree::diamond_count() const {
    std::size_t result = 0;
    for (std::size_t level=0; level<diamond_map_by_level_.size(); ++level) {
      result += diamond_map_by_level_[level]->size();
    }
    return result;
  }

  inline double grid_texture_quadtree::data_missing_fraction() const {
    return data_missing_fraction_;
  }

  inline bool grid_texture_quadtree::has(std::size_t level, const diamond_id_t& id) const {
    if (level>=diamond_map_by_level_.size()) {
      return false;
    } else {
      grid_diamond_t key = grid_diamond_t(id,id,id,id);
      grid_texture_diamond_map_const_iterator_t it = diamond_map_by_level_[level]->find(key);
      if (it == diamond_map_by_level_[level]->end()) {
        return false;
      } else {
        return true;
      }
    }    
  }

  inline bool grid_texture_quadtree::is_open() const {
    return is_open_;
  }
  
  inline const grid_texture_quadtree::texture_cache_t& grid_texture_quadtree::texture_cache() const {
    return texture_cache_;
  }

  inline std::pair<bool, grid_texture_quadtree::grid_texture_diamond_map_const_iterator_t> grid_texture_quadtree::diamond_at(std::size_t level, const diamond_id_t& id) const {
    std::pair<bool, grid_texture_diamond_map_const_iterator_t> result = std::make_pair(false,  diamond_map_by_level_[0]->begin());//grid_texture_diamond_map_const_iterator_t());

    if (level < level_count()) {
      grid_diamond_t key = grid_diamond_t(id,id,id,id);
      result.second = diamond_map_by_level_[level]->find(key);
      result.first = (result.second != diamond_map_by_level_[level]->end());
    }
    return result;
  }

  inline void grid_texture_quadtree::push_level_map() {
    diamond_map_by_level_.push_back(new grid_texture_diamond_map_t());
  }

  inline void grid_texture_quadtree::pop_level_map() {
    assert(diamond_map_by_level_.size()>0);
    
    grid_texture_diamond_map_t* dmap = this->diamond_map_by_level_.back();
    this->diamond_map_by_level_.pop_back();
     
    delete dmap;
  }

  inline priority_diamond grid_texture_quadtree::get_priority_diamond(std::size_t level, const grid_texture_diamond_map_const_iterator_t& it) const {
    assert(refiner_);
    double error = 0.0;

    if (level < last_level_) {
      if (it->second.exist_children()) {
	// pass texture_level to refiner
        error = refiner_->error(level, it->first.id());
      }
    }
    
    return priority_diamond(error, true, level); // geometry level is = 2*texture_level
  }

  inline aabox2d_t grid_texture_quadtree::aabox(const grid_diamond_t& d) const {
    // Note: the procedure has to handle pole and wrap-around problems. 
    // We assume that the mapping from grid point to parametric 
    // is a simple scale+translate. We find the bbox in grid point
    // coordinate and shrink it before transformation to avoid 
    // transforming extrema.

    sl::axis_aligned_box<3,int> gp_box;
    gp_box.to(d.corner(0));
    gp_box.merge(d.corner(1));
    gp_box.merge(d.corner(2));
    gp_box.merge(d.corner(3));

    grid_point_t gp_lo = gp_box[0];
    grid_point_t gp_hi = gp_box[1];
    int plane = 2;
    if (gp_lo[0] == gp_hi[0]) plane=0;
    if (gp_lo[1] == gp_hi[1]) plane=1;

    gp_lo[(plane+1)%3] += 1;
    gp_lo[(plane+2)%3] += 1;

    gp_hi[(plane+1)%3] -= 1;
    gp_hi[(plane+2)%3] -= 1;
    
    point2d_t uv_lo = coordinate_transform_->uv_from_grid(gp_lo);
    point2d_t uv_hi = coordinate_transform_->uv_from_grid(gp_hi);

    // FIXME here, we should undo shrinking by slightly growing
    // the box in parametric coords. We do not do it, assuming
    // shrinking is negligible
    
    aabox2d_t result;
    result.to(uv_lo);
    result.merge(uv_hi);
    return result;
  }

  inline std::size_t grid_texture_quadtree::first_level() const {
    return first_level_;
  }  
  
  inline std::size_t grid_texture_quadtree::last_level() const {
    return last_level_;
  }
  
} // namespace cbdam 

#endif // CBDAM_GRID_TEXTURE_QUADTREE_IPP
