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
#include <vic/cbdam/base/raw_image.hpp>
#include <sl/external_array.hpp>
#include <sl/argument_parser.hpp>
#include <sl/clock.hpp>


using namespace cbdam;

typedef sl::external_array1<cbdam::int16_t>    int16_xarray_t;

// ======================================================================
// Option declararation
static std::vector<std::string> input_file_names;

static sl::any arg_program_name(std::string("dem2raw_image"));
static sl::any arg_output_filename(std::string("terrain.raw")); 
static sl::any arg_width(std::size_t(0));
static sl::any arg_height(std::size_t(0));
static sl::any arg_from_row(std::size_t(0));
static sl::any arg_to_row(std::size_t(0));
static sl::any arg_from_column(std::size_t(0));
static sl::any arg_paste_image(false);
static sl::any arg_new_image(false);
static sl::any arg_msb_integer(false);
static sl::any arg_fill_image_rows(false);
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
  sl::argument_record("--width",
                      "int",
                      new sl::generic_extractor<std::size_t>,
                      &arg_width,
                      "desired new image width"),
  sl::argument_record("--height",
                      "int",
                      new sl::generic_extractor<std::size_t>,
                      &arg_height,
                      "desired new image height"),
  sl::argument_record("--from-row",
                      "int",
                      new sl::generic_extractor<std::size_t>,
                      &arg_from_row,
                      "process output image starting from row (used for paste-image and fill)"),
  sl::argument_record("--to-row",
                      "int",
                      new sl::generic_extractor<std::size_t>,
                      &arg_to_row,
                      "process output image till row (used for fill)"),
  sl::argument_record("--from-column",
                      "int",
                      new sl::generic_extractor<std::size_t>,
                      &arg_from_column,
                      "process output image starting from column (used for paste-image)"),
  sl::argument_record("--new-image",
                      NULL,
                      new sl::generic_extractor<void>,
                      &arg_new_image,
                      "create new image (specify dimensions with: --width --height options"),
  sl::argument_record("--paste-image",
                      NULL,
                      new sl::generic_extractor<void>,
                      &arg_paste_image,
                      "paste image to desired row column of already created output-image"),
  sl::argument_record("--msb-integer",
                      NULL,
                      new sl::generic_extractor<void>,
                      &arg_msb_integer,
                      "msb integer requires swap of input bytes"),
  sl::argument_record("--fill-image-rows",
                      NULL,
                      new sl::generic_extractor<void>,
                      &arg_fill_image_rows,
                      "fill image rows (specify rows with: --from-row --to_row"),
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

static int16_xarray_t* open_dem_raw(const std::string& file_name, std::size_t width, std::size_t height) {
  std::cerr << "load dem " << file_name << std::endl;

  int16_xarray_t* xa = new int16_xarray_t(file_name, "r");
  if (xa->is_open()) {
#if 0
    if (xa->size() != width * height) {
      std::cerr << "wrong dimensions : " << height << " x " << width << " = " << height * width << " != " << xa->size() << std::endl;
      delete xa;
      xa = 0;
    }
#endif
  } else {
    std::cerr << "unable to open " << file_name << std::endl;
    delete xa;
    xa = 0;
  }
  return xa;
}

static void build_new_image(const std::string& file_name, std::size_t height, std::size_t width) {
  std::cerr << "==================================================" << std::endl;
  std::cerr << "DEM 2 RAW IMAGE - Create new image " << height << " x " << width << std::endl;
  std::cerr << "==================================================" << std::endl;
  std::cerr << "open to write " << file_name << std::endl;

  sl::real_time_clock clock;
  clock.restart();
  raw_image ri;
  ri.open_to_write(height, width, file_name);

  std::cerr << "done\n";
  std::cerr << "elapsed " << clock.elapsed().as_seconds() << " seconds\n";
}

static void paste_image(const std::string& input_file,
                        const std::string& output_file,
                        std::size_t height,
                        std::size_t width,
                        std::size_t from_row,
                        std::size_t from_col,
                        bool swap) {
  std::cerr << "==================================================" << std::endl;
  std::cerr << "DEM 2 RAW IMAGE - Paste image " << height << " x " << width << std::endl;
  std::cerr << "==================================================" << std::endl;
  sl::real_time_clock clock;
  clock.restart();

  std::cerr << "w,h = " << width << ", " << height << std::endl;
  int16_xarray_t* xa = open_dem_raw(input_file, width, height);

  // paste into raw imge
  if (xa != 0) {
    raw_image ri;
    ri.open_to_read(output_file);
    if (ri.is_open()) {
      std::cerr << "paste from row-col " << from_row << ", " << from_col << std::endl;
      for(std::size_t y = 0; y < height; ++y) {
        for(std::size_t x = 0; x < width; ++x) {
          cbdam::int16_t h = (*xa)[y * width + x];
          if (swap) {
            std::swap(((char*)&h)[0], ((char*)&h)[1]);
          }
          ri.set_value_at(y + from_row, x + from_col, h);
        }
      }
      std::cerr << "done\n";
      std::cerr << "elapsed " << clock.elapsed().as_seconds() / 60.0f << " minutes\n";
    } else {
      std::cerr << "unable to open raw image file " << output_file << std::endl;
    }
  }
}

static void fill_image_rows(const std::string& file_name,
                            std::size_t from_row,
                            std::size_t to_row) {
  std::cerr << "==================================================" << std::endl;
  std::cerr << "DEM 2 RAW IMAGE - Fill rows " << 
  std::cerr << "==================================================" << std::endl;
  std::cerr << "fill from row - to row " << from_row << ", " << to_row << " (both included)" << std::endl;
  sl::real_time_clock clock;
  clock.restart();
  
  raw_image ri;
  ri.open_to_read(file_name);
  if (ri.is_open()) {
    // select row from which get the fill value
    std::size_t selected_row = 0;
    if (from_row == 0) {
      selected_row = to_row + 1;
    } else {
      selected_row = from_row - 1;
    }

    // get mean value of selected row
    std::size_t width = ri.width();
    std::vector<int>    selected_row_values(width);
    float mean_value = 0;
    for(std::size_t x = 0; x < width; ++x) {
      int ri_x = ri(selected_row, x);
      selected_row_values[x] = ri_x;
      mean_value += ri_x;
    }
    mean_value /= width;
    std::cerr << "mean value for filling from row " << selected_row << " is " << mean_value << std::endl;

    // set mean value
    for(std::size_t y = from_row; y <= to_row; ++y) {
      float avg_y = 0.0f;
      float t = (std::fabs(float(y)-float(selected_row))-1.0f)/(to_row-from_row);
      for(std::size_t x = 0; x < width; ++x) {
        int v0 = selected_row_values[x];
        int v1 = mean_value;
        int v = sl::median(-32767, 32767, int(v0 + t * (v1-v0)));
	avg_y += v;
        ri.set_value_at(y, x, v);
      }
      avg_y /= width;
    }

    std::cerr << "done\n";
    std::cerr << "elapsed " << clock.elapsed().as_seconds() / 60.0f << " minutes\n";
  } else {
    std::cerr << "unable to open raw image file " << file_name << std::endl;    
  }
}

int main(int argc, const char** argv) {
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
    if (sl::matches(fname, "*.img") || sl::matches(fname, "*.hgt")) {
      input_file_names.push_back(fname);
    } else {
      print_error_and_exit(std::string()+"Unknown file type: "+fname);
    }
  }

  if (!arg_new_image.empty()) {
    build_new_image(arg_output_filename.to_string(),
                    sl::any_cast<std::size_t>(arg_height),
                    sl::any_cast<std::size_t>(arg_width));
  } else if (!arg_paste_image.empty()) {
    if (input_file_names.size() < 1) {
      print_error_and_exit(std::string()+"Missing input file name");
    }
    paste_image(input_file_names[0],
                arg_output_filename.to_string(),
                sl::any_cast<std::size_t>(arg_height),
                sl::any_cast<std::size_t>(arg_width),
                sl::any_cast<std::size_t>(arg_from_row),
                sl::any_cast<std::size_t>(arg_from_column),
                !arg_msb_integer.empty());
  } else if (!arg_fill_image_rows.empty()) {
    fill_image_rows(arg_output_filename.to_string(),
                    sl::any_cast<std::size_t>(arg_from_row),
                    sl::any_cast<std::size_t>(arg_to_row));
  } else {
    print_error_and_exit(std::string()+"Choose one option between --new-image --paste-image --fill-image-rows\n");
  }
  
  return 0;
}
