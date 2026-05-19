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
#include <xml_config_parser.hpp>
#include <vic/ratman/decorated_terrain_view.hpp>
#include <vic/ratman/terrain_renderable.hpp>
#include <vic/ratman/network.hpp>
//#include <vic/ratman/terrain_tile_placenames.hpp> // OLD
#include <vic/ratman/terrain_placenames.hpp>
//#include <vic/ratman/terrain_tile_geophotos.hpp>
#include <vic/ratman/terrain_tile_meteo.hpp>
#include <QMessageBox>

#ifdef ITEM3D
#include <vic/ratman/item3d_xml_reader.hpp>
#include <vic/ratman/item3d_manager.hpp>
#include <vic/ratman/osg_item3d_factory.hpp>
#endif

#include <vic/ratman/local_geonames_service.hpp>
#include <vic/ratman/wfs_geonames_service.hpp>

#include <vic/curlstream/url.hpp>
#include <vic/curlstream/curlstream.hpp>

#include <vic/cbdam/base/cbdam_diamond_fetcher.hpp>
#include <vic/cbdam/base/terrain_model.hpp>
#include <vic/cbdam/base/victms_geoimage_quad_fetcher.hpp>
#include <vic/cbdam/base/wms_geoimage_quad_fetcher.hpp>
#include <vic/cbdam/base/loaded_geoimage_quad_fetcher.hpp>
#include <QString>
#include <QDebug>
#include "config.hpp"

namespace ratman {

  // xml_config_parser

  xml_config_parser::xml_config_parser(const QString& file_name) {
    url_ = file_name;
    ratman_tag_=false;
    terrain_view_ = 0;
    state_=IDLE;
    current_str_.clear();
  }


  QString xml_config_parser::add_url(const QString& str) {
    return QString( (vic::url(url_.toStdString()).base() + vic::url(str.toStdString())).url_string().c_str());
  }


  bool xml_config_parser::startElementHome(const QString & /*namespaceURI*/,
					   const QString & /*localName*/,
					   const QString & /*qName*/,
					   const QXmlAttributes &attributes) {
    
    bool ok=true;
    QString u_str = attributes.value("u");
    if (u_str.isEmpty()) {
      error_str_ = QObject::tr("attribute=\"u\" missing.");
      return false;
    }
    double u = u_str.toDouble(&ok);
    if (!ok) {
      error_str_ = QObject::tr("attribute=\"u\" is not a number.");
      return false;
    }

    QString v_str = attributes.value("v");
    if (v_str.isEmpty()) {
      error_str_ = QObject::tr("attribute=\"v\" missing.");
      return false;
    }
    double v = v_str.toDouble(&ok);
    if (!ok) {
      error_str_ = QObject::tr("attribute=\"v\" is not a number.");
      return false;
    }

    QString h_str = attributes.value("h");
    if (h_str.isEmpty()) {
      error_str_ = QObject::tr("attribute=\"h\" missing.");
      return false;
    }
    double h = h_str.toDouble(&ok);
    if (!ok) {
      error_str_ = QObject::tr("attribute=\"h\" is not a number.");
      return false;
    }

    camera_home_uvh_=point3d_t(u, v, h);
    return true;
  }

  bool xml_config_parser::startElementCameraBounds(const QString & /*namespaceURI*/,
						   const QString & /*localName*/,
						   const QString & /*qName*/,
						   const QXmlAttributes &attributes) {
    
    bool ok=true;
    QString u_min_str = attributes.value("u_min");
    if (u_min_str.isEmpty()) {
      error_str_ = QObject::tr("attribute=\"u_min\" missing.");
      return false;
    }
    double u_min = u_min_str.toDouble(&ok);
    if (!ok) {
      error_str_ = QObject::tr("attribute=\"u_min\" is not a number.");
      return false;
    }

    QString u_max_str = attributes.value("u_max");
    if (u_max_str.isEmpty()) {
      error_str_ = QObject::tr("attribute=\"u_max\" missing.");
      return false;
    }
    double u_max = u_max_str.toDouble(&ok);
    if (!ok) {
      error_str_ = QObject::tr("attribute=\"u_max\" is not a number.");
      return false;
    }

    QString v_min_str = attributes.value("v_min");
    if (v_min_str.isEmpty()) {
      error_str_ = QObject::tr("attribute=\"v_min\" missing.");
      return false;
    }
    double v_min = v_min_str.toDouble(&ok);
    if (!ok) {
      error_str_ = QObject::tr("attribute=\"v_min\" is not a number.");
      return false;
    }

    QString v_max_str = attributes.value("v_max");
    if (v_max_str.isEmpty()) {
      error_str_ = QObject::tr("attribute=\"v_max\" missing.");
      return false;
    }
    double v_max = v_max_str.toDouble(&ok);
    if (!ok) {
      error_str_ = QObject::tr("attribute=\"v_max\" is not a number.");
      return false;
    }

    QString h_min_str = attributes.value("h_min");
    if (h_min_str.isEmpty()) {
      error_str_ = QObject::tr("attribute=\"h_min\" missing.");
      return false;
    }
    double h_min = h_min_str.toDouble(&ok);
    if (!ok) {
      error_str_ = QObject::tr("attribute=\"h_min\" is not a number.");
      return false;
    }

    QString h_max_str = attributes.value("h_max");
    if (h_max_str.isEmpty()) {
      error_str_ = QObject::tr("attribute=\"h_max\" missing.");
      return false;
    }
    double h_max = h_max_str.toDouble(&ok);
    if (!ok) {
      error_str_ = QObject::tr("attribute=\"h_max\" is not a number.");
      return false;
    }

    camera_uvh_box_=ratman::aabox3d_t(ratman::point3d_t(u_min, v_min, h_min),
				      ratman::point3d_t(u_max, v_max, h_max));
				      

    return true;
  }

  bool xml_config_parser::startElementElevation(const QString & /*namespaceURI*/,
						const QString & /*localName*/,
						const QString & /*qName*/,
						const QXmlAttributes &attributes) {
    
    QString id = attributes.value("id");   

    QString fname = attributes.value("url");
    if (fname.isEmpty()) {
      error_str_ = QObject::tr("attribute=\"fname\" missing.");
      return false;
    }
    fname = add_url(fname); 
    QString about = attributes.value("about");   
    std::string data_filename = fname.toStdString() + ".data";
    cbdam::cbdam_diamond_fetcher* geometry_fetcher = new  cbdam::cbdam_diamond_fetcher(data_filename, about.toStdString());
    geometry_fetcher->connect();
    if (!geometry_fetcher->is_connected()) {
      error_str_ = QObject::tr("geometry fetcher not connected");
      return false;
    }   
    cbdam::terrain_model* terrain_model = new cbdam::terrain_model(geometry_fetcher);
    if (!terrain_model->is_connected()) {
      error_str_ = QObject::tr("terrain model not connected");
      return false;
    }   
    ratman::terrain_renderable* tr = new ratman::terrain_renderable(terrain_view_, terrain_model);
    tr->set_active(true);
    terrain_view_->set_terrain_layer(tr);


    ratman::point2d_t uv = ratman::point2d_t(camera_home_uvh_[0], camera_home_uvh_[1]);
    camera_reset_position_uvh_ = ratman::oriented_position(terrain_model->uvh_xyz_transform(),
							   uv,
							   camera_home_uvh_[2]);

    state_=STATE_READY;
    return true;
  }


  bool xml_config_parser::startElementRouteService(const QString & /*namespaceURI*/,
						const QString & /*localName*/,
						const QString & /*qName*/,
						const QXmlAttributes &attributes) {
    
    route_service_url_ = attributes.value("url");
    if(route_service_url_.isEmpty()){
      error_str_ = QObject::tr("attribute=\"url\" missing.");
      return false;
    }
     
    return true;
  }

  bool xml_config_parser::startElementGeocodingService(const QString & /*namespaceURI*/,
						const QString & /*localName*/,
						const QString & /*qName*/,
						const QXmlAttributes &attributes) {
    
    geocoding_service_url_ = attributes.value("url");
    if(geocoding_service_url_.isEmpty()){
      error_str_ = QObject::tr("attribute=\"url\" missing.");
      return false;
    }
     
    return true;
  }
  

  bool xml_config_parser::startElementTms(const QString & /*namespaceURI*/,
					  const QString & /*localName*/,
					  const QString & /*qName*/,
					  const QXmlAttributes &attributes) {

    std::cerr << "##################### TMS" << std::endl;

    std::string srs = terrain_view_->terrain_layer()->model()->srs();
    aabox2d_t uv_box = terrain_view_->terrain_layer()->model()->uv_box();
    std::size_t quad_width = 256;

    QString id = attributes.value("id");   

    QString fname = attributes.value("url");
    if (fname.isEmpty()) {
      error_str_ = QObject::tr("attribute=\"fname\" missing.");
      return false;
    }
    fname = add_url(fname); 
     
    std::size_t first_level = 0;
    std::size_t last_level = 64;
    bool ok=false;

    QString fl = attributes.value("first_level");
    if (!fl.isEmpty()) {
      first_level = fl.toULong(&ok);
      if (!ok) {
	      error_str_ = QObject::tr("attribute=\"first_level\" not a long int.");
	      return false;
      }
    }

    QString ll = attributes.value("last_level");
    if (!ll.isEmpty()) {
      last_level = ll.toULong(&ok);
      if (!ok) {
        error_str_ = QObject::tr("attribute=\"last_level\" not a long int.");
        return false;
      }
    }

    double h_min = -1e30;
    double h_max = 1e30;

    QString h_min_str = attributes.value("h_min");
    if (!h_min_str.isEmpty()) {
      h_min = h_min_str.toDouble(&ok);
      if (!ok) {
        error_str_ = QObject::tr("attribute=\"h_min\" not a double");
        return false;
      }
    }

    QString h_max_str = attributes.value("h_max");
    if (!h_max_str.isEmpty()) {
      h_max = h_max_str.toDouble(&ok);
      if (!ok) {
        error_str_ = QObject::tr("attribute=\"h_max\" not a double");
        return false;
      }
    }

    genalpha_behavior_t genalpha_behavior=compressed_image_t::ALPHA_FROM_SRC;
    
    QString genalpha_behavior_str=attributes.value("genalpha_behavior");
    if (!genalpha_behavior_str.isEmpty()) {
      if (genalpha_behavior_str.toUpper()=="ALPHA_KEEP_SRC") {
        genalpha_behavior=compressed_image_t::ALPHA_KEEP_SRC;
      } else if (genalpha_behavior_str.toUpper()=="ALPHA_FROM_CONSTANT") {
        genalpha_behavior=compressed_image_t::ALPHA_FROM_CONSTANT;
      } else if (genalpha_behavior_str.toUpper()=="ALPHA_FROM_SRC") {
        genalpha_behavior=compressed_image_t::ALPHA_FROM_SRC;
      } else if (genalpha_behavior_str.toUpper()=="ALPHA_FROM_BLACK_IF_ABSENT") {
        genalpha_behavior=compressed_image_t::ALPHA_FROM_BLACK_IF_ABSENT;
      } else if (genalpha_behavior_str.toUpper()=="ALPHA_FROM_BLACK_FORCED") {
        genalpha_behavior=compressed_image_t::ALPHA_FROM_BLACK_FORCED;
      } else if (genalpha_behavior_str.toUpper()=="ALPHA_FROM_WHITE_IF_ABSENT") {
        genalpha_behavior=compressed_image_t::ALPHA_FROM_WHITE_IF_ABSENT;
      } else if (genalpha_behavior_str.toUpper()=="ALPHA_FROM_WHITE_FORCED") {
        genalpha_behavior=compressed_image_t::ALPHA_FROM_WHITE_FORCED;
      } else if (genalpha_behavior_str.toUpper()=="ALPHA_FROM_VALUE_IF_ABSENT") {
        genalpha_behavior=compressed_image_t::ALPHA_FROM_VALUE_IF_ABSENT;
      } else if (genalpha_behavior_str.toUpper()=="ALPHA_FROM_VALUE_FORCED") {
        genalpha_behavior=compressed_image_t::ALPHA_FROM_VALUE_FORCED;
      } else {
	      error_str_ = QObject::tr("attribute=\"genalpha_behavior\" is not valid");
	      return false;
      }      
    }

    sl::uint8_t genalpha_constant=10;
    QString genalpha_constant_str=attributes.value("genalpha_constant");
    if (!genalpha_constant_str.isEmpty()) {
      genalpha_constant = sl::median(0,255,genalpha_constant_str.toInt(&ok));
      if (!ok) {
	      error_str_ = QObject::tr("attribute=\"genalpha_constant\" not an int.");
	      return false;
      }
    }

    sl::uint8_t genalpha_alphamin=0;
    QString genalpha_alphamin_str=attributes.value("genalpha_alphamin");
     if (!genalpha_alphamin_str.isEmpty()) {
      genalpha_alphamin = sl::median(0,255,genalpha_alphamin_str.toInt(&ok));
      if (!ok) {
	      error_str_ = QObject::tr("attribute=\"genalpha_alphamin\" not an int.");
	      return false;
      }
    }

    sl::uint8_t genalpha_alphamax=255;
    QString genalpha_alphamax_str=attributes.value("genalpha_alphamax");
     if (!genalpha_alphamax_str.isEmpty()) {
      genalpha_alphamax = sl::median(0,255,genalpha_alphamax_str.toInt(&ok));
      if (!ok) {
	      error_str_ = QObject::tr("attribute=\"genalpha_alphamax\" not an int.");
	      return false;
      }
    }
     
    bool active = false; 
    QString active_str = attributes.value("active");
    if (!active_str.isEmpty()) {
      std::cerr << "Group Active found" << std::endl; 
      active = (active_str.toInt(&ok)==1);
      if (!ok) {
	      error_str_ = QObject::tr("attribute=\"active\" not an int.");
	      return false;
      }
	        if(ratman::config::instance()->exists_property(id))
         active = ratman::config::instance()->get_property(id);  
    }
   
    QString about = attributes.value("about");   

    cbdam::geoimage_quad_fetcher* fetcher = new cbdam::victms_geoimage_quad_fetcher(fname.toStdString(), srs, uv_box, quad_width, about.toStdString());
    fetcher->set_genalpha_behavior(genalpha_behavior, genalpha_constant, genalpha_alphamin, genalpha_alphamax);

    if (state_==STATE_BASE_LAYERS) {
      terrain_view_->terrain_layer()->model()->insert_base_color_layer(id.toStdString(),fetcher,first_level, last_level, h_min, h_max, active);
      std::cerr << "##################### TMS BASE" << std::endl;
    } else if (state_==STATE_OVERLAY_LAYERS) {
      terrain_view_->terrain_layer()->model()->insert_overlay_color_layer(id.toStdString(),fetcher,first_level, last_level, h_min, h_max, active);      
      std::cerr << "##################### TMS OVERLAY" << std::endl;
    }
    return true;
  }

  bool xml_config_parser::startElementLoadedImage(const QString & /*namespaceURI*/,
						  const QString & /*localName*/,
						  const QString & /*qName*/,
						  const QXmlAttributes &attributes) {

    std::cerr << "##################### Loaded Image" << std::endl;

    std::string srs = terrain_view_->terrain_layer()->model()->srs();
    aabox2d_t uv_box = terrain_view_->terrain_layer()->model()->uv_box();
    std::size_t quad_width = 256;

    QString id = attributes.value("id");   

    QString fname = attributes.value("url");
    if (fname.isEmpty()) {
      error_str_ = QObject::tr("attribute=\"fname\" missing.");
      return false;
    }
    fname = add_url(fname); 
     
    std::size_t first_level = 0;
    std::size_t last_level = 64;
    bool ok=false;

    QString fl = attributes.value("first_level");
    if (!fl.isEmpty()) {
      first_level = fl.toULong(&ok);
      if (!ok) {
	      error_str_ = QObject::tr("attribute=\"first_level\" not a long int.");
	      return false;
      }
    }

    QString ll = attributes.value("last_level");
    if (!ll.isEmpty()) {
      last_level = ll.toULong(&ok);
      if (!ok) {
	      error_str_ = QObject::tr("attribute=\"last_level\" not a long int.");
	      return false;
      }
    }

    double h_min = -1e30;
    double h_max = 1e30;

    QString h_min_str = attributes.value("h_min");
    if (!h_min_str.isEmpty()) {
      h_min = h_min_str.toDouble(&ok);
      if (!ok) {
	      error_str_ = QObject::tr("attribute=\"h_min\" not a double");
	      return false;
      }
    }

    QString h_max_str = attributes.value("h_max");
    if (!h_max_str.isEmpty()) {
      h_max = h_max_str.toDouble(&ok);
      if (!ok) {
	      error_str_ = QObject::tr("attribute=\"h_max\" not a double");
	      return false;
      }
    }

    genalpha_behavior_t genalpha_behavior=compressed_image_t::ALPHA_FROM_SRC;
    
    QString genalpha_behavior_str=attributes.value("genalpha_behavior");
    if (!genalpha_behavior_str.isEmpty()) {
      if (genalpha_behavior_str.toUpper()=="ALPHA_KEEP_SRC") {
	      genalpha_behavior=compressed_image_t::ALPHA_KEEP_SRC;
      } else if (genalpha_behavior_str.toUpper()=="ALPHA_FROM_CONSTANT") {
	      genalpha_behavior=compressed_image_t::ALPHA_FROM_CONSTANT;
      } else if (genalpha_behavior_str.toUpper()=="ALPHA_FROM_SRC") {
	      genalpha_behavior=compressed_image_t::ALPHA_FROM_SRC;
      } else if (genalpha_behavior_str.toUpper()=="ALPHA_FROM_BLACK_IF_ABSENT") {
	      genalpha_behavior=compressed_image_t::ALPHA_FROM_BLACK_IF_ABSENT;
      } else if (genalpha_behavior_str.toUpper()=="ALPHA_FROM_BLACK_FORCED") {
	      genalpha_behavior=compressed_image_t::ALPHA_FROM_BLACK_FORCED;
      } else if (genalpha_behavior_str.toUpper()=="ALPHA_FROM_WHITE_IF_ABSENT") {
	      genalpha_behavior=compressed_image_t::ALPHA_FROM_WHITE_IF_ABSENT;
      } else if (genalpha_behavior_str.toUpper()=="ALPHA_FROM_WHITE_FORCED") {
	      genalpha_behavior=compressed_image_t::ALPHA_FROM_WHITE_FORCED;
      } else if (genalpha_behavior_str.toUpper()=="ALPHA_FROM_VALUE_IF_ABSENT") {
	      genalpha_behavior=compressed_image_t::ALPHA_FROM_VALUE_IF_ABSENT;
      } else if (genalpha_behavior_str.toUpper()=="ALPHA_FROM_VALUE_FORCED") {
	      genalpha_behavior=compressed_image_t::ALPHA_FROM_VALUE_FORCED;
      } else {
	      error_str_ = QObject::tr("attribute=\"genalpha_behavior\" is not valid");
	      return false;
      }      
    }

    sl::uint8_t genalpha_constant=10;
    QString genalpha_constant_str=attributes.value("genalpha_constant");
    if (!genalpha_constant_str.isEmpty()) {
      genalpha_constant = sl::median(0,255,genalpha_constant_str.toInt(&ok));
      if (!ok) {
	      error_str_ = QObject::tr("attribute=\"genalpha_constant\" not an int.");
	      return false;
      }
    }
     
    bool active = false; 
    QString active_str = attributes.value("active");
    if (!active_str.isEmpty()) {
      std::cerr << "Group Active found" << std::endl; 
      active = (active_str.toInt(&ok)==1);
      if (!ok) {
        error_str_ = QObject::tr("attribute=\"active\" not an int.");
        return false;
      }
    }

    QString about = attributes.value("about");

    cbdam::geoimage_quad_fetcher* fetcher = new cbdam::loaded_geoimage_quad_fetcher(fname.toStdString(), srs, uv_box, quad_width, about.toStdString() );
    fetcher->set_genalpha_behavior(genalpha_behavior, genalpha_constant);

    if (state_==STATE_BASE_LAYERS) {
      terrain_view_->terrain_layer()->model()->insert_base_color_layer(id.toStdString(),fetcher,first_level, last_level, h_min, h_max, active);
      std::cerr << "##################### Loaded Image BASE" << std::endl;
    } else if (state_==STATE_OVERLAY_LAYERS) {
      terrain_view_->terrain_layer()->model()->insert_overlay_color_layer(id.toStdString(),fetcher,first_level, last_level, h_min, h_max, active);      
      std::cerr << "##################### Loaded Image OVERLAY" << std::endl;
    }
    return true;
  }

  bool xml_config_parser::startElementWms(const QString & /*namespaceURI*/,
					  const QString & /*localName*/,
					  const QString & /*qName*/,
					  const QXmlAttributes &attributes) {

    std::cerr << "##################### WMS" << std::endl;

    std::string srs = terrain_view_->terrain_layer()->model()->srs();
    aabox2d_t uv_box = terrain_view_->terrain_layer()->model()->uv_box();
    std::size_t quad_width = 256;

    QString id = attributes.value("id");   

    QString fname = attributes.value("url");
    if (fname.isEmpty()) {
      error_str_ = QObject::tr("attribute=\"fname\" missing.");
      return false;
    }
    fname = add_url(fname); 

    QString layers = attributes.value("layers");
    if (layers.isEmpty()) {
      error_str_ = QObject::tr("attribute=\"layers\" missing.");
      return false;
    }
     
    QString image_format = attributes.value("image_format");
    if (image_format.isEmpty()) {
      error_str_ = QObject::tr("attribute=\"image_format\" missing.");
      return false;
    }
    
    QString extra_arguments = attributes.value("extra_arguments");

    std::size_t first_level = 0;
    std::size_t last_level = 64;

    bool ok=false;

    QString fl = attributes.value("first_level");
    if (!fl.isEmpty()) {
      first_level = fl.toULong(&ok);
      if (!ok) {
	error_str_ = QObject::tr("attribute=\"first_level\" not a long int.");
	return false;
      }
    }

    QString ll = attributes.value("last_level");
    if (!ll.isEmpty()) {
      last_level = ll.toULong(&ok);
      if (!ok) {
	error_str_ = QObject::tr("attribute=\"last_level\" not a long int.");
	return false;
      }
    }

    double h_min = -1e30;
    double h_max = 1e30;

    QString h_min_str = attributes.value("h_min");
    if (!h_min_str.isEmpty()) {
      h_min = h_min_str.toDouble(&ok);
      if (!ok) {
	error_str_ = QObject::tr("attribute=\"h_min\" not a double");
	return false;
      }
    }

    QString h_max_str = attributes.value("h_max");
    if (!h_max_str.isEmpty()) {
      h_max = h_max_str.toDouble(&ok);
      if (!ok) {
	error_str_ = QObject::tr("attribute=\"h_max\" not a double");
	return false;
      }
    }

    genalpha_behavior_t genalpha_behavior=compressed_image_t::ALPHA_FROM_SRC;
    
    QString genalpha_behavior_str=attributes.value("genalpha_behavior");
    if (!genalpha_behavior_str.isEmpty()) {
      if (genalpha_behavior_str.toUpper()=="ALPHA_KEEP_SRC") {
	genalpha_behavior=compressed_image_t::ALPHA_KEEP_SRC;
      } else if (genalpha_behavior_str.toUpper()=="ALPHA_FROM_CONSTANT") {
	genalpha_behavior=compressed_image_t::ALPHA_FROM_CONSTANT;
      } else if (genalpha_behavior_str.toUpper()=="ALPHA_FROM_SRC") {
	genalpha_behavior=compressed_image_t::ALPHA_FROM_SRC;
      } else if (genalpha_behavior_str.toUpper()=="ALPHA_FROM_BLACK_IF_ABSENT") {
	genalpha_behavior=compressed_image_t::ALPHA_FROM_BLACK_IF_ABSENT;
      } else if (genalpha_behavior_str.toUpper()=="ALPHA_FROM_BLACK_FORCED") {
	genalpha_behavior=compressed_image_t::ALPHA_FROM_BLACK_FORCED;
      } else if (genalpha_behavior_str.toUpper()=="ALPHA_FROM_WHITE_IF_ABSENT") {
	genalpha_behavior=compressed_image_t::ALPHA_FROM_WHITE_IF_ABSENT;
      } else if (genalpha_behavior_str.toUpper()=="ALPHA_FROM_WHITE_FORCED") {
	genalpha_behavior=compressed_image_t::ALPHA_FROM_WHITE_FORCED;
      } else if (genalpha_behavior_str.toUpper()=="ALPHA_FROM_VALUE_IF_ABSENT") {
	genalpha_behavior=compressed_image_t::ALPHA_FROM_VALUE_IF_ABSENT;
      } else if (genalpha_behavior_str.toUpper()=="ALPHA_FROM_VALUE_FORCED") {
	genalpha_behavior=compressed_image_t::ALPHA_FROM_VALUE_FORCED;
      } else {
	error_str_ = QObject::tr("attribute=\"genalpha_behavior\" is not valid");
	return false;
      }      
    }

    sl::uint8_t genalpha_constant=10;
    QString genalpha_constant_str=attributes.value("genalpha_constant");
     if (!genalpha_constant_str.isEmpty()) {
      genalpha_constant = sl::median(0,255,genalpha_constant_str.toInt(&ok));
      if (!ok) {
	error_str_ = QObject::tr("attribute=\"genalpha_constant\" not an int.");
	return false;
      }
    }

    sl::uint8_t genalpha_alphamin=0;
    QString genalpha_alphamin_str=attributes.value("genalpha_alphamin");
     if (!genalpha_alphamin_str.isEmpty()) {
      genalpha_alphamin = sl::median(0,255,genalpha_alphamin_str.toInt(&ok));
      if (!ok) {
	error_str_ = QObject::tr("attribute=\"genalpha_alphamin\" not an int.");
	return false;
      }
    }

    sl::uint8_t genalpha_alphamax=255;
    QString genalpha_alphamax_str=attributes.value("genalpha_alphamax");
     if (!genalpha_alphamax_str.isEmpty()) {
      genalpha_alphamax = sl::median(0,255,genalpha_alphamax_str.toInt(&ok));
      if (!ok) {
	error_str_ = QObject::tr("attribute=\"genalpha_alphamax\" not an int.");
	return false;
      }
    }

    bool active = false;   

    QString active_str = attributes.value("active");
    if (!active_str.isEmpty()) {
      std::cerr << "Group Active found" << std::endl; 
      active = (active_str.toInt(&ok)==1);
      if (!ok) {
	error_str_ = QObject::tr("attribute=\"active\" not an int.");
	return false;
      }
    }

    cbdam::geoimage_quad_fetcher* fetcher = new cbdam::wms_geoimage_quad_fetcher(fname.toStdString(), 
										 layers.toStdString(),
										 image_format.toStdString(),
										 extra_arguments.toStdString(),
										 srs,
										 uv_box,
										 quad_width);

    fetcher->set_genalpha_behavior(genalpha_behavior, genalpha_constant, genalpha_alphamin, genalpha_alphamax);

    if (state_==STATE_BASE_LAYERS) {
      terrain_view_->terrain_layer()->model()->insert_base_color_layer(id.toStdString(),fetcher,first_level, last_level, h_min, h_max, active);
      std::cerr << "##################### TMS BASE" << std::endl;
    } else if (state_==STATE_OVERLAY_LAYERS) {
      terrain_view_->terrain_layer()->model()->insert_overlay_color_layer(id.toStdString(),fetcher,first_level, last_level, h_min, h_max, active);      
      std::cerr << "##################### TMS OVERLAY" << std::endl;
    }
    return true;
  }


  bool xml_config_parser::startElementGroup(const QString & /*namespaceURI*/,
                                            const QString & /*localName*/,
                                            const QString & /*qName*/,
                                            const QXmlAttributes &attributes) {

    QString id = attributes.value("id");
    if (id.isEmpty()) {
      error_str_ = QObject::tr("Missing id");
      return false;
    }

    active_renderable *group = new active_renderable(terrain_view_,id.toStdString());

    bool ok;
   
    QString priority = attributes.value("priority");
    if (!priority.isEmpty()) {
      int group_priority = priority.toInt(&ok);
      if (!ok) {
	error_str_ = QObject::tr("attribute=\"priority\" not a int.");
	return false;
      }
      group->set_priority(group_priority);
    }

    std::cerr << "Check for Group Active" << std::endl; 
    group->set_active(false);
    QString active = attributes.value("active");
    if (!active.isEmpty()) {
      std::cerr << "Group Active found" << std::endl; 
      int group_active = active.toInt(&ok);
      if (!ok) {
	error_str_ = QObject::tr("attribute=\"active\" not a int.");
	return false;
      }
      std::cerr << "Group Active " << group_active << std::endl; 
      if(group_active==1) {
	group->set_active(true);
      }
	        if(ratman::config::instance()->exists_property(id))
         group->set_active(ratman::config::instance()->get_property(id)); 
    }
    
    if(item_stack_.empty()) {
      terrain_view_->insert_decoration_layer(group);
    } else {
      item_stack_.top()->insert_child(group);
    }
    item_stack_.push(group);

    current_str_.clear();
    return true;
  }

  bool xml_config_parser::startElementSearchEngines(const QString & /*namespaceURI*/,
						    const QString & /*localName*/,
						    const QString & /*qName*/,
						    const QXmlAttributes &attributes) {
      
    QString id = attributes.value("id");
    if (id.isEmpty()) {
      error_str_ = QObject::tr("Missing id");
      return false;
    }

    QString placenames_fname = attributes.value("fname");
    if (placenames_fname.isEmpty()) {
      error_str_ = QObject::tr("attribute=\"fname\" missing.");
      return false;
    }
    placenames_fname = add_url(placenames_fname);

    QString server_type = attributes.value("server_type");
    if (placenames_fname.isEmpty()) {
      server_type="local";
    }

    QString wfs_namespace = attributes.value("wfs_namespace");
    QString wfs_feature = attributes.value("wfs_feature");
    QString wfs_property = attributes.value("wfs_property");
    QString wfs_geom = attributes.value("wfs_geom");

    QString wfs_classification_field = attributes.value("wfs_classification_field");
    QString wfs_classification_legend_field = attributes.value("wfs_classification_legend_field");
    qDebug() << "config_parser" << wfs_classification_legend_field;
    geonames_search* search_engine = NULL;
    if (server_type=="local") {
      search_engine = new local_geonames_search(placenames_fname.toStdString(), 
						id.toStdString());
    } else if (server_type=="wfs") {
      std::cerr << "################ WFS" << std::endl;
      std::cerr << wfs_feature.toStdString() << std::endl;
      std::cerr << wfs_property.toStdString() << std::endl;
      std::cerr << "################ WFS" << std::endl;
      search_engine = new wfs_geonames_search(placenames_fname.toStdString(),
					      id.toStdString(),
					      wfs_namespace.toStdString(),
					      wfs_feature.toStdString(),
					      wfs_property.toStdString(),
					      wfs_geom.toStdString(),
					      wfs_classification_field.toStdString(),
					      wfs_classification_legend_field.toStdString());
    }      
					    
    if (!search_engine) {
      error_str_ = QObject::tr("unknown geonames service type");
      return false;
    }

    search_engines_.push_back(search_engine);
    current_str_.clear();
    return true;
  }


  bool xml_config_parser::startElementPlacenames(const QString & /*namespaceURI*/,
						 const QString & /*localName*/,
						 const QString & /*qName*/,
						 const QXmlAttributes &attributes) {
      
    QString id = attributes.value("id");
    if (id.isEmpty()) {
      error_str_ = QObject::tr("Missing id");
      return false;
    }

    QString placenames_fname = attributes.value("fname");
    if (placenames_fname.isEmpty()) {
      error_str_ = QObject::tr("attribute=\"fname\" missing.");
      return false;
    }
    placenames_fname = add_url(placenames_fname);

    QString server_type = attributes.value("server_type");
    if (placenames_fname.isEmpty()) {
      server_type="local";
    }

    QString wfs_namespace = attributes.value("wfs_namespace");
    QString wfs_feature = attributes.value("wfs_feature");
    QString wfs_property = attributes.value("wfs_property");
    QString wfs_geom = attributes.value("wfs_geom");

    QString icon_fname = attributes.value("icon");
    QImage* icon = 0;
    if (!icon_fname.isEmpty()) {
      icon_fname = add_url(icon_fname);
      icon = network::instance()->qimage_fetch(icon_fname.toStdString());
    }

    bool ok;
    QString label_size = attributes.value("label_size");
    if (label_size.isEmpty()) {
      error_str_ = QObject::tr("attribute=\"label_size\" missing.");
      return false;
    }
    double placenames_label_size = label_size.toDouble(&ok);
    if (!ok) {
      error_str_ = QObject::tr("attribute=\"label_size\" not a double.");
      return false;
    }
    QString placemark_dmin = attributes.value("placemark_dmin");
    if (placemark_dmin.isEmpty()) {
      error_str_ = QObject::tr("attribute=\"placemark_dmin\" missing.");
      return false;
    }
    double placenames_placemark_dmin = placemark_dmin.toDouble(&ok);
    if (!ok) {
      error_str_ = QObject::tr("attribute=\"placemark_dmin\" not a double.");
      return false;
    }
    QString placemark_dmax = attributes.value("placemark_dmax");
    if (placemark_dmax.isEmpty()) {
      error_str_ = QObject::tr("attribute=\"placemark_dmax\" missing.");
      return false;
    }
    double placenames_placemark_dmax = placemark_dmax.toDouble(&ok);
    if (!ok) {
      error_str_ = QObject::tr("attribute=\"placemark_dmax\" not a double.");
      return false;
    }
    QString label_color = attributes.value("label_color");
    if (label_color.isEmpty()) {
      error_str_ = QObject::tr("attribute=\"label_color\" missing.");
      return false;
    }
    int r,g,b;
    if (sscanf(label_color.toLatin1(), "%d %d %d", &r, &g, &b) != 3) {
      error_str_ = QObject::tr("attribute=\"label_color\" not a rgb triple.");
      return false;
    }
    ratman::vector4f_t placenames_label_color = ratman::vector4f_t(r/255.0f, g/255.0f, b/255.0f, 1.0f);

    QString max_render_placenames_count_str = attributes.value("max_render_placenames_count");
    std::size_t max_render_placenames_count=30;
    if (!max_render_placenames_count_str.isEmpty()) {
      max_render_placenames_count = max_render_placenames_count_str.toULong(&ok);
      if (!ok) {
	error_str_ = QObject::tr("attribute=\"first_level\" not a long int.");
	return false;
      }
    }
    geonames_search* streaming_engine = NULL;
    if (server_type=="local") {
      streaming_engine = new local_geonames_search(placenames_fname.toStdString(), 
						   id.toStdString());
    } else if (server_type=="wfs") {
      std::cerr << "################ WFS" << std::endl;
      std::cerr << wfs_feature.toStdString() << std::endl;
      std::cerr << wfs_property.toStdString() << std::endl;
      std::cerr << "################ WFS" << std::endl;
      streaming_engine = new wfs_geonames_search(placenames_fname.toStdString(),
						 id.toStdString(),
						 wfs_namespace.toStdString(),
						 wfs_feature.toStdString(),
						 wfs_property.toStdString(),
						 wfs_geom.toStdString(),
						 "", "");
    }      
					    
    if (!streaming_engine) {
      error_str_ = QObject::tr("unknown geonames service type");
      return false;
    }

    ratman::terrain_placenames* placenames = new ratman::terrain_placenames(terrain_view_,
									    id.toStdString(),
									    streaming_engine,
									    placenames_label_size,
									    placenames_label_color,
									    placenames_placemark_dmin,
									    placenames_placemark_dmax,
									    max_render_placenames_count,
									    icon);
    QString priority = attributes.value("priority");
    if (!priority.isEmpty()) {
      int placenames_priority = priority.toInt(&ok);
      if (!ok) {
	error_str_ = QObject::tr("attribute=\"priority\" not a int.");
	return false;
      }
      placenames->set_priority(placenames_priority);
    }
 
    placenames->set_active(false);
    QString active = attributes.value("active");
    if (!active.isEmpty()) {
      int placenames_active = active.toInt(&ok);
      if (!ok) {
	error_str_ = QObject::tr("attribute=\"active\" not a int.");
	return false;
      }
      
      if(placenames_active==1) {
	placenames->set_active(true);
      }
      
      //===========

      if(ratman::config::instance()->exists_property(id))
         placenames->set_active(ratman::config::instance()->get_property(id)); 
      //========
      
    }
    
    if(item_stack_.empty()) {
      terrain_view_->insert_decoration_layer(placenames);
    } else {
      item_stack_.top()->insert_child(placenames);
    }
    current_str_.clear();
    return true;
  }

#if 0
  bool xml_config_parser::startElementGeophotos(const QString & /*namespaceURI*/,
						const QString & /*localName*/,
						const QString & /*qName*/,
						const QXmlAttributes &attributes) {
      
    QString id = attributes.value("id");
    if (id.isEmpty()) {
      error_str_ = QObject::tr("Missing id");
      return false;
    }

    QString geophotos_fname = attributes.value("fname");
    if (geophotos_fname.isEmpty()) {
      error_str_ = QObject::tr("attribute=\"fname\" missing.");
      return false;
    }
    geophotos_fname = add_url(geophotos_fname);
    QString icon_fname = attributes.value("icon");
    QImage* icon = 0;
    if (!icon_fname.isEmpty()) {
      icon_fname = add_url(icon_fname);
      icon = network::instance()->qimage_fetch(icon_fname.toStdString());
    }

    bool ok;
    QString placemark_dmin = attributes.value("placemark_dmin");
    if (placemark_dmin.isEmpty()) {
      error_str_ = QObject::tr("attribute=\"placemark_dmin\" missing.");
      return false;
    }
    double geophotos_placemark_dmin = placemark_dmin.toDouble(&ok);
    if (!ok) {
      error_str_ = QObject::tr("attribute=\"placemark_dmin\" not a double.");
      return false;
    }
    QString placemark_dmax = attributes.value("placemark_dmax");
    if (placemark_dmax.isEmpty()) {
      error_str_ = QObject::tr("attribute=\"placemark_dmax\" missing.");
      return false;
    }
    double geophotos_placemark_dmax = placemark_dmax.toDouble(&ok);
    if (!ok) {
      error_str_ = QObject::tr("attribute=\"placemark_dmax\" not a double.");
      return false;
    }

    ratman::terrain_tile_geophotos* geophotos = new ratman::terrain_tile_geophotos(std::string(id.toLatin1()),
                                                                                   std::string(geophotos_fname.toLatin1()),
                                                                                   *terrain_projection_,
                                                                                   geophotos_placemark_dmin,
                                                                                   geophotos_placemark_dmax,
										   terrain_center_,
										   8, // thumbs/frame
                                                                                   icon);
    QString priority = attributes.value("priority");
    if (!priority.isEmpty()) {
      int geophotos_priority = priority.toInt(&ok);
      if (!ok) {
	error_str_ = QObject::tr("attribute=\"priority\" not a int.");
	return false;
      }
      geophotos->set_priority(geophotos_priority);
    }
 
    geophotos->set_active(false);
    QString active = attributes.value("active");
    if (!active.isEmpty()) {
      int geophotos_active = active.toInt(&ok);
      if (!ok) {
	error_str_ = QObject::tr("attribute=\"active\" not a int.");
	return false;
      }
      if(geophotos_active==1) {
	geophotos->set_active(true);
      }
    }
    
    if(item_stack_.empty()) {
      terrain_view_->insert_decoration_layer(geophotos);
    } else {
      item_stack_.top()->insert_child(geophotos);
    }
    current_str_.clear();
    return true;
  }

#endif

  bool xml_config_parser::startElementMeteo(const QString & /*namespaceURI*/,
					    const QString & /*localName*/,
					    const QString & /*qName*/,
					    const QXmlAttributes &attributes) {
      
    QString id = attributes.value("id");
    if (id.isEmpty()) {
      error_str_ = QObject::tr("Missing id");
      return false;
    }

    QString geophotos_fname = attributes.value("fname");
    if (geophotos_fname.isEmpty()) {
      error_str_ = QObject::tr("attribute=\"fname\" missing.");
      return false;
    }
    geophotos_fname = add_url(geophotos_fname);
    QString icon_fname = attributes.value("icon");
    QImage* icon = 0;
    if (!icon_fname.isEmpty()) {
      icon_fname = add_url(icon_fname);
      icon = network::instance()->qimage_fetch(icon_fname.toStdString());
    }

    bool ok;
    QString placemark_dmin = attributes.value("placemark_dmin");
    if (placemark_dmin.isEmpty()) {
      error_str_ = QObject::tr("attribute=\"placemark_dmin\" missing.");
      return false;
    }
    double geophotos_placemark_dmin = placemark_dmin.toDouble(&ok);
    if (!ok) {
      error_str_ = QObject::tr("attribute=\"placemark_dmin\" not a double.");
      return false;
    }
    QString placemark_dmax = attributes.value("placemark_dmax");
    if (placemark_dmax.isEmpty()) {
      error_str_ = QObject::tr("attribute=\"placemark_dmax\" missing.");
      return false;
    }
    double geophotos_placemark_dmax = placemark_dmax.toDouble(&ok);
    if (!ok) {
      error_str_ = QObject::tr("attribute=\"placemark_dmax\" not a double.");
      return false;
    }

    ratman::terrain_tile_meteo* meteo = new ratman::terrain_tile_meteo(terrain_view_,
								       id.toStdString(),
								       geophotos_fname.toStdString(),
								       geophotos_placemark_dmin,
								       geophotos_placemark_dmax,
								       8, // thumbs/frame
								       icon);
    QString priority = attributes.value("priority");
    if (!priority.isEmpty()) {
      int meteo_priority = priority.toInt(&ok);
      if (!ok) {
	error_str_ = QObject::tr("attribute=\"priority\" not a int.");
	return false;
      }
      meteo->set_priority(meteo_priority);
    }
 
    meteo->set_active(false);
    QString active = attributes.value("active");
    if (!active.isEmpty()) {
      int meteo_active = active.toInt(&ok);
      if (!ok) {
	error_str_ = QObject::tr("attribute=\"active\" not a int.");
	return false;
      }
      if(meteo_active==1) {
	meteo->set_active(true);
      }
    }

	      if(ratman::config::instance()->exists_property(id))
         meteo->set_active(ratman::config::instance()->get_property(id)); 
    
    if(item_stack_.empty()) {
      terrain_view_->insert_decoration_layer(meteo);
    } else {
      item_stack_.top()->insert_child(meteo);
    }
    current_str_.clear();
    return true;
  }

#ifdef ITEM3D
  bool xml_config_parser::startElementItems3dConfig(const QString & /*namespaceURI*/,
						    const QString & /*localName*/,
						    const QString & /*qName*/,
						    const QXmlAttributes &attributes) {
    QString fname = attributes.value("fname");
    if (fname.isEmpty()) {
      error_str_ = QObject::tr("attribute=\"fname\" missing.");
      return false;
    }
    fname = add_url(fname);

    QString config = attributes.value("manager_config");
    if (config.isEmpty()) {
      error_str_ = QObject::tr("attribute=\"manager_config\" missing.");
      return false;
    }
    config = add_url(config);

    ratman::item3d_manager* manager = ratman::item3d_manager::instance();
    vic::icurlstream url_config(config.toStdString().c_str());
    manager->init_manager(terrain_view_, "Items3D Manager", url_config);
    manager->register_factory(
			      std::string("osg_factory"),
			      new ratman::osg_item3d_factory()
			     );

    /// Parsing te items3d xml file
    ratman::item3d_xml_reader* reader = new ratman::item3d_xml_reader();
    vic::icurlstream url(fname.toStdString().c_str());
    reader->read_stream(url);

    return true;
  }
#endif

  bool xml_config_parser::startElement(const QString & namespaceURI,
				       const QString & localName,
				       const QString &qName,
				       const QXmlAttributes &attributes) {
    if (!ratman_tag_ && qName != "ratman") {
      error_str_ = QObject::tr("The file is not a RATMAN file.");
      return false;
    }
    if (qName == "ratman") {
      QString version = attributes.value("version");
      if (version.isEmpty() || version != "2.0") {
        error_str_ = QObject::tr("The file is not a version 2.0 file.");
        return false;
      }
      ratman_tag_ = true;
      state_=IDLE;
      terrain_view_ = new ratman::decorated_terrain_view();
    } else if (qName == "init" && state_==IDLE) {
      state_=STATE_INIT;
      return true;
    } else if (qName == "home" && state_==STATE_INIT) {
      return  startElementHome(namespaceURI, localName, qName, attributes);
    } else if (qName == "camera_bounds" && state_==STATE_INIT) {
      return  startElementCameraBounds(namespaceURI, localName, qName, attributes);
    } else if (qName == "elevation" && state_==STATE_INITIALIZED) {
      return  startElementElevation(namespaceURI, localName, qName, attributes);
    } else if (qName == "route_service" && state_==STATE_READY) {
      return  startElementRouteService(namespaceURI, localName, qName, attributes); 
    } else if (qName == "geocoding_service" && state_==STATE_READY) {
      return  startElementGeocodingService(namespaceURI, localName, qName, attributes); 
    } else if (qName == "base_layers" && state_==STATE_READY) {
      state_=STATE_BASE_LAYERS;
      return true;
    } else if (qName == "overlay_layers" && state_==STATE_READY) {
      state_=STATE_OVERLAY_LAYERS;
      return true;
    } else if (qName == "search_engines" && state_==STATE_READY) {
      state_=STATE_SEARCH_ENGINES;
      return true;
    } else if (qName == "tms" && state_==STATE_BASE_LAYERS) {
      return  startElementTms(namespaceURI, localName, qName, attributes);
    } else if (qName == "tms" && state_==STATE_OVERLAY_LAYERS) {
      return  startElementTms(namespaceURI, localName, qName, attributes);
    } else if (qName == "loaded_image" && state_==STATE_BASE_LAYERS) {
      return  startElementLoadedImage(namespaceURI, localName, qName, attributes);
    } else if (qName == "loaded_image" && state_==STATE_OVERLAY_LAYERS) {
      return  startElementLoadedImage(namespaceURI, localName, qName, attributes);
    } else if (qName == "wms" && state_==STATE_BASE_LAYERS) {
      return  startElementWms(namespaceURI, localName, qName, attributes);
    } else if (qName == "wms" && state_==STATE_OVERLAY_LAYERS) {
      return  startElementWms(namespaceURI, localName, qName, attributes);
    } else if (qName == "group" && state_==STATE_READY) {
      return  startElementGroup(namespaceURI, localName, qName, attributes);
    } else if (qName == "search_engine_item" && state_==STATE_SEARCH_ENGINES) {
      return  startElementSearchEngines(namespaceURI, localName, qName, attributes);
    } else if (qName == "placenames" && state_==STATE_READY) {
      return  startElementPlacenames(namespaceURI, localName, qName, attributes);
#if 0
    } else if (qName == "geophotos" && state_==STATE_READY) {
      return  startElementGeophotos(namespaceURI, localName, qName, attributes);
#endif
    } else if (qName == "meteo" && state_==STATE_READY) {
      return  startElementMeteo(namespaceURI, localName, qName, attributes);
    } else if (qName == "items3d_config" && state_==STATE_READY) {
#ifdef ITEM3D      
      return startElementItems3dConfig(namespaceURI, localName, qName, attributes);
#else
      return true;
#endif
    } else {
      error_str_ = QObject::tr("unknown tag ") + qName;
      std::cerr << "Unknown tag " << qName.toStdString() << " State: " << state_ << std::endl;
      return false;
    }
    current_str_.clear();
    return true;
  }
  
  bool xml_config_parser::endElement(const QString & /* namespaceURI */,
				     const QString & /* localName */,
				     const QString &qName) {
    if (qName == "group") {
      if (item_stack_.empty()) {
	      error_str_ = QObject::tr("too many tag </group>");
	      return false;
      }
      item_stack_.pop();
    }

    if (qName == "init") {
      state_ = STATE_INITIALIZED;
    }

    if (qName == "base_layers") {
      state_ = STATE_READY;
    }

    if (qName == "overlay_layers") {
      state_ = STATE_READY;
    }

    if (qName == "search_engines") {
      state_ = STATE_READY;
    }

    return true;
  }

  
  bool xml_config_parser::characters(const QString &str) {
    current_str_ += str;
    return true;
  }
  
  const QString &xml_config_parser::error_str() const {
    return error_str_;
  }

  const QString &xml_config_parser::current_str() const {
    return current_str_;
  }
  
  bool xml_config_parser::fatalError(const QXmlParseException &exception) {
    QMessageBox::information(0, QObject::tr("RATMAN"),
			     QObject::tr("Parse error at line %1, column %2:\n"
					 "%3")
			     .arg(exception.lineNumber())
			     .arg(exception.columnNumber())
			     .arg(error_str()));
    return false;
  }
}
