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
#ifndef CBDAM_GEOMETRY_LAYER_HPP
#define CBDAM_GEOMETRY_LAYER_HPP

#include <vic/cbdam/base/config.hpp>
#include <vic/cbdam/base/grid_diamond_graph_incore.hpp>
#include <vic/cbdam/base/cbdam_diamond_fetcher.hpp>

namespace cbdam {
  
  /**
   *
   */
  class geometry_layer {
  public:
    typedef grid_diamond_graph_incore  geometry_graph_t;
    typedef cbdam_diamond_fetcher      geometry_fetcher_t;

  protected:
    geometry_fetcher_t*  geometry_fetcher_;
    geometry_graph_t     diamond_graph_;
    
  public:
    geometry_layer(geometry_fetcher_t*  g);
    
    ~geometry_layer();

    grid_diamond_graph_incore& diamond_graph();

    const grid_diamond_graph_incore& diamond_graph() const;
    
    const geometry_fetcher_t* fetcher() const;

    geometry_fetcher_t* fetcher();

  protected:
  
  };


} // namespace cbdam 

#endif // CBDAM_GEOMETRY_LAYER_HPP

#ifndef CBDAM_GEOMETRY_LAYER_IPP
#define CBDAM_GEOMETRY_LAYER_IPP

namespace cbdam {

  inline grid_diamond_graph_incore& geometry_layer::diamond_graph() {
    assert(geometry_fetcher_);
    
    return diamond_graph_;
  }

  inline const grid_diamond_graph_incore& geometry_layer::diamond_graph() const {
    assert(geometry_fetcher_);

    return diamond_graph_;
  }

  inline const geometry_layer::geometry_fetcher_t* geometry_layer::fetcher() const {
    return geometry_fetcher_;
  }

  inline geometry_layer::geometry_fetcher_t* geometry_layer::fetcher() {
    return geometry_fetcher_;
  }

} // namespace cbdam 

#endif // CBDAM_GEOMETRY_LAYER_IPP
