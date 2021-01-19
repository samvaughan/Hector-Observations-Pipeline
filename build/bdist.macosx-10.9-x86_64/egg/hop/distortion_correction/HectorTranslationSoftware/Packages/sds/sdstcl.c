/*                              s d s t c l . c
  
 * Module name:
        sdstcl.c
  
 * Function:
        Tcl commands to interface to Sds.
  
 * Description:
 	Provides implementation of the various Sds releated TCL commands

        The routine Sds___TclAddCommands is used to add the commands to
        a TCL intepreter.  Note that command creation and error reporting
        routines should be supplied by the calling routine - which allows
        DTCL to maintain it's standard approach but allows Sds_Init() to
        provide standalone implementations.
        
  
 * Language:
      C
  
 * Support: Tony Farrell, AAO
  
 * History:
      30-Jun-1995 - TJF - Original version (dtclsds.c)
      03-Feb-1998 - TJF - Fix Borland compilation warnings.
      28-Sep-1998 - TJF - Call Dtcl___CreateCommand instead of
                          Tcl___CreateCommand.
      02-Nov-1998 - TJF - Extracted from DTCL library into SDS to allow
                          SDS TCL programs to be built without the overhead
                          which comes with the rest of DRAMA.
      17-Aug-2006 - TJF - Replace dimensions of 7 by SDS_C_MAXARRAYDIMS
      25-Feb-2015 -  KS - Replace all references to the now deprecated 'result'
                          field of the tcl interpreter by calls to Tcl_SetResult
                          and Tcl_GetResultString. Minor other changes to get a
                          clean compilation using Clang 6.0.
      18-Aug-2015 - TJF - Replace calls to ErsRep() with ERS_M_NOFMT flag set
                           by calls to new ErsRepNF(), to avoid compiler
                           warnings.

  
 *  Copyright (c) Anglo-Australian Telescope Board, 1998-2015.
    Not to be used for commercial purposes without AATB permission.

  
 *     @(#) $Id: ACMM:sds/sdstcl.c,v 3.94 09-Dec-2020 17:15:55+11 ks $
  
 */



static const char *rcsId="@(#) $Id: ACMM:sds/sdstcl.c,v 3.94 09-Dec-2020 17:15:55+11 ks $";
static void *use_rcsId = (0 ? (void *)(&use_rcsId) : (void *) &rcsId);


#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "sdstcl.h"
#include "arg.h"
#include "Ers.h"
#include "sds_err.h"

/*
 * VAXC does not support signed char
 */
#ifdef VAXC
#define schar char
#else
#define schar signed char
#endif

static void STclErsRep(Tcl_Interp * interp, 
			 StatusType * status);


/*   TCL Commands   */

/*+                     A r g N e w
 *  Command name:
      ArgNew

 *  Function:
      Create a new argument structure

 *  Description:
      A new SDS structure is created it and an identifier to it is returned
      

 *  Call:
      ArgNew
 

*   Parameters: 
      None

 *  Language:
      Tcl

 *  See Also: DTCL manual, Sds manual, ArgNew(3), SdsNew(3),
                        ArgPutString(n) ArgPutx(n)


 *  Support: Jeremy Bailey, AAO

 *-

 *  History:
      29-Sep-1993 - JAB - Original version
      24-Mar-1995 - TJF - Use DtclCmdError to report errors.
 */


static int STclArgNew(ClientData clientData , Tcl_Interp *interp, int argc, 
     char *argv[])

{
   SdsIdType id;
   StatusType status;
   SdsTclCmdErrorRoutineType cmdError = (SdsTclCmdErrorRoutineType)clientData;

   if (argc != 1) 
   {
       char result[256];
       sprintf(result,"%s:wrong # args",argv[0]);
       Tcl_SetResult(interp,result,TCL_VOLATILE);
       return (*cmdError)(interp,0,SDS__TCL_ARGERR);
   }
   status = SDS__OK;
   ArgNew(&id,&status);
   if (status == SDS__OK)
     {
       char result[256];
       sprintf(result,"%ld",(long int)id);
       Tcl_SetResult(interp,result,TCL_VOLATILE);
       return TCL_OK; 
     }
   else
     {
	return (*cmdError)(interp,argv[0],status);
     }
}        

/*+                     A r g G e t
 *  Command name:
      ArgGet

 *  Function:
      Get an item from an SDS structure

 *  Description:
      A named item is read from an SDS structure and a character string 
      representation of its value is returned.

      In Tcl there is only a single version of ArgGet rather than a
      version for each type as in C --- This is because in Tcl all
      variables are strings.

      If the top level item is itself a scalar item of the correct name
      its value will be returned. 

      With the exception of string items, the length of the result is
	limited to TCL_RESULT_SIZE (200 bytes) and you will get an error
	if the result is longer (Say for arrays).  You can use SdsArrayGet
	to access such items.  For string items, the extra space is well
	defined and hence is handled here.      
      
 *  Call:
      ArgGet id name
 

 *   Parameters: 
      (>) id  (int)  SDS identifier of structure.
      (>) name (string)  Name of component to get.


 *  Returns:
      The value of the SDS object. 

 *  Language:
      Tcl

 *  See Also: DTCL manual, Sds manual, ArgGetString(3), ArgNew(n)
			ArgPutString(n) ArgPutx(n)

 *  Support: Jeremy Bailey, AAO

 *-

 *  History:
      29-Sep-1993 - JAB - Original version
      11-Jan-1994 - JAB - Allow the scalar to be the top level item 
      24-Mar-1995 - TJF - Use DtclCmdError to report errors.
      24-May-1995 - TJF - Handle the special case of character strings
			longer then TCL_RESULT_SIZE by using SdsInfo
			to determine the length and allocating the space.
			For other types, this is harder to workout and you
			can use SdsArrayGet
      16-May-1996 - TJF - If scalar is the top item, use ArgCvt directly
      			instead of playing around creating a simulation
      			of a non-structured item.
      05-Nov-1997 - TJF - Add SdsExtract, SdsDelete, SdsFillArray,
                       SdsGet, SdsGetExtra, SdsInsertCell, SdsNew, SdsPut,
                       SdsPutExtra and SdsResize
 */

static int STclArgGet(ClientData clientData , Tcl_Interp *interp, int argc, 
     char *argv[])

{
   StatusType status;
   char name[SDS_C_NAMELEN];
   SdsCodeType code;
   long ndims;
   unsigned long dims[SDS_C_MAXARRAYDIMS];
   SdsIdType id;
   SdsTclCmdErrorRoutineType cmdError = (SdsTclCmdErrorRoutineType)clientData;

   if (argc != 3) 
     {
        char result[256];
        sprintf(result,"%s:wrong # args",argv[0]);
        Tcl_SetResult(interp,result,TCL_VOLATILE);
	return (*cmdError)(interp,0,SDS__TCL_ARGERR);
     }
   status = SDS__OK;
   id = atol(argv[1]);
   SdsInfo(id,name,&code,&ndims,dims,&status);
   if ((code != SDS_STRUCT) && (ndims <= 1) && (strcmp(name,argv[2]) == 0))
   {
/*
 *	If we are getting a character string and the length is too great
 *	then allocate sufficient memory for the result.
 */
      char result[256];
      char* resultPtr = result;
      unsigned int result_size = sizeof(result);
      Tcl_FreeProc* freeProc = TCL_VOLATILE;
      if ((ndims == 1)&&(code == SDS_CHAR)&&(dims[0] > result_size))
      {
	      result_size = dims[0]+100;
	      resultPtr = Tcl_Alloc(result_size);
         freeProc = TCL_DYNAMIC;
      }
      ArgCvt(&id,ARG_SDS,ARG_STRING,resultPtr,result_size,&status);
      Tcl_SetResult(interp,resultPtr,freeProc);
   }
   else
   {
/*
 *    Try to convert directly.
 */
      char result[256];
      unsigned int result_size = sizeof(result);
      ErsPush();
      ArgGetString(id,argv[2],result_size,result,&status);
      if (status != ARG__CNVERR)
      {
         Tcl_SetResult(interp,result,TCL_VOLATILE);
      }
      else
      {
/*
 *       Conversion error.  
 *	      If we are getting a character string and the length is too great
 *	      then allocate sufficient memory for the result.
 */
	      StatusType status2 = STATUS__OK;
	      StatusType ignore = STATUS__OK;
	      SdsIdType id2;
	      ndims = 0;
	      SdsFind(id,argv[2],&id2,&status2);
         SdsInfo(id2,name,&code,&ndims,dims,&status2);
         if ((ndims == 1)&&(code == SDS_CHAR)&&(dims[0] > result_size))
         {
/*
 *	         Is character string.  Annul previous error and try with larger
 *	         buffer.
 */
	         char* resultPtr;
	         result_size = dims[0]+100;
	         ErsAnnul(&status);
	         resultPtr = Tcl_Alloc(result_size);
            ArgGetString(id,argv[2],result_size,resultPtr,&status);
            if (status == STATUS__OK)
            {
               Tcl_SetResult(interp,resultPtr,TCL_DYNAMIC);
            }
         }
         SdsFreeId(id2,&ignore);
     }
     ErsPop();
   }
   if (status == SDS__OK)
   {
      return TCL_OK; 
   }
   else
   {
      return (*cmdError)(interp,argv[0],status);
   }
}  
/*+                     A r g G e t T y p e
 *  Command name:
      ArgGetType

 *  Function:
      Get the type of an Arg style SDS structure

 *  Description:
      Return the type of the item which will be fetched by ArgGet

      If the top level item is itself a scalar item of the correct name
      its type will be returned. 
      
      
 *  Call:
      ArgGetType id name
 

 *   Parameters: 
      (>) id  (int)  SDS identifier of structure.
      (>) name (string)  Name of component to get the type of.


 *  Returns:
      The value of the SDS object. 

 *  Language:
      Tcl

 *  See Also: DTCL manual, Sds manual.

 *  Support: Tony Farrell, AAO

 *-

 *  History:
      15-May-1995 - TJF - Original version
      05-Nov-1997 - TJF - Use new SdsTypeToStr routine.
 */

static int STclArgGetType(ClientData clientData , Tcl_Interp *interp, int argc, 
     char *argv[])

{
   StatusType status;
   char name[SDS_C_NAMELEN];
   SdsCodeType code;
   long ndims;
   unsigned long dims[SDS_C_MAXARRAYDIMS];
   SdsIdType id,tid=0;
   SdsTclCmdErrorRoutineType cmdError = (SdsTclCmdErrorRoutineType)clientData;

   if (argc != 3) 
     {
        char result[256];
        sprintf(result,"%s:wrong # args",argv[0]);
        Tcl_SetResult(interp,result,TCL_VOLATILE);
	return (*cmdError)(interp,0,SDS__TCL_ARGERR);
     }
   status = SDS__OK;
   id = (SdsIdType)(atol(argv[1]));
   SdsInfo(id,name,&code,&ndims,dims,&status);
   if (code == SDS_STRUCT)
   {
       SdsFind(id,argv[2],&tid,&status);
       id = tid;
       SdsInfo(id,name,&code,&ndims,dims,&status);
       SdsFreeId(tid,&status);
   }
   if (status == SDS__OK)
   {
       Tcl_SetResult(interp,(char *)SdsTypeToStr(code,1),TCL_VOLATILE);
       return TCL_OK; 
   }
   else
   {
	return (*cmdError)(interp,argv[0],status);
   }
}  
      
/*+                     A r g P u t S t r i n g
 *  Command name:
      ArgPutString

 *  Function:
      Put a character string item into an SDS structure

 *  Description:
      A character string item is written into a named component within
      an SDS structure. The component is created if it does not currently
      exist
      
 *  Call:
      ArgPutString id name value
 

 *   Parameters: 
      (>) id  (int)  SDS identifier of structure.
      (>) name (string)  Name of component to be written to.
      (>) value (string)  String to be written.


 *  Language:
      Tcl

 *  See Also: DTCL manual, Sds manual, ArgPutString(3), ArgGet(n), ArgNew(n)
                        ArgPutString(n) ArgPutx(n)


 *  Support: Jeremy Bailey, AAO

 *-

 *  History:
      29-Sep-1993 - JAB - Original version
 */
static int STclArgPutString(ClientData clientData , Tcl_Interp *interp, 
                            int argc, char *argv[])

{
   StatusType status;
   SdsTclCmdErrorRoutineType cmdError = (SdsTclCmdErrorRoutineType)clientData;

   if (argc != 4) 
     {
        char result[256];
        sprintf(result,"%s:wrong # args",argv[0]);
        Tcl_SetResult(interp,result,TCL_VOLATILE);
	return (*cmdError)(interp,0,SDS__TCL_ARGERR);
     }
   status = SDS__OK;
   ArgPutString((SdsIdType)(atol(argv[1])),argv[2],argv[3],&status);
   if (status == SDS__OK)
     {
       return TCL_OK; 
     }
   else
     {
	return (*cmdError)(interp,argv[0],status);
     }
}

/*+                     A r g P u t c
 *  Command name:
      ArgPutc

 *  Function:
      Put a character item into an SDS structure

 *  Description:
      A character item is written into a named component within
      an SDS structure. The component is created if it does not currently
      exist
      
 *  Call:
      ArgPutc id name value
 

 *   Parameters: 
      (>) id  (int)  SDS identifier of structure.
      (>) name (string)  Name of component to be written to.
      (>) value (int)  Value to be written.


 *  Language:
      Tcl

 *  See Also: DTCL manual, Sds manual, ArgPutc(3), ArgGet(n), ArgNew(n),
                        ArgPutString(n) ArgPutx(n)

 *  Support: Jeremy Bailey, AAO

 *-

 *  History:
      29-Sep-1993 - JAB - Original version
      24-Mar-1995 - TJF - Use DtclCmdError to report errors.
 */
static int STclArgPutc(ClientData clientData , Tcl_Interp *interp, int argc, 
     char *argv[])

{
   StatusType status;
   char i;
   SdsTclCmdErrorRoutineType cmdError = (SdsTclCmdErrorRoutineType)clientData;

   if (argc != 4) 
     {
        char result[256];
        sprintf(result,"%s:wrong # args",argv[0]);
        Tcl_SetResult(interp,result,TCL_VOLATILE);
	return (*cmdError)(interp,0,SDS__TCL_ARGERR);
     }
   status = SDS__OK;
   ArgCvt(argv[3],ARG_STRING,SDS_CHAR,&i,sizeof(char),&status);
   ArgPutc((SdsIdType)(atol(argv[1])),argv[2],i,&status);
   if (status == SDS__OK)
     {
       return TCL_OK; 
     }
   else
     {
	return (*cmdError)(interp,argv[0],status);
     }
}

/*+                     A r g P u t s
 *  Command name:
      ArgPuts

 *  Function:
      Put a short integer item into an SDS structure

 *  Description:
      A short integer item is written into a named component within
      an SDS structure. The component is created if it does not currently
      exist
      
 *  Call:
      ArgPuts id name value
 

 *   Parameters: 
      (>) id  (int)  SDS identifier of structure.
      (>) name (string)  Name of component to be written to.
      (>) value (int)  Value to be written.


 *  Language:
      Tcl

 *  See Also: DTCL manual, Sds manual, ArgPuts(3), ArgGet(n), ArgNew(n),
                        ArgPutString(n) ArgPutx(n)

 *  Support: Jeremy Bailey, AAO

 *-

 *  History:
      29-Sep-1993 - JAB - Original version
      24-Mar-1995 - TJF - Use DtclCmdError to report errors.
 */
static int STclArgPuts(ClientData clientData , Tcl_Interp *interp, int argc, 
     char *argv[])

{
   StatusType status;
   short i;
   SdsTclCmdErrorRoutineType cmdError = (SdsTclCmdErrorRoutineType)clientData;

   if (argc != 4) 
     {
        char result[256];
        sprintf(result,"%s:wrong # args",argv[0]);
        Tcl_SetResult(interp,result,TCL_VOLATILE);
	return (*cmdError)(interp,0,SDS__TCL_ARGERR);
     }
   status = SDS__OK;
   ArgCvt(argv[3],ARG_STRING,SDS_SHORT,&i,sizeof(short),&status);
   ArgPuts((SdsIdType)(atol(argv[1])),argv[2],i,&status);
   if (status == SDS__OK)
     {
       return TCL_OK; 
     }
   else
     {
	return (*cmdError)(interp,argv[0],status);
     }
}

/*+                     A r g P u t u s
 *  Command name:
      ArgPutus

 *  Function:
      Put an unsigned short integer item into an SDS structure

 *  Description:
      An unsigned short integer item is written into a named component within
      an SDS structure. The component is created if it does not currently
      exist
      
 *  Call:
      ArgPutus id name value
 

 *   Parameters: 
      (>) id  (int)  SDS identifier of structure.
      (>) name (string)  Name of component to be written to.
      (>) value (int)  Value to be written.


 *  Language:
      Tcl

 *  See Also: DTCL manual, Sds manual, ArgPutus(3), ArgGet(n), ArgNew(n),
                        ArgPutString(n) ArgPutx(n)

 *  Support: Jeremy Bailey, AAO

 *-

 *  History:
      29-Sep-1993 - JAB - Original version
      24-Mar-1995 - TJF - Use DtclCmdError to report errors.
 */
static int STclArgPutus(ClientData clientData , Tcl_Interp *interp, int argc, 
     char *argv[])

{
   StatusType status;
   unsigned short i;
   SdsTclCmdErrorRoutineType cmdError = (SdsTclCmdErrorRoutineType)clientData;

   if (argc != 4) 
     {
        char result[256];
        sprintf(result,"%s:wrong # args",argv[0]);
        Tcl_SetResult(interp,result,TCL_VOLATILE);
	return (*cmdError)(interp,0,SDS__TCL_ARGERR);
     }
   status = SDS__OK;
   ArgCvt(argv[3],ARG_STRING,SDS_USHORT,&i,sizeof(unsigned short),&status);
   ArgPutus((SdsIdType)(atol(argv[1])),argv[2],i,&status);
   if (status == SDS__OK)
     {
       return TCL_OK; 
     }
   else
     {
	return (*cmdError)(interp,argv[0],status);
     }
}


/*+                     A r g P u t u
 *  Command name:
      ArgPutu

 *  Function:
      Put an unsigned integer item into an SDS structure

 *  Description:
      A unsigned integer item is written into a named component within
      an SDS structure. The component is created if it does not currently
      exist
      
 *  Call:
      ArgPutu id name value
 

 *   Parameters: 
      (>) id  (int)  SDS identifier of structure.
      (>) name (string)  Name of component to be written to.
      (>) value (int)  Value to be written.


 *  Language:
      Tcl

 *  See Also: DTCL manual, Sds manual, ArgPutu(3), ArgGet(n), ArgNew(n),
                        ArgPutString(n) ArgPutx(n)


 *  Support: Jeremy Bailey, AAO

 *-

 *  History:
      29-Sep-1993 - JAB - Original version
      24-Mar-1995 - TJF - Use DtclCmdError to report errors.
 */
static int STclArgPutu(ClientData clientData , Tcl_Interp *interp, int argc, 
     char *argv[])

{
   StatusType status;
   unsigned i;
   SdsTclCmdErrorRoutineType cmdError = (SdsTclCmdErrorRoutineType)clientData;

   if (argc != 4) 
     {
        char result[256];
        sprintf(result,"%s:wrong # args",argv[0]);
        Tcl_SetResult(interp,result,TCL_VOLATILE);
	return (*cmdError)(interp,0,SDS__TCL_ARGERR);
     }
   status = SDS__OK;
   ArgCvt(argv[3],ARG_STRING,SDS_UINT,&i,sizeof(unsigned),&status);
   ArgPutu((SdsIdType)(atol(argv[1])),argv[2],i,&status);
   if (status == SDS__OK)
     {
       return TCL_OK; 
     }
   else
     {
	return (*cmdError)(interp,argv[0],status);
     }
}

/*+                     A r g P u t i
 *  Command name:
      ArgPuti

 *  Function:
      Put an integer item into an SDS structure

 *  Description:
      An integer item is written into a named component within
      an SDS structure. The component is created if it does not currently
      exist
      
 *  Call:
      ArgPuti id name value
 

 *   Parameters: 
      (>) id  (int)  SDS identifier of structure.
      (>) name (string)  Name of component to be written to.
      (>) value (int)  Value to be written.


 *  Language:
      Tcl

 *  See Also: DTCL manual, Sds manual, ArgPuti(3), ArgGet(n), ArgNew(n),
                        ArgPutString(n) ArgPutx(n)


 *  Support: Jeremy Bailey, AAO

 *-

 *  History:
      29-Sep-1993 - JAB - Original version
      24-Mar-1995 - TJF - Use DtclCmdError to report errors.
 */
static int STclArgPuti(ClientData clientData , Tcl_Interp *interp, int argc, 
     char *argv[])

{
   StatusType status;
   long int i;
   SdsTclCmdErrorRoutineType cmdError = (SdsTclCmdErrorRoutineType)clientData;

   if (argc != 4) 
     {
        char result[256];
        sprintf(result,"%s:wrong # args",argv[0]);
        Tcl_SetResult(interp,result,TCL_VOLATILE);
	return (*cmdError)(interp,0,SDS__TCL_ARGERR);
     }
   status = SDS__OK;
   ArgCvt(argv[3],ARG_STRING,SDS_INT,&i,sizeof(i),&status);
   ArgPuti((SdsIdType)(atol(argv[1])),argv[2],i,&status);
   if (status == SDS__OK)
     {
       return TCL_OK; 
     }
   else
     {
	return (*cmdError)(interp,argv[0],status);
     }
}



/*+                     A r g P u t f
 *  Command name:
      ArgPutf

 *  Function:
      Put a floating point item into an SDS structure

 *  Description:
      A floating point item is written into a named component within
      an SDS structure. The component is created if it does not currently
      exist
      
 *  Call:
      ArgPutf id name value
 

 *   Parameters: 
      (>) id  (int)  SDS identifier of structure.
      (>) name (string)  Name of component to be written to.
      (>) value (float)  Value to be written.


 *  Language:
      Tcl

 *  See Also: DTCL manual, Sds manual, ArgPutf(3), ArgGet(n), ArgNew(n),
                        ArgPutString(n) ArgPutx(n)


 *  Support: Jeremy Bailey, AAO

 *-

 *  History:
      29-Sep-1993 - JAB - Original version
      24-Mar-1995 - TJF - Use DtclCmdError to report errors.
 */
static int STclArgPutf(ClientData clientData , Tcl_Interp *interp, int argc, 
     char *argv[])

{
   StatusType status;
   float i;
   SdsTclCmdErrorRoutineType cmdError = (SdsTclCmdErrorRoutineType)clientData;

   if (argc != 4) 
     {
        char result[256];
        sprintf(result,"%s:wrong # args",argv[0]);
        Tcl_SetResult(interp,result,TCL_VOLATILE);
	return (*cmdError)(interp,0,SDS__TCL_ARGERR);
     }
   status = SDS__OK;
   ArgCvt(argv[3],ARG_STRING,SDS_FLOAT,&i,sizeof(float),&status);
   ArgPutf((SdsIdType)(atol(argv[1])),argv[2],i,&status);
   if (status == SDS__OK)
     {
       return TCL_OK; 
     }
   else
     {
	return (*cmdError)(interp,argv[0],status);
     }
}


/*+                     A r g P u t d
 *  Command name:
      ArgPutd

 *  Function:
      Put a double floating point item into an SDS structure

 *  Description:
      A double floating point item is written into a named component within
      an SDS structure. The component is created if it does not currently
      exist
      
 *  Call:
      ArgPutd id name value
 

 *   Parameters: 
      (>) id  (int)  SDS identifier of structure.
      (>) name (string)  Name of component to be written to.
      (>) value (double)  Value to be written.


 *  Language:
      Tcl

 *  See Also: DTCL manual, Sds manual, ArgPutd(3), ArgGet(n), ArgNew(n),
                        ArgPutString(n) ArgPutx(n)


 *  Support: Jeremy Bailey, AAO

 *-

 *  History:
      29-Sep-1993 - JAB - Original version
      24-Mar-1995 - TJF - Use DtclCmdError to report errors.
 */
static int STclArgPutd(ClientData clientData , Tcl_Interp *interp, int argc, 
     char *argv[])

{
   StatusType status;
   double i;
   SdsTclCmdErrorRoutineType cmdError = (SdsTclCmdErrorRoutineType)clientData;

   if (argc != 4) 
     {
        char result[256];
        sprintf(result,"%s:wrong # args",argv[0]);
        Tcl_SetResult(interp,result,TCL_VOLATILE);
	return (*cmdError)(interp,0,SDS__TCL_ARGERR);
     }
   status = SDS__OK;
   ArgCvt(argv[3],ARG_STRING,SDS_DOUBLE,&i,sizeof(double),&status);
   ArgPutd((SdsIdType)(atol(argv[1])),argv[2],i,&status);
   if (status == SDS__OK)
     {
       return TCL_OK; 
     }
   else
     {
	return (*cmdError)(interp,argv[0],status);
     }
}


/*+                     A r g D e l e t e
 *  Command name:
      ArgDelete

 *  Function:
      Delete an argument structure

 *  Description:
      Delete an SDS structure and free it's identifier.

 *  Call:
      ArgDelete id 
 

 *   Parameters: 
      (>) id  (int)  SDS identifier of structure to be deleted.


 *  Language:
      Tcl


 *  Support: Jeremy Bailey, AAO

 *  See Also: DTCL manual, Sds manual, ArgDelete(3), SdsFreeId(n),
		SdsFreeId(3), SdsDelete(3)

 *-

 *  History:
      29-Sep-1993 - JAB - Original version
      24-Mar-1995 - TJF - Use DtclCmdError to report errors.
 */
static int STclArgDelete(ClientData clientData , Tcl_Interp *interp, int argc, 
     char *argv[])

{
   StatusType status;
   SdsTclCmdErrorRoutineType cmdError = (SdsTclCmdErrorRoutineType)clientData;

   if (argc != 2) 
     {
        char result[256];
        sprintf(result,"%s:wrong # args",argv[0]);
        Tcl_SetResult(interp,result,TCL_VOLATILE);
	return (*cmdError)(interp,0,SDS__TCL_ARGERR);
     }
   status = SDS__OK;
   ArgDelete((SdsIdType)(atol(argv[1])),&status);
   if (status == SDS__OK)
     {
       return TCL_OK; 
     }
   else
     {
	return (*cmdError)(interp,argv[0],status);
     }
}

/*+                    S d s A r r a y C e l l
 *  Command name:
      SdsArrayCell

 *  Function:
	Return the contents of one cell of an Sds array.

 *  Description:
	The contents of the specified array cell are returned.  

	For scaler item arrays, the number of dimensions is not checked,
	except for there being no more then seven, since dimensions
	of scaler items don't really mean anything in Sds.

	For structured arrays, the number of dimensions supplied must be
	at least the number in the structure.  Excess dimensions are
	ignored.  Note that for structures, the indexes start from 1.

	For all scaler items, the contents of the specified cell are
	converted to a string.  If the array is an array of structures,
	then the Sds id of the cell is returned.  

 *  Call:
      SdsArrayCell id dim1 [dim2 dim3 dim4 dim5 dim6 dim7]
 

 *  Parameters: 
      (>) id (int)   The SDS identifier of the object 
      (>) dimn (int) The index to the cell.


 *  Language:
      Tcl
 *  See Also: DTCL manual, SdsArrayGet(n), SdsGet(n), SdsGet(3), Sds manual

 *  Support: Tony Farrell, AAO

 *-

 *  History:
      24-Jan-1994 - TJF - Original version
      17-Nov-1994 - TJF - If not indexing structures, then we can tolarate
			  numbers of dimensions not being equal to the
			  dimensions in the item.
      24-Mar-1995 - TJF - Use DtclCmdError to report errors.
      25-Mar-1998 - TJF - Register variable i now marked as int as the old
                          usage is not to be supported in a future version
                          of the C language.

 */
static int STclSdsArrayCell(ClientData clientData , Tcl_Interp *interp, 
                            int argc, char *argv[])

{
   StatusType status;
   char name[SDS_C_NAMELEN];
   SdsCodeType code;
   long ndims;
   unsigned long dims[SDS_C_MAXARRAYDIMS];
   SdsIdType id;
   SdsTclCmdErrorRoutineType cmdError = (SdsTclCmdErrorRoutineType)clientData;

   if (argc < 3) 
     {
        char result[256];
        sprintf(result,"%s:wrong # args",argv[0]);
        Tcl_SetResult(interp,result,TCL_VOLATILE);
	return (*cmdError)(interp,0,SDS__TCL_ARGERR);
     }
   status = SDS__OK;
   id = (SdsIdType)(atol(argv[1]));
   SdsInfo(id,name,&code,&ndims,dims,&status);

 
   if (code == SDS_STRUCT)
   {
      char result[256];
      register int i;
      unsigned long Indices[SDS_C_MAXARRAYDIMS];
      SdsIdType IndexedId;
      if ((argc - 2) < ndims)
      {
         sprintf(result,
		      "%s:Number of dimensions does not correspond",argv[0]);
            Tcl_SetResult(interp,result,TCL_VOLATILE);
         return (*cmdError)(interp,0,SDS__TCL_ARGERR);
      }
      for (i = 0 ; i < ndims; ++i)
         Indices[i] = atol(argv[i+2]);
      SdsCell(id,ndims,Indices,&IndexedId,&status);
      sprintf(result,"%ld",IndexedId);
      Tcl_SetResult(interp,result,TCL_VOLATILE);
   }
   else if ((argc - 2) > SDS_C_MAXARRAYDIMS)
   {
      char result[256];
      sprintf(result,"%s:Too many dimensions",argv[0]);
      Tcl_SetResult(interp,result,TCL_VOLATILE);
      return (*cmdError)(interp,0,SDS__TCL_ARGERR);
   }
   else
   {
/*
 *     We need to calculate the offset in too the data array. 
 *	 This is a bit compilicated as we must do the dimension calculations
 *	 our selves.
 */
       unsigned i;			/* Scratch 			    */
#      ifdef VAXC
#          pragma nostandard	/* Auto init of arrays	*/
#      endif
       unsigned long ShiftedDims[SDS_C_MAXARRAYDIMS] = 	
                                        /* Dimensions array shifted such    */
		       { 0,0,0,0,0,0,0};/* [6] is always the fasted moving  */
       unsigned long CellIndex[SDS_C_MAXARRAYDIMS] = 
                                        /* Cell index required with 	    */
		       { 0,0,0,0,0,0,0};/* corresponding shift		    */
#      ifdef VAXC
#          pragma standard
#      endif
       unsigned long Offsets[SDS_C_MAXARRAYDIMS];/* Offsets for each dimensions	    */
       unsigned shift = SDS_C_MAXARRAYDIMS - (argc -2);	/* Shift required     */
       unsigned long CellOffset;
/*
 *      Fill out ShiftedDims and CellIndex such that the elements of the array
 *	will always have the same meaning no matter what the dimensions of the
 *      array is.
 *
 *      Example A
 *              NOTE, SDS_C_MAXARRAYDIMS=7 at this stage.
 *		ndims = SDS_C_MAXARRAYDIMS, 
 *              dims[SDS_C_MAXARRAYDIMS] = 10,9,8,7,6,5,4
 *		shift = 0;
 *		index required     = i,j,k,l,m,n,o
 *		ShiftedDims[SDS_C_MAXARRAYDIMS]     = 10,9,8,7,6,5,4
 *		CellIndex[SDS_C_MAXARRAYDIMS]       = i,j,k,l,m,n,o
 *
 *      Example B
 *		ndims = 3, dims[SDS_C_MAXARRAYDIMS]  = 10,9,8
 *		shift = 4
 *		index required     = i,j,k
 *		ShiftedDims[SDS_C_MAXARRAYDIMS]     = 0,0,0,0,10,9,8
 *		CellIndex[SDS_C_MAXARRAYDIMS]       = 0,0,0,0,i,j,k
 *	Example C
 *		ndims = 1, dims[SDS_C_MAXARRAYDIMS] = 10
 *		shift = 6
 *		index required     = i
 *		ShiftedDims[SDS_C_MAXARRAYDIMS]     = 0,0,0,0,0,0,10
 *		CellIndex[SDS_C_MAXARRAYDIMS]       = 0,0,0,0,0,0,i
 *	  
 */
       for (i = 6 ; i >= shift ; --i)
       {
	    ShiftedDims[i] = dims[i-shift];
	    CellIndex[i]   = atol(argv[2+i-shift]);
       }
/*
 *     Discard the lowest number, it is not required.
 *
 *	Example A:  SiftedDims[0] = 0;
 *	Example B:  SiftedDims[4] = 0;
 *	Example C:  SiftedDims[6] = 0;
 */
       ShiftedDims[shift] = 0;	
/*
 *     Workout the offsets for each dimension.  Only 6 out of 7 are required.
 *	Example A:
 *		ShiftedDims[7]     = 0,9,8,7,6,5,4
 *		Offsets[7]         = 60480,6720,840,120,20,4,0
 *      Example B:
 *		ShiftedDims[7]     = 0,0,0,0,0,9,8
 *		Offsets[7]         = 0,0,0,0,72,8,0
 *      Example C:
 *		ShiftedDims[7]     = 0,0,0,0,0,0,0
 *		Offsets[7]         = 0,0,0,0,0,0,0
 */
       Offsets[6] = 0;
       Offsets[5] = ShiftedDims[6];
       Offsets[4] = ShiftedDims[5] * Offsets[5];
       Offsets[3] = ShiftedDims[4] * Offsets[4];
       Offsets[2] = ShiftedDims[3] * Offsets[3];
       Offsets[1] = ShiftedDims[2] * Offsets[2];
       Offsets[0] = ShiftedDims[1] * Offsets[1];
/*
 *     Now determine our offset.
 *	Example A:
 *		Offsets[7]         = 60480,6720,840,120,20,4,0
 *		CellIndex[7]       = i,j,k,l,m,n,o
 *		CellOffset         = i*60480 + j * 6720 + k * 840 +
 *				     l * 120 + m * 20 + n * 4 + o
 *		
 *	Example B:
 *		Offsets[7]         = 0,0,0,0,72,8,0
 *		CellIndex[7]       = 0,0,0,0,i,j,k
 *		CellOffset	   = 72*m + 8*j + k
 *	Example C:
 *		Offsets[7]         = 0,0,0,0,0,0,0
 *		CellIndex[7]       = 0,0,0,0,0,0,i
 *		CellOffset	   = i
 */
       CellOffset = Offsets[0]*CellIndex[0] + Offsets[1]*CellIndex[1] +
                    Offsets[2]*CellIndex[2] + Offsets[3]*CellIndex[3]  +
                    Offsets[4]*CellIndex[4] + Offsets[5]*CellIndex[5]  +
		    CellIndex[6];
/*
 *     We now have the index in terms of unit element size into the array.
 *	We need to get the item and convert it to a string.  
 *
 */
	switch (code)
	{
	
	    case SDS_FLOAT:
	    case SDS_DOUBLE:
	    {
		double i;
		unsigned long actlen;
      char result[256];
		SdsGet(id,sizeof(i),CellOffset,&i,&actlen,&status);
		ArgCvt(&i,code,ARG_STRING,result,sizeof(result),&status);
      if (status == STATUS__OK) Tcl_SetResult(interp,result,TCL_VOLATILE);
      else Tcl_SetResult(interp,"",TCL_STATIC);
		break;
            }
	    default:
	    {
		long i;
		unsigned long actlen;
      char result[256];
		SdsGet(id,sizeof(i),CellOffset,&i,&actlen,&status);
		ArgCvt(&i,code,ARG_STRING,result,sizeof(result),&status);
      if (status == STATUS__OK) Tcl_SetResult(interp,result,TCL_VOLATILE);
      else Tcl_SetResult(interp,"",TCL_STATIC);
		break;
	    }
	}
   }  
   if (status == SDS__OK)
     {
       return TCL_OK; 
     }
   else
     {
	return (*cmdError)(interp,argv[0],status);
     }
}        
/*+                     S d s A r r a y G e t
 *  Command name:
      SdsArrayGet

 *  Function:
	Return the contents of a 1 dimensional array

 *  Description:
	If the supplied Sds id describes a one dimensional array, return its
	contents converted to a string in the specified variable.  If
	the object is an array of structures, then the ids' of the
	structures are returned.

	By default, an array is setup in the variable who's name is specified.
        The element numbers are integers starting from 0.

	The -list option will cause the result to be a list.
	

 *  Call:
      SdsArrayGet id name [options]
 

 *   Parameters: 
      (>) id  (int)  SDS identifier of structure.
      (>) name (string) The name of a variable in which the array contents
			will be returned.

*    Options:
        [-list]     The result variable should be setup as a list instead of
                   an array.
	[-string]   As a special case, treat a 2 dimensional array of
			characters as an array of strings, where the first
			index is the maximum size of each string and
			the second is the number of items in the string array.
	[-stringr] As per -string, but the dimension elements are reversed,
			the first being the number of elements in the array
			and the second the maximum size of each string.
	[-element arrayelem] If name refers to a array variable and we
			want the result to be put in this element of this
			array.  For -list is not specified, we construct
			new array element names of the form arrayelem,n
			where n is the index.


 *  Language:
      Tcl

 *  See Also: DTCL manual, SdsGet(n), SdsArrayCell(3), SdsGet(3), Sds manual

 *  Support: Tony Farrell, AAO

 *-

 *  History:
      25-Jan-94 - TJF - Original version
      14-Dec-94 - TJF - Add -list option.
      22-Feb-95 - TJF - Support -string and -stringr options.  Access
			arrays using pointer instead of SdsGet
      24-Mar-1995 - TJF - Use DtclCmdError to report errors.
      08-Apr-1997 - TJf - Add -element option.
      25-Mar-1998 - TJF - Register variable i now marked as unsigned as the old
                          usage is not to be supported in a future version
                          of the C language.  Must cast argc to unsigned
			  to avoid signed/unsigned comparsion.
     05-Aug-1998 - TJF - When output is list, was incorrectly calling
			Tcl_SetVar2 in some cases, causing a segementation
			fault.
 */
static int STclSdsArrayGet(ClientData clientData , Tcl_Interp *interp, 
                           int argc,  char *argv[])

{
   StatusType status;
   SdsIdType id;
   SdsCodeType code;
   char name[SDS_C_NAMELEN];
   long ndims;
   unsigned long dims[SDS_C_MAXARRAYDIMS];
   int list = 0;
   int string= 0;
   int reverse_dims = 0;
   register unsigned int i;
   char *element = NULL;
   SdsTclCmdErrorRoutineType cmdError = (SdsTclCmdErrorRoutineType)clientData;


    if (argc < 3)
    {
        char result[256];
        sprintf(result,"%s:wrong # args",argv[0]);
        Tcl_SetResult(interp,result,TCL_VOLATILE);
	return (*cmdError)(interp,0,SDS__TCL_ARGERR);
    }
    for ( i = 3; i < (unsigned)argc ; ++ i )
    {
       if (strcmp(argv[i],"-list") == 0)
	    list = 1;
       else if (strcmp(argv[i],"-string") == 0)
	    string = 1;
       else if (strcmp(argv[i],"-element") == 0)
       {
	    i++;
	    if (i < (unsigned)argc)
	         element = argv[i];
       }
       else if (strcmp(argv[i],"-stringr") == 0)
       {
	    string = 1;
	    reverse_dims = 1;
       }
       else
       {
             char result[256];
             sprintf(result,"%s:Unknown option - \"%s\"\n",
                        argv[0],argv[i]);
             Tcl_SetResult(interp,result,TCL_VOLATILE);
	    return (*cmdError)(interp,0,SDS__TCL_ARGERR);
       }
    }

   status = SDS__OK;
   id = (SdsIdType)(atol(argv[1]));

   SdsInfo(id,name,&code,&ndims,dims,&status);
   if (status != SDS__OK)
   {
	return (*cmdError)(interp,argv[0],status);
   }
   else if (string && (ndims == 2) && (code == SDS_CHAR))
   {
	code = ARG_STRING;	/* Change type		*/
	if (reverse_dims)	/* Reverse dimensions if necessary */
	{	
	    int s = dims[0];
	    dims[0] = dims[1];
	    dims[1] = s;
	}
   }
   else if (ndims != 1)
   {
	char result[256];
	sprintf(result,
		"%s:Invalid Sds item array dimensions, must be 1",argv[0]);
	Tcl_SetResult(interp,result,TCL_VOLATILE);
	return (*cmdError)(interp,0,SDS__TCL_ARGERR);
   } 
/*
 *  Bracket for convience;
 */
   {
       unsigned long actlen;
       char index[200];
       char value[TCL_RESULT_SIZE];

/*
 *     If setting up as a list, we must unset the variable if it already
 *     exists.
 */
       if (list)
	    Tcl_UnsetVar(interp,argv[2],0);
/*
 *     Get the values and put them in the array. STRUCT arrays are a special
 *	case where we return the Id of each cell in the struct.  Otherwise,
 *	we get the contents of the array and create an element in the
 *	requested variable for each value.
 *
 *	Note, since the conversion to a string is done by ArgCvt, using the
 *	Sds code, all types of the same size can be handled in the one 
 *	case statement.
 */
       switch (code)
       {
 	    case SDS_STRUCT:	/* We use SdsCell to get the id of each cell */
	    {
		SdsIdType StructId;
		unsigned long reqdims[1];
		for (i = 0 ; i < dims[0] ; ++ i)
		{
		    reqdims[0] = i;
		    SdsCell(id,1,reqdims,&StructId,&status);

		    if (element)
		        sprintf(index,"%s,%d",element,i);
		    else
		        sprintf(index,"%d",i);
		    sprintf(value,"%d",i);
		    if (list)
		    {
			if (Tcl_SetVar2(interp, argv[2], element, value,
				TCL_APPEND_VALUE |TCL_LIST_ELEMENT|
				TCL_LEAVE_ERR_MSG) == NULL)
			    return (*cmdError)(interp,0,SDS__TCL_CMDERR);
		    }
 		    else if (Tcl_SetVar2(interp, argv[2], index, value, 0) == NULL)
			return (*cmdError)(interp,0,SDS__TCL_CMDERR);
		}
		break;
	    }
	    case SDS_BYTE:
	    case SDS_CHAR:
	    case SDS_UBYTE:
	    {
		char *b;
		SdsPointer(id,(void **)&b,&actlen,&status);
		for (i = 0 ; i < dims[0] ; ++ i)
		{
                    if (element)
                        sprintf(index,"%s,%d",element,i);
                    else
                        sprintf(index,"%d",i);

		    ArgCvt(&b[i],code,ARG_STRING,value,
			     TCL_RESULT_SIZE,&status);
		    if (list)
		    {
		        if (Tcl_SetVar2(interp, argv[2], element, value,
				 TCL_APPEND_VALUE
				|TCL_LIST_ELEMENT|TCL_LEAVE_ERR_MSG) == NULL)
		   	    return (*cmdError)(interp,0,SDS__TCL_CMDERR);
		    }
 		    else  if (Tcl_SetVar2(interp, argv[2], index, value, 0) == NULL)
			return (*cmdError)(interp,0,SDS__TCL_CMDERR);
		}
		break;
	    }
	    case SDS_SHORT:
	    case SDS_USHORT:
	    {
		short *b;
		SdsPointer(id,(void **)&b,&actlen,&status);
		for (i = 0 ; i < dims[0] ; ++ i)
		{
                    if (element)
                        sprintf(index,"%s,%d",element,i);
                    else
                        sprintf(index,"%d",i);
 
		    ArgCvt(&b[i],code,ARG_STRING,value,
			TCL_RESULT_SIZE,&status);
		    if (list)
		    {
		        if (Tcl_SetVar2(interp, argv[2], element, value,
				TCL_APPEND_VALUE
				|TCL_LIST_ELEMENT|TCL_LEAVE_ERR_MSG) == NULL)
			    return (*cmdError)(interp,0,SDS__TCL_CMDERR);
		    }
 		    else  if (Tcl_SetVar2(interp, argv[2], index, value, 0) == NULL)
			return (*cmdError)(interp,0,SDS__TCL_CMDERR);
		}
		break;
	    }
	    case SDS_UINT:
	    case SDS_INT:
	    {
		long *b;
		SdsPointer(id,(void **)&b,&actlen,&status);
		for (i = 0 ; i < dims[0] ; ++ i)
		{
                    if (element)
                        sprintf(index,"%s,%d",element,i);
                    else
                        sprintf(index,"%d",i);
 
		    ArgCvt(&b[i],code,ARG_STRING,value,
			TCL_RESULT_SIZE,&status);
		    if (list)
		    {
		        if (Tcl_SetVar2(interp, argv[2], element, value, 
				TCL_APPEND_VALUE
				|TCL_LIST_ELEMENT|TCL_LEAVE_ERR_MSG) == NULL)
			    return (*cmdError)(interp,0,SDS__TCL_CMDERR);
		    }
 		    else  if (Tcl_SetVar2(interp, argv[2], index, value, 0) == NULL)
			return (*cmdError)(interp,0,SDS__TCL_CMDERR);
		}
		break;
	    }
	    case SDS_FLOAT:
	    {
		float *b;
		SdsPointer(id,(void **)&b,&actlen,&status);

		for (i = 0 ; i < dims[0] ; ++ i)
		{
                    if (element)
                        sprintf(index,"%s,%d",element,i);
                    else
                        sprintf(index,"%d",i);
 
		    ArgCvt(&b[i],code,ARG_STRING,value,
			TCL_RESULT_SIZE,&status);
		    if (list)
		    {
		        if (Tcl_SetVar2(interp, argv[2], element, value,
				 TCL_APPEND_VALUE
				|TCL_LIST_ELEMENT|TCL_LEAVE_ERR_MSG) == NULL)
			    return (*cmdError)(interp,0,SDS__TCL_CMDERR);
		    }
 		    else  if (Tcl_SetVar2(interp, argv[2], index, value, 0) == NULL)
			return (*cmdError)(interp,0,SDS__TCL_CMDERR);
		}
		break;
	    }
	    case SDS_DOUBLE:
	    {
		double *b;
		SdsPointer(id,(void **)&b,&actlen,&status);

		for (i = 0 ; i < dims[0] ; ++ i)
		{
                    if (element)
                        sprintf(index,"%s,%d",element,i);
                    else
                        sprintf(index,"%d",i);
 
		    ArgCvt(&b[i],code,ARG_STRING,value,
			TCL_RESULT_SIZE,&status);
		    if (list)
		    {
		        if (Tcl_SetVar2(interp, argv[2], element, value,
				 TCL_APPEND_VALUE
				|TCL_LIST_ELEMENT|TCL_LEAVE_ERR_MSG) == NULL)
			return (*cmdError)(interp,0,SDS__TCL_CMDERR);
		    }
 		    else  if (Tcl_SetVar2(interp, argv[2], index, value, 0) == NULL)
			return (*cmdError)(interp,0,SDS__TCL_CMDERR);
		}
		break;
	    }
/*
 *	    Special case, array is an array of strings.
 */
	    case ARG_STRING:
	    {
		char *v = (void *)malloc(dims[0]+1); /* Space for each string */
		char *b;
		SdsPointer(id,(void **)&b,&actlen,&status);
		for (i = 0 ; i < dims[1] ; ++ i)
		{
		    int offset = dims[0]*i;	/* Offset of this item */
		    memcpy(v,&b[offset],dims[0]);	/* get item */
		    v[dims[0]] = '\0';	/* Ensure null termination */
                    if (element)
                        sprintf(index,"%s,%d",element,i);
                    else
                        sprintf(index,"%d",i);
 
		    if (list)
		    {
		        if (Tcl_SetVar2(interp, argv[2], element, v,
			 	 TCL_APPEND_VALUE
				|TCL_LIST_ELEMENT|TCL_LEAVE_ERR_MSG) == NULL)
	   	  	    return (*cmdError)(interp,0,SDS__TCL_CMDERR);
 		    }
 		    else  if (Tcl_SetVar2(interp, argv[2], index, v, 0) == NULL)
			return (*cmdError)(interp,0,SDS__TCL_CMDERR);
		}
	        free((void *)v);
		break;

	    }
         }
   }

   if (status != SDS__OK)
   {
	return (*cmdError)(interp,argv[0],status);
   }
   else
	return TCL_OK;
  
}

/*+                     a r g s
 *  Command name:
      args

 *  Function:
      Create argument structure containing a number of strings

 *  Description:
      An SDS structure is created and components in the structure are
      created containing each of the arguments of the args command.

      It returns the SDS identifier of the structure.

 *  Call:
      args arg1 arg2 .....
 
 *   Parameters: 
      (>) args  (int)  Strings to be inserted into the SDS structure.


 *  Returns:
      The SDS identifier of the created structure.

 *  Language:
      Tcl

 *  See Also: DTCL manual, ArgNew(n), ArgNew(3), ArgPutx(n), ArgDelete(n).

 *-

 *  Support: Jeremy Bailey, AAO

 *  History:
      29-Sep-1993 - JAB - Original version
      24-Mar-1995 - TJF - Use DtclCmdError to report errors.
 */
static int STclArgs(ClientData clientData , Tcl_Interp *interp, int argc, 
     char *argv[])

{
   StatusType status;
   SdsIdType id;
   char name[20];
   int i;
   SdsTclCmdErrorRoutineType cmdError = (SdsTclCmdErrorRoutineType)clientData;

   status = SDS__OK;
   ArgNew(&id,&status);
   for (i=1;i<argc;i++)
     {
       sprintf(name,"Argument%d",i);
       ArgPutString(id,name,argv[i],&status);
     }
   if (status == SDS__OK)
     {
       char result[256];
       sprintf(result,"%ld",(long int)id);
       Tcl_SetResult(interp,result,TCL_VOLATILE);
       return TCL_OK; 
     }
   else
     {
	return (*cmdError)(interp,argv[0],status);
     }
}

/*+                     S d s L i s t
 *  Command name:
      SdsList

 *  Function:
      List contents of an SDS object

 *  Description:
      A listing of the contents of an SDS object is generated on standard
      output. The listing consists of the name, type, dimensions and value
      of each object in the structure. The hierarchical structure is
      indicated by indenting the listing for components at each level.

      For array objects only the values of the first few components are
      listed so that the listing for each object fits on a single line.
      

 *  Call:
      SdsList id [options]
 

*   Parameters: 
      (>) id  (int)   The SDS identifier of the object.
      
*    Options:
        [-printcmd cmd]  A command to be invoked to print each line of the
        		listing.  If specified, we call this command with
        		the string appended for each line.
	[-linelen n]  When -printcmd is specified, this is the maximum number
			of characters to print.

 *  Language:
      Tcl

 *  See Also: DTCL manual, SdsList(3), ArgSdsList(3), Sds manual.

 *  Support: Jeremy Bailey, AAO

 *-

 *  History:
      29-Sep-1993 - JAB - Original version
      24-Mar-1995 - TJF - Use DtclCmdError to report errors.
      18-Apr-1997 - TJF - Add -printcmd and -linelen options to make use
      			of the new ArgSdsList function.
      25-Mar-1998 - TJF - Register variable i now marked as int as the old
                          usage is not to be supported in a future version
                          of the C language.
       1-Sep-1999 - KS  - Removed ARGCONST from STcl___SdsListPrint() status
                          argument. It may not change it, but with the const
                          in it is no longer a precise match for an ArgListFuncType
                          routine and some fussy compilers don't like this.

 */
typedef struct {	
	Tcl_Interp *interp;
	char *command;
} STclSdsListType;

static void STcl___SdsListPrint(void * clientData, const char *line, 
                                StatusType * status) 
{
   STclSdsListType *info = clientData;
   Tcl_DString command;

   Tcl_DStringInit(&command);
   Tcl_DStringAppend(&command,info->command,-1);
   Tcl_DStringAppendElement(&command,(char *)line);
   if (Tcl_Eval(info->interp,command.string) != TCL_OK)
   {
       char s[200];
       sprintf(s,"\n(SdsList output routine \"%s\")",info->command);
       Tcl_AddErrorInfo(info->interp,s);
       *status = SDS__TCL_CMD;
       STclErsRep(info->interp,status);
   }
   Tcl_DStringFree(&command);
    
}

    
static int STclSdsList(ClientData clientData , Tcl_Interp *interp, int argc, 
     char *argv[])

{
   StatusType status;
   STclSdsListType info;
   int useArgSdsList = 0;
   int bufsize = 0;
   register int i;
   SdsTclCmdErrorRoutineType cmdError = (SdsTclCmdErrorRoutineType)clientData;

   if (argc < 2) 
   {
        char result[256];
        sprintf(result,"%s:wrong # args",argv[0]);
        Tcl_SetResult(interp,result,TCL_VOLATILE);
	return (*cmdError)(interp,0,SDS__TCL_ARGERR);
   }
   for (i = 2 ; i < argc ; ++i)
   {
   	if (i+1 == argc)
   	{
             char result[256];
             sprintf(result,
                        "%s:No value for option \"%s\"\n",argv[i],argv[i]);
             Tcl_SetResult(interp,result,TCL_VOLATILE);
             return (*cmdError)(interp,0,SDS__TCL_ARGERR);
        }
        else if (strcmp(argv[i],"-printcmd") == 0)
        {
            ++i;
   	    useArgSdsList = 1;
            info.command = argv[i];
        }
        else if (strcmp(argv[i],"-linelen") == 0)
        { 
            ++i;
            bufsize = atoi(argv[i]);
        }
        else
        {
             char result[256];
             sprintf(result,
                        "TranslateName:Unknown option - \"%s\"\n",argv[i]);
             Tcl_SetResult(interp,result,TCL_VOLATILE);
             return (*cmdError)(interp,0,SDS__TCL_ARGERR);
        }
   }
   status = SDS__OK;
   if (useArgSdsList)
   {
       info.interp = interp;
       ArgSdsList((SdsIdType)(atol(argv[1])),bufsize,0,
       			STcl___SdsListPrint,&info,&status);
   }
   else
       SdsList((SdsIdType)(atol(argv[1])),&status);
   if (status == SDS__OK)
     {
       return TCL_OK; 
     }
   else
     {
	return (*cmdError)(interp,argv[0],status);
     }
}        

/*+                     S d s R e a d
 *  Command name:
      SdsRead

 *  Function:
      Read an SDS object from a file

 *  Description:
      An Sds object is read from a file and the identifier of the created
      external object is returned.
      

 *  Call:
      SdsRead file
 

 *  Parameters: 
      (>) file  (string)   The name of the file to be read.

 *  Returns:
      The SDS identifier of the resulting external object.  


 *  Language:
      Tcl

 *  See Also: DTCL manual, SdsWrite(n), SdsReadFree(n), SdsRead(3), Sds manual.

 *  Support: Jeremy Bailey, AAO

 *-

 *  History:
      29-Sep-1993 - JAB - Original version
      24-Mar-1995 - TJF - Use DtclCmdError to report errors.
 */
static int STclSdsRead(ClientData clientData , Tcl_Interp *interp, int argc, 
     char *argv[])

{
   SdsIdType id;
   StatusType status;
   SdsTclCmdErrorRoutineType cmdError = (SdsTclCmdErrorRoutineType)clientData;

   if (argc != 2) 
     {
        char result[256];
        sprintf(result,"%s:wrong # args",argv[0]);
        Tcl_SetResult(interp,result,TCL_VOLATILE);
	return (*cmdError)(interp,0,SDS__TCL_ARGERR);
     }
   status = SDS__OK;
   SdsRead(argv[1],&id,&status);
   if (status == SDS__OK)
     {
       char result[256];
       sprintf(result,"%ld",(long int)id);
       Tcl_SetResult(interp,result,TCL_VOLATILE);
       return TCL_OK; 
     }
   else
     {
	return (*cmdError)(interp,argv[0],status);
     }
}        
/*+                     S d s R e a d F r e e 
 *  Command name:
      SdsReadFree

 *  Function:
      Free Buffer allocated by SdsRead

 *  Description:
      SdsRead allocates a block of memory to hold the external object read in.
      This memory can be released when the object is no longer required by 
      calling  SdsReadFree (note that it is not possible to SdsDelete an 
      external object).

      If SdsReadFree is given an idientifier which was not produced by a call to
      SdsRead it will do nothing.
      

 *  Call:
      SdsReadFree id
 

 *  Parameters: 
      (>) id  (int)   The SDS identifier of the object.

 *  Language:
      Tcl

 *  See Also: DTCL manual, SdsRead(n), SdsReadFree(3), Sds manual.

 *  Support: Tony Farrell, AAO

 *-

 *  History:
      28-Aug-1995 - TJF - Original Version.
 */
static int STclSdsReadFree(ClientData clientData , Tcl_Interp *interp, int argc, 
     char *argv[])

{
   StatusType status;
   SdsTclCmdErrorRoutineType cmdError = (SdsTclCmdErrorRoutineType)clientData;

   if (argc != 2) 
     {
        char result[256];
        sprintf(result,"%s:wrong # args",argv[0]);
        Tcl_SetResult(interp,result,TCL_VOLATILE);
	return (*cmdError)(interp,0,SDS__TCL_ARGERR);
     }
   status = SDS__OK;
   SdsReadFree((SdsIdType)(atol(argv[1])),&status);
   if (status == SDS__OK)
     {
       return TCL_OK; 
     }
   else
     {
	return (*cmdError)(interp,argv[0],status);
     }
}        

/*+                     S d s W r i t e
 *  Command name:
      SdsWrite

 *  Function:
      Write an SDS object to a file

 *  Description:
      Write an SDS object to a file - the object can be read back using
      SdsRead

 *  Call:
      SdsWrite id file
 

*   Parameters: 
      (>) id  (int)   The SDS identifier of the object.
      (>) file (string)  The name of the file to be written.

 *  Language:
      Tcl

 *  See Also: DTCL manual, SdsWrite(3), SdsRead(n), Sds manual.

 *  Support: Jeremy Bailey, AAO

 *-

 *  History:
      19-Oct-1993 - JAB - Original version
      24-Mar-1995 - TJF - Use DtclCmdError to report errors.
 */
static int STclSdsWrite(ClientData clientData , Tcl_Interp *interp, int argc, 
     char *argv[])

{
   StatusType status;
   SdsTclCmdErrorRoutineType cmdError = (SdsTclCmdErrorRoutineType)clientData;

   if (argc != 3) 
     {
        char result[256];
        sprintf(result,"%s:wrong # args",argv[0]);
        Tcl_SetResult(interp,result,TCL_VOLATILE);
	return (*cmdError)(interp,0,SDS__TCL_ARGERR);
     }
   status = SDS__OK;
   SdsWrite((SdsIdType)(atol(argv[1])),argv[2],&status);
   if (status == SDS__OK)
     {
       return TCL_OK; 
     }
   else
     {
	return (*cmdError)(interp,argv[0],status);
     }
}        

/*+                     S d s F i n d
 *  Command name:
      SdsFind

 *  Function:
      Find an SDS structure component by name

 *  Description:
      Given the name of a component in an SDS structure return an 
      identifier to the component.

 *  Call:
      SdsFind id name
 

 *  Parameters: 
      (>) id (int)   The SDS identifier of the structure.
      (>) name (string)  The name of the component.

 *  Returns:
      The SDS identifier of the component.  


 *  Language:
      Tcl

 *  See Also: DTCL manual, SdsFind(3), Sds manual.

 *  Support: Jeremy Bailey, AAO

 *-

 *  History:
      19-Oct-1993 - JAB - Original version
      24-Mar-1995 - TJF - Use DtclCmdError to report errors.
 */
static int STclSdsFind(ClientData clientData , Tcl_Interp *interp, int argc, 
     char *argv[])

{
   SdsIdType id;
   StatusType status;
   SdsTclCmdErrorRoutineType cmdError = (SdsTclCmdErrorRoutineType)clientData;

   if (argc != 3) 
     {
        char result[256];
        sprintf(result,"%s:wrong # args",argv[0]);
        Tcl_SetResult(interp,result,TCL_VOLATILE);
	return (*cmdError)(interp,0,SDS__TCL_ARGERR);
     }
   status = SDS__OK;
   SdsFind(atol(argv[1]),argv[2],&id,&status);
   if (status == SDS__OK)
     {
       char result[256];
       sprintf(result,"%ld",(long int)id);
       Tcl_SetResult(interp,result,TCL_VOLATILE);
       return TCL_OK; 
     }
   else
     {
       char result[256];
       sprintf(result,"%s:Error finding SDS component - %s",
			argv[0],argv[2]);
       Tcl_SetResult(interp,result,TCL_VOLATILE);
       return (*cmdError)(interp,0,status);

     }
}        


/*+                     S d s I n d e x
 *  Command name:
      SdsIndex

 *  Function:
      Find an SDS structure component by position

 *  Description:
      Given the index number of a component in an SDS structure return an 
      identifier to the component.

 *  Call:
      SdsIndex id index
 

 *  Parameters: 
      (>) id (int)   The SDS identifier of the structure.
      (>) index (int)  Index number of the component to be returned
                       Items in a structure are numbered in order of
                       creation starting with one.

 *  Returns:
      The SDS identifier of the component.  


 *  Language:
      Tcl

 *  See Also: DTCL manual, SdsIndex(3), Sds manual

 *  Support: Jeremy Bailey, AAO

 *-

 *  History:
      7-Apr-1994 - JAB - Original version
      24-Mar-1995 - TJF - Use DtclCmdError to report errors.
 */
static int STclSdsIndex(ClientData clientData , Tcl_Interp *interp, int argc, 
     char *argv[])

{
   SdsIdType id;
   StatusType status;
   SdsTclCmdErrorRoutineType cmdError = (SdsTclCmdErrorRoutineType)clientData;

   if (argc != 3) 
     {
        char result[256];
        sprintf(result,"%s:wrong # args",argv[0]);
        Tcl_SetResult(interp,result,TCL_VOLATILE);
	return (*cmdError)(interp,0,SDS__TCL_ARGERR);
     }
   status = SDS__OK;
   SdsIndex(atol(argv[1]),atol(argv[2]),&id,&status);
   if (status == SDS__OK)
     {
       char result[256];
       sprintf(result,"%ld",(long int)id);
       Tcl_SetResult(interp,result,TCL_VOLATILE);
       return TCL_OK; 
     }
   else
     {
       char result[256];
       sprintf(result,"%s:Error finding SDS component - %s",
			argv[0],argv[2]);
       Tcl_SetResult(interp,result,TCL_VOLATILE);
       return (*cmdError)(interp,0,status);
     }
}        


/*+                     S d s C o p y
 *  Command name:
      SdsCopy

 *  Function:
      Make a copy of an SDS object

 *  Description:
      Make a copy of an object and return an identifier to the copy.
      The copy is a new top level object, the original object is unchanged
      by the operation.

      The object being copied can be either external or internal. The
      copy is always an internal object.

 *  Call:
      SdsCopy id
 

 *  Parameters: 
      (>) id (int)   The SDS identifier of the object to be copied.

 *  Returns:
      The SDS identifier of the copy.  


 *  Language:
      Tcl

 *  See Also: DTCL manual, SdsCopy(3), Sds manual

 *  Support: Jeremy Bailey, AAO

 *-

 *  History:
      19-Oct-1993 - JAB - Original version
      24-Mar-1995 - TJF - Use DtclCmdError to report errors.
 */
static int STclSdsCopy(ClientData clientData , Tcl_Interp *interp, int argc, 
     char *argv[])

{
   SdsIdType id;
   StatusType status;
   SdsTclCmdErrorRoutineType cmdError = (SdsTclCmdErrorRoutineType)clientData;

   if (argc != 2) 
     {
        char result[256];
        sprintf(result,"%s:wrong # args",argv[0]);
        Tcl_SetResult(interp,result,TCL_VOLATILE);
	return (*cmdError)(interp,0,SDS__TCL_ARGERR);
     }
   status = SDS__OK;
   SdsCopy(atol(argv[1]),&id,&status);
   if (status == SDS__OK)
     {
       char result[256];
       sprintf(result,"%ld",(long int)id);
       Tcl_SetResult(interp,result,TCL_VOLATILE);
       return TCL_OK; 
     }
   else
     {
       char result[256];
       sprintf(result,"%s:Error copying SDS object.",argv[0]);
       Tcl_SetResult(interp,result,TCL_VOLATILE);
       return (*cmdError)(interp,0,status);
     }
}        

/*+                     S d s I n s e r t
 *  Command name:
      SdsInsert

 *  Function:
      Insert an existing SDS object into a structure

 *  Description:
      An existing top level object is inserted into a structure

 *  Call:
      SdsInsert parent_id id
 

 *  Parameters: 
      (>) parent_id (int)  The identifier of the structure to which the
                          component is to be added.
      (>) id (int)   The SDS identifier of the object to be inserted.


 *  Language:
      Tcl

 *  See Also: DTCL manual, SdsInsert(3), Sds manual.

 *  Support: Jeremy Bailey, AAO

 *-

 *  History:
      19-Oct-1993 - JAB - Original version
 */
static int STclSdsInsert(ClientData clientData , Tcl_Interp *interp, int argc, 
     char *argv[])

{
   StatusType status;
   SdsTclCmdErrorRoutineType cmdError = (SdsTclCmdErrorRoutineType)clientData;

   if (argc != 3) 
     {
        char result[256];
        sprintf(result,"%s:wrong # args",argv[0]);
        Tcl_SetResult(interp,result,TCL_VOLATILE);
	return (*cmdError)(interp,0,SDS__TCL_ARGERR);
     }
   status = SDS__OK;
   SdsInsert(atol(argv[1]),atol(argv[2]),&status);
   if (status == SDS__OK)
     {
       return TCL_OK; 
     }
   else
     {
       char result[256];
       sprintf(result,"%s:Error inserting SDS object.",argv[0]);
       Tcl_SetResult(interp,result,TCL_VOLATILE);
       return (*cmdError)(interp,0,status);
     }
}        
/*+                     S d s E x t r a c t
 *  Command name:
      SdsExtract

 *  Function:
      Extract an object from a structure.

 *  Description:
      
      Extract an object from a structure. The extracted object becomes a 
      new independent top level object. The object is deleted from the 
      original structure. 

      The identifier must not be that of a structure array component. 

      If the identifier is already that of a top level object, then the 
      function does nothing

 *  Call:
      SdsExtract id
 

 *  Parameters: 
      (>) id (int)   The SDS identifier of the object to be extracted


 *  Language:
      Tcl

 *  See Also: DTCL manual, SdsExtract(3), Sds manual.

 *  Support: Jeremy Bailey, AAO

 *-

 *  History:
      05-Nov-1997 - JAB - Original version
 */
static int STclSdsExtract(ClientData clientData , Tcl_Interp *interp, int argc, 
     char *argv[])

{
   StatusType status;
   SdsTclCmdErrorRoutineType cmdError = (SdsTclCmdErrorRoutineType)clientData;

   if (argc != 2) 
     {
        char result[256];
        sprintf(result,"%s:wrong # args",argv[0]);
        Tcl_SetResult(interp,result,TCL_VOLATILE);
	return (*cmdError)(interp,0,SDS__TCL_ARGERR);
     }
   status = SDS__OK;
   SdsExtract(atol(argv[1]),&status);
   if (status == SDS__OK)
       return TCL_OK; 
   else
   {
       char result[256];
       sprintf(result,"%s:Error extracint SDS object",
			argv[0]);
       Tcl_SetResult(interp,result,TCL_VOLATILE);
       return (*cmdError)(interp,0,status);
   }
}        

/*+                     S d s R e n a m e
 *  Command name:
      SdsRename

 *  Function:
      Rename an SDS object.

 *  Description:
      Specify a new name for an SDS object

 *  Call:
      SdsRename id name
 

 *  Parameters: 
      (>) id (int)   The SDS identifier of the object to be inserted.
      (>) name (string)  New name for object.


 *  Language:
      Tcl

 *  See Also: DTCL manual, SdsRename(3), Sds manual.

 *  Support: Jeremy Bailey, AAO

 *-

 *  History:
      19-Oct-1993 - JAB - Original version
      24-Mar-1995 - TJF - Use DtclCmdError to report errors.
 */
static int STclSdsRename(ClientData clientData , Tcl_Interp *interp, int argc, 
     char *argv[])

{
   StatusType status;
   SdsTclCmdErrorRoutineType cmdError = (SdsTclCmdErrorRoutineType)clientData;

   if (argc != 3) 
     {
        char result[256];
        sprintf(result,"%s:wrong # args",argv[0]);
        Tcl_SetResult(interp,result,TCL_VOLATILE);
	return (*cmdError)(interp,0,SDS__TCL_ARGERR);
     }
   status = SDS__OK;
   SdsRename(atol(argv[1]),argv[2],&status);
   if (status == SDS__OK)
     {
       return TCL_OK; 
     }
   else
     {
       char result[256];
       sprintf(result,"%s:Error renaming SDS object to %s.",
			argv[0],argv[2]);
       Tcl_SetResult(interp,result,TCL_VOLATILE);
       return (*cmdError)(interp,0,status);

     }
}        

/*+                     S d s I n f o
 *  Command name:
      SdsInfo

 *  Function:
      Return information about an SDS object.

 *  Description:
      Given the identifier to an object, return the name, type,
	code and dimensions of the object.

	Note, arguments from codevar onwards are optional.  

 *  Call:
      SdsInfo id namevar codevar ndimsvar dimsvar
 

 *  Parameters: 
      (>) id (int)   The SDS identifier of the object.
      (>) namevar (string)  The name of the TCL variable to be set to
				the object name
      (>) codevar (string)  The name of the TCL variable to be set to
				the object type (optional).
      (>) ndimsvar (string)  The name of the TCL variable to be set to
				the number of dimensions (optional)
      (>) dimsvar (string)  The name of the TCL array variable to be set to
				dimensions.  Indices 0, 1 .. 6 are used if
				required.  If the item is not an array,
				then this variable is not set.


 *  Language:
      Tcl

 *  See Also: DTCL manual, SdsInfo(3), Sds manual.

 *  Support: Tony Farrell, AAO

 *-

 *  History:
      25-Jan-1994 - TJF - Original version
      24-Mar-1995 - TJF - Use DtclCmdError to report errors.
      05-Nov-1997 - TJF - Use SdsTypeToStr to get type string.
      25-Mar-1998 - TJF - Register variable i now marked as int as the old
                          usage is not to be supported in a future version
                          of the C language.

 */
static int STclSdsInfo(ClientData clientData , Tcl_Interp *interp, int argc, 
     char *argv[])

{
   StatusType status;
   char name[SDS_C_NAMELEN];
   SdsCodeType code;
   long ndims;
   unsigned long dims[SDS_C_MAXARRAYDIMS];
   SdsTclCmdErrorRoutineType cmdError = (SdsTclCmdErrorRoutineType)clientData;

   if (argc < 3) 
     {
        char result[256];
        sprintf(result,"%s:wrong # args",argv[0]);
        Tcl_SetResult(interp,result,TCL_VOLATILE);
	return (*cmdError)(interp,0,SDS__TCL_ARGERR);
     }
   status = SDS__OK;
   SdsInfo((SdsIdType)atol(argv[1]),name,&code,&ndims,dims,&status);



   if (status == SDS__OK)
   {
       register int i;
       const char *code_string;
       char number[20];
       char number2[20];
       if (Tcl_SetVar(interp, argv[2], name, 0) == NULL)
	    return (*cmdError)(interp,0,SDS__TCL_CMDERR);
       else if (argc < 4)
	    return(TCL_OK);

       code_string = SdsTypeToStr(code,1);

       if (Tcl_SetVar(interp, argv[3], (char *)code_string, 0) == NULL)
	    return (*cmdError)(interp,0,SDS__TCL_CMDERR);
       else if (argc < 5)
	    return(TCL_OK);

       sprintf(number,"%ld",ndims);
       if (Tcl_SetVar(interp, argv[4], number, 0) == NULL)
	    return (*cmdError)(interp,0,SDS__TCL_CMDERR);
       else if (argc < 6)
	    return(TCL_OK);
   
       for (i = 0; i < ndims ; ++i)
       {
           sprintf(number,"%d",i);
           sprintf(number2,"%ld",dims[i]);
           if (Tcl_SetVar2(interp, argv[5], number, number2, 0) == NULL)
	        return (*cmdError)(interp,0,SDS__TCL_CMDERR);
       }

       return TCL_OK; 
   }
   else
     {
	return (*cmdError)(interp,argv[0],status);
     }
}        


/*+                     S d s F r e e I d
 *  Command name:
      SdsFreeId

 *  Function:
      Free an identifier so that its associated memory may be reused

 *  Description:
      Each identifier allocated by SDS uses memory. To avoid excessive
      allocation of memory the SdsFreeId function can be used to free
      memory associated with an identifier that is no longer needed.
      When this is done the memory will be reused by SDS for a subsequent
      identifier when needed.
      

 *  Call:
      SdsFreeId id
 

*   Parameters: 
      id  (int)   The SDS identifier to be freed.

 *  Language:
      Tcl

 *  See Also: DTCL manual, Sds manual, ArgDelete(3), SdsFreeId(n),
		SdsFreeId(3), SdsDelete(n), ArgDelete(n)

 *  Support: Jeremy Bailey, AAO

 *-

 *  History:
      28-Sep-1994 - JAB - Original version
      24-Mar-1995 - TJF - Use DtclCmdError to report errors.
 */

static int STclSdsFreeId(ClientData clientData , Tcl_Interp *interp, int argc, 
     char *argv[])

{
   StatusType status;
   SdsTclCmdErrorRoutineType cmdError = (SdsTclCmdErrorRoutineType)clientData;

   if (argc != 2) 
     {
        char result[256];
        sprintf(result,"%s:wrong # args",argv[0]);
        Tcl_SetResult(interp,result,TCL_VOLATILE);
	return (*cmdError)(interp,0,SDS__TCL_ARGERR);
     }
   status = SDS__OK;
   SdsFreeId((SdsIdType)(atol(argv[1])),&status);
   if (status == SDS__OK)
     {
       return TCL_OK; 
     }
   else
     {
	return (*cmdError)(interp,argv[0],status);
     }
}        


/*+                     S d s F r e e I d C h e c k
 *  Command name:
      SdsFreeIdCheck

 *  Function:
      Free an identifier so that its associated memory may be reused - checking id.

 *  Description:
      Each identifier allocated by SDS uses memory. To avoid excessive
      allocation of memory the SdsFreeId function can be used to free
      memory associated with an identifier that is no longer needed.
      When this is done the memory will be reused by SDS for a subsequent
      identifier when needed.

      This version will check that SdsDelete()/SdsFreeId() have been run, if required.
      

 *  Call:
      SdsFreeIdCheck id
 

*   Parameters: 
      id  (int)   The SDS identifier to be freed.

 *  Language:
      Tcl

 *  See Also: DTCL manual, Sds manual, ArgDelete(3), SdsFreeId(n),
		SdsFreeIdAndCheck(3), SdsDelete(n), ArgDelete(n)

 *  Support: Jeremy Bailey, AAO

 *-

 *  History:
      28-Sep-1994 - JAB - Original version
      24-Mar-1995 - TJF - Use DtclCmdError to report errors.
 */

static int STclSdsFreeIdCheck(ClientData clientData , Tcl_Interp *interp, int argc, 
     char *argv[])

{
   StatusType status;
   SdsTclCmdErrorRoutineType cmdError = (SdsTclCmdErrorRoutineType)clientData;

   if (argc != 2) 
     {
        char result[256];
        sprintf(result,"%s:wrong # args",argv[0]);
        Tcl_SetResult(interp,result,TCL_VOLATILE);
	return (*cmdError)(interp,0,SDS__TCL_ARGERR);
     }
   status = SDS__OK;
   SdsFreeIdAndCheck(0,(SdsIdType)(atol(argv[1])),&status);
   if (status == SDS__OK)
     {
       return TCL_OK; 
     }
   else
     {
	return (*cmdError)(interp,argv[0],status);
     }
}        



/*+                     S d s D e l e t e
 *  Command name:
      SdsDelete

 *  Function:
      Delete an SDS structure

 *  Description:
      Delete an SDS structure.

      This routine does not free the id, normally you should consider
      using ArgDelete to do both jobs in one operation.  This command
      is provided for consistency with C code.

 *  Call:
      SdsDelete id 
 

 *   Parameters: 
      (>) id  (int)  SDS identifier of structure to be deleted.


 *  Language:
      Tcl


 *  Support: Tony Farrelll, AAO

 *  See Also: DTCL manual, Sds manual, ArgDelete(n), SdsFreeId(n),
		SdsFreeId(3), SdsDelete(3)

 *-

 *  History:
      05-Nov-1997 - TJF - Original version
 */
static int STclSdsDelete(ClientData clientData , Tcl_Interp *interp, int argc, 
     char *argv[])

{
    StatusType status;
   SdsTclCmdErrorRoutineType cmdError = (SdsTclCmdErrorRoutineType)clientData;

    if (argc != 2) 
    {
        char result[256];
        sprintf(result,"%s:wrong # args",argv[0]);
        Tcl_SetResult(interp,result,TCL_VOLATILE);
	return (*cmdError)(interp,0,SDS__TCL_ARGERR);
    }
    status = SDS__OK;
    SdsDelete((SdsIdType)(atol(argv[1])),&status);
    if (status == SDS__OK)
        return TCL_OK; 
    else
	return (*cmdError)(interp,argv[0],status);
}

/*+                     S d s F i n d B y P a t h
 *  Command name:
      SdsFindByPath
      
 *  Function:
 *  	Find an Sds item by path name.

 *  Description:
      Given the name of a path to a component in an SDS structure return an 
      identifier to the component.

 *  Call:
      SdsFindByPath id name
 

 *  Parameters: 
      (>) id (int)   The SDS identifier of the structure.
      (>) name (string)  The path name of the component.

 *  Returns:
      The SDS identifier of the component.  


 *  Language:
      Tcl

 *  See Also: DTCL manual, SdsFindByPath(3), Sds manual.

 *  Support: Tony Farrell, AAO

 *-

 *  History:
      17-May-1996 - TJF - Original version
 */
static int STclSdsFindByPath(ClientData clientData , Tcl_Interp *interp, 
                             int argc, char *argv[])
 
{
   SdsIdType id;
   StatusType status;
   SdsTclCmdErrorRoutineType cmdError = (SdsTclCmdErrorRoutineType)clientData;

   if (argc != 3) 
     {
        char result[256];
        sprintf(result,"%s:wrong # args",argv[0]);
        Tcl_SetResult(interp,result,TCL_VOLATILE);
	return (*cmdError)(interp,0,SDS__TCL_ARGERR);
     }
   status = SDS__OK;
   SdsFindByPath(atol(argv[1]),argv[2],&id,&status);
   if (status == SDS__OK)
     {
       char result[256];
       sprintf(result,"%ld",(long int)id);
       Tcl_SetResult(interp,result,TCL_VOLATILE);
       return TCL_OK; 
     }
   else
     {
       char result[256];
       sprintf(result,"%s:Error finding SDS component - %s",
			argv[0],argv[2]);
       Tcl_SetResult(interp,result,TCL_VOLATILE);
       return (*cmdError)(interp,0,status);

     }
}        



/*+                     S d s N e w
 *  Command name:
      SdsNew

 *  Function:
      Create a new Sds structure

 *  Description:
      A new SDS structure is created it and an identifier to it is returned
      

 *  Call:
      SdsNew name [options]
 

 *  Parameters: 
      (>) name (string) The name for the item.

 *  Options:
      [-type  type]   The Sds Type code.  One of STRUCT, CHAR, BYTE, UBYTE, 
		      SHORT, USHORT, INT, UINT, I64, UI64, FLOAT and DOUBLE.  
		      Defaults to STRUCT.

      [-parent parent_id]  The id of a structured SDS Item which is to be the
                           parent of this structure.  If not supplied, then
                           the new item will be a top level structure

      [-extra info]   Specifies extra information to be assocaited with
                      the item.

      [-dims dimensions]  Specifies the dimensions of the item.  "dimensions"
                      is any array of up to seven positive integers 
                      which are the dimensions of the item if it is an array
                      item.  If not supplied, then the item is scaler.

 *  Language:
      Tcl

 *  See Also: DTCL manual, Sds manual, ArgNew(n), SdsNew(3).


 *  Support: Tony Farrell, AAO

 *-

 *  History:
      05-Nov-1997 - TJF - Original version
      25-Mar-1998 - TJF - Register variables i and j now marked as int as the
                          old usage is not to be supported in a future version
                          of the C language.

 */


static int STclSdsNew(ClientData clientData , Tcl_Interp *interp, int argc, 
     char *argv[])

{
    SdsIdType id;
    SdsIdType parent_id=0;
    SdsCodeType code = SDS_STRUCT;
    int ndims = 0;
    unsigned long dims[SDS_C_MAXARRAYDIMS];
    char *extra = 0;
    int nextra = 0;
    register int i;
    StatusType status = STATUS__OK;
   SdsTclCmdErrorRoutineType cmdError = (SdsTclCmdErrorRoutineType)clientData;

    if (argc < 2) 
    {
        char result[256];
        sprintf(result,"%s:wrong # args, should be \"SdsNew name [options...]\"",argv[0]);
        Tcl_SetResult(interp,result,TCL_VOLATILE);
	return (*cmdError)(interp,0,SDS__TCL_ARGERR);
    }

    for ( i = 2; i < argc ; i+=2 )
    {
       if (i+1 >= argc)
       {
           char result[256];
           sprintf(result,
                        "%s:No argument supplied to %s option\n",
                                argv[0],argv[i]);
           Tcl_SetResult(interp,result,TCL_VOLATILE);
           return (*cmdError)(interp,0,SDS__TCL_ARGERR);
       }
       else if (strcmp(argv[i],"-type") == 0)
       {
           if (strcmp(argv[i+1],"STRUCT") == 0)
               code = SDS_STRUCT;
           else if (strcmp(argv[i+1],"CHAR") == 0)
               code = SDS_CHAR;
           else if (strcmp(argv[i+1],"BYTE") == 0)
               code = SDS_BYTE;
           else if (strcmp(argv[i+1],"UBYTE") == 0)
               code = SDS_UBYTE;
           else if (strcmp(argv[i+1],"SHORT") == 0)
               code = SDS_SHORT;
           else if (strcmp(argv[i+1],"USHORT") == 0)
               code = SDS_USHORT;
           else if (strcmp(argv[i+1],"INT") == 0)
               code = SDS_INT;
           else if (strcmp(argv[i+1],"UINT") == 0)
               code = SDS_UINT;
           else if (strcmp(argv[i+1],"I64") == 0)
               code = SDS_I64;
           else if (strcmp(argv[i+1],"UI64") == 0)
               code = SDS_UI64;
           else if (strcmp(argv[i+1],"FLOAT") == 0)
               code = SDS_FLOAT;
           else if (strcmp(argv[i+1],"DOUBLE") == 0)
               code = SDS_DOUBLE;
           else 
           {
               char result[256];
               sprintf(result,"%s:Invalid SDS Type - \"%s\"\n",
                       argv[0],argv[i+1]);
               Tcl_SetResult(interp,result,TCL_VOLATILE);
               return (*cmdError)(interp,0,SDS__TCL_ARGERR);
           }

       }
       else if (strcmp(argv[i],"-parent") == 0)
       {
	   int value;
           if (Tcl_GetInt(interp, argv[i+1], &value) != TCL_OK)
               return TCL_ERROR;
	   parent_id = value;
           
       }
/*Does not work at the moment, SDS error.
       else if (strcmp(argv[i],"-nextra") == 0)
       {
           if (Tcl_GetInt(interp, argv[i+1], &nextra) != TCL_OK)
               return TCL_ERROR;
       }
*/
       else if (strcmp(argv[i],"-extra") == 0)
       {
           extra = argv[i+1];
           if (nextra == 0)
               nextra = strlen(extra)+1;
       }
       else if (strcmp(argv[i],"-dims") == 0)
       {
           char **listArgv;
           register int j;
           if (Tcl_SplitList(interp, argv[i+1], &ndims, &listArgv) != TCL_OK)
                return TCL_ERROR;
           if ((ndims >SDS_C_MAXARRAYDIMS )||(ndims <= 0))
           {
               char result[256];
               sprintf(result,
                  "%s:Invalid number of dimensions, must be 1 to %d - \"%s\"\n",
                       argv[0],SDS_C_MAXARRAYDIMS, argv[i]);
               Tcl_SetResult(interp,result,TCL_VOLATILE);
               return (*cmdError)(interp,0,SDS__TCL_ARGERR);
           }    
           for (j = 0; j < ndims ; ++j)
           {
               int value;
               if (Tcl_GetInt(interp, listArgv[j], &value)!= TCL_OK)
                   return TCL_ERROR;
               dims[j] = value;
           }
           
            
       }
       else
       {
           char result[256];
           sprintf(result,"%s:Unknown option - \"%s\"\n",
                   argv[0],argv[i]);
           Tcl_SetResult(interp,result,TCL_VOLATILE);
           return TCL_ERROR;
       }
    }
    
    SdsNew(parent_id,argv[1],nextra,extra,code,ndims,dims,&id,&status);

    if (status == SDS__OK)
    {
        char result[256];
        sprintf(result,"%ld",(long int)id);
        Tcl_SetResult(interp,result,TCL_VOLATILE);
        return TCL_OK; 
    }
    else
    {
	return (*cmdError)(interp,argv[0],status);
    }
} 


/*+                     S d s C e l l
 *  Command name:
      SdsCell

 *  Function:
      Find a component of a structured array.

 *  Description:
      Given the indices to a component of a structure array, return an 
      identifier to the component. 

 *  Call:
      SdsCell array_id dims 
 

 *  Parameters: 
      array_id (>) SDS id of the structured array 
      dims     (>) Indicates the array index where we are to insert the
                   item.  If more then one dimension, use a list and 
                   then the number of elements in the list must be the 
		   same as the number of array dimensions.  An exception here
		   is that you can treat all structure arrays as having
		   only one dimension if desired (so that you could walk
		   through all elements for example). 


 *  Language:
      Tcl

 *  See Also: DTCL manual, Sds manual, SdsCell(3)


 *  Support: Tony Farrell, AAO

 *-

 *  History:
      05-Nov-1997 - TJF - Original version
      25-Mar-1998 - TJF - Register variable j now marked as int as the old
                          usage is not to be supported in a future version
                          of the C language.
      14-Feb-2012 - TJF - This had never worked as was not fetching the SDS ID
                           correctly.  Fixed.
 */


static int STclSdsCell(ClientData clientData , Tcl_Interp *interp, 
                             int argc, char *argv[])

{
    SdsIdType id;
    SdsIdType array_id=0;
    int ndims = 0;
    unsigned long dims[SDS_C_MAXARRAYDIMS];
    StatusType status = STATUS__OK;
    char **listArgv;
    register int j;
    int value;
   SdsTclCmdErrorRoutineType cmdError = (SdsTclCmdErrorRoutineType)clientData;


    if (argc < 3) 
    {
        char result[256];
        sprintf(result,"%s:wrong # args, should be \"SdsCell array_id dims\"",argv[0]);
        Tcl_SetResult(interp,result,TCL_VOLATILE);
	return (*cmdError)(interp,0,SDS__TCL_ARGERR);
    }

    if (Tcl_GetInt(interp, argv[1], &value) != TCL_OK)
        return TCL_ERROR;
    array_id = value;

    if (Tcl_SplitList(interp, argv[2], &ndims, &listArgv) != TCL_OK)
        return TCL_ERROR;
    if ((ndims > SDS_C_MAXARRAYDIMS)||(ndims <= 0))
    {
        char result[256];
        sprintf(result,
                "%s:Invalid number of dimensions, must be 1 to %d - \"%s\"\n",
                argv[0],SDS_C_MAXARRAYDIMS, argv[2]);
        Tcl_SetResult(interp,result,TCL_VOLATILE);
        return TCL_OK;
    }    

    for (j = 0; j < ndims ; ++j)
    {
        int value;
        if (Tcl_GetInt(interp, listArgv[j], &value)!= TCL_OK)
            return TCL_ERROR;
        dims[j] = value;
    } 


    SdsCell(array_id,ndims,dims,&id,&status);

    if (status == SDS__OK)
    {
        char result[256];
        sprintf(result,"%ld",(long int)id);
        Tcl_SetResult(interp,result,TCL_VOLATILE);
        return TCL_OK; 
    }
    else
	return (*cmdError)(interp,argv[0],status);


}
       
/*+                     S d s I n s e r t C e l l
 *  Command name:
      SdsInsertCell

 *  Function:
      Insert object into a structure array 

 *  Description:
      Insert a top level object into a structure array at a position 
      specified by its indices. Delete any object that is currently at 
      that position. 

 *  Call:
      SdsInsertCell array_id dims id
 

 *  Parameters: 
      array_id (>) SDS id of the structured array into which we are
                   to insert the item.
      dims     (>) Indicates the array index where we are to insert the
                   item.  If more then one dimension, use a list and 
                   then the number of elements in the list must be the 
		   same as the number of array dimensions.  An exception here
		   is that you can treat all structure arrays as having
		   only one dimension if desired (so that you could walk
		   through all elements for example). 
      id       (>) The id of the item to insert.


 *  Language:
      Tcl

 *  See Also: DTCL manual, Sds manual, SdsInsertCell(3), SdsInsert(n), 
                SdsFillArray(n).


 *  Support: Tony Farrell, AAO

 *-

 *  History:
      05-Nov-1997 - TJF - Original version
      25-Mar-1998 - TJF - Register variable j now marked as int as the old
                          usage is not to be supported in a future version
                          of the C language.

 */


static int STclSdsInsertCell(ClientData clientData , Tcl_Interp *interp, 
                             int argc, char *argv[])

{
    SdsIdType id;
    SdsIdType array_id=0;
    int ndims = 0;
    unsigned long dims[SDS_C_MAXARRAYDIMS];
    StatusType status = STATUS__OK;
    char **listArgv;
    register int j;
    int value;
   SdsTclCmdErrorRoutineType cmdError = (SdsTclCmdErrorRoutineType)clientData;


    if (argc < 4) 
    {
        char result[256];
        sprintf(result,"%s:wrong # args, should be \"SdsInsertCell array_id dims id\"",argv[0]);
        Tcl_SetResult(interp,result,TCL_VOLATILE);
	return (*cmdError)(interp,0,SDS__TCL_ARGERR);
    }

    if (Tcl_GetInt(interp, argv[1], &value) != TCL_OK)
        return TCL_ERROR;
    value = array_id;

    if (Tcl_SplitList(interp, argv[2], &ndims, &listArgv) != TCL_OK)
        return TCL_ERROR;
    if ((ndims > SDS_C_MAXARRAYDIMS)||(ndims <= 0))
    {
        char result[256];
        sprintf(result,
                "%s:Invalid number of dimensions, must be 1 to %d - \"%s\"\n",
                argv[0],SDS_C_MAXARRAYDIMS, argv[2]);
        Tcl_SetResult(interp,result,TCL_VOLATILE);
        return TCL_OK;
    }    

    for (j = 0; j < ndims ; ++j)
    {
        int value;
        if (Tcl_GetInt(interp, listArgv[j], &value)!= TCL_OK)
            return TCL_ERROR;
        dims[j] = value;
    } 


    if (Tcl_GetInt(interp, argv[3], &value) != TCL_OK)
        return TCL_ERROR;
    id = value;

    SdsInsertCell(array_id,ndims,dims,id,&status);

    if (status == SDS__OK)
        return TCL_OK; 
    else
	return (*cmdError)(interp,argv[0],status);


}

/*+                     S d s F i l l A r r a y
 *  Command name:
      SdsFillArray

 *  Function:
      Fill out the contents of a structured array

 *  Description:
      This routine will fill out an array of structures item with the
      copies of a specified struture. 

 *  Call:
      SdsFillArray array_id id
 

 *  Parameters: 
      array_id (>) SDS id of the structured array into which we are
                   to insert the items.
      id       (>) The id of the item to fill the array with.


 *  Language:
      Tcl

 *  See Also: DTCL manual, Sds manual, SdsFillArray(3), SdsInsertCell(3).


 *  Support: Tony Farrell, AAO

 *-

      05-Nov-1997 - TJF - Original version
 */


static int STclSdsFillArray(ClientData clientData , Tcl_Interp *interp, 
                             int argc, char *argv[])

{
    SdsIdType id;
    SdsIdType array_id;
    StatusType status = STATUS__OK;
    int value;
   SdsTclCmdErrorRoutineType cmdError = (SdsTclCmdErrorRoutineType)clientData;


    if (argc < 3) 
    {
        char result[256];
        sprintf(result,"%s:wrong # args, should be \"SdsFillArray array_id id\"",argv[0]);
        Tcl_SetResult(interp,result,TCL_VOLATILE);
	return (*cmdError)(interp,0,SDS__TCL_ARGERR);
    }

    if (Tcl_GetInt(interp, argv[1], &value) != TCL_OK)
        return TCL_ERROR;
    array_id = value;

    if (Tcl_GetInt(interp, argv[2], &value) != TCL_OK)
        return TCL_ERROR;
    id = value;

    SdsFillArray(array_id,id,&status);

    if (status == SDS__OK)
        return TCL_OK; 
    else
	return (*cmdError)(interp,argv[0],status);


}

/*+                     S d s R e s i z e
 *  Command name:
      SdsResize

 *  Function:
      Change the dimensions of an array. 

 *  Description:
      Change the number and/or size of the dimensions of an array. This
      operation can be performed on primitive arrays or structure arrays. Note
      that SdsResize does not move the data elements in the storage
      representing the array, so there is no guarantee that after resizing the
      array the same data will be found at the same array index positions as
      before resizing, though this will be the case for simple changes such 
      as a change in the last dimension only. 
      
      If the size of a primitive array is increased the contents of the extra
      locations is undefined. Decreasing the size causes the data beyond the
      new limit to be lost. 
      
      If a structure array is extended the new elements created will be empty
      structures. If a structure array is decreased in size, the lost 
      elements and all their components will be deleted. 

 *  Call:
      SdsResize id dims


 *  Parameters: 
      id (>) SDS id of the structured array into which we are
                   to insert the item.
      dims     (>) Indicates the new dimensions.
                   IF more then one dimension, use a list and 
                   then the number must be the same as the number of
                   array dimensions required.


 *  Language:
      Tcl

 *  See Also: DTCL manual, Sds manual, SdsResize(3), SdsNew(n).


 *  Support: Tony Farrell, AAO

 *-

 *  History:
      05-Nov-1997 - TJF - Original version
      25-Mar-1998 - TJF - Register variable j now marked as int as the old
                          usage is not to be supported in a future version
                          of the C language.
 */


static int STclSdsResize(ClientData clientData , Tcl_Interp *interp, 
                             int argc, char *argv[])

{
    SdsIdType id;
    int ndims = 0;
    unsigned long dims[SDS_C_MAXARRAYDIMS];
    StatusType status = STATUS__OK;
    char **listArgv;
    register int j;
    int value;
   SdsTclCmdErrorRoutineType cmdError = (SdsTclCmdErrorRoutineType)clientData;


    if (argc < 3) 
    {
        char result[256];
        sprintf(result,"%s:wrong # args, should be \"SdsResize id dims \"",argv[0]);
        Tcl_SetResult(interp,result,TCL_VOLATILE);
	return (*cmdError)(interp,0,SDS__TCL_ARGERR);
    }

    if (Tcl_GetInt(interp, argv[1], &value) != TCL_OK)
        return TCL_ERROR;
    id = value;

    if (Tcl_SplitList(interp, argv[2], &ndims, &listArgv) != TCL_OK)
        return TCL_ERROR;
    if ((ndims > SDS_C_MAXARRAYDIMS)||(ndims <= 0))
    {
        char result[256];
        sprintf(result,
                "%s:Invalid number of dimensions, must be 1 to %d - \"%s\"\n",
                argv[0],SDS_C_MAXARRAYDIMS, argv[2]);
        Tcl_SetResult(interp,result,TCL_VOLATILE);
        return (*cmdError)(interp,0,SDS__TCL_ARGERR);
    }    

    for (j = 0; j < ndims ; ++j)
    {
        int value;
        if (Tcl_GetInt(interp, listArgv[j], &value)!= TCL_OK)
            return TCL_ERROR;
        dims[j] = value;
    } 


    SdsResize(id,ndims,dims,&status);

    if (status == SDS__OK)
        return TCL_OK; 
    else
	return (*cmdError)(interp,argv[0],status);


}
/*+                     S d s P u t
 *  Command name:
      SdsPut

 *  Function:
      Put data into an Sds item.

 *  Description:
      Write data into a primitive object.  The data is supplied as a
      Tcl list.   

      Since Tcl works basically with string data, the data
      must be converted from strings to the desired type.  Whilst this
      commands checks if each item is of the desired underlying type (char,
      integer or real) it does not check the range and overflows may occur 
      when converting to the smaller types.  In addtion, what happens 
      if you attempt to convert negative values into unsigned values
      is undefined.

      As a result of all the string conversions, this routine is not
      particuarly efficient, especially for large arrays,  You should
      consider if the job is better done by implementing new Tcl commands
      to handle your paricular case.


      This routine makes not attempt to handle array dimensions, all 
      data is considered to be one dimensional. 

      This routine cannot handle 64 bit integer types unless the machine on 
      which it is running supports 64 bit long integers.

 *  Call:
      SdsPut id data [options]


 *  Parameters: 
      id (>) SDS id of a primitive item to insert data into.

      data (>) The data to insert.  Must be a single item or list of
               items (for an array item) of values of the correct type.
               For all SDS types, we first convert to "int".  For
               floating point types, we first convert to "double".  For
               CHAR types, should be a list of single characters (not
               a string).

 *  Options:
      [-offset n] The offset in the SDS array to where we put
                  the first item.  Defaults to zero

 *  Language:
      Tcl

 *  See Also: DTCL manual, Sds manual, SdsPut(3), SdsGet(n),
                        ArgPutString(n) ArgPutx(n)


 *  Support: Tony Farrell, AAO

 *-

 *  History:
      05-Nov-1997 - TJF - Original version
      25-Mar-1998 - TJF - Register variables i and j now marked as int as the
                          old usage is not to be supported in a future version
                          of the C language.

 */


static int STclSdsPut(ClientData clientData , Tcl_Interp *interp, 
                             int argc, char *argv[])

{
    SdsIdType id;
    long ndims = 0;
    unsigned long dims[SDS_C_MAXARRAYDIMS];
    char name[SDS_C_NAMELEN];
    SdsCodeType code;

    int offset = 0;
    int nitems = 0;
    StatusType status = STATUS__OK;
    char **listArgv;
    register int i,j;

    int *integerItems=0;
    double   *realItems=0;
    char     *charItems=0;
    void *data = 0;
    long size;
    int value;

   SdsTclCmdErrorRoutineType cmdError = (SdsTclCmdErrorRoutineType)clientData;


    if (argc < 3) 
    {
        char result[256];
        sprintf(result,"%s:wrong # args, should be \"SdsPut id data \"",argv[0]);
        Tcl_SetResult(interp,result,TCL_VOLATILE);
	return (*cmdError)(interp,0,SDS__TCL_ARGERR);
    }

/*
 *   Get the Sds item and ensure it is not a structure items. 
 */
    if (Tcl_GetInt(interp, argv[1], &value) != TCL_OK)
        return TCL_ERROR;
    id = value;

    SdsInfo(id,name,&code,&ndims,dims,&status);

    if (status != STATUS__OK)
	return (*cmdError)(interp,argv[0],status);

    else if (code == SDS_STRUCT)
    {
        char result[256];
        sprintf(result,
                "%s:Sds item cannot be a structure\n",argv[0]);
        Tcl_SetResult(interp,result,TCL_VOLATILE);
        return (*cmdError)(interp,0,SDS__TCL_ARGERR);
    }
#ifndef LONG_64
    else if ((code == SDS_I64)||(code == SDS_UI64))
    {
        char result[256];
        sprintf(result,
                "%s:Sds item cannot be 64 bits unless machine supports 64 bit longs\n",argv[0]);
        Tcl_SetResult(interp,result,TCL_VOLATILE);
        return (*cmdError)(interp,0,SDS__TCL_ARGERR);
    }
#endif

/*
 *  Check the options.
 */
    for ( i = 3; i < argc ; i+=2 )
    {
        if (i+1 >= argc)
        {
            char result[256];
            sprintf(result,
                    "%s:No argument supplied to %s option\n",
                    argv[0],argv[i]);
            Tcl_SetResult(interp,result,TCL_VOLATILE);
            return (*cmdError)(interp,0,SDS__TCL_ARGERR);
        }
        else if (strcmp(argv[i],"-offset") == 0)
        {
            if (Tcl_GetInt(interp, argv[i+1], &offset) != TCL_OK)
                return TCL_ERROR;
            else if (offset < 0)
            {
                char result[256];
                sprintf(result,"%s:Offset %d must be non-negative\n",
                        argv[0],offset);
                Tcl_SetResult(interp,result,TCL_VOLATILE);
                return (*cmdError)(interp,0,SDS__TCL_ARGERR);
            }
        }
        else
        {
            char result[256];
            sprintf(result,"%s:Unknown option - \"%s\"\n",
                    argv[0],argv[i]);
            Tcl_SetResult(interp,result,TCL_VOLATILE);
            return (*cmdError)(interp,0,SDS__TCL_ARGERR);
        }
    }



/*
 *  Split up the DATA list.
 */
    if (Tcl_SplitList(interp, argv[2], &nitems, &listArgv) != TCL_OK)
        return TCL_ERROR;
/*
 *  We must convert the data to one of the three basic types, REAL, INTEGER
 *  or CHAR.
 */
    switch (code)
    {
        case SDS_DOUBLE:
        case SDS_FLOAT:
            size = nitems*sizeof(*realItems);
            if ((realItems = SdsMalloc(size)) == 0)
            {
                char result[256];
                sprintf(result,"%s:Malloc error",argv[0]);
                Tcl_SetResult(interp,result,TCL_VOLATILE);
                return (*cmdError)(interp,0,SDS__TCL_ARGERR);
            }
            for (j = 0; j < nitems ; ++j)
            {
                if (Tcl_GetDouble(interp, listArgv[j], &realItems[j])!= TCL_OK)
                {
                    SdsFree(realItems);
                    return TCL_ERROR;
                }
            } 
            break;
        case SDS_CHAR:
            size = nitems*sizeof(*charItems);
            if ((charItems = SdsMalloc(size)) == 0)
            {
                char result[256];
                sprintf(result,"%s:Malloc error",argv[0]);
                Tcl_SetResult(interp,result,TCL_VOLATILE);
                return (*cmdError)(interp,0,SDS__TCL_ARGERR);
            }
            for (j = 0; j < nitems ; ++j)
            {
                if (strlen(listArgv[j]) != 1)
                {
                    char result[256];
                    SdsFree(charItems);
                    sprintf(result,"%s:Invalid char %s",argv[0],
                            listArgv[j]);
                    Tcl_SetResult(interp,result,TCL_VOLATILE);
                    return (*cmdError)(interp,0,SDS__TCL_ARGERR);
                }
                charItems[j] = listArgv[j][0];
            } 
            break;
        default:        /* Must be Integer type */
            size = nitems*sizeof(*integerItems);
            if ((integerItems = SdsMalloc(size)) == 0)
            {
                char result[256];
                sprintf(result,"%s:Malloc error",argv[0]);
                Tcl_SetResult(interp,result,TCL_VOLATILE);
                return (*cmdError)(interp,0,SDS__TCL_ARGERR);
            }
            for (j = 0; j < nitems ; ++j)
            {
                if (Tcl_GetInt(interp, listArgv[j], &integerItems[j])!= TCL_OK)
                {
                    SdsFree(integerItems);
                    return TCL_ERROR;
                }
            } 
            break;
    }
/*
 *  Now convert to the output format.  Where we already have data in the
 *  required format, we just set the pointer appropiate.
 */
    switch (code)
    {
        case SDS_DOUBLE:
            data = realItems;
            realItems = 0;
            break;
        case SDS_FLOAT:
        {
            float *floatItems;
            size = nitems*sizeof(float);
            if ((floatItems = SdsMalloc(size)) == 0)
            {
                char result[256];
                SdsFree(realItems);
                sprintf(result,"%s:Malloc error",argv[0]);
                Tcl_SetResult(interp,result,TCL_VOLATILE);
                return (*cmdError)(interp,0,SDS__TCL_ARGERR);
            }
            for (j = 0; j < nitems ; ++j)
                floatItems[j] = realItems[j];
            data = floatItems;
            break;
        }    
        case SDS_CHAR:
            data = charItems;
            charItems = 0;
            break;
        case SDS_BYTE:
        {
            schar *byteItems;
            size = nitems*sizeof(schar);
            if ((byteItems = SdsMalloc(size)) == 0)
            {
                char result[256];
                SdsFree(integerItems);
                sprintf(result,"%s:Malloc error",argv[0]);
                Tcl_SetResult(interp,result,TCL_VOLATILE);
                return (*cmdError)(interp,0,SDS__TCL_ARGERR);
            }
            for (j = 0; j < nitems ; ++j)
                byteItems[j] = (schar)integerItems[j];
            data = byteItems;
            break;
        }    
        case SDS_UBYTE:
        {
            unsigned char *ubyteItems;
            size = nitems*sizeof(unsigned char);
            if ((ubyteItems = SdsMalloc(size)) == 0)
            {
                char result[256];
                SdsFree(integerItems);
                sprintf(result,"%s:Malloc error",argv[0]);
                Tcl_SetResult(interp,result,TCL_VOLATILE);
                return (*cmdError)(interp,0,SDS__TCL_ARGERR);
            }
            for (j = 0; j < nitems ; ++j)
                ubyteItems[j] = (unsigned char)integerItems[j];
            data = ubyteItems;
            break;
        }    
        case SDS_SHORT:
        {
            short *shortItems;
            size = nitems*sizeof(short);
            if ((shortItems = SdsMalloc(size)) == 0)
            {
                char result[256];
                SdsFree(integerItems);
                sprintf(result,"%s:Malloc error",argv[0]);
                Tcl_SetResult(interp,result,TCL_VOLATILE);
                return (*cmdError)(interp,0,SDS__TCL_ARGERR);
            }
            for (j = 0; j < nitems ; ++j)
                shortItems[j] = (short)integerItems[j];
            data = shortItems;
            break;
        }    
        case SDS_USHORT:
        {
            unsigned short *ushortItems;
            size = nitems*sizeof(unsigned short);
            if ((ushortItems = SdsMalloc(size)) == 0)
            {
                char result[256];
                SdsFree(integerItems);
                sprintf(result,"%s:Malloc error",argv[0]);
                Tcl_SetResult(interp,result,TCL_VOLATILE);
                return (*cmdError)(interp,0,SDS__TCL_ARGERR);
            }
            for (j = 0; j < nitems ; ++j)
                ushortItems[j] = (unsigned short)integerItems[j];
            data = ushortItems;
            break;
        }    
        case SDS_INT:
        {
            if (sizeof(int) == sizeof(INT32))
            {   
                data = integerItems;
                integerItems = 0;
            }
            else
            {
                INT32 *intItems;
                size = nitems*sizeof(INT32);
                if ((intItems = SdsMalloc(size)) == 0)
                {
                    char result[256];
                    SdsFree(integerItems);
                    sprintf(result,"%s:Malloc error",argv[0]);
                    Tcl_SetResult(interp,result,TCL_VOLATILE);
                    return (*cmdError)(interp,0,SDS__TCL_ARGERR);
                }
                for (j = 0; j < nitems ; ++j)
                    intItems[j] = (INT32)integerItems[j];
                data = intItems;
            }
            break;
        }    
        case SDS_UINT:
         {
            UINT32 *uintItems;
            size = nitems*sizeof(UINT32);
            if ((uintItems = SdsMalloc(size)) == 0)
            {
                char result[256];
                SdsFree(integerItems);
                sprintf(result,"%s:Malloc error",argv[0]);
                Tcl_SetResult(interp,result,TCL_VOLATILE);
                return (*cmdError)(interp,0,SDS__TCL_ARGERR);
            }
            for (j = 0; j < nitems ; ++j)
                uintItems[j] = (UINT32)integerItems[j];
            data = uintItems;
            break;
        }    
#ifdef LONG__64
       case SDS_I64:
        {
            INT64 *i64Items;
            size = nitems*sizeof(INT64);
            if ((i64Items = SdsMalloc(size)) == 0)
            {
                char result[256];
                SdsFree(integerItems);
                sprintf(result,"%s:Malloc error",argv[0]);
                Tcl_SetResult(interp,result,TCL_VOLATILE);
                return (*cmdError)(interp,0,SDS__TCL_ARGERR);
            }
            for (j = 0; j < nitems ; ++j)
            {
                i64Items[j] = (INT64)integerItems[j];
            }
            data = i64Items;
            break;
        }    
        case SDS_UI64:
        {
            UINT64 *ui64Items;
            size = nitems*sizeof(UINT64);
            if ((ui64Items = SdsMalloc(size)) == 0)
            {
                char result[256];
                SdsFree(integerItems);
                sprintf(result,"%s:Malloc error",argv[0]);
                Tcl_SetResult(interp,result,TCL_VOLATILE);
                return (*cmdError)(interp,0,SDS__TCL_ARGERR);
            }
            for (j = 0; j < nitems ; ++j)
                ui64Items[j] = (UINT64)integerItems[j];
            data = ui64Items;
            break;
        }    
#endif
        default:         /* Should not happen */
            break;
    }
    SdsPut(id,size,offset,data,&status);

    if (integerItems)
        SdsFree(integerItems);
    if (realItems)
        SdsFree(realItems);
    if (charItems)
        SdsFree(charItems);
    if (data)
        SdsFree(data);

    if (status == SDS__OK)
        return TCL_OK; 
    else
	return (*cmdError)(interp,argv[0],status);

    

}

/*+                     S d s G e t
 *  Command name:
      SdsGet

 *  Function:
      Get data from an Sds item.

 *  Description:
      Get data from scaler items.

      The data is returned as an Tcl list, with all the items
      converted to string equivalents.  If the type is SDS_CHAR,
      the values are converted to char representations.

      The output of this routine makes no attempt to handle array
      dimensions, all data is considered to be one dimensional.  You
      can use SdsInfo(n) to get array dimensions.

      As a result of all the string conversions, this routine is not
      particularly efficient, especially for large arrays,  You should
      consider if the job is better done by implementing new Tcl commands
      to handle your paricular case.

      This routine cannot handle 64 bit intger types unless the machine on 
      which it is running supports 64 bit long integer.

 *  Call:
      SdsGet id [options]


 *  Parameters: 
      id (>) SDS id of a primitive item to insert data into.

 *  Options:
      [-offset n] The offset in the SDS array to where we get
                  the first item (item offset, not byte offset).
                  Defaults to zero
      [-max    n] The maximum number of items to fetch.  If not
                  specified, we get all the values.

 *  Language:
      Tcl

 *  See Also: DTCL manual, Sds manual, SdsGet(3), SdsPut(n), ArgGet(n).


 *  Support: Tony Farrell, AAO

 *-

 *  History:
      05-Nov-1997 - TJF - Original version
      25-Mar-1998 - TJF - Register variable i now marked as int as the old
                          usage is not to be supported in a future version
                          of the C language.
      14-Nov-2011 - TJF - Use Tcl_PrintDouble() to print doubles and floats,
                          and hence get the stanard Tcl formating approach.
 */


static int STclSdsGet(ClientData clientData , Tcl_Interp *interp, 
                             int argc, char *argv[])

{
    SdsIdType id;
    long ndims = 0;
    unsigned long dims[SDS_C_MAXARRAYDIMS];
    char name[SDS_C_NAMELEN];
    SdsCodeType code;

    int offset = 0;
    int nitems = 0;
    StatusType status = STATUS__OK;
    register int i;
    int maxItems;

    void   *data = 0;
    unsigned long actlen;
    int value;

   SdsTclCmdErrorRoutineType cmdError = (SdsTclCmdErrorRoutineType)clientData;


    if (argc < 2) 
    {
        char result[256];
        sprintf(result,"%s:wrong # args, should be \"SdsGet id [options] \"",argv[0]);
        Tcl_SetResult(interp,result,TCL_VOLATILE);
	return (*cmdError)(interp,0,SDS__TCL_ARGERR);
    }

/*
 *   Get the Sds item and ensure it is not a structure items. 
 */
    if (Tcl_GetInt(interp, argv[1], &value) != TCL_OK)
        return TCL_ERROR;
    id = value;

    SdsInfo(id,name,&code,&ndims,dims,&status);

    if (status != STATUS__OK)
	return (*cmdError)(interp,argv[0],status);

    else if (code == SDS_STRUCT)
    {
        char result[256];
        sprintf(result,
                "%s:Sds item cannot be a structure\n",argv[0]);
        Tcl_SetResult(interp,result,TCL_VOLATILE);
        return (*cmdError)(interp,0,SDS__TCL_ARGERR);
    }
#ifndef LONG_64
    else if ((code == SDS_I64)||(code == SDS_UI64))
    {
        char result[256];
        sprintf(result,
                "%s:Sds item cannot be 64 bits unless machine supports 64 bit longs\n",argv[0]);
        Tcl_SetResult(interp,result,TCL_VOLATILE);
        return (*cmdError)(interp,0,SDS__TCL_ARGERR);
    }
#endif
/*
 *  Get the maximum number of items we can deal with.
 */
    maxItems = 1;
    for (i = 0; i < ndims ; ++i)
        maxItems *= dims[i];

/*
 *  Check the options.
 */
    for ( i = 2; i < argc ; i+=2 )
    {
        if (i+1 >= argc)
        {
            char result[256];
            sprintf(result,
                    "%s:No argument supplied to %s option\n",
                    argv[0],argv[i]);
            Tcl_SetResult(interp,result,TCL_VOLATILE);
            return (*cmdError)(interp,0,SDS__TCL_ARGERR);
        }
        else if (strcmp(argv[i],"-offset") == 0)
        {
            if (Tcl_GetInt(interp, argv[i+1], &offset) != TCL_OK)
                return TCL_ERROR;
            else if (offset < 0)
            {
                char result[256];
                sprintf(result,"%s:Offset %d must be non-negative\n",
                        argv[0],offset);
                Tcl_SetResult(interp,result,TCL_VOLATILE);
                return (*cmdError)(interp,0,SDS__TCL_ARGERR);
            }
               
        }
        else if (strcmp(argv[i],"-max") == 0)
        {
            if (Tcl_GetInt(interp, argv[i+1], &nitems) != TCL_OK)
                return TCL_ERROR;
            else if (nitems <= 0)
            {
                char result[256];
                sprintf(result,"%s:Max items %d must be positive\n",
                        argv[0],nitems);
                Tcl_SetResult(interp,result,TCL_VOLATILE);
                return TCL_ERROR;
            }
            else if (nitems > maxItems)
            {
                char result[256];
                sprintf(result,
                        "%s:Max items %d exceeded actual max of %d\n",
                        argv[0],nitems,maxItems);
                Tcl_SetResult(interp,result,TCL_VOLATILE);
                return (*cmdError)(interp,0,SDS__TCL_ARGERR);
            }
               
        }
        else
        {
            char result[256];
            sprintf(result,"%s:Unknown option - \"%s\"\n",
                    argv[0],argv[i]);
            Tcl_SetResult(interp,result,TCL_VOLATILE);
            return (*cmdError)(interp,0,SDS__TCL_ARGERR);
        }
    }


/*
 *  Validate the offset into the data.
 */
    if ((offset + nitems) >= maxItems)
    {
        char result[256];
        sprintf(result,
               "%s:Max items %d + offset items %d exceeded actual max of %d\n",
                argv[0],nitems,offset,maxItems);
        Tcl_SetResult(interp,result,TCL_VOLATILE);
        return (*cmdError)(interp,0,SDS__TCL_ARGERR);
    }
/*
 *  If use did not specify the number of items, work it out.
 */
    if (nitems == 0)
        nitems = maxItems - offset;
      
    

/*
 *  Access the data array.
 */
    SdsPointer(id,(void **)&data,&actlen,&status);
    if (status != STATUS__OK)
    {
        SdsFree(data);
	return (*cmdError)(interp,argv[0],status);
    }

/*
 *  Now format the data into the output string.
 */
    switch (code)
    {
        case SDS_DOUBLE:
        {
            char buffer[TCL_DOUBLE_SPACE];
            double *ddata = (double *)data + offset;
            for (i = 0; i < nitems ; ++i)
            {
                Tcl_PrintDouble(interp, ddata[i], buffer);
                Tcl_AppendElement(interp,buffer);
            }
            break;
        }
        case SDS_FLOAT:
        {
            char buffer[TCL_DOUBLE_SPACE];
            float *fdata = (float *)data + offset;
            for (i = 0; i < nitems ; ++i)
            {
                Tcl_PrintDouble(interp, fdata[i], buffer);
                Tcl_AppendElement(interp,buffer);
            }
            break;
        }
        case SDS_CHAR:
        {
            char buffer[30];
            char *cdata = (char *)data + offset;
            for (i = 0; i < nitems ; ++i)
            {
                sprintf(buffer,"%c",cdata[i]);
                Tcl_AppendElement(interp,buffer);
            }
            break;
        }
        case SDS_BYTE:
        {
            char buffer[30];
            schar *bdata = (schar *)data + offset;
            for (i = 0; i < nitems ; ++i)
            {
                sprintf(buffer,"%d",bdata[i]);
                Tcl_AppendElement(interp,buffer);
            }
            break;
        }
        case SDS_UBYTE:
        {
            char buffer[30];
            unsigned char *bdata = (unsigned char *)data + offset;
            for (i = 0; i < nitems ; ++i)
            {
                sprintf(buffer,"%u",bdata[i]);
                Tcl_AppendElement(interp,buffer);
            }
            break;
        }
        case SDS_SHORT:
        {
            char buffer[30];
            short *sdata = (short *)data + offset;
            for (i = 0; i < nitems ; ++i)
            {
                sprintf(buffer,"%d",sdata[i]);
                Tcl_AppendElement(interp,buffer);
            }
            break;
        }
        case SDS_USHORT:
        {
            char buffer[30];
            unsigned short *sdata = (unsigned short *)data + offset;
            for (i = 0; i < nitems ; ++i)
            {
                sprintf(buffer,"%u",sdata[i]);
                Tcl_AppendElement(interp,buffer);
            }
            break;
        }
        case SDS_INT:
        {
            char buffer[30];
            INT32 *idata = (INT32 *)data + offset;
            for (i = 0; i < nitems ; ++i)
            {
                sprintf(buffer,"%ld",(long int)idata[i]);
                Tcl_AppendElement(interp,buffer);
            }
            break;
        }
        case SDS_UINT:
        {
            char buffer[30];
            UINT32 *idata = (UINT32 *)data + offset;
            for (i = 0; i < nitems ; ++i)
            {
                sprintf(buffer,"%lu",(long int)idata[i]);
                Tcl_AppendElement(interp,buffer);
            }
            break;
        }
#ifdef LONG_64
        case SDS_I64:
        {
            char buffer[40];
            INT64 *idata = (INT64 *)data + offset;
            for (i = 0; i < nitems ; ++i)
            {
                sprintf(buffer,"%ld",(long int)idata[i]);
                Tcl_AppendElement(interp,buffer);
            }
            break;
        }
        case SDS_UI64:
        {
            char buffer[40];
            UINT64 *idata = (UINT64 *)data + offset;
            for (i = 0; i < nitems ; ++i)
            {
                sprintf(buffer,"%lu",(long int)idata[i]);
                Tcl_AppendElement(interp,buffer);
            }
            break;
        }
#endif
        default:         /* Should not happen */
            break;
    }


    return TCL_OK;
    

}




/*+                     S d s G e t E x t r a
 *  Command name:
      SdsGetExtra

 *  Function:
      Get an Sds structure's Extra infomation field contents.

 *  Description:
      Read bytes from the extra information field of an object.   Note, this
      is treated as a null terminated string.

 *  Call:
      SdsGetExtra id [options]
 

 *  Parameters: 
      id       (>) The id of the item.

 *  Options:
      [-len  n]  The maximum length to expect.  If not supplied, we set this
                 to the maximum length which can be returned by a normal
                 Tcl command without special handling (once TCL_RESULT_SIZE, 
                 normally 200 bytes - now the size of a local array used
                 for the result, and now 256 bytes.)

 *  Language:
      Tcl

 *  See Also: DTCL manual, Sds manual, SdsNew(n), SdsPutExtra(n), 
                    SdsGetExtra(3).


 *  Support: Tony Farrell, AAO

 *-

 *  History:
      05-Nov-1997 - TJF - Original version
      25-Mar-1998 - TJF - Register variable i now marked as int as the old
                          usage is not to be supported in a future version
                          of the C language.
 */


static int STclSdsGetExtra(ClientData clientData , Tcl_Interp *interp, 
                             int argc, char *argv[])

{
    SdsIdType id;
    StatusType status = STATUS__OK;
    char result[256];
    char* resultPtr = result;
    Tcl_FreeProc* freeProc = TCL_VOLATILE;
    int length = sizeof(result); /*Default len, allow for null termination*/
    unsigned long actlen;
    register int i;
    int value;
    SdsTclCmdErrorRoutineType cmdError = (SdsTclCmdErrorRoutineType)clientData;

    if (argc < 2) 
    {
        sprintf(result,"%s:wrong # args, should be \"SdsGetExtra id [options]\"",argv[0]);
        Tcl_SetResult(interp,result,TCL_VOLATILE);
        return (*cmdError)(interp,0,SDS__TCL_ARGERR);
    }

    if (Tcl_GetInt(interp, argv[1], &value) != TCL_OK)
        return TCL_ERROR;
    id = value;

    for ( i = 2; i < argc ; i+=2 )
    {
        if (i+1 >= argc)
        {
            sprintf(result,
                    "%s:No argument supplied to %s option\n",
                    argv[0],argv[i]);
            Tcl_SetResult(interp,result,TCL_VOLATILE);
            return (*cmdError)(interp,0,SDS__TCL_ARGERR);
        }
        else if (strcmp(argv[i],"-len") == 0)
        {
            if (Tcl_GetInt(interp, argv[i+1], &length) != TCL_OK)
                return TCL_ERROR;
            if (length <= 1)
            {
                sprintf(result,
                        "%s:Length must be greater then 1, %s invalid\n",
                        argv[0],argv[i]);
                Tcl_SetResult(interp,result,TCL_VOLATILE);
                return (*cmdError)(interp,0,SDS__TCL_ARGERR);
            }
            if (length >= (int)sizeof(result))
            {
/*
 *         Malloc the required space.  Add 1 to allow for null termination.
 */
                if ((resultPtr = Tcl_Alloc(length+1)) == 0)
                {
                    sprintf(result,"%s:allocation error",argv[0]);
                    Tcl_SetResult(interp,result,TCL_VOLATILE);
                    return (*cmdError)(interp,0,SDS__TCL_ARGERR);
                   
                }
                freeProc = TCL_DYNAMIC;
            }
               
        }
        else
        {
            sprintf(result,"%s:Unknown option - \"%s\"\n",
                    argv[0],argv[i]);
            Tcl_SetResult(interp,result,TCL_VOLATILE);
            return TCL_ERROR;
        }
    }


    SdsGetExtra(id,length,resultPtr,&actlen,&status);

    if (status == SDS__OK)
    {
        resultPtr[actlen-1] = '\0';  /* Ensure null terminated */
        Tcl_SetResult(interp,resultPtr,freeProc);
        return TCL_OK; 
    }
    else
        return (*cmdError)(interp,argv[0],status);


}
/*+                     S d s P u t E x t r a
 *  Command name:
      SdsPutExtra

 *  Function:
      Put an Sds structure's Extra infomation field contents.

 *  Description:
      Read bytes from the extra information field of an object.   Note, this
      is treated as a null terminated string.

 *  Call:
      SdsPutExtra id value
 

 *  Parameters: 
      id       (>) The id of the item.
      value    (>) The value to put (currently, only null terminated strings
                   acceptable)

 *  Language:
      Tcl

 *  See Also: DTCL manual, Sds manual, SdsNew(n), SdsGetExtra(n), 
                    SdsPutExtra(3).


 *  Support: Tony Farrell, AAO

 *-

 *  History:
      05-Nov-1997 - TJF - Original version
 */


static int STclSdsPutExtra(ClientData clientData , Tcl_Interp *interp, 
                             int argc, char *argv[])

{
    SdsIdType id;
    StatusType status = STATUS__OK;
    int value;
   SdsTclCmdErrorRoutineType cmdError = (SdsTclCmdErrorRoutineType)clientData;

    if (argc != 3) 
    {
        char result[256];
        sprintf(result,"%s:wrong # args, should be \"SdsPutExtra id value\"",argv[0]);
        Tcl_SetResult(interp,result,TCL_VOLATILE);
	return (*cmdError)(interp,0,SDS__TCL_ARGERR);
    }

    if (Tcl_GetInt(interp, argv[1], &value) != TCL_OK)
        return TCL_ERROR;
    id = value;

    SdsPutExtra(id,strlen(argv[2])+1,argv[2],&status);

    if (status == SDS__OK)
    {
        return TCL_OK; 
    }
    else
	return (*cmdError)(interp,argv[0],status);


}

/*
 *  Function called to actually create the SDS Tcl commands.  This is
 *  invoked by either Sds_Init or the Dtcl equivalent.  By providing
 *  the cmdError and createCmd routines as arguments, we can ensure the
 *  SDS or DTCL routines are used as appropriate.
 */
extern void SdsTclAddCommands(
    Tcl_Interp *interp,
    SdsTclCmdErrorRoutineType cmdError,
    SdsTclCreateCommandRoutineType createCmd)

{
   (*createCmd)(interp,"ArgNew",STclArgNew,(ClientData *)cmdError,
           (Tcl_CmdDeleteProc *)NULL);
   (*createCmd)(interp,"ArgGet",STclArgGet,(ClientData *)cmdError,
           (Tcl_CmdDeleteProc *)NULL);
   (*createCmd)(interp,"ArgGetType",STclArgGetType,(ClientData *)cmdError,
           (Tcl_CmdDeleteProc *)NULL);
   (*createCmd)(interp,"ArgPutString",STclArgPutString,(ClientData *)cmdError,
           (Tcl_CmdDeleteProc *)NULL);
   (*createCmd)(interp,"ArgPuti",STclArgPuti,(ClientData *)cmdError,
           (Tcl_CmdDeleteProc *)NULL);
   (*createCmd)(interp,"ArgPutc",STclArgPutc,(ClientData *)cmdError,
           (Tcl_CmdDeleteProc *)NULL);
   (*createCmd)(interp,"ArgPuts",STclArgPuts,(ClientData *)cmdError,
           (Tcl_CmdDeleteProc *)NULL);
   (*createCmd)(interp,"ArgPutus",STclArgPutus,(ClientData *)cmdError,
           (Tcl_CmdDeleteProc *)NULL);
   (*createCmd)(interp,"ArgPutu",STclArgPutu,(ClientData *)cmdError,
           (Tcl_CmdDeleteProc *)NULL);
   (*createCmd)(interp,"ArgPutf",STclArgPutf,(ClientData *)cmdError,
           (Tcl_CmdDeleteProc *)NULL);
   (*createCmd)(interp,"ArgPutd",STclArgPutd,(ClientData *)cmdError,
           (Tcl_CmdDeleteProc *)NULL);
   (*createCmd)(interp,"ArgDelete",STclArgDelete,(ClientData *)cmdError,
           (Tcl_CmdDeleteProc *)NULL);

   (*createCmd)(interp,"SdsArrayCell",STclSdsArrayCell,(ClientData *)cmdError,
           (Tcl_CmdDeleteProc *)NULL);
   (*createCmd)(interp,"SdsArrayGet",STclSdsArrayGet,(ClientData *)cmdError,
           (Tcl_CmdDeleteProc *)NULL);


   (*createCmd)(interp,"args",STclArgs,(ClientData *)cmdError,
           (Tcl_CmdDeleteProc *)NULL);
   (*createCmd)(interp,"SdsList",STclSdsList,(ClientData *)cmdError,
           (Tcl_CmdDeleteProc *)NULL);
   (*createCmd)(interp,"SdsRead",STclSdsRead,(ClientData *)cmdError,
           (Tcl_CmdDeleteProc *)NULL);
   (*createCmd)(interp,"SdsReadFree",STclSdsReadFree,(ClientData *)cmdError,
           (Tcl_CmdDeleteProc *)NULL);
   (*createCmd)(interp,"SdsWrite",STclSdsWrite,(ClientData *)cmdError,
           (Tcl_CmdDeleteProc *)NULL);
   (*createCmd)(interp,"SdsFind",STclSdsFind,(ClientData *)cmdError,
           (Tcl_CmdDeleteProc *)NULL);
   (*createCmd)(interp,"SdsCopy",STclSdsCopy,(ClientData *)cmdError,
           (Tcl_CmdDeleteProc *)NULL);
   (*createCmd)(interp,"SdsInsert",STclSdsInsert,(ClientData *)cmdError,
           (Tcl_CmdDeleteProc *)NULL);
   (*createCmd)(interp,"SdsExtract",STclSdsExtract,(ClientData *)cmdError,
           (Tcl_CmdDeleteProc *)NULL);
   (*createCmd)(interp,"SdsRename",STclSdsRename,(ClientData *)cmdError,
           (Tcl_CmdDeleteProc *)NULL);
   (*createCmd)(interp,"SdsInfo",STclSdsInfo,(ClientData *)cmdError,
           (Tcl_CmdDeleteProc *)NULL);
   (*createCmd)(interp,"SdsIndex",STclSdsIndex,(ClientData *)cmdError,
           (Tcl_CmdDeleteProc *)NULL);
   (*createCmd)(interp,"SdsFreeId",STclSdsFreeId,(ClientData *)cmdError,
           (Tcl_CmdDeleteProc *)NULL);
   (*createCmd)(interp,"SdsFreeIdCheck",STclSdsFreeIdCheck,(ClientData *)cmdError,
           (Tcl_CmdDeleteProc *)NULL);
   (*createCmd)(interp,"SdsDelete",STclSdsDelete,(ClientData *)cmdError,
           (Tcl_CmdDeleteProc *)NULL);
   (*createCmd)(interp,"SdsFindByPath",STclSdsFindByPath,
   		(ClientData *)cmdError,
           (Tcl_CmdDeleteProc *)NULL);
   (*createCmd)(interp,"SdsNew",STclSdsNew,
   		(ClientData *)cmdError,(Tcl_CmdDeleteProc *)NULL);
   (*createCmd)(interp,"SdsCell",STclSdsCell,
   		(ClientData *)cmdError, (Tcl_CmdDeleteProc *)NULL);
   (*createCmd)(interp,"SdsInsertCell",STclSdsInsertCell,
   		(ClientData *)cmdError, (Tcl_CmdDeleteProc *)NULL);
   (*createCmd)(interp,"SdsFillArray",STclSdsFillArray,
   		(ClientData *)cmdError, (Tcl_CmdDeleteProc *)NULL);

   (*createCmd)(interp,"SdsResize",STclSdsResize,
   		(ClientData *)cmdError, (Tcl_CmdDeleteProc *)NULL);

   (*createCmd)(interp,"SdsGetExtra",STclSdsGetExtra,
   		(ClientData *)cmdError, (Tcl_CmdDeleteProc *)NULL);

   (*createCmd)(interp,"SdsPutExtra",STclSdsPutExtra,
   		(ClientData *)cmdError, (Tcl_CmdDeleteProc *)NULL);

   (*createCmd)(interp,"SdsPut",STclSdsPut,
   		(ClientData *)cmdError, (Tcl_CmdDeleteProc *)NULL);

   (*createCmd)(interp,"SdsGet",STclSdsGet,
   		(ClientData *)cmdError, (Tcl_CmdDeleteProc *)NULL);
}   


/*
 *  STclErsRep -> Used to report Tcl errors using Ers.
 */
static void STclErsRep(Tcl_Interp * interp, 
			 StatusType * status)
{
    register const char *i;
    const char *start;
    char result[256];
    strncpy(result,Tcl_GetStringResult(interp),sizeof(result));
    result[sizeof(result) - 1] = '\0';
/*
 *  Initialise link break code.
 */        
    i = result;
    start = result;
        
/*
 *  Break at requested line breaks.
 */
    for (; *i ; ++i)
    {
/*
 *      If current character is \n then, if this the start of a line,
 *              ignore it, otherwise, output line so far.
 */
        if ((*i == '\n')&&(i == start))
            ++start;
        else if (*i == '\n')
        {
            char *outstring;
            int length = i-start;
            outstring = malloc(length+1);
            strncpy(outstring,start,length);
            outstring[length] = '\0';
            ErsRepNF(0, status,outstring);
            free (outstring);
            start = i+1;
        }
    }
/*
 *  Output remaining part of line.
 */
    if (i > start)
    {
        char *outstring;
        int length = i-start;
        outstring = malloc(length+1);
        strncpy(outstring,start,length);
        outstring[length] = '\0';
        ErsRepNF(0,status,outstring);
        free (outstring);
    }
}

