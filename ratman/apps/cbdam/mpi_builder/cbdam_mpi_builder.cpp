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
#include <vic/cbdam/base/coordinate_transform.hpp>
#include <vic/cbdam/geo/map_raster_sampler.hpp>
#include <vic/cbdam/geo/map_mosaic_sampler.hpp>
#include <vic/cbdam/mpi/mpi_builder.hpp>
#include <sl/argument_parser.hpp>
#include <sl/clock.hpp>
#include <shapefil.h>



// ======================================================================
// Option declararation
// user can specify multiple directories, with a single pattern for all the dir.
// First in First Sampled FIFS strategy
static std::vector<std::string> arg_input_file_names;

static sl::any arg_program_name(std::string("cbdam_mpi_builder"));
static sl::any arg_output_filename(std::string("cbdamrepo")); 
static sl::any arg_pattern(std::string("")); 

static sl::any arg_patch_dim(std::size_t(64));
static sl::any arg_dh(float(1.0f));
static sl::any arg_use_amax_error(false);
static sl::any arg_eps(float(1.0f));
static sl::any arg_u0(float(0.0));
static sl::any arg_v0(float(0.0));
static sl::any arg_u1(float(0.0));
static sl::any arg_v1(float(0.0));
static sl::any arg_color_remapping_alpha(std::size_t(0));
static sl::any arg_color_remapping_beta(std::size_t(255));
static sl::any arg_color(false);
static sl::any arg_keep_graph(false);
static sl::any arg_reuse_graph(false);
static sl::any arg_is_planar(false);
static sl::any arg_is_cylindrical_latlon(false);
static sl::any arg_is_spherical_latlon(false);
static sl::any arg_min_sample_spacing(float(1.0f));
static sl::any arg_spherical_radius(float(1.0f));

static sl::any arg_tmp_dir(std::string(""));
static sl::any arg_srs(std::string("EPSG:4326"));
static sl::any arg_about(std::string("CBDAM Elevation Layer"));
static sl::any arg_help(false);

static sl::argument_record arg_table [] = {
  sl::argument_record("--help",    
                      NULL,   
                      new sl::generic_extractor<void>,                
                      &arg_help,       
                      "show program usage and command line options"),
  sl::argument_record("--output-file",
                      "file",
                      new sl::generic_extractor<std::string>,
                      &arg_output_filename,
                      "cbdam file."),
  sl::argument_record("--pattern",
                      "file",
                      new sl::generic_extractor<std::string>,
                      &arg_pattern,
                      "pattern to select files present in the input directory."),
  sl::argument_record("--tmp-dir",
                      "file",
                      new sl::generic_extractor<std::string>,
                      &arg_tmp_dir,
                      "path to tmp directory - by default the same as the output file's directory."),
  sl::argument_record("--patch-dim",
                      "int",
                      new sl::generic_extractor<std::size_t>,
                      &arg_patch_dim,
                      "Desired patch dim"),
  sl::argument_record("--height-scale",
                      "float",
                      new sl::generic_extractor<float>,
                      &arg_dh,
                      "Desired height scale factor"),
  sl::argument_record("--use-amax-error",
                      NULL,
                      new sl::generic_extractor<void>,
                      &arg_use_amax_error,
                      "use amax tolerance instead of rms tolerance"),
  sl::argument_record("--tolerance",
                      "float",
                      new sl::generic_extractor<float>,
                      &arg_eps,
                      "Desired error in meters"),
  sl::argument_record("--u0",
                      "float",
                      new sl::generic_extractor<float>,
                      &arg_u0,
                      "parametric u origin"),
  sl::argument_record("--v0",
                      "float",
                      new sl::generic_extractor<float>,
                      &arg_v0,
                      "parametric v origin"),
  sl::argument_record("--u1",
                      "float",
                      new sl::generic_extractor<float>,
                      &arg_u1,
                      "parametric u end"),
  sl::argument_record("--v1",
                      "float",
                      new sl::generic_extractor<float>,
                      &arg_v1,
                      "parametric v end"),
  sl::argument_record("--color-remapping-alpha",
                      "uint8_t",
                      new sl::generic_extractor<std::size_t>,
                      &arg_color_remapping_alpha,
                      "low threshold for color remapping"),
  sl::argument_record("--color-remapping-beta",
                      "uint8_t",
                      new sl::generic_extractor<std::size_t>,
                      &arg_color_remapping_beta,
                      "high threshold for color remapping"),
  sl::argument_record("--color",
                      NULL,
                      new sl::generic_extractor<void>,
                      &arg_color,
                      "build color information for terrain from input image"),
  sl::argument_record("--reuse-graph",
                      NULL,
                      new sl::generic_extractor<void>,
                      &arg_reuse_graph,
                      "reuse already built graph from another construction with same parameters"),
  sl::argument_record("--keep-graph",
                      NULL,
                      new sl::generic_extractor<void>,
                      &arg_keep_graph,
                      "build persistent graph to be reused for other constructions"),
  sl::argument_record("--planar",
                      NULL,
                      new sl::generic_extractor<void>,
                      &arg_is_planar,
                      "build planar terrain"),
  sl::argument_record("--min-sample-spacing",
                      NULL,
                      new sl::generic_extractor<float>,
                      &arg_min_sample_spacing,
                      "minimum sample spacing"),
  sl::argument_record("--cylindrical-latlon",
                      NULL,
                      new sl::generic_extractor<void>,
                      &arg_is_cylindrical_latlon,
                      "build cylindrical terrain"),
  sl::argument_record("--spherical-latlon",
                      NULL,
                      new sl::generic_extractor<void>,
                      &arg_is_spherical_latlon,
                      "build spherical terrain"),
  sl::argument_record("--spherical-planet-radius",
                      NULL,
                      new sl::generic_extractor<float>,
                      &arg_spherical_radius,
                      "planet radius for spherical datasets"),
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

static void print_error(const std::string& message) {
  std::cerr << arg_program_name.to_string() << ": " << message << std::endl;
  std::cerr << "Try `" << arg_program_name.to_string() << " --help' for more information." << std::endl;
}

//=========================================================================================================

static int build_planar_height() {
  std::cerr << "build planar height " << arg_input_file_names[0] << std::endl;
  std::string image_name = arg_input_file_names[0];
  double u0 = sl::any_cast<float>(arg_u0);
  double v0 = sl::any_cast<float>(arg_v0);
  double u1 = sl::any_cast<float>(arg_u1);
  double v1 = sl::any_cast<float>(arg_v1);                     
  std::size_t patch_dim = sl::any_cast<std::size_t>(arg_patch_dim);
  float dh = sl::any_cast<float>(arg_dh);
  float tolerance =sl::any_cast<float>(arg_eps);
  bool use_amax_error = !arg_use_amax_error.empty();
  std::string tmp_dir = arg_tmp_dir.to_string();
  std::string srs = arg_srs.to_string();
  std::string about = arg_about.to_string();
  std::string out_file_name = arg_output_filename.to_string();
  std::string pattern = arg_pattern.to_string();
  float min_sample_spacing = sl::any_cast<float>(arg_min_sample_spacing);
  bool keep_graph = !arg_keep_graph.empty();
  bool reuse_graph = !arg_reuse_graph.empty();

  std::cerr << "create sampler " << std::endl;
  
  // code which works with more directories, with same patterns
  std::vector<vic::geo::map_mosaic_sampler> sampler_array(arg_input_file_names.size());
  vic::geo::map_mosaic_sampler mosaic_sampler;
  if (vic::mpi::process_rank() != 0) {
    mosaic_sampler.set_verbose(false);
  }
  if (arg_input_file_names.size() > 1) {
    // n directories: insert each one as a separate sampler. First sampler has priority over next samplers.
    SL_TRACE_OUT(-1) << "FIXME: use same pattern for all directories" << std::endl;
    for(std::size_t i = 0; i < arg_input_file_names.size(); ++i) {
      if (vic::mpi::process_rank() != 0) {
	sampler_array[i].set_verbose(false);
      }
      std::cerr << "insert dir " << arg_input_file_names[i] << std::endl;
      sampler_array[i].insert_directory(arg_input_file_names[i], pattern);
    }    
    for(std::size_t i = 0; i < arg_input_file_names.size(); ++i) {
      std::cerr << "insert sampler " << i << std::endl;
      mosaic_sampler.insert(&(sampler_array[i]));
    }
  } else {
    // insert directly the only one directory requested
    mosaic_sampler.insert_directory(arg_input_file_names[0], pattern);
  }

  vic::geo::map_int32_sampler height_sampler(&mosaic_sampler);

  if (!height_sampler.is_empty()) {
    cbdam::aabox2d_t box;
    if (u0 == 0 && v0 == 0 && u1 ==0 && v1 ==0) {
      box = mosaic_sampler.bounding_rectangle();
    } else {
      box = cbdam::aabox2d_t(cbdam::point2d_t(u0, v0),
			     cbdam::point2d_t(u1, v1));
    }
    cbdam::planar_coordinate_transform geo_xform(box);
    cbdam::mpi_builder<cbdam::height_operator> builder;
    
    builder.arg_patch_dim() = patch_dim;
    builder.arg_tolerance() = tolerance;
    builder.arg_min_sample_spacing() = min_sample_spacing;
    builder.arg_use_amax_error() = use_amax_error;
    builder.arg_data_scale_factor() = dh;
    builder.arg_tmp_dir() = tmp_dir;
    builder.arg_reuse_graph() = reuse_graph;
    builder.arg_keep_graph() = keep_graph;
    builder.arg_srs() = srs;
    builder.arg_about() = about;

    builder.build(out_file_name,
                  &height_sampler,
                  &geo_xform);
    if (builder.last_build_successful()) {
      std::cerr << "Build ok." << std::endl;
      return 0;
    } else {
      std::cerr << "Build error." << std::endl;
      return -1;
    }      
  } else {
    std::cerr << "Unable to build sampler for input: " << image_name << std::endl;
    return -2;
  }  
}

static int build_cylindrical_height() {
  std::cerr << "build cylindrical height " << arg_input_file_names[0] << std::endl;
  std::string image_name = arg_input_file_names[0];
  std::size_t patch_dim = sl::any_cast<std::size_t>(arg_patch_dim);
  float dh = sl::any_cast<float>(arg_dh);
  float tolerance =sl::any_cast<float>(arg_eps);
  bool use_amax_error = !arg_use_amax_error.empty();
  std::string tmp_dir = arg_tmp_dir.to_string();
  std::string srs = arg_srs.to_string();
  std::string about = arg_about.to_string();
  std::string out_file_name = arg_output_filename.to_string();
  std::string pattern = arg_pattern.to_string();
  float min_sample_spacing = sl::any_cast<float>(arg_min_sample_spacing);
  bool keep_graph = !arg_keep_graph.empty();
  bool reuse_graph = !arg_reuse_graph.empty();
  double radius = sl::any_cast<float>(arg_spherical_radius);
  std::cerr << "radius " << radius << std::endl;

  std::cerr << "create sampler " << std::endl;
  
  // code which works with more directories, with same patterns
  std::vector<vic::geo::map_mosaic_sampler> sampler_array(arg_input_file_names.size());
  vic::geo::map_mosaic_sampler mosaic_sampler;
  if (vic::mpi::process_rank() != 0) {
    mosaic_sampler.set_verbose(false);
  }
  if (arg_input_file_names.size() > 1) {
    // n directories: insert each one as a separate sampler. First sampler has priority over next samplers.
    SL_TRACE_OUT(-1) << "FIXME: use same pattern for all directories" << std::endl;
    for(std::size_t i = 0; i < arg_input_file_names.size(); ++i) {
      if (vic::mpi::process_rank() != 0) {
	sampler_array[i].set_verbose(false);
      }
      std::cerr << "insert dir " << arg_input_file_names[i] << std::endl;
      sampler_array[i].insert_directory(arg_input_file_names[i], pattern);
    }    
    for(std::size_t i = 0; i < arg_input_file_names.size(); ++i) {
      std::cerr << "insert sampler " << i << std::endl;
      mosaic_sampler.insert(&(sampler_array[i]));
    }
  } else {
    // insert directly the only one directory requested
    mosaic_sampler.insert_directory(arg_input_file_names[0], pattern);
  }

  vic::geo::map_int32_sampler height_sampler(&mosaic_sampler);

  if (!height_sampler.is_empty()) {
    cbdam::cylindrical_coordinate_transform geo_xform(radius);
    cbdam::mpi_builder<cbdam::height_operator> builder;
    
    builder.arg_patch_dim() = patch_dim;
    builder.arg_tolerance() = tolerance;
    builder.arg_min_sample_spacing() = min_sample_spacing;
    builder.arg_use_amax_error() = use_amax_error;
    builder.arg_data_scale_factor() = dh;
    builder.arg_tmp_dir() = tmp_dir;
    builder.arg_reuse_graph() = reuse_graph;
    builder.arg_keep_graph() = keep_graph;
    builder.arg_srs() = srs;
    builder.arg_about() = about;

    builder.build(out_file_name,
                  &height_sampler,
                  &geo_xform);
    if (builder.last_build_successful()) {
      std::cerr << "Build ok." << std::endl;
      return 0;
    } else {
      std::cerr << "Build error." << std::endl;
      return -1;
    }      
  } else {
    std::cerr << "Unable to build sampler for input: " << image_name << std::endl;
    return -2;
  }  
}

static int build_planar_color() {
  std::string image_name = arg_input_file_names[0];
  double u0 = sl::any_cast<float>(arg_u0);
  double v0 = sl::any_cast<float>(arg_v0);
  double u1 = sl::any_cast<float>(arg_u1);
  double v1 = sl::any_cast<float>(arg_v1);                     
  std::size_t color_remapping_alpha = sl::any_cast<std::size_t>(arg_color_remapping_alpha);
  std::size_t color_remapping_beta = sl::any_cast<std::size_t>(arg_color_remapping_beta);
  std::size_t patch_dim = sl::any_cast<std::size_t>(arg_patch_dim);
  float tolerance =sl::any_cast<float>(arg_eps);
  bool use_amax_error = !arg_use_amax_error.empty();
  float min_sample_spacing = sl::any_cast<float>(arg_min_sample_spacing);
  bool keep_graph = !arg_keep_graph.empty();
  bool reuse_graph = !arg_reuse_graph.empty();

  std::string tmp_dir = arg_tmp_dir.to_string();
  std::string srs = arg_srs.to_string();
  std::string about = arg_about.to_string();
  std::string out_file_name = arg_output_filename.to_string();
  std::string pattern = arg_pattern.to_string();

  vic::geo::map_mosaic_sampler mosaic_sampler;
  if (vic::mpi::process_rank() != 0) {
    mosaic_sampler.set_verbose(false);
  }
  mosaic_sampler.insert_directory(image_name, pattern);

  vic::geo::map_rgb_int16_8_sampler rgb_sampler(&mosaic_sampler);
  rgb_sampler.set_nodata_value(vic::geo::map_rgb_int16_8_sampler::value_t(0, 0, 0));
  rgb_sampler.set_remap_lo_hi(color_remapping_alpha, color_remapping_beta);
  if (!rgb_sampler.is_empty()) {
    cbdam::aabox2d_t box;
    if (u0 == 0 && v0 == 0 && u1 ==0 && v1 ==0) {
      box = mosaic_sampler.bounding_rectangle();
    } else {
      box = cbdam::aabox2d_t(cbdam::point2d_t(u0, v0),
			     cbdam::point2d_t(u1, v1));
    }
    cbdam::planar_coordinate_transform geo_xform(box);

    cbdam::mpi_builder<cbdam::color_operator> builder;
    builder.arg_patch_dim() = patch_dim;
    builder.arg_tolerance() = tolerance;
    builder.arg_min_sample_spacing() = min_sample_spacing;
    builder.arg_use_amax_error() = use_amax_error;
    builder.arg_data_scale_factor() = 1.0;
    builder.arg_tmp_dir() = tmp_dir;
    builder.arg_reuse_graph() = reuse_graph;
    builder.arg_keep_graph() = keep_graph;
    builder.arg_srs() = srs;
    builder.arg_about() = about;
   
    builder.build(out_file_name,
                  &rgb_sampler,
                  &geo_xform);
    if (builder.last_build_successful()) {
      std::cerr << "Build ok." << std::endl;
      return 0;
    } else {
      std::cerr << "Build error." << std::endl;
      return -1;
    }      
  } else {
    std::cerr << "Unable to build sampler for input: " << image_name << std::endl;
    return -2;
  }  
}

int main(int argc, const char** argv) {
  int result  = 0;
  vic::mpi::initialize(&argc, const_cast<char***>(&argv));
    
  arg_program_name = std::string(argv[0]);
  arg_parser.accept_extra_arguments(true);
  arg_parser.reset_defaults();

  // Parse arguments
  arg_parser.parse(argc, (const char**)argv);
  if (!arg_help.empty()) {
    print_usage();
    result = 0;
  }
  if (!arg_parser.last_operation_success()) {
    print_error(arg_parser.last_error_string());
    result = 1;
  } else {
    for (std::list<std::string>::const_iterator it = arg_parser.last_extra_arguments().begin();
         it != arg_parser.last_extra_arguments().end();
         ++it) {
      std::string fname = *it;
      arg_input_file_names.push_back(fname);
    }
    
    if (arg_input_file_names.size() < 1) {
      print_error(std::string()+"Missing input file name");
      result = 2;
    } else {

      std::size_t projection_types = 0;
      if (!arg_is_planar.empty()) ++projection_types;
      if (!arg_is_cylindrical_latlon.empty()) ++projection_types;
      if (!arg_is_spherical_latlon.empty()) ++projection_types;
      if (projection_types != 1) {
        print_error(std::string()+"Choose one and only one input data representation!");
        result = 3;
      } else if (!arg_is_spherical_latlon.empty()) {
        // SPHERICAL
        print_error("spherical - not implemented for mpi main...");
        result = 4;
      } else if (!arg_is_cylindrical_latlon.empty()) {
	// CYLINDRICAL
	if (arg_color.empty()) {
	  result = build_cylindrical_height();
	} else {
	  print_error("cylindrical color - not implemented for mpi main...");
	  result = 5;
	}
      } else {
        // PLANAR
        if (arg_color.empty()) {
          result = build_planar_height();
        } else {
          result = build_planar_color();
        }
      }
    }
  }

  if (result) {
    std::cerr << "Exiting with error " << result << std::endl;
  } else {
    std::cerr << "Exiting" << std::endl;
  }
  vic::mpi::finalize();
  return result;
}

