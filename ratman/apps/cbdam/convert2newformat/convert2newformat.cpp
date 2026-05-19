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
// HACK to avoid ambiguous overload with c++ 4.1

#include <vector>
#include <utility>
#include <algorithm>
#define swap sl_swap
#include <sl/utility.hpp>
#include <sl/fastest.hpp>
#undef swap

#include <vic/cbdam/base/diamond_repository_storage.hpp>
#include <vic/cbdam/base/diamond_graph.hpp>
#include <vic/cbdam/base/value_operator.hpp>
#include "diamond_repository_storage_old.hpp"

#include <vic/vfs/repository.hpp>


template<class IN_DIAMOND_REPOSITORY_T, class OUT_DIAMOND_REPOSITORY_T>
void convert_to_new_format(IN_DIAMOND_REPOSITORY_T& input_repo, 
			   OUT_DIAMOND_REPOSITORY_T& output_repo) {
  std::cerr << "copy parameters " << std::endl;
  // copy params
  output_repo.set_patch_dim(input_repo.patch_dim());
  output_repo.set_mono_scale(input_repo.is_mono_scale());
  output_repo.set_wavelet_alpha(input_repo.wavelet_alpha());
  if (input_repo.is_planar()) {
    output_repo.set_planar_with_side_length(input_repo.planar_terrain_root_side_length());
  } else {
    output_repo.set_spherical_with_radius(input_repo.spherical_terrain_radius());
  }

  // copy roots
  std::cerr << "copy roots" << std::endl;
  std::vector<cbdam::uint8_t> buffer_p;
  std::vector<cbdam::uint8_t> buffer_q;

  const typename IN_DIAMOND_REPOSITORY_T::map_coords_t& map_id_roots = input_repo.map_id_roots();
  typename IN_DIAMOND_REPOSITORY_T::map_coords_t::const_iterator it;
  cbdam::uint32_t count = 0;
  for(it = map_id_roots.begin(); it != map_id_roots.end(); ++it) {
    input_repo.get_root_buffers(it, buffer_p);
    output_repo.set_root_data(it->first, buffer_p);
    ++count;
  }
  std::cerr << "converted " << count << " roots" << std::endl;

  // copy data
  std::cerr << "copy data" << std::endl;
  const typename IN_DIAMOND_REPOSITORY_T::map_coords_t& map_id_data = input_repo.map_id_data();
  std::vector<std::pair<cbdam::uint64_t, cbdam::grid_point_t> > offset_key_pairs;
  cbdam::uint32_t size = map_id_data.size();
  count = 0;
  for(it = map_id_data.begin();
      it != map_id_data.end();
      ++it) {
    offset_key_pairs.push_back(std::make_pair(it->second.first_index(), it->first));
  }
  std::cerr << "sorting ids.." << std::endl;
  std::sort(offset_key_pairs.begin(), offset_key_pairs.end());

  for(std::size_t i = 0; i < size; ++i) {
    if (i < 10) {
      std::cerr << "reading " << offset_key_pairs[i].first << std::endl;
    }

    input_repo.get_buffers(offset_key_pairs[i].second, buffer_p, buffer_q);
    if (buffer_p.size() + buffer_q.size() > 0) {
      output_repo.set_data(offset_key_pairs[i].second, buffer_p, buffer_q);

      // check data
      std::vector<cbdam::uint8_t> check_buffer_p;
      std::vector<cbdam::uint8_t> check_buffer_q;
      output_repo.get_data_buffers(offset_key_pairs[i].second, check_buffer_p, check_buffer_q);
      if (check_buffer_p.size() != buffer_p.size() ||
	  check_buffer_q.size() != buffer_q.size()) {
	std::cerr << std::endl << "Different check sizes for " << i << " element, p:"
		  <<  buffer_p.size() << ", check_p " << check_buffer_p.size() << ", q:" 
		  <<  buffer_q.size() << ", check_q " << check_buffer_q.size() << std::endl;
	break;
      }

      ++count;
      if ((count < 1000 && count%100 == 0) ||
          (count % 1000 == 0)) {
        std::cerr.precision(2);
        std::cerr << "converted " << count << "/" << size << "  " << (float)(count) * 100.0f / (float)size << "% " << "\r";
      }
    } else {
      std::cerr << std::endl << offset_key_pairs[i].first << " null size diamond" << std::endl;
    }
  }

  std::cerr << std::endl << "close file" << std::endl;
  output_repo.close();
  //  input_repo.close();
  std::cerr << "done" << std::endl;
}

void print_usage(const char *filename) {
  std::cerr << filename << "  input-file output-file file-type(color|height)" << std::endl;
}

void check_bad_diamond(const char* filename) {
  typedef cbdam::diamond_repository_storage<cbdam::height_operator>::array2_delta_t array_delta_t;
  array_delta_t p;
  array_delta_t q;
  cbdam::diamond_repository_storage<cbdam::height_operator> drs;
  drs.open_read(filename);
  drs.get_data(cbdam::grid_point_t(-80740352, -92274688, 268435456), p, q);
  std::cerr << "p extent " << p.extent()[0] << ", " << p.extent()[1] << ", " 
	    << "q extent " << q.extent()[0] << ", " << q.extent()[1] << ", " << std::endl;
  drs.close();

  std::cerr << "check with virtual_file_system_network" << std::endl;
  vic::vfs::repository repo;
  std::string repo_filename = std::string(filename) + ".data";
  repo.open_read(repo_filename);
  cbdam::uint32_t size;
  const cbdam::uint8_t* data = repo.get_data(vic::vfs::repository::key_t(-80740352, -92274688, 268435456), size);
  std::cerr << "buffer size " << size << std::endl;
  std::cerr << "first patch size " << cbdam::byte_array_accessor::first_patch_size(data) << std::endl;
  std::cerr << "second patch size " << cbdam::byte_array_accessor::second_patch_size(data, size) << std::endl;
  repo.close();
}

int main(int argc, const char**argv) {
  if (argc < 4) {
    print_usage(argv[0]);
    return 0;
  }

  std::cerr << "input file  " << argv[1] << std::endl;
  std::cerr << "output file " << argv[2] << std::endl;
  std::cerr << "OUTPUT FILE is gonna be rewritten, are you sure (yes/no) ? ";
  char c[10];
  std::cin >> c;
  if (strcmp(c, "yes") == 0) {
    if (strcmp(argv[3], "height") == 0) {
      cbdam::diamond_repository_storage_old<cbdam::height_operator> input_repo;
      cbdam::diamond_repository_storage<cbdam::height_operator> output_repo;
      input_repo.open_read(argv[1]);
      if (!input_repo.last_operation_success()) {
	std::cerr << "unable to open input " << argv[1] << std::endl;
	return 0;
      }
      const std::size_t expected_height_average_data_size = 1024;
      output_repo.open_write(argv[2], expected_height_average_data_size);
      if (!output_repo.is_open()) {
	std::cerr << "unable to open output " << argv[2] << std::endl;
	return 0;
      }
      convert_to_new_format(input_repo, output_repo);
    } else  if (strcmp(argv[3], "color") == 0) {
      cbdam::diamond_repository_storage_old<cbdam::color_operator> input_repo;
      cbdam::diamond_repository_storage<cbdam::color_operator> output_repo;
      input_repo.open_read(argv[1]);
      if (!input_repo.last_operation_success()) {
	std::cerr << "unable to open input " << argv[1] << std::endl;
	return 0;
      }
      const std::size_t expected_color_average_data_size = 8192;
      output_repo.open_write(argv[2], expected_color_average_data_size);
      if (!output_repo.is_open()) {
	std::cerr << "unable to open output " << argv[2] << std::endl;
	return 0;
      }
      convert_to_new_format(input_repo, output_repo);
    } 
  }
  return 0;
}
