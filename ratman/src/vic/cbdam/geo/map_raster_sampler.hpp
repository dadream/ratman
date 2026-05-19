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
#ifndef VIC_GEO_MAP_RASTER_SAMPLER_HPP
#define VIC_GEO_MAP_RASTER_SAMPLER_HPP

#include <vic/cbdam/geo/map_sampler.hpp>
#include <list>
#include <string>

struct GDALDataset;

namespace vic {

  namespace geo {
    
    class map_raster_sampler : public map_sampler {
    public:
      typedef map_sampler                                       super_t;
      typedef sl::aabox2d                                       aabox_t;
      typedef sl::int32_t                                       int32_t;
      typedef sl::fixed_size_point<2, int32_t>                  point2i_t;
      typedef sl::fixed_size_point<2, double>                   point2d_t;
      typedef std::list<std::pair<point2i_t, int32_t*> >        cache_list_t;
      typedef cache_list_t::iterator                            cache_list_iterator_t;
      
    protected:
      mutable sl::uint64_t      stat_loaded_sample_count_;

      // Dataset
      mutable GDALDataset*      dataset_;
      std::size_t		dataset_x_size_;
      std::size_t		dataset_y_size_;
      std::size_t		dataset_band_count_;
      aabox_t                   bounding_rectangle_;
      std::string               dataset_file_name_;
      sl::matrix3d              geographic_from_pixel_;
      sl::matrix3d              pixel_from_geographic_;
      int32_t*                  dataset_nodata_value_;
      bool*                     dataset_has_nodata_value_;
      
    protected:
      // Cache
      mutable std::size_t		cache_tile_width_;
      mutable std::size_t		cache_tile_height_;
      mutable std::size_t		cache_tile_capacity_;
      mutable std::size_t               cache_byte_capacity_;

      mutable cache_list_t	cache_matrices_;

    public:

      map_raster_sampler();

      virtual ~map_raster_sampler();

      virtual bool is_empty() const;

      virtual void minimize_footprint() const;

      void open(const std::string& file_name);

      void close();
      
      void set_cache_capacity(std::size_t x);
      
      void cache_reset(std::size_t tile_height, std::size_t tile_width) const;
      
    public: // Stat

      virtual inline void stat_clear() {
	super_t::stat_clear();
	stat_loaded_sample_count_ = 0; 
      }

      virtual inline sl::uint64_t stat_loaded_sample_count() const { 
	return stat_loaded_sample_count_; 
      }

    public: // Read
      
      virtual std::size_t band_count() const;
      
      virtual bool sample_in(int32_t* sample,
                             double u, double v) const;

    public: // Extent

      virtual aabox_t bounding_rectangle() const;

      std::size_t raster_width() const;

      std::size_t raster_height() const;
      
    public: // Sampling resolution

      virtual double minimum_sample_spacing(const aabox_t& b, double eps=0.0) const;
      
    protected:
      
      void reopen() const;
      
      void refresh_cache(std::size_t tile_x, std::size_t tile_y) const;
      
      void geographic_from_pixel(point2d_t& geo, const point2i_t& pix) const;

      void pixel_from_geographic(point2d_t& pix, const point2d_t& geo) const;

      std::size_t matrix_offset(std::size_t x, std::size_t y) const;

      bool grid_sample_in(int32_t* sample, int32_t x, int32_t y) const;
    };
  }
}

#endif

#ifndef VIC_GEO_MAP_RASTER_SAMPLER_IPP
#define VIC_GEO_MAP_RASTER_SAMPLER_IPP

namespace vic {

  namespace geo {
    
    inline std::size_t map_raster_sampler::raster_width() const {
      return dataset_x_size_;
    }

    inline std::size_t map_raster_sampler::raster_height() const {
      return dataset_y_size_;
    }

    inline bool map_raster_sampler::is_empty() const{
      return dataset_ == NULL;
    }

    inline std::size_t map_raster_sampler::band_count() const{
      return dataset_band_count_;
    }
      
    inline map_raster_sampler::aabox_t map_raster_sampler::bounding_rectangle() const{
      return bounding_rectangle_;
    }

    inline void map_raster_sampler::geographic_from_pixel(point2d_t& geo, const point2i_t& pix) const {
      geo[0] = geographic_from_pixel_(0,2) + geographic_from_pixel_(0,0)*pix[0] + geographic_from_pixel_(0,1)*pix[1];
      geo[1] = geographic_from_pixel_(1,2) + geographic_from_pixel_(1,0)*pix[0] + geographic_from_pixel_(1,1)*pix[1];
    }

    inline void map_raster_sampler::pixel_from_geographic(point2d_t& pix, const point2d_t& geo) const {
      pix[0] = pixel_from_geographic_(0,2) + pixel_from_geographic_(0,0)*geo[0] + pixel_from_geographic_(0,1)*geo[1];
      pix[1] = pixel_from_geographic_(1,2) + pixel_from_geographic_(1,0)*geo[0] + pixel_from_geographic_(1,1)*geo[1];
    }

    inline std::size_t map_raster_sampler::matrix_offset(std::size_t x, std::size_t y) const {
      return (y*cache_tile_width_ + x)*dataset_band_count_;
    }

  }
  
}

#endif
