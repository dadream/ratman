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
#ifndef CBDAM_DIAMOND_GRAPH_BUILDER_HPP
#define CBDAM_DIAMOND_GRAPH_BUILDER_HPP

#include <vic/cbdam/base/config.hpp>
#include <vic/cbdam/base/coordinate_transform.hpp>
#include <vic/cbdam/base/grid_diamond.hpp>
#include <vic/cbdam/geo/map_sampler.hpp>

#include <vic/cbdam/base/grid_diamond_graph.hpp> 
#ifndef _WIN32
#include <vic/cbdam/base/grid_diamond_graph_off_core.hpp>
#endif

namespace cbdam {
  
  /**
   *
   */
  class diamond_graph_builder {
  public:
    // FIXME - For now, off core graph is supported only on Linux
    // (Just for backward compat. reasons)

    typedef grid_diamond_state  diamond_state_t;    
#ifdef _WIN32
    typedef grid_diamond_graph<diamond_state_t>		 diamond_graph_t;
#else
    typedef grid_diamond_graph_off_core<diamond_state_t> diamond_graph_t;
#endif
    typedef grid_point_t	diamond_id_t;
    typedef grid_diamond        diamond_t;

    
  public:
    diamond_graph_builder();

    ~diamond_graph_builder();

    diamond_graph_t* new_diamond_graph(const std::string& basename,
                                       const vic::geo::map_sampler* s,
                                       const coordinate_transform* geo_xform,
                                       uint32_t patch_dim,
                                       double min_sample_spacing,
				       bool reuse_graph,
				       bool keep_graph);
    
  protected:
  
  };


} // namespace cbdam 

#endif // CBDAM_DIAMOND_GRAPH_BUILDER_HPP

#ifndef CBDAM_DIAMOND_GRAPH_BUILDER_IPP
#define CBDAM_DIAMOND_GRAPH_BUILDER_IPP

namespace cbdam {

} // namespace cbdam 

#endif // CBDAM_DIAMOND_GRAPH_BUILDER_IPP
