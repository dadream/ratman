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
#ifndef VIC_MATH_NELDER_MEAD_MINIMIZER_HPP
#define VIC_MATH_NELDER_MEAD_MINIMIZER_HPP

#include <sl/math.hpp>
#include <vic/math/scalar_functor_solver.hpp>

#include <algorithm>
#include <cmath>
#include <iostream>
#include <vector>

namespace vic {

  namespace math {

    template <class T_RET, class T_ARG>
    class nelder_mead_minimizer: public scalar_functor_solver<T_RET,T_ARG> {
    public:
      typedef scalar_functor_solver<T_RET,T_ARG>          super_t;
      typedef typename super_t::value_t              value_t;
      typedef typename super_t::arg_value_t          arg_value_t;
      typedef typename super_t::objective_function_t objective_function_t;
    protected:

      std::vector<arg_value_t> arg_start_;

      std::vector<value_t>                   simplex_energy_;
      std::vector<std::vector<arg_value_t> > simplex_;
      std::vector<arg_value_t>               simplex_sum_;

    public:
    
      nelder_mead_minimizer() {

      }

      virtual ~nelder_mead_minimizer() {

      }

      void set_starting_guess(const arg_value_t* x ) {
	std::size_t N = this->arg_dimension();
	arg_start_.resize(N);
	std::copy(x, x+N, arg_start_.begin());

	this->step_count_ = 0; // force restart
      }

      virtual void restart() {
	const std::size_t N = this->arg_dimension();

	super_t::restart();
 
	// Check start guess
	if (arg_start_.size() != N) {
	  // ???
	  SL_TRACE_OUT(-1) << "START GUESS NOT SET!" << arg_start_.size() << " vs. " << N << std::endl;
	  arg_start_.resize(N);
	  for (std::size_t j=0; j < N; j++) {
	    arg_start_[j] = 0.0;
	  }
	}
	std::cerr << "RESTART[" << "GUESS" << "]: (";
	for (std::size_t j=1; j <= N; j++) {
	  std::cerr << " " << arg_start_[j-1];
	}
	std::cerr << ")" << std::endl;

	// Init simplex to canonical basis with
	// center on start (slightly perturbed)
	this->best_value_ = sl::scalar_math<value_t>::finite_upper_bound();
	simplex_energy_.resize(N+1);

	simplex_.clear();
	simplex_.resize(N+1);
	for (std::size_t i=1; i <= N+1; ++i) {
	  simplex_[i-1].resize(N);
	  for (std::size_t j=1; j <= N; j++) {
	    simplex_[i-1][j-1] = arg_start_[j-1];
	  }
	  if (i>1) {
	    simplex_[i-1][i-2] += 1.0 + 0.01*(i); // FIXME
	  }
	  simplex_energy_[i-1] = this->fn(&(simplex_[i-1][0]));

	  std::cerr << "RESTART[" << i << "]: (";
	  for (std::size_t j=1; j <= N; j++) {
	    std::cerr << " " << simplex_[i-1][j-1];
	  }
	  std::cerr << "): E=" << simplex_energy_[i-1] << std::endl;

	  // Update best guess
	  if (simplex_energy_[i-1]<this->best_value_) {
	    this->best_value_ = simplex_energy_[i-1];
	    std::copy(simplex_[i-1].begin(), simplex_[i-1].end(), (this->best_argument_).begin());
	    std::cerr << "[" << "RESTART:" << i-1 << "]: E = " << this->best_value_ << std::endl;
	  }
	}
    
	// Init simplex sum
	simplex_sum_.resize(N+1);
	update_simplex_sum();
      }

      virtual void solve_step() {
	if (this->step_count_ == 0) this->restart(); // First step

	simplex_step();

	// Update current solution
	if (simplex_energy_[0] < this->best_value_) {
	  this->best_value_ = simplex_energy_[0];
	  std::copy(simplex_[0].begin(), simplex_[0].end(), this->best_argument_.begin());
	
	  std::cerr << "[" << this->step_count_ << "]: E = " << this->best_value_ << std::endl;
	}

	++(this->step_count_);
      }

    protected: // Numerical recipes port...

      void update_simplex_sum() {
	const std::size_t N = this->arg_dimension();
	for (std::size_t j=1;j<=N;j++) {
	  arg_value_t ssum = 0.0;
	  for (std::size_t i=1; i<=N+1;i++) {
	    ssum += simplex_[i-1][j-1]; 
	  }
	  simplex_sum_[j-1]=ssum; 
	}
      }

      void simplex_step() {
	std::size_t N = this->arg_dimension();

	std::size_t ilo = 1;
	std::size_t ihi = (simplex_energy_[1-1] > simplex_energy_[2-1]) ? (1) : (2);
	std::size_t inhi= (simplex_energy_[1-1] > simplex_energy_[2-1]) ? (2) : (1);
	for (std::size_t i = 1; i <= N+1; ++i) {
	  if (simplex_energy_[i-1] < simplex_energy_[ilo-1]) {
	    ilo = i;
	  }
	  if (simplex_energy_[i-1] > simplex_energy_[ihi-1]) {
	    inhi = ihi;
	    ihi = i;
	  } else if (simplex_energy_[i-1] > simplex_energy_[inhi-1]) {
	    if (i != ihi) inhi = i;
	  }
	}


	value_t rtol = 
	  2.0*
	  std::abs(simplex_energy_[ihi-1]-simplex_energy_[ilo-1])/
	  (std::abs(simplex_energy_[ihi-1])+fabs(simplex_energy_[ilo-1]));

	if (rtol < 1e-6/*FIXME ftol */) { 
	  // DONE - Tolerance reached
	} else {
	  value_t ytry = simplex_try(ihi, -1.0);
	  if (ytry <= simplex_energy_[ilo-1]) {
	    ytry = simplex_try(ihi, 0.5);
	  } else if (ytry >= simplex_energy_[inhi-1]) {
	    value_t ysave = simplex_energy_[ihi-1];
	    ytry = simplex_try(ihi,2.0);
	    if (ytry >= ysave) {
	      for (std::size_t i = 1; i <= N+1; i++) {
		if (i != ilo) {
		  for (std::size_t j = 1; j <= N; j++) {
		    simplex_sum_[j-1] = 0.5*(simplex_[i-1][j-1]+simplex_[ilo-1][j-1]);
		    simplex_[i-1][j-1] = simplex_sum_[j-1];
		  }
		  simplex_energy_[i-1] = this->fn(&(simplex_sum_[0])); // offset
		}
	      }
	      update_simplex_sum();
	    }
	  }
	}
      }

      value_t simplex_try(int ihi, arg_value_t fac) {
	std::size_t N = this->arg_dimension();

	std::vector<arg_value_t> ptry(N);

	arg_value_t fac1 = (1.0-fac)/arg_value_t(N);
	arg_value_t fac2 = fac1 - fac;
	for (std::size_t j = 1; j <= N; j++) {
	  ptry[j-1] = simplex_sum_[j-1]*fac1-simplex_[ihi-1][j-1]*fac2;
	}
	value_t ytry = this->fn(&(ptry[0]));
	if (ytry < simplex_energy_[ihi-1]) {
	  simplex_energy_[ihi-1] = ytry;
	  for (std::size_t j = 1; j <= N; j++) {
	    simplex_sum_[j-1] += ptry[j-1]-simplex_[ihi-1][j-1];
	    simplex_[ihi-1][j-1] = ptry[j-1];
	  }
	}
	return ytry;
      }
    };

  } // namespace math
} // namespace vic


#endif
