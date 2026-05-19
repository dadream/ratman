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
#ifndef VIC_RATMAN_GEONAMES_SERVICE_HPP
#define VIC_RATMAN_GEONAMES_SERVICE_HPP

#include <string>

#include <sl/fixed_size_point.hpp>
#include <sl/axis_aligned_box.hpp>

namespace ratman {

  /**
   *  A name associated to a geographic location
   */
  class geonames_entry {
  public:
    typedef sl::fixed_size_point<3,double> point3d_t;
  protected:
    point3d_t   location_;    /// longitude, latitude, elevation
    std::string name_;        /// The name
    std::string description_; /// Associated textual description
    std::string url_;         /// Hyperlink to associated data
  public:
    
    inline geonames_entry(const point3d_t& location,
			  const std::string& name,
			  const std::string& description = "", 
			  const std::string& url = "" ) :
      location_(location),
      name_(name),
      description_(description),
      url_(url) {
    }

    /// The location in WGS84 lon lat. Elevation set to -9999.0 if unknown
    inline const point3d_t& location() const { return location_; }
    
    /// The short name
    inline const std::string& name() const { return name_; }
    
    /// The description
    inline const std::string& description() const { return description_; }
    
    /// The url
    inline const std::string& url() const { return url_; }
    
  };
  
  /**
   *  A simple geographic search engine interface. Subclasses
   *  implement specific services, e.g., based on WFS.
   *
   *  Example
   *
   *   gazetteer = new WFS_placename_search("http://www.test.it/....",
   *                                        "citynames",
   *                                        ...);
   *   // get all names matching cagliari... 
   *   gazetteer->search("cagliari");
   *   if (!gazetteer->last_search_ok()) {
   *     ... connection error ... 
   *   } else {
   *     for (std::size_t i=0; i<gazetteer->last_search_results().size(); ++i) {
   *       ... do something with last_search_results()[i]
   *     }
   *   }
   *
   *   ...
   *   // Get all names in a lon/lat box
   *   gazeteer->search("",
   *                    aabox2d_t(point2d_t(6.0, 13.0),
   *                              point2d_t(6.1, 13.1)));
   *   ... 
   */
  class geonames_search {
  public:
    typedef sl::fixed_size_point<2,double> point2d_t;
    typedef sl::fixed_size_point<3,double> point3d_t;
    typedef sl::axis_aligned_box<2,double> aabox2d_t;
  protected:
    std::string                 base_url_;
    std::string                 id_;
    std::vector<geonames_entry> last_search_results_;
    bool                        last_search_ok_;
    std::string  classification_tag_;
    std::string  classification_legend_tag_;
    bool using_classification_legend_;
  public:
    
    geonames_search(const std::string& url, const std::string& id, 
    const std::string & classification_tag = "", const std::string & classification_legend_tag = "", 
    bool using_classification_legend = false) :
      base_url_(url),
      id_(id),
      classification_tag_(classification_tag),
      classification_legend_tag_(classification_legend_tag),
      using_classification_legend_(using_classification_legend){
    }
    
    virtual ~geonames_search() {
    }
    
    /**
     *  Searches for information about a placename given
     *  the arguments. query_name contains the search string,
     *  query_box the WGS84 bounding box of the search area,
     *  query_max_rows the maximum number of placenames 
     *  returned, query_start_row is used for paging results
     *  (if you want to get results 30 to 40, use start_row=30 and max_rows=10).
     *  On exit, consult last_search_ok and last_search_results to read the response.
     */
    virtual void search(const std::string& query_name,
			const aabox2d_t& query_box = aabox2d_t(point2d_t(-180.0,-90.0),
							       point2d_t( 180.0, 90.0)),
			std::size_t query_max_rows = 1000,
			std::size_t query_start_row = 0) = 0;
    
    virtual inline void search_box(const aabox2d_t& query_box = aabox2d_t(point2d_t(-180.0,-90.0),
									  point2d_t( 180.0, 90.0))) {
      return search("", query_box);
    }
    
    
    inline std::string classification_legend_tag(){
      return classification_legend_tag_;    	
    }

    /// True iff last search was performed without errors
    inline bool last_search_ok() const {
      return last_search_ok_;
    }
    
    inline bool using_classification_legend() const {
      return using_classification_legend_;
    }
    
    /// The table containing the results of last search
    inline const std::vector<geonames_entry>& last_search_results() const {
      return last_search_results_;
    }
    
    inline const std::string& id() const {
      return id_;
    }
    
  };
  
} // namespace ratman

#endif


