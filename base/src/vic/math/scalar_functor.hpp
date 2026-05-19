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
#ifndef VIC_MATH_SCALAR_FUNCTOR_HPP
#define VIC_MATH_SCALAR_FUNCTOR_HPP

#include <sl/config.hpp>

namespace vic {

  namespace math {

    /** 
     *  A scalar function of n parameters, typically
     *  used as objective for numerical optimization.
     */
    template <class T_RET, class T_ARG>
    class scalar_functor {
    public:
      typedef T_RET value_t;
      typedef T_ARG arg_value_t;
    protected:
      std::size_t arg_dim_;
    public:
      
      scalar_functor(std::size_t arg_dim): arg_dim_(arg_dim) {
      }
      
      virtual ~scalar_functor() {
      }
      
      std::size_t arg_dimension() const {
	return arg_dim_;
      }
      
      virtual value_t operator()(const arg_value_t* x) const = 0;
      
    };

  } // namespace math
} // namespace vic

#endif
