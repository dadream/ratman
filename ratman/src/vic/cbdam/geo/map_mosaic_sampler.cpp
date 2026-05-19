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
#include <vic/cbdam/geo/map_mosaic_sampler.hpp>
#include <vic/cbdam/geo/map_raster_sampler.hpp>
#include <sl/dense_array.hpp>
#include <sl/utility.hpp>
#include <cassert>

// ------ opendir, readdir
#ifndef _WIN32
#include <sys/types.h>
#include <dirent.h>
#endif

namespace vic {

  namespace geo {

    map_mosaic_sampler::map_mosaic_sampler() {
      stat_loaded_sample_count_ = 0;
      tiles_.clear();
      bounding_rectangle_.to_empty();
      buckets_.resize(sl::index<2>(0,0));
      bucket_o_ = sl::point2d(0.0,0.0);
      bucket_duv_ = sl::vector2d(0.0,0.0);
      bucket_one_over_duv_ = sl::vector2d(0.0,0.0);
      tile_lru_.clear();
      tile_lru_capacity_ = 16; // FIXME?
    }

    map_mosaic_sampler::map_mosaic_sampler(const std::string& dirname,
                                           const std::string& pattern) {
      stat_loaded_sample_count_ = 0;
      tiles_.clear();
      bounding_rectangle_.to_empty();
      buckets_.resize(sl::index<2>(0,0));
      bucket_o_ = sl::point2d(0.0,0.0);
      bucket_duv_ = sl::vector2d(0.0,0.0);
      bucket_one_over_duv_ = sl::vector2d(0.0,0.0);
      tile_lru_.clear();
      tile_lru_capacity_ = 16;

      insert_directory(dirname, pattern);
    }
    
    map_mosaic_sampler::~map_mosaic_sampler() {
      // FIXME memory leak!
      tiles_.clear();
      bounding_rectangle_.to_empty();
      buckets_.resize(sl::index<2>(0,0));
      bucket_o_ = sl::point2d(0.0,0.0);
      bucket_duv_ = sl::vector2d(0.0,0.0);
      bucket_one_over_duv_ = sl::vector2d(0.0,0.0);
      tile_lru_.clear();
    }

    bool map_mosaic_sampler::is_empty() const {
      update();
      return bounding_rectangle_.is_empty();
    }

    void map_mosaic_sampler::set_verbose(bool x) {
      is_verbose_ = x;
      for (std::size_t i=0; i<tiles_.size(); ++i) {
	tiles_[i]->set_verbose(is_verbose_);
      }
    }

    void map_mosaic_sampler::minimize_footprint() const {
      // Minimize footprint of all tiles in lru cache, then empty cache
      for (std::list<std::size_t>::iterator it = tile_lru_.begin();
           it!= tile_lru_.end();
           ++it) {
        tiles_[*it]->minimize_footprint();
      }
      tile_lru_.clear();
    }

    std::size_t map_mosaic_sampler::tile_count() const {
      return tiles_.size();
    }

    void map_mosaic_sampler::insert_directory(const std::string& dirname,
                                              const std::string& pattern) {
#ifndef _WIN32
      DIR* dir = opendir(dirname.c_str());
      if (!dir) {
        std::cerr << "MOSAIC: Unable to open directory: " << dirname << std::endl;
      } else {
        if (is_verbose()) {
	  std::cerr << "MOSAIC: Scanning directory: " << dirname << " for files matching " << pattern << std::endl;
        }

	struct dirent* d;
        while ((d=readdir(dir))!=0) {
          std::string fname = std::string(d->d_name);
          if (sl::matches(fname, pattern)) {
            // FIXME CREATE reader...
            std::string fullname = dirname + "/" + fname;
            map_raster_sampler* tile = new map_raster_sampler();
	    tile->set_verbose(is_verbose_);
            tile->open(fullname);
            if (!tile->is_empty()) {
	      if (is_verbose()) {
		std::cerr << "MOSAIC:   Inserting: " << fullname << std::endl;
	      }
	      insert(tile);
            } else {
              std::cerr << "MOSAIC:   Unable to open: " << fullname << std::endl;
            }              
          }
        }
        closedir(dir);
      }
#else
      SL_TRACE_OUT(-1) << "READ DIR NOT SUPPORTED UNDER WINDOWS" << std::endl;
#endif
    }
    
    void map_mosaic_sampler::insert(map_sampler* tile) {
      if (!tiles_.empty() && band_count() != tile->band_count()) {
	std::cerr << "MOSAIC: Attempting to insert tile with wrong band count = " <<
	  tile->band_count() << " != " << band_count() << 
	  std::endl;
	std::cerr << "SKIPPING!" << std::endl;
      } else {
	minimize_footprint();
	
	// Clear spatial index
	bounding_rectangle_.to_empty();
	buckets_.resize(sl::index<2>(0,0));
	bucket_o_ = sl::point2d(0.0,0.0);
	bucket_duv_ = sl::vector2d(0.0,0.0);
	bucket_one_over_duv_ = sl::vector2d(0.0,0.0);
	
	// Insert tile
	tile->minimize_footprint();
	tile->set_verbose(is_verbose_);
	tiles_.push_back(tile);
      }
    }
    
    void map_mosaic_sampler::clear() {
      minimize_footprint();
      
      tiles_.clear();

      // Clear spatial index
      bounding_rectangle_.to_empty();
      buckets_.resize(sl::index<2>(0,0));
      bucket_o_ = sl::point2d(0.0,0.0);
      bucket_duv_ = sl::vector2d(0.0,0.0);
      bucket_one_over_duv_ = sl::vector2d(0.0,0.0);
    }
            
    std::size_t map_mosaic_sampler::band_count() const {
      // FIXME !
      if (tiles_.empty()) {
        return 0;
      } else {
        return tiles_[0]->band_count();
      }
    }
    
      
    bool map_mosaic_sampler::sample_in(int32_t* sample,
                                       double u, double v) const {
      // HACK
      update();

      ++stat_requested_sample_count_;

      bool result = false;
      
      int  bucket_i = int((u-bucket_o_[0])*bucket_one_over_duv_[0]);
      int  bucket_j = int((v-bucket_o_[1])*bucket_one_over_duv_[1]);
      if (bucket_i >= 0 && bucket_i < int(buckets_.extent()[0]) &&
          bucket_j >= 0 && bucket_j < int(buckets_.extent()[1])) {
        const std::list<std::size_t>& bucket_tiles = buckets_(bucket_i, bucket_j);
        for (std::list<std::size_t>::const_iterator tile_it = bucket_tiles.begin();
             (!result) && (tile_it != bucket_tiles.end());
             ++tile_it) {
          std::size_t k = *tile_it;
          const map_sampler* tile_k = tiles_[k];

	  sl::uint64_t tile_k_old_loaded_sample_count = tile_k->stat_loaded_sample_count(); 

          result = tile_k->sample_in(sample, u, v);
	  
	  sl::uint64_t delta_io = (tile_k->stat_loaded_sample_count() - 
				   tile_k_old_loaded_sample_count);

	  stat_loaded_sample_count_ += delta_io;

          // LRU update
          touch(k);
        }
      }
      return result;
    }

    
    map_mosaic_sampler::aabox_t map_mosaic_sampler::bounding_rectangle() const {
      update();
      return bounding_rectangle_;
    }
    
    
    double map_mosaic_sampler::minimum_sample_spacing(const aabox_t& b, double eps) const {
      assert(!b.is_empty());
      update();
      
      double result = b.diagonal().two_norm();
      
      int  i0 = std::max(0, int((b[0][0]-bucket_o_[0])*bucket_one_over_duv_[0]));
      int  j0 = std::max(0, int((b[0][1]-bucket_o_[1])*bucket_one_over_duv_[1]));
      int  i1 = std::min(int(buckets_.extent()[0])-1, int((b[1][0]-bucket_o_[0])*bucket_one_over_duv_[0]));
      int  j1 = std::min(int(buckets_.extent()[1])-1, int((b[1][1]-bucket_o_[1])*bucket_one_over_duv_[1]));
      
      for (int bucket_i=i0; bucket_i<=i1 && result>=eps; ++bucket_i) {
        for (int bucket_j=j0; bucket_j<=j1 && result>= eps; ++bucket_j) {
          const std::list<std::size_t>& bucket_tiles = buckets_(bucket_i, bucket_j);
          for (std::list<std::size_t>::const_iterator tile_it = bucket_tiles.begin();
               (tile_it != bucket_tiles.end());
               ++tile_it) {
            std::size_t k = *tile_it;
            const map_sampler* tile_k = tiles_[k];

            result = std::min(result, tile_k->minimum_sample_spacing(b));
          }
        }
      }

      return result; 
    }
    
    void map_mosaic_sampler::touch(std::size_t k) const {
      const std::list<std::size_t>::iterator lru_begin = tile_lru_.begin(); 
      const std::list<std::size_t>::iterator lru_end   = tile_lru_.end(); 
      std::list<std::size_t>::iterator lru_it = lru_begin; 
      while (lru_it != lru_end && (*lru_it) != k) {
        ++lru_it;
      }
      if (lru_it != lru_end) {
        // Already in lru
        if (lru_it != lru_begin) {
          // Move to front of lru
          tile_lru_.push_front(k);
          tile_lru_.erase(lru_it);
        }
      } else {
        // Not in lru, add it, possibly removing last one
        tile_lru_.push_front(k);
        SL_TRACE_OUT(0) << "Inserting " << k << " into cache" << std::endl;
        if (tile_lru_.size() > tile_lru_capacity_) {
          std::size_t k_oldest = tile_lru_.back();

          SL_TRACE_OUT(0) << "Moving " << k_oldest << " out of cache" << std::endl;
          tiles_[k_oldest]->minimize_footprint();
          tile_lru_.pop_back();
        }
      }
    }
    
    void map_mosaic_sampler::update() const {
      if (!tiles_.empty() &&
          buckets_.count() == 0) {
        // Update bbox and guess tiling deltas
        sl::vector2d tile_extent;
        std::size_t  nonempty_tile_count = 0;
        bounding_rectangle_.to_empty();
        for (std::size_t k=0; k<tiles_.size(); ++k) {
          const map_sampler* tile_k = tiles_[k];
          const aabox_t s_box = tile_k->bounding_rectangle();
          if (!s_box.is_empty()) {
#if 0
            std::cerr << "MOSAIC: " << "BR[" << k << "] = " <<
              "(" << s_box[0][0] << " " << s_box[0][1] << ") " <<
              "(" << s_box[1][0] << " " << s_box[1][1] << ")" <<
              std::endl;
#endif
            bounding_rectangle_.merge(s_box);
            tile_extent += s_box.diagonal();
            ++nonempty_tile_count;
          }
        }
        
        // Determine bucket map parameters
        if (nonempty_tile_count==0) {
          // All tiles empty!
          bucket_o_  = sl::point2d(0.0,0.0);
          bucket_duv_ = sl::vector2d(1.0,1.0);
          bucket_one_over_duv_ = sl::vector2d(1.0,1.0);
          buckets_.resize(sl::index<2>(0,0));
        } else {
          sl::vector2d buckets_extent = sl::vector2d(std::max(bounding_rectangle_.diagonal()[0],1e-10),
                                                     std::max(bounding_rectangle_.diagonal()[1],1e-10));
          
          tile_extent/= double(nonempty_tile_count);
          
          std::size_t u_bucket_count =
            sl::median(1,1024,
                       (int)(0.5+buckets_extent[0]/std::max(1e-10,0.5*tile_extent[0])));
          std::size_t v_bucket_count =
            sl::median(1,1024,
                       (int)(0.5+buckets_extent[1]/std::max(1e-10,0.5*tile_extent[1])));
          bucket_duv_ = sl::vector2d(buckets_extent[0]/double(u_bucket_count),
                                     buckets_extent[1]/double(v_bucket_count));
          bucket_one_over_duv_ = sl::vector2d(1.0/bucket_duv_[0],
                                              1.0/bucket_duv_[1]);
          // Add empty boundary
          u_bucket_count += 2;
          v_bucket_count += 2;

          // Compute origin and allocate
          bucket_o_ = bounding_rectangle_[0]-bucket_duv_;
          buckets_.resize(sl::index<2>(u_bucket_count, v_bucket_count));
        }
        // Fill bucket map
        for (std::size_t k=0; k<tiles_.size(); ++k) {
          const map_sampler* tile_k = tiles_[k];
          const aabox_t s_box = tile_k->bounding_rectangle();
          if (!s_box.is_empty()) {
            int  i0 = std::max(0, int((s_box[0][0]-bucket_o_[0])*bucket_one_over_duv_[0]));
            int  j0 = std::max(0, int((s_box[0][1]-bucket_o_[1])*bucket_one_over_duv_[1]));
            int  i1 = std::min(int(buckets_.extent()[0])-1, int((s_box[1][0]-bucket_o_[0])*bucket_one_over_duv_[0]));
            int  j1 = std::min(int(buckets_.extent()[1])-1, int((s_box[1][1]-bucket_o_[1])*bucket_one_over_duv_[1]));
            for (int i=i0; i<=i1; ++i) {
              for (int j=j0; j<=j1; ++j) {
                buckets_(i,j).push_back(k);
              }
            }
          }
        }

	/// Output mosaic stats
	if (is_verbose()) {
	  if (buckets_.count()) {
	    std::cerr << "---------------------------------------------------" << std::endl;
	    std::cerr << "MOSAIC: Created bucket map " <<
	      buckets_.extent()[0] << "x" << buckets_.extent()[1] << " for " << tiles_.size() << " tiles." << std::endl;
	    double max_density = 0.0;
	    double avg_density = 0.0;
	    double bucket_count = 0.0;
	    double nonempty_bucket_count = 0.0;
	    for (std::size_t i=0; i< buckets_.extent()[0]; ++i) {
	      for (std::size_t j=0; j< buckets_.extent()[1]; ++j) {
		double sz_ij = double(buckets_(i,j).size());
		max_density  = std::max(sz_ij, max_density);
		avg_density += sz_ij;
		bucket_count += 1.0;
		if (sz_ij>0) nonempty_bucket_count += 1.0;
	      }
	    }
	    std::cerr << "  Bounding rectangle               = " <<
	      "(" << bounding_rectangle_[0][0] << " " << bounding_rectangle_[0][1] << ") " <<
	      "(" << bounding_rectangle_[1][0] << " " << bounding_rectangle_[1][1] << ")" <<
	      std::endl;
	    std::cerr << "  Mosaic density                   = " << nonempty_bucket_count/bucket_count << " non-empty buckets/bucket" << std::endl;
	    std::cerr << "  Average bucket density           = " << avg_density/bucket_count << " tiles/bucket" << std::endl;
	    std::cerr << "  Average non-empty bucket density = " << avg_density/nonempty_bucket_count << " tiles/bucket" << std::endl;
	    std::cerr << "  Maximum bucket density           = " << max_density << " tiles/bucket" << std::endl;
	    std::cerr << "---------------------------------------------------" << std::endl;
	  }
	}
      }
    }
  }

}
