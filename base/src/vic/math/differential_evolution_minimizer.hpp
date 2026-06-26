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
#ifndef VIC_MATH_DIFFERENTIAL_EVOLUTION_MINIMIZER_HPP
#define VIC_MATH_DIFFERENTIAL_EVOLUTION_MINIMIZER_HPP

#include <sl/random.hpp>
#include <sl/math.hpp>
#include <vic/math/scalar_functor_solver.hpp>

#include <algorithm>
#include <iostream>
#include <vector>

namespace vic {

  namespace math {
    template <class T_RET, class T_ARG>
    class differential_evolution_minimizer: public scalar_functor_solver<T_RET,T_ARG> {
    public:
      typedef scalar_functor_solver<T_RET,T_ARG>          super_t;
      typedef typename super_t::value_t              value_t;
      typedef typename super_t::arg_value_t          arg_value_t;
      typedef typename super_t::objective_function_t objective_function_t;
      
    public:
      typedef enum {
	stBest1Exp = 0,
	stRand1Exp = 1,
	stRandToBest1Exp = 2,
	stBest2Exp = 3,
	stRand2Exp = 4,
	stBest1Bin = 5,
	stRand1Bin = 6,
	stRandToBest1Bin = 7,
	stBest2Bin = 8,
	stRand2Bin = 9
      } mutation_strategy_t;

    protected:
      mutable sl::random::uniform<value_t> rng_;
    protected:
  
      mutation_strategy_t  mutation_strategy_;
      arg_value_t          mutation_scale_;
      arg_value_t          crossover_probability_;
      std::size_t          population_count_;


      std::vector<arg_value_t>                arg_min_;
      std::vector<arg_value_t>                arg_max_;

      bool                                    are_bound_constrains_enabled_;

      std::vector<value_t>                    population_energy_;
      std::vector< std::vector<arg_value_t> > population_;

    protected:

      value_t random_uniform(value_t minValue, value_t maxValue) const {
	return minValue + rng_.value()*(maxValue-minValue);
      }

    public:

      differential_evolution_minimizer() {
	mutation_strategy_     = stRand1Exp;
	mutation_scale_        = 0.8;
	crossover_probability_ = 0.9;
	population_count_      = 8;

	are_bound_constrains_enabled_ = true; // FIXME?
	arg_min_.clear();
	arg_max_.clear();

	population_energy_.clear();
	population_.clear();
      }

      ~differential_evolution_minimizer() {
	mutation_strategy_     = stRand1Exp;
	mutation_scale_        = 0.0;
	crossover_probability_ = 0.0;
	population_count_      = 8;

	arg_min_.clear();
	arg_max_.clear();

	population_energy_.clear();
	population_.clear();
      }

      bool are_bound_constrains_enabled() const {
	return are_bound_constrains_enabled_;
      }

      void set_are_bound_constrains_enabled(bool x) {
	are_bound_constrains_enabled_ = x;
	this->step_count_ = 0; // force restart
      }
      
      void set_arg_bounds(const arg_value_t argument_min[],
			  const arg_value_t argument_max[]) {
	std::size_t N = this->arg_dimension();
	arg_min_.resize(N);
	arg_max_.resize(N);

	std::copy(argument_min, argument_min+N, arg_min_.begin());
	std::copy(argument_max, argument_max+N, arg_max_.begin());

	this->step_count_ = 0; // force restart
      }

      void set_mutation_strategy(mutation_strategy_t x) {
	mutation_strategy_ = x; 

	this->step_count_ = 0;  // force restart
      }

      mutation_strategy_t mutation_strategy() const {
	return mutation_strategy_;
      }
 
      void set_mutation_scale(const arg_value_t& x) {
	mutation_scale_ = x; 

	this->step_count_ = 0;  // force restart
      }

      const arg_value_t& mutation_scale() const {
	return mutation_scale_;
      }

      void set_crossover_probability(const arg_value_t& x) {
	crossover_probability_ = x; 

	this->step_count_ = 0;  // force restart
      }

      const arg_value_t& crossover_probability() const {
	return crossover_probability_;
      }

      void set_population_count(std::size_t sz) {
	population_count_ = std::max(std::size_t(8), sz);

	this->step_count_ = 0;  // force restart
      }

      std::size_t population_count() const  {
	return population_count_;
      }

      void set_defaults() {
	set_mutation_strategy(stRand1Exp);
	set_population_count(16*this->arg_dimension());
	set_mutation_scale(0.7);
	set_crossover_probability(0.9);
	this->set_max_step_count(8*population_count());
      }

      virtual void restart() {
	const std::size_t N = this->arg_dimension();

	super_t::restart();
 
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
      
	// Init population
	population_energy_.resize(population_count_);
	population_.clear();
	population_.resize(population_count_);
	for (std::size_t i=0; i < population_count_; ++i) {
	  population_[i].resize(N);
	  for (std::size_t j=0; j < N; j++) {
	    population_[i][j] = random_uniform(arg_min_[j],
					       arg_max_[j]);
	  }
	  population_energy_[i] = sl::scalar_math<value_t>::finite_upper_bound();
	}

	// Init best guess
	this->best_value_ = sl::scalar_math<value_t>::finite_upper_bound();
	for (std::size_t j=0; j < N; ++j) { 
	  // Must start in a valid place for some mutation strategies
	  this->best_argument_[j] = random_uniform(arg_min_[j], arg_max_[j]); 
	}
      }

      virtual void solve_step() {
	if (this->step_count_ == 0) this->restart(); // First step

	// Evolve to next generation
	std::size_t N = this->arg_dimension();
	std::vector<arg_value_t> trial_solution(N);
	value_t                  trial_energy;

	for (std::size_t candidate = 0;
	     candidate < population_count_; 
	     ++candidate) {
	  generate_candidate(candidate, trial_solution);
	  trial_energy = this->fn(&(trial_solution[0]));

	  if (trial_energy < population_energy_[candidate]) {
	    // New minimum for this candidate
	    population_energy_[candidate] = trial_energy;
	    std::copy(trial_solution.begin(), trial_solution.end(), population_[candidate].begin());

	    // Check if current global minimum
	    if (trial_energy < this->best_value_) {
	      this->best_value_ = trial_energy;
	      std::copy(trial_solution.begin(), trial_solution.end(), this->best_argument_.begin());

	      std::cerr << "[" << this->step_count_ << "]: E = " << this->best_value_ << std::endl;
	    }
	  }
	}
     
	++(this->step_count_);
      }

    protected: // Generation of candidates

      void select_samples(std::size_t candidate,
			  std::size_t *r1,
			  std::size_t *r2=0,
			  std::size_t *r3=0,
			  std::size_t *r4=0,
			  std::size_t *r5=0) const {
	if (r1) {
	  do {
	    *r1 = (std::size_t)random_uniform(0.0,(double)population_count_);
	  } while (*r1 == candidate);
	}

	if (r2) {
	  do {
	    *r2 = (std::size_t)random_uniform(0.0,(double)population_count_);
	  }
	  while ((*r2 == candidate) || (*r2 == *r1));
	}

	if (r3) {
	  do {
	    *r3 = (std::size_t)random_uniform(0.0,(double)population_count_);
	  } while ((*r3 == candidate) || (*r3 == *r2) || (*r3 == *r1));
	}

	if (r4) {
	  do {
	    *r4 = (std::size_t)random_uniform(0.0,(double)population_count_);
	  } while ((*r4 == candidate) || (*r4 == *r3) || (*r4 == *r2) || (*r4 == *r1));
	}
      }

      inline void handle_bounds(std::vector<arg_value_t>& trial_solution, 
				std::size_t j,
				const arg_value_t& mutation_origin_j) {
	if (are_bound_constrains_enabled()) {
	  if (trial_solution[j] < arg_min_[j]) {
	    trial_solution[j] = random_uniform(arg_min_[j],mutation_origin_j);
	  } else if (trial_solution[j] > arg_max_[j]) {
	    trial_solution[j] = random_uniform(mutation_origin_j, arg_max_[j]);
	  }
	}
      }

      void generate_candidate(std::size_t candidate,
			      std::vector<arg_value_t>& trial_solution) {
	// Mutate candidate and store it in trial solution
	switch (mutation_strategy_) {
	case stBest1Exp      : Best1Exp(candidate, trial_solution);       break;
	case stRand1Exp      : Rand1Exp(candidate, trial_solution);       break;
	case stRandToBest1Exp: RandToBest1Exp(candidate, trial_solution); break;
	case stBest2Exp      : Best2Exp(candidate, trial_solution);       break;
	case stRand2Exp      : Rand2Exp(candidate, trial_solution);       break;
	case stBest1Bin      : Best1Bin(candidate, trial_solution);       break;	  
	case stRand1Bin      : Rand1Bin(candidate, trial_solution);       break;	  
	case stRandToBest1Bin: RandToBest1Bin(candidate, trial_solution); break;    
	case stBest2Bin      : Best2Bin(candidate, trial_solution);       break;    
	case stRand2Bin      : Rand2Bin(candidate, trial_solution);       break;
	default              : Best1Exp(candidate, trial_solution);       break;
	}
      }

      void Best1Exp(std::size_t candidate,
		    std::vector<arg_value_t>& trial_solution) {
	const std::size_t N = this->arg_dimension();

	std::size_t r1, r2;
	std::size_t n;
      
	select_samples(candidate,&r1,&r2);
	n = (std::size_t)random_uniform(0.0,(double)N);

	for (std::size_t i=0; (random_uniform(0.0,1.0) < crossover_probability_) && (i < N); i++) {
	  trial_solution[n] = 
	    (this->best_argument_)[n]
	    + mutation_scale_ * (population_[r1][n] - population_[r2][n]);
	  handle_bounds(trial_solution, n, (this->best_argument_)[n]);
	  n = (n + 1) % N;
	}
      }
    
      void Rand1Exp(std::size_t candidate,
		    std::vector<arg_value_t>& trial_solution) {
	const std::size_t N = this->arg_dimension();

	std::size_t r1, r2, r3;
	std::size_t n;
      
	select_samples(candidate,&r1,&r2,&r3);
	n = (std::size_t)random_uniform(0.0,(double)N);

	for (std::size_t i=0; (random_uniform(0.0,1.0) < crossover_probability_) && (i < N); i++) {
	  trial_solution[n] = 
	    population_[r1][n]
	    + mutation_scale_ * (population_[r2][n] - population_[r3][n]);
	  handle_bounds(trial_solution, n, population_[r1][n]);
	  n = (n + 1) % N;
	}
      }
    
      void RandToBest1Exp(std::size_t candidate,
			  std::vector<arg_value_t>& trial_solution) {
	const std::size_t N = this->arg_dimension();

	std::size_t r1, r2;
	std::size_t n;
      
	select_samples(candidate,&r1,&r2);
	n = (std::size_t)random_uniform(0.0,(double)N);

	for (std::size_t i=0; (random_uniform(0.0,1.0) < crossover_probability_) && (i < N); i++) {
	  trial_solution[n] = 
	    population_[candidate][n] 
	    + mutation_scale_ * ((this->best_argument_)[n] - trial_solution[n])
	    + mutation_scale_ * (population_[r1][n] - population_[r2][n]);
	  handle_bounds(trial_solution, n, population_[candidate][n]);
	  n = (n + 1) % N;
	}
      }
    
      void Best2Exp(std::size_t candidate,
		    std::vector<arg_value_t>& trial_solution) {
	const std::size_t N = this->arg_dimension();

	std::size_t r1, r2, r3, r4;
	std::size_t n;
      
	select_samples(candidate,&r1,&r2,&r3,&r4);
	n = (std::size_t)random_uniform(0.0,(double)N);

	for (std::size_t i=0; (random_uniform(0.0,1.0) < crossover_probability_) && (i < N); i++) {
	  trial_solution[n] = 
	    (this->best_argument_)[n] 
	    + mutation_scale_ * (population_[r1][n] - population_[r3][n])
	    + mutation_scale_ * (population_[r2][n] - population_[r4][n]);
	  handle_bounds(trial_solution, n, (this->best_argument_)[n]);
	  n = (n + 1) % N;
	}
      }
    
      void Rand2Exp(std::size_t candidate,
		    std::vector<arg_value_t>& trial_solution) {
	const std::size_t N = this->arg_dimension();

	std::size_t r1, r2, r3, r4, r5;
	std::size_t n;
      
	select_samples(candidate,&r1,&r2,&r3,&r4,&r5);
	n = (std::size_t)random_uniform(0.0,(double)N);

	for (std::size_t i=0; (random_uniform(0.0,1.0) < crossover_probability_) && (i < N); i++) {
	  trial_solution[n] = 
	    population_[r1][n]
	    + mutation_scale_ * (population_[r2][n] - population_[r4][n])
	    + mutation_scale_ * (population_[r3][n] - population_[r5][n]);
	  handle_bounds(trial_solution, n, population_[r1][n]);
	  n = (n + 1) % N;
	}
      }

      void Best1Bin(std::size_t candidate,
		    std::vector<arg_value_t>& trial_solution) {
	const std::size_t N = this->arg_dimension();

	std::size_t r1, r2;
	std::size_t n;
      
	select_samples(candidate,&r1,&r2);
	n = (std::size_t)random_uniform(0.0,(double)N);

	for (std::size_t i=0; i < N; i++) {
	  if ((random_uniform(0.0,1.0) < crossover_probability_) || (i == (N - 1))) {
	    trial_solution[n] = 
	      (this->best_argument_)[n] + 
	      mutation_scale_ * (population_[r1][n] - population_[r2][n]);
	    handle_bounds(trial_solution, n, (this->best_argument_)[n]);
	  }
	  n = (n + 1) % N;
	}
      }

      void Rand1Bin(std::size_t candidate,
		    std::vector<arg_value_t>& trial_solution) {
	const std::size_t N = this->arg_dimension();

	std::size_t r1, r2, r3;
	std::size_t n;
      
	select_samples(candidate,&r1,&r2,&r3);
	n = (std::size_t)random_uniform(0.0,(double)N);

	for (std::size_t i=0; i < N; i++)  {
	  if ((random_uniform(0.0,1.0) < crossover_probability_) || (i  == (N - 1))) {
	    trial_solution[n] = population_[r1][n]
	      + mutation_scale_ * (population_[r2][n] - population_[r3][n]);
	    handle_bounds(trial_solution, n, population_[r1][n]);
	  }
	  n = (n + 1) % N;
	}
      }

      void RandToBest1Bin(std::size_t candidate,
			  std::vector<arg_value_t>& trial_solution) {
	const std::size_t N = this->arg_dimension();

	std::size_t r1, r2;
	std::size_t n;
      
	select_samples(candidate,&r1,&r2);
	n = (std::size_t)random_uniform(0.0,(double)N);

	for (std::size_t i=0; i < N; i++) {
	  if ((random_uniform(0.0,1.0) < crossover_probability_) || (i  == (N - 1))) {
	    trial_solution[n] = 
	      population_[candidate][n]
	      + mutation_scale_ * ((this->best_argument_)[n] - trial_solution[n]) 
	      + mutation_scale_ * (population_[r1][n] - population_[r2][n]);
	    handle_bounds(trial_solution, n, population_[candidate][n]);
	  }
	  n = (n + 1) % N;
	}
      }

      void Best2Bin(std::size_t candidate,
		    std::vector<arg_value_t>& trial_solution) {
	const std::size_t N = this->arg_dimension();

	std::size_t r1, r2, r3, r4;
	std::size_t n;
      
	select_samples(candidate,&r1,&r2,&r3,&r4);
	n = (std::size_t)random_uniform(0.0,(double)N);
 
	for (std::size_t i=0; i < N; i++) {
	  if ((random_uniform(0.0,1.0) < crossover_probability_) || (i  == (N - 1))) {
	    trial_solution[n] = 
	      (this->best_argument_)[n]
	      + mutation_scale_ * (population_[r1][n] - population_[r3][n])
	      + mutation_scale_ * (population_[r2][n] - population_[r4][n]);
	    handle_bounds(trial_solution, n, (this->best_argument_)[n]);
	  }
	  n = (n + 1) % N;
	}
      }

      void Rand2Bin(std::size_t candidate,
		    std::vector<arg_value_t>& trial_solution) {
	const std::size_t N = this->arg_dimension();

	std::size_t r1, r2, r3, r4, r5;
	std::size_t n;
      
	select_samples(candidate,&r1,&r2,&r3,&r4,&r5);
	n = (std::size_t)random_uniform(0.0,(double)N);

	for (std::size_t i=0; i < N; i++) {
	  if ((random_uniform(0.0,1.0) < crossover_probability_) || (i  == (N - 1))) {
	    trial_solution[n] = 
	      population_[r1][n]
	      + mutation_scale_ * (population_[r2][n] - population_[r4][n])
	      + mutation_scale_ * (population_[r3][n] - population_[r5][n]);
	    handle_bounds(trial_solution, n, population_[r1][n]);
	  }
	  n = (n + 1) % N;
	}
      }
    };

  } // namespace math
} // namespace vic 

#endif
