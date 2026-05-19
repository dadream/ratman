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
#include "repository_parameters_old.hpp"
#include <vic/cbdam/base/repository_parameters.hpp>
#include <vic/vfs/repository.hpp>
#include <vic/vfs/db_repository.hpp>

typedef vic::vfs::repository	in_repository_t;
typedef vic::vfs::db_repository	out_repository_t;

bool convert_parameters(const char* input, const char* output) {
  cbdam::repository_parameters_old in_rp;
  cbdam::repository_parameters     out_rp;
  FILE* fp = fopen(input, "rb");
  if (!fp) {
    std::cerr << "unable to open " << input << " for reading params" << std::endl;
    return false;
  } else {
    in_rp.read_from_file(fp, true);
  }
  fclose(fp);

  out_rp.patch_dim() = in_rp.patch_dim();
  out_rp.is_planar() = in_rp.is_planar();
  out_rp.is_mono_scale() = in_rp.is_mono_scale();
  out_rp.wavelet_alpha() = in_rp.wavelet_alpha();
  out_rp.length() = in_rp.length();
  for(std::size_t i = 0; i < in_rp.roots().size(); ++i) {
    const cbdam::grid_point_t& r = in_rp.roots()[i];
    out_rp.add_root(r);
  }

  out_rp.write_to_file(output, true);

  if (!out_rp.last_operation_success()) {
    std::cerr << "unable to write parameters correctly" << std::endl;
    return false;
  } else {
    return true;
  }
}

void convert2db(const char* input, const char* output) {
  std::string in_data_name  = std::string(input) + ".data";
  std::string in_root_name  = std::string(input) + ".root";
  std::string in_param_name = std::string(input) + ".param";

  std::string out_data_name  = std::string(output) + ".data";
  std::string out_root_name  = std::string(output) + ".root";
  std::string out_param_name = std::string(output) + ".xml";

  // copy parameters
  std::cerr << "== Converting parameters..." << std::endl;
  if (!convert_parameters(in_param_name.c_str(), out_param_name.c_str())) {
    std::cerr << "--> Error!" << std::endl;
    return;
  }
  std::cerr << "== Done." << std::endl;

  // copy root
  std::cerr << "== Converting roots..." << std::endl;
  in_repository_t in_repo_root;   in_repo_root.open_read(in_root_name.c_str());
  if (!in_repo_root.is_open()) {
    std::cerr << "--> Error: unable to open " << in_root_name << " for reading" << std::endl;
    return;
  }
  const in_repository_t::key_data_handle_sorted_vector_t& kdm_root = in_repo_root.key_data_vector();
  std::size_t average_root_size = 1+in_repo_root.size()/in_repo_root.number_of_elements();
  out_repository_t out_repo_root; out_repo_root.open_write(out_root_name.c_str(), average_root_size);
  if (!out_repo_root.is_open()) {
    std::cerr << "--> Error: unable to open " << out_root_name << " for writing" << std::endl;
    in_repo_root.close();
    return;
  }

  in_repository_t::key_data_handle_sorted_vector_t::const_iterator it;
  std::size_t kdm_size = kdm_root.size();
  std::size_t count = 0;
  for(it = kdm_root.begin(); it != kdm_root.end(); ++it) {
    cbdam::uint32_t size;
    const cbdam::uint8_t* data = in_repo_root.get_data(it->first, size);
    out_repo_root.set_data(it->first, data, size);
    ++count;
    if ((count == kdm_size) ||
	(count < 1000 && count%100 == 0) ||
	(count % 1000 == 0)) {
      std::cerr.precision(2);
      std::cerr << "   root: converted " << count << "/" << kdm_size << "  " << (float)(count) * 100.0f / (float)kdm_size << "% " << "      \r";
    }
  }
  std::cerr << std::endl;
  in_repo_root.close();
  out_repo_root.close();
  std::cerr << "== Done." << std::endl;

  // copy data
  std::cerr << "== Converting data..." << std::endl;
  in_repository_t in_repo_data;   in_repo_data.open_read(in_data_name.c_str());

  if (!in_repo_data.is_open()) {
    std::cerr << "--> Error: unable to open " << in_data_name << " for reading" << std::endl;
    return;
  }
  const in_repository_t::key_data_handle_sorted_vector_t& kdm_data = in_repo_data.key_data_vector();
  kdm_size = kdm_data.size();

  // estimate average data size
  std::size_t average_data_size = 1+in_repo_data.size()/in_repo_data.number_of_elements();

  out_repository_t out_repo_data; out_repo_data.open_write(out_data_name.c_str(), average_data_size);
  if (!out_repo_data.is_open()) {
    std::cerr << "--> Error: unable to open " << out_data_name << " for writing" << std::endl;
    in_repo_data.close();
    return;
  }

  count = 0;
  for(it = kdm_data.begin(); it != kdm_data.end(); ++it) {
    cbdam::uint32_t size;
    const cbdam::uint8_t* data = in_repo_data.get_data(it->first, size);
    out_repo_data.set_data(it->first, data, size);
    ++count;
    if ((count == kdm_size) ||
	(count < 1000 && count%100 == 0) ||
	(count % 1000 == 0)) {
      std::cerr.precision(2);
      std::cerr << "   data: converted " << count << "/" << kdm_size << "  " << (float)(count) * 100.0f / (float)kdm_size << "% " << "      \r";
    }
  }
  std::cerr << std::endl;
  in_repo_data.close();
  out_repo_data.close();
  std::cerr << "== Done." << std::endl;

  std::cerr << "done" << std::endl;
}

int main(int argc, const char** argv) {
  if (argc < 2 || argc > 4) {
    std::cerr << "usage prog input-file output-file [only-param]" << std::endl;
    return 0;
  }

  std::cerr << argv[2] << " is gonna be rewritten, do you want to continue ? (yes/no)" << std::endl;
  char c[256];
  std::cin >> c;
  if (strcmp(c, "yes") != 0) {
    std::cerr << "bye" << std::endl;   
    return 0;
  }

  if (argc == 3) {
    convert2db(argv[1], argv[2]);
  } else if (argc == 4 && strcmp(argv[3], "only-param") == 0) {
    std::string input = std::string(argv[1]) + ".param";
    std::string output = std::string(argv[2]) + ".xml";
    convert_parameters(input.c_str(), output.c_str());
  } else {
    std::cerr << "usage prog input-file output-file [only-param]" << std::endl;
  }
  return 0;
}
