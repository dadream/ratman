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
#include <vic/ratman/bookmarks_service.hpp>
#include <vic/ratman/string_utility.hpp>
#include <QString>

namespace ratman {

  bookmarks_service::bookmarks_service(const std::string& url, const std::string& id, bookmarks* b) :
    geonames_search(url, id) {
    load_bookmarks(b);
  }

  void bookmarks_service::load_bookmarks(bookmarks* b) {
    bookmarks_.clear();
    for ( std::size_t i=0; i<b->count(); i++ ){
      point3d_t p = point3d_t(b->location(i)[0],b->location(i)[1],b->location(i)[2]);
      std::string n = b->name(i).toStdString();
      append_geoname(p,n);
    }
  }
  

  void bookmarks_service::append_geoname(const point3d_t& location,
					     const std::string& name,
					     const std::string& description, 
					     const std::string& url) {
    bookmarks_.push_back(geonames_entry(location, name, description, url));
  }
  
  void bookmarks_service::search(const std::string& query_name,
				     const aabox2d_t& query_box,
				     std::size_t query_max_rows,
				     std::size_t query_start_row) { 
    last_search_results_.clear();
    size_t search_count=0;

    bool has_str_query = (query_name != "");

    std::string str_query = std::string("*")+string_utility::to_lower(query_name)+std::string("*");
    
    for (std::size_t i=0; i<bookmarks_count() && last_search_results_.size()<query_max_rows; ++i) {
      point2d_t loc=point2d_t(geonames(i).location()[0], 
			      geonames(i).location()[1]);
      if (query_box.contains(loc)) {
	bool found = !(has_str_query) || sl::matches(string_utility::to_lower(geonames(i).name()),
						     str_query);
	if (found) {
	  if (search_count>=query_start_row) {
	    last_search_results_.push_back(geonames(i));
	  }
	  search_count++;
	}
      }
    }

    last_search_ok_=true;
  }
       
} // namespace ratman
