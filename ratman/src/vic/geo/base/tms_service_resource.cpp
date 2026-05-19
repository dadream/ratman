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
#include <vic/geo/base/tms_service_resource.hpp>
#include <sstream>
#include <iostream>
namespace vic {

  namespace geo {

    namespace base {

      tms_service_resource::tms_service_resource(const std::string& xml_description) {
	xml_parse_string(xml_description,std::string("TileMapService"));
      }

      tms_service_resource::~tms_service_resource() {
	clear();
      }

      void tms_service_resource::xml_parse(vic::xml::node_iterator ptr) {
	clear();
	if (ptr.is_null()) {
	  fail("Root is null");
	} else if (!ptr.is_element_node()) {
	  fail("Root is not an element node");
	} else {
	  version_=ptr.attribute("version");
	  if (ptr.error()) {
	    fail(ptr.error_msg());
	  } else {
	    root_url_=ptr.attribute("services");
	    if (ptr.error()) {
	      fail(ptr.error_msg());
	    } else {
	      for(vic::xml::node_iterator child_it=ptr.down();
		  !child_it.is_null() && last_operation_success_;
		  child_it = child_it.next()) {
		xml_parse_tilemap_service_element(child_it);
	      }
	    }
	  }
	}
	if (!last_operation_success_) {
	  clear();
	}
      }

      void tms_service_resource::xml_parse_tilemap_service_element(vic::xml::node_iterator ptr) {
	if (ptr.is_null()) {
	  fail("TileMapService is null");
	} else if (!ptr.is_element_node()) {
	  fail("TileMapService is not an element node");
	} else if (ptr.tag() == "Title") {
	  title_="";
	  vic::xml::node_iterator child_it=ptr.down();
	  if (!child_it.is_null()) {
	    title_=child_it.text();
	  }
	} else if (ptr.tag() == "Abstract") {
	  abstract_="";
	  vic::xml::node_iterator child_it=ptr.down();
	  if (!child_it.is_null()) {
	    abstract_=child_it.text();
	  }
	} else if (ptr.tag() == "TileMaps") {
	  for(vic::xml::node_iterator child_it=ptr.down();
	      !child_it.is_null() && last_operation_success_;
	      child_it = child_it.next()) {
	    xml_parse_tilemap_element(child_it);
	  }
	}
      }

      void tms_service_resource::xml_parse_tilemap_element(vic::xml::node_iterator ptr) {
	if (ptr.is_null()) {
	  fail("TileMaps is null");
	} else if (!ptr.is_element_node()) {
	  fail("TileMaps children is not an element node");
	} else if (ptr.tag() == "TileMap") {
	  std::string title=ptr.attribute("title");
	  if (ptr.error()) {
	    fail(ptr.error_msg());
	  } else {
	    std::string srs=ptr.attribute("srs");
	    if (ptr.error()) {
	      fail(ptr.error_msg());
	    } else {
	      std::string profile=ptr.attribute("profile");
	      if (ptr.error()) {
		fail(ptr.error_msg());
	      } else {
		std::string url=ptr.attribute("href");
		if (ptr.error()) {
		  fail(ptr.error_msg());
		} else {
		  insert_tilemap(title, srs, profile, url);
		}
	      }
	    }
	  }
	}
      }

      std::string tms_service_resource::description() const {
	std::ostringstream os;	
	os << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>" << std::endl;
	os << "<TileMapService" 
	   << " version=\"" << version_ << "\""
	   << " services=\"" << root_url_ << "\""
	   << " />" << std::endl;
	os << "\t<Title>" << title_ << "</Title>" << std::endl;
	  os << "\t<Abstract>" << abstract_ << "</Abstract>" << std::endl;
	os << "\t<TileMaps>" << std::endl;
	for (std::size_t i=0; i<tilemap_count(); ++i) {
	  os << "\t\t<TileMap" << std::endl
	     << "\t\t title=\"" << tilemap_title_[i] << "\"" << std::endl
	     << "\t\t srs=\"" << tilemap_srs_[i] << "\"" << std::endl
	     << "\t\t profile=\"" << tilemap_profile_[i] << "\"" << std::endl
	     << "\t\t href=\"" << tilemap_url_[i] << "\"" 
	     << " />" << std::endl;	    
	}
	os << "\t</TileMaps>" << std::endl;
	os << "</TileMapService>" << std::endl;
	return os.str();
      }

      void tms_service_resource::clear() {
	title_="Tile Map Service";
	abstract_="Tile Map Service Default Abstract";
	tilemap_title_.clear();
	tilemap_srs_.clear();
	tilemap_profile_.clear();
	tilemap_url_.clear();
      }

      std::size_t tms_service_resource::tilemap_count() const {
	return tilemap_title_.size();
      }
      
      const std::string& tms_service_resource::tilemap_title(std::size_t i) const {
	return tilemap_title_[i];
      }

      const std::string& tms_service_resource::tilemap_srs(std::size_t i) const {
	return tilemap_srs_[i];
      }
 
      const std::string& tms_service_resource::tilemap_profile(std::size_t i) const {
	return tilemap_profile_[i];
      }

      const std::string& tms_service_resource::tilemap_url(std::size_t i) const {
	return tilemap_url_[i];
      }

 
      void tms_service_resource::insert_tilemap(const std::string& title,
						const std::string& srs,
						const std::string& profile,
						const std::string& url) {
	tilemap_title_.push_back(title);
	tilemap_srs_.push_back(srs);
	tilemap_profile_.push_back(profile);
	tilemap_url_.push_back(url);
      }
    }
  }
}

