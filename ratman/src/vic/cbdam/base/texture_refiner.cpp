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
#include <vic/cbdam/base/texture_refiner.hpp>

#ifdef _WIN32
  #undef max
  #undef min
#endif



namespace cbdam {

  texture_refiner_grid_diamond_graph::texture_refiner_grid_diamond_graph(grid_diamond_graph_incore* dg) {
    diamond_graph_ = dg;
  }

  texture_refiner_grid_diamond_graph::~texture_refiner_grid_diamond_graph() {

  }

  double texture_refiner_grid_diamond_graph::error(std::size_t texture_level, const grid_point_t& diamond_id) const {
    assert(diamond_graph_);

    std::size_t geo_level = texture_level * 2;

    std::pair<bool,grid_diamond_graph_incore::grid_diamond_map_const_iterator_t> it = diamond_graph_->diamond_at(geo_level, diamond_id);
    if (!it.first) {
      // We are more refined than terrain, coarsen!
      return -2.0;
    } else if (!diamond_graph_->is_visible(it.second->second.bounding_volume())) {
      // We are out of view frustum, coarsen!
      return -1.0;
    } else {
      // We are coarser or just at terrain resolution. Compute error
      std::size_t mrl = max_reached_level(geo_level, it.second->first);
      double er =  mrl / 2  - texture_level;
      //      std::cerr << "texture_level " << texture_level << " diamond_id " << diamond_id << "max reached level " << mrl <<  " error " << er << std::endl;
      return er;
    }
  }
  
  std::size_t texture_refiner_grid_diamond_graph::max_reached_level(std::size_t geo_level,
                                                                    grid_diamond_t diamond) const {
    assert(diamond_graph_->has(geo_level, diamond.id()));

    std::size_t result = geo_level;
    std::size_t child_level = geo_level+2;
    for(int r = 0; r < 2; ++r) {
      for(int c = 0; c < 2; ++c) {
	grid_diamond_t d_child = diamond.quad_child_diamond(r,c);
	if (diamond_graph_->has(child_level, d_child.id())) {
	  result = std::max(result,  max_reached_level(child_level, d_child));
	}
      }
    }
    return result;
  }
  
}
