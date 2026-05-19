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
#include <vic/cbdam/base/geometry_layer.hpp>

namespace cbdam {

  geometry_layer::geometry_layer(geometry_fetcher_t*  gf) {
    geometry_fetcher_ = gf;
    diamond_graph_.set_cache_capacity(2048); // FIXME
    diamond_graph_.set_decoded_diamond_budget(32);
    diamond_graph_.open(geometry_fetcher_);
  
    if (!diamond_graph_.is_open()) {
      std::cerr << "failed to open geometry " << gf->base_url() << std::endl;
    } else {
      diamond_graph_.init_heaps(); // FIXME
    }
  }
    
  geometry_layer::~geometry_layer() {
    diamond_graph_.clear();

    if (geometry_fetcher_  != 0) {
      delete geometry_fetcher_;
      geometry_fetcher_ = 0;
    }
  }

}
