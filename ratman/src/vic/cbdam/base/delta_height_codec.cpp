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
#include <vic/cbdam/base/delta_height_codec.hpp>

namespace cbdam {

  delta_height_codec::delta_height_codec() :
      super_t() {
    matrix_points_ = 0;
    matrix_normals_ = 0;
    geo_xform_ = 0;
    scale_factor_ = 1.0;
  }
    
  delta_height_codec::~delta_height_codec() {
    if (matrix_points_) {
      delete[] matrix_points_;
      matrix_points_ = 0;
    }
    
    if (matrix_normals_) {
      delete[] matrix_normals_;
      matrix_normals_ = 0;
    }

    geo_xform_ = 0;
  }

  void delta_height_codec::init(int patch_dim, double scale_factor, const coordinate_transform* geo_xform) {
    super_t::init(patch_dim);
    scale_factor_ = scale_factor;
    geo_xform_ = geo_xform;

    matrix_points_ = new point3d_t[matrix_width_*matrix_width_];
    matrix_normals_ = new normal_t[matrix_width_*matrix_width_];
  }

  void delta_height_codec::distribute_data_to_root(const array2_t& offset, const grid_diamond_t& r,
                                                   diamond_data_t* root0, diamond_data_t* root1) {
    super_t::distribute_data_to_root(offset, r, root0, root1);

    // compute 3d points
    sl::dense_array<point3_t, 2, void> points(offset.extent());
    const int N = int(patch_dim_);
    const double duv = 1.0/double(N);
    diamond_data_t* ptr_dd[2] = {root0, root1};
    for(int patch_id = 0; patch_id < 2; ++patch_id) {
      const std::vector<int32_t>& heights = ptr_dd[patch_id]->values();
      std::vector<point3_t>& gl_points = ptr_dd[patch_id]->gl_points();
      
      const grid_point_t gp0 = r.corner((1+2*patch_id)%4);
      const grid_point_t gp1 = r.corner((2+2*patch_id)%4);
      const grid_point_t gp2 = r.corner((0+2*patch_id)%4);
      
      point3d_t  dgp0     = point3d_t(gp0[0], gp0[1], gp0[2]);
      vector3d_t dgp0dgp1 = point3d_t(gp1[0], gp1[1], gp1[2]) - dgp0;
      vector3d_t dgp0dgp2 = point3d_t(gp2[0], gp2[1], gp2[2]) - dgp0;

      int count = 0;        
      for(int y = 0; y <= N; ++y) {
        for(int x = 0; x <= N - y; ++x) {
          point3d_t dgp_p = (dgp0 + dgp0dgp1 * duv * x + dgp0dgp2 * duv * y);
          grid_point_t gp_p = grid_point_t(int32_t(dgp_p[0]), int32_t(dgp_p[1]), int32_t(dgp_p[2]));
          point3d_t pd = geo_xform_->xyz_from_grid(gp_p, scale_factor_ *  heights[count]);
          gl_points[count] = point3_t(pd[0], pd[1], pd[2]);
          
          // store points also in the matrix, for easier index computation while computing normals.
          if (patch_id == 0) {
            points(y, x) = gl_points[count];
          } else {
            points(N-y, N-x) = gl_points[count];
          }
          ++count;                     
        }
      }
    }

    // compute and set normals in diamond_data_t
    std::vector<normal_t>& gl_normals0 = root0->gl_normals();
    std::vector<normal_t>& gl_normals1 = root1->gl_normals();
    int count0 = 0;
    int count1 = (N+2)*(N+1)/2-1;
    for(int y = 0; y <= N; ++y) {
      int yl = std::max(0, y-1);
      int yh = std::min(N, y+1);
      for(int x = 0; x <= N; ++x) {
        int xl = sl::max(0, x-1);
        int xh = sl::min(N, x+1);
        vector3_t v0 = points(yh, xh) - points(yl, xl);
        vector3_t v1 = points(yl, xh) - points(yh, xl);
        normal_t n = v0.normal(v1).ok_normalized();
        if (y < N-x) {
          gl_normals0[count0] = n;
          ++count0;
        } else if (y > N-x) {
          gl_normals1[count1] = n;
          --count1;
        } else {
          gl_normals0[count0] = n;
          gl_normals1[count1] = n;
          ++count0;
          --count1;
        }
      }
    }
  }
                                 
  void delta_height_codec::decode_values(const array2_t& offset,
                                         const grid_diamond_t& d,
                                         const diamond_data_t* d0,
                                         const diamond_data_t* d1) {
    // fill matrix values with parent heights
    super_t::decode_values(offset, d, d0, d1);

    // fill boundaries of normal matrix with parent normals
    if (d0) {
      int idx_patch_vert = (patch_dim_+2)*(patch_dim_+1)/2-1;
      int idx_matrix_left = (matrix_width_-1)*matrix_width_;
      int idx_matrix_top = 0;
      const std::vector<normal_t>& normal0 = d0->gl_normals();
      
      for(int i = 0; i <= patch_dim_; ++i) {
        matrix_normals_[idx_matrix_left]   = normal0[idx_patch_vert];     // left   (bottom->top)
        matrix_normals_[idx_matrix_top ]   = normal0[i];                  // top    (left->right)
        idx_patch_vert -= (i+2);
        idx_matrix_left -= 2*matrix_width_;
        idx_matrix_top += 2;
      }
    }

    if (d1) {
      int idx_patch_vert = (patch_dim_+2)*(patch_dim_+1)/2-1;
      int idx_matrix_right = matrix_width_-1;
      int idx_matrix_bottom = matrix_width_*matrix_width_-1;
      const std::vector<normal_t>& normal1 = d1->gl_normals();
      
      for(int i = 0; i <= patch_dim_; ++i) {      
        matrix_normals_[idx_matrix_right]  = normal1[idx_patch_vert];     // right  (top->bottom)
        matrix_normals_[idx_matrix_bottom] = normal1[i];                  // bottom (right->left)
        idx_patch_vert -= (i+2);
        idx_matrix_right += 2*matrix_width_;
        idx_matrix_bottom -= 2;
      }
    }

    // FIXME CALL compute_patch_3dpoints from here, passing the 3 corners
  }

  void delta_height_codec::fill_matrix_normal() {
    // normals for inner control points
    for(int y = 2; y < matrix_width_-1; y += 2) {
      int yl = (y - 1) * matrix_width_;
      int yy = y * matrix_width_;
      int yh = (y + 1) * matrix_width_;
      for(int x = 2; x < matrix_width_ - 1; x += 2) {
        vector3d_t v0 = matrix_points_[yh+x+1] - matrix_points_[yl+x-1];
        vector3d_t v1 = matrix_points_[yl+x+1] - matrix_points_[yh+x-1];
	normald_t n = v0.normal(v1).ok_normalized();
        matrix_normals_[yy + x] = normal_t(n[0], n[1], n[2]);
      }
    }
    
    // normals for new points
    for(int y = 1; y < matrix_width_-1; y += 2) {
      int yl = (y - 1) * matrix_width_;
      int yy = y * matrix_width_;
      int yh = (y + 1) * matrix_width_;
      for(int x = 1; x < matrix_width_ - 1; x += 2) {
        vector3d_t v0 = matrix_points_[yh+x+1] - matrix_points_[yl+x-1];
        vector3d_t v1 = matrix_points_[yl+x+1] - matrix_points_[yh+x-1];
	normald_t n = v0.normal(v1).ok_normalized();
        matrix_normals_[yy + x] = normal_t(n[0], n[1], n[2]);
      }
    }    
  }
  
  void delta_height_codec::compute_patch_3dpoints(const grid_diamond_t& child_d, int patch_id, int child_i, int child_j) {
    // compute and set in matrix_points the 3d point derived from h, for patch (i,j)
    
    const grid_point_t gp0 = child_d.corner((1+2*patch_id)%4);
    const grid_point_t gp1 = child_d.corner((2+2*patch_id)%4);
    const grid_point_t gp2 = child_d.corner((0+2*patch_id)%4);
#if 0
    std::cerr << "Computing pts for dmd " << child_d.id() << ", patch " << patch_id << ",  i " << child_i << ", j " << child_j << std::endl;

    std::cerr << "    gp0 " << child_d.corner(0) << std::endl
	      << ",   gp1 " << child_d.corner(1) << std::endl
              << ",   gp2 " << child_d.corner(2) << std::endl
	      << ",   gp3 " << child_d.corner(3) << std::endl;

#endif

    const point3d_t  dgp0     = point3d_t(gp0[0], gp0[1], gp0[2]);
    const vector3d_t dgp0dgp1 = point3d_t(gp1[0], gp1[1], gp1[2]) - dgp0;
    const vector3d_t dgp0dgp2 = point3d_t(gp2[0], gp2[1], gp2[2]) - dgp0;
        
    const int N = int(patch_dim_);
    const double duv = 1.0/double(N);
    int count = 0;

    for(int y = 0; y <= N; ++y) {
      for(int x = 0; x <= N - y; ++x) {
        point3d_t dgp_p = (dgp0 + dgp0dgp1 * duv * x + dgp0dgp2 * duv * y);
        grid_point_t gp_p = grid_point_t(int32_t(dgp_p[0]), int32_t(dgp_p[1]), int32_t(dgp_p[2]));

        // get index to access matrix, depending on which child we are
        int idx = matrix_index(y, x, child_i, child_j);
        matrix_points_[idx] = geo_xform_->xyz_from_grid(gp_p, scale_factor_ *  matrix_values_[idx]);
      }
      ++count;
    }
  }

  void delta_height_codec::distribute_data_to_child(const diamond_id_t& id, diamond_data_t* dd, int child_i, int child_j) {
    const int N = int(patch_dim_);
    int count = 0;
    std::vector<int32_t>& patch_values  = dd->values();
    std::vector<point3_t>& patch_points = dd->gl_points();
    std::vector<normal_t>& patch_normal = dd->gl_normals();
    bool use_normal = patch_normal.size() > 0;
    point3d_t diamond_center =  geo_xform_->xyz_from_grid(id, 0);
    dd->diamond_center() = diamond_center;

    for(int y = 0; y <= N; ++y) {
      for(int x = 0; x <= N - y; ++x) {
        int idx = matrix_index(y, x, child_i, child_j);
        patch_values[count] = matrix_values_[idx];
	// set gl point relative to diamond center
	vector3d_t v = matrix_points_[idx] - diamond_center;
        patch_points[count] = point3_t(v[0], v[1], v[2]);
        if (use_normal) {
          patch_normal[count] = matrix_normals_[idx];
        }
        ++count;
      }
    }
  }
  
} // namespace cbdam
