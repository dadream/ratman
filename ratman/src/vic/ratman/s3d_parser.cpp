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
#include <vic/ratman/s3d_parser.hpp>
#include <vic/curlstream/curlstream.hpp>

#include <sl/assert.hpp>

#include <QTextStream>
#include <QDomDocument>

namespace ratman {

  s3d_parser::s3d_parser() :
    lon_(0),
    lat_(0),
    error_(false),
    error_msg_("") {
  }


  void s3d_parser::read_and_parse(const QString& fname) {
    error_=false;
    error_msg_="";
    lon_=0;
    lat_=0;

    vic::icurlstream ifile;
    ifile.open(fname.toStdString().c_str());
    if (!ifile) {
      QTextStream(&error_msg_) << "Failed to open '" << fname << "'";
      error_=true;
      return;
    } 
    QByteArray data;
    while (ifile.good()) {
      std::string line;
      std::getline(ifile,line);
      if(!line.empty()) {
        data.append(QString(line.c_str()));
        data.append('\n');
      }
    }
    ifile.close();
    QDomDocument doc;
    doc.setContent(data);
    QDomElement root = doc.documentElement();
    QDomNodeList placemarks = root.elementsByTagName("Placemark");
    if (placemarks.size() < 1) {
      QTextStream(&error_msg_) << "File '" << fname << "' not correct";
      error_=true;
    }
    QDomNodeList lookat_items=placemarks.item(0).toElement().elementsByTagName("LookAt");
    if (lookat_items.size() < 1) {
      QTextStream(&error_msg_) << "File '" << fname << "' is not correct";
      error_=true;
      return;
    }
    QDomNodeList lon_items = lookat_items.item(0).toElement().elementsByTagName("longitude");
    QDomNodeList lat_items = lookat_items.item(0).toElement().elementsByTagName("latitude");
    if (lon_items.size() < 1 && lat_items.size() < 1) {
      QTextStream(&error_msg_) << "Coordinates missing in file '" << fname;
      error_=true;
      return;
    }
    QDomNode temp=lon_items.item(0);
    lon_=temp.toElement().text().toDouble();
    temp=lat_items.item(0);
    lat_=temp.toElement().text().toDouble();
    SL_TRACE_OUT(1) << "LON: " << lon_ << " LAT: " << lat_ << std::endl;
  }
  

}
