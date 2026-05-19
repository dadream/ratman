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
#ifndef CBDAM_PRIORITY_DIAMOND
#define CBDAM_PRIORITY_DIAMOND

#include <vic/cbdam/base/grid_point.hpp>

namespace cbdam {
  
  /**
   *
   */
  class priority_diamond {
  public:
    typedef grid_point_t          diamond_id_t;

  protected:
    float               priority_;
    bool                visible_;
    std::size_t         level_;
    
  public:
    priority_diamond() :
        priority_(0), visible_(false) {
      
    }
    

    priority_diamond(float priority, bool visible, std::size_t level) :
        priority_(priority), visible_(visible), level_(level) {

    }

    ~priority_diamond() {

    }
    
    void set_priority(float priority) {
      priority_ = priority;
    }
    
    float priority() const {
      return priority_;
    }

    void set_visible(bool visible) {
      visible_ = visible;
    }
    
    bool visible() const {
      return visible_;
    }

    void set_level(std::size_t level) {
      level_ = level;
    }
    
    std::size_t level() const {
      return level_;
    }
  };


  struct refine_less {
    bool operator()(const std::pair<priority_diamond,grid_point_t>& lhs,
                    const std::pair<priority_diamond,grid_point_t>& rhs) const {
      if (!lhs.first.visible() && rhs.first.visible()) {
        return true;
      } else if (lhs.first.visible() && !rhs.first.visible()) {
        return false;
      } else {
        // both visible or both occluded
        if (lhs.first.priority() < rhs.first.priority()) {
          return true;
        } else if (lhs.first.priority() == rhs.first.priority()) {
          return lhs.second < rhs.second;
        } else {
          return false;
        }
      }
    }
  };
  
  // operator< for coarsening_heap_ where I want data with lower error to be on the top of the q.
  // !visible data must stay always on the bottom of the q.
  struct  coarsen_less {
    bool operator()(const std::pair<priority_diamond,grid_point_t>& lhs,
                    const std::pair<priority_diamond,grid_point_t>& rhs) const {
      struct refine_less rl;
      return rl(rhs, lhs);
    }
  };
    


} // namespace cbdam 

#endif // CBDAM_PRIORITY_DIAMOND

#ifndef CBDAM_PRIORITY_DIAMOND_IPP
#define CBDAM_PRIORITY_DIAMOND_IPP

namespace cbdam {

} // namespace cbdam 

#endif // CBDAM_PRIORITY_DIAMOND_IPP
