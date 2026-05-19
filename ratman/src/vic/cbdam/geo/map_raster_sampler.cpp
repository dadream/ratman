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
#include <vic/cbdam/geo/map_raster_sampler.hpp>
#include <gdal_priv.h>

namespace vic {

  namespace geo {
    
    map_raster_sampler::map_raster_sampler() {
      stat_loaded_sample_count_ = 0;
      dataset_ = NULL;
      dataset_x_size_ = 0;
      dataset_y_size_ = 0;
      dataset_band_count_ = 0;
      bounding_rectangle_.to_empty();
      dataset_file_name_ = "";
      dataset_nodata_value_ = 0;
      dataset_has_nodata_value_ = 0;
      cache_byte_capacity_ = 16*1024*1024;
      cache_reset(32, 32);

      // 	Initialize the library if we are linking statically
      GDALAllRegister();
    }
    
    map_raster_sampler::~map_raster_sampler() {
      close();
      if (dataset_nodata_value_!= 0) {delete[] dataset_nodata_value_;}
      if (dataset_has_nodata_value_!= 0) {delete[] dataset_has_nodata_value_;}
    }
    
    void map_raster_sampler::minimize_footprint() const{
      cache_reset(cache_tile_height_, cache_tile_width_);

      if (dataset_) {
        delete dataset_;
        dataset_ = NULL;
        // leave all other parameters untouched and available for a reopen call
      }
    }

    void map_raster_sampler::open(const std::string& file_name){
      dataset_file_name_ = file_name;
      dataset_ = (GDALDataset *) GDALOpen(dataset_file_name_.c_str(), GA_ReadOnly );
      if( dataset_ == NULL ) {
	std::cerr << "unable to open " << dataset_file_name_ << " for reading" << std::endl;
	return;
      } else {
        // open a new file, read all parameters, otherwhise it is a reopen
	dataset_x_size_ = dataset_->GetRasterXSize();
        dataset_y_size_ = dataset_->GetRasterYSize();
        dataset_band_count_ =  dataset_->GetRasterCount();
        dataset_nodata_value_ = new int32_t[dataset_band_count_];
        dataset_has_nodata_value_ = new bool[dataset_band_count_];
          
        if (is_verbose()) {
	  std::cerr << "Opening raster " << dataset_file_name_ << " for reading" << std::endl;
	  std::cerr << "  dataset [" << dataset_x_size_ << " x " << dataset_y_size_ << " x " << dataset_->GetRasterCount() << "]" << std::endl; 
        }
          
        std::size_t tile_height = dataset_y_size_; // forced max
        std::size_t tile_width  = dataset_x_size_; // forced max
        for (std::size_t i=0; i<dataset_band_count_; ++i) {
          GDALRasterBand  *poBand = dataset_->GetRasterBand( 1+i );
          int    has_nodatavalue;
          double nodatavalue = poBand->GetNoDataValue(&has_nodatavalue);
          int has_minimum;
          double minimum = poBand->GetMinimum(&has_minimum);
          int has_maximum;
          double maximum = poBand->GetMaximum(&has_maximum);
          
          int        nXBlockSize, nYBlockSize;
          poBand->GetBlockSize( &nXBlockSize, &nYBlockSize );
          tile_height = std::min(tile_height, std::size_t(nYBlockSize));
          tile_width = std::min(tile_width, std::size_t(nXBlockSize));
          
          dataset_has_nodata_value_[i] = has_nodatavalue;
          dataset_nodata_value_[i] = has_nodatavalue ? int32_t(nodatavalue) : std::numeric_limits<int32_t>::min();

          if (is_verbose()) {
	    std::cerr << "   band "  << i << ": ";
	    std::cerr << " BLOCK=" << tile_height << "x" << tile_width << " ";
	    if (has_nodatavalue) std::cerr << " NODATA=" << nodatavalue;
	    if (has_minimum) std::cerr << " MIN=" << minimum;
	    if (has_maximum) std::cerr << " MAX=" << maximum;
	    std::cerr << std::endl;
	  }
        }

        // set image geographic box
        double gt[6];
        dataset_->GetGeoTransform(gt); // on error return default transform 0,1,0,0,0,1
        geographic_from_pixel_ = sl::matrix3d(gt[1], gt[2], gt[0],
                                              gt[4], gt[5], gt[3],
                                              0.0,   0.0,   1.0);
        //std::cerr << "GfromP" << std::endl << geographic_from_pixel_ << std::endl;
        pixel_from_geographic_ = geographic_from_pixel_.inverse();
        //std::cerr << "PfromG" << std::endl << pixel_from_geographic_  << std::endl;
        point2d_t geo;
        geographic_from_pixel(geo, point2i_t(0,0));
        bounding_rectangle_.to(geo);
        geographic_from_pixel(geo, point2i_t(dataset_x_size_, 0));
        bounding_rectangle_.merge(geo);
        geographic_from_pixel(geo, point2i_t(dataset_x_size_, dataset_y_size_));
        bounding_rectangle_.merge(geo);
        geographic_from_pixel(geo, point2i_t(0, dataset_y_size_));
        bounding_rectangle_.merge(geo);
        //std::cerr << "Bounding rectangle " << bounding_rectangle_ << std::endl;

        // Reset cache
        cache_reset(tile_height, tile_width);
      }
    }

    void map_raster_sampler::reopen() const {
      if (dataset_ == NULL) {
        dataset_ = (GDALDataset *) GDALOpen(dataset_file_name_.c_str(), GA_ReadOnly );
        if( dataset_ == NULL ) {
          std::cerr << "unable to reopen " << dataset_file_name_ << " for reading" << std::endl;
        }
      }
    }
    
    void  map_raster_sampler::close() {
      minimize_footprint();
    }
      
    bool map_raster_sampler::sample_in(int32_t* sample,
                                       double u, double v) const {
      ++stat_requested_sample_count_;

      if (!dataset_) {
        reopen();
        if (!dataset_) {
          return false;
        }
      }

      point2d_t pix(0,0);
      point2d_t geo(u,v);
      pixel_from_geographic(pix, geo);
#if 1
      const int32_t x = int32_t(pix[0]);
      const int32_t y = int32_t(pix[1]);
      
      const std::size_t band_count = dataset_band_count_;
      int32_t* s00 = new int32_t[band_count];
      int32_t* s01 = new int32_t[band_count];
      int32_t* s10 = new int32_t[band_count];
      int32_t* s11 = new int32_t[band_count];
      const bool r00 = grid_sample_in(s00, x, y);
      const bool r01 = grid_sample_in(s01, x, y+1);
      const bool r10 = grid_sample_in(s10, x+1, y);
      const bool r11 = grid_sample_in(s11, x+1, y+1);
      
      int valid_count = 0;
      if (r00) ++valid_count;
      if (r01) ++valid_count;
      if (r10) ++valid_count;
      if (r11) ++valid_count;

      const bool result = (valid_count>0);
#if 0
      if (result) {
	if (valid_count != 4) {
	  // Make sure all values are valid
	  for(std::size_t i = 0; i < band_count; ++i) {
	    double d_res = 0.0;
	    if (r00) {d_res += s00[i];}
	    if (r01) {d_res += s01[i];}
	    if (r10) {d_res += s10[i];}
	    if (r11) {d_res += s11[i];}
	    d_res /= valid_count;
	    int32_t savg_i = int32_t(d_res + 0.5);
	    if (!r00) { s00[i] = savg_i; }
	    if (!r01) { s01[i] = savg_i; }
	    if (!r10) { s10[i] = savg_i; }
	    if (!r11) { s11[i] = savg_i; }
	  }
	}

	const double fx = pix[0] - x;
	const double fy = pix[1] - y;

	const double w00 = (1.0-fx) * (1.0-fy);
	const double w01 = (1.0-fx) * fy;
	const double w10 = fx * (1.0-fy);
	const double w11 = fx * fy;

	for(std::size_t i = 0; i < band_count; ++i) {
	  double d_res = 0.0;
	  if (r00) {d_res += w00 * s00[i];}
	  if (r01) {d_res += w01 * s01[i];}
	  if (r10) {d_res += w10 * s10[i];}
	  if (r11) {d_res += w11 * s11[i];}
	  sample[i] = int32_t(d_res + 0.5);
	}
      } 
      delete[] s00;
      delete[] s01;
      delete[] s10;
      delete[] s11;
      return result;
#else
      if (result) {
	if (valid_count == 4) {
	  // take weighted average
	  const double fx = pix[0] - x;
	  const double fy = pix[1] - y;
	  
	  const double w00 = (1.0-fx) * (1.0-fy);
	  const double w01 = (1.0-fx) * fy;
	  const double w10 = fx * (1.0-fy);
	  const double w11 = fx * fy;
	  
	  for(std::size_t i = 0; i < band_count; ++i) {
	    double d_res = 0.0;
	    if (r00) {d_res += w00 * s00[i];}
	    if (r01) {d_res += w01 * s01[i];}
	    if (r10) {d_res += w10 * s10[i];}
	    if (r11) {d_res += w11 * s11[i];}
	    sample[i] = int32_t(d_res + 0.5);
	  }
	} else {
	  // take first valid
	  const int32_t* res = 0;
	  if (r00) {
	    res = s00;
	  } else if (r01) {
	    res = s01;
	  } else if (r10) {
	    res = s10;
	  } else if (r11) {
	    res = s11;
	  }
	  for(std::size_t i = 0; i < band_count; ++i) {
	    sample[i] = res[i];
	  }
	}
      } 
      delete[] s00;
      delete[] s01;
      delete[] s10;
      delete[] s11;
      return result;
#endif
      

#else
      return grid_sample_in(sample, int32_t(pix[0]), int32_t(pix[1]));
#endif
    }

    bool map_raster_sampler::grid_sample_in(int32_t* sample,
					    int32_t x, int32_t y) const {
      bool result = false;
 
      if (x >= 0 && x < int32_t(dataset_x_size_) &&
          y >= 0 && y < int32_t(dataset_y_size_)) {
        std::size_t tile_x = x / cache_tile_width_;
        std::size_t tile_y = y / cache_tile_height_;
        std::size_t res_x  = x - tile_x * cache_tile_width_;
        std::size_t res_y  = y - tile_y * cache_tile_height_;

        refresh_cache(tile_x, tile_y);

        const int32_t* p_data = &(cache_matrices_.front().second)[matrix_offset(res_x, res_y)];

        // invalid: all component[i] == no data value[i]
        bool all_novalid = true;
        const std::size_t band_count = dataset_band_count_;
        for(std::size_t i = 0; i < band_count; ++i) {
          sample[i] = p_data[i];
          all_novalid = all_novalid && dataset_has_nodata_value_[i] && (p_data[i] == dataset_nodata_value_[i]);
        }
        result = !all_novalid;
      } 
      
      return result;
    }

    double map_raster_sampler::minimum_sample_spacing(const aabox_t& b, double /*eps*/) const{
      if ((b[0][0]>bounding_rectangle_[1][0]) ||
          (b[1][0]<bounding_rectangle_[0][0]) ||
          (b[0][1]>bounding_rectangle_[1][1]) ||
          (b[1][1]<bounding_rectangle_[0][1])) {
        return b.diagonal().two_norm();
      } else {
        return std::min(bounding_rectangle_.diagonal()[0]/double(dataset_x_size_),
                        bounding_rectangle_.diagonal()[1]/double(dataset_y_size_));
      }
    }

    void map_raster_sampler::set_cache_capacity(std::size_t x) {
      cache_byte_capacity_ = x;
      cache_reset(cache_tile_height_, cache_tile_width_);
    }

    void map_raster_sampler::cache_reset(std::size_t tile_height, std::size_t tile_width) const {
      // Empty cache
      for(cache_list_iterator_t i = cache_matrices_.begin();
	  i != cache_matrices_.end();
	  ++i) {
	delete [] i->second;
      }
      cache_matrices_.clear();

      // Reset
      if (tile_height >= 8 && tile_width >= 8) {
	// Assume file has natural tile size, use it (within limits)
	cache_tile_height_ = sl::median(std::size_t(8), std::size_t(256), tile_height); // FIXME
	cache_tile_width_  = sl::median(std::size_t(8), std::size_t(256), tile_width);  // FIXME
      } else {
	// Assume file has no natural tile size, guess one
	std::size_t blockdim = std::min(tile_height, tile_width);
	while (blockdim<64) blockdim*= 2;

	cache_tile_height_ = blockdim;
	cache_tile_width_  = blockdim;
      }

      std::size_t cache_tile_size = (cache_tile_width_*
				     cache_tile_height_*
				     std::max(dataset_band_count_,std::size_t(1)))*sizeof(int32_t);
      cache_tile_capacity_ = sl::median(std::size_t(2), 
					std::size_t(1024),
					cache_byte_capacity_ / cache_tile_size);
      SL_TRACE_OUT(0) << "CACHE RESET: " << cache_tile_height_ << "x" << cache_tile_width_ 
		      << ", cache byte capacity " << cache_byte_capacity_ 
		      << ", cache tile capacity " << cache_tile_capacity_ << std::endl;
    }

    void map_raster_sampler::refresh_cache(std::size_t tile_x, std::size_t tile_y) const {
      // search in the list
      point2i_t tile_id(tile_x, tile_y);

      cache_list_iterator_t ci = cache_matrices_.begin();
      while (ci != cache_matrices_.end() && ci->first != tile_id) {
	++ci;
      }

      if (ci != cache_matrices_.end()) {
	if (ci != cache_matrices_.begin()) {
	  // push it on the front of the list
	  cache_matrices_.push_front(*ci);
	  cache_matrices_.erase(ci);
	}
      } else {
	// missing data: refresh cache
	int32_t* matrix = 0;
	if (cache_matrices_.size() >= cache_tile_capacity_) {
	  // get last cache matrix and change its content
	  matrix = cache_matrices_.back().second;
	  cache_matrices_.pop_back();
	} else {
	  // create a new matrix
	  matrix = new int32_t[cache_tile_width_ * cache_tile_height_ * dataset_band_count_];
	}
	// put this matrix int the top of the list
	cache_matrices_.push_front(std::pair<point2i_t, int32_t*>(tile_id, matrix));

	// fill the matrix
	std::size_t start_x = tile_x * cache_tile_width_;
	std::size_t start_y = tile_y * cache_tile_height_;
	std::size_t end_x = std::min(start_x + cache_tile_width_ - 1, dataset_x_size_ - 1);  
	std::size_t end_y = std::min(start_y + cache_tile_height_ - 1, dataset_y_size_ - 1);  
	std::size_t count_x = end_x - start_x + 1;
	std::size_t count_y = end_y - start_y + 1;

	stat_loaded_sample_count_ += sl::uint64_t(count_x*count_y);
#if 0   
	double cache_load_efficiency = double(stat_requested_sample_count_)/double(stat_loaded_sample_count_);

	std::cerr << "CACHE MISS: Efficiency=" << cache_load_efficiency << std::endl;
	std::cerr << "GDAL CACHE: " << GDALGetCacheUsed() << "/" << GDALGetCacheMax() << " bytes" << std::endl; 
#endif

#if 1
	// FIXME !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// Read raster and convert to int32 format
        dataset_->RasterIO(GF_Read, 
			   start_x, start_y, count_x, count_y,
                           matrix, count_x, count_y, GDT_Int32,
                           dataset_band_count_, NULL,
                           dataset_band_count_ * sizeof(int32_t),                         // pixel space
                           cache_tile_width_ * dataset_band_count_ * sizeof(int32_t),     // line space
                           sizeof(int32_t));                                              // band space
#endif
      }
    }
    
  } // namespace geo
  
} // namespace vic


  
