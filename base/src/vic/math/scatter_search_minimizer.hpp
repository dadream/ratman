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
#ifndef VIC_MATH_SCATTER_SEARCH_MINIMIZER_HPP
#define VIC_MATH_SCATTER_SEARCH_MINIMIZER_HPP

#include <sl/math.hpp>
#include <vic/math/scalar_functor_solver.hpp>

#include <vic/math/SS.h> // FIXME Remove 

namespace vic {

  namespace math {

    template <class T_RET, class T_ARG>
    class scatter_search_minimizer: public scalar_functor_solver<T_RET,T_ARG> {
    public:
      typedef scatter_search_minimizer<T_RET,T_ARG>  this_t;
      typedef scalar_functor_solver<T_RET,T_ARG>     super_t;
      typedef typename super_t::value_t              value_t;
      typedef typename super_t::arg_value_t          arg_value_t;
      typedef typename super_t::objective_function_t objective_function_t;
    protected:
      
      SS* ss_;

      std::vector<arg_value_t> arg_min_;
      std::vector<arg_value_t> arg_max_;
      
      bool        is_local_optimization_enabled_;
      std::size_t diversity_set_count_;
      std::size_t quality_set_count_;
      std::size_t diversificator_set_count_;

      std::vector<arg_value_t> arg_trial_;

    protected:
      
      static double SS_evaluate(void *userdata, double* x) {
	this_t* self = static_cast<this_t*>(userdata);
	assert(self);
	
	// Convert from 1-offset vectors and cast to right type
	const std::size_t N = self->arg_dimension();
	std::vector<arg_value_t>& xx = self->arg_trial_;
	for (std::size_t i=0; i<N; ++i) {
	  xx[i] = static_cast<arg_value_t>(x[i+1]);
	}
	value_t result = self->fn(&(xx[0]));
	return static_cast<double>(result);
      }
      
    public:
      
      scatter_search_minimizer() {
	ss_ = 0;
	is_local_optimization_enabled_ = true;
	diversity_set_count_ = 10;
	quality_set_count_ = 10;
	diversificator_set_count_ = 100;
	
	set_defaults();
      }
      
      ~scatter_search_minimizer() {
	if (ss_) { SSdelete(ss_); ss_ = 0; }
      }
      
      void set_arg_bounds(const arg_value_t argument_min[],
			const arg_value_t argument_max[]) {
	std::size_t N = this->arg_dimension();
	arg_min_.resize(N);
	arg_max_.resize(N);
	
	std::copy(argument_min, argument_min+N, arg_min_.begin());
	std::copy(argument_max, argument_max+N, arg_max_.begin());
	
	if (ss_) { SSdelete(ss_); ss_ = 0; }
	this->step_count_ = 0; // force restart
      }

      void set_diversity_set_count(std::size_t x) {
	diversity_set_count_ = x;
	
	if (ss_) { SSdelete(ss_); ss_ = 0; }
	this->step_count_ = 0; // force restart
      }

      std::size_t diversity_set_count() const {
	return diversity_set_count_;
      }

      void set_quality_set_count(std::size_t x) {
	quality_set_count_ = x;
	
	if (ss_) { SSdelete(ss_); ss_ = 0; }
	this->step_count_ = 0; // force restart
      }

      std::size_t quality_set_count() const {
	return quality_set_count_;
      }

      std::size_t reference_set_count() const {
	return diversity_set_count()+quality_set_count();
      }

      void set_diversificator_set_count(std::size_t x) {
	diversificator_set_count_ = x;
	
	if (ss_) { SSdelete(ss_); ss_ = 0; }
	this->step_count_ = 0; // force restart
      }

      std::size_t diversificator_set_count() const {
	return diversificator_set_count_;
      }

      void set_is_local_optimization_enabled(bool x) {
	is_local_optimization_enabled_ = x;
      }

      bool is_local_optimization_enabled() const {
	return is_local_optimization_enabled_;
      }

      void set_defaults() {
	set_is_local_optimization_enabled(true);
	set_diversity_set_count(10);
	set_quality_set_count(10);
	set_diversificator_set_count(std::max(std::size_t(100),5*reference_set_count()));

	this->set_max_step_count(20); // FIXME
      }

      void restart() {
	const std::size_t N = this->arg_dimension();

	super_t::restart();

	if (ss_) { SSdelete(ss_); ss_ = 0; }
	
	// Check bounds
	if (arg_min_.size() != N) {
	  // ???
	  SL_TRACE_OUT(-1) << "BOUNDS NOT SET!" << arg_min_.size() << " vs. " << N << std::endl;
	  arg_min_.resize(N);
	  arg_max_.resize(N);
	  for (std::size_t j=0; j < N; j++) {
	    arg_min_[j] = -1E10; 
	    arg_max_[j] = 1E10;
	  }
	}
	
	// Allocate space for communicating arguments
	arg_trial_.resize(N);
	
	// Allocate new solver
	ss_ = SSnew(this, SS_evaluate,
		    N, 
		    quality_set_count(),
		    diversity_set_count(),
		    diversificator_set_count(),
		    is_local_optimization_enabled());

	// Assign variable bounds 
	for(std::size_t i=1;i<=N;i++) {
	  ss_->low[i]  = static_cast<double>(arg_min_[i-1]);
	  ss_->high[i] = static_cast<double>(arg_max_[i-1]);
	}
	
	// Build Reference Set 
	SSInitiate_RefSet(ss_);

	// Init best guess
	this->best_value_ = sl::scalar_math<value_t>::finite_upper_bound();
      }

      virtual void solve_step() {
	if (this->step_count_ == 0) this->restart(); // First step
	
	// Evolve to next generation
	if (ss_->new_elements) {
	  SSCombine_RefSet(ss_);
	} else {	
	  SSUpdate_RefSet2(ss_);
	  SSCombine_RefSet(ss_);
	}
	
	// Update best solution so far
	const std::size_t N = this->arg_dimension();
	
	this->best_value_ = static_cast<value_t>(ss_->value1[ss_->order1[1]]);
	for(std::size_t i=1;i<=N;++i) {
	  this->best_argument_[i-1] = ss_->RefSet1[ss_->order1[1]][i];
	}

	SL_TRACE_OUT(-1) << "[" << this->step_count_ << "]: E = " << this->best_value_ << std::endl;
	
	// Go to next step
	++(this->step_count_);
      }

    };

  }
}

#endif
