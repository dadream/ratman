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
#include <vic/cbdam/base/geoimage_quad_fetcher.hpp>
#include <vic/cbdam/base/dummy_geoimage_quad_fetcher.hpp>
#include <vic/cbdam/base/victms_geoimage_quad_fetcher.hpp>
#include <vic/cbdam/base/wms_geoimage_quad_fetcher.hpp>
#include <vic/cbdam/base/loaded_geoimage_quad_fetcher.hpp>
#include <sl/argument_parser.hpp>
#include <qapplication.h>

#include "cbdam_window.hpp"

/*
  The main program is here.
 */

//////////// FIXME
// ======================================================================
// Option declararation
static std::vector<std::string> color_file_names;
std::string elevation_file_name;

static sl::any arg_program_name(std::string("cbdam_viewer"));
static sl::any arg_elevation_file_name(std::string(""));
static sl::any arg_buildings_file_name(std::string(""));
static sl::any arg_procedural_texture(false);
static sl::any arg_help(false);
static sl::any arg_verify_script(std::string(""));
static sl::any arg_verify_output_dir(std::string(""));
static sl::any arg_verify_exit(false);
static sl::any arg_verify_window_size(std::string(""));
static sl::any arg_verify_log_state(false);

static sl::argument_record arg_table [] = {
  sl::argument_record("--help",    
                      NULL,   
                      new sl::generic_extractor<void>,                
                      &arg_help,       
                      "show program usage and command line options"),
  sl::argument_record("--elevation",
                      "file",
                      new sl::generic_extractor<std::string>,
                      &arg_elevation_file_name,
                      "elevation file"),
  sl::argument_record("--buildings-file",
                      "file",
                      new sl::generic_extractor<std::string>,
                      &arg_buildings_file_name,
                      "buildings file"),
  sl::argument_record("--procedural-texture",
                      "comment",
                      new sl::generic_extractor<void>,
                      &arg_procedural_texture,
                      "add procedural texture layer"),
  sl::argument_record("--verify-script",
                      "file",
                      new sl::generic_extractor<std::string>,
                      &arg_verify_script,
                      "run deterministic viewer verification actions from file"),
  sl::argument_record("--verify-output-dir",
                      "dir",
                      new sl::generic_extractor<std::string>,
                      &arg_verify_output_dir,
                      "write viewer verification artifacts to directory"),
  sl::argument_record("--verify-exit",
                      NULL,
                      new sl::generic_extractor<void>,
                      &arg_verify_exit,
                      "exit after verification actions finish"),
  sl::argument_record("--verify-window-size",
                      "WIDTHxHEIGHT",
                      new sl::generic_extractor<std::string>,
                      &arg_verify_window_size,
                      "set deterministic verification window size"),
  sl::argument_record("--verify-log-state",
                      NULL,
                      new sl::generic_extractor<void>,
                      &arg_verify_log_state,
                      "log structured verification state events"),

};

static std::size_t arg_table_count = sizeof(arg_table)/sizeof(sl::argument_record);

static  sl::argument_parser arg_parser = sl::argument_parser(arg_table, arg_table_count);

static void print_usage() {
  std::cerr << "Usage: " << arg_program_name.to_string() << " [options] [file]" << std::endl;
  std::cerr << std::endl;
  std::cerr << "Options: " << std::endl;
  std::cerr << arg_parser.glossary();
  std::cerr << std::endl;
}

static void print_error_and_exit(const std::string& message) {
  std::cerr << arg_program_name.to_string() << ": " << message << std::endl;
  std::cerr << "Try `" << arg_program_name.to_string() << " --help' for more information." << std::endl;
  exit(-1);
}

int main( int argc, char **argv ) {
  std::cerr << "starting program" << std::endl;
  arg_program_name = std::string(argv[0]);
  arg_parser.accept_extra_arguments(true);
  arg_parser.reset_defaults();

  // Parse arguments
  arg_parser.parse(argc, (const char**)argv);
  if (!arg_help.empty()) {
    print_usage();
    exit(0);
  }
  if (!arg_parser.last_operation_success()) {
    print_error_and_exit(arg_parser.last_error_string());
  }

  for (std::list<std::string>::const_iterator it = arg_parser.last_extra_arguments().begin();
       it != arg_parser.last_extra_arguments().end();
       ++it) {
    std::string fname = *it;
    color_file_names.push_back(fname);
  }
  
  QApplication::setColorSpec( QApplication::CustomColor );
  QApplication qt_app(argc,argv);

  if ( !QGLFormat::hasOpenGL() ) {
    qWarning( "This system has no OpenGL support. Exiting." );
    return -1;
  }

  cbdam_window* cbdam_w = new cbdam_window;

  // load heights
  std::string elevation_file_name = arg_elevation_file_name.to_string();
  if (elevation_file_name.empty()) {
    std::cerr << "no elevation file name specified (use --elevation option)" << std::endl;
    delete cbdam_w;
    return 1;
  } else {
    if (!cbdam_w->open(elevation_file_name)) {
      std::cerr << "unable to load elevation " << elevation_file_name << std::endl;
      delete cbdam_w;
      return 2;
    }
  }

  
  // load textures
  std::string srs = cbdam_w->elevation_fetcher()->srs();
  cbdam::aabox2d_t uv_box = cbdam_w->elevation_fetcher()->uv_box();
  std::size_t quad_width = 256;
  for(std::size_t i = 0; i < color_file_names.size(); ++i) {
    std::cerr << "adding texture layer " << color_file_names[i] << std::endl;
    cbdam::geoimage_quad_fetcher* fetcher = new cbdam::victms_geoimage_quad_fetcher(color_file_names[i], srs, uv_box, quad_width);
    fetcher->connect();
    if (fetcher->is_connected()) {
      std::cerr << "connected to texture layer " << color_file_names[i] << std::endl;
      std::size_t first_level = 0;
      std::size_t last_level = 64;
      // SOME HACKS FOR SELCTION OF base vs overlay and heights. if 4rd layer is clouds it will be active up to 3000km.
      bool is_active = true; //i == 0;
      bool is_base_layer = i == 0;
      double min_altitude = (i != 3 ? -10e30 : 4000000.0);
      double max_altitude = 10e30;
      cbdam_w->insert_color_layer(color_file_names[i], fetcher, first_level, last_level, min_altitude, max_altitude, is_base_layer, is_active);
    } else {
      std::cerr << "cannot connect to texture layer " << color_file_names[i] << std::endl;
    }
  }
  
#if 0
  // ========== FIXME WMS TEST
  {
    cbdam::geoimage_quad_fetcher* fetcher = new cbdam::wms_geoimage_quad_fetcher("http://www2.demis.nl/mapserver/request.asp",
										 "Borders,Coastlines",
										 "image/png",
										 "Transparent=true",
										 srs, uv_box, quad_width);

    fetcher->connect();
    if (fetcher->is_connected()) {
      std::cerr << "connected to WMS texture layer " << std::endl;
      std::size_t first_level = 4;
      std::size_t last_level = 64;
      bool is_active = true; //i == 0;
      cbdam_w->insert_color_layer("WMS TEST", fetcher, first_level, last_level, -10e0, 10e30, false, is_active);
    } else {
      std::cerr << "cannot connect to WMS texture layer " << std::endl;
    }
  }
#endif

#if 0
  // ========== FIXME LOADED_IMAGE TEST
  {
    cbdam::geoimage_quad_fetcher* fetcher = new cbdam::loaded_geoimage_quad_fetcher("http://www.crs4.it/vic/data/images/img-exported/stmatthew-geowall.png",
										    srs, uv_box, quad_width);

    fetcher->connect();
    if (fetcher->is_connected()) {
      std::cerr << "connected to loaded_image texture layer " << std::endl;
      std::size_t first_level = 0;
      std::size_t last_level = 64;
      bool is_active = true; //i == 0;
      cbdam_w->insert_color_layer("IMAGE TEST", fetcher, first_level, last_level, -10e30, 10e30, false, is_active);
    } else {
      std::cerr << "cannot connect to IMAGE texture layer " << std::endl;
    }
  }
#endif
  
  if (!arg_procedural_texture.empty()) {
    std::cerr << "adding procedural texture layer" << std::endl;
    cbdam::geoimage_quad_fetcher* fetcher = new cbdam::dummy_geoimage_quad_fetcher(srs, uv_box, quad_width);
    std::size_t first_level = 0;
    std::size_t last_level = 64;
    fetcher->connect();
    cbdam_w->insert_color_layer("Procedural Texture", fetcher, first_level, last_level, 100000.0, 10e30, false, true);
  }

  // load buildings
  if (arg_buildings_file_name.to_string().size() != 0) {
    std::string buildings_file_name = arg_buildings_file_name.to_string();    
    cbdam_w->init_buildings(buildings_file_name.c_str());
  }

  QObject::connect(cbdam_w, SIGNAL(stop_rendering()), &qt_app, SLOT(quit()));

  int window_width = 800;
  int window_height = 600;
  std::string verify_window_size = arg_verify_window_size.to_string();
  if (!verify_window_size.empty()) {
    std::size_t x_pos = verify_window_size.find('x');
    if (x_pos == std::string::npos) {
      x_pos = verify_window_size.find('X');
    }
    if (x_pos != std::string::npos) {
      window_width = std::atoi(verify_window_size.substr(0, x_pos).c_str());
      window_height = std::atoi(verify_window_size.substr(x_pos + 1).c_str());
    }
    if (window_width <= 0 || window_height <= 0) {
      std::cerr << "invalid verify window size " << verify_window_size << std::endl;
      delete cbdam_w;
      return 3;
    }
  }
  cbdam_w->resize(window_width, window_height);

  std::string verify_script = arg_verify_script.to_string();
  if (!verify_script.empty()) {
    std::string verify_output_dir = arg_verify_output_dir.to_string();
    if (verify_output_dir.empty()) {
      std::cerr << "--verify-output-dir is required with --verify-script" << std::endl;
      delete cbdam_w;
      return 3;
    }
    if (!cbdam_w->configure_verification(verify_script,
					 verify_output_dir,
					 !arg_verify_exit.empty(),
					 !arg_verify_log_state.empty())) {
      delete cbdam_w;
      return 3;
    }
  }

  cbdam_w->show();

  bool result = qt_app.exec();
  if (cbdam_w->verification_failed()) {
    result = true;
  }

  delete cbdam_w;
  return result;
}

