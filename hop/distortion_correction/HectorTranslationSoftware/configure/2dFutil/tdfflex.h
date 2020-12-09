#ifndef __TDFFLEX__
#define __TDFFLEX__ 1

/*
 * "@(#) $Id: ACMM:2dFutil/tdfflex.h,v 5.7 14-Mar-2016 09:20:46+11 tjf $"
 */

#ifdef __cplusplus
 extern "C" {
#endif



typedef struct TdfFlexType
   {
      double offset;
      double length;
      double k;
   } TdfFlexType;
   

void TdfGetFlex(char *path, TdfFlexType *pars, StatusType *status);

void TdfSetFlex(TdfFlexType pars, char *file, StatusType *status);

void TdfFlexure(long int x, long int y, double ha, double dec,
                TdfFlexType *pars, long int *dx, long int *dy, StatusType *status);

#ifdef __cplusplus
}
#endif
#endif
