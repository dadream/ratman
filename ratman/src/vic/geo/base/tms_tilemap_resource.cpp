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
#include <vic/geo/base/tms_tilemap_resource.hpp>
#include <sstream>
#include <iostream>
namespace vic {

  namespace geo {

    namespace base {

      tms_tilemap_resource::tms_tilemap_resource(const std::string& xml_description) {
	xml_parse_string(xml_description,std::string("TileMap"));
      }

      tms_tilemap_resource::~tms_tilemap_resource() {
	clear();
      }

      void tms_tilemap_resource::xml_parse(vic::xml::node_iterator ptr) {
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
	    service_url_=ptr.attribute("tilemapservice");
	    if (ptr.error()) {
	      fail(ptr.error_msg());
	    } else {
	      for(vic::xml::node_iterator child_it=ptr.down();
		  !child_it.is_null() && last_operation_success_;
		  child_it = child_it.next()) {
		xml_parse_tilemap_element(child_it);
	      }
	    }
	  }
	}
	if (!last_operation_success_) {
	  clear();
	}
      }

      void tms_tilemap_resource::xml_parse_tilemap_element(vic::xml::node_iterator ptr) {
	if (ptr.is_null()) {
	  fail("TileMap is null");
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
	} else if (ptr.tag() == "SRS") {
	  srs_="";
	  vic::xml::node_iterator child_it=ptr.down();
	  if (!child_it.is_null()) {
	    srs_=child_it.text();
	  }
	} else if (ptr.tag() == "BoundingBox") {
	  point2_t min_p;
	  min_p[0]=ptr.attributed("minx");
	  if (ptr.error()) {
	    fail(ptr.error_msg());
	  } else {
	    min_p[1]=ptr.attributed("miny");
	    if (ptr.error()) {
	      fail(ptr.error_msg());
	    } else {
	      point2_t max_p;
	      max_p[0]=ptr.attributed("maxx");
	      if (ptr.error()) {
		fail(ptr.error_msg());
	      } else {
		max_p[1]=ptr.attributed("maxy");
		if (ptr.error()) {
		  fail(ptr.error_msg());
		} else {
		  bounding_box_=aabox2_t(min_p,max_p);
		}
	      }
	    }
	  }
	} else if (ptr.tag() == "Origin") {
	  point2_t p0;
	  p0[0]=ptr.attributed("x");
	  if (ptr.error()) {
	    fail(ptr.error_msg());
	  } else {
	    p0[1]=ptr.attributed("y");
	    if (ptr.error()) {
	      fail(ptr.error_msg());
	    } else {
	      origin_=p0;
	    }
	  }
	} else if (ptr.tag() == "TileFormat") {
	  img_width_=ptr.attributei("width");
	  if (ptr.error()) {
	    fail(ptr.error_msg());
	  } else {
	    img_height_=ptr.attributei("height");
	    if (ptr.error()) {
	      fail(ptr.error_msg());
	    } else {
	      img_mime_=ptr.attribute("mime-type");
	      if (ptr.error()) {
		fail(ptr.error_msg());
	      } else {
		img_extension_=ptr.attribute("extension");
		if (ptr.error()) {
		  fail(ptr.error_msg());
		}
	      }
	    }
	  }
	} else if (ptr.tag() == "TileSets") {
	  tileset_profile_=ptr.attribute("profile");
	  if (ptr.error()) {
	    tileset_profile_=std::string("global-geodetic");
	  }     
	  for(vic::xml::node_iterator child_it=ptr.down();
	      !child_it.is_null() && last_operation_success_;
	      child_it = child_it.next()) {
	    xml_parse_tileset_element(child_it);
	  }
	}
      }

      void tms_tilemap_resource::xml_parse_tileset_element(vic::xml::node_iterator ptr) {
	if (ptr.is_null()) {
	  fail("TileSets is null");
	} else if (!ptr.is_element_node()) {
	  fail("TileSets children is not an element node");
	} else if (ptr.tag() == "TileSet") {
	  std::string url=ptr.attribute("href");
	  if (ptr.error()) {
	    fail(ptr.error_msg());
	  } else {
	    double upp=ptr.attributed("units-per-pixel");
	    if (ptr.error()) {
	      fail(ptr.error_msg());
	    } else {
	      insert_tileset(url,upp);
	    }
	  }
	}
      }

      std::string tms_tilemap_resource::description() const {
	std::ostringstream os;	
	os << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>" << std::endl;
	os << "<TileMap" 
	   << " version=\"" << version_ << "\""
	   << " tilemapservices=\"" << service_url_ << "\""
	   << " />" << std::endl;
	os << "\t<Title>" << title_ << "</Title>" << std::endl;
	os << "\t<Abstract>" << abstract_ << "</Abstract>" << std::endl;
	os << "\t<SRS>" << srs_ << "</SRS>" << std::endl;
	os << "\t<BoundingBox" 
	   << " minx=\"" << bounding_box_[0][0] << "\""
	   << " miny=\"" << bounding_box_[0][1] << "\""
	   << " maxx=\"" << bounding_box_[1][0] << "\""
	   << " maxy=\"" << bounding_box_[1][1] << "\""
	   << " />" << std::endl;
	os << "\t<Origin" 
	   << " x=\"" << origin_[0] << "\""
	   << " y=\"" << origin_[1] << "\""
	   << " />" << std::endl;
	os << "\t<TileFormat" 
	   << " width=\"" << img_width_ << "\""
	   << " height=\"" << img_height_ << "\""
	   << " mime-type=\"" << img_mime_ << "\""
	   << " extension=\"" << img_extension_ << "\""
	   << " />" << std::endl;
	os << "\t<TileSets" 
	   << " profile=\"" << tileset_profile_ << "\""
	   << " />" << std::endl;
	for (std::size_t i=0; i<tileset_count(); ++i) {
	  os << "\t\t<TileSet" << std::endl
	     << "\t\t href=\"" << tileset_url_[i] << "\"" << std::endl
	     << "\t\t units-per-pixel=\"" << tileset_units_per_pixel_[i] << "\"" << std::endl
	     << "\t\t order=\"" << i << "\""
	     << " />" << std::endl;	    
	}
	os << "\t</TileSets>" << std::endl;
	os << "</TileMap>" << std::endl;
	return os.str();
      }

      void tms_tilemap_resource::clear() {
	title_="TileMap";
	abstract_="TileMap Default Abstract";
	tileset_url_.clear();
	tileset_units_per_pixel_.clear();
      }

      std::size_t tms_tilemap_resource::tileset_count() const {
	return tileset_url_.size();
      }
      
      const std::string& tms_tilemap_resource::tileset_url(std::size_t i) const {
	return tileset_url_[i];
      }

      double tms_tilemap_resource::tileset_units_per_pixel(std::size_t i) const {
	return tileset_units_per_pixel_[i];
      }

 
      void tms_tilemap_resource::insert_tileset(const std::string& url,
						 double units_per_pixel) {
	tileset_url_.push_back(url);
	tileset_units_per_pixel_.push_back(units_per_pixel);
      }

    }
  }
}

