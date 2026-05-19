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
#ifndef VIC_RATMAN_LOCAL_GEONAMES_SERVICE_HPP
#define VIC_RATMAN_LOCAL_GEONAMES_SERVICE_HPP

#include <vic/ratman/geonames_service.hpp>
#include <cassert>

namespace ratman {

  /**
   * Perform search over a set of local geonames entry.
   */
  class local_geonames_search : public geonames_search {
  public:
  protected:
    std::vector<geonames_entry>       geonames_; // map id->label

  public:

    local_geonames_search(const std::string& url, const std::string& id);
    virtual void search(const std::string& query_name="",
			const aabox2d_t& query_box = aabox2d_t(point2d_t(-90.0,-180.0),
							       point2d_t( 90.0, 180.0)),
			std::size_t query_max_rows = 1000,
			std::size_t query_start_row = 0);

    void append_geoname(const point3d_t& location,
			const std::string& name,
			const std::string& description = "", 
			const std::string& url = "");

    inline std::size_t geonames_count() const {
      return geonames_.size();
    }

    inline bool is_good_geonames_index(std::size_t i) const {
      return i < geonames_count();
    }

    inline const geonames_entry& geonames(std::size_t i) {
      assert(is_good_geonames_index(i));
      return geonames_[i];
    }

  protected:
    void load_geonames();
      
  };

} // namespace ratman

#endif


