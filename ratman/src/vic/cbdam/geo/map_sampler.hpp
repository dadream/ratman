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
#ifndef VIC_GEO_MAP_SAMPLER_HPP
#define VIC_GEO_MAP_SAMPLER_HPP

#include <sl/axis_aligned_box.hpp>
#include <sl/integer.hpp>

namespace vic {

  namespace geo {
    
    class map_sampler {
    public:
      typedef sl::aabox2d aabox_t;
      typedef sl::int32_t int32_t;
    protected:
      mutable sl::uint64_t stat_requested_sample_count_;
      bool    is_verbose_;
    public:

      inline map_sampler() { is_verbose_ = true; stat_requested_sample_count_ = 0; }

      virtual inline ~map_sampler() {}

      virtual bool is_verbose() const {
	return is_verbose_;
      }

      virtual void set_verbose(bool x) {
	is_verbose_ = x;
      }

      virtual bool is_empty() const = 0;

      virtual inline void minimize_footprint() const {}

    public: // I/O statistics

      virtual void         stat_clear() { 
	stat_requested_sample_count_ = 0; 
      }

      virtual sl::uint64_t stat_loaded_sample_count() const { 
	return 0; 
      }

      inline sl::uint64_t stat_requested_sample_count() const { 
	return stat_requested_sample_count_; 
      }

    public: // Read
      
      virtual std::size_t band_count() const = 0;
      
      virtual bool sample_in(int32_t* sample,
                             double u, double v) const = 0;

    public: // Extent

      virtual aabox_t bounding_rectangle() const = 0;

    public: // Sampling resolution

      virtual double minimum_sample_spacing(const aabox_t& b, double eps=0.0) const = 0;
      
    };

    
    template <class T>
    class generic_map_sampler: public map_sampler {
    public:
      typedef map_sampler::aabox_t aabox_t;
      typedef T value_t;
    public:
      inline generic_map_sampler() {}

      virtual inline ~generic_map_sampler() {}

      virtual value_t value_at(double u, double v) const = 0;      

      virtual int32_t unit_scale() const {
	return 1;
      }
    };

    template <class T>
    class generic_map_sampler_wrapper: public generic_map_sampler<T> {
    public:
      typedef typename generic_map_sampler<T>::aabox_t aabox_t;
      typedef T value_t;
    protected:
      map_sampler* sampler_;
    public:
      inline generic_map_sampler_wrapper(map_sampler* s = 0): sampler_(s) {
	if (sampler_) sampler_->set_verbose(this->is_verbose_);
      }

      virtual inline ~generic_map_sampler_wrapper() {
        sampler_ = 0;
      }

      const map_sampler* sampler() const {
        return sampler_;
      }

      virtual void set_sampler(map_sampler* s) {
        sampler_ = s;
	if (sampler_) sampler_->set_verbose(this->is_verbose_);
      }
 
      virtual void set_verbose(bool x) {
	this->is_verbose_ = x;
	if (sampler_) sampler_->set_verbose(this->is_verbose_);
      }
	
      virtual bool is_empty() const {
        return sampler_ ? (sampler_->is_empty()) : true;
      }

      virtual void minimize_footprint() const {
        if (sampler_) sampler_->minimize_footprint();
      }

    public: // Stats

     virtual sl::uint64_t stat_loaded_sample_count() const { 
	return sampler_ ? (sampler_->stat_loaded_sample_count()) : 0; 
      }

    public: // Read
      
      virtual std::size_t band_count() const {
        return sampler_ ? (sampler_->band_count()) : 0;
      }
      
      virtual bool sample_in(int32_t* sample,
                             double u, double v) const {
	++(this->stat_requested_sample_count_);
        return sampler_ ? (sampler_->sample_in(sample, u, v)) : false;
      }

    public: // Extent

      virtual aabox_t bounding_rectangle() const {
        aabox_t result;
        if (sampler_) {
          result = sampler_->bounding_rectangle();
        } else {
          result.to_empty();
        }
        return result;
      }

    public: // Sampling resolution

      virtual double minimum_sample_spacing(const aabox_t& b, double eps=0.0) const {
        if (sampler_) {
          return sampler_->minimum_sample_spacing(b,eps);
        } else {
          return b.diagonal().two_norm();
        }
      }
      
    };
    
    
    template <std::size_t Bits>
    class map_rgb_sampler: public generic_map_sampler_wrapper< sl::fixed_size_vector<sl::column_orientation, 3, typename sl::uint_t<Bits>::least > > {
    public:
      typedef typename sl::uint_t<Bits>::least                               component_t;
      typedef sl::fixed_size_vector<sl::column_orientation, 3, component_t > value_t;

      typedef generic_map_sampler_wrapper<value_t> super_t;
      typedef typename super_t::aabox_t aabox_t;
    protected:
      int32_t remap_lo_;
      int32_t remap_hi_;

      double  remap_scale_;
      value_t   nodata_value_;
    public:

      inline map_rgb_sampler(map_sampler* s = 0) :
          super_t(s) {
        remap_lo_ = 0;
        remap_hi_ = std::numeric_limits<component_t>::max();
        remap_scale_ = double(std::numeric_limits<component_t>::max())/(remap_hi_-remap_lo_);
      }

      virtual inline ~map_rgb_sampler() {
      }

      inline sl::int32_t remap_lo() const {
        return remap_lo_;
      }

      inline sl::int32_t remap_hi() const {
        return remap_hi_;
      }

      inline void set_remap_lo_hi(int32_t lo, int32_t hi) {
        remap_lo_ = lo;
        remap_hi_ = hi;
        remap_scale_ =  double(std::numeric_limits<component_t>::max())/(remap_hi_-remap_lo_);
      }

      inline const value_t nodata_value() const {
        return nodata_value_;
      }

      inline void set_nodata_value(const value_t& x) {
        nodata_value_ = x;
      }
      
      inline component_t remap(int32_t x) const {
        double xd = (double(x)-double(remap_lo_))*remap_scale_;
        return static_cast<component_t>(sl::median(xd, 0.0, double(std::numeric_limits<component_t>::max())));
      }
             
      virtual value_t value_at(double u, double v) const {
        int32_t x[32]; // FIXME
        bool x_exists = this->sample_in(x, u, v);
        if (x_exists) {
          value_t result;
          std::size_t N= this->band_count();
          if (N<3) {
            component_t l = remap(x[0]);
            result[0] = l;
            result[1] = l;
            result[2] = l;
          } else {
            result[0] = remap(x[0]);
            result[1] = remap(x[1]);
            result[2] = remap(x[2]);
          }
          return result;
        } else {
          return nodata_value_;
        }
      }
      
    };

    // Default for height
    class map_int32_sampler: public generic_map_sampler_wrapper<sl::int32_t > {
    public:
      typedef int32_t                                   value_t;
      typedef generic_map_sampler_wrapper<value_t>	super_t;
      typedef super_t::aabox_t				aabox_t;

    protected:
      value_t nodata_value_;
      int32_t scale_; // FIXME
      int32_t unit_scale_;

    public:

      inline map_int32_sampler(map_sampler* s = 0) :
	super_t(s) {
        nodata_value_  = 0;
        scale_ = 1;
	unit_scale_ = 64;
      }

      virtual inline ~map_int32_sampler() {
      }
             
      virtual value_t value_at(double u, double v) const {
        int32_t x[32]; // FIXME
        bool x_exists = this->sample_in(x, u, v);
        if (x_exists) {
          return x[0]*scale_*unit_scale_;
        } else {
          return nodata_value_;
        }
      }

      virtual int32_t unit_scale() const {
	return unit_scale_;
      }
    };

    // Default for color: 8 bit sampling, signed 16 bit conversion
    class map_rgb_int16_8_sampler: public generic_map_sampler_wrapper< sl::fixed_size_vector<sl::column_orientation, 3, sl::int16_t > > {
    public:
      typedef sl::int16_t                                                    component_t;
      typedef sl::fixed_size_vector<sl::column_orientation, 3, component_t > value_t;

      typedef generic_map_sampler_wrapper<value_t> super_t;
      typedef super_t::aabox_t aabox_t;
    protected:
      int32_t remap_lo_;
      int32_t remap_hi_;

      double  remap_scale_;
      value_t nodata_value_;
    public:

      inline map_rgb_int16_8_sampler(map_sampler* s = 0) :
          super_t(s) {
        remap_lo_    = 0;
        remap_hi_    = 255; // 
        remap_scale_ = double(255)/(remap_hi_-remap_lo_);
      }

      virtual inline ~map_rgb_int16_8_sampler() {
      }

      inline int32_t remap_lo() const {
        return remap_lo_;
      }

      inline int32_t remap_hi() const {
        return remap_hi_;
      }

      inline void set_remap_lo_hi(int32_t lo, int32_t hi) {
        remap_lo_ = lo;
        remap_hi_ = hi;
        remap_scale_ = double(255)/(remap_hi_-remap_lo_);
      }

      inline const value_t nodata_value() const {
        return nodata_value_;
      }

      inline void set_nodata_value(const value_t& x) {
        nodata_value_ = x;
      }
      
      inline component_t remap(int32_t x) const {
        double xd = (double(x)-double(remap_lo_))*remap_scale_;
        return static_cast<component_t>(sl::median(xd, 0.0, 255.0));
      }
             
      virtual value_t value_at(double u, double v) const {
        int32_t x[32]; // FIXME
        bool x_exists = this->sample_in(x, u, v);
        if (x_exists) {
          value_t result;
          std::size_t N= this->band_count();
          if (N<3) {
            component_t l = remap(x[0]);
            result[0] = l;
            result[1] = l;
            result[2] = l;
          } else {
            result[0] = remap(x[0]);
            result[1] = remap(x[1]);
            result[2] = remap(x[2]);
          }
          return result;
        } else {
          return nodata_value_;
        }
      }
      
    };
  }
}

#endif
