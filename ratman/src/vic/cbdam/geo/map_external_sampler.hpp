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
#ifndef CBDAM_INPUT_SAMPLER_EXTERNAL_HPP
#define CBDAM_INPUT_SAMPLER_EXTERNAL_HPP

#include <vic/cbdam/geo/map_sampler.hpp>
#include <sl/cstdint.hpp>



namespace vic {

  namespace geo {

    // Basic data types 
    typedef sl::uint8_t         uint8_t;
    typedef sl::int32_t         int32_t;

    /// A color represented in rgb 8 bit format for external interface
    typedef struct rgb8_color {
      uint8_t r;
      uint8_t g;
      uint8_t b;
    } rgb8_color_t;

    typedef int32_t (*elevation_field_sampler_t)(void* context,
						 double u,
						 double v);

    /**
     *  Functions that return color at parametric
     *  coordinates (u,v)
     */
    typedef rgb8_color_t (*color_field_sampler_t)(void* context,
						  double u,
						  double v);

    /**
     * Base pure virtual class for external planar color and height samplers.
     * Derived class samples data through a callback
     */
    template<class T>
    class map_external_sampler : public generic_map_sampler<T> {
    public:
      typedef generic_map_sampler<T>	super_t;
      //     typedef typename super_t::aabox_t		aabox_t;
      typedef sl::aabox2d		aabox_t;
      typedef sl::point2d		point2d_t;
      typedef T				value_t;

    protected:
      double     ds_;
      double     u_extent_;
      double     v_extent_;

    public:
      map_external_sampler();
    
      map_external_sampler(double ds, double u_extent, double v_extent);

      ~map_external_sampler();

      virtual bool sample_in(int32_t* sample, double u, double v) const;

      virtual aabox_t bounding_rectangle() const;
    
      /**
       * Return target_sampling_rate when b and bounding 
       * rectangle intersects, b.diagonal otherwise.
       */
      virtual double minimum_sample_spacing(const aabox_t& b, double eps=0.0) const;

      /**
       * Set image sample spacing: width/sample_count
       */
      void set_target_sampling_rate(double ds);

      double target_sampling_rate() const;

      void set_extent(double u_extent, double v_extent);

    protected:
      point2d_t parametric_from_absolute(double u, double v) const;
    };

    class map_height_int32_external_sampler: public map_external_sampler<int32_t> {
    public:
      typedef map_external_sampler<int32_t> super_t;
      typedef sl::aabox2d aabox_t;
      typedef int32_t	value_t;
    
    protected:
      void*                       callback_context_;
      elevation_field_sampler_t   callback_;
      double	                  height_scale_factor_;
      int32_t			  unit_scale_;

    public:
      map_height_int32_external_sampler();

      map_height_int32_external_sampler(double ds, 
					double u_extent, 
					double v_extent,
					void* cb_context,
					elevation_field_sampler_t cb,
					double height_scale_factor);
    
      virtual ~map_height_int32_external_sampler();

      virtual bool is_empty() const;

      virtual void minimize_footprint() const;

      void set_callback(void* cb_context, elevation_field_sampler_t cb);

      void set_height_scale_factor(double height_scale_factor);

    public: // Read
      
      virtual std::size_t band_count() const;

      virtual value_t value_at(double u, double v) const;      

      value_t value_at_parametric(double u, double v) const;      

      virtual int32_t unit_scale() const;
    };

    /**
     * Implementation of the planar_external_sampler for colors
     */
    class map_rgb_int16_8_external_sampler: public map_external_sampler<sl::fixed_size_vector<sl::column_orientation, 3, sl::int16_t > > {
    public:
      typedef map_external_sampler<sl::fixed_size_vector<sl::column_orientation, 3, sl::int16_t > > super_t;
      typedef sl::int16_t                                                    component_t;
      typedef sl::fixed_size_vector<sl::column_orientation, 3, component_t > value_t;

    protected:
      void*                       callback_context_;
      color_field_sampler_t	callback_;
      int32_t remap_lo_;
      int32_t remap_hi_;
      double  remap_scale_;
     
    public:

      map_rgb_int16_8_external_sampler();

      map_rgb_int16_8_external_sampler(double ds, 
				       double u_extent, 
				       double v_extent,
				       void* cb_context,
				       color_field_sampler_t cb,
				       int32_t remap_lo = 0,
				       int32_t remap_hi = 255);
                                
      virtual ~map_rgb_int16_8_external_sampler();
    
      virtual bool is_empty() const;

      virtual void minimize_footprint() const;

      void set_callback(void* cb_context, color_field_sampler_t cb);

      void set_remap_lo_hi(int32_t lo, int32_t hi);

    public: // Read
      
      virtual std::size_t band_count() const;

      virtual value_t value_at(double u, double v) const;      

      value_t value_at_parametric(double u, double v) const;      

    protected:
      component_t remap(int32_t x) const;
    };

  } // namespace geo
} // namespace vic 

#endif // CBDAM_INPUT_SAMPLER_EXTERNAL_HPP

#ifndef CBDAM_INPUT_SAMPLER_EXTERNAL_IPP
#define CBDAM_INPUT_SAMPLER_EXTERNAL_IPP

namespace vic {

  namespace geo {


    template<class T>
    inline map_external_sampler<T>::map_external_sampler() {
      ds_ = 1.0;
      u_extent_ = 0.0;
      v_extent_ = 0.0;
    }

    template<class T>
    inline map_external_sampler<T>::map_external_sampler(double ds, 
							 double u_extent, 
							 double v_extent) :
      ds_(ds), u_extent_(u_extent), v_extent_(v_extent) {

    }

    template<class T>
    inline map_external_sampler<T>::~map_external_sampler() {

    }

    template<class T>
    inline void map_external_sampler<T>::set_extent(double u_extent, double v_extent) {
      u_extent_ = u_extent;
      v_extent_ = v_extent;
    }
  
    template<class T>
    inline void map_external_sampler<T>::set_target_sampling_rate(double ds) {
      ds_ = ds;
    }

    template<class T>
    inline double map_external_sampler<T>::target_sampling_rate() const {
      return ds_;
    }

    template<class T>
    inline typename map_external_sampler<T>::aabox_t map_external_sampler<T>::bounding_rectangle() const {
      return aabox_t(point2d_t(-u_extent_*0.5, -v_extent_*0.5), 
		     point2d_t( u_extent_*0.5,  v_extent_*0.5));
    }
  
    template<class T>
    inline double map_external_sampler<T>::minimum_sample_spacing(const aabox_t& b, double /*eps*/) const {
      const double half_u_extent = 0.5 * u_extent_;
      const double half_v_extent = 0.5 * v_extent_;
      if ((b[0][0]> half_u_extent) ||
	  (b[1][0]<-half_u_extent) ||
	  (b[0][1]> half_v_extent) ||
	  (b[1][1]<-half_v_extent)) {
	// do not intersect
	return b.diagonal().two_norm();
      } else {
	return ds_;
      }
    }
    
    template<class T>
    inline bool map_external_sampler<T>::sample_in(int32_t* /*sample*/, double /*u*/, double /*v*/) const {
      // in external sampler sampling is done directly from value_at functions.
      SL_TRACE_OUT(-1) << "SHOULD NOT BE CALLED" << std::endl;
      abort();
      return false;
    }

    template<class T>
    inline typename map_external_sampler<T>::point2d_t map_external_sampler<T>::parametric_from_absolute(double u, double v) const {
      return point2d_t(u/u_extent_ + 0.5, v/v_extent_ + 0.5);  
    }

    /////////////////////////////////////// map_height_int32_external_sampler /////////////////////////////////////////

    inline map_height_int32_external_sampler::map_height_int32_external_sampler() {
      callback_ = 0;
      callback_context_ = 0;
      height_scale_factor_ = 1.0;
      unit_scale_ = 64;
    }

    inline map_height_int32_external_sampler::map_height_int32_external_sampler(double ds, 
										double u_extent, 
										double v_extent,
										void* cb_context,
										elevation_field_sampler_t cb,
										double height_scale_factor) :
      super_t(ds, u_extent, v_extent), callback_context_(cb_context), 
      callback_(cb), height_scale_factor_(height_scale_factor) {
      unit_scale_ = 64;
    }
    
    inline map_height_int32_external_sampler::~map_height_int32_external_sampler() {
      callback_ = 0;
      callback_context_ = 0;
    }

    inline bool map_height_int32_external_sampler::is_empty() const {
      return callback_ == 0;
    }

    inline void map_height_int32_external_sampler::minimize_footprint() const {

    }

    inline void map_height_int32_external_sampler::set_callback(void* cb_context, 
								elevation_field_sampler_t cb) {
      callback_context_ = cb_context;
      callback_ = cb;
    }

    inline void map_height_int32_external_sampler::set_height_scale_factor(double height_scale_factor) {
      height_scale_factor_ = height_scale_factor;
    }

    inline std::size_t map_height_int32_external_sampler::band_count() const {
      return 1;
    }

    inline map_height_int32_external_sampler::value_t map_height_int32_external_sampler::value_at(double u, double v) const {
      point2d_t p = parametric_from_absolute(u, v);
      return (int32_t)(height_scale_factor_*callback_(callback_context_, p[0], p[1])*unit_scale_);
    }
 
    inline map_height_int32_external_sampler::value_t map_height_int32_external_sampler::value_at_parametric(double u, double v) const {
      return (int32_t)(height_scale_factor_*callback_(callback_context_, u, v)*unit_scale_);
    }

    inline int32_t map_height_int32_external_sampler::unit_scale() const {
      return unit_scale_;
    } 
    //////////////////////////////////////////// map_rgb_int16_8_external_sampler /////////////////////

    inline map_rgb_int16_8_external_sampler::map_rgb_int16_8_external_sampler() {
      callback_context_ = 0;
      callback_ = 0;
      set_remap_lo_hi(0, 255);
    }

    inline map_rgb_int16_8_external_sampler::map_rgb_int16_8_external_sampler(double ds, 
									      double u_extent, 
									      double v_extent,
									      void* cb_context,
									      color_field_sampler_t cb,
									      int32_t remap_lo,
									      int32_t remap_hi) :
      super_t(ds, u_extent, v_extent), callback_context_(cb_context), callback_(cb) {
      set_remap_lo_hi(remap_lo, remap_hi);    
    }
                                
    inline map_rgb_int16_8_external_sampler::~map_rgb_int16_8_external_sampler() {
      callback_context_ = 0;
      callback_ = 0;
    }
    
    inline bool map_rgb_int16_8_external_sampler::is_empty() const {
      return callback_ == 0;
    }

    inline void map_rgb_int16_8_external_sampler::minimize_footprint() const {

    }

    inline void map_rgb_int16_8_external_sampler::set_callback(void* cb_context, color_field_sampler_t cb) {
      callback_context_ = cb_context;
      callback_ = cb;
    }

    inline void map_rgb_int16_8_external_sampler::set_remap_lo_hi(int32_t lo, int32_t hi) {
      remap_lo_ = lo;
      remap_hi_ = hi;
      remap_scale_ = double(255)/(remap_hi_-remap_lo_);
    }

    inline std::size_t map_rgb_int16_8_external_sampler::band_count() const {
      return 3;
    }

    inline map_rgb_int16_8_external_sampler::value_t map_rgb_int16_8_external_sampler::value_at(double u, double v) const {
      point2d_t p = parametric_from_absolute(u, v);
      rgb8_color_t c = callback_(callback_context_, p[0], p[1]);
      return value_t(remap(c.r), remap(c.g), remap(c.b));
    }      

    inline map_rgb_int16_8_external_sampler::value_t map_rgb_int16_8_external_sampler::value_at_parametric(double u, double v) const {
      rgb8_color_t c = callback_(callback_context_, u, v);
      return value_t(remap(c.r), remap(c.g), remap(c.b));
    }      

    inline map_rgb_int16_8_external_sampler::component_t map_rgb_int16_8_external_sampler::remap(int32_t x) const {
      double xd = (double(x)-double(remap_lo_))*remap_scale_;
      return static_cast<component_t>(sl::median(xd, 0.0, 255.0));
    }

  } // namespace geo
} // namespace vic 

#endif // CBDAM_INPUT_SAMPLER_EXTERNAL_IPP

