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
#ifndef VIC_GEO_MAP_MOSAIC_SAMPLER_HPP
#define VIC_GEO_MAP_MOSAIC_SAMPLER_HPP

#include <vic/cbdam/geo/map_sampler.hpp>
#include <sl/dense_array.hpp>

#include <vector>
#include <list>

namespace vic {

  namespace geo {
    
    class map_mosaic_sampler: public map_sampler {
    public:
      typedef map_sampler super_t;
      typedef sl::aabox2d aabox_t;
    protected:
      mutable sl::uint64_t                                     stat_loaded_sample_count_;
      std::vector<map_sampler*>                                tiles_;
      mutable aabox_t                                          bounding_rectangle_;
      mutable sl::dense_array<std::list<std::size_t>, 2, void> buckets_;
      mutable sl::point2d                                      bucket_o_;
      mutable sl::column_vector2d                              bucket_duv_;
      mutable sl::column_vector2d                              bucket_one_over_duv_;
      mutable std::list<std::size_t>                           tile_lru_;
      std::size_t                                              tile_lru_capacity_;
    public:

      map_mosaic_sampler();

      map_mosaic_sampler(const std::string& dirname,
                         const std::string& pattern = std::string("*.tif"));

      virtual ~map_mosaic_sampler();

      virtual void set_verbose(bool x);

      virtual bool is_empty() const;

      virtual void minimize_footprint() const;

    public: // Tile manipulation
      
      std::size_t tile_count() const;

      void insert_directory(const std::string& dirname,
                            const std::string& pattern = std::string("*.tif"));
      
      void insert(map_sampler* tile);

      void clear();
      
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

    public: // Sampling resolution

      virtual double minimum_sample_spacing(const aabox_t& b, double eps=0.0) const;

    protected:

      void update() const;

      void touch(std::size_t) const;
      
    }; // class map_mosaic_sampler
    
  } // namespace geo
} // namespace vic

#endif
