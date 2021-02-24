/*                              s d s t c l i n i t. c
  
 * Module name:
        sdstclinit.c
  
 * Function:
        Impelments the standalone (non-DRAMA) SDS Tcl interface.
  
 * Description:
        Provides the function Sds_Init(), which adds SDS Tcl commands
        to Tcl.
        
  
 * Language:
      C
  
 * Support: Tony Farrell, AAO
  
 * History:
      03-Nov-1998 - TJF - Original version (dtclsds.c)
      21-Jan-2001 - TJF - Insert compilation fixes from N.Rees, JACH.
      12-Feb-2002 - KS  - Include strings.h and string.h to prevent compilation
                          warnings on some systems.
      12-Jan-2007 - TJF - Tcl_SetErrorCode() must use (char *)0
                          rather then 0 as the final argument - (char *)0 is
                          different then 0 in a variable argument list on a
                          64 bit machine (0 passes a 32 bit int, (char *)0
                          passed a 64 bit pointer).
      25-Feb-2015 -  KS - Replace the reference to the now deprecated 'result'
                          field of the tcl interpreter by a call to 
                          Tcl_SetResult().
  
 *  Copyright (c) Anglo-Australian Telescope Board, 1998-2015.
    Not to be used for commercial purposes without AATB permission.

  
 *     @(#) $Id: ACMM:sds/sdstclinit.c,v 3.94 09-Dec-2020 17:15:24+11 ks $
  
 */


static const char *rcsId="@(#) $Id: ACMM:sds/sdstclinit.c,v 3.94 09-Dec-2020 17:15:24+11 ks $";
static void *use_rcsId = (0 ? (void *)(&use_rcsId) : (void *) &rcsId);


#include <strings.h>
#include <string.h>
#include "sdstcl.h"
#include "Ers.h"

/*
 *  Internal SDS error reporting routine.
 *
 *  This is called whenever we get an error from the SDS Tcl commands due
 *  to status being set bad.  It provides consistent formating of error
 *  codes.
 *  
 */
static int TclCmdError(
    Tcl_Interp *interp,
    SDSCONST char *cmd,
    StatusType error)
{
    char value[20]; /* For text of message number */
/*
 *  Format the command name, followed by a colon and the error test
 *  into the Tcl result string. (KS: this code really should be testing
 *  that the buffer isn't going to overflow. However, it's worked up to
 *  now writing directly into interp->result, and I've made result[]
 *  bigger than that, so I guess it's OK. I didn't want to make more
 *  substrantial changes before I had a good way to test them, in case I got
 *  them wrong)
 */
    if (cmd)
    {
        char result[256];
        int len = strlen(cmd);
        strcpy(result,cmd);
        result[len++] = ':';
        sprintf(&result[len],"%s",SdsErrorCodeString(error));
        result[sizeof(result)-1] = '\0';
        Tcl_SetResult(interp,result,TCL_VOLATILE);
    }
/*
 *  Format the errorCode info.
 */
    sprintf(value,"%ld",(long)error);
/*
 *  Set the error code. We set the first value in the array to SDS and
 *  the second to the decimal value.
 */
    Tcl_SetErrorCode(interp,"SDS",value, (char *)0);

    if (error != STATUS__OK)
        ErsFlush(&error);

    return TCL_ERROR;

}

/*
 *  Internal SDS command creation routine.  Adds the specified command to
 *  the Sds namespace, if we have Tcl namespace support.  Otherwise, it
 *  is put into the global namespace.
 */
static void CreateCommand(
    Tcl_Interp *interp,
    SDSCONST char * SDSCONST name,
    Tcl_CmdProc *proc,
    ClientData * clientData,
    Tcl_CmdDeleteProc *deleteProc)
{
    Tcl_DString dString;

    Tcl_DStringInit(&dString);

#if TCL_MAJOR_VERSION >= 8
    Tcl_DStringAppend(&dString,"sds::",-1);
#endif
    Tcl_DStringAppend(&dString,(char *)name,-1);

    Tcl_CreateCommand(interp, Tcl_DStringValue(&dString),
                      proc, clientData, deleteProc);
    

}


/*+	S d s _ I n i t
 
 * Function Name:
        Sds_Init

 * Function:
        Add SDS commands to a TCL intepreter.
 
 * Descriptions:
        Add the SDS TCL commands to a TCL intepreter.  This assumes
        a non-DRAMA TCL intepreter.  Otherwise, DTCL should be used.

        The intention is to allow SDS commands to be added to a non-DRAMA
        TCL intpreter.

        Commands are added in the "sds" namespace, unless we don't
        have namespace support, in which case commands are added to
        the global namespace.

 * Include File:
	sdstcl.h

 *  Call:
         (int) = Sds_Init(interp)

 *   Parameters:  (">" input, "!" modified, "W" workspace, "<" output)
      (>) interp (Tcl_Interp) The Tcl interperter to add the commands too.
 
 * Author: Tony Farrell, AAO

 *  See Also: SDS manual, DTCL manual.

 *-
 
 *  Copyright (c) Anglo-Australian Telescope Board, 1998.
    Not to be used for commercial purposes without AATB permission.

 *     @(#) $Id: ACMM:sds/sdstclinit.c,v 3.94 09-Dec-2020 17:15:24+11 ks $
  
 * History:
      03-Nov-1998 - TJF - Original version.
      01-Sep-1999 - KS  - Allows for non-Unix builds where SDS_VERNUM is
                          undefined. (Normally, this is defined using a -D
                          option in a makefile.)
      
 */

SDSEXTERN int Sds_Init(         
    Tcl_Interp *interp)
{
    char verString[40];
/*
 *  Tell Tcl what package we are.  
 */
 
#ifndef SDS_VERNUM
#define SDS_VERNUM "0.0"
#endif

#if TCL_MAJOR_VERSION >= 8
    sprintf(verString,"%s",SDS_VERNUM);
    if (Tcl_PkgProvide(interp,"Sds",verString) == TCL_ERROR)
        return TCL_ERROR;
#endif

/*
 *  Add the commands
 */
    SdsTclAddCommands(interp, TclCmdError, CreateCommand);

    return TCL_OK;
    
}
