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
#ifndef VIC_MATH_SCALAR_FUNCTOR_SOLVER_HPP
#define VIC_MATH_SCALAR_FUNCTOR_SOLVER_HPP

#include <vic/math/scalar_functor.hpp>
#include <vector>
#include <cassert>

namespace vic {

  namespace math {

    /**
     *  Root class for (iterative) root finding or
     *  minimization
     */
    template <class T_RET, class T_ARG>
    class scalar_functor_solver {
    public:
      typedef T_RET value_t;
      typedef T_ARG arg_value_t;
      typedef scalar_functor<value_t, arg_value_t> objective_function_t;
    protected:
      objective_function_t*     objective_function_;
      std::size_t               step_count_;
      std::size_t               max_step_count_;

    protected:
      std::vector<arg_value_t>  best_argument_;
      value_t                   best_value_;
    public:
    
      scalar_functor_solver(objective_function_t* objective_function = 0) {
	objective_function_ = objective_function;
	step_count_ = 0;
	max_step_count_ = 1;

	best_argument_.clear();
	best_value_ = 0;
      }

      virtual ~scalar_functor_solver() {
	objective_function_ = 0;
	step_count_ = 0;
	max_step_count_ = 0;

	best_argument_.clear();
	best_value_ = 0.0;
      }      

      virtual void set_objective_function(objective_function_t* f) {
	objective_function_ = f;
	best_argument_.resize(arg_dimension());
	step_count_ = 0; // will force a restart
      }

      const objective_function_t* objective_function() const {
	return objective_function_;
      }

      std::size_t arg_dimension() const {
	return ((objective_function_) ? objective_function_->arg_dimension() : 0);
      }

      std::size_t step_count() const {
	return step_count_;
      }

      std::size_t max_step_count() const {
	return max_step_count_;
      }

      void set_max_step_count(std::size_t x) {
	max_step_count_ = x;
      }

      inline value_t fn(const arg_value_t* x) const {
	assert(objective_function_);
	return (*objective_function_)(x);
      }

      // Restart
      virtual void restart() {
	step_count_ = 0;
	best_value_ = 0.0;
	best_argument_.resize(arg_dimension());
      }

      // Complete 
      virtual void solve() {
	while (!stop_solving()) {
	  solve_step();
	}
      }

      // Perform a single optimization step, without restarting
      virtual void solve_step() {
	if (step_count_ == 0) restart(); // First step
	++step_count_;
      }

      virtual bool stop_solving() const {
	return step_count()>max_step_count();
      }

      virtual const value_t& best_value() const { 
	return best_value_; 
      }

      virtual const arg_value_t* best_argument() const { 
	return &(best_argument_[0]); 
      }

    };

  } // namespace math
} // namespace vic

#endif
