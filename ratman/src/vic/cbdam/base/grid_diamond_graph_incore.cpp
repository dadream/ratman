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
#include <vic/cbdam/base/grid_diamond_graph_incore.hpp>
#include <sl/utility.hpp>
#include <stack>

#ifdef _WIN32
  #undef max
  #undef min
#endif


namespace cbdam {

  grid_diamond_graph_incore::grid_diamond_graph_incore() {
    decoded_diamond_count_ = 0;
    decoded_diamond_budget_ = 16;
    data_missing_fraction_ = 0.0;
    is_open_ = false;
    texture_tile_width_ = 0;
    previous_threshold_ = 1.0;

    geometry_fetcher_ = 0;

    geometry_cache_.set_capacity(50); // FIXME CHECK
  }
  

  grid_diamond_graph_incore::~grid_diamond_graph_incore() {
    SL_TRACE_OUT(1) << "~grid_diamond_graph_incore()" << std::endl;
    clear();
    //    SL_TRACE_OUT(1) << "~grid_diamond_graph_incore() done" << std::endl;
  }

  void grid_diamond_graph_incore::clear() {
    // dereference everything in the caches
    SL_TRACE_OUT(1) << "grid_diamond_graph_incore::clear level_count " << level_count() << std::endl;
    super_t::clear();
  }
  
  void grid_diamond_graph_incore::open(geometry_fetcher_t* geometry_fetcher) {
    clear();    

    geometry_fetcher_ = geometry_fetcher;

    height_repository_parameters_ = &(geometry_fetcher_->get_repository_parameters());
    delta_height_codec_.init(height_patch_dim(), height_scale_factor(), uvh_xyz_transform());
    procedural_height_.resize(sl::index<2>(height_patch_dim(), height_patch_dim()));
#if 0
    // FIXME ######################
    SL_TRACE_OUT(-1) << "PROCEDURAL HACK!" << std::endl;
    for (std::size_t i=0; i<height_patch_dim(); ++i) {
      for (std::size_t j=0; j<height_patch_dim(); ++j) {
	procedural_height_(i,j) = -32+random()%64; // FIXME
      }
    }
    // FIXME ######################
#endif
    read_roots(30);
    SL_TRACE_OUT(-1) << "read root " << (is_open_? "OK" : "FAILED") << std::endl;
  }

  void grid_diamond_graph_incore::read_roots(uint32_t timeout_s) {
    // compile request lists
    is_open_ = false;
    std::string root_url = geometry_fetcher_->base_url();
    root_url = sl::pathname_without_extension(root_url) + ".root";
    geometry_fetcher_t* root_fetcher = new geometry_fetcher_t(root_url);
    root_fetcher->connect();
    if (!root_fetcher->is_connected()) {
      SL_TRACE_OUT(-1) << "root fetcher not connected" << std::endl;
      return;
    } else {
      std::size_t root_count = uvh_xyz_transform()->root_count();
      SL_TRACE_OUT(1) << "read_roots " << root_count << std::endl;

      if (root_count != 1 && root_count != 6 && root_count != 8) {
	SL_TRACE_OUT(1) << "diamond graph wrong root count " << root_count << " can be 1 for planar, 6 for spherical, 8 for cylindrical" << std::endl;
	return;
      }

      // build vector of root diamonds and root ids
      std::vector<grid_diamond_t> roots;
      std::vector<diamond_id_t> root_ids;
      std::vector<aabox2d_t> root_box;
      for(std::size_t i = 0; i < root_count; ++i) {
	if (root_count == 1 || root_count == 6) {
	  grid_diamond_t r = grid_diamond_t::canonical_root(i);
	  roots.push_back(r);
	  root_ids.push_back(r.id());
	  root_box.push_back(aabox(r)); // FIXME 
	} else {
	  // root_count = 8
	  grid_diamond_t r = grid_diamond_t::cylindrical_canonical_root(i);
	  roots.push_back(r);
	  root_ids.push_back(r.id());
	  root_box.push_back(aabox(r)); // FIXME 
	}
      }
      
      // read geometry roots
      std::size_t build_root_count = 0;
      for(std::size_t i = 0; i < root_count; ++i) {
	geometry_fetcher_t::status_data_pair_t res = root_fetcher->synchronous_fetch(root_ids[i], root_box[i], timeout_s);
	switch(res.first) {
	case geometry_fetcher_t::DONE: {
	  SL_TRACE_OUT(1) << "geometry root " << i << " data" << std::endl;
	  const array2_height_t* offset_height = res.second;
	  build_canonical_root(roots[i], offset_height);
	  delete offset_height;
	  ++build_root_count;
	} break;
	case geometry_fetcher_t::NULL_DATA:
	  SL_TRACE_OUT(1) << "geometry root " << i << " null data" << std::endl;
	  break;
	case geometry_fetcher_t::FAILED:
	  SL_TRACE_OUT(-1) << "geometry failed to read root " << i << std::endl;
	  break;
	case geometry_fetcher_t::NOT_FOUND:
	  SL_TRACE_OUT(-1) << "syncrhonous call returned NOT FOUND accessing root " << i << std::endl;
	  break;
	}
      }
      if (build_root_count == root_count) {
	is_open_ = true;
      } 
      SL_TRACE_OUT(1) << "read " << build_root_count << " roots" << std::endl;
    }
    delete root_fetcher;
    SL_TRACE_OUT(1) << "end function" << std::endl;
  }
  
  void grid_diamond_graph_incore::set_cache_capacity(std::size_t cc) {
    geometry_cache_.set_capacity(cc);
  }

  void grid_diamond_graph_incore::set_texture_tile_width(uint32_t texture_tile_width) {
    texture_tile_width_ = texture_tile_width;
  }

  void grid_diamond_graph_incore::build_canonical_root(const grid_diamond_t& r,
                                                       const array2_height_t* offset_height) {
    assert(offset_height);
    if (0==level_count()) {
      push_level_map();
    }

    // set diamond and diamond state
    diamond_id_t id = r.id();
    int patch_dim = height_patch_dim();
    double el = patch_edge_length(r)/(double)patch_dim;
    grid_diamond_state_t ds(true, true, true, el, false);

    // allocate room in the cache / decode data
    diamond_vertices* dv0 = new diamond_vertices(&geometry_cache_, patch_vertices_count());
    diamond_vertices* dv1 = new diamond_vertices(&geometry_cache_, patch_vertices_count());
    //    dv0->ref();    dv1->ref(); called from set_reference_counted_patch_data
    ds.set_reference_counted_patch_data(0, dv0);
    ds.set_reference_counted_patch_data(1, dv1);
    (*diamond_map_by_level_[0])[r] = ds;

    geometry_cache_.insert(std::make_pair(id, 0), dv0);
    geometry_cache_.insert(std::make_pair(id, 1), dv1);
    delta_height_codec_.distribute_data_to_root(*offset_height, r, dv0, dv1);

    grid_diamond_map_iterator_t it = diamond_map_by_level_[0]->find(r);
    assert(it != diamond_map_by_level_[0]->end());
    set_bounding_volume(it);
  }

  grid_diamond_graph_incore::grid_diamond_map_iterator_t grid_diamond_graph_incore::get_or_create_child(std::size_t level,
                                                                                                        const grid_diamond_t& d_child,
                                                                                                        std::size_t child_fragment_id,
                                                                                                        bool is_procedural) {
    // Expand graph if needed d_child is on level l
    if (level >= level_count()) {
      push_level_map();
    }
    
    // get ptr to child diamond if it exist
    diamond_id_t                child_id  = d_child.id();
    grid_diamond_t              child_key = grid_diamond_t(child_id, child_id, child_id, child_id);
    grid_diamond_map_iterator_t child_it  = diamond_map_by_level_[level]->find(child_key);
    if (child_it == diamond_map_by_level_[level]->end()) {
      // NOT FOUND build a new one
      double el = patch_edge_length(d_child)/(double)height_patch_dim();
      grid_diamond_state_t ds_child(true, child_fragment_id == 0, child_fragment_id == 1, el, is_procedural);
      (*diamond_map_by_level_[level])[d_child] = ds_child;

      child_it = diamond_map_by_level_[level]->find(child_key);
    } else {
      // update existing one diamond state
      child_it->second.set_has_fragment(child_fragment_id, true);
      child_it->second.set_procedural(is_procedural);
    }

    return child_it;
  }

  void grid_diamond_graph_incore::time_critical_refine(std::size_t level,
                                                       diamond_id_t id, // recursive, pass by value!
                                                       const sl::real_time_clock& timeout_clock,
                                                       sl::uint64_t max_refine_time) {
    
    grid_diamond_t key = grid_diamond_t(id,id,id,id);
    grid_diamond_map_iterator_t it = diamond_map_by_level_[level]->find(key);

    if (it!= diamond_map_by_level_[level]->end()) {
      const grid_diamond_t& d = it->first;
      const grid_diamond_state_t& ds = it->second;
      assert(ds.is_leaf());

      if (level>0) {
        if (!ds.has_fragment(0) && has(d.parent_id(0))) {
          time_critical_refine(level-1, d.parent_id(0), timeout_clock, max_refine_time);
        }
        if (sl::uint64_t(timeout_clock.elapsed().as_milliseconds()) >= max_refine_time &&
            decoded_diamond_count_ > 0) {
          return;
        }
        if (!ds.has_fragment(1) && has(d.parent_id(1))) {
          time_critical_refine(level-1, d.parent_id(1), timeout_clock, max_refine_time);
        }
        if (sl::uint64_t(timeout_clock.elapsed().as_milliseconds()) >= max_refine_time &&
            decoded_diamond_count_ > 0) {
          return;
        }
      }
      refine(level, it);
    }
  }

  void grid_diamond_graph_incore::refine_if_present(std::size_t level, diamond_id_t id) {
    grid_diamond_t key = grid_diamond_t(id,id,id,id);
    grid_diamond_map_iterator_t it = diamond_map_by_level_[level]->find(key);

    if (it!= diamond_map_by_level_[level]->end()) {
      refine(level, it);
    }
  }

  void grid_diamond_graph_incore::refine(std::size_t level, grid_diamond_map_iterator_t& it) {
    const grid_diamond_t d = it->first;
    grid_diamond_state_t& ds = it->second;
    diamond_id_t id = d.id();
    grid_diamond_t key = grid_diamond_t(id,id,id,id);

    assert(ds.is_leaf());

     // CHECK IF PARENTS NEED REFINEMENT
    if (level>0) {
      // after call refine check if d has requested patches or if returned beacause data was not available
      for(std::size_t i = 0; i < 2; ++i) {
        if (!ds.has_fragment(i) && d.is_valid_fragment(i)) {
          refine_if_present(level-1,d.parent_id(i));
          if (!ds.has_fragment(i)) {
            return;
          }
        }
      }

      // Here: has_fragment(i) is true if not a border
      assert(!has(d.parent_id(0)) || ds.has_fragment(0));
      assert(!has(d.parent_id(1)) || ds.has_fragment(1));
    }

    // CREATE 4 DIAMOND CHILDREN, GET POINTER TO THEIR DATA IN THE CACHE IF EXIST
    grid_diamond_t d_child[4];
    std::size_t patch_id[4];
    diamond_vertices* dv[4] = {0,0,0,0};
    bool all_in_cache = true;
    for(int i = 0; i < 2; ++i) {
      if (ds.has_fragment(i)) {
        for(int j = 0; j < 2; ++j) {
          int idx = 2*i+j;
          d_child[idx] = child_diamond(level, d, i, j);

	  int check_i = 0;
	  int check_j = 0;
	  if (d.child_ij_from_child_id(d_child[idx].id(), check_i, check_j)) {
	    if (check_i != i || check_j != j) {
	      SL_TRACE_OUT(-1) << "CHILD_IJ WRONG " << check_i << " != " << i << " || " << check_j << " != " << j << std::endl;
	    }
	  }
	  //	  std::cerr << "access fragment_id (1): " << i << " " << j <<  ", level " << level <<std::endl;
          patch_id[idx] = d_child[idx].fragment_id_deriving_from_parent(id);
          std::pair<diamond_id_t, std::size_t> diamond_patch_id = std::make_pair(d_child[idx].id(), patch_id[idx]);
          dv[idx] = geometry_cache_[diamond_patch_id];
          all_in_cache &= (dv[idx] != 0);
        }
      }
    }

    if (all_in_cache) {
      // EVERYTHING ALREADY IN CACHE, SIMPLY INSERT DIAMOND IN GRAPH AND REFERENCE DATA IN CACHE
      SL_TRACE_OUT(1) << "all_in_cache" << std::endl;
      ds.set_is_leaf(false);
      for(int i = 0; i < 2; ++i) {
        if (ds.has_fragment(i)) {
          for(int j = 0; j < 2; ++j) {
            int idx = 2*i+j;
            assert(dv[idx]);
	    //            dv[idx]->ref(); called from set_reference_counted_patch_data

            grid_diamond_map_iterator_t c_it = get_or_create_child(level+1, d_child[idx], patch_id[idx], ds.is_procedural());
	    // set pointer to the reference counted patch vertices
	    c_it->second.set_reference_counted_patch_data(patch_id[idx], dv[idx]);
            set_bounding_volume(c_it);
          }
        }
      }
      
       ++decoded_diamond_count_;
       refine_update_heap(level, it);
    } else {
      // SOMETHING COULD BE IN CACHE, CREATE A NEW DIAMOND IN GRAPH, DECODE DATA AND INSERT IN CACHE

      // ACCESS DATA FROM ACCESS CHANNELS (all these lines (next 2 blocks))
      array2_height_t* offset_height = 0;
      if (ds.is_procedural()) {
	// Get precomputed data
	offset_height = &procedural_height_;
      } else {
	// try to read
       geometry_fetcher_t::status_data_pair_t res = geometry_fetcher_->fetch(id, aabox(d));
	if (res.first == geometry_fetcher_t::DONE) {
	  offset_height = res.second;
	  SL_TRACE_OUT(1) << id << " DONE" << std::endl;
	} else if (res.first == geometry_fetcher_t::NULL_DATA) {
	  ds.set_procedural(true);
	  offset_height = &procedural_height_;
	  SL_TRACE_OUT(1) << id << " NULL" << std::endl;
	} else {
	  SL_TRACE_OUT(1) << id << " OTHER " << res.first << std::endl;
	}
	 
	// if NOT_FOUND or FAILED: let offset_height = 0
      } 

      bool is_refinable = offset_height != 0;
      if (is_refinable) {
        // CREATE NEW DIAMOND DATA WITH WAVELET SYNTHESIS
        SL_TRACE_OUT(1) << "create new data for " << id << std::endl;
        // Mark as non leaf
        ds.set_is_leaf(false);
        
        const diamond_vertices* dv_parent[2] = {0,0};
        grid_diamond_map_iterator_t c_it[4];
        for(int i = 0; i < 2; ++i) {
          if (ds.has_fragment(i)) {
            // get parent data
	    //            dv_parent[i] = geometry_cache_[std::make_pair(id, i)];
            dv_parent[i] = ds.reference_counted_patch_data(i);
            assert(dv_parent[i]);
            
            for(int j = 0; j < 2; ++j) {
              // insert child in graph and allocate its data
              int idx = 2*i+j;
              assert(idx<4);
              std::pair<diamond_id_t, std::size_t> diamond_patch_id = std::make_pair(d_child[idx].id(), patch_id[idx]);
              if (dv[idx] == 0) {
                dv[idx] = new diamond_vertices(&geometry_cache_, patch_vertices_count());
		//                dv[idx]->ref(); called from set_reference_counted_patch_data
                geometry_cache_.insert(diamond_patch_id, dv[idx]);
	      } else {
		//		dv[idx]->ref();  called from set_reference_counted_patch_data
	      }
              assert(dv[idx]);
              c_it[idx] = get_or_create_child(level+1, d_child[idx], patch_id[idx], ds.is_procedural());
	      c_it[idx]->second.set_reference_counted_patch_data(patch_id[idx], dv[idx]);
            }
          } // has_fragment
        }

        // DECODE HEIGHT VALUES, all this could be grouped in the decode values function
        delta_height_codec_.decode_values(*offset_height, d, dv_parent[0], dv_parent[1]);
	if (offset_height != &procedural_height_) {
	  delete offset_height;
	}
        for(int i = 0; i < 2; ++i) {
          if (ds.has_fragment(i)) {
            for(int j = 0; j < 2; ++j) {
              int idx = 2*i+j;
              delta_height_codec_.compute_patch_3dpoints(d_child[idx], patch_id[idx], i, j);
            }
          }
        }
        delta_height_codec_.fill_matrix_normal();

        // DISTRIBUTE DECODED DATA TO CHILDREN
        for(int i = 0; i < 2; ++i) {
          if (ds.has_fragment(i)) {
            for(int j = 0; j < 2; ++j) {
              int idx = 2*i+j;
              delta_height_codec_.distribute_data_to_child(d_child[idx].id(), dv[idx], i, j);
              assert(dv[idx]);
              set_bounding_volume(c_it[idx]);
            }
          }
        }
        ++decoded_diamond_count_;
        refine_update_heap(level, it);
      }
    }
  }

  void grid_diamond_graph_incore::coarsen(std::size_t level, const diamond_id_t id) { // recursive, pass by value!
    SL_TRACE_OUT(1) << "coarsen at " << level << " : " << id << std::endl; 
    grid_diamond_t key = grid_diamond_t(id,id,id,id);
    grid_diamond_map_iterator_t it = diamond_map_by_level_[level]->find(key);
    assert(it!= diamond_map_by_level_[level]->end());
    
    const grid_diamond_t d = it->first;
    grid_diamond_state_t& ds = it->second;

    // All children are leaf here
    for (int i=0; i<2; ++i) {
      for (int j=0; j<2; ++j) {
        diamond_id_t d_child_id = d.child_id(i,j);
        //          diamond_t* d_child = get_diamond(d_child_id);
        assert(level+1 < level_count());

        grid_diamond_t key_child = grid_diamond_t(d_child_id,d_child_id,d_child_id,d_child_id);
        grid_diamond_map_iterator_t it_child = diamond_map_by_level_[level+1]->find(key_child);
        if (it_child != diamond_map_by_level_[level+1]->end()) {
          const grid_diamond_t d_child = it_child->first;
          grid_diamond_state_t& ds_child = it_child->second;
            
          // deref patch data
	  //	  std::cerr << "access fragment_id (2): " << i << " " << j << ", level " << level << std::endl;
          int patch_id = d_child.fragment_id_deriving_from_parent(id);
#if 0
          diamond_patch_id_t dp_id = std::make_pair(d_child_id, patch_id);
          geometry_cache_[dp_id]->deref();
#else
	  //	  ds_child.reference_counted_patch_data(patch_id)->deref(); called from ~ds_child
#endif
          ds_child.set_has_fragment(patch_id, false);
	  //          it_child->second = ds_child;

          // if also other patch has been deleted remove d from map
          if (!ds_child.has_fragment(1-patch_id)) {
	    //	    std::cerr << "geometry erase child " << d_child_id << std::endl;
            diamond_map_by_level_[level+1]->erase(it_child);
          } 
        } 
      }
    }

    // FIXME CHECK VALIDITY
    ds.set_is_leaf(true);
    coarsen_update_heap(level, it);
  }
  
  void grid_diamond_graph_incore::coarsen_update_heap(std::size_t level,
                                                      const grid_diamond_map_const_iterator_t& it) {

   // coarsed id :
    // - remove id children from refinement_heap_
    const grid_diamond_t& d = it->first;
    for (int i=0; i<2; ++i) {
      for (int j=0; j<2; ++j) {
        diamond_id_t c_id = d.child_id(i,j);
        // remove a child only if both its patches are no more needed from the graph
        // (which means diamond has been removed from graph by refine procedure)
	// grid_diamond_t key(c_id, c_id, c_id, c_id);
	//	if (diamond_map_by_level_[level+1]->find(key) == diamond_map_by_level_[level+1]->end()) {
	if (!has(level+1,c_id)) {
          refinement_heap_.erase(c_id);
        }
      }
    }
    
    // remove d from coarsening heap
    diamond_id_t id = d.id();
    coarsening_heap_.erase(id);
    
    // - add id to refinement_heap_
    //  add_to_refinement_heap(d, refinement_heap_);
    assert(!refinement_heap_.has(id));
    refinement_heap_.push_new(std::make_pair(get_priority_diamond(level, it), id));

    // if id is not a root
    // - add id->parents to coarsening_heap_ only if it's valid and all its children are leafs
    if (level > 0) {
      for(int p = 0; p < 2; ++p) {
        diamond_id_t p_id = d.parent_id(p);
        grid_diamond_t key_parent = grid_diamond_t(p_id, p_id, p_id, p_id);
        grid_diamond_map_const_iterator_t parent_it = diamond_map_by_level_[level-1]->find(key_parent);
        if(parent_it != diamond_map_by_level_[level-1]->end()) {
          if (check_all_children_are_leafs(level-1, parent_it)) {
            //            add_to_coarsening_heap(parent, coarsening_heap_);
            assert(!coarsening_heap_.has(p_id));
            coarsening_heap_.push_new(std::make_pair(get_priority_diamond(level-1, parent_it), p_id));
          }
        }
      }
    }
  }

  void grid_diamond_graph_incore::refine_update_heap(std::size_t level,
                                                     const grid_diamond_map_const_iterator_t& it) {
    const grid_diamond_t& d = it->first;

    // - remove id from refinement_heap_
    diamond_id_t id = d.id();
    refinement_heap_.erase(id);

    // - add id children (if leafs) to refinement_heap_
    if (level+1<level_count()) {
      for(int i = 0; i < 2; ++i) {
        for(int j = 0; j < 2; ++j) {
          diamond_id_t c_id = d.child_id(i,j);
          grid_diamond_t key_child = grid_diamond_t(c_id, c_id, c_id, c_id);
          grid_diamond_map_const_iterator_t it_child = diamond_map_by_level_[level+1]->find(key_child);

          if (it_child !=  diamond_map_by_level_[level+1]->end() &&
              it_child->first.is_valid() &&
              it_child->second.is_leaf()) {
            refinement_heap_.push(std::make_pair(get_priority_diamond(level+1, it_child), c_id));
          } 
        }
      }
    }
    
    // remove d parents from coarsening_heap_
    diamond_id_t p_id = d.parent_id(0);
    if (coarsening_heap_.has(p_id)) {coarsening_heap_.erase(p_id);}
    p_id = d.parent_id(1);
    if (coarsening_heap_.has(p_id)) {coarsening_heap_.erase(p_id);}     //    if (is_valid(p_id)) {coarsening_heap_.erase(p_id);}
    
    // - add id to coarsening_heap_ only if all its children are leafs
    assert(check_all_children_are_leafs(level, it));
    assert(!coarsening_heap_.has(id));
    coarsening_heap_.push_new(std::make_pair(get_priority_diamond(level, it), id));
  }

  bool grid_diamond_graph_incore::check_all_children_are_leafs(std::size_t level,
                                                               const grid_diamond_map_const_iterator_t& it) const {
    bool all_leafs = true;
    if (level+1<level_count()) {
      const grid_diamond_t& d = it->first;
      for(int i = 0; i < 2 && all_leafs; ++i) {
        for(int j = 0; j < 2 && all_leafs; ++j) {
          diamond_id_t c_id = d.child_id(i,j);
          grid_diamond_t key_child = grid_diamond_t(c_id, c_id, c_id, c_id);
          grid_diamond_map_const_iterator_t it_child = diamond_map_by_level_[level+1]->find(key_child);
          if (it_child !=  diamond_map_by_level_[level+1]->end()) {
            if (!(it_child->second.is_leaf())) {
              all_leafs = false;
            }
          }
        }
      }
    }
    return all_leafs;
  }

  double grid_diamond_graph_incore::patch_edge_length(const grid_diamond_t& d) const {
    diamond_id_t id = d.id();
    const coordinate_transform* geo_xform = uvh_xyz_transform();
    point3d_t p_id = geo_xform->xyz_from_grid(id, 0);
    point3d_t p_diagonal = geo_xform->xyz_from_grid(d.corner(0), 0);
    if (geo_xform->is_planar()) {
      double el = 1.41 * (p_id - p_diagonal).two_norm();
      return el;
    } else {
      // planar approx
      uint32_t id_other = d.is_valid_fragment(0) ? 1 : 3;
      point3d_t p_other  = geo_xform->xyz_from_grid(d.corner(id_other), 0);
      
      normald_t n_id     = normald_t(p_id[0], p_id[1], p_id[2]).ok_normalized();
      normald_t n_diag   = normald_t(p_diagonal[0], p_diagonal[1], p_diagonal[2]).ok_normalized();
      normald_t n_other  = normald_t(p_other[0], p_other[1], p_other[2]).ok_normalized();
      
      // planar approx
      double radius = dynamic_cast<const spherical_coordinate_transform*>(geo_xform)->radius();
      return float(radius * 1.41 * std::max((n_id-n_diag).two_norm(),
                                            (n_id-n_other).two_norm()));
    }
  }


  std::pair<float, float> grid_diamond_graph_incore::estimated_elevation_range() const {
    std::pair<float,float> result = std::make_pair(1e30f,-1e30f);
    uint32_t patch_dim = height_patch_dim();
    int size = (patch_dim+1)*(patch_dim+2)/2;
    for (grid_diamond_map_iterator_t it = diamond_map_by_level_[0]->begin();
         it != diamond_map_by_level_[0]->end();
         ++it) {
      const grid_diamond_t& d = it->first;
      
      for(int i = 0; i < 2; ++i) {
        if (it->second.has_fragment(i)) {
          const diamond_vertices* dv = geometry_cache_[std::make_pair(d.id(), i)];
          assert(dv != 0);
          if (dv ==0) {
            SL_TRACE_OUT(-1) << "accessed null fragment in geo cache for root" << std::endl;
          }
          for(int j = 0; j < size; ++j) {
            float h = dv->values()[j];
            result.first  = std::min(result.first, h);
            result.second = std::max(result.second, h);
          }
        }
      } 
    }
    if (result.first>result.second) result = std::make_pair(0.0f,0.0f);
    result.first *= height_scale_factor();
    result.second *= height_scale_factor();
    //    std::cerr << "#### DIAMOND GRAPH ELEVATION RANGE: " << result.first << " ... " << result.second << std::endl;
    
    return result;
  }
    
  void grid_diamond_graph_incore::init_heaps() {
    refinement_heap_.clear();
    coarsening_heap_.clear();

    // add all leafs to refinement_heap_
    SL_TRACE_OUT(1) << "geo init heaps:" << std::endl;
    for(std::size_t l = 0; l < level_count(); ++l) {
      for(grid_diamond_map_const_iterator_t mi = diamond_map_by_level_[l]->begin();
          mi != diamond_map_by_level_[l]->end();
          ++mi) {
        if (mi->second.is_leaf()) {
          diamond_id_t id = mi->first.id();
	  //	  SL_TRACE_OUT(1) << "geo refinement_heap_ inserting " << id << " at level " << l << std::endl;
          refinement_heap_.push_new(std::make_pair(get_priority_diamond(l, mi), id));
            
          // if id is not a root
          if (l>0) {
            // add its parent to coarsening heap
            for(int i = 0; i < 2; ++i) {
              const diamond_id_t p_id = mi->first.parent_id(i);
              grid_diamond_t key(p_id, p_id, p_id, p_id);
              grid_diamond_map_const_iterator_t p_it = diamond_map_by_level_[l-1]->find(key);
              if (p_it != diamond_map_by_level_[l-1]->end()) {
                if (check_all_children_are_leafs(l-1, p_it)) {
                  coarsening_heap_.push_new(std::make_pair(get_priority_diamond(l-1, p_it), p_id));
                }
              }
            }
          }
        }
      }
    }
    SL_TRACE_OUT(1) << "geo refine sz: " << refinement_heap_.size() << ", coarsen sz: " << coarsening_heap_.size() << std::endl;
  }

  void grid_diamond_graph_incore::extract_cut(float threshold,
                                              const projective_map_t& P,
                                              const rigid_body_map_t& V,
                                              int& incremental_updates_count) {
    if (texture_tile_width_ != 0) {
      threshold = threshold * texture_tile_width_ / height_patch_dim();
    }


    static int frame_counter =0;
    ++frame_counter;
      
    // Reset
    incremental_updates_count = 0;
    decoded_diamond_count_ = 0;
    std::size_t coarsen_count = 0;
    
    // FIXME Params or member vars
    const sl::uint64_t max_refine_time = 25; // FIXME !!!!
    sl::real_time_clock timeout_clock;
    timeout_clock.restart();
    projective_map_t old_pv = camera_pv_;
    camera_pv_ = P * V;
    sl::quaternion<double> q;
    vector3d_t v;
    (~V).factorize_to( q, v );
    view_point_ = point3d_t(v[0], v[1], v[2]);

    bool camera_moved = old_pv != camera_pv_;
    if ((!camera_moved) && data_missing_fraction_ == 0 && threshold == previous_threshold_) {
      return;
    }
    previous_threshold_ = threshold;

    // Heap 
    if (camera_moved) {
      recompute_heap_priority();
    }
    //    prefetch_refine_data(threshold);

    // Refine
    std::size_t refine_count  = 0;
#if 0
    std::size_t diamond_count_before_refine = diamond_map_.size();
#endif
    bool refine_data_available = true;
    bool timeout = false;
#if 0
    SL_TRACE_OUT(1) << "ref heap empty " << refinement_heap_.empty() 
              << ", ref heap top prio " << refinement_heap_.top().first.priority()
              << ", thre " << threshold 
              << ", dec dm cnt " << decoded_diamond_count_
              << ", dec dm bgt " << decoded_diamond_budget_ << std::endl;
#endif
    while (refine_data_available
           &&
           (!refinement_heap_.empty() && refinement_heap_.top().first.priority() > threshold)
           &&
           (!timeout) 
           &&
           decoded_diamond_count_ < decoded_diamond_budget_) {
      std::size_t previous_decoded_diamond_count = decoded_diamond_count_;
      time_critical_refine(refinement_heap_.top().first.level(),
                           refinement_heap_.top().second,
                           timeout_clock, 
                           max_refine_time);
#if 0
      SL_TRACE_OUT(1) << "ref heap empty " << refinement_heap_.empty() 
		<< ", ref heap top prio " << refinement_heap_.top().first.priority()
		<< ", thre " << threshold 
		<< ", dec dm cnt " << decoded_diamond_count_
		<< ", dec dm bgt " << decoded_diamond_budget_ 
		<< ", ref count  " << refine_count << std::endl;
#endif
      ++refine_count;

      refine_data_available = (decoded_diamond_count_ != previous_decoded_diamond_count);
      timeout = (sl::uint64_t(timeout_clock.elapsed().as_milliseconds()) > max_refine_time);
    }

#if 0
    if (!refinement_heap_.empty() && refinement_heap_.top().first.priority() > threshold) {
      // HEAP
      SL_TRACE_OUT(1) << "NEEDS REFINE: err = " << refinement_heap_.top().first.priority() << " vs. " << threshold << std::endl;
      SL_TRACE_OUT(1) << "  decoded_cnt = " << decoded_diamond_count_ << " vs. " << decoded_diamond_budget_ << std::endl;
      SL_TRACE_OUT(1) << "  timeout = " << timeout << std::endl;
    }
#endif
      
    // Coarsen
#if 0
    std::size_t diamond_count_after_refine = diamond_map_.size();
    std::size_t diamond_count_before_coarsen = diamond_map_.size();
#endif
    if (frame_counter%10 == 1) { // FIXME!!
      while ((!coarsening_heap_.empty() && coarsening_heap_.top().first.priority() < threshold)) {
        coarsen(coarsening_heap_.top().first.level(), coarsening_heap_.top().second);
        ++coarsen_count;
      }
    }
#if 0
    std::size_t diamond_count_after_coarsen = diamond_map_.size();
#endif
    
#if 0
    if (timeout_clock.elapsed().as_milliseconds() > max_refine_time) {
      SL_TRACE_OUT(1) 
        << "TIMEOUT: " 
        << (float)(timeout_clock.elapsed().as_milliseconds())
        << " > " << max_refine_time 
        << " #deleted: " << diamond_count_before_coarsen - diamond_count_after_coarsen 
        << " #created: " << diamond_count_after_refine - diamond_count_before_refine
        << std::endl;
      SL_TRACE_OUT(1) << "  Coarsen count: " << coarsen_count << " - refine_count: " << refine_count << std::endl;
    }
#endif
    
    
    // compute stream percent
    uint32_t patch_to_refine_count = 0;
    for(refinement_heap_t::const_data_key_iterator_t it = refinement_heap_.begin();
        (it != refinement_heap_.end()) && (it->first.priority() > threshold); ++it) {
      ++patch_to_refine_count;
    }
    data_missing_fraction_ = (float)patch_to_refine_count / (float)refinement_heap_.size();

    assert(check_heap_consistency());
     
    //    cut_updated = (decoded_diamond_count_>0 || coarsen_count>0);
    incremental_updates_count = decoded_diamond_count_ + coarsen_count;
  }
  
  void grid_diamond_graph_incore::set_decoded_diamond_budget(uint32_t x) {
    decoded_diamond_budget_ = x;
  }

  void grid_diamond_graph_incore::prefetch_refine_data(float threshold) {
    for(refinement_heap_t::const_data_key_iterator_t it = refinement_heap_.begin();
        (it != refinement_heap_.end()) && (it->first.priority() > threshold); ++it) {
      prefetch_refine_data(it->first.level(), it->second);
    }
  }

  // recursive: pass by value
  void grid_diamond_graph_incore::prefetch_refine_data(std::size_t level, diamond_id_t id) {
    grid_diamond_t key(id, id, id, id);
    grid_diamond_map_const_iterator_t it = diamond_map_by_level_[level]->find(key);
    assert(it != diamond_map_by_level_[level]->end());
    
    const grid_diamond_t d = it->first;
    grid_diamond_state_t ds = it->second;
    if (ds.is_procedural()) {
      return;
    }        

    // Check if parents need refinement
    if (level>0) {
      // after call refine check if d has requested patches or if returned beacause data was not available
      if (!ds.has_fragment(0) && d.is_valid_fragment(0)) {
        prefetch_refine_data(level-1,d.parent_id(0));
      }
      if (!ds.has_fragment(1) && d.is_valid_fragment(1)) {
        prefetch_refine_data(level-1,d.parent_id(1));
      }
    }

    bool all_in_cache = true;
    for(int i = 0; i < 2; ++i) {
      if (ds.has_fragment(i)) {
        for(int j = 0; j < 2; ++j) {
          grid_diamond_t d_child = child_diamond(level, d, i, j);
	  //	  std::cerr << "access fragment_id (3): " << i << " " << j <<  ", level " << level << std::endl;
          std::size_t patch_id = d_child.fragment_id_deriving_from_parent(id);
	  all_in_cache &= geometry_cache_[std::make_pair(d_child.id(), patch_id)] != 0;
        }
      }
    }
    
    if (!all_in_cache) {
      geometry_fetcher_->prefetch(id, aabox(d));
    }
  }

  void grid_diamond_graph_incore::recompute_heap_priority() {
    refinement_heap_t tmp_ref;
    for(refinement_heap_t::const_data_key_iterator_t rh_it = refinement_heap_.begin();
        rh_it != refinement_heap_.end(); ++rh_it) {
      std::size_t level = rh_it->first.level();
      diamond_id_t id = rh_it->second;
      grid_diamond_t key(id, id, id, id);
      grid_diamond_map_const_iterator_t it = diamond_map_by_level_[level]->find(key);
      assert(it != diamond_map_by_level_[level]->end());
      tmp_ref.push_new(std::make_pair(get_priority_diamond(level, it), id));
    }
      
    refinement_heap_.clear();
    std::swap(refinement_heap_,tmp_ref);

    coarsening_heap_t tmp_coarse;
    for(coarsening_heap_t::const_data_key_iterator_t ch_it = coarsening_heap_.begin();
        ch_it != coarsening_heap_.end(); ++ch_it) {
      diamond_id_t id = ch_it->second;
      std::size_t level = ch_it->first.level();
      grid_diamond_t key(id, id, id, id);
      grid_diamond_map_const_iterator_t it = diamond_map_by_level_[level]->find(key);
      if (it != diamond_map_by_level_[level]->end()) {        
        tmp_coarse.push_new(std::make_pair(get_priority_diamond(level, it), id));
      }
    }
      
    coarsening_heap_.clear();
    std::swap(coarsening_heap_,tmp_coarse);
  }

  grid_diamond_render_state::bounding_volume_t diamond_bounding_volume(const vector3d_t& center_normal,
                                                                       const point3d_t&  center_hi, 
                                                                       const point3d_t&  p0_lo,
                                                                       const point3d_t&  p1_lo,
                                                                       const point3d_t&  p2_lo,
                                                                       const point3d_t&  p3_lo) {
    // Construct basis
    vector3d_t z = center_normal;
    vector3d_t u = (p1_lo-p0_lo).ok_normalized();
    vector3d_t x = (u - (z.dot(u)*z)).ok_normalized();
    sl::rigid_body_map3d from_box_space_map = sl::linear_map_factory3d::basis_from(0, x,
										   2, z);

    // Construct a local AAB
    point3d_t c_local = inverse_transformation(from_box_space_map,center_hi);
    point3d_t p_local[4] = {
      inverse_transformation(from_box_space_map,p0_lo),
      inverse_transformation(from_box_space_map,p1_lo),
      inverse_transformation(from_box_space_map,p2_lo),
      inverse_transformation(from_box_space_map,p3_lo),
    };
      
    point3d_t p_local_inf = c_local;
    point3d_t p_local_sup = c_local;
    for (std::size_t i=0; i<4; ++i) {
      for (std::size_t d=0; d<3; ++d) {
	const double p_local_d = p_local[i][d];
	if (p_local_d > p_local_sup[d]) { p_local_sup[d] = p_local_d; }
	if (p_local_d < p_local_inf[d]) { p_local_inf[d] = p_local_d; }
      }
    }
        
    // Compute hsl and new center
    vector3d_t local_hsl = 0.5 * (p_local_sup-p_local_inf);
    point3d_t  p_local_center = p_local_inf + local_hsl;

    // Recenter
    point3d_t  p_center = transformation(from_box_space_map, p_local_center);
    sl::matrix4d m = from_box_space_map.as_matrix();
    for (size_t i=0; i<3; ++i) {
      m(i,3) = p_center[i];
    }

    // Return volume
    //    SL_TRACE_OUT(1) << "bounding_volume_t hsl " << local_hsl[0] << " " << local_hsl[1] << " " << local_hsl[2] << std::endl;
    return grid_diamond_graph_incore::bounding_volume_t(sl::rigid_body_map3d(m), local_hsl);
  }

  void grid_diamond_graph_incore::set_bounding_volume(grid_diamond_map_iterator_t& it) {
    const grid_diamond_t& d = it->first;
    const diamond_id_t id = d.id();
    grid_diamond_state_t& ds = it->second;
    // get max h stored in d
    const uint32_t N = patch_vertices_count();
    const double dh = height_scale_factor();
    double max_h = -1e30;
    double min_h =  1e30;
    for(int i = 0; i < 2; ++i) {
      if (ds.has_fragment(i)) {
        const diamond_vertices* dv = ds.reference_counted_patch_data(i); // geometry_cache_[std::make_pair(id, i)];
        assert(dv);
        const std::vector<int32_t >& heights = dv->values();
        for(uint32_t j = 0; j < N; ++j) {
          double h = dh * heights[j];
          if (max_h < h) {max_h = h;}
          if (min_h > h) {min_h = h;}
        }
      }
    }
    if (min_h>max_h) { min_h = max_h = 0; }

    const coordinate_transform* geo_xform = uvh_xyz_transform();
    vector3d_t center_normal = geo_xform->up_from_grid(id);
    point3d_t  center_hi     = geo_xform->xyz_from_grid(id, max_h);
    point3d_t  p0_lo         = geo_xform->xyz_from_grid(d.corner(0), min_h);
    point3d_t  p1_lo         = geo_xform->xyz_from_grid((d.is_valid_fragment(0) ? d.corner(1) : d.corner(3)), min_h);
    point3d_t  p2_lo         = geo_xform->xyz_from_grid(d.corner(2), min_h);
    point3d_t  p3_lo         = geo_xform->xyz_from_grid((d.is_valid_fragment(1) ? d.corner(3) : d.corner(1)), min_h);

    ds.set_bounding_volume(diamond_bounding_volume(center_normal, 
                                                   center_hi, 
                                                   p0_lo, 
                                                   p1_lo,
                                                   p2_lo,
                                                   p3_lo));
  }

  bool grid_diamond_graph_incore::check_heap_consistency() const {
    bool refinement_good = true;
    for(refinement_heap_t::const_data_key_iterator_t rh_it = refinement_heap_.begin();
        rh_it != refinement_heap_.end(); 
	++rh_it) {
      const diamond_id_t& id = rh_it->second;
      std::size_t level = rh_it->first.level();
      if (level >= level_count()) {
	refinement_good = false;
	SL_TRACE_OUT(-1) << "check_heap_consistency: refinement: level " << level << " level count " << level_count() << std::endl; 	
      } else {
	grid_diamond_t key(id, id, id, id);
	grid_diamond_map_const_iterator_t it = diamond_map_by_level_[level]->find(key);
	if (it ==  diamond_map_by_level_[level]->end()) {
	  refinement_good = false;
	  SL_TRACE_OUT(-1) << "check_heap_consistency: refinement: missing " << id << " from cut at level " << level << std::endl; 
	} else {
	  if (!it->second.is_leaf()) {
	    refinement_good = false;
	    SL_TRACE_OUT(-1) << "check_heap_consistency: refinement: " << id << " not a leaf" << std::endl;
	  } else {
	    for(int i = 0; i < 2; ++i) {
	      if (it->second.has_fragment(i)) {
		//		if (geometry_cache_[std::make_pair(id, i)] == 0) {
		if (it->second.reference_counted_patch_data(i) == 0) {
		  refinement_good = false;
		  SL_TRACE_OUT(-1) << "check_heap_consistency: refinement: " << id << " patch " << i << ": void geometry ptr" << std::endl;
		} else if (it->second.reference_counted_patch_data(i)->use_count() == 0) {
		  refinement_good = false;
		  SL_TRACE_OUT(-1) << "check_heap_consistency: refinement: " << id << " patch " << i << ": use count 0" << std::endl;
		}
	      }
	    }
	  }
	}
      }
    }

    bool coarsen_good = true;
    for(coarsening_heap_t::const_data_key_iterator_t ch_it = coarsening_heap_.begin();
        ch_it != coarsening_heap_.end(); 
	++ch_it) {
      const diamond_id_t& id = ch_it->second;
      std::size_t level = ch_it->first.level();
      if (level >= level_count()) {
	coarsen_good = false;
	SL_TRACE_OUT(-1) << "check_heap_consistency: coarsen: level " << level << " level count " << level_count() << std::endl; 	
      } else {
	grid_diamond_t key(id, id, id, id);
	grid_diamond_map_const_iterator_t it = diamond_map_by_level_[level]->find(key);
	if (it ==  diamond_map_by_level_[level]->end()) {
	  coarsen_good = false;
	  SL_TRACE_OUT(-1) << "check_heap_consistency: coarsen: missing " << id << " from cut at level " << level << std::endl; 
	} else {
	  if (it->second.is_leaf()) {
	    coarsen_good = false;
	    SL_TRACE_OUT(-1) << "check_heap_consistency: coarsen: is leaf " << id << " at level " << level << std::endl; 
	  } else {
	    if (!check_all_children_are_leafs(level, it)) {
	      coarsen_good = false;
	      SL_TRACE_OUT(-1) << "check_heap_consistency: coarsen: not all LEAF children  of " << id << " at level " << level << std::endl; 
	    }
	  }
	}
      }     
    }

    if (!coarsen_good) {
      SL_TRACE_OUT(-1) << "coarsen check failed, coarsening_heap_size " << coarsening_heap_.size() 
		<< " refinement_heap_size " << refinement_heap_.size() << std::endl;
    }

    return refinement_good;
  }

  void grid_diamond_graph_incore::print() const {
    for(std::size_t l = 0; l < level_count(); ++l) {
      std::cerr << "level " << l << " leafs:" << std::endl;
      for(grid_diamond_map_const_iterator_t li = level_begin(l);
	  li != level_end(l);
	  ++li) {
	if (li->second.is_leaf()) {
	  std::cerr << "  " << li->first.id() << " ";
	  if (li->second.has_fragment(0)) {
	    std::cerr << "has fragment 0, ";
	    std::cerr << "c1 " << li->first.corner(1) << ", ";
	  }
	  if (li->second.has_fragment(1)) {
	    std::cerr << "has fragment 1, ";
	    std::cerr << "c3 " << li->first.corner(3);
	  }
	  std::cerr << std::endl;
	}
      }
    }
  }

} // namespace cbdam















#if 0

    // ray tracing procedures
  public:
    std::pair<bool, double> current_graph_elevation_from_absolute(double x, double y) const;
    std::pair<bool, double> current_graph_elevation_from_parametric(double u, double v) const;
    std::pair<bool, double> current_graph_intersection_from_absolute(const point3d_t& origin, const point3d_t& extremity) const;
    std::pair<bool, double> current_graph_intersection_from_parametric(const point3d_t& param_origin, const point3d_t& extremity) const;

  protected:
    point3d_t absolute_from_parametric(const point3d_t& p) const;

    std::pair<bool, double> current_graph_elevation(std::size_t level, grid_diamond_map_const_iterator_t it,
                                                    uint32_t patch_id, const point3d_t& x) const;

    point2d_t triangle_barycentric_coords_of_point(const point3d_t& p0, const point3d_t& p1, 
						   const point3d_t& p2, const point3d_t& x) const;

    std::pair<bool, double> patch_ray_intersection(const grid_diamond_map_const_iterator_t& it, int patch_id,
                                                   ray r, normald_t *normal = 0) const;

    class bsp_stack_element {
    protected:
      const diamond_id_t id_;
      uint32_t           level_;
      uint32_t		 patch_id_;
      ray		 r_;

    public:
      bsp_stack_element(const diamond_id_t& id, uint32_t level, uint32_t patch_id, const ray& r) :
          id_(id), level_(level), patch_id_(patch_id), r_(r) {
	
      }
      
      CBDAM_R_ACCESSOR(diamond_id_t, id);
      CBDAM_R_ACCESSOR(uint32_t, level);
      CBDAM_R_ACCESSOR(uint32_t, patch_id);
      CBDAM_R_ACCESSOR(ray, r);
    };						     

  std::pair<bool, double> grid_diamond_graph_incore::current_graph_elevation_from_absolute(double x, double y) const {
    if (!is_planar()) {
      SL_TRACE_OUT(1) << "graph elevation disabled for spherical terrains\n";
      return std::make_pair(false, 0);
    } else {
      // check first patch 0
      point3d_t p(x, y, 0);
      std::pair<bool, double> r0 = current_graph_elevation(0, diamond_map_by_level_[0]->begin(), 0, p);
      if (r0.first) {
        return r0;
      } else {
        // if fails check patch 1
        return current_graph_elevation(0, diamond_map_by_level_[0]->begin(), 1, p);
      }
    }
  }
    
  std::pair<bool, double> grid_diamond_graph_incore::current_graph_elevation_from_parametric(double u, double v) const {
    if (!is_planar()) {
      SL_TRACE_OUT(1) << "graph elevation disabled for spherical terrains\n";
      return std::make_pair(false, 0);
    } else {
      point3d_t p = absolute_from_parametric(point3d_t(u, v, 0));
      return current_graph_elevation_from_absolute(p[0], p[1]);
    }
  }
    
  static void split_ray_in(ray& r_front, ray& r_back, bool& success,const ray& r, const sl::plane3d& split_plane) {
    // FIXME: DO NOT SPLIT ANYTHING
#if 1
    r_front = r;
    r_back = r;
    success = true;
    return;
#else
    point3_t p_far = r.point_at(r.t_far());
    float t = split_plane.intersection(r.origin(), p_far);
    t *= (p_far - r.origin()).two_norm();

    if (t <= r.t_near() || t >= r.t_far()) {
      // no ray-plane intersection: 
      r_front = r;
      success = false;
    } else {
      r_front = r; r_front.t_far() = t * 1.05;
      r_back = r;  r_back.t_near() = t * 0.95;
      success = true;
      //      SL_TRACE_OUT(1) << "r_front :" << r_front.t_near() << ", " << r_front.t_far() << std::endl;
      //      SL_TRACE_OUT(1) << "r_back :" << r_back.t_near() << ", " << r_back.t_far() << std::endl;
    }
#endif
  }

  std::pair<bool, double> grid_diamond_graph_incore::current_graph_intersection_from_absolute(const point3d_t& origin, const point3d_t& extremity) const {
    int checked_patch_count = 0;
    int traversed_patch_count = 0;

    grid_diamond_map_const_iterator_t r_it = diamond_map_by_level_[0]->begin();
    const grid_diamond_t& root = r_it->first;
    diamond_id_t root_id = root.id();
    
    double l = (extremity - origin).two_norm();
    if (l == 0) {l = 1;}
    ray r(origin, (extremity - origin) / l, 0, l);
    std::stack<bsp_stack_element> bsp_stack;

    // diamond sketch. numbers mean: corners indices and patch indices
    // 1---2
    // |0/1|
    // 0---3

    // INIT STACK WITH 2 ROOT PATCHES
    const coordinate_transform* geo_xform = uvh_xyz_transform();
    normald_t plane_normal = as_dual(geo_xform->xyz_from_grid(root.corner(3), 0.0) - 
				     geo_xform->xyz_from_grid(root.corner(1), 0.0));
    point3d_t p = geo_xform->xyz_from_grid(root.corner(0), 0.0);
    sl::plane3d split_plane(plane_normal, p);

    // select front and back root patches
    // positive distance: patch 1 is nearest to the origin: see diamond sketch some lines before
    int front = 0;
    int back = 1;
    if (split_plane.signed_distance(r.point_at(r.t_near())) > 0) {
      front = 1;
      back = 0;
    } 

    // split ray
    bool split_success;
    ray r_front = r;
    ray r_back = r;
    split_ray_in(r_front, r_back, split_success, r, split_plane);
    if (split_success) {
      bsp_stack.push(bsp_stack_element(root_id, 0, back, r_back));
      bsp_stack.push(bsp_stack_element(root_id, 0, front, r_front));
    } else {
      bsp_stack.push(bsp_stack_element(root_id, 0, front, r));
    }

    // ITERATE OVER THE STACK UNTIL EMPTY OR CLOSEST INTERSECTION HAVE BEEN FOUND
    std::pair<bool, double> hit = std::make_pair(false, -1);
    while(!bsp_stack.empty()) {
      ++traversed_patch_count;

      // pop first element
      bsp_stack_element bse = bsp_stack.top();
      bsp_stack.pop();
      //      const diamond* current_diamond = get_diamond(bse.id());
      const diamond_id_t& current_id = bse.id();
      const grid_diamond_t key(current_id, current_id, current_id, current_id);
      const grid_diamond_map_const_iterator_t current_it = diamond_map_by_level_[bse.level()]->find(key);
      assert(current_it != diamond_map_by_level_[bse.level()]->end());
      
      const grid_diamond_t& current_diamond = current_it->first;
      const grid_diamond_state_t& current_diamond_state = current_it->second;
      ray current_ray = bse.r();	
      int current_patch_id = bse.patch_id();
      double tmin = current_ray.t_near();
      double tmax = current_ray.t_far();
      bool off = false;
        
      // check bbox intersection
      //      SL_TRACE_OUT(1) << "\tcurrent ray: tmin " << tmin << ", tmax " << tmax << " ";
      current_diamond_state.bounding_volume().clip(current_ray.origin(), current_ray.direction(), off, tmin, tmax);
        
      if (off) {
        //	SL_TRACE_OUT(1) << "OFF" << std::endl;
      } else {
        //	SL_TRACE_OUT(1) << "IN" << std::endl;
        if (is_leaf(bse.id())) {
          // intersect with all patch triangles
          ++checked_patch_count;
          hit = patch_ray_intersection(current_it, current_patch_id, current_ray);
          if (hit.first) {
            break;
          } else {
            //	    SL_TRACE_OUT(1) << "missed intersection\n";
          }
        } else {
          // recurse children
          // p0--p1
          //  |\1/
          //  |0/
          // p2/
          
          uint32_t i0 = current_patch_id == 0 ? 1 : 3;
          uint32_t i1 = current_patch_id == 0 ? 2 : 0;
          uint32_t i2 = current_patch_id == 0 ? 0 : 2;
            
          point3d_t p0 = geo_xform->xyz_from_grid(current_diamond.corner(i0), 0.0);
          point3d_t p1 = geo_xform->xyz_from_grid(current_diamond.corner(i1), 0.0);
          point3d_t p2 = geo_xform->xyz_from_grid(current_diamond.corner(i2), 0.0);
            
          // order patches
          // plane normal is always from p2 to p1. If origin is below plane: it is on the side of the patch 0
          // otherwise is on the side of patch 1
          plane_normal = as_dual(p1 - p2).ok_normalized();
          split_plane = sl::plane3d(plane_normal, p0);
          if (split_plane.signed_distance(current_ray.origin()) > 0) {
            front = 1;
            back = 0;
          } else {
            front = 0;
            back = 1;
          }
            
          // split ray and push onto the stack
          if (bse.level()+1<level_count()) {
            split_ray_in(r_front, r_back, split_success, current_ray, split_plane);
            if (split_success) {
              bsp_stack.push(bsp_stack_element(current_diamond.child_id(current_patch_id, back), bse.level() + 1, back, r_back));
              bsp_stack.push(bsp_stack_element(current_diamond.child_id(current_patch_id, front), bse.level() + 1, front, r_front));
            } else {
              bsp_stack.push(bsp_stack_element(current_diamond.child_id(current_patch_id, front), bse.level() + 1, front, current_ray));
            }
          }
        }
      }
    }

    //    SL_TRACE_OUT(1) << "traversed_patch_count " << traversed_patch_count << ", checked_patch_count " << checked_patch_count << std::endl;
    //    SL_TRACE_OUT(1) << "hit exist " << hit.first << ", at " << hit.second << std::endl;
    return hit;
  }
    
  std::pair<bool, double> grid_diamond_graph_incore::current_graph_intersection_from_parametric(const point3d_t& param_origin,
                                                                                                const point3d_t& param_extremity) const {
    const planar_coordinate_transform* geo_xform = dynamic_cast<const planar_coordinate_transform*>(uvh_xyz_transform());
    if (geo_xform == 0) {
      SL_TRACE_OUT(1) << "current_graph_intersection_from_parametric disabled for spherical terrains" << std::endl;
      return std::make_pair(false, 0.0);
    } else {
      point3d_t origin = absolute_from_parametric(param_origin);
      point3d_t extremity = absolute_from_parametric(param_extremity);
        
      return current_graph_intersection_from_absolute(origin, extremity);
    }
  }

  point3d_t grid_diamond_graph_incore::absolute_from_parametric(const point3d_t& p) const {
    const planar_coordinate_transform* geo_xform = 
      dynamic_cast<const planar_coordinate_transform*>(uvh_xyz_transform());
    if (geo_xform == 0) {
      SL_TRACE_OUT(1) << "current_graph_intersection_from_parametric disabled for spherical terrains" << std::endl;
      return point3d_t(0,0,0);
    } else {
      planar_coordinate_transform::aabox_t b = geo_xform->bounding_rectangle();
      const vector2d_t& length = b.diagonal();
        
      return point3d_t(b[0][0] + p[0] * length[0],  
                       b[0][1] + p[1] * length[1],
                       p[2]); 
    }
  }

  std::pair<bool, double> grid_diamond_graph_incore::current_graph_elevation(std::size_t level, grid_diamond_map_const_iterator_t it,
                                                                             uint32_t patch_id, const point3d_t& x) const {
    std::pair<bool, double> result = std::make_pair(false, 0);

    const grid_diamond_t& d = it->first;
    const grid_diamond_state_t& ds = it->second;

    if (ds.has_fragment(patch_id)) {
      uint32_t i0 = patch_id == 0 ? 1 : 3;
      uint32_t i1 = patch_id == 0 ? 2 : 0;
      uint32_t i2 = patch_id == 0 ? 0 : 2;

      const coordinate_transform* geo_xform = uvh_xyz_transform();
      point3d_t p0 = geo_xform->xyz_from_grid(d.corner(i0), 0.0);
      point3d_t p1 = geo_xform->xyz_from_grid(d.corner(i1), 0.0);
      point3d_t p2 = geo_xform->xyz_from_grid(d.corner(i2), 0.0);
    
      // check if point is inside the triangle
      point2d_t uv = triangle_barycentric_coords_of_point(p0,p1,p2, x);
      if (uv[0] >= 0.0 && uv[1] >= 0.0 &&
          (uv[0] + uv[1] <= 1.0)) {
        // uv is inside patch
        if (ds.is_leaf()) {
          const diamond_vertices* dv = geometry_cache_[std::make_pair(d.id(), patch_id)];
          assert(dv);
          int patch_width = height_patch_dim();
          // get height at uv: get index of the closest point to uv
          int selected_x = int(patch_width * uv[0]);
          int selected_y = int(patch_width * uv[1]);
          int point_idx = selected_x;
          for(int y = 0; y < selected_y; ++y) {
            point_idx += (patch_width + 1 - y);
          }
          result = std::make_pair(true, height_scale_factor() * dv->values()[point_idx]);
        } else {
          // recurse child 0
          if (level+1 < level_count()) {
            for(std::size_t i = 0; i < 2 && !result.first; ++i) {
              //            if (ds.has_fragment(i)) {
              diamond_id_t c_id = d.child_id(patch_id, i);
              grid_diamond_t key(c_id, c_id, c_id, c_id);
              grid_diamond_map_const_iterator_t c_it = diamond_map_by_level_[level+1]->find(key);
              assert(c_it != diamond_map_by_level_[level+1]->end());
              result = current_graph_elevation(level+1, c_it, i, x);
            }
          }
        }
      }
    }
    return result;
  }

  point2d_t grid_diamond_graph_incore::triangle_barycentric_coords_of_point(const point3d_t& p0, const point3d_t& p1, 
                                                                            const point3d_t& p2, const point3d_t& x) const {
    vector3d_t d0 = x - p0;
    vector3d_t d1 = p1 - p0;
    vector3d_t d2 = p2 - p0;
    double det = d1.cross(d2)[2];
    double u = d0.cross(d2)[2] / det;
    double v = d1.cross(d0)[2] / det;
    return point2d_t(u, v);
  }

  std::pair<bool, double> grid_diamond_graph_incore::patch_ray_intersection(const grid_diamond_map_const_iterator_t& it,
                                                                            int patch_id, ray r, normald_t *normal) const {
    bool found = false;
    double t = -1;

    const grid_diamond_t& d = it->first;
    const grid_diamond_state_t& ds = it->second;

    if (ds.has_fragment(patch_id)) {
      const diamond_vertices* dv = geometry_cache_[std::make_pair(d.id(), patch_id)];
      if (dv != 0) {
        int w = height_patch_dim();
        const std::vector<diamond_vertices::point3_t >& patch_points = dv->gl_points();

	// FIXME!!!! MOVE TO DIAMOND_VERTICES
        uint32_t count = 0;
        for(int y = 0; y < w; ++y) {
          for(int x = 0; x < w - y; ++x) {
            uint32_t count_next_row = count + w + 1 - y;
            point3_t p00 = patch_points[count];
            point3_t p10 = patch_points[count+1];
            point3_t p01 = patch_points[count_next_row];

            // check upper triangle
            // *-*
            // |/
            // *
            //	r.closest_triangle_intersection(p00, p10, p01, hit, t, normal);
            bool ok = false;
            point3d_t pd00(p00[0],p00[1],p00[2]);
            point3d_t pd10(p10[0],p10[1],p10[2]);
            point3d_t pd01(p01[0],p01[1],p01[2]);
            r.closest_triangle_intersection(pd00, pd01, pd10, ok, t, normal);	    
            found |= ok;
            if (x < w - y - 1) {
              // check also the other triangle
              //   *
              //  /|
              // *-*
              point3_t p11 =patch_points[count_next_row + 1];
              point3d_t pd11(p11[0],p11[1],p11[2]);

              ok = false;
              //	  r.closest_triangle_intersection(p11, p01, p10, ok, t, normal);	    	  
              r.closest_triangle_intersection(pd11, pd10, pd01, ok, t, normal);	    	  
              found |= ok;
            }
            ++count;
          }
          ++count;
        }
      }
    }
    return std::make_pair(found, t);
  }

#endif
