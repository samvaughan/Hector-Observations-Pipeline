/*  SDS TCL module Include file                              */
/*                                                           */
/*  Tony Farrell      1998  Nov 3                            */
/*                                                           */
/*     @(#) $Id: ACMM:sds/sdstcl.h,v 3.94 09-Dec-2020 17:15:24+11 ks $                        */
/*                                                           */

#ifndef _SDSTCL_INCLUDE_
#define _SDSTCL_INCLUDE_ 1

#include "sds.h"

/*
 *  This cam from gitlogger.C, version 3.7 and was inserted into that file
 *  by Keith Shortridge.
 *
 *  USE_NON_CONST is used by Tcl 8.4 to control compatability with old
 *  versions of code. We set it here and it has the effect under 8.4
 *  of not using const declarations in a number of critical places - all
 *  the places where const was added in 8.4, I assume. This means that
 *  under Tcl 8.4 the definition of a TclCmdProc has a CONST84 char* argument
 *  and with USE_NON_CONST defined the CONST84 is a null, so we don't need
 *  a const in the definition of initCmd(). And this should work for earlier
 *  versions which simply don't have that const at all.
 */

#define USE_NON_CONST

#ifdef VAXC
#pragma nostandard
#endif
#include <tcl.h>
#ifdef VAXC
#pragma standard
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*
 *  Typedef for routine arguments to SdsTclAddCommands.
 */
typedef int (*SdsTclCmdErrorRoutineType)(
    Tcl_Interp *interp,
    SDSCONST char *cmd,
    StatusType error);

typedef void (*SdsTclCreateCommandRoutineType)(
    Tcl_Interp *interp,
    SDSCONST char * SDSCONST name,
    Tcl_CmdProc *proc,
    ClientData * clientData,
    Tcl_CmdDeleteProc *deleteProc);



/*
 *  Function to create the SDS Tcl commands.  Can be invoked by either
 *  Sds_Init or Dtcl.
 */
SDSEXTERN void SdsTclAddCommands(  /* sdstcl.c */
    Tcl_Interp *interp,
    SdsTclCmdErrorRoutineType cmdError,
    SdsTclCreateCommandRoutineType createCmd);
    
/*
 *  Function to add Sds to a Tcl interpreter.
 */
SDSEXTERN int Sds_Init(            /* sdstclinit.c */
    Tcl_Interp *interp);


#ifdef __cplusplus
} /* extern "C" */
#endif
#endif


