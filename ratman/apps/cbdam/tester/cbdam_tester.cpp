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
#include <vic/cbdam/base/delta_height_codec.hpp>
#include <vic/cbdam/base/diamond_repository_procedural.hpp>
#include <vic/cbdam/base/diamond_graph.hpp>
#include <sl/linear_map_factory.hpp>
#include <sl/clock.hpp>

using namespace cbdam;

diamond::patch_vertices_t build_patch_vertices(int patch_dim, float value) {
  diamond::patch_vertices_t points;
  int N = (patch_dim+1) * (patch_dim+2)/2;
  for(int i = 0; i < N; ++i) {
    points.push_back(point3_t(value,0,0));
  }
  return points;
}

void test_codec(int patch_dim) {
  diamond d(grid_canonical_point(0), grid_canonical_point(1),
            grid_canonical_point(2), grid_canonical_point(3));
  d.set_patch(0, build_patch_vertices(patch_dim, 1));
  d.set_patch(1, build_patch_vertices(patch_dim, 2));

  std::vector<float> offset_new;
  std::vector<float> offset_control;

#if 1
  int N = (patch_dim-1)*(patch_dim-1);
  for(int i = 0; i < N; ++i) {
    offset_control.push_back(0.5);
  }
  N = patch_dim*patch_dim;
  for(int i = 0; i < N; ++i) {
    offset_new.push_back(0.25);
  }
#else
  offset_control.resize((patch_dim-1)*(patch_dim-1));
  offset_new.resize(patch_dim*patch_dim);
  diamond_repository_procedural drp;
  drp.init(patch_dim);
  drp.get_offsets(d.id(), 10, offset_control, offset_new);
#endif
  
  delta_height_codec dhc;
  dhc.init(patch_dim);

  std::cerr << "decode\n";
  dhc.decode_heights(offset_control, offset_new, &d);
  std::cerr << "decoded\n";
  diamond* d00 = d.create_child(0,0, invalid_grid_point());
  diamond* d10 = d.create_child(1,0, invalid_grid_point());
  diamond* d01 = d.create_child(0,1, invalid_grid_point());
  diamond* d11 = d.create_child(1,1, invalid_grid_point());
  
  dhc.distribute_data_to_children(d.id(), d00, d01, d10, d11);
  for(int i = 0; i < 4; ++i) {
    diamond* d_ptr = 0;
    switch(i) {
    case 0:
      std::cerr << "patch 00 : west\n";
      d_ptr = d00;
      break;
    case 1:
      std::cerr << "patch 01 : north\n";
      d_ptr = d01;
      break;
    case 2:
      std::cerr << "patch 10 : east\n";
      d_ptr = d10;
      break;
    case 3:
      std::cerr << "patch 11 : south\n";
      d_ptr = d11;
      break;
    }
    if (d_ptr) {
      diamond::patch_vertices_t p0 = d_ptr->patch_vertices(d_ptr->patch_id_deriving_from_parent(d.id()));
      int count = 0;
      for(int y = 0; y <= patch_dim; ++y) {
        for(int x = 0; x <= patch_dim - y; ++x) {
          std::cerr << p0[count++][0] << " ";
        }
        std::cerr << std::endl;
      }
    }
  }
}

static std::vector<diamond_graph::diamond_id_t> cut_parents;

//void coarse(diamond_graph& dg, const diamond_graph::diamond_id_t& id) {}

void refine(diamond_graph& dg, const diamond_graph::diamond_id_t& id, int level, int max_level) {
  if (id == invalid_grid_point()) {
    return;
  }
  dg.refine(id);
  if (level == max_level) {
    return;
  } else {

    if (level == max_level - 1) {
      cut_parents.push_back(id);
    }
    
    for(int i = 0; i < 2; ++i) {
      for(int j = 0; j < 2; ++j) {
        const diamond* d = dg.get_diamond(id);
        if (d == 0) {
          SL_TRACE_OUT(-1) << "accessing missing dm " << d << std::endl;
        } else {
          //          std::cerr << "ref " << i << ", " << j <<", " << level+1 << std::endl;
          refine(dg, d->child_id(i,j), level+1, max_level);
        }
      }
    }
  }
}

void test_graph(int patch_dim) {
  diamond_graph dg;
  dg.set_parameters(patch_dim, 6369000);
  dg.build_canonical_root(0);
  diamond_graph::diamond_id_t root = dg.get_canonical_root_id(0);
  std::cerr << "refine root :" << root[0] << "," << root[1] << ", " << root[2] << std::endl;
  refine(dg, root, 0, 2);
  std::cerr << "refined\ncoarsen";
  for(int i = 0; i < (int)cut_parents.size(); ++i) {
    dg.coarsen(cut_parents[i]);
  }
  std::cerr << "coarsened\n";
}

void test_rendering_traversal(int patch_dim) {
  diamond_graph dg;
  dg.clear_graph();
  dg.set_parameters(patch_dim, 6369000);
  dg.build_canonical_root(0);
  dg.init_heaps();
  std::cerr << dg.patch_edge_length(dg.get_canonical_root_id(0))/patch_dim << std::endl;
  const diamond* d = dg.get_diamond(dg.get_canonical_root_id(0));
  const diamond_graph::bounding_sphere_t& bs = d->bounding_sphere();
  float fovy = 3.14 / 3;
  float p_far =  4 * bs.radius();
  float p_near = p_far / 10000;
  diamond_graph::projection_map_t P = sl::linear_map_factory3f::perspective(fovy, p_near, p_far);
  diamond_graph::modelview_map_t  V0 =
    sl::linear_map_factory3f::translation(0,0,-(bs.radius())) *
    sl::linear_map_factory3f::rotation(0, 3.14/4.0) *
    sl::linear_map_factory3f::translation(0,0,-(bs.center()[2]));
  diamond_graph::modelview_map_t  V1 =
    sl::linear_map_factory3f::translation(0,0,-0.2*(bs.radius())) *
    sl::linear_map_factory3f::rotation(0, 3.14/4.0) *
    sl::linear_map_factory3f::translation(0,0,-(bs.center()[2]));
  float screen_tolerance = 3.0 / 600.0;
  float threshold = screen_tolerance * 2 * tan(fovy/2);
  std::cerr.precision(10);
  std::cerr << "threshold " << threshold << std::endl;
  sl::cpu_time_clock clock;
  clock.restart();
  int N = 100;
  for(int i =0; i < N; ++i) {
    diamond_graph::modelview_map_t V = V0.lerp(V1, i/float(N-1));
    dg.extract_cut(threshold, P, V);
  }
  for(int i =0; i < N; ++i) {
    diamond_graph::modelview_map_t V = V1.lerp(V0, i/float(N-1));
    dg.extract_cut(threshold, P, V);
  }
  std::cerr << "elapsed " << (float)clock.elapsed().as_microseconds() / (N*1000.0f) << " millisec\n";
}

int main(int argc, char** argv ) {
  int patch_width = 64;
  if ( argc == 2 ) {
    patch_width = atoi(argv[1]);
  }
  //  test_codec(4);
  test_rendering_traversal(patch_width);

  return 0;
}
