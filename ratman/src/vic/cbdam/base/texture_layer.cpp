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
#include <vic/cbdam/base/texture_layer.hpp>


namespace cbdam {

  ///////////////////////////////////////////// texture_tile_stack ////////////////////////////////////////////////
  
  void texture_tile_stack::build_representation_in(uncompressed_rgba_image_t* img, uint8_t bg_r, uint8_t bg_g, uint8_t bg_b, uint8_t bg_a, bool add_boundaries) {
    assert(img);
    assert(img->extent(1) == img->extent(2));
    assert(img->extent(0) == 4);

    img->fill(bg_r, bg_g, bg_b, bg_a);

    vic::img::gl_quadtree_image_processor gl_quadproc;
    const grid_point_t& dst_lxy = level_xy_;

    std::size_t sz = tile_stack_.size();
    for(std::size_t i = 0; i < sz; ++i) {
      const grid_point_t& src_lxy = tile_stack_[i].level_xy();
      assert(tile_stack_[i].image());
      uncompressed_rgba_image_t stack_img;
      tile_stack_[i].image()->uncompress_to(stack_img);

      assert(img->extent(0) == stack_img.extent(0));
      assert(img->extent(1) == stack_img.extent(1));
      assert(img->extent(2) == stack_img.extent(2));

      gl_quadproc.blend_in(*img, dst_lxy[0], dst_lxy[1], dst_lxy[2],
                           stack_img, src_lxy[0], src_lxy[1], src_lxy[2]);
    }

    if (add_boundaries) {
      std::size_t tile_width = img->extent(1);
      for(std::size_t i = 0; i < tile_width; ++i) {
	for(int c = 0; c < 4; ++c) {
	  (*img)(c, 0, i) = 255;
	  (*img)(c, tile_width-1, i) = 255;
	  (*img)(c, i, 0) = 255;
	  (*img)(c, i, tile_width-1) = 255;
	}
      }
    }
  }

  texture_tile_stack::affine_map_t texture_tile_stack::texture_matrix(const grid_point_t& d_level_xy, bool odd,
                                                                      const grid_diamond_t& d, int patch_id, std::size_t root_count) const {
    //    if (odd) SL_TRACE_OUT(-1) << "fix texture matrix for odd levels" << std::endl;
    //    std::cerr << "texture_matrix, repr level " << representation_.level_xy()[0] << " dmd level " << d_level_xy[0] << std::endl;
    // suppose its even
    int half_diamond_level = d_level_xy[0]/2;
    int delta_level = half_diamond_level - level_xy_[0];
    assert(delta_level>=0);
    int zoom_factor = (1 << (delta_level));
    double scale = 1.0 / double(zoom_factor);
    const grid_point_t& dc0 = d.corner(0);
    const grid_point_t& dc1 = d.corner(2);

    // identify translation rotation for the tex matr depending on diamond diagonal orientation.
    // Diagonal lies on one of 4 planes: plane +Z when root count == 1, plane +Z+X-Z-X root_count = 8
    // identify the coordinates which have to be compared to understand diagonal orientation.
    // pure alchemy!
    int x0 = dc0[0];
    int y0 = dc0[1];
    int x1 = dc1[0];
    int y1 = dc1[1];

    if (root_count == 8) {
      const grid_point_t id = d.id();
      const grid_point_t patch_corner = d.corner(1 + patch_id*2);

      // identify plane
      // cylindrical root size = half root size
      int cylindrical_root_size = (max_grid_coord() - min_grid_coord()) / 2; 
      int half_root_size = cylindrical_root_size /2;
      if (id[2] == half_root_size && patch_corner[2] == half_root_size) {
	// +Z
	// as in root_count = 1
      } else if (id[0] == half_root_size && patch_corner[0] == half_root_size) {
	// +X
	x0 = -dc0[2];
	x1 = -dc1[2];
      } else if (id[2] == -half_root_size && patch_corner[2] == -half_root_size) {
	// -Z
	x0 = -dc0[0];
	x1 = -dc1[0];
      } else if (id[0] == -half_root_size && patch_corner[0] == -half_root_size) {
	// -X
	x0 = dc0[2];
	x1 = dc1[2];
      } else {
	SL_TRACE_OUT(-1) << "texture_tile_stack grid point not lying on boundaries " << id << std::endl;
      }

    } 

    if (odd) {
      int K = 0;  // rotate k*90deg to go back to diagonal 45 deg
      double t_back_x = 0;
      double t_back_y = 0;
      if (x0 == x1) {
	if (y0 < y1) {
	  K = 0; // dir points to n
	  t_back_x = patch_id == 0 ? 1.0 : 0.0;
	  t_back_y = 0.5;
	} else {
	  K = 2; // dir points to s
	  t_back_x = patch_id == 0 ? 0.0 : 1.0;
	  t_back_y = 0.5;
	}
      } else {
	assert(y0 == y1);
	if (x0 < x1) {
	  K = 1; // dir points to e
	  t_back_x = 0.5;
	  t_back_y = patch_id == 0 ? 0.0 : 1.0;
	} else {
	  K = 3; // dir points to w
	  t_back_x = 0.5;
	  t_back_y = patch_id == 0 ? 1.0 : 0.0;
	}
      }
      

      double tr_x = d_level_xy[1] * scale - level_xy_[1];
      double tr_y = d_level_xy[2] * scale - level_xy_[2];
#if 0
      std::cerr << "texture matrix for odd d " << d.id() << ", dlxy " << d_level_xy << " to stack lxy " << level_xy_ << ", patch_id " << patch_id 
		<< " xy0 " << x0 << "," << y0 << ", xy1 " << x1 << ", " << y1 
		<< ", tr xy " << tr_x << ", " << tr_y  << ", scale " << scale << ", rot K " << K << std::endl;
#endif
      double invsqrt2 = 1.0 / sqrt(2.0);
      return (sl::linear_map_factory3d::translation(tr_x, tr_y, 0.0) *
	      sl::linear_map_factory3d::scaling(scale, scale, 1.0) *
	      sl::linear_map_factory3d::translation(t_back_x, t_back_y, 0.0) *
	      sl::linear_map_factory3d::scaling(invsqrt2, invsqrt2, 1.0) *
	      sl::linear_map_factory3d::rotation(2, 0.785398163 - K * 1.570796327) *
	      sl::linear_map_factory3d::translation(-0.5, -0.5, 0.0));  
    } else {
      // which rotation with respect to main diagonal from l,l -> h,h (45deg)
      int K = 0;  // rotate k*90deg to go back to diagonal 45 deg
      if (x0 < x1) {
	if (y0 < y1) {
	  K = 0; // dir points to ne
	} else {
	  K = 3; // dir points to se
	}
      } else {
	if (y0 < y1) {
	  K = 1; // dir points to nw
	} else {
	  K = 2; // dir points to sw
	}
      }

      double tr_x = d_level_xy[1] * scale - level_xy_[1];
      double tr_y = d_level_xy[2] * scale - level_xy_[2];
#if 0
      std::cerr << "texture matrix for d " << d.id() << ", dlxy " << d_level_xy << " to stack lxy " << level_xy_ << patch_id 
		<< " xy0 " << x0 << "," << y0 << ", xy1 " << x1 << ", " << y1 
		<< ", tr xy " << tr_x << ", " << tr_y  << ", scale " << scale << ", rot K " << K << std::endl;
#endif
      return (sl::linear_map_factory3d::translation(tr_x, tr_y, 0) *
	      sl::linear_map_factory3d::scaling(scale, scale, 1) *
	      sl::linear_map_factory3d::translation(0.5, 0.5, 0) *
	      sl::linear_map_factory3d::rotation(2, K * 1.570796327) *
	      sl::linear_map_factory3d::translation(-0.5, -0.5, 0));    
    }
  }

  ///////////////////////////////////////////// texture_layer ////////////////////////////////////////////////

  texture_layer::texture_layer(const std::string& id, texture_fetcher_t* tf, texture_refiner* tr, const coordinate_transform* geo_xform, std::size_t first_level, std::size_t last_level,
			       double min_altitude, double max_altitude) :
    id_(id), texture_fetcher_(tf), texture_refiner_(tr), min_altitude_(min_altitude), max_altitude_(max_altitude) {
    assert(texture_fetcher_);
    assert(texture_refiner_);
    quadtree_ = new grid_texture_quadtree(); // FIXME move params to creator

    quadtree_->set_texture_refiner(texture_refiner_);
    quadtree_->open(texture_fetcher_, geo_xform, first_level, last_level);
    is_active_ = quadtree_->is_open();

    if (is_active_) {
      quadtree_->set_decoded_diamond_budget(32); // FIXME
      quadtree_->init_heaps();
    }
  }

  texture_layer::~texture_layer() {
    SL_TRACE_OUT(1) << "Start delete" << std::endl;

    assert(quadtree_);
    delete quadtree_;
    quadtree_ = 0;

    SL_TRACE_OUT(1) << "Del fetcher" << std::endl;
    if (texture_fetcher_) {
      delete texture_fetcher_;
      texture_fetcher_ = 0;
    }
    SL_TRACE_OUT(1) << "Del refiner" << std::endl;
    if (texture_refiner_) {
      delete texture_refiner_;
      texture_refiner_ = 0;
    }
    SL_TRACE_OUT(1) << "End delete" << std::endl;
  }
  
}
