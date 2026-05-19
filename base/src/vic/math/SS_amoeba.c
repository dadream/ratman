/* Nelder and Mead Simplex Method for Nonlinear unconstrained optimization problems          */

#include "SS.h"
#include <math.h>
#include <malloc.h>

#define NMAX 5000
#define ALPHA 1.0
#define BETA 0.5
#define GAMMA 2.0

#define GET_PSUM for (j=1;j<=ndim;j++) { for (i=1, sum=0.0;i<=mpts;i++) sum += p[i][j]; psum[j]=sum; }

int SSamoeba(double **p, double *y, int ndim, double ftol, double (*funk)(void *, double*), int *nfunk, void *userdata)
{
	int i, j, ilo, ihi, inhi, mpts=ndim+1;
	double ytry, ysave, sum, rtol, *psum;

	psum = SSallocate_double_array(ndim+1);
	*nfunk = 0;
	GET_PSUM
	for (;;) {
		ilo = 1;
		ihi = y[1] > y[2] ? (inhi = 2, 1) : (inhi = 1, 2);
		for (i = 1; i <= mpts; i++) {
			if (y[i] < y[ilo]) ilo = i;
			if (y[i] > y[ihi]) {
				inhi = ihi;
				ihi = i;
			} else if (y[i] > y[inhi])
				if (i != ihi) inhi = i;
		}
		rtol = 2.0*fabs(y[ihi]-y[ilo])/(fabs(y[ihi])+fabs(y[ilo]));
		if (rtol < ftol) break;
		if (*nfunk >= NMAX) 
			return -1;
		ytry = SSamotry(p, y, psum, ndim, funk, ihi, nfunk, -ALPHA,userdata);
		if (ytry <= y[ilo])
			ytry = SSamotry(p, y, psum, ndim, funk, ihi, nfunk, GAMMA,userdata);
		else if (ytry >= y[inhi]) {
			ysave = y[ihi];
			ytry = SSamotry(p, y, psum, ndim, funk, ihi, nfunk, BETA,userdata);
			if (ytry >= ysave) {
				for (i = 1; i <= mpts; i++) {
					if (i != ilo) {
						for (j = 1; j <= ndim; j++) {
							psum[j] = 0.5*(p[i][j]+p[ilo][j]);
							p[i][j] = psum[j];
						}
						y[i] = (*funk)(userdata,psum);
					}
				}
				*nfunk += ndim;
				GET_PSUM
			}
		}
	}
	free(psum);
	return 0;
}

double SSamotry(double **p, double *y, double *psum, int ndim, double (*funk)(void*,double*), int ihi, int *nfunk, double fac, void*userdata)

{
	int j;
	double fac1, fac2, ytry, *ptry;

	ptry = SSallocate_double_array(ndim+1);
	fac1 = (1.0-fac)/(double)ndim;
	fac2 = fac1 - fac;
	for (j = 1; j <= ndim; j++) ptry[j] = psum[j]*fac1-p[ihi][j]*fac2;
	ytry = (*funk)(userdata,ptry);
	++(*nfunk);
	if (ytry < y[ihi]) {
		y[ihi] = ytry;
		for (j = 1; j <= ndim; j++) {
			psum[j] += ptry[j]-p[ihi][j];
			p[ihi][j] = ptry[j];
		}
	}
	free(ptry);
	return ytry;
}
