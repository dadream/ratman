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
#include <vic/geo/base/tms_root_resource.hpp>
#include <sstream>

namespace vic {

  namespace geo {

    namespace base {

      tms_root_resource::tms_root_resource(const std::string& xml_description) {
	xml_parse_string(xml_description,std::string("Services"));
      }

      tms_root_resource::~tms_root_resource() {
	clear();
      }

      void tms_root_resource::xml_parse(vic::xml::node_iterator ptr) {
	clear();
	if (ptr.is_null()) {
	  fail("Root is null");
	} else if (!ptr.is_element_node()) {
	  fail("Root is not an element node");
	} else {
	  for(vic::xml::node_iterator child_it=ptr.down();
	      !child_it.is_null() && last_operation_success_;
	      child_it = child_it.next()) {
	    xml_parse_service(child_it);
	  }
	}

	if (!last_operation_success_) {
	  clear();
	}
      }

      void tms_root_resource::xml_parse_service(vic::xml::node_iterator ptr) {
	if (ptr.is_null()) {
	  fail("Service is null");
	} else if (!ptr.is_element_node()) {
	  fail("Service is not an element node");
	} else if (ptr.tag() == "TileMapService") {
	  std::string title=ptr.attribute("title");
	  if (ptr.error()) {
	    fail(ptr.error_msg());
	  } else {
	    std::string version=ptr.attribute("version");
	    if (ptr.error()) {
	      fail(ptr.error_msg());
	    } else {
	      std::string url=ptr.attribute("href");
	      if (ptr.error()) {
		fail(ptr.error_msg());
	      } else {
		insert_service(title, version, url);
	      }
	    }
	  }
	}	
      }

      std::string tms_root_resource::description() const {
	std::ostringstream os;	
	os << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>" << std::endl;
	os << "<Services>" << std::endl;
	for (std::size_t i=0; i<service_count(); ++i) {
	  os << "\t<TileMapService"
	     << " title=\"" << service_title_[i] <<"\""
	     << " version=\"" << service_version_[i] <<"\""
	     << " href=\"" << service_url_[i] <<"\""
	     << " />" << std::endl;	    
	}
	os << "</Services>" << std::endl;
	return os.str();
      }

      void tms_root_resource::clear() {
	service_title_.clear();
	service_version_.clear();
	service_url_.clear();
      }

      std::size_t tms_root_resource::service_count() const {
	return service_title_.size();
      }
      
      const std::string& tms_root_resource::service_title(std::size_t i) const {
	return service_title_[i];
      }

      const std::string& tms_root_resource::service_version(std::size_t i) const {
	return service_version_[i];
      }

      const std::string& tms_root_resource::service_url(std::size_t i) const {
	return service_url_[i];
      }
 
      void tms_root_resource::insert_service(const std::string& title,
					     const std::string& version,
					     const std::string& url) {
	service_title_.push_back(title);
	service_version_.push_back(version);
	service_url_.push_back(url);
      }

    }
  }
}
 
