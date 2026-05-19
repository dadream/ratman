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
#include <vic/ratman/wfs_geonames_service.hpp>
#include <vic/ratman/network.hpp>
#include <vic/ratman/string_utility.hpp>
#include <sl/utility.hpp>
#include <QString>
#include <QStringList>
#include <QByteArray>
#include <QDomDocument>
#include <QDebug>

namespace ratman {

  wfs_geonames_search::wfs_geonames_search(const std::string& url, 
					   const std::string& id,
					   const std::string& namespace_tag,
					   const std::string& feature_tag,
					   const std::string& property_tag,
					   const std::string& geom_tag,
					   const std::string& classification_tag,
					   const std::string& classification_legend_tag,
					   bool using_classification_legend
					   ) :
    geonames_search(url, id, classification_tag, classification_legend_tag, using_classification_legend),
    namespace_tag_(namespace_tag),
    feature_tag_(feature_tag),
    property_tag_(property_tag), 
    geom_tag_(geom_tag)
    {
    
    formatted_property_tag_list_ = QString(property_tag_.c_str()).split(",");
    if (!namespace_tag_.empty()) {
      for(int i=0; i < formatted_property_tag_list_.size(); ++i){
	   formatted_property_tag_list_[i] = QString(namespace_tag_.c_str()) + ":" + formatted_property_tag_list_.at(i);
      }
    }	
    
    if(!classification_tag.empty() && !classification_legend_tag.empty())
    {
      using_classification_legend_ = true;	
      
      classification_legend_tag_list_ = QString(classification_legend_tag_.c_str()).split(",");
      QStringList app;
      for(int i = 0; i < classification_legend_tag_list_.size(); ++i){
  	    app = classification_legend_tag_list_[i].split("=");
  	   
        assert(app.size() == 2);
  	    classification_map[app[0]] = app[1] ;
      }
    }	
  }

  // GML SYNTAX
  void wfs_geonames_search::request_geonames(const std::string& query) {
    geonames_.clear();
    std::istream* in_file = network::instance()->istream_open(query);
    if (!in_file) {
      std::cerr << "ERROR unable to connect to WFS" << std::endl;
    } else {
      QByteArray bArray;
      while ( in_file->good() ) {
	char c; in_file->get(c);
	if (in_file->good()) {
	  bArray.append(c);
	}
      }
      // Close file
      network::instance()->istream_close(in_file);
    //  qDebug() << "risultato=" << bArray;
      QDomDocument doc;
      doc.setContent(bArray);
    
      QDomElement docElem = doc.documentElement();
      QDomNodeList element_list = docElem.elementsByTagName(feature_tag_.c_str());

      SL_TRACE_OUT(1) << " ======== PARSING " << element_list.length() << " FNAMES" << std::endl;
      for (std::size_t i=0; i < element_list.length(); ++i){
	QDomNode poiNode = element_list.item(i);
	QDomElement poiElement = poiNode.toElement();

	// Location
	double lon = 0.0;
	double lat = 0.0;
	double elv = 0.0;

	std::string formatted_geom_tag = "";
	if (namespace_tag_.empty()) {
	  formatted_geom_tag = geom_tag_;
	} else {
	  formatted_geom_tag = namespace_tag_ + ":" + geom_tag_;
	}

	QDomNodeList geometry_list = poiElement.elementsByTagName(formatted_geom_tag.c_str());
	if (geometry_list.length() >= 1) {
	  QDomNode geomNode = geometry_list.item(0);
	  QDomElement geomElement = geomNode.toElement();

	  QString locationString = geomElement.elementsByTagName("gml:pos").item(0).toElement().text();
	
	  QStringList locationElements = locationString.split(' ');
	  lon = locationElements[0].toDouble();
	  lat = locationElements[1].toDouble();
	  SL_TRACE_OUT(1) << "LON " << lon << std::endl;
	  SL_TRACE_OUT(1) << "LAT " << lat << std::endl;
	  if (locationElements.size() == 3){
	    elv = locationElements[2].toDouble();
	  }
	}

	std::string name = poiElement.elementsByTagName(formatted_property_tag_list_.at(0)).item(0).toElement().text().toStdString();
	std::string desc = "";
	std::string url = "";
	if(formatted_property_tag_list_.size() > 1) {
	  url  = poiElement.elementsByTagName(formatted_property_tag_list_.at(1)).item(0).toElement().text().toStdString();
	}
	if ( !classification_tag_.empty() ) {
	  
	  std::string formatted_classification_tag = "";
	  if (namespace_tag_.empty()) {
	    formatted_classification_tag = classification_tag_;
	  } else {
	    formatted_classification_tag = namespace_tag_ + ":" + classification_tag_;
	  }
	  SL_TRACE_OUT(1) << "formatted_classification_tag= " << formatted_classification_tag.c_str() << std::endl;
	  desc = poiElement.elementsByTagName(formatted_classification_tag.c_str()).item(0).toElement().text().toStdString();
	  desc = classify(desc);
	  SL_TRACE_OUT(1) << "desc= " << desc.c_str() << std::endl;
	}
	//else url = "";
	//if (url == "0") url = "";

	if (name.empty()) {
	  std::cout << "SKIPPING UNNAMED ENTRY" << std::endl;
	}

	append_geoname(point3d_t(lon,lat,elv), 
				 name, 
				 desc, 
				 url);
      }
    }
  }

  void wfs_geonames_search::append_geoname(const point3d_t& location,
					   const std::string& name,
					   const std::string& description, 
					   const std::string& url) {
    geonames_.push_back(geonames_entry(location, name, description, url));
  }




  void wfs_geonames_search::search(const std::string& query_name,
				   const aabox2d_t& query_box,
				   std::size_t query_max_rows,
				   std::size_t query_start_row) { 
    last_search_results_.clear();
    size_t search_count=0;

    bool has_str_query = (query_name != "");
    QString str_query = "*";
    if (has_str_query) {
    // str_query = "*"+QString(string_utility::to_lower(query_name).c_str())+"*";
      str_query = "*" + QString(query_name.c_str()) + "*";
    }

    QString str_box;
    str_box.sprintf("%f,%f%%20%f,%f", query_box[0][0], query_box[0][1], 
		    query_box[1][0], query_box[1][1]);


    QString req(base_url_.c_str());

    //non parametrica, da migliorare. utilizza il campo RICERCA sempre tutto maiuscolo
    req += QString("?request=GetFeature&PROPERTYNAME=") + formatted_property_tag_list_.join(",") +(",ras:the_geom&version=1.1.0&typeName=") + feature_tag_.c_str();
    req += QString("&outputFormat=GML3&srsName=EPSG:4326&FILTER=");
    req += QString("%3CFilter%20xmlns='http://www.opengis.net/ogc'%20xmlns:gml='http://www.opengis.net/gml'%3E");
    
    req += QString("%3CAnd%3E%3CPropertyIsLike%20wildCard='*'%20singleChar='0x'%20escapeChar='!'%3E");
    req += QString("%3CPropertyName%3E") + "ricerca" + QString("%3C/PropertyName%3E");
    req += QString("%3CLiteral%3E")  + str_query.toUpper().replace(" ","%20") +  QString("%3C/Literal%3E%3C/PropertyIsLike%3E");
  
    req += QString("%3CBBOX%3E%3CPropertyName%3Ethe_geom%3C/PropertyName%3E");
    req += QString("%3Cgml:Box%20srsName='EPSG:4326'%3E%3Cgml:coordinates%3E");
    req += str_box;
    req += QString("%3C/gml:coordinates%3E%3C/gml:Box%3E%3C/BBOX%3E%3C/And%3E%3C/Filter%3E");
  
    
   
    SL_TRACE_OUT(-1) << "SEARCH: " << req.toStdString() << std::endl;
    SL_TRACE_OUT(-1) << "prova=" << req.toStdString() << std::endl;
    request_geonames(req.toStdString());
    for (std::size_t i=0; i<geonames_count() && last_search_results_.size()<query_max_rows; ++i) {
      if (search_count>=query_start_row) {
	last_search_results_.push_back(geonames(i));
      }
      search_count++;
    }
    last_search_ok_=true;
  }
  
  
  std::string wfs_geonames_search::classify(const std::string& desc){
  	 if(using_classification_legend_ == false) return "";
  	 return classification_map[QString(desc.c_str())].isEmpty() ? "description missing" : classification_map[QString(desc.c_str())].toStdString();
  }
    
} // namespace ratman
