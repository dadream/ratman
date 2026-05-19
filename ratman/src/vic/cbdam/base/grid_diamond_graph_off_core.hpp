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
#ifndef CBDAM_GRID_DIAMOND_GRAPH_OFF_CORE_HPP
#define CBDAM_GRID_DIAMOND_GRAPH_OFF_CORE_HPP

#include <vic/cbdam/base/grid_diamond_graph.hpp>
#include <vic/persistent/map.hpp> // std::map on top of Berkeley DB

namespace cbdam {
  
  /**
   *  In-core graph of diamonds implemented with std::map 
   */
  template <class GRID_DIAMOND_STATE_T = grid_diamond_state>
  class grid_diamond_graph_off_core: 
    public grid_diamond_graph_base< vic::persistent::map<grid_diamond, GRID_DIAMOND_STATE_T>, GRID_DIAMOND_STATE_T> {
  public:
    typedef grid_point_t          diamond_id_t;
    typedef grid_diamond          grid_diamond_t;
    typedef GRID_DIAMOND_STATE_T  grid_diamond_state_t;
    typedef vic::persistent::map<grid_diamond, grid_diamond_state_t>  grid_diamond_map_t;

  protected:
    std::string db_mode_; // t=temporary, w=persistent, empty, w+=persistent
    std::string db_basename_; // FIXME
    
  protected: // Diamond map creation and destruction

    std::string db_basename(std::size_t level) const {
      return db_basename_ + "_dgraphdb_" + sl::to_string(level) + ".db";
    }
    
    
    // Create a new level
    virtual void push_level_map() {
      std::size_t level = this->diamond_map_by_level_.size();
      SL_TRACE_OUT(1) << "Creating " << db_basename(level) << std::endl;
      bool persistent = !sl::matches(db_mode_.c_str(), "t");
      grid_diamond_map_t* dmap = new grid_diamond_map_t(db_basename(level).c_str(), 
							(persistent ? "w" : "t"));
      
      this->diamond_map_by_level_.push_back(dmap);
    }

    // Delete last level map
    virtual void pop_level_map() {
      assert(this->diamond_map_by_level_.size()>0);
      
      grid_diamond_map_t* dmap = this->diamond_map_by_level_.back();
      this->diamond_map_by_level_.pop_back();
      std::size_t level = this->diamond_map_by_level_.size();

      SL_TRACE_OUT(1) << "Deleting " << db_basename(level) << std::endl;
      delete dmap; // This will close db and remove file
    }
    
  public:
    
    // Creation
    inline grid_diamond_graph_off_core(const std::string& basename,
				       const std::string& mode = "t") {
      SL_TRACE_OUT(1) << "Reopening " << basename << " mode " << mode << std::endl;
      db_mode_ = mode;
      db_basename_ = basename;
      if (sl::matches(db_mode_.c_str(), "r+*") ||
	  sl::matches(db_mode_.c_str(), "w+*")) {
	std::size_t level = 0;
	bool done = false;
	while (!done) {
	  std::string fname = db_basename(level);
	  grid_diamond_map_t* dmap = new grid_diamond_map_t(fname.c_str(), db_mode_);
	  if (dmap->is_open()) {
	    // Exists!
	    SL_TRACE_OUT(1) << "Reopening " << fname << " at level " << level << std::endl;
	    this->diamond_map_by_level_.push_back(dmap);
	    ++level;
	  } else {
	    done = true;
	  }
	}
      }
    }

    // Destruction
    virtual inline ~grid_diamond_graph_off_core() {
      this->clear();
    }

    // Save
    virtual inline void commit() {
      for(typename std::vector<grid_diamond_map_t*>::iterator it = this->diamond_map_by_level_.begin();
	  it != this->diamond_map_by_level_.end();
	  ++it) {
	std::cerr << "level diamonds " << (*it)->size() << " ";
	(*it)->close();
	(*it)->reopen();	
	std::cerr << " vs " << (*it)->size() << std::endl;
      }
    }

  };
} // namespace cbdam


#endif // CBDAM_GRID_DIAMOND_GRAPH_OFF_CORE_HPP
