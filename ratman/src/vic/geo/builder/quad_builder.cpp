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
#include <vic/geo/builder/quad_builder.hpp>
#include <vic/geo/builder/geo_utility.hpp>
#include <vic/geo/builder/quad_warper.hpp>
#include <vic/geo/builder/quad_accessor.hpp>
#include <vic/geo/builder/quad_processor.hpp>
#include <vic/geo/builder/geo_transform.hpp>
#include <vic/geo/builder/color_remap_transform.hpp>
#include <sl/utility.hpp>
#include <sl/clock.hpp>
#include <sys/time.h>

#include <cassert>

// GDAL include
#include <gdal_priv.h>
#include <cpl_string.h>

// ------ opendir, readdir
#include <sys/types.h>
#include <dirent.h>

namespace vic {
  namespace geo {

    quad_builder::quad_builder() :
      last_operation_success_(true) {

      qaccessor_= new quad_accessor();
      qprocessor_= new quad_processor();
      qprocessor_->set_min_data_value(10); // FIXME???
      qwarper_ = new quad_warper();

      color_remap_black_out_ = 0;
      color_remap_black_in_ = 0;
      color_remap_white_out_ = 255;
      color_remap_white_in_ = 255;
      color_remap_below_to_black_ = 0;
      color_remap_above_to_black_ = -1;
      
      damaged_level_min_=-1;
      damaged_level_max_=-1;

      max_incore_tile_memory_ = 640*1024*1024; // FIXME

      out_quad_index_enabled_ = true; // FIXME

      set_quadtree_output_format();
      set_quad_size(256,256);
      set_quad_band_count(3);
      set_quadtree_root_count(4,2);
      set_quad_warp_max_error(0.2); // FIXME
      set_quadtree_projection("EPSG:4326");
      set_quadtree_extent(-180.0, -90.0, 180.0, 90.0);
    }

    quad_builder::~quad_builder() {
      if (qaccessor_) delete qaccessor_;
      qaccessor_=0;
      if (qprocessor_) delete qprocessor_;
      qprocessor_=0;
      if (qwarper_) delete qwarper_;
      qwarper_=0;
    }

    std::string quad_builder::out_quad_index_fname() const {
      return qaccessor_->root_dir() + "/" + "quad_index_000.log";
    }

    void quad_builder::out_quad_index_begin() {
      if (out_quad_index_enabled()) {
        out_quad_index_file_.open(out_quad_index_fname().c_str());
	if (!out_quad_index_file_) {
	  std::cerr << "ERROR: Quad index log file " << out_quad_index_fname() << " not open!" << std::endl;
	}
      }
    }
    
    void quad_builder::out_quad_index_end() {
      if (out_quad_index_enabled()) {
        out_quad_index_file_.close();
      }
    }

    void quad_builder::out_quad_index_write(int l, int x, int y) {
      if (out_quad_index_file_) {
	struct timeval tv;
	gettimeofday(&tv, 0);
	uint64_t timestamp = uint64_t(tv.tv_sec)*1000000 + uint64_t(tv.tv_usec);
 
	out_quad_index_file_ << timestamp << '\t' << l << '\t' << x << '\t' << y << std::endl;
      }
    }

    void quad_builder::set_quadtree_output_format(const std::string& format,
						  const std::string& extension,  
						  const std::vector<std::string>& opts) {
      qaccessor_->set_file_extension(extension); 
      qaccessor_->set_output_format(format,opts); 
    }

    void quad_builder::set_quadtree_extent( double x0, double y0, double x1, double y1) {
      quadtree_extent_ = sl::aabox2d(sl::point2d(x0, y0),
				     sl::point2d(x1, y1));
    }

    void quad_builder::set_quadtree_root_count(std::size_t nx, std::size_t ny) {
      qaccessor_->set_quadtree_root_count(nx,ny);
    }

    void quad_builder::set_default_src_proj(const std::string& proj) {
      default_src_srs_=geo_utility::proj2srs(proj);
    }

    void quad_builder::set_quadtree_projection(const std::string& quad_proj) {
      // FIXME CHECK ERROR?
      quad_srs_=geo_utility::proj2srs(quad_proj);
    }

    void quad_builder::set_quadtree_root_dir(const std::string& root_dir) {
      qaccessor_->set_root_dir(root_dir);
    }

    void quad_builder::begin_processing() {
      std::cout << "============== QUAD BUILDER: starting" << std::endl;
      input_tile_fname_level_.clear();
      for (std::size_t i=0; i<damaged_nodes_.size(); ++i) {
	if (damaged_nodes_[i]) delete damaged_nodes_[i];
	damaged_nodes_[i] = 0;
      }
      damaged_nodes_.clear();
      identify_damaged_nodes();

      out_quad_index_begin();
    }

    void quad_builder::process_tile(const std::string& tile_fname, int level) {
      input_tile_fname_level_.push_back(std::make_pair(tile_fname, level));
    }

    void quad_builder::tile_parameters_in(std::size_t tile_idx,
					  int& tile_level, 
					  geo_transform& tile_warp_xform, 
					  aabox2i_t& quad_box_xy) {
      std::string tile_fname = input_tile_fname_level_[tile_idx].first;
      
      tile_level = input_tile_fname_level_[tile_idx].second;

      GDALDataset* tile = reinterpret_cast<GDALDataset*>(GDALOpen(tile_fname.c_str(), GA_ReadOnly));
      if (!tile) {
	std::cerr << "Unable to open: " << tile_fname << std::endl;
	tile_level = -1;
      } else {
	// Read srs and xform
	std::string tile_srs = default_src_srs_;
	if (std::string(tile->GetProjectionRef()) != "") {
	  tile_srs = tile->GetProjectionRef();
	}
	double tile_mat[6];
	tile->GetGeoTransform(tile_mat);
	geo_matrix tile_xform(tile_mat);
		
	// Parameterize warping
	tile_warp_xform = geo_transform(tile_srs, 
					quad_srs_, 
					quad_warp_max_error_);
	tile_warp_xform.set_src_geo_matrix(tile_xform); 
	geo_matrix identity_xform;
	tile_warp_xform.set_dst_geo_matrix(identity_xform); // Set to indenty
	
	// Estimate warped projection extent
	double mat[6];
	int warped_width;
	int warped_height;
	GDALSuggestedWarpOutput(tile, 
				tile_warp_xform.get_trasformation(), 
				tile_warp_xform.to_pointer(),
				mat,
				&warped_width,
				&warped_height);
	delete tile;

	double pixel_to_world_x  = mat[1];
	double pixel_to_world_y  = mat[5];
	  
	// Estimate level if not imposed
	if (tile_level<0) {
#if 0
	  double world_tile_width  = fabs(pixel_to_world_x) * quad_width_ * qaccessor_->quadtree_root_count(0);
	  double world_tile_height = fabs(pixel_to_world_y) * quad_height_ * qaccessor_->quadtree_root_count(1);
	  double level_x = log(world_tile_width / quadtree_extent_.diagonal()[0])/log(0.5);
	  double level_y = log(world_tile_height / quadtree_extent_.diagonal()[1])/log(0.5);
	    
	  tile_level = sl::max(0,int(0.5 + (level_x + level_y)/2.0));
#else
	  tile_level = 0;
	  double tile_pixel_size = 0.5*(fabs(pixel_to_world_x)+fabs(pixel_to_world_y));
	  
	  bool accurate_enough = false;
	  do {
	    double level_pixel_size = 0.5*(quadtree_extent_.diagonal()[0]/(quad_width_ * qaccessor_->level_quad_count(tile_level, 0))+
					   quadtree_extent_.diagonal()[1]/(quad_height_ * qaccessor_->level_quad_count(tile_level, 1)));
	    double next_level_pixel_size = 0.5*level_pixel_size;
	    
	    accurate_enough = 
	      (level_pixel_size<=tile_pixel_size) ||
	      ((next_level_pixel_size<= tile_pixel_size) &&
	       (fabs(level_pixel_size-tile_pixel_size) <= fabs(next_level_pixel_size-tile_pixel_size)));

	    if (!accurate_enough) {
	      ++tile_level;
	    }
	  } while (!accurate_enough);
#endif
	}

	
	// Estimate tile bbox
	std::size_t level_quad_count_x = qaccessor_->level_quad_count(tile_level,0);
	std::size_t level_quad_count_y = qaccessor_->level_quad_count(tile_level,1);
	
	double quad_world_width = quadtree_extent_.diagonal()[0] / double(level_quad_count_x);
	double quad_world_height = quadtree_extent_.diagonal()[1] / double(level_quad_count_y);
	
	double warped_u0 = mat[0] - quadtree_extent_[0][0];
	double warped_v0 = mat[3] - quadtree_extent_[0][1];
	double warped_u1 = warped_u0 + pixel_to_world_x*warped_width;
	double warped_v1 = warped_v0 + pixel_to_world_y*warped_height;
	
	sl::aabox2d warped_box;
	warped_box.to(sl::point2d(warped_u0,warped_v0));
	warped_box.merge(sl::point2d(warped_u1,warped_v1));
	  
	int quad_x0 = sl::median(int(0),int(level_quad_count_x)-1, int(floor(warped_box[0][0] / quad_world_width)));
	int quad_y0 = sl::median(int(0),int(level_quad_count_y)-1, int(floor(warped_box[0][1] / quad_world_height)));
	int quad_x1 = sl::median(int(0),int(level_quad_count_x)-1, int( ceil(warped_box[1][0] / quad_world_width)));
	int quad_y1 = sl::median(int(0),int(level_quad_count_y)-1, int( ceil(warped_box[1][1] / quad_world_height)));

	quad_box_xy = aabox2i_t(point2i_t(quad_x0, quad_y0),
				point2i_t(quad_x1, quad_y1));
      }
    }

    void quad_builder::process() {
      std::cout << "-------------- inserting input tiles" << std::endl;
      std::size_t tile_count = input_tile_fname_level_.size();
      for (std::size_t tile_idx=0; tile_idx<tile_count; ++tile_idx) {
	const std::string& tile_fname = input_tile_fname_level_[tile_idx].first;
	int                tile_level = input_tile_fname_level_[tile_idx].second;

	geo_transform tile_warp_xform;
	aabox2i_t     quad_box_xy;

	tile_parameters_in(tile_idx, 
			   tile_level,
			   tile_warp_xform,
			   quad_box_xy);

	if (tile_level>=0) {
	  int quad_count = 
	    (quad_box_xy[1][0]+1-quad_box_xy[0][0])*
	    (quad_box_xy[1][1]+1-quad_box_xy[0][1]);

	  std::cout << "[" << tile_idx+1 << "/" << tile_count << "] => L= " << tile_level << " N Quads: " << quad_count << std::endl;
	 
	  generate_quads_from_tile(tile_fname, 
				   tile_warp_xform, 
				   tile_level, 
				   quad_box_xy,
				   true);
	}
      }
    }

    void quad_builder::process_directory(const std::string& dirname,
					 const std::string& pattern,
					 int level) {
      DIR* dir = opendir(dirname.c_str());
      if (!dir) {
        std::cerr << "QUAD_BUILDER: Unable to open directory: " << dirname << std::endl;
      } else {
	std::cout << "QUAD_BUILDER: Scanning directory: " << dirname << " for files matching " << pattern << std::endl;
        struct dirent* d;
        while ((d=readdir(dir))!=0) {
          std::string fname = std::string(d->d_name);
          if (sl::matches(fname, pattern)) {
            // FIXME CREATE reader...
            std::string fullname = dirname + "/" + fname;
	    process_tile(fullname, level);
          }
        }
        closedir(dir);
      }
    }

    void quad_builder::end_processing() {
      process();
      repair();
      out_quad_index_end();

      for (std::size_t i=0; i<damaged_nodes_.size(); ++i) {
	if (damaged_nodes_[i]) delete damaged_nodes_[i];
	damaged_nodes_[i] = 0;
      }
      damaged_nodes_.clear();

      std::cout << "============== QUAD BUILDER: done" << std::endl;
    }
   
    void quad_builder::identify_damaged_nodes() {
      DIR* rootdir = opendir(qaccessor_->root_dir().c_str());
      if (rootdir) {
	struct dirent* leveld;
	while ((leveld=readdir(rootdir))!=0) {
	  std::string levelfname = std::string(leveld->d_name);
	  int level;
	  if (sscanf(levelfname.c_str(), "%d", &level) == 1) {
	    if (level >= damaged_level_min_ && level <= damaged_level_max_) {
	      std::string leveldir_name=qaccessor_->root_dir()+"/"+levelfname;
	      DIR* leveldir = opendir(leveldir_name.c_str());
	      if (leveldir) {
		struct dirent* rowd;
		while ((rowd=readdir(leveldir))!=0) {
		  std::string rowfname = std::string(rowd->d_name);
		  int y0;
		  if (sscanf(rowfname.c_str(), "%d", &y0) == 1) {
		    std::string rowdir_name=leveldir_name+"/"+rowfname;
		    DIR* rowdir = opendir(rowdir_name.c_str());
		    if (rowdir) {
		      struct dirent* quadd;
		      while ((quadd=readdir(rowdir))!=0) {
			std::string quadfname = std::string(quadd->d_name);
			int x;
			int y;
			if (sscanf(quadfname.c_str(), "%d_%d", &x, &y) == 2) {
			  std::size_t level_quad_count_x = qaccessor_->level_quad_count(level,0);
			  std::size_t level_quad_count_y = qaccessor_->level_quad_count(level,1);
			  if (y==y0 &&
			      y>=0 && y<int(level_quad_count_y) &&
			      x>=0 && x<int(level_quad_count_x)) {
			    damage(level,x,y);
			  }
			}
		      }
		      closedir(rowdir);
		    }
		  }
		}
		closedir(leveldir);
	      }
	    }
	  }
	}
	closedir(rootdir);
      }
    }

    void quad_builder::damage(int l, int x, int y) {
      if (l>0) {
	// Mark parent needs rebuild
	sl::tuple2i xy;
	int level=l-1;
	xy[0]=x/2;
	xy[1]=y/2;
	while (int(damaged_nodes_.size())<=level) {
	  damaged_nodes_.push_back(new sorted_xy_set_t());
	}

	damaged_nodes_[level]->insert(xy);
      }
    }

    void quad_builder::generate_quads_from_tile(const std::string& tile_fname, 
						const geo_transform& tile_warp_xform, 
						int level, 
						const aabox2i_t& quad_box_xy,
						bool try_load_in_memory) {
      assert(level>=0);
	
      sl::real_time_clock performance_clock;
	
      // Allocate working memory
      GDALDataset* quad = GetGDALDriverManager()->GetDriverByName("MEM")->Create("Quad",
										 quad_width_,
										 quad_height_,
										 quad_band_count_,
										 GDT_Byte,
										 0);
      if (!quad) {
	std::cerr << "ERROR GENERATE QUADS FROM TILE: " <<  tile_fname << std::endl;
	std::cerr << "Unable to allocate memory for quad" << std::endl;
	return;
      }

      // Open input dataset 
      GDALDataset* tile_in = reinterpret_cast<GDALDataset*>(GDALOpen(tile_fname.c_str(), GA_ReadOnly));
      if (!tile_in) {
	std::cerr << "ERROR GENERATE QUADS FROM TILE: " <<  tile_fname << std::endl;
	std::cerr << "Unable to open tile: " << tile_fname << std::endl;
	
	delete quad;
	return;
      }


      // Copy to memory if asked to do so
      GDALDataset *tile = tile_in;
      std::size_t tile_x = tile->GetRasterXSize();
      std::size_t tile_y = tile->GetRasterYSize();
      std::size_t tile_z = tile->GetRasterCount();
      std::size_t tile_Bpp = (GDALGetDataTypeSize(tile->GetRasterBand(1)->GetRasterDataType()) + 7)/8;

      std::size_t tile_sz = tile_x * tile_y * tile_z * tile_Bpp;
      SL_TRACE_OUT(1) << 
	"IMG: " <<  tile_x << "x" << tile_y << "x" << tile_z << "x" << tile_Bpp << 
	" SZ: " << sl::human_readable_size(tile_sz) << std::endl;

      bool global_nodata_remapping = false;
      if (try_load_in_memory && (tile_sz < max_incore_tile_memory())) {
	/**/performance_clock.restart();
	/**/SL_TRACE_OUT(1) << "BENCH: Loading dataset of size " << sl::human_readable_size(tile_sz) << "..." << std::endl;
	SL_TRACE_OUT(1) << "CACHE BEFORE: " << sl::human_readable_size(GDALGetCacheUsed()) << std::endl;
	tile =  GetGDALDriverManager()->GetDriverByName("MEM")->CreateCopy("MEM",
									   tile_in,
									   0,
									   0,
									   GDALDummyProgress,
									   0);
	if (tile) {
	  delete tile_in;

	  // TILE IS IN MEMORY, TRY TO PROCESS GLOBALLY
	  qprocessor_->global_remap_nodata_to_black(tile, 
						    color_remap_black_out_,
						    color_remap_black_in_,
						    color_remap_white_out_,
						    color_remap_white_in_,
						    color_remap_below_to_black_,
						    color_remap_above_to_black_);
	  global_nodata_remapping = true;	  
	} else {
	  std::cerr << "WARNING GENERATE QUADS FROM TILE: " <<  tile_fname << std::endl;
	  std::cerr << "Unable to move tile incore - will continue processing off-core" << std::endl;
	}
	/**/if (performance_clock.elapsed().as_milliseconds() > 500) SL_TRACE_OUT(1) << "BENCH: Load dataset of size " << sl::human_readable_size(tile_sz) << " T= " << sl::human_readable_duration(performance_clock.elapsed()) << std::endl;

	SL_TRACE_OUT(1) << "CACHE AFTER: " << sl::human_readable_size(GDALGetCacheUsed()) << std::endl;
      }

      // Process

      color_remap_transform color_xform;
      if (!global_nodata_remapping) {
	// Tile per tile remapping
	color_xform = color_remap_transform(color_remap_black_out_,
					    color_remap_black_in_,
					    color_remap_white_out_,
					    color_remap_white_in_,
					    color_remap_below_to_black_,
					    color_remap_above_to_black_);
      }

      qwarper_->reset_error();
      geo_transform warper_tile_warp_xform(tile_warp_xform);
      qwarper_->create(tile, quad, warper_tile_warp_xform, color_xform);
      if (!qwarper_->last_operation_success()) {
	std::cerr << "ERROR GENERATE QUADS FROM TILE: " <<  tile_fname << std::endl;
	std::cerr << last_error_message() << std::endl;
      } else {
	std::size_t level_quad_count_x = qaccessor_->level_quad_count(level,0);
	std::size_t level_quad_count_y = qaccessor_->level_quad_count(level,1);
	
	double quad_world_width = quadtree_extent_.diagonal()[0] / double(level_quad_count_x);
	double quad_world_height = quadtree_extent_.diagonal()[1] / double(level_quad_count_y);
	
	int quad_x0 = quad_box_xy[0][0];
	int quad_y0 = quad_box_xy[0][1];
	int quad_x1 = quad_box_xy[1][0];
	int quad_y1 = quad_box_xy[1][1];
	int quad_count = (quad_x1-quad_x0+1)*(quad_y1-quad_y0+1);

	std::size_t mkpath_count = 0;
	std::string previous_path = "UnKnOwn";

	// Loop on y, then x to try being more coherent
	// for scanline oriented files... 
	for (int y = quad_y0; y <= quad_y1; ++y) {
	  for (int x = quad_x0; x <= quad_x1; ++x) {
	    
	    for (int i = 1; i <= quad_band_count_; ++i) {
	      quad->GetRasterBand(i)->Fill(0.0); 
	    }
	    qwarper_->set_dst_geo_matrix(geo_matrix(quadtree_extent_[0][0] + x * quad_world_width, 
						    quadtree_extent_[0][1] + (y+1) * quad_world_height, 
						    quad_world_width / quad_width_,
						    -quad_world_height / quad_height_)); 
	    
	    
	    /**/performance_clock.restart();
	    
	    qwarper_->reset_error();
	    qwarper_->chunk_and_warp_image(0, 0, quad_width_, quad_height_);
	    
	    /**/if (performance_clock.elapsed().as_milliseconds() > 500) SL_TRACE_OUT(1) << "BENCH: warping:" << x << " " << y << " T= " << sl::human_readable_duration(performance_clock.elapsed()) << std::endl;
	    
	    if (!qwarper_->last_operation_success()) {
	      std::cerr << "ERROR GENERATE QUADS FROM TILE: " <<  tile_fname << std::endl;
	      std::cerr << last_error_message() << std::endl;
	      std::cerr << "Quad l=" << level << " x=" << x << " y=" << y << " not warped" << std::endl;
	    } else {
	      // Combine and write
	      int non_null_pixel_count = 0;
	      int null_pixel_count = 0;
	      qprocessor_->compute_null_pixel_stats(quad, &null_pixel_count, &non_null_pixel_count);
	      bool is_fully_transparent = (non_null_pixel_count == 0);
	      bool is_fully_opaque      = (null_pixel_count == 0);
	      if (!is_fully_transparent) { 
		if (!is_fully_opaque) {
		  /**/performance_clock.restart();
		  GDALDataset *prev_quad = qaccessor_->read_to_memory(level, x, y);
		  if (prev_quad) {
		    qprocessor_->combine(quad, prev_quad, quad);
		    delete prev_quad;
		  } 
		  /**/if (performance_clock.elapsed().as_milliseconds() > 500) SL_TRACE_OUT(1) << "BENCH: combining:" << x << " " << y << " T= " << sl::human_readable_duration(performance_clock.elapsed()) << std::endl;
		}
		/**/performance_clock.restart();

		std::string this_path = sl::pathname_directory(qaccessor_->quad_filename(level, x, y));
		if (this_path != previous_path) {
		  SL_TRACE_OUT(1) << "mkpath (" << level << " " << x << " " << y << ") " << this_path << std::endl;
		  if (!geo_utility::mkpath(this_path)) std::cerr << "Unable to create path: " << this_path << std::endl;
		  previous_path = this_path;
		  ++mkpath_count;
		}

		qaccessor_->reset_error();
		qaccessor_->write(level, x, y, quad, true);
		out_quad_index_write(level, x, y); // LOG

		/**/if (performance_clock.elapsed().as_milliseconds() > 500) SL_TRACE_OUT(1) << "BENCH: writing:" << x << " " << y << " T= " << sl::human_readable_duration(performance_clock.elapsed()) << std::endl;
		
		if(!qaccessor_->last_operation_success()) {
		  std::cerr << "ERROR GENERATE QUADS FROM TILE: " <<  tile_fname << std::endl;
		  std::cerr << qaccessor_->last_error_message() << std::endl;
		}
		damage(level, x, y);
	      }
	    }
	    
	  }
	}

	SL_TRACE_OUT(1) << "mkpath: " << mkpath_count << "/" << quad_count << std::endl;

      }
      // End tiling
      qwarper_->destroy();
	
      // Cleanup
      delete quad;
      delete tile;
    }

    void quad_builder::generate_quad_from_children(int l, int x, int y, bool path_known_to_exist) {
      // Read children
      std::vector<GDALDataset *> children_quad;
      GDALDataset* non_null_child = 0;
      for(int i=1; i>=0; --i) { // FIXME
	for(int j=0; j<2; ++j) {
	  int l_ij = l+1;
	  int x_ij = x*2+j;
	  int y_ij = y*2+i;
	  qaccessor_->reset_error();
	  GDALDataset* child_ij = qaccessor_->read_to_memory(l_ij, x_ij, y_ij);
	  if(!qaccessor_->last_operation_success()) {
	    std::cerr << "ERROR GENERATE QUAD FROM CHILDREN: "  << l << " " << x << " " << y << std::endl;
	    std::cerr << qaccessor_->last_error_message() << std::endl;
	  }
	  children_quad.push_back(child_ij);
	  if (child_ij) non_null_child = child_ij;
	}
      }
	
      // Coarsen and merge
      if (!non_null_child) {
	std::cerr << "ERROR GENERATE QUAD FROM CHILDREN: " << l << " " << x << " " << y << std::endl;
	std::cerr << "  REBUILDING FROM NULL CHILDREN!!!" << std::endl;
      } else {
	GDALDataset *parent_quad=qprocessor_->create_from_template(non_null_child);
	
	qprocessor_->coarsen(parent_quad, children_quad);
	{
	  // Combine and write - note: we also write 
	  // null quads to erase previous data...
	  int non_null_pixel_count = 0;
	  int null_pixel_count = 0;
	  qprocessor_->compute_null_pixel_stats(parent_quad, &null_pixel_count, &non_null_pixel_count);
	  //bool is_fully_transparent = (non_null_pixel_count == 0);
	  bool is_fully_opaque = (null_pixel_count == 0);
	  if (!is_fully_opaque) {
	    GDALDataset *prev_quad = qaccessor_->read_to_memory(l, x, y);
	    if (prev_quad) {
	      qprocessor_->combine(parent_quad, prev_quad, parent_quad);
	      delete prev_quad;
	    } 
	  }
	  qaccessor_->reset_error();
	  qaccessor_->write(l,x,y,parent_quad, path_known_to_exist);
	  out_quad_index_write(l,x,y); // LOG
	  if(!qaccessor_->last_operation_success()) {
	    std::cerr << "ERROR GENERATE QUAD FROM CHILDREN: " << l << " " << x << " " << y << std::endl;
	    std::cerr << "  Write error:" << qaccessor_->last_error_message() << std::endl;
	  }
	}
	delete parent_quad;
      }

      // Cleanup
      for (std::size_t i=0; i<children_quad.size(); ++i) {
	if (children_quad[i]) delete children_quad[i];
      }
    }

    void quad_builder::repair() {
      std::cout << "-------------- rebuilding parent nodes" << std::endl;
      int level_count = int(damaged_nodes_.size());
      for (int l = level_count-1; l>=0; --l) {
	std::cout << "[" << l << "/" << level_count << "]" << " N Quads: " << damaged_nodes_[l]->size() << std::endl;
	repair_level(l);
      }
    }

    void quad_builder::repair_level(int level) {
      std::string previous_path = "UnKnOwN";

      while (!damaged_nodes_[level]->empty()) {
	sl::tuple2i xy=*(damaged_nodes_[level]->rbegin());
	damaged_nodes_[level]->erase(xy);

	std::string this_path = sl::pathname_directory(qaccessor_->quad_filename(level, xy[0], xy[1]));
	if (this_path != previous_path) {
	  if (!geo_utility::mkpath(this_path)) std::cerr << "Unable to create path: " << this_path << std::endl;
	  previous_path = this_path;
	}

	generate_quad_from_children(level, xy[0], xy[1], true);
	damage(level,xy[0],xy[1]);
      }
    }
         
  } // namespace geo
} // namespace vic
