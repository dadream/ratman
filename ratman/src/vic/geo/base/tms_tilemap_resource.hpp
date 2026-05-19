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
#ifndef VIC_GEO_BASE_TMS_TILEMAP_RESOURCE_HPP
#define VIC_GEO_BASE_TMS_TILEMAP_RESOURCE_HPP

#include <vic/geo/base/tms_resource.hpp>
#include <sl/axis_aligned_box.hpp>
#include <string>
#include <vector>

namespace vic {

  namespace geo {

    namespace base {

      class tms_tilemap_resource: public tms_resource {
      public:
	typedef sl::aabox2d aabox2_t;
	typedef sl::point2d point2_t;
      protected:
	std::string version_;
	std::string service_url_;
	std::string title_;
	std::string abstract_;
	std::string srs_;
	aabox2_t bounding_box_;
	point2_t origin_;
	std::size_t img_width_;
	std::size_t img_height_;
	std::string img_mime_;
	std::string img_extension_;

	std::string tileset_profile_;
	std::vector<std::string> tileset_url_;
	std::vector<double> tileset_units_per_pixel_;

	void xml_parse(vic::xml::node_iterator ptr);
	void xml_parse_tilemap_element(vic::xml::node_iterator ptr);
	void xml_parse_tileset_element(vic::xml::node_iterator ptr);

      public:
	// <?xml version="1.0" encoding="UTF-8" ?>  
	// <TileMap version="1.0.0" tilemapservice="http://http://tms.osgeo.org/1.0.0">
	// <Title>VMAP0 World Map</Title>
	// <Abstract>A map of the world built from the NGA VMAP0 vector data set.</Abstract>
	// <SRS>EPSG:4326</SRS>
	// <BoundingBox minx="-180" miny="-90" maxx="180" maxy="90" />
	// <Origin x="-180" y="-180" />  
	// <TileFormat width="256" height="256" mime-type="image/jpeg" extension="jpg" />
	// <TileSets profile=global-geodetic">
	//   <TileSet href="http://tms.osgeo.org/1.0.0/vmap0/0" units-per-pixel="0.703125" order="0" />
	//   <TileSet href="http://tms.osgeo.org/1.0.0/vmap0/1" units-per-pixel="0.3515625" order="1" />
	//   <TileSet href="http://tms.osgeo.org/1.0.0/vmap0/2" units-per-pixel="0.17578125" order="2" />
	//   <TileSet href="http://tms.osgeo.org/1.0.0/vmap0/3" units-per-pixel="0.08789063" order="3" />
	// </TileSets>
	// </TileMap>



	tms_tilemap_resource(const std::string& xml_description);

	virtual ~tms_tilemap_resource();

	virtual std::string description() const;

      public:

	void clear();

	inline const std::string& version() const {
	  return version_;	  
	}

	inline void set_version(const std::string& version) {
	  version_=version;
	}

	inline const std::string& service_url() const {
	  return service_url_;
	}

	inline void set_service_url(const std::string& service_url) {
	  service_url_=service_url;
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

	inline const std::string& srs() const {
	  return srs_;
	}

	inline void set_srs(const std::string& srs) {
	  srs_=srs;
	}

	inline const aabox2_t& bounding_box() const {
	  return bounding_box_;
	}

	inline void set_bounding_box(const aabox2_t& bounding_box) {
	  bounding_box_=bounding_box;
	}

	inline const point2_t& origin() const {
	  return origin_;
	}

	inline void set_origin(const point2_t& origin) {
	  origin_=origin;
	}

	inline std::size_t img_width() const {
	  return img_width_;
	}

	inline void set_img_width(size_t img_width) {
	  img_width_=img_width;
	}

	inline std::size_t img_height() const {
	  return img_height_;
	}

	inline void set_img_height(size_t img_height) {
	  img_height_=img_height;
	}

	inline const std::string& img_mime() const {
	  return img_mime_;
	}

	inline void set_img_mime(const std::string& img_mime) {
	  img_mime_=img_mime;
	}

	inline const std::string& img_extension() const {
	  return img_extension_;
	}

	inline void set_img_extension(const std::string& img_extension) {
	  img_extension_=img_extension;
	}

	std::size_t tileset_count() const;

	const std::string& tileset_url(std::size_t i) const;

	double tileset_units_per_pixel(std::size_t i) const;

	void insert_tileset(const std::string& url,
			    double units_per_pixel);

	
      };

    }
  }
}

#endif
