#include "SS.h"
#include <math.h>
#include <malloc.h>

#define SSgetrandom(p,min,max)  (int)( (SSrandNum(p)*(max+1-min)) + (min))

void SSInitiate_RefSet(SS *prob) {
  double *current,*min_dist,*value,**solutions;
  int j,i,i1,a,*index,*index2,cont=0;
  double d,dmax,current_value;

  prob->iter=1;
  prob->new_elements = 1;

  current = SSallocate_double_array(prob->n_var);
  if(!current) SSabort("Memory allocation problem");

  min_dist = SSallocate_double_array(prob->PSize);	
  if(!min_dist) SSabort("Memory allocation problem");

  value = SSallocate_double_array(prob->PSize);	
  if(!min_dist) SSabort("Memory allocation problem");

  solutions = SSallocate_double_matrix(prob->PSize,prob->n_var);
  if(!solutions) SSabort("Memory allocation problem");

  for(i=1;i<=prob->PSize;i++)
    {
      /* Generate new solution */
      for(j=1;j<=prob->n_var;j++)	
	current[j] = SSGenerate_value(prob,j);

      /* Evaluate Solution */
      current_value=prob->evaluate(prob->userdata,current);

      if(prob->LS) SSimprove_solution(prob,current,&current_value);

      if(SSis_new(prob,solutions,i-1,current))
	{
	  /* Store solution in matrix "solutions" */
	  for(j=1;j<=prob->n_var;j++)
	    solutions[i][j] = current[j];
		
	  value[i]=current_value;
	}
      else 
	{
	  i--;
	  cont++;
	}

      if(cont>prob->PSize/2)
	{
	  prob->digits++;
	  cont=0;
	}
    }
	
  index = SSorden_indices(value,prob->PSize,-1);

  /* Add the best b1 to RefSet1 */
  for(i=1;i<=prob->b1;i++)
    {
      for(j=1;j<=prob->n_var;j++)
	prob->RefSet1[i][j] = solutions[index[i]][j];
		
      prob->value1[i] = value[index[i]];
      prob->order1[i] = i;
      prob->iter1[i]  = 1;
    }

  /* Compute minimum distances */
  for(i=1;i<=prob->PSize;i++)
    min_dist[i]=SSdistance_to_RefSet1(prob,solutions[i]);

  /*Add the second b2 to RefSet2 */
  for(i=1;i<=prob->b2;i++)
    {
      /* Select the solution with maximum min. dist */
      a = 1; dmax=min_dist[1];
      for(j=1;j<=prob->PSize;j++)
	if(min_dist[j]>dmax)
	  {
	    dmax=min_dist[j];
	    a=j;
	  }

      for(j=1;j<=prob->n_var;j++)
	prob->RefSet2[i][j] = solutions[a][j];
      prob->value2[i] = min_dist[a];

      /* Update minimum distances */
      for(i1=1;i1<=prob->PSize;i1++)
	{
	  d=0;
	  for(j=1;j<=prob->n_var;j++)
	    d += pow(solutions[i1][j]-solutions[a][j],2);
	  if(d<min_dist[i1])
	    min_dist[i1]=d;
	}
    }

  /* Update minimum distances in RefSet2 */

  for(i=1;i<=prob->b2;i++)
    {
      for(a=1;a<=prob->b2 && a!=i ;a++)
	{
	  d=0;
	  for(j=1;j<=prob->n_var;j++)
	    d += pow(prob->RefSet2[i][j]-prob->RefSet2[a][j],2);
	  if(prob->value2[i] > d)
	    prob->value2[i]=d;
	}
    }

  index2 = SSorden_indices(prob->value2,prob->b2,1);
	
  for(i=1;i<=prob->b2;i++)
    {
      prob->order2[i] = index2[i];
      prob->iter2[i]  = 1;
    }

  free(index);free(index2);free(current);free(min_dist);
  free(value);SSfree_double_matrix(solutions,prob->PSize);
}


void SSUpdate_RefSet2(SS *prob) {
  double *current,*min_dist,*value,**solutions;
  int j,i,i1,a,*index2,cont=0;
  double d,dmax;

  prob->iter++;
  prob->digits++;

  current = SSallocate_double_array(prob->n_var);
  if(!current) SSabort("Memory allocation problem");

  min_dist = SSallocate_double_array(prob->PSize);	
  if(!min_dist) SSabort("Memory allocation problem");

  value = SSallocate_double_array(prob->PSize);	
  if(!min_dist) SSabort("Memory allocation problem");

  solutions = SSallocate_double_matrix(prob->PSize,prob->n_var);
  if(!solutions) SSabort("Memory allocation problem");

  for(i=1;i<=prob->PSize;i++)
    {
      /* Generate new solution */
      for(j=1;j<=prob->n_var;j++)	
	current[j] = SSGenerate_value(prob,j);

      if(SSis_new(prob,solutions,i-1,current))
	{
	  /* Store solution in matrix "solutions" */
	  for(j=1;j<=prob->n_var;j++)
	    solutions[i][j] = current[j];
	}
      else 
	{
	  i--;
	  cont++;
	}

      if(cont>prob->PSize/2)
	{
	  prob->digits++;
	  cont=0;
	}
    }
	
  /* Compute minimum distances */
  for(i=1;i<=prob->PSize;i++)
    min_dist[i]=SSdistance_to_RefSet1(prob,solutions[i]);

  /*Add to RefSet */
  for(i=1;i<=prob->b2;i++)
    {
      /* Select the solution with maximum min. dist */
      a=1; dmax=min_dist[1];
      for(j=1;j<=prob->PSize;j++)
	if(min_dist[j]>dmax)
	  {
	    dmax=min_dist[j];
	    a=j;
	  }

      for(j=1;j<=prob->n_var;j++)
	prob->RefSet2[i][j] = solutions[a][j];
		
      /* Update minimum distances */
      for(i1=1;i1<=prob->PSize;i1++)
	{
	  d=0;
	  for(j=1;j<=prob->n_var;j++)
	    d += pow(solutions[i1][j]-solutions[a][j],2);
	  if(d<min_dist[i1])
	    min_dist[i1]=d;
	}
    }

  /* Update minimum distances in RefSet2 */

  for(i=1;i<=prob->b2;i++)
    {
      for(a=1;a<=prob->b2 && a!=i ;a++)
	{
	  d=0;
	  for(j=1;j<=prob->n_var;j++)
	    d += pow(prob->RefSet2[i][j]-prob->RefSet2[a][j],2);
	  if(min_dist[i] > d)
	    min_dist[i]=d;
	}
      prob->value2[i] = min_dist[i];
    }

  index2 = SSorden_indices(prob->value2,prob->b2,1);
	
  for(i=1;i<=prob->b2;i++)
    {
      prob->order2[i] = index2[i];
      prob->iter2[i]  = prob->iter;
    }

  prob->new_elements = 1;

  free(index2);
  free(current);
  free(min_dist);
  free(value);
  SSfree_double_matrix(solutions,prob->PSize);
}

void SSCombine_RefSet(SS *prob) {
  int i,j,a,s,pull_size,total_size;
  double **offsprings,**pull;

  prob->new_elements=0;
  offsprings = SSallocate_double_matrix(4,prob->n_var);

  /* New solutions are temporarily stored in a pull */
  pull_size=0;
  total_size=(4*prob->b1*prob->b1)/2+(3*prob->b1*prob->b2)+
    (2*prob->b2*prob->b2)/2;
  pull = SSallocate_double_matrix(total_size,prob->n_var);


  /* Combine elements in RefSet1 */
  for(i=1;i<prob->b1;i++)
    for(j=i+1;j<=prob->b1;j++)
      {
	/* Combine solutions not combined in the past */
	if(prob->iter1[i]>prob->last_combine ||
	   prob->iter1[j]>prob->last_combine   )
	  {
	    SScombine(prob,prob->RefSet1[i],prob->RefSet1[j],offsprings,4);

	    for(a=1;a<=4;a++)
	      {
		pull_size++;
		for(s=1;s<=prob->n_var;s++)
		  pull[pull_size][s]=offsprings[a][s];
	      }
	  }
      }


  /* Combine RefSet1 with RefSet2*/
  for(i=1;i<= prob->b1;i++)
    for(j=1;j<= prob->b2;j++)
      {
	if(prob->iter1[i]>prob->last_combine ||
	   prob->iter2[j]>prob->last_combine   )
	  {
	    SScombine(prob,prob->RefSet1[i],prob->RefSet2[j],offsprings,3);

	    for(a=1;a<=3;a++)
	      {
		pull_size++;
		for(s=1;s<=prob->n_var;s++)
		  pull[pull_size][s]=offsprings[a][s];
	      }
	  }
      }

  /* Combine elements in Refset2 */
  for(i=1;i<prob->b2;i++)
    for(j=i+1;j<=prob->b2;j++)
      {
	if(prob->iter2[i]>prob->last_combine ||
	   prob->iter2[j]>prob->last_combine   )
	  {
	    SScombine(prob,prob->RefSet2[i],prob->RefSet2[j],offsprings,2);

	    for(a=1;a<=2;a++)
	      {
		pull_size++;
		for(s=1;s<=prob->n_var;s++)
		  pull[pull_size][s]=offsprings[a][s];
	      }
	  }
      }

  /* Update, if necessary, Reference Set */
	
  prob->last_combine=prob->iter;
  prob->iter++;
  for(a=1;a<=pull_size;a++)
    {
      SStry_add_RefSet1(prob,pull[a]);
      SStry_add_RefSet2(prob,pull[a]);
    }

  SSfree_double_matrix(pull,total_size);
  SSfree_double_matrix(offsprings,4);
	
}


double SSGenerate_value(SS *prob,int a)
{
  int i,j;
  int *rfrec; /* reverse frec to penalize high frecs. */
  double r,value,low,range;
  int *frec;

  frec = prob->ranges[a];
  low  = prob->low[a];
  range= prob->high[a]-prob->low[a];

  rfrec = SSallocate_int_array(5);
  if(!rfrec) SSabort("Problems allocating memory");

  for(i=1;i<=4;i++)
    {
      rfrec[i]  = frec[0] - frec[i];
      rfrec[0] += rfrec[i];
    }

  if(rfrec[0]==0)
    i = SSgetrandom(prob,1,4);
  else
    {
      /* Select a subrange (from 1 to 4) according to rfrec */
      j = SSgetrandom(prob,1,rfrec[0]);
      i=1;
      while(j>rfrec[i])
	j -= rfrec[i++];
    }
  if(i>4) SSabort("Problems generating values");;

  /* i is the selected subrange */
  frec[i]++;
  frec[0]++;
  free(rfrec);

  /* Randomly select an element in subrange i */
  r = SSrandNum(prob);
  value=low+(i-1)*(range/4) + (r*range/4);
  return value;
}

void SSimprove_solution(SS *prob, double *sol,double *value)
{
  double **p,*y,range,perturb;
  int i,j,a,nfunk,best_sol;

  p = SSallocate_double_matrix(prob->n_var+1,prob->n_var);
  y = SSallocate_double_array(prob->n_var+1);
	

  for(i=1;i<=prob->n_var;i++)
    p[1][i]=sol[i];
	
  y[1]=*value;

  for(j=1;j<=prob->n_var;j++)
    {
      range   = prob->high[j]-prob->low[j];
      perturb = 0.1*range;
      sol[j] += perturb;

      if(sol[j]>prob->high[j]) sol[j]=prob->high[j];
      if(sol[j]<prob->low[j])  sol[j]=prob->low[j];

      for(i=1;i<=prob->n_var;i++)
	p[j+1][i]=sol[i];
		
      y[j+1] = prob->evaluate(prob->userdata,sol);
      sol[j] -= perturb;
    }

  /* Call Nelder and Mead's Simplex method */
  a=SSamoeba(p, y, prob->n_var, 0.1, prob->evaluate, &nfunk, prob->userdata);

  best_sol=0;
  for(i=1;i<=prob->n_var+1;i++)
    {
      if( *value>y[i])
	{
	  *value=y[i];
	  best_sol=i;
	}
    }

  if(best_sol>0)
    {
      for(i=1;i<=prob->n_var;i++)
	{
	  sol[i]=p[best_sol][i];

	  if(sol[i]<prob->low[i])
	    sol[i]=prob->low[i];
	  if(sol[i]>prob->high[i])
	    sol[i]=prob->high[i];
	}
	
      *value = prob->evaluate(prob->userdata,sol);
    }

  free(y);
  SSfree_double_matrix(p,prob->n_var+1);
}




