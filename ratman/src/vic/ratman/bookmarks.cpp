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
#include "bookmarks.hpp"
#include <vic/ratman/network.hpp>
#include <QByteArray>
#include <QFile>
#include <QTextStream>

namespace ratman {

  bookmarks::bookmarks() {
  }

  bookmarks::~bookmarks() {
    clear_container();
  }

  void bookmarks::load_kml(QString filename){
    std::cerr << "bookmarks filename " <<filename.toStdString()<< std::endl;
    clear_container();
    std::istream* in_file = network::instance()->istream_open(filename.toStdString());
    
    if (!in_file) {
      std::cerr << "BOOKMARKS ERROR unable to connect to kml service" << std::endl;
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

      //QDomDocument doc;
      doc_.setContent(bArray);
      QDomElement docElem = doc_.documentElement();
      QDomNodeList element_list = docElem.elementsByTagName("Placemark");

      QString name;
      QString longitude;
      QString latitude;
      QString altitude;
      QString range;
      QString tilt;
      QString heading;

      for (std::size_t i=0; i < element_list.length(); ++i){
	QDomNode poiNode = element_list.item(i);
	QDomElement poiElement = poiNode.toElement();
	for(QDomNode n = poiElement.firstChild(); !n.isNull(); n = n.nextSibling()) {
	  if (n.isElement()) {
	    QDomElement nElement = n.toElement();
	    if (nElement.tagName() == "name") {
	      name = nElement.text();
	    } 
	    else if (nElement.tagName() == "LookAt"){
	      
	      for(QDomNode look_at_node = nElement.firstChild(); 
		  !look_at_node.isNull(); 
		  look_at_node = look_at_node.nextSibling()) {
		
		if (look_at_node.isElement()) {
		  QDomElement look_at_element=look_at_node.toElement();
		  
		  if (look_at_element.tagName() == "longitude") {
		    longitude=look_at_element.text();
		  }
		  if (look_at_element.tagName() == "latitude") {
		    latitude = look_at_element.text();
		  }
		  if (look_at_element.tagName() == "altitude") {
		    altitude = look_at_element.text();
		  }
		  if (look_at_element.tagName() == "range") {
		    range = look_at_element.text();
		  }
		  if (look_at_element.tagName() == "tilt") {
		    tilt = look_at_element.text();
		  }
		  if (look_at_element.tagName() == "heading") {
		    heading = look_at_element.text();
		  }
		}
	      } 
	    }
	  }
	}
	ratman::point3d_t location = ratman::point3d_t(longitude.toDouble(),
						       latitude.toDouble(),
						       altitude.toDouble());
	
	ratman::point3d_t orientation = ratman::point3d_t(range.toDouble(),
							  tilt.toDouble(),
							  heading.toDouble());
	
	insert(name, location, orientation);

      }
    }

    if (doc_.documentElement().isNull()) {
      //      std::cerr << "bookmarks:load_kml():null doc element: build empty bookmarkdoc" << std::endl;
      doc_ = QDomDocument("MyML");
      QDomElement root = doc_.createElement("kml");
      root.setAttribute("xmlns", "http://earth.google.com/kml/2.1");
      doc_.appendChild(root);

      QDomElement tag = doc_.createElement("Document");
      root.appendChild(tag);

      QDomElement tag1 = doc_.createElement("Name");
      tag.appendChild(tag1);

      QDomText t = doc_.createTextNode("default_myplaces_it_IT.kml");
      tag1.appendChild(t);

      QDomElement tag2 = doc_.createElement("Folder");
      tag.appendChild(tag2);

      QDomElement tag3 = doc_.createElement("Name");
      tag2.appendChild(tag3);

      QDomText t1 = doc_.createTextNode("bookmarks");
      tag3.appendChild(t1);

      //      QString xml = doc_.toString();
      //      std::cerr << xml.toStdString() << std::endl;
    }
  }

  void bookmarks::save_kml(QString filename){
    std::cerr << "Open File " << filename.toStdString() << std::endl;
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)){
      std::cerr << "ERROR failed to write in kml file" << std::endl;
      return;
    }
    QTextStream bookmarks(&file);
    doc_.documentElement().save(bookmarks,0);
    std::cerr << "Saved bookmarks " << std::endl;
  }

  void bookmarks::insert_xml_node(const QString& s,const point3d_t& p, const point3d_t& o){
    QDomElement new_placemark = doc_.createElement("Placemark") ;
    
    doc_.documentElement().firstChildElement("Document").firstChildElement("Folder").appendChild(new_placemark);
    
    QDomElement new_name = doc_.createElement("name");
    new_placemark.appendChild(new_name);
    QDomText text_name = doc_.createTextNode(s);
    new_name.appendChild(text_name);
    

    QDomElement new_lookat = doc_.createElement("LookAt");
    new_placemark.appendChild(new_lookat);
     
    QDomElement new_longitude = doc_.createElement("longitude");
    new_lookat.appendChild(new_longitude);
    QDomText text_longitude = doc_.createTextNode(QString::number(p[0]));
    new_longitude.appendChild(text_longitude);
   
    QDomElement new_latitude = doc_.createElement("latitude");
    new_lookat.appendChild(new_latitude);
    QDomText text_latitude = doc_.createTextNode(QString::number(p[1]));
    new_latitude.appendChild(text_latitude);

    QDomElement new_altitude = doc_.createElement("altitude");
    new_lookat.appendChild(new_altitude);
    QDomText text_altitude = doc_.createTextNode(QString::number(p[2]));
    new_altitude.appendChild(text_altitude);

    QDomElement new_range = doc_.createElement("range");
    new_lookat.appendChild(new_range);
    QDomText text_range = doc_.createTextNode(QString::number(o[0]));
    new_range.appendChild(text_range);

    QDomElement new_tilt = doc_.createElement("tilt");
    new_lookat.appendChild(new_tilt);
    QDomText text_tilt = doc_.createTextNode(QString::number(o[1]));
    new_tilt.appendChild(text_tilt);

    QDomElement new_heading = doc_.createElement("heading") ;
    new_lookat.appendChild(new_heading);
    QDomText text_heading = doc_.createTextNode(QString::number(o[2]));
    new_heading.appendChild(text_heading);

  }

  void bookmarks::set_name(const QString& s,uint i){
    if( i < names_.size() ){
      edit_kml_bookmark(names_[i],false,s);
      names_[i]=s;
    }
  }

  void bookmarks::delete_bookmark(uint i){
    edit_kml_bookmark(names_[i],true);
    std::vector<QString> temp_names_ = names_;
    std::vector<point3d_t> temp_locations_ = locations_;
    std::vector<point3d_t> temp_orientations_ = orientations_;
    clear_container();    
    for ( std::size_t j=0; j<temp_names_.size(); j++ ){
      if (j != i) {
	names_.push_back(temp_names_[j]);
	locations_.push_back(temp_locations_[j]);
	orientations_.push_back(temp_orientations_[j]);
      }
    }
   }

  void bookmarks::edit_kml_bookmark(const QString& s, bool to_delete, const QString& new_n){
    QDomNodeList element_list = doc_.documentElement().elementsByTagName("Placemark");
    for (std::size_t i=0; i < element_list.length(); ++i){
      QDomNode poiNode = element_list.item(i);
      QDomElement poiElement = poiNode.toElement();
      for(QDomNode n = poiElement.firstChild(); !n.isNull(); n = n.nextSibling()) {
	if (n.isElement()) {
	  QDomElement nElement = n.toElement();
	  if (nElement.tagName() == "name" && nElement.text() == s) {
	    if (to_delete){
	      doc_.documentElement().
		firstChildElement("Document").
		firstChildElement("Folder").
		removeChild(poiElement);
	    }else{
	      QDomElement oldTitleElement = poiElement.firstChildElement("name");

	      QDomElement newTitleElement = doc_.createElement("name");
	      QDomText text_name = doc_.createTextNode(new_n);
	      newTitleElement.appendChild(text_name);

	      poiElement.replaceChild(newTitleElement, oldTitleElement);
	      //std::cerr << "modify failed "<< std::endl;
	    }
	  } 
	}
      }
    }
  }


 
}
