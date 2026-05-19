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
#ifndef CBDAM_DIAMOND_OPERATOR_HPP
#define CBDAM_DIAMOND_OPERATOR_HPP

#include <vic/cbdam/base/config.hpp>
#include <vic/cbdam/base/grid_point.hpp>
#include <vic/cbdam/base/color_rgb.hpp>
#include <sl/dense_array.hpp>
#include <sl/array_codec.hpp>
#include <sl/fixed_size_vector.hpp>
#include <algorithm>

namespace cbdam {

  // FIXME move to sl
  template <class T>
  static inline T max(const T& x00,
                      const T& x01,
                      const T& x10,
                      const T& x11) {
    return std::max(std::max(std::max(x00, x01), x10), x11);
  }

  // FIXME move to sl
  template <class T>
  static inline T min(const T& x00,
                      const T& x01,
                      const T& x10,
                      const T& x11) {
    return std::min(std::min(std::min(x00, x01), x10), x11);
  }

  // FIXME move to sl
  static inline int32_t component_max(int32_t x00,
				      int32_t x01) {
    return std::max(x00,x01);
  }

  static inline int32_t component_min(int32_t x00,
				      int32_t x01) {
    return std::min(x00,x01);
  }


  // FIXME move to sl
  static inline int32_t average(int32_t x00,
				int32_t x01,
				int32_t x10,
				int32_t x11) {
    int32_t x = (x00+x01+x10+x11);
    return (x>0)?((x+2)/4):((x-2)/4); // integer rounding
  }

  static inline int32_t half_average(int32_t x00,
				     int32_t x01,
				     int32_t x10,
				     int32_t x11) {
    int32_t x = (x00+x01+x10+x11);
    return (x>0)?((x+4)/8):((x-4)/8); // integer rounding
  }

  static inline int32_t median(int32_t x00,
			       int32_t x01,
			       int32_t x10,
			       int32_t x11) {
    int32_t values[4];
    values[0] = x00;
    values[1] = x01;
    values[2] = x10;
    values[3] = x11;
    std::sort(&(values[0]), &(values[0])+4);
    int32_t x = (values[1]+values[2]);
    return (x>0)?((x+1)/2):((x-1)/2); // integer rounding
  }

  static inline int32_t half_median(int32_t x00,
				    int32_t x01,
				    int32_t x10,
				    int32_t x11) {
    int32_t values[4];
    values[0] = x00;
    values[1] = x01;
    values[2] = x10;
    values[3] = x11;
    std::sort(&(values[0]), &(values[0])+4);
    int32_t x = (values[1]+values[2]);
    return (x>0)?((x+2)/4):((x-2)/4); // integer rounding
  }

  static inline int32_t half_cisl_inf(int32_t x00,
				      int32_t x01,
				      int32_t x10,
				      int32_t x11) {
    return sl::median(int32_t(0),
                      min(x00, x01, x10, x11),
                      max(x00, x01, x10, x11))/2; // integer division
  }

  static inline int32_t neville4(int32_t v00, int32_t v01, int32_t v02, int32_t v03, 
				 int32_t v10, int32_t v11, int32_t v12, int32_t v13, 
				 int32_t v20, int32_t v21, int32_t v22, int32_t v23, 
				 int32_t v30, int32_t v31, int32_t v32, int32_t v33) {
#if 0
    const double ZZ =   0.0;
    const double R0 =  10.0/32.0;
    const double R1 =  -1.0/32.0;
    // FIXME convert to integer
    return int32_t(ZZ*v00 + R1*v01 + R1*v02 + ZZ*v03 +
                   R1*v10 + R0*v11 + R0*v12 + R1*v13
                   R1*v20 + R0*v21 + R0*v22 + R1*v23
                   ZZ*v30 + R1*v31 + R1*v32 + ZZ*v33 +
                   0.5);
#else
    if (v00) {};if (v30) {};if (v03) {};if (v33) {};
    int32_t x = 10*(v11+v12+v21+v22)-(v01+v02+v10+v20+v13+v23+v31+v32);
    return (x>0) ? (x+16)/32 : (x-16)/32;
#endif
  }

  static inline int32_t half_neville4(int32_t v00, int32_t v01, int32_t v02, int32_t v03, 
				      int32_t v10, int32_t v11, int32_t v12, int32_t v13, 
				      int32_t v20, int32_t v21, int32_t v22, int32_t v23, 
				      int32_t v30, int32_t v31, int32_t v32, int32_t v33) {
#if 0
    const double ZZ =   0.0;
    const double R0 =  10.0/32.0;
    const double R1 =  -1.0/32.0;
    // FIXME convert to integer
    return int32_t(0.5*(ZZ*v00 + R1*v01 + R1*v02 + ZZ*v03 +
                        R1*v10 + R0*v11 + R0*v12 + R1*v13
                        R1*v20 + R0*v21 + R0*v22 + R1*v23
                        ZZ*v30 + R1*v31 + R1*v32 + ZZ*v33) +
                   0.5);
#else
    if (v00) {};if (v30) {};if (v03) {};if (v33) {};
    int32_t x = 10*(v11+v12+v21+v22)-(v01+v02+v10+v20+v13+v23+v31+v32);
    return (x>0) ? (x+32)/64 : (x-32)/64;
#endif
  }

  static inline int32_t neville6(int32_t v00, int32_t v01, int32_t v02, int32_t v03, int32_t v04, int32_t v05, 
				 int32_t v10, int32_t v11, int32_t v12, int32_t v13, int32_t v14, int32_t v15,  
				 int32_t v20, int32_t v21, int32_t v22, int32_t v23, int32_t v24, int32_t v25,  
				 int32_t v30, int32_t v31, int32_t v32, int32_t v33, int32_t v34, int32_t v35,
				 int32_t v40, int32_t v41, int32_t v42, int32_t v43, int32_t v44, int32_t v45,
				 int32_t v50, int32_t v51, int32_t v52, int32_t v53, int32_t v54, int32_t v55) {
    // FIXME convert to integer
    const double ZZ =   0.0;
    const double R0 =  87.0/256.0;
    const double R1 = -27.0/512.0;
    const double R2 =   1.0/256.0;
    const double R3 =   3.0/512.0;
    return int32_t(ZZ*v00 + ZZ*v01 + R3*v02 + R3*v03 + ZZ*v04 + ZZ*v05 + 
                   ZZ*v10 + R2*v11 + R1*v12 + R1*v13 + R2*v14 + ZZ*v15 + 
                   R3*v20 + R1*v21 + R0*v22 + R0*v23 + R1*v24 + R3*v25 + 
                   R3*v30 + R1*v31 + R0*v32 + R0*v33 + R1*v34 + R3*v35 +
                   ZZ*v40 + R2*v41 + R1*v42 + R1*v43 + R2*v44 + ZZ*v45 +
                   ZZ*v50 + ZZ*v51 + R3*v52 + R3*v53 + ZZ*v54 + ZZ*v55 +
                   0.5);
  }

  static inline int32_t half_neville6(int32_t v00, int32_t v01, int32_t v02, int32_t v03, int32_t v04, int32_t v05, 
                                      int32_t v10, int32_t v11, int32_t v12, int32_t v13, int32_t v14, int32_t v15,  
                                      int32_t v20, int32_t v21, int32_t v22, int32_t v23, int32_t v24, int32_t v25,  
                                      int32_t v30, int32_t v31, int32_t v32, int32_t v33, int32_t v34, int32_t v35,
                                      int32_t v40, int32_t v41, int32_t v42, int32_t v43, int32_t v44, int32_t v45,
                                      int32_t v50, int32_t v51, int32_t v52, int32_t v53, int32_t v54, int32_t v55) {
    // FIXME convert to integer
    const double ZZ =   0.0;
    const double R0 =  87.0/256.0;
    const double R1 = -27.0/512.0;
    const double R2 =   1.0/256.0;
    const double R3 =   3.0/512.0;
    return int32_t(0.5*(ZZ*v00 + ZZ*v01 + R3*v02 + R3*v03 + ZZ*v04 + ZZ*v05 + 
                        ZZ*v10 + R2*v11 + R1*v12 + R1*v13 + R2*v14 + ZZ*v15 + 
                        R3*v20 + R1*v21 + R0*v22 + R0*v23 + R1*v24 + R3*v25 + 
                        R3*v30 + R1*v31 + R0*v32 + R0*v33 + R1*v34 + R3*v35 +
                        ZZ*v40 + R2*v41 + R1*v42 + R1*v43 + R2*v44 + ZZ*v45 +
                        ZZ*v50 + ZZ*v51 + R3*v52 + R3*v53 + ZZ*v54 + ZZ*v55) +
                   0.5);
  }

  template <enum sl::vector_orientation O, std::size_t N, class T>
  static inline sl::fixed_size_vector<O, N, T> component_max(const sl::fixed_size_vector<O, N, T>& x00,
							     const sl::fixed_size_vector<O, N, T>& x01) {
    sl::fixed_size_vector<O, N, T> result; // = sl::tags::not_initalized();

    for (std::size_t d = 0; d<N; ++d) {
      result[d] = component_max(x00[d], x01[d]);
    }
    return result;
  }

  template <enum sl::vector_orientation O, std::size_t N, class T>
  static inline sl::fixed_size_vector<O, N, T> component_min(const sl::fixed_size_vector<O, N, T>& x00,
							     const sl::fixed_size_vector<O, N, T>& x01) {
    sl::fixed_size_vector<O, N, T> result; // = sl::tags::not_initalized();

    for (std::size_t d = 0; d<N; ++d) {
      result[d] = component_min(x00[d], x01[d]);
    }
    return result;
  }
    
  template <enum sl::vector_orientation O, std::size_t N, class T>
  static inline sl::fixed_size_vector<O, N, T> median(const sl::fixed_size_vector<O, N, T>& x00,
						      const sl::fixed_size_vector<O, N, T>& x01,
						      const sl::fixed_size_vector<O, N, T>& x10,
						      const sl::fixed_size_vector<O, N, T>& x11) {
    sl::fixed_size_vector<O, N, T> result; // = sl::tags::not_initalized();

    for (std::size_t d = 0; d<N; ++d) {
      result[d] = median(x00[d], x01[d], x10[d], x11[d]);
    }
    return result;
  }

  template <enum sl::vector_orientation O, std::size_t N, class T>
  static inline sl::fixed_size_vector<O, N, T> half_median(const sl::fixed_size_vector<O, N, T>& x00,
							   const sl::fixed_size_vector<O, N, T>& x01,
							   const sl::fixed_size_vector<O, N, T>& x10,
							   const sl::fixed_size_vector<O, N, T>& x11) {
    sl::fixed_size_vector<O, N, T> result; // = sl::tags::not_initalized();

    for (std::size_t d = 0; d<N; ++d) {
      result[d] = half_median(x00[d], x01[d], x10[d], x11[d]);
    }
    return result;
  }

  template <enum sl::vector_orientation O, std::size_t N, class T>
  static inline sl::fixed_size_vector<O, N, T> half_cisl_inf(const sl::fixed_size_vector<O, N, T>& x00,
							     const sl::fixed_size_vector<O, N, T>& x01,
							     const sl::fixed_size_vector<O, N, T>& x10,
							     const sl::fixed_size_vector<O, N, T>& x11) {
    sl::fixed_size_vector<O, N, T> result; // = sl::tags::not_initalized();

    for (std::size_t d = 0; d<N; ++d) {
      result[d] = half_cisl_inf(x00[d], x01[d], x10[d], x11[d]);
    }
    return result;
  }
 
  template <enum sl::vector_orientation O, std::size_t N, class T>
  static inline sl::fixed_size_vector<O, N, T> average(const sl::fixed_size_vector<O, N, T>& x00,
						       const sl::fixed_size_vector<O, N, T>& x01,
						       const sl::fixed_size_vector<O, N, T>& x10,
						       const sl::fixed_size_vector<O, N, T>& x11) {
    sl::fixed_size_vector<O, N, T> result; // = sl::tags::not_initalized();

    for (std::size_t d = 0; d<N; ++d) {
      result[d] = average(x00[d], x01[d], x10[d], x11[d]);
    }
    return result;
  }
 

  template <enum sl::vector_orientation O, std::size_t N, class T>
  static inline sl::fixed_size_vector<O, N, T> half_average(const sl::fixed_size_vector<O, N, T>& x00,
							    const sl::fixed_size_vector<O, N, T>& x01,
							    const sl::fixed_size_vector<O, N, T>& x10,
							    const sl::fixed_size_vector<O, N, T>& x11) {
    sl::fixed_size_vector<O, N, T> result; // = sl::tags::not_initalized();

    for (std::size_t d = 0; d<N; ++d) {
      result[d] = half_average(x00[d], x01[d], x10[d], x11[d]);
    }
    return result;
  }

  template <enum sl::vector_orientation O, std::size_t N, class T>
  static inline sl::fixed_size_vector<O, N, T> neville4(const sl::fixed_size_vector<O, N, T>& v00, const sl::fixed_size_vector<O, N, T>& v01, 
							const sl::fixed_size_vector<O, N, T>& v02, const sl::fixed_size_vector<O, N, T>& v03, 
							const sl::fixed_size_vector<O, N, T>& v10, const sl::fixed_size_vector<O, N, T>& v11, 
							const sl::fixed_size_vector<O, N, T>& v12, const sl::fixed_size_vector<O, N, T>& v13, 
							const sl::fixed_size_vector<O, N, T>& v20, const sl::fixed_size_vector<O, N, T>& v21, 
							const sl::fixed_size_vector<O, N, T>& v22, const sl::fixed_size_vector<O, N, T>& v23, 
							const sl::fixed_size_vector<O, N, T>& v30, const sl::fixed_size_vector<O, N, T>& v31, 
							const sl::fixed_size_vector<O, N, T>& v32, const sl::fixed_size_vector<O, N, T>& v33) {
    sl::fixed_size_vector<O, N, T> result; // = sl::tags::not_initalized();

    for (std::size_t d = 0; d<N; ++d) {
      result[d] = neville4(v00[d], v01[d], v02[d], v03[d],
			   v10[d], v11[d], v12[d], v13[d],
			   v20[d], v21[d], v22[d], v23[d],
			   v30[d], v31[d], v32[d], v33[d]);
    }
    return result;
  }


  template <enum sl::vector_orientation O, std::size_t N, class T>
  static inline sl::fixed_size_vector<O, N, T> half_neville4(const sl::fixed_size_vector<O, N, T>& v00, const sl::fixed_size_vector<O, N, T>& v01, 
							     const sl::fixed_size_vector<O, N, T>& v02, const sl::fixed_size_vector<O, N, T>& v03, 
							     const sl::fixed_size_vector<O, N, T>& v10, const sl::fixed_size_vector<O, N, T>& v11, 
							     const sl::fixed_size_vector<O, N, T>& v12, const sl::fixed_size_vector<O, N, T>& v13, 
							     const sl::fixed_size_vector<O, N, T>& v20, const sl::fixed_size_vector<O, N, T>& v21, 
							     const sl::fixed_size_vector<O, N, T>& v22, const sl::fixed_size_vector<O, N, T>& v23, 
							     const sl::fixed_size_vector<O, N, T>& v30, const sl::fixed_size_vector<O, N, T>& v31, 
							     const sl::fixed_size_vector<O, N, T>& v32, const sl::fixed_size_vector<O, N, T>& v33) {
    sl::fixed_size_vector<O, N, T> result; // = sl::tags::not_initalized();

    for (std::size_t d = 0; d<N; ++d) {
      result[d] = half_neville4(v00[d], v01[d], v02[d], v03[d],
				v10[d], v11[d], v12[d], v13[d],
				v20[d], v21[d], v22[d], v23[d],
				v30[d], v31[d], v32[d], v33[d]);
    }
    return result;
  }

  template <enum sl::vector_orientation O, std::size_t N, class T>
  static inline sl::fixed_size_vector<O, N, T> neville6(const sl::fixed_size_vector<O, N, T>& v00, const sl::fixed_size_vector<O, N, T>& v01,
                                                        const sl::fixed_size_vector<O, N, T>& v02, const sl::fixed_size_vector<O, N, T>& v03,
                                                        const sl::fixed_size_vector<O, N, T>& v04, const sl::fixed_size_vector<O, N, T>& v05, 
                                                        const sl::fixed_size_vector<O, N, T>& v10, const sl::fixed_size_vector<O, N, T>& v11,
                                                        const sl::fixed_size_vector<O, N, T>& v12, const sl::fixed_size_vector<O, N, T>& v13,
                                                        const sl::fixed_size_vector<O, N, T>& v14, const sl::fixed_size_vector<O, N, T>& v15,  
                                                        const sl::fixed_size_vector<O, N, T>& v20, const sl::fixed_size_vector<O, N, T>& v21,
                                                        const sl::fixed_size_vector<O, N, T>& v22, const sl::fixed_size_vector<O, N, T>& v23,
                                                        const sl::fixed_size_vector<O, N, T>& v24, const sl::fixed_size_vector<O, N, T>& v25,  
                                                        const sl::fixed_size_vector<O, N, T>& v30, const sl::fixed_size_vector<O, N, T>& v31,
                                                        const sl::fixed_size_vector<O, N, T>& v32, const sl::fixed_size_vector<O, N, T>& v33,
                                                        const sl::fixed_size_vector<O, N, T>& v34, const sl::fixed_size_vector<O, N, T>& v35,
                                                        const sl::fixed_size_vector<O, N, T>& v40, const sl::fixed_size_vector<O, N, T>& v41,
                                                        const sl::fixed_size_vector<O, N, T>& v42, const sl::fixed_size_vector<O, N, T>& v43,
                                                        const sl::fixed_size_vector<O, N, T>& v44, const sl::fixed_size_vector<O, N, T>& v45,
                                                        const sl::fixed_size_vector<O, N, T>& v50, const sl::fixed_size_vector<O, N, T>& v51,
                                                        const sl::fixed_size_vector<O, N, T>& v52, const sl::fixed_size_vector<O, N, T>& v53,
                                                        const sl::fixed_size_vector<O, N, T>& v54, const sl::fixed_size_vector<O, N, T>& v55) {
    sl::fixed_size_vector<O, N, T> result; // = sl::tags::not_initalized();

    for (std::size_t d = 0; d<N; ++d) {
      result[d] = neville6(v00[d], v01[d], v02[d], v03[d], v04[d], v05[d],
			   v10[d], v11[d], v12[d], v13[d], v14[d], v15[d],
			   v20[d], v21[d], v22[d], v23[d], v24[d], v25[d],
			   v30[d], v31[d], v32[d], v33[d], v34[d], v35[d],
			   v40[d], v41[d], v42[d], v43[d], v44[d], v45[d],
			   v50[d], v51[d], v52[d], v53[d], v54[d], v55[d]);
    }
    return result;
  }

  template <enum sl::vector_orientation O, std::size_t N, class T>
  static inline sl::fixed_size_vector<O, N, T> half_neville6(const sl::fixed_size_vector<O, N, T>& v00, const sl::fixed_size_vector<O, N, T>& v01,
                                                             const sl::fixed_size_vector<O, N, T>& v02, const sl::fixed_size_vector<O, N, T>& v03,
                                                             const sl::fixed_size_vector<O, N, T>& v04, const sl::fixed_size_vector<O, N, T>& v05, 
                                                             const sl::fixed_size_vector<O, N, T>& v10, const sl::fixed_size_vector<O, N, T>& v11,
                                                             const sl::fixed_size_vector<O, N, T>& v12, const sl::fixed_size_vector<O, N, T>& v13,
                                                             const sl::fixed_size_vector<O, N, T>& v14, const sl::fixed_size_vector<O, N, T>& v15,  
                                                             const sl::fixed_size_vector<O, N, T>& v20, const sl::fixed_size_vector<O, N, T>& v21,
                                                             const sl::fixed_size_vector<O, N, T>& v22, const sl::fixed_size_vector<O, N, T>& v23,
                                                             const sl::fixed_size_vector<O, N, T>& v24, const sl::fixed_size_vector<O, N, T>& v25,  
                                                             const sl::fixed_size_vector<O, N, T>& v30, const sl::fixed_size_vector<O, N, T>& v31,
                                                             const sl::fixed_size_vector<O, N, T>& v32, const sl::fixed_size_vector<O, N, T>& v33,
                                                             const sl::fixed_size_vector<O, N, T>& v34, const sl::fixed_size_vector<O, N, T>& v35,
                                                             const sl::fixed_size_vector<O, N, T>& v40, const sl::fixed_size_vector<O, N, T>& v41,
                                                             const sl::fixed_size_vector<O, N, T>& v42, const sl::fixed_size_vector<O, N, T>& v43,
                                                             const sl::fixed_size_vector<O, N, T>& v44, const sl::fixed_size_vector<O, N, T>& v45,
                                                             const sl::fixed_size_vector<O, N, T>& v50, const sl::fixed_size_vector<O, N, T>& v51,
                                                             const sl::fixed_size_vector<O, N, T>& v52, const sl::fixed_size_vector<O, N, T>& v53,
                                                             const sl::fixed_size_vector<O, N, T>& v54, const sl::fixed_size_vector<O, N, T>& v55) {
    sl::fixed_size_vector<O, N, T> result; // = sl::tags::not_initalized();

    for (std::size_t d = 0; d<N; ++d) {
      result[d] = half_neville6(v00[d], v01[d], v02[d], v03[d], v04[d], v05[d],
                                v10[d], v11[d], v12[d], v13[d], v14[d], v15[d],
                                v20[d], v21[d], v22[d], v23[d], v24[d], v25[d],
                                v30[d], v31[d], v32[d], v33[d], v34[d], v35[d],
                                v40[d], v41[d], v42[d], v43[d], v44[d], v45[d],
                                v50[d], v51[d], v52[d], v53[d], v54[d], v55[d]);
    }
    return result;
  }

  /**
   * Base class which performs operations needed by delta_codec, and builder
   * over heights and color values.
   * T: type of delta encoded data
   */
  template <class T>
  class diamond_operator {
  public:
    typedef grid_point_t                        diamond_id_t;
    typedef T                                   value_t;
    typedef sl::dense_array<value_t,2,void>     array2_t;
    typedef std::vector<uint8_t>                data_buffer_t;

  public:
    
    static value_t quincunx_predict(const array2_t& P, int i, int j);

    static value_t quincunx_half_update(const array2_t& H, int i, int j);
   
    static void analysis_in(array2_t& L,
			    array2_t& H,
			    const array2_t& P,
			    const array2_t& Q);

    static void synthesis_in(array2_t& P,
			     array2_t& Q,
			     const array2_t& L,
			     const array2_t& H);
  };


  class color_operator : public diamond_operator<delta_color3_t> {
  public:
    typedef diamond_operator<delta_color3_t>	super_t;
    typedef delta_color3_t			value_t;

  public:
    // diamond_repository_storage
    static void compress_to_target_error(const array2_t& delta, data_buffer_t& compressed_delta,
					 float tolerance, sl::array_codec* codec, bool use_amax_error);
    static void compress_lossless(const array2_t& delta, data_buffer_t& compressed_delta,
				  sl::array_codec* codec);
    
    static void decompress_to(array2_t& delta, const uint8_t* data_buf, uint32_t data_buf_size, sl::array_codec* codec);
    
    static double amax_difference(const array2_t& a1, const array2_t& a2);
 
    static double rms(const array2_t& d0, const array2_t& d1);

    static double amean(const array2_t& a1);

    static double amax(const array2_t& a1);

    static double fraction_zero_values(const array2_t& a1, double tolerance);

    static void quantize(array2_t& p, float tolerance);

    static void decorrelate_channels(array2_t& decorrelated, const array2_t& correlated);

    static void recorrelate_channels(array2_t& correlated, const array2_t& decorrelated);

  };
    
  class height_operator : public diamond_operator<int32_t> {
  public:
    typedef int32_t value_t;
    
  public:
    // diamond_repository_storage
    static void compress_to_target_error(const array2_t& delta, data_buffer_t& compressed_delta,
					 float tolerance, sl::array_codec* codec, bool use_amax_error);
    static void compress_lossless(const array2_t& delta, data_buffer_t& compressed_delta,
				  sl::array_codec* codec);

    static void decompress_to(array2_t& delta, const uint8_t* data_buf, uint32_t data_buf_size, sl::array_codec* codec);
    
    static double amax_difference(const array2_t& a1, const array2_t& a2);

    static double rms(const array2_t& d0, const array2_t& d1);

    static double amean(const array2_t& a1);

    static double amax(const array2_t& a1);

    static double fraction_zero_values(const array2_t& a1, double tolerance);

    static void decorrelate_channels(array2_t& decorrelated, const array2_t& correlated);
    static void recorrelate_channels(array2_t& correlated, const array2_t& decorrelated);

  };
  
} // namespace cbdam 

#endif // CBDAM_DIAMOND_OPERATOR_HPP

#ifndef CBDAM_DIAMOND_OPERATOR_IPP
#define CBDAM_DIAMOND_OPERATOR_IPP

namespace cbdam {
  
  ///////////////////////////// diamond_operator /////////////////////////////////

  // Median/cisl_inf wavelet
  template <class T>
  inline T diamond_operator<T>::quincunx_predict(const array2_t& P, int i, int j) {
#if 0
    if (i>1 && i+3<int(P.extent()[0]) && j>1 && j+3<int(P.extent()[1])) {
      return neville6(P(i-2,j-2), P(i-2,j-1), P(i-2,j+0), P(i-2,j+1), P(i-2,j+2), P(i-2,j+3), 
                      P(i-1,j-2), P(i-1,j-1), P(i-1,j+0), P(i-1,j+1), P(i-1,j+2), P(i-1,j+3), 
                      P(i+0,j-2), P(i+0,j-1), P(i+0,j+0), P(i+0,j+1), P(i+0,j+2), P(i+0,j+3), 
                      P(i+1,j-2), P(i+1,j-1), P(i+1,j+0), P(i+1,j+1), P(i+1,j+2), P(i+1,j+3), 
                      P(i+2,j-2), P(i+2,j-1), P(i+2,j+0), P(i+2,j+1), P(i+2,j+2), P(i+2,j+3), 
                      P(i+3,j-2), P(i+3,j-1), P(i+3,j+0), P(i+3,j+1), P(i+3,j+2), P(i+3,j+3));
    } else
#endif
      if (i>0 && i+2<int(P.extent()[0]) && j>0 && j+2<int(P.extent()[1])) {
      return neville4(P(i-1,j-1), P(i-1,j+0), P(i-1,j+1), P(i-1,j+2), 
		      P(i+0,j-1), P(i+0,j+0), P(i+0,j+1), P(i+0,j+2), 
		      P(i+1,j-1), P(i+1,j+0), P(i+1,j+1), P(i+1,j+2), 
		      P(i+2,j-1), P(i+2,j+0), P(i+2,j+1), P(i+2,j+2));      
    } else {
      return average(P(i+0,j+0), P(i+0,j+1), 
		     P(i+1,j+0), P(i+1,j+1));
    }
  }

  template <class T>
  inline T diamond_operator<T>::quincunx_half_update(const array2_t& H, int i, int j) {
#if 0
    if (i>1 && i+1<int(H.extent()[0]) && j>1 && j+1<int(H.extent()[1])) {
      return half_neville4(H(i-2,j-2), H(i-2,j-1), H(i-2,j+0), H(i-2,j+1), 
			   H(i-1,j-2), H(i-1,j-1), H(i-1,j+0), H(i-1,j+1), 
			   H(i+0,j-2), H(i+0,j-1), H(i+0,j+0), H(i+0,j+1), 
			   H(i+1,j-2), H(i+1,j-1), H(i+1,j+0), H(i+1,j+1));      
    } else
#endif
      {
      return half_average(H(i-1,j-1), H(i-1,j-0), 
			  H(i-0,j-1), H(i-0,j-0));
    }
  }

  template <class T>
  inline void diamond_operator<T>::analysis_in(array2_t& L,
							       array2_t& H,
							       const array2_t& P,
							       const array2_t& Q) {
    int h=int(P.extent()[0]);
    int w=int(P.extent()[1]);
   
#if 0
    // FIXME HACK
    // Max/min
    T Q_min = Q(0,0);
    T Q_max = Q(0,0);
    for (int i=0; i<h-1; ++i) {
      for (int j=0; j<w-1; ++j) {
	Q_min = component_min(Q_min, Q(i,j));
	Q_max = component_max(Q_max, Q(i,j));
      }
    }
    // High pass: substract prediction
    for (int i=0; i<h-1; ++i) {
      for (int j=0; j<w-1; ++j) {
        H(i,j) = Q(i,j) - component_max(Q_min, component_min(Q_max, quincunx_predict(P, i, j)));
      }
    }
#else
    for (int i=0; i<h-1; ++i) {
      for (int j=0; j<w-1; ++j) {
        H(i,j) = Q(i,j) - quincunx_predict(P, i, j);
      }
    }
#endif

    // Low pass: half_update
    for (int i=0; i<h; ++i) {
      for (int j=0; j<w; ++j) {
        if ((i==0 || j==0 || i==h-1 || j==w-1)) {
          L(i,j) = P(i,j);
        } else {
          L(i,j) = P(i,j) + quincunx_half_update(H, i, j);
        }
      }
    }
  }

  template <class T>
  inline void diamond_operator<T>::synthesis_in(array2_t& P,
                                                array2_t& Q,
                                                const array2_t& L,
                                                const array2_t& H) {
    int h=int(P.extent()[0]);
    int w=int(P.extent()[1]);
    
    // Low pass
    for (int i=0; i<h; ++i) {
      for (int j=0; j<w; ++j) {
        if ((i==0 || j==0 || i==h-1 || j==w-1)) {
          P(i,j) = L(i,j);
        } else {
          P(i,j) = L(i,j) - quincunx_half_update(H,i,j);
        }
      }
    }

    // High pass
    for (int i=0; i<h-1; ++i) {
      for (int j=0; j<w-1; ++j) {
        Q(i,j) = H(i,j) + quincunx_predict(P, i, j);
      }
    }
  }


  ///////////////////////////////////////////////////////////////////////////////////
  // height_operator

#if 0
  inline void height_operator::set_root_value_in(diamond_data_t& p, const array2_t& offset, int y, int x, int patch_id, int patch_dim) {
    int yl = sl::max(0, y-1);
    int yh = sl::min(patch_dim, y+1);
    int xl = sl::max(0, x-1);
    int xh = sl::min(patch_dim, x+1);

    // h
    p[0] = offset(y,x);

    // dhdu, dhdv
    if (patch_id == 0) {
      p[1] = (offset(y, xh) - offset(y, xl)) / 2;        
      p[2] = (offset(yh, x) - offset(yl, x)) / 2;
    } else {
      p[1] = -(offset(y, xh) - offset(y, xl)) / 2;
      p[2] = -(offset(yh, x) - offset(yl, x)) / 2;
    }
  }
  
  inline void height_operator::rotate_normal_for_child00(diamond_data_t& p, bool border_point) {
    if (border_point) {
      // boundary point rotate normal from original patch uv of +- 135 deg + scale 1/sqrt(2)
      diamond_data_t::value_t u = (-p[1] + p[2]) / 2; 
      p[2] = (-p[1] - p[2]) / 2; 
      p[1] = u;
    } else {
      // inner point rotate computed normal of  -90 from south -> west
      diamond_data_t::value_t u = p[1];
      p[1] = p[2];
      p[2] = -u;
    }
    
  }
  
  inline void height_operator::rotate_normal_for_child01(diamond_data_t& p, bool border_point) {
    if (border_point) {
      // boundary point rotate normal from original patch uv of +- 135 deg + scale 1/sqrt(2)
      diamond_data_t::value_t u = (-p[1] - p[2]) / 2; 
      p[2] = ( p[1] - p[2]) / 2; 
      p[1] =  u;
    } else {
      // inner point rotate computed normal of  180 from south -> north
      p[1] = -p[1];
      p[2] = -p[2];
    }
  }
  
  inline void height_operator::rotate_normal_for_child10(diamond_data_t& p, bool border_point) {
    if (border_point) {
      // boundary point rotate normal from original patch uv of +- 135 deg + scale 1/sqrt(2)
      diamond_data_t::value_t u = (-p[1] + p[2]) / 2; 
      p[2] = (-p[1] - p[2]) / 2; 
      p[1] = u;
    } else {
      // inner point rotate computed normal of 90 from south -> east
      diamond_data_t::value_t u = p[1];
      p[1] = -p[2];
      p[2] = u;
    }
  }

  inline void height_operator::rotate_normal_for_child11(diamond_data_t& p, bool border_point) {
    if (border_point) {
      // boundary point rotate normal from original patch uv of +- 135 deg + scale 1/sqrt(2)
      diamond_data_t::value_t u = (-p[1] - p[2]) / 2; 
      p[2] = ( p[1] - p[2]) / 2; 
      p[1] =  u;
    }
    // inner point rotate computed normal of 0 from south -> south, remains unchanged
  }
#endif
  
} // namespace cbdam 

#endif // CBDAM_DIAMOND_OPERATOR_IPP
