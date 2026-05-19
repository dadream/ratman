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
#include <vic/cbdam/base/terrain_model.hpp>
#include <vic/cbdam/base/background_thread.hpp>
#include <sl/linear_map_factory.hpp>
#include <sl/fixed_size_ray.hpp>

namespace cbdam {

  float previous_dmf = 0;

  class terrain_model_thread : public background_thread {
  protected:
    mutable QMutex mutex_;
    terrain_model* terrain_model_;
    bool stop_requested_;
    uint32_t update_frame_counter_;

  public:
    terrain_model_thread(terrain_model* x) :
      terrain_model_(x), stop_requested_(false) {

    }

    virtual ~terrain_model_thread() {
      SL_TRACE_OUT(1) << std::endl;
    }

    bool stop_requested() const {
      bool result;
      mutex_.lock();
      result = stop_requested_;
      mutex_.unlock();
      return result;
    }

    void request_stop() {
      mutex_.lock();
      stop_requested_ = true;
      mutex_.unlock();
    }

    virtual void run() {
      // call base class run to set proper priority
      background_thread::run();

      sl::real_time_clock cycle_clock;
      
      update_frame_counter_ = terrain_model_->frame_counter();
      const int enough_changes = 4;
      const uint32_t enough_frames = 4;

      while (!stop_requested()) {
	cycle_clock.restart();

	bool updated = false;
        terrain_model_->update_refine(updated);
	uint32_t tm_frame_counter = terrain_model_->frame_counter();
	// perform swap cuts if enough changes happened or if more than 4 frames have passed.
	// anyway more than one frame have to be passed from previous update
        if ( (tm_frame_counter > update_frame_counter_ && terrain_model_->incremental_updates_count() > enough_changes) ||
	     tm_frame_counter > update_frame_counter_ + enough_frames) {
	  update_frame_counter_ = tm_frame_counter;
          terrain_model_->update_swap_cuts();
        }

	if (cycle_clock.elapsed().as_milliseconds()>10) {
	  SL_TRACE_OUT(1) << "REFINE CYCLE TIME: " << sl::human_readable_duration(cycle_clock.elapsed()) << std::endl;
	}

#if 1
	if (terrain_model_->fetchers_data_available()) {
	  // do not wait too much if data is available for a new refine
	  background_thread::cpu_yield();
	} else {
	  background_thread::cpu_long_yield();
	}
#else
	background_thread::cpu_yield();
#endif
      }

      SL_TRACE_OUT(1) << "############## EXITING FROM UPDATE THREAD" << std::endl;
    }
  };

  //////////////////////////////////////// terrain_model //////////////////////////////////

  //////////////////////////////// functions called by MAIN thread //////////////////////////////////

  terrain_model::terrain_model(geometry_fetcher_t* gf) :
    representation_mutex_(QMutex::Recursive),
    layers_mutex_(QMutex::Recursive),
    parameters_mutex_(QMutex::Recursive) {

    // Init defaults
    update_thread_ = 0;
    geometry_layer_ = 0;
    camera_projection_ = sl::linear_map_factory3d::perspective(0.75, 1.0, 0.1, 1000.0);
    camera_view_ = sl::linear_map_factory3d::identity();
    threshold_ = 1.0f;
    focus_fraction_ = 0.75f;
    texture_cache_capacity_ = 512; // FIXME
    texture_out_cache_.set_capacity(texture_cache_capacity_);
    // FIXME    background_color_ = color4_t(44,75,87, 255);
    background_color_ = color4_t(0, 0, 0, 255);
    data_missing_fraction_ = 1.0;

    current_representation_ = new diamond_data_map_t();
    // Open geometry
    connect(gf);

    // Init srs
    spatial_reference_.reset(srs().c_str());
  }

  terrain_model::~terrain_model() {
    update_stop_and_clear();

    delete current_representation_;
    current_representation_ = 0;
  }

  void terrain_model::connect(geometry_fetcher_t* gf) {
    layers_mutex_.lock();
    {
      if (geometry_layer_) {
        delete geometry_layer_;
      }
      geometry_layer_ = new geometry_layer(gf);
      if (!geometry_layer_->diamond_graph().is_open()) {
	delete geometry_layer_;
	geometry_layer_ = 0;
      }
    }
    layers_mutex_.unlock();
  }

  bool terrain_model::is_connected() const {
    bool result;
    layers_mutex_.lock();
    {
      result = geometry_layer_ && geometry_layer_->diamond_graph().is_open();
    }
    layers_mutex_.unlock();
    return result;
  }

  std::string terrain_model::srs() const {
    // FIXME
    if (is_connected()) {
      return geometry_layer_->fetcher()->srs();
    } else {
      return std::string("NONE");
    }
  }

  aabox2d_t terrain_model::uv_box() const {
    aabox2d_t result;
    if (is_connected()) {
      result = geometry_layer_->fetcher()->uv_box();
    }
    return result;
  }

  void terrain_model::update_stop_and_clear() {
    SL_TRACE_OUT(1) << "update_stop_and_clear" << std::endl;

    // Stop update thread
    update_stop();

    // Clear data cut, must be done *BEFORE* clearing the graphs
    representation_mutex_.lock();
    {
      SL_TRACE_OUT(1) << "clear_data_cut" << std::endl;
      clear_data_cut(*current_representation_);

      SL_TRACE_OUT(1) << "clear_out cache" << std::endl;
      texture_out_cache_.minimize_footprint();

      SL_TRACE_OUT(1) << "After clear" << std::endl;
      //      texture_out_cache_.print_stats();
    }
    representation_mutex_.unlock();

    // Clear graphs and their caches
    layers_mutex_.lock();
    {
      SL_TRACE_OUT(1) << "clear_geometry" << std::endl;
      geometry_layer_->diamond_graph().clear();
      delete geometry_layer_;
      geometry_layer_ = 0;

      SL_TRACE_OUT(1) << "clear base color layers" << std::endl;
      clear_base_color_layers();

      SL_TRACE_OUT(1) << "clear overlay color layers" << std::endl;
      clear_overlay_color_layers();
    }
    layers_mutex_.unlock();
  }

  void terrain_model::update_start() {
    incremental_updates_count_ = 0;
    if (!update_thread_) {
      update_thread_ = new terrain_model_thread(this);
      update_thread_->start(QThread::LowPriority);
    }
  }

  void terrain_model::update_stop() {
    if (update_thread_) {
      update_thread_->request_stop();
      update_thread_->wait();
      delete update_thread_;
      update_thread_ = 0;
      SL_TRACE_OUT(1) << "update thread has stopped" << std::endl;
    }
  }

  void terrain_model::lock_current_representation() const {
    representation_mutex_.lock();
  }

  const terrain_model::diamond_data_map_t& terrain_model::current_representation() const {
    return *current_representation_;
  }

  void terrain_model::unlock_current_representation() const {
    representation_mutex_.unlock();
  }

  void terrain_model::set_refine_parameters(float threshold,
					    const projective_map_t& P,
					    const rigid_body_map_t& V,
					    float focus_fraction) {
    parameters_mutex_.lock();
    {
      threshold_ = threshold;
      camera_projection_ = P;
      camera_view_ = V;
      focus_fraction_ = focus_fraction;
      ++frame_counter_;
    }
    parameters_mutex_.unlock();
  }

  ///////////////////////////////////////////////////// functions called by SECONDARY thread //////////////////////////////////

  void terrain_model::update_refine(bool& cut_updated) {
#if 0
    {
      static sl::real_time_clock stat_clock;
      if (stat_clock.elapsed().as_milliseconds() > 2000) {
	std::cerr << "terrain_model::cache sz/cpc: geometry " << geometry_layer_->diamond_graph().geometry_cache().size() << "/" << geometry_layer_->diamond_graph().geometry_cache().capacity();

	std::size_t geo_sz = geometry_layer_->diamond_graph().geometry_cache().size() * geometry_layer_->diamond_graph().patch_vertices_count() * (sizeof(int32_t)+sizeof(sl::point3f)+sizeof(sl::row_vector3f));

	std::cerr << " texture ";
	std::size_t tex_sz = 0;
	for(std::size_t j = 0; j < 2; ++j) {
	  for(std::size_t i = 0; i < base_overlay_color_layers_[j].size(); ++i) {
	    std::cerr << base_overlay_color_layers_[j][i]->quadtree().texture_cache().size() << "/" << base_overlay_color_layers_[j][i]->quadtree().texture_cache().capacity() << " ";
	    tex_sz +=  base_overlay_color_layers_[j][i]->quadtree().texture_cache().size() * 256 * 256 * 4 * sizeof(uint8_t);
	  }
	}

	std::cerr << texture_out_cache_.size() << "/" << texture_out_cache_.capacity();
	std::size_t out_sz =  texture_out_cache_.size() * 256 * 256 * 4 * sizeof(uint8_t);

	std::cerr << " | SZ: geo: " << sl::human_readable_size(geo_sz) << " + tex: " << sl::human_readable_size(tex_sz) << " + out: " << sl::human_readable_size(out_sz) << " = " << sl::human_readable_size(geo_sz+tex_sz+out_sz) << std::endl;

	stat_clock.restart();
      }
    }
#endif

    // Extract parameters
    sl::projective_map3d P;
    sl::rigid_body_map3d V;

    float eps;
    float focus;
    parameters_mutex_.lock();
    {
      P = camera_projection_;
      V = camera_view_;
      eps = threshold_;
      focus = focus_fraction_;
    }
    parameters_mutex_.unlock();

    sl::projective_map3d P_prime = sl::linear_map_factory3d::scaling(sl::column_vector3d(1.0/focus, 1.0/focus, 1.0)) * P;
    float eps_prime = eps / focus;

    layers_mutex_.lock();
    {
      // Refine geometry layer
      int geometry_incremental_updates_count = 0;
      geometry_layer_->fetcher()->receive();
      geometry_layer_->diamond_graph().extract_cut(eps_prime, P_prime, V, geometry_incremental_updates_count);
      geometry_layer_->fetcher()->tick();
      background_thread::cpu_yield();

      // Refine texture layers
      std::size_t texture_timeout_ms = 50;
      bool recompute_heap_priority = data_missing_fraction_ > 0 || geometry_incremental_updates_count > 0;
      cut_updated = geometry_incremental_updates_count > 0;
      incremental_updates_count_ += geometry_incremental_updates_count;
      for(int j = 0; j < 2; ++j) {
	std::size_t layer_count = base_overlay_color_layers_[j].size();
	if (layer_count>0) {
	  for(std::size_t i = 0; i < layer_count; ++i) {
	    //	  std::cerr << "texture_cache_ " << i << " sz " << color_layers_[j][i]->quadtree().texture_cache().size() << std::endl;
	    if (base_overlay_color_layers_[j][i]->is_active()) {
	      int texture_incremental_updates_count = 0;
	      base_overlay_color_layers_[j][i]->fetcher()->receive();
	      base_overlay_color_layers_[j][i]->quadtree().extract_cut(texture_incremental_updates_count, recompute_heap_priority, texture_timeout_ms);
	      base_overlay_color_layers_[j][i]->fetcher()->tick();
	      incremental_updates_count_ += texture_incremental_updates_count;
	      cut_updated |= (texture_incremental_updates_count > 0);
	      background_thread::cpu_yield();
	    } else {
	      SL_TRACE_OUT(1) << "layer " << j << " " << i  << " not active" << std::endl;
	    }
	  }
	}
      }
    }
    layers_mutex_.unlock();
  }

  void terrain_model::update_swap_cuts() {
    diamond_data_map_t* tmp_representation = new diamond_data_map_t();
    std::vector<std::string> tmp_current_color_copyrigths;
    double dmf = 0.0;

    layers_mutex_.lock();
    {
      fill_data_cut(*tmp_representation, tmp_current_color_copyrigths);

      // Update data missing fraction
      dmf = geometry_layer_->diamond_graph().data_missing_fraction();

      for(std::size_t j = 0; j < 2; ++j) {
	std::size_t layer_count = base_overlay_color_layers_[j].size();
   	for(std::size_t i = 0; i < layer_count; ++i) {
	  if (base_overlay_color_layers_[j][i]->is_active()) {
	    dmf += base_overlay_color_layers_[j][i]->quadtree().data_missing_fraction();
	  }
	}
	dmf /= (1.0 + base_overlay_color_layers_[0].size() + base_overlay_color_layers_[1].size());
      }

      // reset counter of updated patch: FIXME CHECK IF IT HAS TO BE MOVED AFTER NEXT LOCK
      incremental_updates_count_ = 0;
    }
    layers_mutex_.unlock();

    //    SL_TRACE_OUT(1) << "cut size " << tmp_representation->size() << std::endl;
    representation_mutex_.lock();
    {
      current_color_copyrights_ = tmp_current_color_copyrigths;
      std::swap(tmp_representation, current_representation_);

      data_missing_fraction_ = dmf;
    }
    representation_mutex_.unlock();

    clear_data_cut(*tmp_representation);
    delete tmp_representation;
  }

  void terrain_model::update_tick() {
    geometry_layer_->fetcher()->tick();


    for(std::size_t j = 0; j < 2; ++j) {
      std::size_t layer_count = base_overlay_color_layers_[j].size();
      for(std::size_t i = 0; i < layer_count; ++i) {
	if (base_overlay_color_layers_[j][i]->is_active()) {
	  base_overlay_color_layers_[j][i]->fetcher()->tick();
	}
      }
    }
  }

  void terrain_model::fill_data_cut(diamond_data_map_t& cut,
				    std::vector<std::string>& copyrights) {
    bool has_texture = base_overlay_color_layers_[0].size() + base_overlay_color_layers_[1].size() > 0;
    std::size_t root_count = geometry_layer_->diamond_graph().root_count();
    const grid_diamond_graph_incore& dg = geometry_layer_->diamond_graph();
    const grid_diamond_graph_incore::refinement_heap_t& refinement_heap = dg.refinement_heap();
    point3d_t eye_pos = ~camera_view_ * point3d_t(0.0, 0.0, 0.0);
    double camera_h = uvh_xyz_transform()->altitude_from_xyz(eye_pos);

    // Copyrigths
    layer_set_t used_texture_layers;

    // for each diamond
    for(grid_diamond_graph_incore::refinement_heap_t::const_data_key_iterator_t rh_it = refinement_heap.begin();
	rh_it != refinement_heap.end();
	++rh_it) {
      const priority_diamond& pd = rh_it->first;
      const grid_diamond_graph_incore::diamond_id_t& id = rh_it->second;
      std::size_t level = pd.level();
      assert(level < dg.level_count());

      std::pair<bool, grid_diamond_graph_incore::grid_diamond_map_const_iterator_t> d_it = dg.diamond_at(level, id);
      if (d_it.first) {
	const grid_diamond_graph_incore::grid_diamond_t& d = d_it.second->first;
	const grid_diamond_graph_incore::grid_diamond_state_t& ds = d_it.second->second;

	// create a diamond accessor with pointer to diamond data, and reference() data
	bool is_odd_level = level%2;
	int needed_texture_tiles = level%2 + 1;
	grid_point_t ts_level_xy;
	affine_map_t texture_matrix;
	// for each diamond patch
	for(int patch_id = 0; patch_id < 2; ++patch_id) {
	  diamond_patch_id_t d_patch_id = std::make_pair(id, patch_id);
	  if (ds.has_fragment(patch_id)) {
	    // ref geometry
	    assert(ds.reference_counted_patch_data(patch_id));

	    // texture_stuff
	    // perform texture work only once when it is possible
	    if (has_texture) {
	      if (patch_id < needed_texture_tiles ||
		  !ds.has_fragment(0)) {
		// build texture stack for this diamond_level_xy
		texture_tile_stack ts;

		// get level_xy of the quad covering this patch diamond.
		// get also its corresponding geo diamond, needed for the build_texture_stack_in procedure
		grid_point_t tile_level_xy;
		grid_diamond_t tile_diamond;
		tile_covering_diamond_in(tile_level_xy, tile_diamond, level, d, patch_id);
		//		SL_TRACE_OUT(1) << "build texture stack in lev xy " << tile_level_xy << ", d " << tile_diamond.id() << ", patch_id " << patch_id << std::endl;
		build_texture_stack_in(ts, tile_level_xy, tile_diamond, used_texture_layers, camera_h);

		ts_level_xy = ts.stack_level_xy();
		//		SL_TRACE_OUT(1) << "texture_stack lev xy " << ts_level_xy << std::endl;

		reference_counted_uncompressed_image_t* ref_cnt_image = texture_out_cache_[ts_level_xy];
		if (ref_cnt_image == 0 || ref_cnt_image->global_time_stamp() != ts.stack_global_time_stamp()) {
		  // assume all layers have same texture width: FIXME
		  std::size_t quad_width = texture_quad_width();
		  uncompressed_rgba_image_t* img = new uncompressed_rgba_image_t(4, quad_width, quad_width);
		  ts.build_representation_in(img, background_color_[0], background_color_[1], background_color_[2], background_color_[3]);

		  ref_cnt_image = new reference_counted_uncompressed_image_t(&texture_out_cache_, img, ts.stack_global_time_stamp());

		  texture_out_cache_.insert(ts_level_xy, ref_cnt_image);
		}
		texture_matrix = ts.texture_matrix(tile_level_xy, is_odd_level, d, patch_id, root_count);

		// HERE WE INSERT DATA IN THE CUT
		cut.insert(std::make_pair(d_patch_id,
					  new diamond_patch_accessor(ds, patch_id, pd.visible(), ref_cnt_image, ts_level_xy, texture_matrix)));
	      } else {
		// texture has been built at previous patch id
		// ts_level_xy has been set at previous patch id and is the same.
		reference_counted_uncompressed_image_t* ref_cnt_image = texture_out_cache_[ts_level_xy];
		assert(ref_cnt_image); // at least inserted at previous patch

		cut.insert(std::make_pair(d_patch_id,
					  new diamond_patch_accessor(ds, patch_id, pd.visible(), ref_cnt_image, ts_level_xy, texture_matrix)));
	      }
	    } else {
	      // no texturing
	      cut.insert(std::make_pair(d_patch_id,
					new diamond_patch_accessor(ds, patch_id, pd.visible(), 0, grid_point_t(0,0,0), sl::linear_map_factory3d::identity())));
	    }
	  }
	}
      }
    }

    copyrights.clear();
    for(layer_set_t::iterator it = used_texture_layers.begin(); it != used_texture_layers.end(); ++it) {
      copyrights.push_back(base_overlay_color_layers_[it->first][it->second]->fetcher()->about());
    }

  }


  void terrain_model::clear_data_cut(diamond_data_map_t& cut) {
    //    SL_TRACE_OUT(1) << "clear_data_cut " << std::endl;
    for (diamond_data_map_t::iterator it = cut.begin();
	 it != cut.end();
	 ++it) {
      diamond_patch_accessor* da = it->second;
      assert(da);
      delete da;
    }
    cut.clear();
  }

  //////////////////////////////////// texture stuff ////////////////////////////////////

  void terrain_model::set_base_color_layer_active(std::size_t l) {
    assert(l < base_overlay_color_layers_[0].size());

    for(std::size_t i = 0; i < base_overlay_color_layers_[0].size(); ++i) {
      base_overlay_color_layers_[0][i]->set_active(false);
    }
    base_overlay_color_layers_[0][l]->set_active(true);
    incremental_updates_count_ += 1000;
    //    regenerate_out_cache();
  }

  void terrain_model::set_overlay_color_layer_active(std::size_t l, bool x) {
    assert(l < base_overlay_color_layers_[1].size());
    base_overlay_color_layers_[1][l]->set_active(x);
    incremental_updates_count_ += 1000;
    //    regenerate_out_cache();
  }

  bool terrain_model::is_overlay_color_layer_active(std::size_t l) const {
    assert(l < base_overlay_color_layers_[1].size());
    return base_overlay_color_layers_[1][l]->is_active();
  }


  std::size_t terrain_model::insert_base_color_layer(const std::string& id,
						     texture_fetcher_t* tf,
						     std::size_t first_level,
						     std::size_t last_level,
						     double min_altitude,
						     double max_altitude,
						     bool is_active) {
    std::size_t result = std::size_t(-1);

    layers_mutex_.lock();
    {
      texture_refiner* tr = new texture_refiner_grid_diamond_graph(&geometry_layer_->diamond_graph());
      const coordinate_transform* geo_xform = geometry_layer_->diamond_graph().uvh_xyz_transform();

      texture_layer* tl = new texture_layer(id, tf, tr, geo_xform, first_level, last_level, min_altitude, max_altitude);
      if (tl->quadtree().is_open()) {

	tl->quadtree().set_cache_capacity(texture_cache_capacity_); // FIXME
	tl->set_active(is_active);

	base_overlay_color_layers_[0].push_back(tl);
	SL_TRACE_OUT(1) << "inserted base color layer " << base_overlay_color_layers_[0].size() << std::endl;
	if (geometry_layer_) {
	  geometry_layer_->diamond_graph().set_texture_tile_width(tl->fetcher()->quad_width());
	}
	result = base_overlay_color_layers_[0].size()-1;
	if (is_active) set_base_color_layer_active(result);
      } else {
	delete tl;
	SL_TRACE_OUT(1) << "unable to open base color layer " << base_overlay_color_layers_[0].size() << std::endl;
      }
    }
    layers_mutex_.unlock();

    return result;
  }

  std::size_t terrain_model::insert_overlay_color_layer(const std::string& id,
							texture_fetcher_t* tf,
							std::size_t first_level,
							std::size_t last_level,
							double min_altitude,
							double max_altitude,
							bool is_active) {
    std::size_t result = std::size_t(-1);

    layers_mutex_.lock();
    {
      texture_refiner* tr = new texture_refiner_grid_diamond_graph(&geometry_layer_->diamond_graph());
      const coordinate_transform* geo_xform = geometry_layer_->diamond_graph().uvh_xyz_transform();

      texture_layer* tl = new texture_layer(id, tf, tr, geo_xform, first_level, last_level, min_altitude, max_altitude);
      if (tl->quadtree().is_open()) {

	tl->quadtree().set_cache_capacity(texture_cache_capacity_); // FIXME
	tl->set_active(is_active);

	base_overlay_color_layers_[1].push_back(tl);
	SL_TRACE_OUT(1) << "inserted overlay color layer " << base_overlay_color_layers_[1].size() << std::endl;
	if (geometry_layer_) {
	  geometry_layer_->diamond_graph().set_texture_tile_width(tl->fetcher()->quad_width());
	}
	result = base_overlay_color_layers_[1].size()-1;
	set_overlay_color_layer_active(result, is_active);
      } else {
	delete tl;
	SL_TRACE_OUT(-1) << "unable to open overlay color layer " << base_overlay_color_layers_[1].size() << std::endl;
      }
    }
    layers_mutex_.unlock();

    return result;
  }

  void terrain_model::clear_base_color_layers() {
    for(std::size_t i = 0; i < base_overlay_color_layers_[0].size(); ++i) {
      delete base_overlay_color_layers_[0][i];
    }
    base_overlay_color_layers_[0].clear();
  }

  void terrain_model::clear_overlay_color_layers() {
    for(std::size_t i = 0; i < base_overlay_color_layers_[1].size(); ++i) {
      delete base_overlay_color_layers_[1][i];
    }
    base_overlay_color_layers_[1].clear();
  }

  void terrain_model::build_texture_stack_in(texture_tile_stack& t_stack, const grid_point_t& level_xy, const grid_diamond_t& d, 
					     layer_set_t& used_texture_layers,
					     double camera_h) const {
    for(std::size_t j = 0; j < 2; ++j) {
      std::size_t layer_count = base_overlay_color_layers_[j].size();
      for(std::size_t i = 0; i < layer_count; ++i) {
	if (base_overlay_color_layers_[j][i]->is_active() &&
	    base_overlay_color_layers_[j][i]->is_camera_in_range(camera_h)) {
	  // hack to access firstly the base layer (if exist), next all overlays
	  const grid_texture_quadtree& tq = base_overlay_color_layers_[j][i]->quadtree();
	  int level = level_xy[0];
	  grid_diamond_t new_d = d;
	  bool has_diamond = tq.has(level/2, new_d.id());
	  int first_level = base_overlay_color_layers_[j][i]->quadtree().first_level();
	  //	  int last_level = base_overlay_color_layers_[j][i]->quadtree().last_level();

	  while (!has_diamond && level > first_level+1) {
	    grid_point_t quad_parent_id = new_d.quad_parent_id();

	    // ask diamond to diamond_graph_ only to be able to build parent diamond
	    // parent diamond cannot be asked to texture, because we don't know if it is present
	    std::pair<bool, grid_diamond_graph_incore::grid_diamond_map_const_iterator_t> d_it = geometry_layer_->diamond_graph().diamond_at(level-2, quad_parent_id);
	    if (!d_it.first) {
	      SL_TRACE_OUT(-1) << "unable to get quad parent " << quad_parent_id << std::endl;
	      break;
	    } else {
	      new_d = d_it.second->first;
	      level -= 2;
	      has_diamond = tq.has(level/2, quad_parent_id);
	    }
	  }

	  if (has_diamond) {
	    used_texture_layers.insert(std::make_pair(j, i));
	    // insert tile in stack
	    std::pair<bool, grid_texture_quadtree::grid_texture_diamond_map_const_iterator_t> td_it = tq.diamond_at(level/2, new_d.id());
	    if (td_it.first) {
	      //	  SL_TRACE_OUT(1) << "asking to layer " << i << " diamond " << new_d.id() << " level xy " << t_state.level_xy_id() << std::endl;
	      //          const reference_counted_image_t* ref_cnt_img = tq.texture_cache()[t_state.level_xy_id()];
	      const grid_texture_quadtree::grid_texture_state_t& t_state = td_it.second->second;
	      const reference_counted_compressed_image_t* ref_cnt_img = t_state.reference_counted_image();// tq.texture_cache()[t_state.level_xy_id()];

	      if (ref_cnt_img) {
		//	  SL_TRACE_OUT(1) << "layer " << i << "tex stack add d id " << new_d.id() << ", lev xy " << t_state.level_xy_id() << " img sz "
		//		    << ref_cnt_img->object().extent(0) << " " << ref_cnt_img->object().extent(1) << " "<< ref_cnt_img->object().extent(2) << std::endl;

		assert(ref_cnt_img->use_count() > 0);
		t_stack.insert_tile(texture_tile_descriptor(t_state.level_xy_id(), ref_cnt_img->global_time_stamp(), ref_cnt_img->object()));
	      }
	    } else {
	      SL_TRACE_OUT(-1) << "missing texture diamond " << new_d.id() << " at level " << level /2 << " on layer " << i << std::endl;
	    }
	  } else {
	    // NOT AN ERROR SL_TRACE_OUT(1) << "unable to get covering diamond for " << d.id() << " on layer " << i << std::endl;
	  }
	}
      }
    }
  }

  void terrain_model::tile_covering_diamond_in(grid_point_t& tile_level_xy, grid_diamond_t& tile_d,
					       std::size_t level, const grid_diamond_t& d, int patch_id) const {
    // 1 for odd levels, 0 for even
    int odd = level%2;
    int quad_level = level / 2;

    // out_diamond is needed for build_texture_stack_in procedure
    // we want: diamond for even levels, parent for odd
    grid_point_t target_id = (odd ? d.parent_id(patch_id) : d.id());
    if (odd) {
      std::pair<bool, grid_diamond_graph_incore::grid_diamond_map_const_iterator_t> d_it = geometry_layer_->diamond_graph().diamond_at(level-1, target_id);
      assert(d_it.first);
      tile_d = d_it.second->first;
    } else {
      tile_d = d;
    }

    std::size_t root_count = geometry_layer_->diamond_graph().root_count();
    // get level xy of the texture quadtree (xy correspond to geometry quadtree)
    tile_level_xy = grid_texture_quadtree::level_xy_from_diamond(quad_level, tile_d, root_count);
    // set level to geometry level
    tile_level_xy[0] = level - odd;
    //    SL_TRACE_OUT(1) << "terrain_model::tile_covering_diamond_in " << d.id() << " at level " << level << ", level xy " << tile_level_xy << std::endl;
  }

  void terrain_model::regenerate_out_cache() {
    SL_TRACE_OUT(-1) << "FIXME DISABLED" << std::endl;
    return;

    // Clear current cut
    representation_mutex_.lock();
    {
      SL_TRACE_OUT(1) << "clear data cut" << std::endl;
      clear_data_cut(*current_representation_);

      SL_TRACE_OUT(1) << "out cache minimize footprint" << std::endl;
      texture_out_cache_.minimize_footprint();
      SL_TRACE_OUT(1) << "out cache sz " << texture_out_cache_.size() << std::endl;
    }
    representation_mutex_.unlock();

    // Prepare new cut
    diamond_data_map_t* tmp_representation = new diamond_data_map_t();
    std::vector<std::string> tmp_current_color_copyrigths;

    layers_mutex_.lock();
    {
      fill_data_cut(*tmp_representation, tmp_current_color_copyrigths);
    }
    layers_mutex_.unlock();

    // Make new cut current
    representation_mutex_.lock();
    {
      current_color_copyrights_ = tmp_current_color_copyrigths;
      std::swap(tmp_representation, current_representation_);
    }
    representation_mutex_.unlock();

    // Delete old cut
    clear_data_cut(*tmp_representation);
    delete tmp_representation;
  }

  //  public: // Coordinate conversions

  point3d_t terrain_model::xyz_from_WGS84_lonlat(const point3d_t& uvh) const {
    point3d_t uvh_srs = spatial_reference_.from_WGS84_lonlat(uvh);
    if (spatial_reference_transformation_t::is_valid(uvh_srs)) {
      return xyz_from_uvh(uvh_srs);
    } else {
      return uvh_srs; // INVALID
    }
  }

  point3d_t terrain_model::xyz_from_uvh(const point3d_t& uvh) const {
    return uvh_xyz_transform()->xyz_from_uvh(uvh);
  }

  point3d_t terrain_model::uvh_from_xyz(const point3d_t& xyz) const {
    return uvh_xyz_transform()->uvh_from_xyz(xyz);
  }

  point3d_t terrain_model::WGS84_lonlat_from_xyz(const point3d_t& xyz) const {
    point3d_t uvh = uvh_from_xyz(xyz);
    return spatial_reference_.to_WGS84_lonlat(uvh);
  }

  point3d_t terrain_model::uvh_from_WGS84_lonlat(const point3d_t& uvh) const {
   return spatial_reference_.from_WGS84_lonlat(uvh);
  }

  point3d_t terrain_model::WGS84_lonlat_from_uvh(const point3d_t& uvh) const {
   return spatial_reference_.to_WGS84_lonlat(uvh);
  }

  vector3d_t terrain_model::xyz_up_from_xyz(const point3d_t& xyz) const {
    return uvh_xyz_transform()->up_from_xyz(xyz);
  }

  vector3d_t terrain_model::xyz_north_from_xyz(const point3d_t& xyz) const {
    return uvh_xyz_transform()->north_from_xyz(xyz);
  }

  vector3d_t terrain_model::xyz_east_from_xyz(const point3d_t& xyz) const {
     return uvh_xyz_transform()->east_from_xyz(xyz);
  }

  vector3d_t terrain_model::xyz_up_from_uvh(const point3d_t& uvh) const {
    return uvh_xyz_transform()->up_from_uvh(uvh);
  }

  vector3d_t terrain_model::xyz_north_from_uvh(const point3d_t& uvh) const {
    return uvh_xyz_transform()->north_from_uvh(uvh);
  }

  vector3d_t terrain_model::xyz_east_from_uvh(const point3d_t& uvh) const {
     return uvh_xyz_transform()->east_from_uvh(uvh);
  }

  vector3d_t terrain_model::xyz_up_from_WGS84_lonlat(const point3d_t& uvh) const {
    point3d_t uvh_srs = spatial_reference_.from_WGS84_lonlat(uvh);
    return xyz_up_from_uvh(uvh_srs);
  }

  vector3d_t terrain_model::xyz_north_from_WGS84_lonlat(const point3d_t& uvh) const {
    point3d_t uvh_srs = spatial_reference_.from_WGS84_lonlat(uvh);
    return xyz_north_from_uvh(uvh_srs);
  }

  vector3d_t terrain_model::xyz_east_from_WGS84_lonlat(const point3d_t& uvh) const {
    point3d_t uvh_srs = spatial_reference_.from_WGS84_lonlat(uvh);
    return xyz_east_from_uvh(uvh_srs);
  }

  terrain_model::rigid_body_map_t terrain_model::local_to_global_from_WGS84_lonlat(const point3d_t& uvh) const {
    sl::point3d O = xyz_from_WGS84_lonlat(uvh);
    sl::vector3d X = xyz_east_from_WGS84_lonlat(uvh);
    sl::vector3d Y = xyz_north_from_WGS84_lonlat(uvh);
    sl::vector3d Z = xyz_up_from_WGS84_lonlat(uvh);
    return rigid_body_map_t(sl::matrix4d(X[0], Y[0], Z[0], O[0],
			                 X[1], Y[1], Z[1], O[1],
				         X[2], Y[2], Z[2], O[2],
					 0   ,    0,    0,  1));
  }


  const terrain_model::spatial_reference_t& terrain_model::spatial_reference_system() const {
    return spatial_reference_;
  }

  std::pair<bool, double> terrain_model::current_representation_ground_elevation_from_WGS84_lonlat(const point2d_t& uv) const {
    point3d_t uvh = uvh_from_WGS84_lonlat(point3d_t(uv[0], uv[1], 0.0));
    return current_representation_ground_elevation_from_uv(point2d_t(uvh[0], uvh[1]));
  }

  std::pair<bool, double> terrain_model::current_representation_ground_elevation_from_uv(const point2d_t& uv) const {
    point3d_t xyz = xyz_from_uvh(point3d_t(uv[0], uv[1], 0.0));
    return current_representation_ground_elevation_from_xyz(xyz);
  }

  std::pair<bool, double> terrain_model::current_representation_ground_elevation_from_xyz(const point3d_t& xyz) const {
    point3d_t ground_xyz = uvh_xyz_transform()->xyz_on_ground(xyz);
    vector3d_t up_xyz = uvh_xyz_transform()->up_from_xyz(xyz);

    double max_h;
    if (is_planar()) {
      // These are extremely conservative bounds!
      max_h = uvh_xyz_transform()->bounding_rectangle().diagonal().two_norm();
    } else {
      // These are extremely conservative bounds!
      max_h = 0.2*radius();
    }
    double min_h = -max_h;

    point3d_t origin_xyz = ground_xyz + max_h * up_xyz;
    point3d_t extremity_xyz = ground_xyz + min_h * up_xyz;

#if 0
    // FIXME Straightforward implementation based on general raycasting.
    // FIXME Speed-up by handling special case of vertical segment

    std::pair<bool, double> hit = current_representation_nearest_intersection_from_xyz(origin_xyz, extremity_xyz);
    if (hit.first) {
      point3d_t hit_xyz = origin_xyz + hit.second * (extremity_xyz - origin_xyz);
      double hit_ground_elevation = (hit_xyz - ground_xyz).dot(up_xyz);
      return std::make_pair(true, hit_ground_elevation);
    } else {
      return std::make_pair(false, 0.0);
    }
#else
    std::pair<bool, double> result = std::make_pair(false, 0.0);
    lock_current_representation();
    {
      sl::fixed_size_ray<3, double> r(origin_xyz, extremity_xyz);
      for(diamond_data_map_t::const_iterator it = current_representation_->begin();
	  it != current_representation_->end() && !result.first;
	  ++it) {
	if (it->second->diamond_state().bounding_volume().intersection_exists(r)) {
	  result = patch_elevation(origin_xyz, extremity_xyz, it->second->vertices());
	}
      }
    }
    unlock_current_representation();

    return result;
#endif
  }

  std::pair<bool, double> terrain_model::patch_elevation(const point3d_t& origin_xyz, const point3d_t& extremity_xyz,
							 const diamond_vertices_t* vertices) const {
    // 3 patch corners
    const int patch_dim = height_patch_dim();
    const point3d_t& dc = vertices->diamond_center();
    const point3_t& p0 = vertices->gl_points()[0];
    const point3_t& p1 = vertices->gl_points()[patch_dim];
    const point3_t& p2 = vertices->gl_points()[vertices->gl_points().size()-1];


    // find barycentric cooords
    double u, v;
    ray r(origin_xyz, extremity_xyz);
    bool ok;
    double t;
    r.closest_triangle_intersection(point3d_t(p0[0] + dc[0], p0[1] + dc[1], p0[2] + dc[2]),
				    point3d_t(p1[0] + dc[0], p1[1] + dc[1], p1[2] + dc[2]),
				    point3d_t(p2[0] + dc[0], p2[1] + dc[1], p2[2] + dc[2]),
				    ok, t, u, v, (normald_t*)0);

    if (ok) {
      // get nearest height to intersection
      int selected_x = int(patch_dim * u);
      int selected_y = int(patch_dim * v);
      int point_idx = selected_x;
      for(int y = 0; y < selected_y; ++y) {
	point_idx += (patch_dim + 1 - y);
      }
      return std::make_pair(true, geometry_layer_->diamond_graph().height_scale_factor() * vertices->values()[point_idx]);
    } else {
      return std::make_pair(false, 0.0);
    }

#if 0
    // barycentric coordinates
    vector3d_t d0 = x - p0;
    vector3d_t d1 = p1 - p0;
    vector3d_t d2 = p2 - p0;
    double det = d1.cross(d2)[2];
    double u = d0.cross(d2)[2] / det;
    double v = d1.cross(d0)[2] / det;
#endif




  }

  std::pair<bool, double> terrain_model::current_representation_nearest_intersection_from_xyz(const point3d_t& origin,
											      const point3d_t& extremity) const {
    const int patch_dim = height_patch_dim();
    bool found = false;
    double t = 1e30;
    int intersection_count = 0;

    lock_current_representation();
    {
      // FIXME Optimize
      for(diamond_data_map_t::const_iterator it = current_representation_->begin();
	  it != current_representation_->end();
	  ++it) {
	if (it->second->diamond_state().bounding_volume().intersection_exists(sl::fixed_size_ray<3, double>(origin, extremity))) {
	  std::pair<bool, double> tmp_result = it->second->vertices()->patch_ray_intersection(origin, extremity, patch_dim, (normald_t*)0);
	  if (tmp_result.first && tmp_result.second < t) {
	    found = true;
	    t = tmp_result.second;
	    ++intersection_count;
	  }
	}
      }
    }
    unlock_current_representation();

    return std::make_pair(found, t);
  }

  const std::string& terrain_model::current_elevation_copyright() const {
    static std::string missing_copyright("missing elevation data");
    if (geometry_layer_ && geometry_layer_->fetcher()) {
      return geometry_layer_->fetcher()->about();
    } else {
      return missing_copyright;
    }
 
  }

  bool terrain_model::fetchers_data_available() const {
    bool result = geometry_layer_->fetcher()->data_available();
    if (!result) {
      layers_mutex_.lock();
      {
	std::size_t N =  base_overlay_color_layers_[0].size();
	for(std::size_t i = 0; i < N && !result; ++i) {
	  if (base_overlay_color_layers_[0][i]->is_active()) {
	    result = base_overlay_color_layers_[0][i]->fetcher()->data_available();
	  }
	}

	N =  base_overlay_color_layers_[1].size();
	for(std::size_t i = 0; i < N && !result; ++i) {
	  if (base_overlay_color_layers_[1][i]->is_active()) {
	    result = base_overlay_color_layers_[1][i]->fetcher()->data_available();
	  }
	}
      }
      layers_mutex_.unlock();
    }

    return result;
  }

}


