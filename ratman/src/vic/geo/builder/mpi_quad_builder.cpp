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
#include <vic/geo/builder/mpi_quad_builder.hpp>
#include <vic/geo/builder/geo_transform.hpp>
#include <vic/geo/builder/geo_utility.hpp>
#include <vic/geo/builder/quad_accessor.hpp>
#include <sl/std_serializer.hpp>
#include <sl/utility.hpp>
#include <iomanip>
#include <algorithm>
#include <cassert>
#include <cstdio>

namespace vic {

  namespace geo {

    void mpi_tile_chunk::store_to(sl::output_serializer& s) const {
      sl::uint64_t tl = tile_level_;
      s << tile_fname_ << tl << tile_warp_xform_ << quad_box_xy_ << is_split_;
    }

    void mpi_tile_chunk::retrieve_from(sl::input_serializer& s) {
      sl::uint64_t tl;
      s >> tile_fname_ >> tl >> tile_warp_xform_ >> quad_box_xy_ >> is_split_;
      tile_level_ = tl;
    }
    
    mpi_quad_builder::mpi_quad_builder() {
    }

    mpi_quad_builder::~mpi_quad_builder() {
    }

    std::string mpi_quad_builder::out_quad_index_fname() const {
      char tmp[64];
      sprintf(tmp, "%03d", mpi::process_rank());
      return qaccessor_->root_dir() + "/" + "quad_index_"+ std::string(tmp) + ".log";
    }

    void mpi_quad_builder::begin_processing() {
      if (mpi::process_count() < 2) {
	// Sequential!
	super_t::begin_processing();
      } else if (mpi::process_rank() == 0) {
	// Coordinator
	super_t::begin_processing();
	std::cout << "STARTING PROCESSING WITH 1 coordinator and " << mpi::process_count()-1 << " workers" << std::endl;
	std::cerr << "INFO: == Starting processing with 1 coordinator and " << mpi::process_count()-1 << " workers" << std::endl;
      } else {
	// Worker
	// just create log
	out_quad_index_begin();
      }
      build_clock_.restart();
    }

    void mpi_quad_builder::process_tile(const std::string& tile_fname, int level) {
      if (mpi::process_count() < 2) {
	// Sequential!
	super_t::process_tile(tile_fname, level);
      } else if (mpi::process_rank() == 0) {
	// Coordinator
	super_t::process_tile(tile_fname, level);
      } else {
	// Worker
	// do nothing
      }
    }

    void mpi_quad_builder::process_directory(const std::string& dirname,
					     const std::string& pattern, int level) {
      if (mpi::process_count() < 2) {
	// Sequential!
	super_t::process_directory(dirname, pattern, level);
      } else if (mpi::process_rank() == 0) {
	// Coordinator
	super_t::process_directory(dirname, pattern, level);
      } else {
	// Worker
	// do nothing
      }
    }


    void mpi_quad_builder::end_processing() {
      super_t::end_processing();

      // Kill workers
      if (mpi::process_rank() == 0) {
	std::cout << "Killing workers..." << std::endl;
	for (int worker=1; worker<mpi::process_count(); ++worker) {
	  mpi::send(worker, MPI_TAG_DIE, true);
	}
	std::cout << "Done." << std::endl;
	std::cout << "------------------------------------------------" << std::endl;
	sl::time_duration t_current = build_clock_.elapsed();
	std::cout << "TOTAL TIME: " << sl::human_readable_duration(t_current) << std::endl;
	std::cout << "------------------------------------------------" << std::endl;

	std::cerr << "INFO: == Processing completed." << std::endl;
	std::cerr << "INFO: == Total processing time: " << sl::human_readable_duration(t_current)<< std::endl;  
      }
    }

    void mpi_quad_builder::process() {
      if (mpi::process_count() < 2) {
	// Sequential!
	super_t::process();
      } else if (mpi::process_rank() == 0) {
	// Coordinator
	mpi_main_create_tile_chunks();
	mpi_main_process();
      } else {
	// Worker
	mpi_worker_process_requests();
      }
    }

    void mpi_quad_builder::repair() {
      if (mpi::process_count() < 2) {
	// Sequential!
	super_t::repair();
      } else if (mpi::process_rank() == 0) {
	// Coordinator
	super_t::repair();
      } else {
	// Worker
	// do nothing
      } 
    }

    void mpi_quad_builder::damage(int l, int x, int y) {
      if (vic::mpi::process_rank() == 0) {
	// Coordinator
	super_t::damage(l,x,y);
      } else {
	// Worker signal damage to coordinator
	mpi::send(0, MPI_TAG_SIGNAL_QUAD_DAMAGED, l, x, y);
      }
    }

    void mpi_quad_builder::repair_level(int l) {
      if (mpi::process_count() < 2) {
	// Sequential!
	super_t::repair_level(l);
      } else if (mpi::process_rank() == 0) {
	// Coordinator
	mpi_main_repair_level(l);
      } else {
	// Worker
	// do nothing
      } 
    }
    
    // =========================================================
    void mpi_quad_builder::mpi_main_insert_tile_chunks(const mpi_tile_chunk& ck) {

      point2i_t lo = ck.quad_box_xy()[0];
      point2i_t hi = ck.quad_box_xy()[1];

      int quad_count_x = (hi[0]+1-lo[0]);
      int quad_count_y = (hi[1]+1-lo[1]);
      int quad_count = quad_count_x*quad_count_y;
      int pixel_count = quad_count * quad_width_ * quad_height_;
      std::size_t byte_count = std::size_t(3 * pixel_count);
      if (byte_count<=max_incore_tile_memory()) {
	SL_TRACE_OUT(1) << "INSERT: " << 
	  "(" << ck.quad_box_xy()[0][0] << " " << ck.quad_box_xy()[0][1] << ")" << 
	  "(" << ck.quad_box_xy()[1][0] << " " << ck.quad_box_xy()[1][1] << ")" << 
	  " QUADS " << quad_count << std::endl;
	mpi_tile_chunks_.push_back(ck);
      } else if (quad_count_x > quad_count_y) {
	// Split x
	int split = lo[0]+quad_count_x/2;
	mpi_tile_chunk chunk_lo(ck.tile_fname(),
				ck.tile_level(),
				ck.tile_warp_xform(),
				aabox2i_t(point2i_t(lo[0],lo[1]),
					  point2i_t(split,hi[1])),
				true); // is split
	mpi_tile_chunk chunk_hi(ck.tile_fname(),
				ck.tile_level(),
				ck.tile_warp_xform(),
				aabox2i_t(point2i_t(split+1,lo[1]),
					  point2i_t(hi[0],hi[1])),
				true); // is split
	SL_TRACE_OUT(1) << "SPLIT X: " << 
	  ck.quad_box_xy() << 
	  "L: (" << chunk_lo.quad_box_xy()[0][0] << " " << chunk_lo.quad_box_xy()[0][1] << ")" << 
	  "(" << chunk_lo.quad_box_xy()[1][0] << " " << chunk_lo.quad_box_xy()[1][1] << ")" << 
	  "H: (" << chunk_hi.quad_box_xy()[0][0] << " " << chunk_hi.quad_box_xy()[0][1] << ")" << 
	  "(" << chunk_hi.quad_box_xy()[1][0] << " " << chunk_hi.quad_box_xy()[1][1] << ")" << 
	  std::endl;
	mpi_main_insert_tile_chunks(chunk_lo);
	mpi_main_insert_tile_chunks(chunk_hi);
      } else {
	// Split y
	int split = lo[1]+quad_count_y/2;
	mpi_tile_chunk chunk_lo(ck.tile_fname(),
				ck.tile_level(),
				ck.tile_warp_xform(),
				aabox2i_t(point2i_t(lo[0],lo[1]),
					  point2i_t(hi[0],split)),
				true); // is split
	mpi_tile_chunk chunk_hi(ck.tile_fname(),
				ck.tile_level(),
				ck.tile_warp_xform(),
				aabox2i_t(point2i_t(lo[0],split+1),
					  point2i_t(hi[0],hi[1])),
				true); // is split
	SL_TRACE_OUT(1) << "SPLIT Y: " << 
	  ck.quad_box_xy() << 
	  "L: (" << chunk_lo.quad_box_xy()[0][0] << " " << chunk_lo.quad_box_xy()[0][1] << ")" << 
	  "(" << chunk_lo.quad_box_xy()[1][0] << " " << chunk_lo.quad_box_xy()[1][1] << ")" << 
	  "H: (" << chunk_hi.quad_box_xy()[0][0] << " " << chunk_hi.quad_box_xy()[0][1] << ")" << 
	  "(" << chunk_hi.quad_box_xy()[1][0] << " " << chunk_hi.quad_box_xy()[1][1] << ")" << 
	  std::endl;
	mpi_main_insert_tile_chunks(chunk_lo);
	mpi_main_insert_tile_chunks(chunk_hi);
      }
    }

    void mpi_quad_builder::mpi_main_create_tile_chunks() {
      std::cout << "----------------------------------------------------------------------------------------" << std::endl;
      std::cout << "Preparing chunk list from set of input tiles" << std::endl;
      std::cout << "----------------------------------------------------------------------------------------" << std::endl;
      
      sl::real_time_clock process_clock;
      process_clock.restart();

      // Randomize order of tiles in order to 
      // probabilistically minimize overlapping
      // of nearby ones 
      std::cout << "RESHUFFLING...." << std::endl;

      const std::size_t tile_count=input_tile_fname_level_.size();
      std::vector<std::size_t> tile_index(tile_count);
      for (std::size_t tile_idx=0; tile_idx<tile_count; ++tile_idx) {
	tile_index[tile_idx] = tile_idx;
      }

      std::random_shuffle(tile_index.begin(),
			  tile_index.end());

      std::cout << "DONE." << std::endl;

      std::size_t quad_count = 0;
      mpi_tile_chunks_.clear();
      for (std::size_t tile_k=0; tile_k<tile_count; ++tile_k) {
	const std::size_t  tile_idx = tile_index[tile_k];

	const std::string& tile_fname = input_tile_fname_level_[tile_idx].first;
	int                tile_level = input_tile_fname_level_[tile_idx].second;
	geo_transform      tile_warp_xform;
	aabox2i_t          quad_box_xy;

	tile_parameters_in(tile_idx, 
			   tile_level,
			   tile_warp_xform,
			   quad_box_xy);
	if (tile_level<0) {
	  std::cout << "[" << std::setw(4) << tile_k << "/" << std::setw(4) << tile_count << "] => BAD TILE!" << std::endl;
	  std::cerr << "WARNING: Skipping bad tile: " << tile_fname << std::endl;
	} else {
	  // Insert chunks
	  mpi_tile_chunk tile_chunk(tile_fname,
				    tile_level,
				    tile_warp_xform,
				    quad_box_xy,
				    false); // not split

	  std::size_t old_chunk_count = mpi_tile_chunks_.size();
	  mpi_main_insert_tile_chunks(tile_chunk);
	  std::size_t tile_chunk_count = mpi_tile_chunks_.size() - old_chunk_count;

	  // Print progress report
	  int tile_quad_count = 
	    (quad_box_xy[1][0]+1-quad_box_xy[0][0])*
	    (quad_box_xy[1][1]+1-quad_box_xy[0][1]);

	  quad_count += tile_quad_count;

	  sl::time_duration t_current = process_clock.elapsed();
	  double us_per_tile = double(t_current.as_microseconds())/double(tile_k+1);
	  sl::time_duration t_remaining = sl::time_duration(sl::uint64_t(us_per_tile*(tile_count-tile_k)));
	  std::cout << "[" << std::setw(4) << tile_k << "/" << std::setw(4) << tile_count << "] => " <<
	    " L= " << std::setw(2) << tile_level << 
	    " -- N Quads: " << std::setw(4) << tile_quad_count << 
	    " -- N Chunks: " << std::setw(2) << tile_chunk_count  << 	      
	    " -- T: " << std::setw(10) << sl::human_readable_duration(t_current) <<
	    " -- ETA: " << std::setw(10) << sl::human_readable_duration(t_remaining) << 
	    " -- TOTAL: " << std::setw(10) << sl::human_readable_duration(build_clock_.elapsed()) <<
	    std::endl;
	}
      }

      // FIXME - this is here bacause right now distribution of
      // chunks to workers does not preserve image coherency. 
      // At this point, we fixed the problems by having large
      // chunks and fully randomizing them. Large chunks lead
      // to a good local utilization of caching (image open/close
      // overhead is amortized over many processed pixels). 
      // Chunk randomization tries to reduce dependencies and
      // increase probability of having workers working on
      // different disks. 
      std::random_shuffle(mpi_tile_chunks_.begin(),
			  mpi_tile_chunks_.end());

      std::cout << "DONE." << std::endl;
      std::cout << " Input data projects to approximately " << sl::human_readable_quantity(quad_count*quad_width_*quad_height_) << "pixels" << std::endl; 
      std::cout << input_tile_fname_level_.size() << " tiles => " << mpi_tile_chunks_.size() << " chunks" << std::endl;
      std::cout << "----------------------------------------------------------------------------------------" << std::endl;

      std::cerr << "INFO: " << input_tile_fname_level_.size() << " tiles split into " << mpi_tile_chunks_.size() << " chunks." << std::endl;
      std::cerr << "INFO: Input data projects to approximately " << sl::human_readable_quantity(quad_count*quad_width_*quad_height_) << "pixels." << std::endl; 
    }

    void mpi_quad_builder::mpi_main_init_worker_boxes() {
      aabox2i_t empty_box;
      empty_box.to_empty();

      mpi_main_worker_box_.clear();
      mpi_main_worker_box_.resize(mpi::process_count(),
				  empty_box);
    }
     
    void mpi_quad_builder::mpi_main_assign_box(std::size_t i, 
					       const aabox2i_t& b) {
      assert(i>0);
      assert(i<mpi::process_count());
      mpi_main_worker_box_[i] = b;
    }
     
    void mpi_quad_builder::mpi_main_mark_idle(std::size_t i) {
      assert(i>0);
      assert(i<mpi::process_count());
      aabox2i_t empty_box;
      empty_box.to_empty();
      mpi_main_worker_box_[i] = empty_box;
    }
   
    std::size_t mpi_quad_builder::mpi_main_busy_worker_count() const {
      std::size_t result = 0;
      for (int i=1; i<int(mpi_main_worker_box_.size()); ++i) {
	if (!mpi_main_worker_box_[i].is_empty()) {
	  ++result;
	}      
      }
      return result;
    }

    double mpi_quad_builder::mpi_main_worker_load() const {
      return double(mpi_main_busy_worker_count())/double(mpi_main_worker_box_.size()-1);
    }

    int  mpi_quad_builder::mpi_main_worker_for(const aabox2i_t& b) const {
      int result = -1;
      // Find first empty box
      for (int i=1; 
	   ((i<int(mpi_main_worker_box_.size())) && 
	    (result<0)); 
	   ++i) {
	if (mpi_main_worker_box_[i].is_empty()) {
	  result = i;
	}
      }

      // Check if there is a conflict, i.e., 
      // a worker already assigned to a box overlapping with
      // given one
      if (result>0) {
	for (int i=1;
	     ((i<int(mpi_main_worker_box_.size())) && 
	      (result>0)); 
	     ++i) {
	  const aabox2i_t& other_b = mpi_main_worker_box_[i];
	  bool disjoint = 
	    (other_b[0][0]>b[1][0]) ||
	    (other_b[1][0]<b[0][0]) ||
	    (other_b[0][1]>b[1][1]) ||
	    (other_b[1][1]<b[0][1]);
	  if (!disjoint) {
	    result = -1;
	  }	    
	}
      }

      return result;
    }

    void mpi_quad_builder::mpi_main_process() {
      std::cout << "----------------------------------------------------------------------------------------" << std::endl;
      std::cout << "Generating quads from tile chunks" << std::endl;
      std::cout << "----------------------------------------------------------------------------------------" << std::endl;
      sl::real_time_clock process_clock;
      process_clock.restart();
      mpi_main_init_worker_boxes();

      const std::size_t chunk_count = mpi_tile_chunks_.size();
      std::size_t send_count = 0;
      std::size_t receive_count = 0;

      std::size_t processed_quads = 0;

      bool done = receive_count==chunk_count;
      bool aborted = false;
      while (!done && !aborted) {

	// Try sending
	bool all_workers_busy = send_count>=chunk_count;
	if (send_count<chunk_count) {
	  // We have still a chunk to process, check if
	  // there is a worker able to deal with it

	  int lookahead = 0;
	  int worker = mpi_main_worker_for(mpi_tile_chunks_[send_count+lookahead].quad_box_xy());
	  
	  while ((worker<0) && 
		 (lookahead<16) &&
		 (send_count+lookahead+1<chunk_count)) {
	    ++lookahead;
	    worker = mpi_main_worker_for(mpi_tile_chunks_[send_count+lookahead].quad_box_xy());
	    if (worker>0) {
	      // FOUND, MOVE TO FIRST!
	      std::swap(mpi_tile_chunks_[send_count+lookahead],
			mpi_tile_chunks_[send_count]);
	    } 
	  }

	  if (worker>0) {

	    const mpi_tile_chunk& ck = mpi_tile_chunks_[send_count];

	    // OK, we can send
	    mpi::send(worker, 
		      MPI_TAG_GENERATE_QUADS_FROM_TILE,
		      ck);
	    mpi_main_assign_box(worker, ck.quad_box_xy());
	    ++send_count;

	    // Dump stats
	    int quad_count = 
	      (ck.quad_box_xy()[1][0]+1-ck.quad_box_xy()[0][0])*
	      (ck.quad_box_xy()[1][1]+1-ck.quad_box_xy()[0][1]);

	    sl::time_duration t_current = process_clock.elapsed();
	    double us_per_tile = double(t_current.as_microseconds())/double(receive_count+1);
	    sl::time_duration t_remaining = sl::time_duration(sl::uint64_t(us_per_tile*(chunk_count-receive_count-1)));

	    processed_quads += quad_count;
	    sl::uint64_t pixels_per_second = sl::uint64_t((1000.0*processed_quads*quad_width_*quad_height_) / double(1+t_current.as_milliseconds()));

	    std::cout << "[" << std::setw(4) << send_count << "/" << std::setw(4) << chunk_count << "] => " <<
	      "Worker: " << std::setw(2) << worker << 
	      " L= " << std::setw(2) << ck.tile_level() << 
	      " -- LOAD: " << std::setw(5) << sl::human_readable_percent(100.0*mpi_main_worker_load()) << 
	      " -- N Quads: " << std::setw(4) << quad_count << 
	      " -- T: " << std::setw(10) << sl::human_readable_duration(t_current) <<
	      " -- ETA: " << std::setw(10) << sl::human_readable_duration(t_remaining) << 
	      " -- TOTAL: " << std::setw(10) << sl::human_readable_duration(build_clock_.elapsed()) <<
	      " -- Speed: " << std::setw(6) << sl::human_readable_quantity(pixels_per_second) << "pixels/s" <<
	      std::endl;
	  } else {
	    all_workers_busy = true;
	  }
	}

	// Try receiving
	std::pair<bool, std::pair<int,int> > has_source_tag = 
	  (all_workers_busy ? 
	   std::make_pair(true, mpi::probe()) :
	   mpi::nonblocking_probe());
	if (has_source_tag.first) {
	  int src = has_source_tag.second.first;
	  int tag    = has_source_tag.second.second;
	  switch (tag) {

	  case MPI_TAG_SIGNAL_QUAD_DAMAGED: {
	    int l, x, y;
	    mpi::receive(src, tag, l, x, y);
	    damage(l, x, y);
	  } break;
	  
	  case MPI_TAG_SIGNAL_WORK_DONE: {
	    int n=0;
	    mpi::receive(src, tag, n);
	    mpi_main_mark_idle(src);
	    receive_count+=n;
	  } break;

	  case MPI_TAG_ERROR: {
	    std::string str;
	    mpi::receive(src, tag, str);

	    std::cerr << "ERROR: Error received from " << src << ": " << str << std::endl;
	    aborted = true;
	  } break;

	  default: {
	    std::cerr << "ERROR: Unknown message received from " << src << ": TAG=" << tag << std::endl;
	    aborted = true;
	  } break;
	  }
	} // if has message

	// Verify if we processed all tiles
	done = (receive_count == chunk_count);
      } // while 

      if (aborted) {
	// Cleanup
	std::cerr << "ERROR: ############ ABORTED!!! NO PARENT NODES WILL BE REPAIRED" << std::endl;
	for (std::size_t i=0; i<damaged_nodes_.size(); ++i) {
	  if (damaged_nodes_[i]) delete damaged_nodes_[i];
	  damaged_nodes_[i] = 0;
	}
	damaged_nodes_.clear();
      }
      std::cout << "----------------------------------------------------------------------------------------" << std::endl;
      std::cerr << "INFO: Input reprojection and quad generation took " << sl::human_readable_duration(process_clock.elapsed()) << std::endl;
    }

    // =========================================================

    void mpi_quad_builder::mpi_main_repair_level(int level) {
      std::cout << "----------------------------------------------------------------------------------------" << std::endl;
      std::cout << "Rebuilding quads at level " << level << " from child values" << std::endl;
      std::cout << "----------------------------------------------------------------------------------------" << std::endl;
      sl::real_time_clock repair_clock;
      repair_clock.restart();

      int next_unemployed_worker = 1;
      
      const sorted_xy_set_t& damaged_nodes_set = *(damaged_nodes_[level]);
      const std::size_t damaged_count = damaged_nodes_set.size();
      std::cout << "Extracting " <<  damaged_count << " damaged  nodes..." << std::endl;

      // Move to vector
      std::vector<sl::tuple2i> level_damaged_nodes;
      level_damaged_nodes.reserve(damaged_count);
      for (sorted_xy_set_t::const_iterator it = damaged_nodes_set.begin(); 
	   it != damaged_nodes_set.end();
	   ++it) {
	level_damaged_nodes.push_back(*it);
      }
      delete damaged_nodes_[level];
      damaged_nodes_[level] = 0;

      std::cout << "Done." << std::endl;

      // Send to workers and repair

      // Send N_per_batch requests to worker
      const std::size_t N_per_batch = sl::median(std::size_t(16),
						 damaged_count/mpi::process_count(),
						 std::size_t(2048));

      const std::size_t report_period = 1; // FIXME

      std::size_t send_count = 0;
      std::size_t receive_count = 0;

      bool done = (receive_count==damaged_count);
      bool aborted = false;
      while (!done && !aborted) {

	int worker = -1;
	if (send_count < damaged_count && 
	    next_unemployed_worker<int(mpi::process_count())) {
	  worker = next_unemployed_worker;
	  ++next_unemployed_worker;
	} else {
	  // All workers are busy, wait
	  int src;
	  int tag;
	  sl::tie(src,tag) = mpi::probe(MPI_ANY_SOURCE); // FIXME - handle timeouts?
	  switch (tag) {
	  case MPI_TAG_SIGNAL_WORK_DONE: {
	    int n;
	    mpi::receive(src, tag, n);
	    worker = src;
	    receive_count+=n;

	    if ((receive_count%report_period==0) || (receive_count==std::size_t(n)) || (receive_count==damaged_count)) {

	      sl::time_duration t_current = repair_clock.elapsed();
	      double us_per_tile = double(t_current.as_microseconds())/double(receive_count);
	      sl::time_duration t_remaining = sl::time_duration(sl::uint64_t(us_per_tile*(damaged_count-receive_count-1)));

	      sl::uint64_t pixels_per_second = sl::uint64_t((1000.0*receive_count*quad_width_*quad_height_) / double(1+t_current.as_milliseconds()));
	      
	      std::cout << "  [" << std::setw(4) << receive_count << "/" << std::setw(4) << damaged_count << "] <= " <<
		"Worker:" << std::setw(2) << worker << 
		" -- T: " << std::setw(10) << sl::human_readable_duration(t_current) <<
		" -- ETA: " << std::setw(10) << sl::human_readable_duration(t_remaining) << 
		" -- TOTAL: " << std::setw(10) << sl::human_readable_duration(build_clock_.elapsed()) <<
		" -- Speed: " << std::setw(10) << sl::human_readable_quantity(pixels_per_second) << "pixels/s" <<
		std::endl;
	    }  
	  } break;
	   
	  case MPI_TAG_ERROR: {
	    std::string str;
	    mpi::receive(src, tag, str);

	    std::cerr << "ERROR: Error received from " << src << ": " << str << std::endl;
	    aborted = true;
	  } break;
	    
	  default: {
	    std::cerr << "ERROR: Unknown message received from " << src << ": TAG=" << tag << std::endl;
	    aborted = true;
	  } break;
	  }
	}
	
	/// Send
	if (!aborted && (send_count<damaged_count)) {
	  if (worker > 0 && worker<mpi::process_count()) {
	    // Send N_per_batch to worker and mark parents damaged
	    std::vector<point3i_t> worker_level_x_y;
	    worker_level_x_y.reserve(N_per_batch);
	    for (std::size_t i=0; 
		 (i<N_per_batch) &&(send_count<damaged_count); 
		 ++i) {
	      const sl::tuple2i& xy=level_damaged_nodes[send_count];
	      worker_level_x_y.push_back(point3i_t(level, xy[0], xy[1]));
	      super_t::damage(level, xy[0], xy[1]);
	      ++send_count;
	    }
	    mpi::send(worker, MPI_TAG_GENERATE_QUADS_FROM_CHILDREN, worker_level_x_y);
	  } else {
	    std::cerr << "ERROR: No worker available!! ABORTING."<< std::endl;
	    // abort!!!
	    aborted = true;
	  }
	}

	done = (receive_count == damaged_count);
      }
      std::cerr << "INFO: Rebuilding level " << level << " took " << sl::human_readable_duration(repair_clock.elapsed()) << std::endl;
    }

    void mpi_quad_builder::mpi_worker_process_requests() {
      SL_TRACE_OUT(1) << "WORKER # " << mpi::process_rank() << " waiting for command..." << std::endl;
      bool running = true;
      bool aborted = false;
      while (running && !aborted) {
	SL_TRACE_OUT(1) << "WORKER # " << mpi::process_rank() << " waiting for command..." << std::endl;
	int src;
	int tag;
	sl::tie(src,tag) = mpi::probe(MPI_ANY_SOURCE); // FIXME - handle timeouts?
	switch (tag) {
      
	case MPI_TAG_DIE: {
	  SL_TRACE_OUT(1) << "WORKER # " << mpi::process_rank() << " processing DIE command..." << std::endl;
	  bool x;
	  mpi::receive(src, tag, x);
	  running = false;
	} break;

	case MPI_TAG_PROCNAME: {
	  SL_TRACE_OUT(1) << "WORKER # " << mpi::process_rank() << " processing PROCNAME command..." << std::endl;
	  std::string master_procname;
	  mpi::receive(src, tag, master_procname);
	  mpi::send(src, tag, mpi::processor_name());
	} break;
	
	case MPI_TAG_GENERATE_QUADS_FROM_TILE: {
	  SL_TRACE_OUT(1) << "WORKER # " << mpi::process_rank() << " start processing GENERATE_QUADS_FROM_TILE command..." << std::endl;
	  mpi_tile_chunk ck;
	  mpi::receive(src, tag, ck);
	  generate_quads_from_tile(ck.tile_fname(),
				   ck.tile_warp_xform(),
				   (int)ck.tile_level(),
				   ck.quad_box_xy(),
				   !ck.is_split());

	  mpi::send(src, MPI_TAG_SIGNAL_WORK_DONE, int(1));
	  SL_TRACE_OUT(1) << "WORKER # " << mpi::process_rank() << " end processing GENERATE_QUADS_FROM_TILE command..." << std::endl;
	} break;

	case MPI_TAG_GENERATE_QUADS_FROM_CHILDREN: {
	  SL_TRACE_OUT(1) << "WORKER # " << mpi::process_rank() << " start processing GENERATE_QUADS_FROM_CHILDREN command..." << std::endl;
	  std::vector<point3i_t> level_x_y;
	  mpi::receive(src, tag,
		       level_x_y);
 
	  std::size_t mkpath_count = 0;
	  std::string previous_path = "UnKnOwN";
	  std::size_t n = level_x_y.size();
	  for (std::size_t i=0; i<n; ++i) {

	    std::string this_path = sl::pathname_directory(qaccessor_->quad_filename(level_x_y[i][0],
										     level_x_y[i][1],
										     level_x_y[i][2]));
	    if (this_path != previous_path) {
	      if (!geo_utility::mkpath(this_path)) std::cerr << "WARNING: Unable to create path: " << this_path << std::endl;
	      previous_path = this_path;
	      ++mkpath_count;
	    }

	    generate_quad_from_children(level_x_y[i][0], 
					level_x_y[i][1],
					level_x_y[i][2],
					true);
	  }
	  SL_TRACE_OUT(1) << "mkpath: " << mkpath_count << "/" << n << std::endl;
	  mpi::send(src, MPI_TAG_SIGNAL_WORK_DONE, int(n));
	  SL_TRACE_OUT(1) << "WORKER # " << mpi::process_rank() << " end processing GENERATE_QUADS_FROM_CHILDREN command..." << std::endl;
	} break;

	default: {
	  SL_TRACE_OUT(1) << "WORKER # " << mpi::process_rank() << " processing UNKNOWN command: TAG= " << tag << " -- aborting" << std::endl;
	  mpi::send(src,MPI_TAG_ERROR, std::string("Received unknown command"));
	  running = false;
	  aborted = true;
	} break;

	} // switch
      } // while running && !aborted
    }


  }

}
