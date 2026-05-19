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
#ifndef CBDAM_TEXTURE_REFINER_HPP
#define CBDAM_TEXTURE_REFINER_HPP

#include <vic/cbdam/base/config.hpp>
#include <vic/cbdam/base/grid_diamond_graph_incore.hpp>

namespace cbdam {
  
  /**
   *
   */
  class texture_refiner {
  public:
    texture_refiner();

    virtual ~texture_refiner();

    virtual double error(std::size_t level, const grid_point_t& diamond_id) const = 0;
    
  protected:
    
  };

  class texture_refiner_grid_diamond_graph : public texture_refiner {
  public:
    typedef grid_diamond	grid_diamond_t;

  protected:
    grid_diamond_graph_incore* diamond_graph_;
    
  public:
    texture_refiner_grid_diamond_graph(grid_diamond_graph_incore* dg);

    virtual ~texture_refiner_grid_diamond_graph();

    virtual double error(std::size_t texture_level, const grid_point_t& diamond_id) const;
    
  protected:
    std::size_t max_reached_level(std::size_t geo_level, grid_diamond_t diamond) const;
  };




} // namespace cbdam 

#endif // CBDAM_TEXTURE_REFINER_HPP

#ifndef CBDAM_TEXTURE_REFINER_IPP
#define CBDAM_TEXTURE_REFINER_IPP

namespace cbdam {

  inline texture_refiner::texture_refiner() {

  }

  inline texture_refiner::~texture_refiner() {

  }
  
} // namespace cbdam 

#endif // CBDAM_TEXTURE_REFINER_IPP
