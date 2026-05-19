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
#include <vic/geo/builder/mpi_quad_builder.hpp> 
#include <vic/geo/base/tilemap_config.hpp>
#include <vic/geo/builder/geo_utility.hpp>
#include <vic/xml/document.hpp>

#include <gdal_priv.h>
#include <iostream>
#include <fstream>
#include <string>
#include <sl/argument_parser.hpp>
#include <sl/clock.hpp>


// ======================================================================
// Option declararation

static std::string config_name = "victms.xml";

// ----------------------------------------------------------------------
// GENERAL OPTIONS

static sl::any arg_program_name(std::string("vic_geo_raster_quadtree_builder"));

static sl::any arg_main_help(false);
static sl::any arg_main_create(false);
static sl::any arg_main_update(false);

static sl::argument_record arg_main_table [] = {
  sl::argument_record("--help",
                      NULL,
                      new sl::generic_extractor<void>,
                      &arg_main_help,
                      "show program usage and command line options"),
  sl::argument_record("--create",
                      NULL,
                      new sl::generic_extractor<void>,
                      &arg_main_create,
                      "create a new quadtree"),
  sl::argument_record("--update",
                      NULL,
                      new sl::generic_extractor<void>,
                      &arg_main_update,
                      "update an existing quadtree by merging a set of tiles"),
};
static std::size_t arg_main_table_count = sizeof(arg_main_table)/sizeof(sl::argument_record);

static  sl::argument_parser arg_main_parser = sl::argument_parser(arg_main_table, 
								  arg_main_table_count);

static void arg_main_print_usage() {
  std::cerr << "Usage: " << arg_program_name.to_string() << " [options]" << std::endl;
  std::cerr << std::endl;
  std::cerr << "Options: " << std::endl;
  std::cerr << arg_main_parser.glossary();
  std::cerr << "Type : " << arg_program_name.to_string() << " [option] --help for option specific instructions" << std::endl;
  std::cerr << std::endl;
}

static void arg_main_print_error(const std::string& message) {
  std::cerr << arg_program_name.to_string() << ": " << message << std::endl;
  std::cerr << "Try `" << arg_program_name.to_string() << " --help' for more information." << std::endl;
}

// ----------------------------------------------------------------------
// CREATOR OPTIONS
static sl::any arg_create(false);
static sl::any arg_create_help(false);

static sl::any arg_create_quadtree_base_dir(std::string(""));
static sl::any arg_create_quadtree_name(std::string(""));

static sl::any arg_create_quadtree_profile(std::string("global-geodetic"));

static sl::any arg_create_quadtree_srs(std::string("EPSG:4326"));
static sl::any arg_create_quadtree_u0(double(-180.0));
static sl::any arg_create_quadtree_v0(double(- 90.0));
static sl::any arg_create_quadtree_u1(double( 180.0));
static sl::any arg_create_quadtree_v1(double(  90.0));
static sl::any arg_create_quadtree_nu(std::size_t(2));
static sl::any arg_create_quadtree_nv(std::size_t(1));

static sl::any arg_create_quadtree_img_width(std::size_t(256));
static sl::any arg_create_quadtree_img_height(std::size_t(256));
static sl::any arg_create_quadtree_img_format(std::string("JPG"));

static sl::argument_record arg_create_table [] = {
  sl::argument_record("--create",
                      NULL,
                      new sl::generic_extractor<void>,
                      &arg_create,
                      "create a new quadtree"),
  sl::argument_record("--help",
                      NULL,
                      new sl::generic_extractor<void>,
                      &arg_create_help,
                      "show program usage and command line options"),
  sl::argument_record("--quadtree-base-dir",
                      NULL,
                      new sl::generic_extractor<std::string>,
                      &arg_create_quadtree_base_dir,
                      "quadtree base directory - files are created in a subdirectory"),
  sl::argument_record("--quadtree-name",
                      NULL,
                      new sl::generic_extractor<std::string>,
                      &arg_create_quadtree_name,
                      "quadtree name, i.e, the subdirectory name"),
  sl::argument_record("--quadtree-profile",
                      NULL,
                      new sl::generic_extractor<std::string>,
                      &arg_create_quadtree_profile,
                      "TMS profile: global-geodetic / global-mercator / none"),
  sl::argument_record("--quadtree-srs",
                      NULL,
                      new sl::generic_extractor<std::string>,
                      &arg_create_quadtree_srs,
                      "output quadtree projection"),
  sl::argument_record("--quadtree-u0",
                      NULL,
                      new sl::generic_extractor<double>,
                      &arg_create_quadtree_u0,
                      "output quadtree bbox - u0"),
  sl::argument_record("--quadtree-v0",
                      NULL,
                      new sl::generic_extractor<double>,
                      &arg_create_quadtree_v0,
                      "output quadtree bbox - v0"),
  sl::argument_record("--quadtree-u1",
                      NULL,
                      new sl::generic_extractor<double>,
                      &arg_create_quadtree_u1,
                      "output quadtree bbox - u1"),
  sl::argument_record("--quadtree-v1",
                      NULL,
                      new sl::generic_extractor<double>,
                      &arg_create_quadtree_v1,
                      "output quadtree bbox - v1"),
  sl::argument_record("--quadtree-nu",
                      NULL,
                      new sl::generic_extractor<std::size_t>,
                      &arg_create_quadtree_nu,
                      "output quadtree root count in u direction"),
  sl::argument_record("--quadtree-nv",
                      NULL,
                      new sl::generic_extractor<std::size_t>,
                      &arg_create_quadtree_nv,
                      "output quadtree root count in v direction"),
  sl::argument_record("--quadtree-img-width",
                      NULL,
                      new sl::generic_extractor<std::size_t>,
                      &arg_create_quadtree_img_width,
                      "output quadtree quad pixel count in u direction"),
  sl::argument_record("--quadtree-img-height",
                      NULL,
                      new sl::generic_extractor<std::size_t>,
                      &arg_create_quadtree_img_height,
                      "output quadtree quad pixel count in v direction"),
  sl::argument_record("--quadtree-img-format",
                      NULL,
                      new sl::generic_extractor<std::string>,
                      &arg_create_quadtree_img_format,
                      "output quadtree quad format (JPG [RGB] | PNG [RGBA])"),
};
static std::size_t arg_create_table_count = sizeof(arg_create_table)/sizeof(sl::argument_record);

static  sl::argument_parser arg_create_parser = sl::argument_parser(arg_create_table, 
								    arg_create_table_count);

static void arg_create_print_usage() {
  std::cerr << "Usage: " << arg_program_name.to_string() << " --create [options]" << std::endl;
  std::cerr << std::endl;
  std::cerr << "Options: " << std::endl;
  std::cerr << arg_create_parser.glossary();
  std::cerr << std::endl;
}

static void arg_create_print_error(const std::string& message) {
  std::cerr << arg_program_name.to_string() << ": " << message << std::endl;
  std::cerr << "Try `" << arg_program_name.to_string() << " --create --help' for more information." << std::endl;
}

// ----------------------------------------------------------------------
// MERGER OPTIONS
static std::vector<std::string> arg_update_input_tiles;

static sl::any arg_update(false);
static sl::any arg_update_help(false);
static sl::any arg_update_quadtree_dir(std::string(""));

static sl::any arg_update_input_tiles_directory(std::string(""));
static sl::any arg_update_input_tiles_pattern(std::string("*.tif"));

static sl::any arg_update_input_tiles_default_srs(std::string("EPSG:4326"));
static sl::any arg_update_input_tiles_level(int(-1));

static sl::any arg_update_color_remap_black_out(int(0));
static sl::any arg_update_color_remap_black_in(int(0));
static sl::any arg_update_color_remap_white_out(int(255));
static sl::any arg_update_color_remap_white_in(int(255));
static sl::any arg_update_color_remap_below_to_black(int(0));
static sl::any arg_update_color_remap_above_to_black(int(254));

static sl::any arg_update_warp_max_error(double(0.25));

static sl::any arg_update_damaged_level_min(int(-1));				     
static sl::any arg_update_damaged_level_max(int(-1));

// ----------------------------------------------------------------------
static sl::argument_record arg_update_table [] = {
  sl::argument_record("--update",
                      NULL,
                      new sl::generic_extractor<void>,
                      &arg_update,
                      "update an existing quadtree by merging a set of tiles"),
  sl::argument_record("--help",
                      NULL,
                      new sl::generic_extractor<void>,
                      &arg_update_help,
                      "show program usage and command line options"),
  sl::argument_record("--quadtree-dir",
                      NULL,
                      new sl::generic_extractor<std::string>,
                      &arg_update_quadtree_dir,
                      "quadtree directory, containing data and victms.xml configuration"),
  sl::argument_record("--input-tiles-directory",
                      NULL,
                      new sl::generic_extractor<std::string>,
                      &arg_update_input_tiles_directory,
                      "directory where input tiles are located"),
  sl::argument_record("--input-tiles-pattern",
                      NULL,
                      new sl::generic_extractor<std::string>,
                      &arg_update_input_tiles_pattern,
                      "pattern for selecting input tiles in input dir"),
  sl::argument_record("--input-tiles-default-srs",
                      NULL,
                      new sl::generic_extractor<std::string>,
                      &arg_update_input_tiles_default_srs,
                      "projection of input tiles (used if files are not fully geocoded)"),
  sl::argument_record("--input-tiles-level",
                      NULL,
                      new sl::generic_extractor<int>,
                      &arg_update_input_tiles_level,
                      "quadtree level at which to insert given tiles (if -1, level is automatically determined from input resolution)"),
  sl::argument_record("--color-remap-black-out",
                      NULL,
                      new sl::generic_extractor<int>,
                      &arg_update_color_remap_black_out,
                      "output quadtree color remap black output"),
  sl::argument_record("--color-remap-black-in",
                      NULL,
                      new sl::generic_extractor<int>,
                      &arg_update_color_remap_black_in,
                      "output quadtree color remap black input"),
  sl::argument_record("--color-remap-white-out",
                      NULL,
                      new sl::generic_extractor<int>,
                      &arg_update_color_remap_white_out,
                      "output quadtree color remap white output"),
  sl::argument_record("--color-remap-white-in",
                      NULL,
                      new sl::generic_extractor<int>,
                      &arg_update_color_remap_white_in,
                      "output quadtree color remap white input"),
  sl::argument_record("--color-remap-below-to-black",
                      NULL,
                      new sl::generic_extractor<int>,
                      &arg_update_color_remap_below_to_black,
                      "output quadtree color remap below to black"),
  sl::argument_record("--color-remap-above-to-black",
                      NULL,
                      new sl::generic_extractor<int>,
                      &arg_update_color_remap_above_to_black,
                      "output quadtree color remap above to black"),
  sl::argument_record("--warp-max-error",
                      NULL,
                      new sl::generic_extractor<double>,
                      &arg_update_warp_max_error,
                      "maximum error for image reprojection in output pixels"),

#if 0
  // DOES NOT WORK
  sl::argument_record("--quadtree-damaged-level-min",
                      NULL,
                      new sl::generic_extractor<int>,
                      &arg_update_damaged_level_min,
                      "rebuild parents of all tiles from this level to max"),
  sl::argument_record("--quadtree-damaged-level-max",
                      NULL,
                      new sl::generic_extractor<int>,
                      &arg_update_damaged_level_max,
                      "rebuild parents of all tiles from min to this level"),
#endif
};
static std::size_t arg_update_table_count = sizeof(arg_update_table)/sizeof(sl::argument_record);

static  sl::argument_parser arg_update_parser = sl::argument_parser(arg_update_table, 
								    arg_update_table_count);

static void arg_update_print_usage() {
  std::cerr << "Usage: " << arg_program_name.to_string() << " --update [options]" << std::endl;
  std::cerr << std::endl;
  std::cerr << "Options: " << std::endl;
  std::cerr << arg_update_parser.glossary();
  std::cerr << std::endl;
}

static void arg_update_print_error(const std::string& message) {
  std::cerr << arg_program_name.to_string() << ": " << message << std::endl;
std::cerr << "Try `" << arg_program_name.to_string() << " --create --help' for more information." << std::endl;
}


// ======================================================================

static int builder_create(int argc, char *argv[]) {
  arg_program_name = std::string(argv[0]);

  arg_create_parser.accept_extra_arguments(false);
  arg_create_parser.reset_defaults();

  // Parse arguments
  arg_create_parser.parse(argc, (const char**)argv);

  int result = 0;
  if (!arg_create_help.empty()) {
    arg_create_print_usage();
    result = 0;
  } else if (!arg_create_parser.last_operation_success()) {
    arg_create_print_error(arg_create_parser.last_error_string());
    result = 1;
  } else {
    // CREATE A NEW QUADTREE FROM PARAMETERS

    std::string quadtree_base_dir        = arg_create_quadtree_base_dir.to_string();
    std::string quadtree_name            = arg_create_quadtree_name.to_string();
    std::string quadtree_profile         = arg_create_quadtree_profile.to_string();

    std::string quadtree_srs             = arg_create_quadtree_srs.to_string();
    double      quadtree_u0              = sl::any_cast<double>(arg_create_quadtree_u0);
    double      quadtree_v0              = sl::any_cast<double>(arg_create_quadtree_v0);
    double      quadtree_u1              = sl::any_cast<double>(arg_create_quadtree_u1);
    double      quadtree_v1              = sl::any_cast<double>(arg_create_quadtree_v1);

    std::size_t quadtree_nu              = sl::any_cast<std::size_t>(arg_create_quadtree_nu);
    std::size_t quadtree_nv              = sl::any_cast<std::size_t>(arg_create_quadtree_nv);
    std::size_t quadtree_img_width       = sl::any_cast<std::size_t>(arg_create_quadtree_img_width);
    std::size_t quadtree_img_height      = sl::any_cast<std::size_t>(arg_create_quadtree_img_height);
    std::string quadtree_img_format      = arg_create_quadtree_img_format.to_string();
    
    std::string quadtree_dir = quadtree_base_dir + "/" + quadtree_name;
    if (quadtree_base_dir.empty()) {
      arg_create_print_error(std::string() + "Must specify quadtree base directory.");
      result = 1;
    } else if (quadtree_name.empty()) {
      arg_create_print_error(std::string() + "Must specify quadtree name directory.");
      result = 1;
    } else if (!vic::geo::geo_utility::has_dir(quadtree_base_dir)) {
      arg_create_print_error(std::string() + "Directory '" + quadtree_base_dir + "' non existent");
      result = 1;
    } else if (vic::geo::geo_utility::has_dir(quadtree_dir)) {
      arg_create_print_error(std::string() + "Directory '" + quadtree_base_dir + "' already exists");
      result = 1;
    } else if (!vic::geo::geo_utility::mkpath(quadtree_dir)) {
      arg_create_print_error(std::string() + "Unable to create directory '" + quadtree_base_dir + "'");
      result = 1;
    } else {
      vic::geo::base::tilemap_config tc;
   	      
      tc.set_name(quadtree_name);
      tc.set_profile(quadtree_profile);
      if (quadtree_profile == "global-geodetic") {
	// FIXME CHECK SRS
	tc.set_srs("EPSG:4326");
	tc.set_bbox(-180.0, -90.0, 180.0, 90.0);
	tc.set_nu_nv(2, 1);
	tc.set_img_width_height(256, 256);
      } else if (quadtree_profile == "global-mercator") {
	tc.set_srs("OSGEO:41001");
	tc.set_bbox(-20037508.34, -20037508.34, 20037508.34, 20037508.34);
	tc.set_nu_nv(2, 2);
	tc.set_img_width_height(256, 256);
      }	else if (quadtree_profile == "none") {
	tc.set_srs(quadtree_srs);
	tc.set_bbox(quadtree_u0, quadtree_v0, quadtree_u1, quadtree_v1);
	tc.set_nu_nv(quadtree_nu, quadtree_nv);
	tc.set_img_width_height(quadtree_img_width, quadtree_img_height);
      }	else {
	arg_create_print_error(std::string() + "Unsupported profile");
	result = 1;
      }	
    
      if (quadtree_img_format == "JPG" ||
	  quadtree_img_format == "jpg" ||
	  quadtree_img_format == "jpeg" ||
	  quadtree_img_format == "JPEG") {
	tc.set_mime("image/jpeg");
	tc.set_extension("jpg");
      } else if ((quadtree_img_format == "PNG") ||
		 (quadtree_img_format == "png")) {
	tc.set_mime("image/png");
	tc.set_extension("png");
      }	else {
	arg_create_print_error(std::string() + "Unsupported image format");
	result = 1;
      }
    
      if (result == 0) {
	std::string config_full_fname = quadtree_dir + "/" + config_name;
	std::ofstream fout(config_full_fname.c_str());
	if (!fout) {
	  arg_create_print_error(std::string() + "Cannot open " + config_full_fname);
	  result = 1;
	} else {
	  fout << 
	    "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n" <<
	    "<victms>\n" << 
	    tc.description() << "\n"
	    "</victms>\n";
	  fout.close();
	}	  
      }
    }
  }
  return result;
}

// ======================================================================

static int builder_update(int argc, char *argv[]) {
  arg_program_name = std::string(argv[0]);

  arg_update_parser.accept_extra_arguments(true);
  arg_update_parser.reset_defaults();

  // Parse arguments
  arg_update_parser.parse(argc, (const char**)argv);

  // Parse arguments
  arg_update_parser.parse(argc, (const char**)argv);
  arg_update_input_tiles.clear();
  for (std::list<std::string>::const_iterator it = arg_update_parser.last_extra_arguments().begin();
       it != arg_update_parser.last_extra_arguments().end();
       ++it) {
    std::string fname = *it;
    arg_update_input_tiles.push_back(fname);
  }

  int result = 0;
  if (!arg_update_help.empty()) {
    arg_update_print_usage();
    result = 0;
  } else if (!arg_update_parser.last_operation_success()) {
    arg_update_print_error(arg_update_parser.last_error_string());
    result = 1;
  } else {
    // UPDATE AN EXISTING QUADTREE FROM SET OF TILES

    std::string quadtree_dir = arg_update_quadtree_dir.to_string();

    std::string input_tiles_directory     = arg_update_input_tiles_directory.to_string();
    std::string input_tiles_pattern       = arg_update_input_tiles_pattern.to_string();
    std::string input_tiles_default_srs   = arg_update_input_tiles_default_srs.to_string();
    int         input_tiles_level         = sl::any_cast<int>(arg_update_input_tiles_level);

    int         color_remap_black_out = sl::any_cast<int>(arg_update_color_remap_black_out);
    int         color_remap_black_in = sl::any_cast<int>(arg_update_color_remap_black_in);
    int         color_remap_white_out = sl::any_cast<int>(arg_update_color_remap_white_out);
    int         color_remap_white_in = sl::any_cast<int>(arg_update_color_remap_white_in);
    int         color_remap_below_to_black = sl::any_cast<int>(arg_update_color_remap_below_to_black);
    int         color_remap_above_to_black = sl::any_cast<int>(arg_update_color_remap_above_to_black);

    double      warp_max_error = sl::any_cast<double>(arg_update_warp_max_error);

#if 0
    int         quadtree_damaged_level_min = sl::any_cast<int>(arg_update_quadtree_damaged_level_min);
    int         quadtree_damaged_level_max = sl::any_cast<int>(arg_update_quadtree_damaged_level_max);
#endif
    
    std::string config_full_fname = quadtree_dir + "/" + config_name;
    //    std::cerr << "color_remap_above_to_black " << color_remap_above_to_black << std::endl;

    std::ifstream fin(config_full_fname.c_str());
    if (!fin) {
      arg_update_print_error(std::string() + "Cannot open " + config_full_fname);
      result = 1;
    } else {
      vic::xml::document doc;
      doc.parse(fin);
      fin.close();
      if(doc.error()) {
	arg_update_print_error(std::string() + "Error parsing " + config_full_fname + ":" + doc.error_msg());
	result = 1;
      } else {	
	vic::geo::base::tilemap_config tc;
	vic::xml::node_iterator xml_ptr=doc.first_root("victms");
	if (xml_ptr.is_null() || !tc.parse(xml_ptr.down())) {
	  arg_update_print_error(std::string() + "Error parsing " + config_full_fname);
	  result = 1;
	} else {	
	  // init GDAL
	  GDALAllRegister();
	  GDALSetCacheMax(256*1024*1024);  // FIXME?

	  if (vic::mpi::process_rank() == 0) {
	    std::cout << "READ DESCRIPTION: " << std::endl << tc.description() << std::endl;
	  }

	  // Customize builder using loaded data
	  vic::geo::mpi_quad_builder builder;

	  builder.set_quadtree_root_dir(quadtree_dir);
	  builder.set_quadtree_projection(tc.srs());
	  builder.set_quadtree_extent(tc.bbox_lo(0),
				      tc.bbox_lo(1),
				      tc.bbox_hi(0),
				      tc.bbox_hi(1));
	  builder.set_quadtree_root_count(tc.nu(), 
					  tc.nv());
	  builder.set_quad_size(tc.img_width(),
				tc.img_height());

	  if ((tc.mime() == "image/jpeg") &&
	      (tc.extension() == "jpg")) {
	    builder.set_quadtree_output_format("JPEG", "jpg"); // Fixme quality?
	    builder.set_quad_band_count(3);
	  } else if ((tc.mime() == "image/png") &&
		     (tc.extension() == "png")) {
	    builder.set_quadtree_output_format("PNG", "png"); // Fixme quality?
	    builder.set_quad_band_count(4);
	  } else {
	    arg_update_print_error(std::string() + "Unsupported image format/extension");
	    result = 1;
	  }

	  builder.set_quad_warp_max_error(warp_max_error);
	  //	  std::cerr << "color_remap_above_to_black " << color_remap_above_to_black << std::endl;
	  builder.set_color_remap_parameters(color_remap_black_out,
					     color_remap_black_in,
					     color_remap_white_out,
					     color_remap_white_in,
					     color_remap_below_to_black,
					     color_remap_above_to_black);
#if 0
	  builder.set_damaged_level_range(quadtree_damaged_level_min, quadtree_damaged_level_max);
#endif
	  builder.set_default_src_proj(input_tiles_default_srs);	

	  if (!result) {
	    // Process
	    
	    builder.begin_processing();
	    {
	      if (!input_tiles_directory.empty()) {
		builder.process_directory(input_tiles_directory,
					  input_tiles_pattern,
					  input_tiles_level);
	      }
	      for (std::size_t i=0; i<arg_update_input_tiles.size(); ++i) {
		builder.process_tile(arg_update_input_tiles[i], input_tiles_level);
	      }
	    }
	    builder.end_processing();
	   
	    // FIXME
	    // Here, update xml with new level count
	  }
	}
      }
    }
  }
  return result;
}


// ======================================================================

static int builder_main(int argc, char *argv[]) {
  arg_program_name = std::string(argv[0]);

  arg_main_parser.accept_extra_arguments(true);
  arg_main_parser.reset_defaults();

  // Parse arguments
  arg_main_parser.parse(argc, (const char**)argv);

  int result = 0;
  if (!arg_main_create.empty() && !arg_main_update.empty()) {
    arg_main_print_error("Cannot use --create and --update at the same time");
    result = 1;
  } else if (!arg_main_create.empty()) {
    result = builder_create(argc, argv);
  } else if (!arg_main_update.empty()) {
    result = builder_update(argc, argv);
  } else if (!arg_main_help.empty()) {
    arg_main_print_usage();
    result = 0;
  } else if (arg_main_create.empty() && arg_main_update.empty()) {
    arg_main_print_error("Select --create or --update");
    result = 1;
  } else if (!arg_main_parser.last_operation_success()) {
    arg_main_print_error(arg_main_parser.last_error_string());
    result = 1;
  }
  return result;
}

// ======================================================================
// Processing

int main(int argc, char *argv[]) {
  vic::mpi::initialize(&argc, const_cast<char***>(&argv));

  arg_program_name = std::string(argv[0]);

  int result = builder_main(argc, argv);

  if (result) {
    std::cerr << "Process " << vic::mpi::process_rank() << "/" << vic::mpi::process_count() << " on " << vic::mpi::processor_name() << " exiting with error " << result << std::endl;
  } else {
    std::cerr << "Process " << vic::mpi::process_rank() << "/" << vic::mpi::process_count() << " on " << vic::mpi::processor_name() << " exiting without error" << std::endl;
  }
  vic::mpi::finalize();

  return result;
}
