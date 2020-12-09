#ifndef __TDFXYFIT__
#define __TDFXYFIT__ 1
/*
 * "@(#) $Id: ACMM:2dFutil/tdfxyfit.h,v 5.7 14-Mar-2016 09:20:46+11 tjf $"
 */
           
void TdfFullFit(int nobs, double *rd, double *xy, char *ex, double rc, 
                double dc, TdfXyType *xypars, TdfDistType *dist, 
                double mjd, double coeffs[6], int fixdist[6], 
                int fixlin[6], double *xyp, 
                double pvar[12], double *rms, double *psd, 
                void progress(float), StatusType *status);


void TdfXyVec(int nobs,double rc,double dc,TdfXyType *xypars,double pars[12],
               double mjd, double *rd, double *xy, StatusType *status);

void TdfFit(SdsIdType cal, TdfXyType *xypars, TdfDistType *dist,
            double coeffs[6], StatusType *status);

#endif
