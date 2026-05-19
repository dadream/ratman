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
#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <utility> // std::pair
#include <vic/geo/base/victms_conventions.hpp>

//geo::base::victms_conventions::quad_filename(root_dir(), l, x, y, file_extension())
typedef std::pair<int, int> xy_t;

static bool ok_file(const std::string& root_dir, int l, int x, int y) {
  std::string fname=vic::geo::base::victms_conventions::quad_filename(root_dir, l, x, y);
  std::ifstream fin(fname.c_str());
  bool result=fin;
  fin.close();
  return result;
}

static void init_root(const std::string& root_dir, std::vector<xy_t>& roots) {
  // FIXME read tilemap_config!!! 
  int nx = 2;
  int ny = 1;
  for (int y=0; y < ny; ++y) {
    for (int x=0; x < nx; ++x) {
      if (ok_file(root_dir, 0, x, y)) {
	roots.push_back(xy_t(x, y));
      }
    }
  }
}

static void generate_children(const std::string& root_dir, 
			      std::size_t        parent_level,
			      std::vector<xy_t>& parents, 
			      std::vector<xy_t>& children) {
  int p_l = std::size_t(parent_level);
  int c_l = p_l + 1;
  for (int t=0; t<int(parents.size()); ++t) {
    int p_x = parents[t].first;
    int p_y = parents[t].second;
    
    for (int j=0; j<2; ++j) {
      for (int i=0; i<2; ++i) {
	int c_x = p_x*2+i;
	int c_y = p_y*2+j;
	if (ok_file(root_dir, c_l, c_x, c_y)) {
	  //std::cout << "FOUND: " << c_l << " " << c_x << " " << c_y << std::endl;
	  children.push_back(xy_t(c_x, c_y));
	} else {
	  //std::cout << "NOT FOUND: " << c_x << " " << c_x << " " << c_y << std::endl;
	}	
      }
    }
  }
}

static void print_quads(const std::string& root_dir,
			std::size_t level,
			const std::vector<xy_t>& quads) {
  for (int t=0; t<int(quads.size()); ++t) {
    std::string fname=vic::geo::base::victms_conventions::quad_filename(root_dir, 
									level, 
									quads[t].first, 
									quads[t].second);
 #if 0
    std::cout << level << '\t'
	      << quads[t].first << '\t'
	      << quads[t].second << '\t'
	      << std::endl;
#else
    std::cout << fname << std::endl;
#endif
  }
}

int main(int argc, char *argv[]) {
  if (argc != 2) {
    std::cerr << "Usage: " << std::endl;
    std::cerr << "   " << basename(argv[0])
	      << " <root_dir>" 
	      << std::endl;
    exit(1);
  }
  std::string root_dir=std::string(argv[1]);
  
  std::vector<xy_t> parents;
  std::vector<xy_t> children;
 
  init_root(root_dir, parents);
  int l = 0;
  std::size_t total_quad_count = 0;
  while (!parents.empty()) {
    total_quad_count += parents.size(); 
    std::cerr << "[" << std::setw(4) << l << "] " << std::setw(8) << parents.size() << " quads" << std::endl;
    print_quads(root_dir, l, parents);
    generate_children(root_dir, l, parents, children);
    std::swap(parents,children);
    children.clear();
    ++l;
  }
  std::cerr << " DONE: TOTAL QUAD COUNT = " << total_quad_count << std::endl;
}
