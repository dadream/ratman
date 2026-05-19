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
#ifndef CBDAM_DELTA_CODEC_HPP
#define CBDAM_DELTA_CODEC_HPP

#include <vic/cbdam/base/config.hpp>
#include <vic/cbdam/base/grid_diamond.hpp>
#include <vic/cbdam/base/diamond_operator.hpp>
#include <sl/dense_array.hpp>

namespace cbdam {
  
  /**
   * T             : type of data encoded:              int32_t / color
   * DIAMOND_DATA_T: type of data stored in diamond     point3_t / color
   * OPERATOR_T    : perform operations on data         height_operator / color_operator
   */
  template<class OPERATOR_T, class DIAMOND_DATA_T>
  class delta_codec {
  public:
    typedef OPERATOR_T                                  operator_t;
    typedef DIAMOND_DATA_T                              diamond_data_t;
    typedef grid_point_t                                diamond_id_t;
    typedef grid_diamond                                grid_diamond_t;
    typedef typename operator_t::value_t                value_t;
    typedef typename sl::dense_array<value_t,2,void>    array2_t;
    
  protected:
    operator_t  diamond_operator_;
    value_t*    matrix_values_;
    array2_t    filtered_l_;
    array2_t    p_;
    array2_t	q_;
    int32_t     patch_dim_;
    int32_t     matrix_width_;

  public:
    delta_codec();
    
    virtual ~delta_codec();

    virtual void init(int32_t patch_dim);

    virtual void distribute_data_to_root(const array2_t& offset, const grid_diamond_t& r,
                                         diamond_data_t* root0, diamond_data_t* root1);
                                 
    virtual void decode_values(const array2_t& offset, const grid_diamond_t& r,
                               const diamond_data_t* d0, const diamond_data_t* d1);

  protected:
    int matrix_index(int y, int x, int child_i, int child_j);

    void fill_matrix_values(const diamond_data_t* d0,
                            const diamond_data_t* d1);
    
    void mirror_array_along_diagonal(bool copy_topleft_to_bottomright);
  };


} // namespace cbdam 

#endif // CBDAM_DELTA_CODEC_HPP

#ifndef CBDAM_DELTA_CODEC_IPP
#define CBDAM_DELTA_CODEC_IPP

namespace cbdam {

  template<class OPERATOR_T, class DIAMOND_DATA_T>
  inline delta_codec<OPERATOR_T, DIAMOND_DATA_T>::delta_codec() {
    patch_dim_ = 0;
    matrix_width_ = 0;
    matrix_values_ = 0;
  }
    
  template<class OPERATOR_T, class DIAMOND_DATA_T>
  inline delta_codec<OPERATOR_T, DIAMOND_DATA_T>::~delta_codec() {
    delete[] matrix_values_;
  }

  template<class OPERATOR_T, class DIAMOND_DATA_T>
  inline void delta_codec<OPERATOR_T, DIAMOND_DATA_T>::init(int32_t patch_dim) {
    patch_dim_ = patch_dim;
    matrix_width_ = 2 * patch_dim_ + 1;
    matrix_values_ = new value_t[matrix_width_ * matrix_width_];
    filtered_l_.resize(sl::index<2>(patch_dim_+1, patch_dim_+1));
    p_.resize(sl::index<2>(patch_dim_+1, patch_dim_+1));
    q_.resize(sl::index<2>(patch_dim_, patch_dim_));
  }

  template<class OPERATOR_T, class DIAMOND_DATA_T>
  inline void delta_codec<OPERATOR_T, DIAMOND_DATA_T>::distribute_data_to_root(const array2_t& offset,  const grid_diamond_t& r,
                                                                               diamond_data_t* root0, diamond_data_t* root1) {
    int32_t count = 0;
    // get a reference to the root array of points and modify it
    std::vector<value_t >& patch_values0 = root0->values();
    
    // fill patch :patch 0 triangle NW, top to bottom, right to left; patch 1  SE
    for(int32_t y = 0; y <= patch_dim_; ++y) {
      for(int32_t x = 0; x <= patch_dim_ - y; ++x) {
        patch_values0[count] = offset(y, x);
        ++count;
      }
    }

    count = 0;
    std::vector<value_t >& patch_values1 = root1->values();
    
    for(int32_t y = patch_dim_; y >= 0; --y) {
      for(int32_t x = patch_dim_; x >= patch_dim_ - y; --x) {
        patch_values1[count] = offset(y, x);
        ++count;
        }
    }
  }

  template<class OPERATOR_T, class DIAMOND_DATA_T>
  inline void delta_codec<OPERATOR_T, DIAMOND_DATA_T>::decode_values(const array2_t& offset, const grid_diamond_t& /*r*/,
                                                                     const diamond_data_t* d0, const diamond_data_t* d1) {
    // Put v0,v1 in filtered_l_		(patch_dim_+1, patch_dim_+1)
    // Compute q from filtered_l_	(patch_dim_,   patch_dim_)
    // Compute inner part of p from q	(patch_dim_+1, patch_dim_+1)
    // Store everything in the output matrix
    // This is the output matrix for patch_dim_ = 4. Matrix_width = 9.
    // Only half (interleaved) of the values of the matrix are meaningful.
    // v0  v0  v0  v0  v1
    //   q   q   q   q
    // v0  p   p   p   v1
    //   q   q   q   q
    // v0  p   p   p   v1
    //   q   q   q   q
    // v0  p   p   p   v1
    //   q   q   q   q
    // v0  v1  v1  v1  v1

    fill_matrix_values(d0, d1);
    if (d0 == 0) {
      mirror_array_along_diagonal(false);
    } else if (d1 == 0) {
      mirror_array_along_diagonal(true);
    }

    diamond_operator_.synthesis_in(p_, q_, filtered_l_, offset);

    // copy p
    for(int32_t y = 1; y < patch_dim_; ++y) {
      int32_t y_offset = 2 * y * matrix_width_;
      for(int32_t x = 1; x < patch_dim_; ++x) {
        matrix_values_[y_offset+2*x] = p_(y, x);
      }
    }

    // copy q to matrix
    for (int32_t y = 1; y < patch_dim_+1; ++y) {
      int32_t y_offset = (2 * y - 1) * matrix_width_;
      for(int32_t x = 1; x < patch_dim_+1; ++x) {
        matrix_values_[y_offset+2*x-1] = q_(y-1, x-1);
      }
    }
    
    // copy boundaries from input data. They must remain equal to previous level values.
    uint32_t offset_bottom = 2 * patch_dim_ * matrix_width_ + 2 * patch_dim_;
    if (d0 != 0) {
      const std::vector<value_t >& v0 = d0->values();
      uint32_t count = 0;
      for (int32_t i = 0; i < patch_dim_ + 1; ++i) {
	matrix_values_[2 * i * matrix_width_] = v0[count];	// copy left column
	matrix_values_[2 * i] = v0[i];				// copy top row
	if (d1 == 0) {
	  // miss patch 1 mirror boundaries of patch 0 along diagonal
	  matrix_values_[2 * (patch_dim_ - i) * matrix_width_ + 2 * patch_dim_] = v0[i];// copy right column
	  matrix_values_[offset_bottom - 2 * i] = v0[count];				// copy bottom row
	}
	count += patch_dim_ + 1 - i;
      }
    }
    if (d1 != 0) {
      const std::vector<value_t >& v1 = d1->values();
      uint32_t count = 0;
      for (int32_t i = 0; i < patch_dim_ + 1; ++i) {
	matrix_values_[2 * (patch_dim_ - i) * matrix_width_ + 2 * patch_dim_] = v1[count];// copy right column
	matrix_values_[offset_bottom - 2 * i] = v1[i];					  // copy bottom row
	if (d0 == 0) {
	  // miss patch 0 mirror boundaries of patch 1 along diagonal
	  matrix_values_[2 * i * matrix_width_] = v1[i];	// copy left column
	  matrix_values_[2 * i] = v1[count];			// copy top row
	}
	count += patch_dim_+1-i;
      }
    }
  }

  template<class OPERATOR_T, class DIAMOND_DATA_T>
  inline void  delta_codec<OPERATOR_T, DIAMOND_DATA_T>::mirror_array_along_diagonal(bool copy_topleft_to_bottomright) {
    int32_t h = filtered_l_.extent()[0];
    int32_t w = filtered_l_.extent()[1];
    assert(w==h);
    if (copy_topleft_to_bottomright) {
      for (int32_t y = 0; y < w; ++y) {
        for(int32_t x = 0; x < h - y; ++x) {
          filtered_l_(w-1-x, w-1-y) = filtered_l_(y, x);
        }
      }
    } else {
      for (int32_t y = 0; y < w; ++y) {
        for(int32_t x = 0; x < w - y; ++x) {
          filtered_l_(y, x) = filtered_l_(w-1-x, w-1-y);
        }
      }
    }    
  }
  
  template<class OPERATOR_T, class DIAMOND_DATA_T>
  inline void delta_codec<OPERATOR_T, DIAMOND_DATA_T>::fill_matrix_values(const diamond_data_t* d0,
                                                                          const diamond_data_t* d1) {
    int32_t count = 0;
    if (d0 != 0) {
      const std::vector<value_t >& v0 = d0->values();
      for(int32_t y = 0; y < patch_dim_+1; ++y) {
        for(int32_t x = 0; x < patch_dim_+1 - y; ++x) {
          filtered_l_(y, x) = v0[count];
          ++count;
        }
      }	
    }
    
    if (d1 != 0) {
      const std::vector<value_t >& v1 = d1->values();
      count = (patch_dim_+1) * (patch_dim_+2) / 2 - 1;
      for(int32_t y = 0; y < patch_dim_+1; ++y) {
        for(int32_t x = patch_dim_+1 - y - 1; x < patch_dim_+1; ++x) {
          filtered_l_(y, x) = v1[count];
          --count;
        }
      }
    }
  }

  template<class OPERATOR_T, class DIAMOND_DATA_T>
  inline int delta_codec<OPERATOR_T, DIAMOND_DATA_T>::matrix_index(int y, int x, int child_i, int child_j) {
    if (child_i == 0) {
      if (child_j == 0) {
        return ( x - y + patch_dim_) * matrix_width_ + (-x - y + patch_dim_);
      } else {
        return (-x - y + patch_dim_) * matrix_width_ + (-x + y + patch_dim_);
      }
    } else {
      if (child_j == 0) {
        return (-x + y + patch_dim_) * matrix_width_ + ( x + y + patch_dim_);
      } else {
        return ( x + y + patch_dim_) * matrix_width_ + ( x - y + patch_dim_);
      }
    }
  }
  
#if 0
  template<class OPERATOR_T, class DIAMOND_DATA_T>
  inline void delta_codec<OPERATOR_T, DIAMOND_DATA_T>::distribute_data_to_child00(diamond_data_t* d) {
    // get values relative to the child i,j
    int32_t x_new, y_new, idx;
    int32_t count = 0;

    // get a reference to the diamond array of points and modify it
    std::vector<value_t>& patch_values = d->values();
    
    for(int32_t y = 0; y <= patch_dim_; ++y) {
      for(int32_t x = 0; x <= patch_dim_ - y; ++x) {
        // west
        x_new = (-x - y + patch_dim_);
        y_new = ( x - y + patch_dim_);
        idx = y_new * matrix_width_ + x_new;
        assert(idx >= 0 && idx < matrix_width_*matrix_width_);

        value_t& p = patch_values[count];
        ++count;
        p = matrix_values_[idx];
        //        operator_t::rotate_normal_for_child00(p, x + y == patch_dim_);
      }
    }
  }

  template<class OPERATOR_T, class DIAMOND_DATA_T>
  inline void delta_codec<OPERATOR_T, DIAMOND_DATA_T>::distribute_data_to_child01(diamond_data_t* d) {
    // get values relative to the child i,j
    int32_t x_new, y_new, idx;
    int32_t count = 0;
    
    // get a reference to the diamond array of points and modify it
    std::vector<value_t>& patch_values = d->values();

    for(int32_t y = 0; y <= patch_dim_; ++y) {
      for(int32_t x = 0; x <= patch_dim_ - y; ++x) {
        // north       
        x_new = (-x + y + patch_dim_);
        y_new = (-x - y + patch_dim_);
        idx = y_new * matrix_width_ + x_new;
        assert(idx >= 0 && idx < matrix_width_*matrix_width_);

        value_t& p = patch_values[count];
        ++count;
        p = matrix_values_[idx];
        //        operator_t::rotate_normal_for_child01(p, x + y == patch_dim_);
      }
    }
  }

  template<class OPERATOR_T, class DIAMOND_DATA_T>
  inline void delta_codec<OPERATOR_T, DIAMOND_DATA_T>::distribute_data_to_child10(diamond_data_t* d) {
    // get values relative to the child i,j
    int32_t x_new, y_new, idx;
    int32_t count = 0;

    // get a reference to the diamond array of points and modify it
    std::vector<value_t>& patch_values = d->values();
    
    for(int32_t y = 0; y <= patch_dim_; ++y) {
      for(int32_t x = 0; x <= patch_dim_ - y; ++x) {
        // east
        x_new = ( x + y + patch_dim_);
        y_new = (-x + y + patch_dim_);
        idx = y_new * matrix_width_ + x_new;
        assert(idx >= 0 && idx < matrix_width_*matrix_width_);

        value_t& p = patch_values[count];
        ++count;
        p = matrix_values_[idx];
        //        operator_t::rotate_normal_for_child10(p, x + y == patch_dim_);
      }
    }
  }

  template<class OPERATOR_T, class DIAMOND_DATA_T>
  inline void delta_codec<OPERATOR_T, DIAMOND_DATA_T>::distribute_data_to_child11(diamond_data_t* d) {
    // get values relative to the child i,j
    int32_t x_new, y_new, idx;
    int32_t count = 0;

    // get a reference to the diamond array of points and modify it
    std::vector<value_t>& patch_values = d->values();
     
    for(int32_t y = 0; y <= patch_dim_; ++y) {
      for(int32_t x = 0; x <= patch_dim_ - y; ++x) {
        // south
        x_new = ( x - y + patch_dim_);
        y_new = ( x + y + patch_dim_);
        idx = y_new * matrix_width_ + x_new;
        assert(idx >= 0 && idx < matrix_width_*matrix_width_);

        value_t& p = patch_values[count];
        ++count;
        p = matrix_values_[idx];
        //        operator_t::rotate_normal_for_child11(p, x + y == patch_dim_);
      }
    }
  }
#endif
  
} // namespace cbdam 

#endif // CBDAM_DELTA_CODEC_IPP
