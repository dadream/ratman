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
#include <vic/ratman/local_geonames_service.hpp>
#include <vic/ratman/network.hpp>
#include <vic/ratman/string_utility.hpp>
#include <vic/cbdam/base/background_thread.hpp>
#include <sl/utility.hpp>
#include <QString>
#include <QStringList>
#include <QByteArray>
#include <qdom.h>

namespace ratman {

  local_geonames_search::local_geonames_search(const std::string& url, const std::string& id) :
    geonames_search(url, id) {
    load_geonames();
  }

#if 0
  void local_geonames_search::load_geonames() {
    geonames_.clear();
    std::istream* in_file = network::instance()->istream_open(base_url_);
    if (!in_file) {
      std::cerr << "ERROR unable to open input places file" << base_url_ << std::endl;
    } else {
      int line = 0;
      while ( in_file->good() ) {
	++line;
	std::string str; std::getline(*in_file, str);
	if(!str.empty()) {
	  std::vector<std::string> site_data;
	  string_utility::split(str, "\t", site_data);
	  if (site_data.size() != 5 && (site_data.size() != 6)) {
	    std::cerr << "ERROR: parse error at " << base_url_ << ":" << line <<  std::endl;
	  } else {
	    std::string name = site_data[0];
	    double lat = string_utility::convert_into<double>(site_data[2]);
	    double lon = string_utility::convert_into<double>(site_data[3]);
	    double elv = string_utility::convert_into<double>(site_data[4]);
	    std::string url = "";
	    if (site_data.size() == 6){
	      url = site_data[5];
	      if (url.size() <=1)
		url = "";
	    }
	    point3d_t loc = point3d_t(lon, lat, elv);
	    append_geoname(loc, name, "", url);
	  }
	}
      }
    }

    // Close file
    network::instance()->istream_close(in_file);
  }
#else
  // GML SYNTAX
  void local_geonames_search::load_geonames() {
    geonames_.clear();
    std::istream* in_file = network::instance()->istream_open(base_url_);
    if (!in_file) {
      std::cerr << "ERROR unable to open input places file" << base_url_ << std::endl;
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
      QDomDocument doc;
      doc.setContent(bArray);
      QDomElement docElem = doc.documentElement();
      QDomNodeList element_list = docElem.elementsByTagName("gml:featureMember");

      SL_TRACE_OUT(1) << " ======== PARSING " << element_list.length() << " FNAMES" << std::endl;
      for (std::size_t i=0; i < element_list.length(); ++i){
	QDomNode poiNode = element_list.item(i);
	QDomElement poiElement = poiNode.toElement();

	// Location
	double lon = 0.0;
	double lat = 0.0;
	double elv = 0.0;

	QString locationString = poiElement.elementsByTagName("gml:coordinates").item(0).toElement().text();
	QStringList locationElements = locationString.split(',');
	lon = locationElements[0].toDouble();
	lat = locationElements[1].toDouble();
	if (locationElements.size() == 3){
	  elv = locationElements[2].toDouble();
	}
	std::string name = poiElement.elementsByTagName("topp:NOME").item(0).toElement().text().toLatin1().data();
	std::string desc = "";
	std::string url  = poiElement.elementsByTagName("topp:URL").item(0).toElement().text().toLatin1().data();
	if (url == "0") url = "";

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
#endif 


  void local_geonames_search::append_geoname(const point3d_t& location,
					     const std::string& name,
					     const std::string& description, 
					     const std::string& url) {
    geonames_.push_back(geonames_entry(location, name, description, url));
  }

  void local_geonames_search::search(const std::string& query_name,
				     const aabox2d_t& query_box,
				     std::size_t query_max_rows,
				     std::size_t query_start_row) { 
    last_search_results_.clear();
    size_t search_count=0;

    bool has_str_query = (query_name != "");

    std::string str_query = std::string("*")+string_utility::to_lower(query_name)+std::string("*");
    
    for (std::size_t i=0; i<geonames_count() && last_search_results_.size()<query_max_rows; ++i) {
      if (i%5000==0) cbdam::background_thread::cpu_yield(); // FIXME

      point2d_t loc=point2d_t(geonames(i).location()[0], 
			      geonames(i).location()[1]);
      if (query_box.contains(loc)) {
	bool found = !(has_str_query) || sl::matches(string_utility::to_lower(geonames(i).name()),
						     str_query);
	if (found) {
	  if (search_count>=query_start_row) {
	    last_search_results_.push_back(geonames(i));
	  }
	  search_count++;
	}
      }
    }

    last_search_ok_=true;
  }
    
} // namespace ratman
