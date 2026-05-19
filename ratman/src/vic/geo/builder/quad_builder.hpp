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
#ifndef QUAD_BUILDER_HPP
#define QUAD_BUILDER_HPP

#include <vector>
#include <string>
#include <set>
#include <fstream>
#include <sl/axis_aligned_box.hpp>
#include <sl/fixed_size_point.hpp>
#include <stdint.h>

class GDALDataset;

namespace vic {
  namespace geo {

    namespace detail {

      struct row_major_less {
	inline bool operator() (const sl::tuple2i& xy0,
				const sl::tuple2i& xy1) const {
	  return 
	    (xy0[1] < xy1[1]) ||
	    ((xy0[1] == xy1[1]) && (xy0[0] < xy1[0]));
	}
      };
    }

    class quad_processor;
    class quad_accessor;
    class quad_warper;
    class geo_transform;

    class quad_builder {
    protected:
      typedef sl::axis_aligned_box<2,int> aabox2i_t;
      typedef aabox2i_t::point_t          point2i_t;

    protected:
      mutable bool last_operation_success_;
      mutable std::string last_error_message_;
      std::string default_src_srs_;
      std::string quad_srs_;
      quad_accessor *qaccessor_;
      quad_processor *qprocessor_;
      quad_warper *qwarper_;
      sl::aabox2d quadtree_extent_;
      int quad_width_;
      int quad_height_;
      int quad_band_count_;
      std::string tile_fname_;
      double quad_warp_max_error_;

      bool out_quad_index_enabled_;
      std::ofstream out_quad_index_file_;

      int color_remap_black_out_;
      int color_remap_black_in_;
      int color_remap_white_out_;
      int color_remap_white_in_;
      int color_remap_below_to_black_;
      int color_remap_above_to_black_;

      int damaged_level_min_;
      int damaged_level_max_;

      std::size_t max_incore_tile_memory_;

    protected:
      std::vector< std::pair<std::string,int> > input_tile_fname_level_;
      
      typedef std::set<sl::tuple2i, detail::row_major_less> sorted_xy_set_t;
      std::vector< sorted_xy_set_t* > damaged_nodes_;
    public:

      quad_builder();
      virtual ~quad_builder();
      
      inline bool out_quad_index_enabled() const {
	return out_quad_index_enabled_;
      }

      virtual std::string out_quad_index_fname() const;

      virtual void out_quad_index_begin();

      virtual void out_quad_index_end();

      virtual void out_quad_index_write(int l, int x, int y);
      
      inline bool last_operation_success() const {
	return last_operation_success_;
      }
      
      inline const std::string& last_error_message() const {
	return last_error_message_;
      }

      inline void reset_error() {
	last_operation_success_=true;
	last_error_message_=std::string("");
      }

      inline void set_color_remap_parameters(int color_remap_black_out,
					     int color_remap_black_in,
					     int color_remap_white_out,
					     int color_remap_white_in,
					     int color_remap_below_to_black,
					     int color_remap_above_to_black) {
	color_remap_black_out_=color_remap_black_out;
	color_remap_black_in_=color_remap_black_in;
	color_remap_white_out_=color_remap_white_out;
	color_remap_white_in_=color_remap_white_in;
	color_remap_below_to_black_=color_remap_below_to_black;
	color_remap_above_to_black_=color_remap_above_to_black;
      }

      inline void set_damaged_level_range(int damaged_level_min, int damaged_level_max) {
	damaged_level_min_=damaged_level_min;
	damaged_level_max_=damaged_level_max;
      }
      
      void set_default_src_proj(const std::string& quad_proj);

      /// Tiles occupying less than this will be loaded into memory ... 
      std::size_t max_incore_tile_memory() const {
	return max_incore_tile_memory_;
      }

      void set_max_incore_tile_memory(std::size_t x) {
	max_incore_tile_memory_ = x;
      }

      void set_quad_warp_max_error(double x) {
	quad_warp_max_error_ = x;
      }
      void set_quadtree_output_format(const std::string& format="JPEG",
				      const std::string& extension="jpg",  
				      const std::vector<std::string>& opts=std::vector<std::string>());
      void set_quadtree_root_dir(const std::string& root_dir);
      void set_quadtree_projection(const std::string& quad_projection);
      void set_quadtree_extent(double x0, double y0, double x1, double y1);
      void set_quadtree_root_count(std::size_t nx, std::size_t ny);
           
      inline void set_quad_size( int width, int height ) {
	quad_width_= width;
	quad_height_ = height;
      }

      inline void set_quad_band_count(int x) {
	quad_band_count_ = x;
      }

    public: // Processing

      virtual void begin_processing();
      virtual void process_tile(const std::string& tile_fname, int level= -1);
      virtual void process_directory(const std::string& dirname,
				     const std::string& pattern = std::string("*.tif"), int level= -1);
      virtual void end_processing();
      
    protected: 

      virtual void tile_parameters_in(std::size_t tile_idx,
				      int& tile_level, 
				      geo_transform& tile_warp_xform, 
				      aabox2i_t& quad_box_xy);

      virtual void process();

      virtual void identify_damaged_nodes();

      virtual void damage(int l, int x, int y);

      virtual void repair();

      virtual void repair_level(int l);

    protected:

      virtual void generate_quads_from_tile(const std::string& tile_fname, 
					    const geo_transform& tile_warp_xform, 
					    int level, 
					    const aabox2i_t& quad_index_box,
					    bool try_load_in_memory = false);

      virtual void generate_quad_from_children(int l, int x, int y, bool path_known_to_exist=false);
    };

  } // namespace geo
} // namespace vic

#endif
