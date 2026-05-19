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
#include <vic/cbdam/base/diamond_graph_builder.hpp>
#include <vic/cbdam/base/grid_diamond_graph.hpp>
#include <iostream>
#include <iomanip>

namespace cbdam {

  diamond_graph_builder::diamond_graph_builder() {

  }

  diamond_graph_builder::~diamond_graph_builder() {

  }

  diamond_graph_builder::diamond_graph_t* diamond_graph_builder::new_diamond_graph(const std::string& basename,
                                                                                   const vic::geo::map_sampler* s,
                                                                                   const coordinate_transform* geo_xform,
                                                                                   uint32_t patch_dim,
                                                                                   double min_sample_spacing,
										   bool reuse_graph,
										   bool keep_graph) {
    diamond_graph_builder::diamond_graph_t* result = 0;

#ifdef _WIN32
    SL_TRACE_OUT(-1) << "WIN32: Allocating diamond graph in main memory!" << std::endl;
    
    result = new diamond_graph_t();
#else
    std::string db_mode;
    if (reuse_graph) {
      db_mode = "r+";
    } else {
      if (keep_graph) {
	db_mode == "w";
      } else {
	db_mode = "t";
      }
    }
    SL_TRACE_OUT(-1) << "Allocating diamond graph out of core: basename=" << basename << ", db_mode " << db_mode << std::endl;
    result = new diamond_graph_t(basename, db_mode); // PERSISTENT
#endif

    /// Create roots
    if (!reuse_graph) {
      if (geo_xform->is_planar()) {
	result->canonical_init_planar();
      } else if (dynamic_cast<const cylindrical_coordinate_transform*>(geo_xform) != 0) {
	result->canonical_init_cylindrical();
      } else {
	assert(dynamic_cast<const spherical_coordinate_transform*>(geo_xform));
	result->canonical_init_spherical();
      }
    } else {
      // sanity check: already existent graph, check consistency with projection
      if ((geo_xform->is_planar() && result->root_count() > 1) ||
	  (!geo_xform->is_planar() && result->root_count() ==1)) {
	result->clear();
      }	  
    }

    double stats_min_sampling_distance = 1e30;
    
    /// Traverse level by level, refine when needed
    for (std::size_t x_level = 0;
         x_level < result->level_count(); // Changes at each step
         ++x_level) {
      std::size_t scanned_diamonds = 0;
      for (diamond_graph_t::grid_diamond_map_const_iterator_t it = result->level_begin(x_level);
           it != result->level_end(x_level);
           ++it) {
        const diamond_t       x       = it->first;
        const diamond_state_t x_state = it->second;

        if (x_state.is_leaf()) {
          const point2d_t x0 = geo_xform->uv_from_grid(x.corner(0));
          const point2d_t x1 = geo_xform->uv_from_grid(x.corner(1));
          const point2d_t x2 = geo_xform->uv_from_grid(x.corner(2));
          const point2d_t x3 = geo_xform->uv_from_grid(x.corner(3));

          const double one_over_sqrt2              = 0.70710678118654752440;
          const double pq_samples_along_diagonal   = 2*patch_dim+1;
          const double x_diagonal_length           = geo_xform->uv_distance_between(x0, x2); // manage wrap around // const double x_diagonal_length = (x0-x2).two_norm(); 
          const double x_pq_sample_spacing         = x_diagonal_length / pq_samples_along_diagonal;
          const double x_pq_sample_spacing_refined = x_pq_sample_spacing * one_over_sqrt2;

          // Update stats
	  //	  std::cerr << "level " << x_level << ", diamond " << x.id() << ", diag len " << x_diagonal_length << ", xpq sam spac ref " << x_pq_sample_spacing_refined << ", min sam spac " << min_sample_spacing << std::endl;
          stats_min_sampling_distance = std::min(stats_min_sampling_distance, x_pq_sample_spacing);
		
          if (x_pq_sample_spacing_refined > min_sample_spacing) {
            // Check refine
	    //            aabox2d_t dbox; 
	    //            dbox.to(x0); dbox.merge(x1); dbox.merge(x2); dbox.merge(x3);
	    // box vector will contain generally one box. Two boxes when there is wrap around for spherical and cylindrical dataset
	    std::vector<aabox2d_t> bv;
	    geo_xform->uv_box_containing(bv, x0, x1, x2, x3);
	    if (bv.size() == 2) {
	      point2d_t p0 = bv[0][0];
	      point2d_t p1 = bv[0][1];
	      std::cerr << "bv[" << 0 << "] = (" << p0[0] << "," << p0[1]  << ") <-> (" << p1[0] << ", " << p1[1] << ")" << std::endl;
	      p0 = bv[1][0];
	      p1 = bv[1][1];
	      std::cerr << "bv[" << 1 << "] = (" << p0[0] << "," << p0[1]  << ") <-> (" << p1[0] << ", " << p1[1] << ")" << std::endl;
	      std::cerr << "x0(" << x0[0] << "," << x0[1]  << "), x1(" << x1[0] << ", " << x1[1] << ")" << " ";
	      std::cerr << "x2(" << x2[0] << "," << x2[1]  << "), x1(" << x3[0] << ", " << x3[1] << ")" << std::endl;
	    }
	    double target_sampling_distance = 1e30;
	    for(std::size_t i = 0; i < bv.size(); ++i) {
	      if (bv.size() == 2) {
		
	      }
	      aabox2d_t::vector_t d = bv[i].diagonal();
	      bool null_box = (d[0]*d[1] == 0);
	      if (!null_box) {
		target_sampling_distance  = std::min(target_sampling_distance,
						     s->minimum_sample_spacing(bv[i], std::max(min_sample_spacing, 
											       x_pq_sample_spacing_refined)));
	      } else {
		std::cerr << "NULL BOX" << std::endl;
	      }
	    }
            
            const bool subdivide =
              (x_pq_sample_spacing_refined >= target_sampling_distance) ||
              ((x_pq_sample_spacing > target_sampling_distance) &&
               (x_pq_sample_spacing-target_sampling_distance>target_sampling_distance-x_pq_sample_spacing_refined));
          
	    //	    std::cerr << "bv size " << bv.size() << ", target_sampling_distance " << target_sampling_distance << ", subdivide " << subdivide << std::endl;
            if (subdivide) {
              result->refine(x_level, x.id()); 
            } 
          }
        }

        ++scanned_diamonds;
        if (scanned_diamonds % 1000 == 0) {
          std::cerr <<
            "LEVEL " << std::setw(2) << x_level << ": "
            "[" << sl::human_readable_percent(100.0f*scanned_diamonds/float(result->level_diamond_count(x_level))) << "]" <<
            "  Sampling distance: " << std::setw(6) << std::setprecision(3) << stats_min_sampling_distance << " - Total diamonds = " << std::setw(9) << sl::human_readable_quantity(result->diamond_count()) << "    \r";
        }
      }
      
      std::cerr <<
        "LEVEL " << x_level << ": "
        "[" << sl::human_readable_percent(100.0f) << "]" <<
        "  Sampling distance: " << std::setw(6) << std::setprecision(3) << stats_min_sampling_distance << " - Total diamonds = " << std::setw(9) << sl::human_readable_quantity(result->diamond_count()) << "      " << std::endl;
    }
    std::cerr << "DONE." << std::endl;

    return result;
  }

} // namespace cbdam
