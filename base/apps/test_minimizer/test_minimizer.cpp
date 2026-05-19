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
#include <vic/math/scatter_search_minimizer.hpp>
#include <cstdio>

class polyfit: public vic::math::scalar_functor<double,double> {
public:
  typedef vic::math::scalar_functor<double,double> super_t;
public:
  polyfit(std::size_t arg_dim): super_t(arg_dim) {
  }

  double operator()(const double *trial) const {
    std::size_t i, j;
    std::size_t const M=60;
    double px, x=-1, dx=M, result=0;
    
    dx = 2.0 / dx;
    for (i=0; i<=M; i++) {
      px = trial[0];
      for (j=1;j<arg_dim_;j++)
	px = x*px + trial[j];
      
      if (px<-1 || px>1)
	result += (1 - px) * (1 - px);
      
      x += dx;
    }
    
    px = trial[0];
    for (j=1;j<arg_dim_;j++)
      px = 1.2*px + trial[j];
    
    px = px - 72.661;
    if (px<0)
      result += px * px;
    
    px = trial[0];
    for (j=1; j<arg_dim_; j++)
      px = -1.2*px + trial[j];
    
    px = px - 72.661;
    
    if (px<0)
      result+=px*px;
    
    return(result);
  }
};

#define N_DIM 9

int main(void) {
  double vmin[N_DIM];
  double vmax[N_DIM];
  int i;
  
  vic::math::scatter_search_minimizer<double,double> solver;
  
  polyfit f = polyfit(N_DIM);
  for (i=0;i<N_DIM;i++) {
    vmax[i] =  300.0;
    vmin[i] = -300.0;
  }
  
  solver.set_objective_function(&f);
  solver.set_arg_bounds(vmin, vmax);
  solver.set_defaults();
#if 0
  solver.set_diversity_set_count(20);
  solver.set_quality_set_count(20);
  solver.set_diversificator_set_count(100);
  solver.set_max_step_count(100);
#endif

  printf("Calculating...\n\n");

  solver.solve();    
  const double *solution = solver.best_argument();
  
  printf("\n\nBest Coefficients:\n");
  for (i=0;i<N_DIM;i++)
    printf("[%d]: %lf\n",i,solution[i]);
  printf("\n\nBest Energy:\n");
  printf("%lf\n",solver.best_value());
  
  return 0;
}
