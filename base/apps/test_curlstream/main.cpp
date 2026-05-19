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
#include <vic/curlstream/curlstream.hpp>
#include <stdlib.h>
#include <iostream>
#include <string>

int main(int argc, char *argv[]) {
  std::string file_name;
  if (argc==2) {
    file_name = argv[1];
  } else {
    std::cerr << "Filename/URL missing" << std::endl;
    exit(EXIT_FAILURE);    
  }

  vic::icurlstream ifile;
  ifile.open(file_name.c_str());

  if (!ifile) {
    std::cerr << "Load error" << std::endl;
    exit(EXIT_FAILURE);
  }
  while (ifile.good()) {
    std::string line;
    std::getline(ifile,line);
    if(!line.empty()) {
      std::cout << line << std::endl;
    }
    if (ifile.fail() &&!ifile.eof()) {
      std::cerr << "Failed to read" << std::endl;
      exit(EXIT_FAILURE);
    }      
  }
  ifile.close();
}
