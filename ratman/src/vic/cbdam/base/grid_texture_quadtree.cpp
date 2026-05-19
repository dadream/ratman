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
#include <vic/cbdam/base/grid_texture_quadtree.hpp>
#include <vic/img/gl_quadtree_image_processor.hpp>

namespace cbdam {

  uint64_t grid_texture_quadtree::global_time_stamp() {
   static sl::real_time_clock* global_clock_ = 0;
   if (global_clock_ == 0) {
     global_clock_ = new sl::real_time_clock;
     SL_TRACE_OUT(1) << "initialize global clock" << std::endl;
     global_clock_->restart();
    }    
    assert(global_clock_);
    
    return global_clock_->elapsed().as_milliseconds();
  }  

  grid_texture_quadtree::grid_texture_quadtree() {
    decoded_diamond_count_ = 0;
    data_missing_fraction_ = 0.0;
    is_open_ = false;
    refiner_ = 0;

    texture_cache_.set_capacity(64);
  }

  grid_texture_quadtree::~grid_texture_quadtree() {
    clear();
  }

  void grid_texture_quadtree::clear() {
    // dereference everything in the caches
    SL_TRACE_OUT(1) << "Start clear" << std::endl;

    // remove all levels
    while (diamond_map_by_level_.size() > 0) {
      SL_TRACE_OUT(1) << "Pop level " << diamond_map_by_level_.size() << std::endl;
      pop_level_map();
    }  
    SL_TRACE_OUT(1) << "End popping..." << std::endl;

    SL_TRACE_OUT(1) << "Minimize cache footprint" << std::endl;
    texture_cache_.minimize_footprint();
    
    SL_TRACE_OUT(1) << "End clear" << std::endl;

  }

  void grid_texture_quadtree::set_texture_refiner(const texture_refiner* ref) {
    refiner_ = ref;
  }

  void grid_texture_quadtree::set_cache_capacity(std::size_t cc) {
    texture_cache_.set_capacity(cc);
  }

  
  void grid_texture_quadtree::set_decoded_diamond_budget(uint32_t x) {
    decoded_diamond_budget_ = x;
  }
  
  void grid_texture_quadtree::open(texture_fetcher_t* texture_fetcher, 
				   const coordinate_transform* geo_xform, 
				   std::size_t first_level, 
				   std::size_t last_level) {
    SL_TRACE_OUT(1) << "open" << std::endl;
    assert(refiner_);
    assert(texture_fetcher);
    assert(geo_xform);
    if (texture_fetcher == 0) SL_TRACE_OUT(-1) << "missing texture fetcher" << std::endl;
    if (geo_xform == 0) SL_TRACE_OUT(-1) << "missing geo_xform" << std::endl;

    texture_fetcher_ = texture_fetcher;
    coordinate_transform_ = geo_xform;
    first_level_ = first_level;
    last_level_ = last_level;

    is_open_ = false;

    texture_fetcher_->connect();
    SL_TRACE_OUT(1) << "check connection" << std::endl;
    if (texture_fetcher_->is_connected()) {
      SL_TRACE_OUT(1) << "read roots" << std::endl;
      read_roots(30);
    }
  }

  void grid_texture_quadtree::read_roots(uint32_t timeout_s) {
    is_open_ = false;
    SL_TRACE_OUT(1) << "texture read_roots" << std::endl;

    std::size_t root_count = coordinate_transform_->root_count();
    if (root_count != 1 && root_count != 8) {
      SL_TRACE_OUT(1) << "texture quadtree wrong root count " << root_count << " can be 1 for planar, 8 for cylindrical" << std::endl;
      return;
    }

    // compile request lists
    std::vector<grid_diamond_t> roots;
    std::vector<key_t> root_lxy;
    std::vector<aabox2d_t> root_box;
    for(std::size_t i = 0; i < root_count; ++i) {
      if (root_count == 1) {
        roots.push_back(grid_diamond_t::canonical_root(i));
        root_lxy.push_back(key_t(0,0,0));
	root_box.push_back(aabox(roots[i]));
      } else {
        // root_count = 8
        roots.push_back(grid_diamond_t::cylindrical_canonical_root(i));
        root_lxy.push_back(key_t(0, i%4, 1-i/4));
	root_box.push_back(aabox(roots[i]));
      }
    }

    if (level_count() == 0) {
      push_level_map(); 
    }

    if (first_level_ == 0) {
      std::size_t built_root_count = 0;
      for(std::size_t i = 0; i < root_count; ++i) {
	SL_TRACE_OUT(1) << "Requesting root: " << i << std::endl;
	texture_fetcher_t::status_data_pair_t res = texture_fetcher_->synchronous_fetch(root_lxy[i], root_box[i], timeout_s);
	reference_counted_compressed_image_t* rc_root = 0;
	switch(res.first) {
	case texture_fetcher_t::DONE: {
	  SL_TRACE_OUT(1) << "texture root " << i << " got data" << std::endl;
	  compressed_rgba_image_t* gl_image = res.second;
	  rc_root = new reference_counted_compressed_image_t(&texture_cache_, gl_image, global_time_stamp());

	  if (level_count()==0) push_level_map();
	  (*diamond_map_by_level_[0])[roots[i]] = grid_texture_state_t(root_lxy[i], true, true, rc_root);

	  texture_cache_.insert(root_lxy[i], rc_root);
	  ++built_root_count;
	  //	  assert(rc_root->use_count() == 1);
	} break;
	case texture_fetcher_t::NULL_DATA: {
	  SL_TRACE_OUT(1) << "texture root " << i << " null data" << std::endl;
	  (*diamond_map_by_level_[0])[roots[i]] = grid_texture_state_t(root_lxy[i], true, true, 0);
	  SL_TRACE_OUT(1) << "texture root " << i << " null data inserted" << std::endl;
	}  break;
	case texture_fetcher_t::FAILED: {
	  SL_TRACE_OUT(-1) << "texture failed to read root " << i << std::endl;
	  is_open_ = false;
	}  break;
	case texture_fetcher_t::NOT_FOUND: {
	  SL_TRACE_OUT(-1) << "synchronous call returned NOT FOUND accessing root " << i << std::endl;
	  is_open_ = false;
	}  break;
	default: {
	  SL_TRACE_OUT(-1) << "synchronous call returned unknown code: " << res.first << " accessing root " << i << std::endl;
	  is_open_ = false;
	} break;
	}
	SL_TRACE_OUT(1) << "done root " << i << std::endl;
      }
      
      //      std::cerr << "grid_texture_quadtree::read_roots: read " << built_root_count << " roots" << std::endl;
      is_open_  = (built_root_count != 0);
    } else {
      SL_TRACE_OUT(1) << "texture read_roots: first level > 0" << std::endl;
      // first_level > 0: insert all void roots
      is_open_ = true;

      for(std::size_t i = 0; i < root_count; ++i) {
	//	(*diamond_map_by_level_[0])[roots[i]] = grid_texture_state_t(root_lxy[i], true, true, 0);
	//	std::cerr << "grid_texture_quadtree::read_roots(): insert root " << i << std::endl;
	(diamond_map_by_level_[0])->insert(std::make_pair(roots[i], grid_texture_state_t(root_lxy[i], true, true, 0)));
      }
    }
    //    std::cerr << "grid_texture_quadtree::read_roots(): done" << std::endl;
  }

  void grid_texture_quadtree::init_heaps() {
    refinement_heap_.clear();
    coarsening_heap_.clear();

    // add all leafs to refinement_heap_
    SL_TRACE_OUT(1) << "texture init heaps" << std::endl;
    for(std::size_t l = 0; l < level_count(); ++l) {
      for(grid_texture_diamond_map_const_iterator_t mi = diamond_map_by_level_[l]->begin();
          mi != diamond_map_by_level_[l]->end();
          ++mi) {
        if (mi->second.is_leaf()) {
          diamond_id_t id = mi->first.id();
	  //	  SL_TRACE_OUT(1) << "texture refinement_heap_ inserting " << id << " at level " << l << std::endl;
          refinement_heap_.push_new(std::make_pair(get_priority_diamond(l, mi), id));
            
          // if id is not a root
          if (l>0) {
            // add its parent to coarsening heap
            const diamond_id_t p_id = mi->first.quad_parent_id();
            grid_diamond_t key(p_id, p_id, p_id, p_id);
            grid_texture_diamond_map_const_iterator_t p_it = diamond_map_by_level_[l-1]->find(key);
            if (p_it != diamond_map_by_level_[l-1]->end()) {
              if (check_all_children_are_leafs(l-1, p_it)) {
                coarsening_heap_.push_new(std::make_pair(get_priority_diamond(l-1, p_it), p_id));
              }
            }
          }
        }
      }
    }
    SL_TRACE_OUT(1) << "texture refine sz: " << refinement_heap_.size() << ", coarsen sz: " << coarsening_heap_.size() << std::endl;
  }

  void grid_texture_quadtree::extract_cut(int& incremental_updates_count, bool recompute_hp, sl::uint64_t max_refine_time) {
    SL_TRACE_OUT(1) << "EXTRACT_CUT " << std::endl;
    double threshold = 0;
    static int frame_counter =0;
    ++frame_counter;
      
    // Reset
    incremental_updates_count = 0;
    decoded_diamond_count_ = 0;
    std::size_t coarsen_count = 0;
    
    sl::real_time_clock timeout_clock;
    timeout_clock.restart();

    if (recompute_hp) {
      recompute_heap_priority();
    }

    // Refine
    std::size_t refine_count  = 0;
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
      //      SL_TRACE_OUT(1) << "  tile error " << refinement_heap_.top().first.priority() << std::endl;
      refine(refinement_heap_.top().first.level(), refinement_heap_.top().second);
#if 1
      SL_TRACE_OUT(1) << "   ref heap empty " << refinement_heap_.empty() 
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
#if 0
      std::cerr << "coarsening_heap_.size() " << coarsening_heap_.size();
      if (coarsening_heap_.size() != 0) {
	std::cerr << ", top.priority " << coarsening_heap_.top().first.priority() << ", threshold " << threshold << std::endl;
      } else {
	std::cerr << std::endl;
      }
#endif
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

    //    cut_updated = (decoded_diamond_count_>0 || coarsen_count>0);
    incremental_updates_count = decoded_diamond_count_ + coarsen_count;

    //    assert(check_heap_consistency());
  }
  
  void grid_texture_quadtree::refine(std::size_t level, diamond_id_t id) {
    //    SL_TRACE_OUT(1) << "    refine level " << level << " diamond " << id << std::endl;

    assert(level < last_level_);
    grid_diamond_t key (id, id, id, id);
    grid_texture_diamond_map_iterator_t it = diamond_map_by_level_[level]->find(key);
    assert(it != diamond_map_by_level_[level]->end());
    
    const grid_diamond_t d = it->first;
    grid_texture_state_t& ds = it->second;
    assert(ds.is_leaf());

    if(!ds.exist_children()) {
      SL_TRACE_OUT(1) << "not exist children of " << id << std::endl;
      return;
    }

    if (level > last_level_) {
      SL_TRACE_OUT(1) << "not exist children of " << id << std::endl;
      return;
    }

    // create 4 diamond children, get pointer to their data in the cache if exist or from texture_fetcher_
    std::size_t root_count = coordinate_transform_->root_count();
    grid_diamond_t d_child[4];
    compressed_rgba_image_t* gl_image[4] = {0,0,0,0};
    reference_counted_compressed_image_t* rc_image[4] = {0,0,0,0};
    key_t level_xy_id[4];

    bool least_one_valid = false;     // at least one img != 0 && !img->emtpy
    bool all_exist = true;            // all img != 0 
    bool child_level_void = level + 1 < first_level_;
    bool nothing_exist = true;        // all image are missing and diamond cannot be refined.

    // iterate on row column to get the 4 children diamond, and relative data if present
    for(int r = 0; r < 2; ++r) {
      for(int c = 0; c < 2; ++c) {
        int idx = 2 * r + c;
        d_child[idx] = d.quad_child_diamond(r, c);

        // id to access cache is level, x, y
        level_xy_id[idx] = level_xy_from_diamond(level + 1, d_child[idx], root_count); // ds.child_level_xy_id(r, c);
	//	SL_TRACE_OUT(1) << "rc " << r << " " << c << " : " << d_child[idx].id() << " <-> " << level_xy_id[idx] << std::endl;

        if (!child_level_void) {
	  // take real data
	  rc_image[idx] = texture_cache_[level_xy_id[idx]];

	  if (rc_image[idx] == 0) {
	    // data not in cache, ask to texture_fetcher_
	    texture_fetcher_t::status_data_pair_t res = texture_fetcher_->fetch(level_xy_id[idx], aabox(d_child[idx]));
	    if (res.first == texture_fetcher_t::DONE) {
	      // set pointer to the image not to the reference_counted_compressed_image_t which will be created only if refinable
	      gl_image[idx] = res.second;
	    } 
	    
	    //	  SL_TRACE_OUT(1) << "    requeted img " << level_xy_id[idx] << (rc_image[idx] ? " arrived" : " missing") << std::endl;
	    // set flags
	    least_one_valid |= (res.first == texture_fetcher_t::DONE);
	    nothing_exist   &= (res.first == texture_fetcher_t::NULL_DATA);
	    all_exist       &= (res.first == texture_fetcher_t::DONE) || (res.first == texture_fetcher_t::NULL_DATA);
	  } else {
	    least_one_valid = true;
	    nothing_exist = false;
	  }
	}
      }
    }

    if (nothing_exist && !child_level_void) {
      // this is a real leaf of the input quadtree
      ds.exist_children() = false;
      return;
    }
    
    bool is_refinable = (least_one_valid && all_exist) || child_level_void;
    
    if (is_refinable) {
      // all data available: from cache or from the fetcher
      if (level+1 >= level_count()) {
        push_level_map();
      }
      
      // create new children 
      ds.is_leaf() = false;

      if (child_level_void) {
	// insert 4 diamond pointing to no data
	for(int r = 0; r < 2; ++r) {
	  for(int c = 0; c < 2; ++c) {
	    int idx = 2 * r + c;
	    (*diamond_map_by_level_[level+1])[d_child[idx]] = grid_texture_state_t(level_xy_id[idx], true, true, 0);
	  }
	}
      } else {
	// after first level and data exist: build 4 children
	sl::uint64_t time_stamp = global_time_stamp();
	//	reference_counted_compressed_image_t* parent_img = texture_cache_[ds.level_xy_id()];
	// parent_img could be null if this is the first ;evel
	std::size_t quad_width = texture_fetcher_->quad_width();
	const grid_point_t& parent_lxy = ds.level_xy_id();
	vic::img::gl_quadtree_image_processor gl_quadproc;
	for(int r = 0; r < 2; ++r) {
	  for(int c = 0; c < 2; ++c) {
	    int idx = 2 * r + c;
	    if (rc_image[idx] == 0) {
	      if (gl_image[idx] == 0) {
		// null data: copy from parent
		SL_TRACE_OUT(0) << "zoom in " << std::endl; 
		image_rgba_t zoomed_img(4, quad_width, quad_width);

		if (ds.reference_counted_image() != 0) {
		  image_rgba_t parent_img;
		  ds.reference_counted_image()->object()->uncompress_to(parent_img);
		  gl_quadproc.magnify_in(zoomed_img,
					 level_xy_id[idx][0],level_xy_id[idx][1],level_xy_id[idx][2],
					 parent_img,
					 parent_lxy[0], parent_lxy[1], parent_lxy[2]);
		} else {
		  SL_TRACE_OUT(-1) << "quad of first_level " << level << " with null image " << idx << ": fill with black " << std::endl;
		  zoomed_img.fill(0,0,0,0);
		}
		gl_image[idx] = new compressed_rgba_image_t(zoomed_img);
	      } // else data fetched from the fetcher
	      // create new reference_counted_compressed_image_t for images still not available from cache 
	      // but present in the fetcher or just created from parent
	      assert(gl_image[idx]);
	      rc_image[idx] = new reference_counted_compressed_image_t(&texture_cache_, gl_image[idx], time_stamp);
	      texture_cache_.insert(level_xy_id[idx], rc_image[idx]);
	    } else {
	      rc_image[idx]->global_time_stamp() = time_stamp;
	    }
	    assert(rc_image[idx]);

	    // insert diamond
	    (*diamond_map_by_level_[level+1])[d_child[idx]] = grid_texture_state_t(level_xy_id[idx], true, true, rc_image[idx]); // rc_image[idx]->ref
	    //	  std::cerr << "image use cnt " <<  (*diamond_map_by_level_[level+1])[d_child[idx]].reference_counted_image()->use_count() << std::endl;
	    //	  assert(rc_image[idx]->use_count() == 1);
	  }
	}
      }
      ++decoded_diamond_count_;
      refine_update_heap(level, it);
    }
  }

  void grid_texture_quadtree::coarsen(std::size_t level, const diamond_id_t id) {
    SL_TRACE_OUT(0) << "coarsen at " << level << " : " << id << std::endl; 
    grid_diamond_t key = grid_diamond_t(id,id,id,id);
    grid_texture_diamond_map_iterator_t it = diamond_map_by_level_[level]->find(key);
    assert(it!= diamond_map_by_level_[level]->end());
    
    const grid_diamond_t d = it->first;
    grid_texture_state_t& ds = it->second;

    // All children are leaf here
    for (int r=0; r<2; ++r) {
      for (int c=0; c<2; ++c) {
        diamond_id_t d_child_id = d.quad_child_diamond(r,c).id();
        //          diamond_t* d_child = get_diamond(d_child_id);
        assert(level+1 < level_count());

        grid_diamond_t key_child = grid_diamond_t(d_child_id,d_child_id,d_child_id,d_child_id);
        grid_texture_diamond_map_iterator_t it_child = diamond_map_by_level_[level+1]->find(key_child);
        if (it_child != diamond_map_by_level_[level+1]->end()) {
	  SL_TRACE_OUT(0) << "coarsening " << it_child->first.id() << std::endl;
          assert(it_child->second.is_leaf());

          // deleting diamond deref patch data
          diamond_map_by_level_[level+1]->erase(it_child);
        } else {
          SL_TRACE_OUT(-1) << "Coarsen missing child " << r << ", " << c << " of " << id << std::endl;
        }
      }
    }

    // FIXME CHECK VALIDITY
    ds.is_leaf() = true;
    coarsen_update_heap(level, it);
  } 

  void grid_texture_quadtree::recompute_heap_priority() {
    refinement_heap_t tmp_ref;
    for(refinement_heap_t::const_data_key_iterator_t rh_it = refinement_heap_.begin();
        rh_it != refinement_heap_.end(); ++rh_it) {
      std::size_t level = rh_it->first.level();
      diamond_id_t id = rh_it->second;
      grid_diamond_t key(id, id, id, id);
      grid_texture_diamond_map_const_iterator_t it = diamond_map_by_level_[level]->find(key);
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
      grid_texture_diamond_map_const_iterator_t it = diamond_map_by_level_[level]->find(key);
      if (it != diamond_map_by_level_[level]->end()) {        
        tmp_coarse.push_new(std::make_pair(get_priority_diamond(level, it), id));
      }
    }
      
    coarsening_heap_.clear();
    std::swap(coarsening_heap_,tmp_coarse);
  }

  void grid_texture_quadtree::refine_update_heap(std::size_t level, const grid_texture_diamond_map_const_iterator_t& it) {
    const grid_diamond_t& d = it->first;

    // - remove id from refinement_heap_
    diamond_id_t id = d.id();
    refinement_heap_.erase(id);

    // - add id children (if leafs) to refinement_heap_
    if (level+1<level_count()) {
      for(int r = 0; r < 2; ++r) {
        for(int c = 0; c < 2; ++c) {
          diamond_id_t c_id = d.quad_child_diamond(r, c).id();
          grid_diamond_t key_child = grid_diamond_t(c_id, c_id, c_id, c_id);
          grid_texture_diamond_map_const_iterator_t it_child = diamond_map_by_level_[level+1]->find(key_child);

          if (it_child !=  diamond_map_by_level_[level+1]->end() &&
              it_child->first.is_valid() &&
              it_child->second.is_leaf()) {
            refinement_heap_.push(std::make_pair(get_priority_diamond(level+1, it_child), c_id));
          } else {
            SL_TRACE_OUT(-1) << "NOT FOUND VALID CHILD" << r << " " << c << std::endl;
          }
        }
      }
    }
    
    // remove d parents from coarsening_heap_
    diamond_id_t p_id = d.quad_parent_id();
    if (coarsening_heap_.has(p_id)) {coarsening_heap_.erase(p_id);}
    
    // - add id to coarsening_heap_ only if all its children are leafs
    assert(check_all_children_are_leafs(level, it));
    assert(!coarsening_heap_.has(id));
    coarsening_heap_.push_new(std::make_pair(get_priority_diamond(level, it), id));
  }

  void grid_texture_quadtree::coarsen_update_heap(std::size_t level, const grid_texture_diamond_map_const_iterator_t& it) {
    // coarsed id :
    // - remove id children from refinement_heap_
    const grid_diamond_t& d = it->first;
    for (int r=0; r<2; ++r) {
      for (int c=0; c<2; ++c) {
        diamond_id_t c_id = d.quad_child_diamond(r, c).id();
        // remove a child from the heap, if this diamond is no more in the graph
        // (which means diamond has been removed from graph by coarsen procedure)
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
      diamond_id_t p_id = d.quad_parent_id();
      grid_diamond_t key_parent = grid_diamond_t(p_id, p_id, p_id, p_id);
      grid_texture_diamond_map_const_iterator_t parent_it = diamond_map_by_level_[level-1]->find(key_parent);
      if(parent_it != diamond_map_by_level_[level-1]->end()) {
        if (check_all_children_are_leafs(level-1, parent_it)) {
          //            add_to_coarsening_heap(parent, coarsening_heap_);
          assert(!coarsening_heap_.has(p_id));
          coarsening_heap_.push_new(std::make_pair(get_priority_diamond(level-1, parent_it), p_id));
        }
      }
    }
  }

  bool grid_texture_quadtree::check_all_children_are_leafs(std::size_t level,
                                                           const grid_texture_diamond_map_const_iterator_t& it) const {
    bool all_leafs = true;
    if (level+1<level_count()) {
      const grid_diamond_t& d = it->first;
      for(int r = 0; r < 2 && all_leafs; ++r) {
        for(int c = 0; c < 2 && all_leafs; ++c) {
          diamond_id_t c_id = d.quad_child_diamond(r, c).id();
          grid_diamond_t key_child = grid_diamond_t(c_id, c_id, c_id, c_id);
          grid_texture_diamond_map_const_iterator_t it_child = diamond_map_by_level_[level+1]->find(key_child);
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
  
  grid_point_t grid_texture_quadtree::level_xy_from_diamond(std::size_t level, const grid_diamond_t& d, std::size_t root_count) {
    int32_t root_size = max_grid_coord() - min_grid_coord();
    grid_point_t id = d.id();

    if (root_count == 1) {
      int half_root_size = root_size / 2;
      int level_tile_width = root_size >> level;

      //    SL_TRACE_OUT(1) << "level_xy_from_diamond " << level << ", d " << id << " , rs " << root_size << ", level_tile_width " << level_tile_width << std::endl;
      return grid_point_t(level,
			  (id[0] + half_root_size) / level_tile_width,
			  (id[1] + half_root_size) / level_tile_width);    
    } else {
      // cylindrical dataset: half quad sized
      root_size /= 2;
      int32_t half_root_size = root_size / 2;
      int32_t level_tile_width = root_size >> level;
      int32_t level_tile_count = 1 << level;
      grid_point_t level_xy;
      // compute level
      level_xy[0] = level;

      // compute X
      // unroll cylinder in 4 faces: |+Z|+X|-Z|-X|
      // add for face i-th all x-tiles lying on faces from 0 to i-th -1
      // on each face there are 1 << quad_level tiles.

      // BAD BUT NEXT 'IF SEQUENCE' SHOULD BE IN SYNCH WITH 
      //  - cylindrical_coordinate_transform::uv_from_grid
      //  - texture_tile_stack::texture_matrix
      // even level: diamond on a single face: derive face id from the center:
      if (id[2] == half_root_size) {
	// +Z
	level_xy[1] = 0  * level_tile_count + ( id[0] + half_root_size) / level_tile_width;
      } else if (id[0] == half_root_size) {
	// +X
	level_xy[1] = 1 * level_tile_count + (-id[2] + half_root_size) / level_tile_width;
      } else if (id[2] == -half_root_size) {
	// -Z
	level_xy[1] = 2 * level_tile_count + (-id[0] + half_root_size) / level_tile_width;
      } else if (id[0] == -half_root_size) {
	// -X
	level_xy[1] = 3 * level_tile_count + ( id[2] + half_root_size) / level_tile_width;
      } else {
	SL_TRACE_OUT(-1) << "grid point not lying on boundaries " << id << " cyl half root size " << half_root_size << std::endl;
      }

      // compute Y
      level_xy[2] = (id[1] + root_size) / level_tile_width;

      //      SL_TRACE_OUT(1) << "grid_texture_quadtree::level_xy_from_diamond level " << level << " diamond " << id << " level xy " << level_xy << std::endl;
      return level_xy;
    }

  }
} // namespace cbdam
