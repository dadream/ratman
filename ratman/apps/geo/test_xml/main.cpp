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
#include <iostream>
#include <string>
#include <fstream>
#include <vic/geo/base/tms_root_resource.hpp>
#include <vic/geo/base/tms_service_resource.hpp>
#include <vic/geo/base/tms_tilemap_resource.hpp>


int main(int argc, char *argv[]) {

  std::cerr << "############### Root Test" << std::endl;
  std::ifstream in_root("root.xml", std::ios::in);

  std::string xml;
  while (!in_root.eof()) {
    std::string data;
    std::getline(in_root,data);
    xml+=data + '\n';
  }
  in_root.close();

  std::cerr << xml << std::endl;
  if (!xml.empty()) {
    vic::geo::base::tms_root_resource r(xml);
    if (!r.last_operation_success()) {
      std::cerr << "ERROR " << r.last_error_message() << std::endl;
    } else {
      std::cerr << "Service Count= " << r.service_count() << std::endl;
      std::cerr << r.description() << std::endl;
    }
  }

  std::cerr << "############### End Root Test" << std::endl << std::endl;

  std::cerr << "############### Service Test" << std::endl;
  std::ifstream in_service("service.xml", std::ios::in);
  xml="";
  while (!in_service.eof()) {
    std::string data;
    std::getline(in_service,data);
    xml+=data + '\n';
  }
  in_service.close();

  std::cerr << xml << std::endl;
  if (!xml.empty()) {
    vic::geo::base::tms_service_resource r(xml);
    if (!r.last_operation_success()) {
      std::cerr << "ERROR " << r.last_error_message() << std::endl;
    } else {
      std::cerr << "Tilemap Count= " << r.tilemap_count() << std::endl;
      std::cerr << r.description() << std::endl;
    }
  }
  std::cerr << "############### End Service Test" << std::endl << std::endl;

  std::cerr << "############### Tilemap Test" << std::endl;
  std::ifstream in_tilemap("tilemap.xml", std::ios::in);
  xml="";
  while (!in_tilemap.eof()) {
    std::string data;
    std::getline(in_tilemap,data);
    xml+=data + '\n';
  }
  in_tilemap.close();

  std::cerr << xml << std::endl;
  if (!xml.empty()) {
    vic::geo::base::tms_tilemap_resource r(xml);
    if (!r.last_operation_success()) {
      std::cerr << "ERROR " << r.last_error_message() << std::endl;
    } else {
      std::cerr << "Tileset Count= " << r.tileset_count() << std::endl;
      std::cerr << r.description() << std::endl;
    }
  }
  std::cerr << "############### End Tilemap Test" << std::endl << std::endl;

}
