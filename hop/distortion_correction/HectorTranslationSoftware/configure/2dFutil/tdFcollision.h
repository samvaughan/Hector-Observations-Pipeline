/*+            T D F U T I L

 *  Module name:
      tdFcollision.h

 *  Function:
      Prototypes for tdFcollision functions.

 *  Description:

 *  Language:
      C

 *  Support: James Wilcox, AAO

 *-

 *  History:
      01-Jul-1994  JW   Original version
      {@change entry@}

 *  @(#) $Id: ACMM:2dFutil/tdFcollision.h,v 5.7 14-Mar-2016 09:20:46+11 tjf $ (mm/dd/yy)
 */

#ifndef __TDFCOLLISION_H__
#define __TDFCOLLISION_H__
#ifdef __cplusplus
 extern "C" {
#endif

#include "tdFutil.h"

TDFUTIL_INTERNAL int  tdFcollisionFibFib (
        double  xapiv,
        double  yapiv,
        double  xa,
        double  ya,
        double  xbpiv,
        double  ybpiv,
        double  xb,
        double  yb);

TDFUTIL_INTERNAL int  tdFcollisionButFib (
        double    butX,
        double    butY,
        double    theta,
        double    fvpX,
        double    fvpY,
        double    pivX,
        double    pivY,
        long int  fibreClear);

TDFUTIL_INTERNAL int  tdFcollisionButBut (
        double    butXa,
        double    butYa,
        double    thetaa,
        double    butXb,
        double    butYb,
        double    thetab,
        long int  buttClear);

TDFUTIL_INTERNAL int  tdFcollisionMarkFib (
        long int  end1x,
        long int  end1y,
        long int  end2x,
        long int  end2y,
        long int  markx,
        long int  marky,
        double    tolSqrd);

TDFUTIL_INTERNAL int  tdFcollisionMarkBut (
        long int  butx,
        long int  buty,
        double    theta,
        long int  markx,
        long int  marky,
        double    tolSqrd);

TDFUTIL_INTERNAL int  tdFcollisionScrew (
        long int  butx,
        long int  buty,
        double    theta,
        double    tolSqrd);

#ifdef __cplusplus
}
#endif
#endif
