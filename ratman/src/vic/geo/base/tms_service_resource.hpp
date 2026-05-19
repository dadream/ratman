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
#ifndef VIC_GEO_BASE_TMS_SERVICE_RESOURCE_HPP
#define VIC_GEO_BASE_TMS_SERVICE_RESOURCE_HPP

#include <vic/geo/base/tms_resource.hpp>
#include <string>
#include <vector>

namespace vic {

  namespace geo {

    namespace base {

      class tms_service_resource: public tms_resource {
      public:
	
      protected:
	std::string version_;
	std::string root_url_;
	std::string title_;
	std::string abstract_;

	std::vector<std::string> tilemap_title_;
	std::vector<std::string> tilemap_srs_;
	std::vector<std::string> tilemap_profile_;
	std::vector<std::string> tilemap_url_;

	void xml_parse(vic::xml::node_iterator ptr);
	void xml_parse_tilemap_service_element(vic::xml::node_iterator ptr);
	void xml_parse_tilemap_element(vic::xml::node_iterator ptr);


      public:
	// <?xml version="1.0" encoding="UTF-8" ?>
	// <TileMapService version="1.0.0" services="http://tms.osgeo.org">
	// <Title>Example Tile Map Service</Title>
	// <Abstract>This is a longer description of the example tiling map service.</Abstract>
	// <TileMaps>
	//   <TileMap 
	//     title="VMAP0 World Map" 
	//     srs="EPSG:4326" 
	//     profile="global-geodetic" 
	//     href="http://tms.osgeo.org/1.0.0/vmap0" />
	//   <TileMap 
	//     title="British Columbia Landsat Imagery (2000)" 
	//     srs="EPSG:3005" 
	//     profile="local" 
	//     href="http://tms.osgeo.org/1.0.0/landsat2000" />
	// </TileMaps>
	// </TileMapService>


	tms_service_resource(const std::string& xml_description);

	virtual ~tms_service_resource();

	virtual std::string description() const;

      public:

	void clear();

	inline const std::string& version() const {
	  return version_;
	}

	inline void set_version(const std::string& version) {
	  version_=version;
	}

	inline const std::string& root_url() const {
	  return root_url_;
	}

	inline void set_root_url(const std::string& root_url) {
	  root_url_=root_url;
	}

	inline const std::string& title() const {
	  return title_;
	}

	inline void set_title(const std::string& title) {
	  title_=title;
	}

	inline const std::string& abstract() const {
	  return abstract_;
	}

	inline void set_abstract(const std::string& abstract) {
	  abstract_=abstract;
	}

	std::size_t tilemap_count() const;

	const std::string& tilemap_title(std::size_t i) const;
	const std::string& tilemap_srs(std::size_t i) const;
	const std::string& tilemap_profile(std::size_t i) const;
	const std::string& tilemap_url(std::size_t i) const;

	void insert_tilemap(const std::string& title,
			    const std::string& srs,
			    const std::string& profile,
			    const std::string& url);

	
      };

    }
  }
}

#endif
