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
#include <opossum_window.hpp>
#include <qapplication.h>
#include <qgl.h>
#include <iostream>

void usage() { 
  std::cerr << "usage: opossum_viewer -height height-file-name [-color color-file-name] [-buildings buildings-file-name]\n";
}


void print_error_and_exit(const std::string& str) {
  std::cerr << str << std::endl;
  usage();
  exit(0);
}

int main(int argc, char **argv) {
  if (argc < 3) {
    print_error_and_exit("wrong parameter number");
  }

  const char* input_height_file_name = 0;
  const char* input_color_file_name = 0;
  const char* input_buildings_file_name = 0;
  int count = 1;

  // parse file name parameters
  while(count < argc) {
    if (argv[count][0] == '-') {
      const char* param = &argv[count][1];
      ++count;
      if (count < argc) {
        if (strcmp(param, "height") == 0) {
          input_height_file_name = argv[count];
        } else if (strcmp(param, "color") == 0) {
          input_color_file_name = argv[count];
        } else if (strcmp(param, "buildings") == 0) {
          input_buildings_file_name = argv[count];
        } 
        
        ++count;
      } else {
        print_error_and_exit((std::string)"missing parameter after " + argv[count-1]);
      }
    } else {
      print_error_and_exit((std::string)"missing type of parameter " + argv[count-1]);
    }
  }

  QApplication::setColorSpec(QApplication::CustomColor);
  QApplication qt_app(argc,argv);

  if (!QGLFormat::hasOpenGL()) {
    qWarning("This system has no OpenGL support. Exiting.");
    return -1;
  }

  opossum_window opossum_window;
    
  if (!opossum_window.init(input_height_file_name, input_color_file_name, input_buildings_file_name)) {
    print_error_and_exit("Failed loading data. Exiting.\n");
  } else {
    QObject::connect(&opossum_window, SIGNAL(stop_rendering()), &qt_app, SLOT(quit()));

    opossum_window.resize(800, 600);
    qt_app.setMainWidget(&opossum_window);
    
    opossum_window.show();

    return qt_app.exec();
  }
}





