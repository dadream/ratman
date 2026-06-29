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
#ifndef RATMAN_XML_HANDLER_HPP
#define RATMAN_XML_HANDLER_HPP
#include <GL/glew.h>
#include <QApplication>
#include <QString>
#include <QWidget>
#include <QXmlDefaultHandler>
#include <QXmlSimpleReader>
#include <iostream>
#include <vic/ratman/ratman.hpp>
#include <vic/ratman/oriented_position.hpp>
#include <vic/cbdam/base/compressed_rgba32_image.hpp>
#include <stack>
#include <vector>

namespace ratman {

  class active_renderable;
  class decorated_terrain_view;
  class geonames_search;

  /**
   * Configuration file parser used to setup the application.
   */
  class xml_config_parser : public QXmlDefaultHandler {
  public:
    typedef cbdam::compressed_rgba32_image                 compressed_image_t;
    typedef compressed_image_t::genalpha_behavior_t        genalpha_behavior_t;

  protected:
    enum state {
      IDLE,
      STATE_INIT,
      STATE_INITIALIZED,
      STATE_READY,
      STATE_BASE_LAYERS,
      STATE_OVERLAY_LAYERS,
      STATE_SEARCH_ENGINES,
    };

    ratman::decorated_terrain_view*  terrain_view_;

    ratman::oriented_position        camera_reset_position_uvh_;
    ratman::point3d_t                camera_home_uvh_;
    ratman::aabox3d_t                camera_uvh_box_;
    
    QString current_str_;
    QString error_str_;
    bool ratman_tag_;
    std::stack<ratman::active_renderable *> item_stack_;
    std::vector<geonames_search*> search_engines_;
    state state_;
    QString url_;
    QString route_service_url_;
    QString geocoding_service_url_;

  public:
    xml_config_parser(const QString& file_name);
    bool startElement(const QString &namespaceURI, 
      const QString &localName,
      const QString &qName, 
      const QXmlAttributes &attributes);      
    bool endElement(const QString &namespaceURI, 
      const QString &localName,
      const QString &qName);
    bool characters(const QString &str);
    bool fatalError(const QXmlParseException &exception);
    const QString &error_str() const;
    const QString &current_str() const;

    bool startElementHome(const QString & namespaceURI,
      const QString & localName,
      const QString &qName,
      const QXmlAttributes &attributes);
    bool startElementCameraBounds(const QString & namespaceURI,
      const QString & localName,
      const QString &qName,
      const QXmlAttributes &attributes);
    bool startElementElevation(const QString & namespaceURI,
      const QString & localName,
      const QString &qName,
      const QXmlAttributes &attributes);
    bool startElementRouteService(const QString & namespaceURI,
      const QString & localName,
      const QString &qName,
      const QXmlAttributes &attributes);
    bool startElementGeocodingService(const QString & namespaceURI,
      const QString & localName,
      const QString &qName,
      const QXmlAttributes &attributes);
    bool startElementTms(const QString & namespaceURI,
      const QString & localName,
      const QString &qName,
      const QXmlAttributes &attributes);
    bool startElementLoadedImage(const QString & namespaceURI,
      const QString & localName,
      const QString &qName,
      const QXmlAttributes &attributes);
    bool startElementWms(const QString & namespaceURI,
      const QString & localName,
      const QString &qName,
      const QXmlAttributes &attributes);
    bool startElementGroup(const QString & namespaceURI,
      const QString & localName,
      const QString &qName,
      const QXmlAttributes &attributes);
    bool startElementSearchEngines(const QString & namespaceURI,
      const QString & localName,
      const QString &qName,
      const QXmlAttributes &attributes);
    bool startElementPlacenames(const QString & namespaceURI,
      const QString & localName,
      const QString &qName,
      const QXmlAttributes &attributes);
#if 0
    bool startElementGeophotos(const QString & namespaceURI,
				const QString & localName,
				const QString &qName,
				const QXmlAttributes &attributes);
#endif
    bool startElementMeteo(const QString & namespaceURI,
			   const QString & localName,
			   const QString &qName,
			   const QXmlAttributes &attributes);

#ifdef ITEM3D
    bool startElementItems3dConfig(const QString & namespaceURI,
				   const QString & localName,
				   const QString &qName,
				   const QXmlAttributes &attributes);
#endif

    inline ratman::decorated_terrain_view* decorated_terrain_view() {
      return terrain_view_;
    }

    inline const ratman::oriented_position& camera_reset_position_uvh() const {
      return camera_reset_position_uvh_;
    }

    inline const ratman::aabox3d_t& camera_uvh_box() const {
      return camera_uvh_box_;
    }

    inline const std::vector<geonames_search*>& search_engines() const {
      return search_engines_;
    }

    inline const QString route_service_url() const{
      return route_service_url_;
    }

    inline const QString geocoding_service_url() const{
      return geocoding_service_url_;
    }

  protected:
    QString add_url(const QString& str);
   

  };

}


#endif
