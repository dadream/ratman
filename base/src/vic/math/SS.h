#ifndef SS_H
#define SS_H

#ifdef __cplusplus
extern "C" {
#endif

typedef double (*SS_objective_function_t)(void *userdata, double *x);

typedef struct SS {
  void * userdata; // Added by Ego.
  SS_objective_function_t evaluate;

  int n_var;	
  double digits;
  double *high;
  double *low;
  int **ranges; /* Diversification Generator */
  int PSize;
  int LS;		  /* =1 LocalSearch ON, 0 OFF */
  int iter;
  
  int b1;
  double **RefSet1;// Solutions 
  double *value1;  // Objective value
  int *order1;	 //	Order of solutions
  int *iter1;		 // Number of iter of each sol.
  
  int b2;
  double **RefSet2;
  double *value2;	 // Dissim value
  int *order2;
  int *iter2;
  
  int last_combine;  //Number of iter of last solution combination
  int new_elements;  //True if new elem. added since last combine
  
  /* Random number parameters */
  long idum;	    				
  int seed_reset;	
  int iff;									
  long ir[98];
  long iy;
} SS;


/* File SS_RefSet.c */

extern void SSInitiate_RefSet(SS *prob);
extern double SSGenerate_value(SS *prob,int a);
extern void SSimprove_solution(SS *prob, double *sol,double *value);
extern void SSUpdate_RefSet2(SS *prob);
extern void SSCombine_RefSet(SS *prob);

/* File SS_Memory.c */

extern SS *SSnew(void *userdata, 
		 SS_objective_function_t fn,
		 int nvar,int b1,int b2,int PSize,int LocalSearch);
extern void SSdelete(SS *prob);
extern int **SSallocate_int_matrix(int rows,int columns);
extern double **SSallocate_double_matrix(int rows,int columns);
extern double *SSallocate_double_array(int size);
extern int *SSallocate_int_array(int size);
extern void SSfree_double_matrix(double **matrix,int rows);
extern void SSfree_int_matrix(int **matrix,int rows);


/* File SS_tools.c */

extern float SSrandNum(SS *p);
extern int *SSorden_indices(double *pesos,int num,int tipo);
extern void SSabort(char *texto);
extern double SSdistance_to_RefSet(SS *prob,double *sol);
extern double SSdistance_to_RefSet1(SS *prob,double *sol);
extern int SSis_new(SS *prob,double **solutions,int dim,double *sol);
extern void SScombine(SS *prob,double *x,double *y,double **offsprings,int number);
extern void SStry_add_RefSet1(SS *prob,double *sol);
extern void SStry_add_RefSet2(SS *prob,double *sol);

/* File nmsimplex.c */

extern int   SSamoeba(double **p, double *y, int ndim, double ftol, double (*funk)(void*, double*), int *nfunk, void *prob);
extern double SSamotry(double **p, double *y, double *psum, int ndim, double (*funk)(void*, double*), int ihi, int *nfunk, double fac, void *prob);

#ifdef __cplusplus
};
#endif


#endif
