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
#ifndef CBDAM_GRID_DIAMOND_GRAPH_HPP
#define CBDAM_GRID_DIAMOND_GRAPH_HPP

#include <vic/cbdam/base/grid_diamond.hpp>
#include <vic/cbdam/base/grid_diamond_state.hpp>
#include <sl/serializer.hpp>
#include <vector>
#include <map>

namespace cbdam {

  /**
   *  Base class for a graph of diamonds, maintained as a per level
   *  map of diamonds to diamond states. Subclasses define how
   *  to create/destroy specific maps (in-core or out of core)
   */
  template <class GRID_DIAMOND_MAP_T, class GRID_DIAMOND_STATE_T = grid_diamond_state >
  class grid_diamond_graph_base {
  public:
    typedef grid_point_t          diamond_id_t;
    typedef grid_diamond          grid_diamond_t;
    typedef GRID_DIAMOND_STATE_T  grid_diamond_state_t;
    
    typedef GRID_DIAMOND_MAP_T    grid_diamond_map_t;

    typedef typename grid_diamond_map_t::const_iterator grid_diamond_map_const_iterator_t;

  protected: // Storage
    std::vector<grid_diamond_map_t*>    diamond_map_by_level_;

  protected: // Diamond map creation and destruction

    // Create a new level
    virtual void push_level_map() = 0;

    // Delete last level map
    virtual void pop_level_map() = 0;
      
  public: // Creation and destruction
    
    // Creation
    inline grid_diamond_graph_base() {
    }

    // Destruction
    virtual inline ~grid_diamond_graph_base() {
      // FIXME - Clear done in subclasses
    }

  protected: // Insertion primitives

    inline grid_diamond_t child_diamond(std::size_t /*l*/,
                                        const grid_diamond_t& d,
                                        int i, int j) const {
      // FIXME HACK HACK HACK
      // Not easy to get, we really need neighbor search!!
      // For now, we HACK under the assumption that the
      // root is either canonical planar or canonical
      // spherical
      switch (root_count()) {
      case 1: // ASSUME CANONICAL PLANAR!!
        return d.canonical_planar_child_diamond(i, j);
      case 6: // ASSUME CANONICAL SPHERICAL!!
        return d.canonical_spherical_child_diamond(i,j);
      case 8:
	return d.canonical_cylindrical_child_diamond(i, j);
      default:
        SL_FAIL("Implementation limitation: unable to do neighbor searching on general graphs");
        return grid_diamond_t();
      }        
    }
    
    inline void insert(std::size_t l,
                       const grid_diamond_t& d,
                       bool d_is_leaf,
                       bool d_has_fragment0,
                       bool d_has_fragment1) {
      assert(l<=level_count());
      // Expand graph if needed
      if (l==level_count()) {
        push_level_map();
      }
      assert(l<level_count());

      //std::cerr << "INSERT[" << l << "]" << d.id() << " leaf=" << d_is_leaf << " f0=" << d_has_fragment0 << " f1=" << d_has_fragment1 << std::endl;
      (*diamond_map_by_level_[l])[d] = grid_diamond_state_t(d_is_leaf, d_has_fragment0, d_has_fragment1);
      assert(has(d.id()));
      assert(level(d.id()) == l);
    }
    
    void create_or_update_child(std::size_t level, 
				grid_diamond_t d, int i, int j,
                                std::vector<diamond_id_t>* new_diamonds = 0) {
      // Expand graph if needed
      if (level+1 >= level_count()) {
        push_level_map();
      }

      // get ptr to child diamond if it exist
      diamond_id_t child_id = d.child_id(i,j);
      //std::cerr << "parent: " << d << std::endl;
      //std::cerr << "child(" << i << " " << j << ") = " << child_id << std::endl;

      grid_diamond_t child_key = grid_diamond_t(child_id, child_id, child_id, child_id);
      typename grid_diamond_map_t::iterator child_it = diamond_map_by_level_[level+1]->find(child_key);
      if (child_it == diamond_map_by_level_[level+1]->end()) {
        //std::cerr << "NOT FOUND" << std::endl;
        // not exist: build it 
        grid_diamond_t d_child = child_diamond(level, d, i, j);
        assert(d_child.id() == child_id);
        
	//	std::cerr << "grid_diamond_graph_base::access fragment_id (0): " << i << " " << j << std::endl;
        int child_fragment_id = d_child.fragment_id_deriving_from_parent(d.id());

	insert(level+1,
               d_child,
               true, // is leaf
               (child_fragment_id == 0),
               (child_fragment_id == 1));
        if (new_diamonds) new_diamonds->push_back(child_id);

        assert(has(child_id));
        assert(diamond_state(child_id).second.has_fragment(child_fragment_id));
      } else {
        //std::cerr << "FOUND" << std::endl;
        // exist: set true the fragment deriving from the parent
        const grid_diamond_t d_child = child_it->first;
        int child_fragment_id = d_child.fragment_id_deriving_from_parent(d.id());

        grid_diamond_state_t d_state = child_it->second;
        d_state.set_has_fragment(child_fragment_id, true);
    	child_it->second = d_state;

        assert(has(child_id));
        assert(diamond_state(child_id).second.has_fragment(child_fragment_id));
      }
    }

  public: // Clear and init
    
    virtual inline void clear() {
      while (diamond_map_by_level_.size() > 0) {
        pop_level_map();
      }
    }
    
    inline void canonical_init_planar() {
      clear();
      insert(0, grid_diamond_t::canonical_root(0), true, true, true);
    }

    inline void canonical_init_spherical() {
      clear();
      for(int i = 0; i < 6; ++i) {
        insert(0, grid_diamond_t::canonical_root(i), true, true, true);
      }
    }

    inline void canonical_init_cylindrical() {
      clear();
      for(int i = 0; i < 8; ++i) {
        insert(0, grid_diamond_t::cylindrical_canonical_root(i), true, true, true);
      }
    }
    
    
  public: // Queries

    /// The number of graph levels
    inline std::size_t level_count() const {
      return diamond_map_by_level_.size();
    }

    /// The number of diamonds in the given level
    inline std::size_t level_diamond_count(std::size_t level) const {
      assert(level<level_count());
      return diamond_map_by_level_[level]->size();
    }

    inline std::size_t root_count() const {
      return (level_count() == 0) ? 0 : level_diamond_count(0);
    }
    
    /// The total number of diamonds in the graph
    inline std::size_t diamond_count() const {
      std::size_t result = 0;
      for (std::size_t level=0; level<diamond_map_by_level_.size(); ++level) {
        result += diamond_map_by_level_[level]->size();
      }
      return result;
    }

    /// Iterator pointing to the first diamond in the level
    inline grid_diamond_map_const_iterator_t level_begin(std::size_t level) const {
      assert(level<level_count());
      return diamond_map_by_level_[level]->begin();
    }

    /// Iterator pointing after the last diamond in the level
    inline grid_diamond_map_const_iterator_t level_end(std::size_t level) const {
      assert(level<level_count());
      return diamond_map_by_level_[level]->end();
    }

  public: // Searching

    /// The pair level, iterator if present, (-1, undef) if not
    std::pair<std::size_t, grid_diamond_map_const_iterator_t> find(const diamond_id_t& id) const {
      std::pair<std::size_t, grid_diamond_map_const_iterator_t> result;

      grid_diamond_t key = grid_diamond_t(id,id,id,id);
      for (std::size_t l=0; (l<diamond_map_by_level_.size()); ++l) {
        grid_diamond_map_const_iterator_t it = diamond_map_by_level_[l]->find(key);
        if (it != diamond_map_by_level_[l]->end()) {
          return std::make_pair(l, it);
        }
      }

      result.first = std::size_t(-1);
      return result;
    }
    
    
    /// The state of diamond id at given level
    inline std::pair<bool, grid_diamond_state_t> diamond_state(std::size_t level,
                                                               const diamond_id_t& id) const {
      if (level>=diamond_map_by_level_.size()) {
        return std::make_pair(false, grid_diamond_state_t()); // NOT FOUND
      } else {
        grid_diamond_t key = grid_diamond_t(id,id,id,id);
        typename grid_diamond_map_t::const_iterator it = diamond_map_by_level_[level]->find(key);
        if (it == diamond_map_by_level_[level]->end()) {
          return std::make_pair(false, grid_diamond_state_t()); // NOT FOUND
        } else {
          return std::make_pair(true, it->second);
        }
      }
    }

    /// The state of diamond id
    inline std::pair<bool, grid_diamond_state_t> diamond_state(const diamond_id_t& id) const {
      std::pair<bool, grid_diamond_state_t> result = std::make_pair(false, grid_diamond_state_t());
      for (std::size_t level=0; (result.first==false) && (level<diamond_map_by_level_.size()); ++level) {
        result = diamond_state(level, id);
      }
      return result;
    }

    /// The level of diamond id
    inline std::size_t level(const diamond_id_t& id) const {
      std::size_t result = std::size_t(-1);
      for (std::size_t l=0; (result== std::size_t(-1)) && (l<diamond_map_by_level_.size()); ++l) {
        if (has(l,id)) {
          result = l;
        }
      }
      return result;
    }

    /// True iff level has id
    inline bool has(std::size_t level, const diamond_id_t& id) const {
      return diamond_state(level, id).first;
    }

    /// True iff graph has id
    inline bool has(const diamond_id_t& id) const {
      return diamond_state(id).first;
    }

    /// True iff diamond id at level l has given fragment (i.e., it is a root, or has
    /// it derived from a parent
    inline bool has_fragment(std::size_t level, const diamond_id_t& id, int i) const {
      return diamond_state(level, id).second.has_fragment(i);
    }

    /// True iff diamond id at level l has given fragment (i.e., it is a root, or has
    /// it derived from a parent
    inline bool has_fragment(const diamond_id_t& id, int i) const {
      return diamond_state(id).second.has_fragment(i);
    }
    
    inline bool is_leaf(std::size_t level, const diamond_id_t& id) const {
      return diamond_state(level, id).second.is_leaf();
    }
      
    inline bool is_leaf(const diamond_id_t& id) const {
      return diamond_state(id).second.is_leaf();
    }
    
  public: // Refinement

    void refine_if_present(std::size_t level,
                           diamond_id_t id, // By value!
                           std::vector<diamond_id_t>* new_diamonds = 0) {
      grid_diamond_t key = grid_diamond_t(id,id,id,id);
      typename grid_diamond_map_t::const_iterator it = diamond_map_by_level_[level]->find(key);
      
      if (it!= diamond_map_by_level_[level]->end()) {
        const grid_diamond_t d = it->first;
        //std::cerr << "DIAMOND(" << id << ") = " << std::endl << d << std::endl;
                
        grid_diamond_state_t ds = it->second;
        // Refine only leafs
        if (ds.is_leaf()) {

          // Check if parents need refinement
          if (level>0) {
            bool state_update = false;
            if (!ds.has_fragment(0) && d.is_valid_fragment(0)) { state_update = true; refine_if_present(level-1,d.parent_id(0), new_diamonds); }
            if (!ds.has_fragment(1) && d.is_valid_fragment(1)) { state_update = true; refine_if_present(level-1,d.parent_id(1), new_diamonds); }

            // Re-read state in case of parent refinement. Just to
            // simplify integration with out-of-core std::map 
            if (state_update) ds = diamond_map_by_level_[level]->find(key)->second;

            assert(!has(d.parent_id(0)) || ds.has_fragment(0));
            assert(!has(d.parent_id(1)) || ds.has_fragment(1));
          }
          
          // Mark as non leaf
          ds.set_is_leaf(false);
          (*diamond_map_by_level_[level])[d] = ds;
          assert(!is_leaf(id));
          
          // Create childrent
          if (ds.has_fragment(0) /*&& d.is_valid_fragment(0)*/) {
            //std::cerr << "id -> HAS 0" << std::endl;
            create_or_update_child(level, d, 0,0, new_diamonds);
            create_or_update_child(level, d, 0,1, new_diamonds);
          } 
          if (ds.has_fragment(1) /*&& d.is_valid_fragment(1)*/) {
            //std::cerr << "id -> HAS 1" << std::endl;
            create_or_update_child(level, d, 1,0, new_diamonds);
            create_or_update_child(level, d, 1,1, new_diamonds);
          }
        }
      }
    }
      
    void refine(std::size_t level,
		diamond_id_t id, // By value!
                std::vector<diamond_id_t>* new_diamonds = 0) {
      assert(has(level,id));
      assert(is_leaf(level,id));
      
      refine_if_present(level, id, new_diamonds);
    }
    
    void refine(const diamond_id_t& id, // By value!
                std::vector<diamond_id_t>* new_diamonds = 0) {
      assert(has(id));
      refine(level(id), id, new_diamonds);
    }

    // Make sure out-of-core rep is in sync if it exists
    virtual inline void commit() {

    }

  }; // class grid_diamond_graph_base<T>
  
} // namespace cbdam 


namespace cbdam {
  
  /**
   *  In-core graph of diamonds implemented with std::map 
   */
  template <class GRID_DIAMOND_STATE_T = grid_diamond_state>
  class grid_diamond_graph: 
    public grid_diamond_graph_base< std::map<grid_diamond, GRID_DIAMOND_STATE_T>, GRID_DIAMOND_STATE_T> {
  public:
    typedef grid_point_t          diamond_id_t;
    typedef grid_diamond          grid_diamond_t;
    typedef GRID_DIAMOND_STATE_T  grid_diamond_state_t;
    
    typedef std::map<grid_diamond_t, grid_diamond_state_t>  grid_diamond_map_t;

    typedef typename grid_diamond_map_t::const_iterator grid_diamond_map_const_iterator_t;

  protected: // Diamond map creation and destruction

    // Create a new level
    virtual void push_level_map() {
      this->diamond_map_by_level_.push_back(new grid_diamond_map_t());
    }

    // Delete last level map
    virtual void pop_level_map() {
      assert(this->diamond_map_by_level_.size()>0);
      
      grid_diamond_map_t* dmap = this->diamond_map_by_level_.back();
      this->diamond_map_by_level_.pop_back();

      delete dmap;
    }
    
  public:
    
    // Creation
    inline grid_diamond_graph() {
      // Nothing to do
    }

    // Destruction
    virtual inline ~grid_diamond_graph() {
      this->clear();
    }

  };
} // namespace cbdam


#endif // CBDAM_GRID_DIAMOND_GRAPH_HPP
