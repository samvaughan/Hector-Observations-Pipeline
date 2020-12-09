/*+            T D F U T I L

 *  Module name:
      tdFutil.h

 *  Function:
      Include file for 2dF utility routines.

 *  Description:

 *  Language:
      C

 *  Support: James Wilcox, AAO

 *-

 *  History:
      01-Jul-1994  JW   Original version
      15-Sep-1997  TJF  Add tdFposVmeIO functions.
      31-Jul-2006  TJF  Add tdFposNSMaskCheck()
      {@change entry@}

 *  @(#) $Id: ACMM:2dFutil/tdFutil.h,v 5.7 14-Mar-2016 09:20:46+11 tjf $ (mm/dd/yy)
 */

#ifndef __TDFUTIL_H__
#define __TDFUTIL_H__
#ifdef __cplusplus
 extern "C" {
#endif


#include "status.h"
/*
 *  Use to declare constant arguments.
 *  Under GNU may get some warnings when compiling TDFUTIL itself due to C
 *  routines not being declared correctly.
 */
#define TDFUTIL_CONST_ARGVAL  const   /* Use for constant argument values     */
#define TDFUTIL_CONST_ARGPNT  const   /* Use for constant argument pointers   */
#define TDFUTIL_CONST_VAR     const   /* Use for constant variables           */

/*
 *  Used to declare routines.
 */
#define TDFUTIL_PUBLIC   extern       /* routines available to other packages */
#define TDFUTIL_INTERNAL extern       /* routines available thoughout tdFpt   */
#define TDFUTIL_PRIVATE  static       /* routines internal to one module      */


/*
 *  Utility functions.
 */
extern double tdFautoTheta(long int pivotx, long int pivoty, 
			 long int butX,   long int butY);
extern double tdFautoThetaPos(long int x,   long int y);

extern char * tdFtimestamp (int len, char *buf);

extern void tdFstateBitSet (unsigned char bit, StatusType *status);
extern void tdFstateBitClear (unsigned char bit, StatusType *status);

extern void tdFposVmeIOInit (void);
extern int tdFposVmeIOWrite (unsigned char clear, 
                              unsigned char set);

extern unsigned char tdFposVmeIORead ();
extern unsigned char * tdFposVmeIOMaskAddr ();


extern void tdFposNSMaskCheck(
    int debug,
    int simFlag,
    int plate,
    const char *parameters[],
    int * safe,
    StatusType *status);
                                   

#ifdef __cplusplus
}
#endif
#endif
