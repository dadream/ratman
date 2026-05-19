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
#include "tilemap_config.hpp"
#include <fstream>
#include <sstream>
#include <iomanip>

#if VIC_STANDALONE
#include "document.hpp"
#else
#include "vic/xml/document.hpp"
#endif

namespace vic {

  namespace geo {

    namespace base {

      tilemap_config::tilemap_config() {
	profile_="global-geodetic";
	mime_="image/jpeg";
	extension_="jpg";
	max_level_=26;
	srs_="EPSG:4326";
	bbox_lo_[0]=-180.0;
	bbox_lo_[1]=-90.0;
	bbox_hi_[0]= 180.0;
	bbox_hi_[1]= 90.0;
	nu_=2;
	nv_=1;
	img_width_=256;
	img_height_=256;
      }

      std::string tilemap_config::description() const {
	std::ostringstream out;
	out 
	  << "<tilemap " << std::endl
	  << "  name=\"" << name_  << "\"" << std::endl
	  << "  profile=\"" << profile_  << "\"" << std::endl
	  << "  mime-type=\"" << mime_  << "\"" << std::endl
	  << "  extension=\"" << extension_  << "\"" << std::endl
	  << "  max-level=\"" << max_level_  << "\"" << std::endl;
	if (profile_ == "global-geodetic") {
	  // All defaults
	} else if (profile_ == "global-mercator") {
	  // All defaults
	} else {
	  out 
	    << "  srs=\"" << srs_  << "\"" << std::endl
	    << "  bbox_lo_0=\"" << std::setprecision(10) << bbox_lo_[0]  << "\"" << std::endl
	    << "  bbox_lo_1=\"" << std::setprecision(10) << bbox_lo_[1]  << "\"" << std::endl
	    << "  bbox_hi_0=\"" << std::setprecision(10) << bbox_hi_[0]  << "\"" << std::endl
	    << "  bbox_hi_1=\"" << std::setprecision(10) << bbox_hi_[1]  << "\"" << std::endl
	    << "  nu=\"" << nu_  << "\"" << std::endl
	    << "  nv=\"" << nv_  << "\"" << std::endl
	    << "  img_width=\"" << img_width_  << "\"" << std::endl
	    << "  img_height=\"" << img_height_  << "\"" << std::endl;
	}
	out
	  << "/>" << std::endl;
	return out.str();
      }

      tilemap_config::~tilemap_config() {
      }

      double tilemap_config::units_per_pixel(std::size_t i) const {
	double uppx=(bbox_hi_[0]-bbox_lo_[0])/(double(nu_)*double(img_width_)*double(1<<i));
	double uppy=(bbox_hi_[1]-bbox_lo_[1])/(double(nv_)*double(img_height_)*double(1<<i));
	return  0.5*(uppx+uppy);
      }

      bool tilemap_config::parse(vic::xml::node_iterator ptr) {
	bool result=false;
	if (!ptr.is_null() && ptr.is_element_node() && ptr.tag() == "tilemap") {
	  result=true;
	  name_=ptr.attribute("name");
	  result=result && !ptr.error();

	  profile_=ptr.attribute("profile");
	  result=result && !ptr.error();

	  mime_=ptr.attribute("mime-type");
	  result=result && !ptr.error();

	  extension_=ptr.attribute("extension");
	  result=result && !ptr.error();

	  max_level_=ptr.attributei("max-level");
	  result=result && !ptr.error();

	  if (result && (profile_ == "global-geodetic")) {
	    srs_="EPSG:4326";
	    bbox_lo_[0]=-180.0;
	    bbox_lo_[1]=-90.0;
	    bbox_hi_[0]= 180.0;
	    bbox_hi_[1]= 90.0;
	    nu_=2;
	    nv_=1;
	    img_width_=256;
	    img_height_=256;      
	  } else if (result && (profile_ == "global-mercator")) {
	    srs_="OSGEO:41001";
	    bbox_lo_[0]=-20037508.34;
	    bbox_lo_[1]=-20037508.34;
	    bbox_hi_[0]= 20037508.34;
	    bbox_hi_[1]= 20037508.34;
	    nu_=2;
	    nv_=2;
	    img_width_=256;
	    img_height_=256;      
	  } else if (result) {
	    srs_=ptr.attribute("srs");
	    result=result && !ptr.error();

	    bbox_lo_[0]=ptr.attributed("bbox_lo_0");
	    result=result && !ptr.error();
    
	    bbox_lo_[1]=ptr.attributed("bbox_lo_1");
	    result=result && !ptr.error();
 
	    bbox_hi_[0]=ptr.attributed("bbox_hi_0");
	    result=result && !ptr.error();

	    bbox_hi_[1]=ptr.attributed("bbox_hi_1");
	    result=result && !ptr.error();

	    nu_=ptr.attributei("nu");
	    result=result && !ptr.error();

	    nv_=ptr.attributei("nv");
	    result=result && !ptr.error();

	    img_width_=ptr.attributei("img_width");
	    result=result && !ptr.error();

	    img_height_=ptr.attributei("img_height");
	    result=result && !ptr.error();      
	  }
	}
	return result;
      }

    }
  }
} // End namespace
