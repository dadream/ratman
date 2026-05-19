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
#ifndef VIC_GEO_BASE_TMS_ROOT_RESOURCE_HPP
#define VIC_GEO_BASE_TMS_ROOT_RESOURCE_HPP

#include <vic/geo/base/tms_resource.hpp>
#include <vector>
#include <string>

namespace vic {

  namespace geo {

    namespace base {

      class tms_root_resource: public tms_resource {
      public:
	
      protected:
	std::vector<std::string> service_title_;
	std::vector<std::string> service_version_;
	std::vector<std::string> service_url_;

	void xml_parse(vic::xml::node_iterator ptr);
	void xml_parse_service(vic::xml::node_iterator ptr);

      public:
	// <?xml version="1.0" ?>
	// <Services>
	// <TileMapService title="Example Static Tile Map Service" version="1.0.0" href="http://www.osgeo.org/services/tilemapservice.xml" />
	// </Services>

	tms_root_resource(const std::string& xml_description);

	virtual ~tms_root_resource();

	virtual std::string description() const;

      public:

	void clear();

	std::size_t service_count() const;
	
	const std::string& service_title(std::size_t i) const;
	const std::string& service_version(std::size_t i) const;
	const std::string& service_url(std::size_t i) const;

	void insert_service(const std::string& title,
			    const std::string& version,
			    const std::string& url);

	
      };

    }
  }
}

#endif
