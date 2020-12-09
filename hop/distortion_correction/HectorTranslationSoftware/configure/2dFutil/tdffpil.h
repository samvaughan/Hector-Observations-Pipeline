#ifndef _TDFFPIL_INC
#define _TDFFPIL_INC
#ifdef __cplusplus
extern "C" {
#endif

/*
 *+			T D F F P I L

 * Include File name:
      tdffpil

 * Function:
      Include file for tdFfpilMin and tdFfpilFull modules.

 * Description:
      Contains extern definitions from tdFfpilMin.c and tdFfpilFull.c
      and internal prototypes of tdFfpilMin routines used by tdFfpilFull.

 * Language:
      C


 * Support: Tony Farrell, AAO

 *-

 *     @(#) $Id: ACMM:2dFutil/tdffpil.h,v 5.7 14-Mar-2016 09:20:47+11 tjf $

 * History:
     10-Dec-1999 - TJF - Original version
     15-Dec-2004 - TJF - Replace TdfFpilFullInit() by TdfFpilFullInit400 and 
                            TdfFpilFullInit404() due to plate change.
     07-Mar-2005 -  KS - Add TdfFpilFullInitAAomega.
     05-Apr-2012 - MVV - Add TdfFpilFullInitHERMES.
 */

/*
 *  Include files
 */
#include "fpil.h"


/*
 * exported function prototypes
 */
extern void  TdfFpil___SetupMinStructs(
    FpilCollisionChecksType *routines,
    FpilInstParamsType      *params);

extern void TdfFpilMinInit(FpilType *inst);
extern void TdfFpilFullInit400(FpilType *inst);
extern void TdfFpilFullInit404(FpilType *inst);
extern void TdfFpilFullInitAAOmega(FpilType *inst);
extern void TdfFpilFullInitHERMES(FpilType *inst);



#ifdef TDFFPIL_BUILD
extern void TdfFpilFull___AccessCons (
    const char *baseFile,
    const char *dirOverride,
    const char *dirFallback,
    FpilRptRoutineType RptRoutine,
    void * RptArg,
    SdsIdType *id,
    StatusType *status);
extern void TdfFpilFull___SetupFullStructs(
    FpilCollisionChecksType *routines,
    FpilInstParamsType      *params,
    FpilModelRoutinesType   *modelRoutines);

extern void TdfFpilFull_CreateCombined(
    const SdsIdType sourceId,
    SdsIdType * const id,
    StatusType * const status);


#define TDFCONSDIR  "/instsoft/2dF/positioner" /* Directory at the AAT/AAO */




#endif /* TDFFPIL_BUILD */

#define TDFFPIL__MALLOCERR 10

#ifdef __cplusplus
}
#endif
#endif
