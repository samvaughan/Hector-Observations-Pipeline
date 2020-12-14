/*			E r s _ S t a r. c

 *  Module name:
      Ers_Star.c

 *  Function:
      Implements the Ers (Error reporting system) routines for using
      in Starlink programs.

 *  Description:

      Implements the Wham Ers routines (Except ErsStart and ErsStop) on top
      of the Starlink Err routines so that routines call Ers can be used
      in starlink applications.


 *  Language:
      C

 *  Support: Tony Farrell, AAO

 *  Copyright (c) Anglo-Australian Telescope Board, 1995.
    Not to be used for commercial purposes without AATB permission.
  

 *  History:
      20-Oct-1992 - TJF - Original version.
      29-Sep-1993 - TJF - Add Sccs id
      {@change entry@}
 
 
 *   @(#) $Id: ACMM:DramaErs/ers_star.c,v 3.19 09-Dec-2020 17:17:02+11 ks $
 
 */
/*
 *  Include files
 */
#define ERS_MAIN	/* Indicates to include files we are compling this module */


static char *rcsId="@(#) $Id: ACMM:DramaErs/ers_star.c,v 3.19 09-Dec-2020 17:17:02+11 ks $";
static void *use_rcsId = (0 ? (void *)(&use_rcsId) : (void *) &rcsId);


#include "Ers.h" 
#include "Ers_Err.h" 
#include <string.h>
#include <stdio.h>

/*
 *  Pick up VMS include files
 */
#ifdef VMS
#    include "strmac.h"/* Vms macro's for efficient implements of string functions*/
#endif
/*
 *  We perfer stdarg.h, but not all machines have it 
 */
#ifdef ERS_STDARG_OK
#    include <stdarg.h>			/* Variable argument lists (ansi)*/
#else
#    include <varargs.h>		/* Variable argument lists (old) */
#endif

/*
 *  Fudge prorotypes for C run time library system functions  (sun only)
 */
#ifdef ERS_FUDGE_PROTOTYPES_C
     int vsprintf (char *string,  ERS_CONST_ARGPNT char *format, ...);
#endif

#pragma nostandard
#include "cnf.h"
#include "f77.h"
#pragma standard


/*
 *  Prototypes for Starlink err routines
 */
extern F77_SUBROUTINE(err_rep)( CHARACTER (name),
				CHARACTER (message),
				INTEGER (status)
				TRAIL(name) TRAIL(message) );
extern F77_SUBROUTINE(err_out)( CHARACTER (name),
				CHARACTER (message),
				INTEGER (status)
				TRAIL(name) TRAIL(message) );
extern F77_SUBROUTINE(err_annul)( INTEGER (status));
extern F77_SUBROUTINE(err_flush)( INTEGER (status));
extern F77_SUBROUTINE(err_mark)( void );
extern F77_SUBROUTINE(err_rlse)( void );

/*
 *+			E r s R e p

 *  Function name:
      ErsRep

 *  Function:
      Report an Error message.


 *  Description:
	Starlink version of ErsRep.  flags argument is ignored

 *  Language:
      C

 *  Call:
      (Void) = ErsRep (flags, status, format, [arg ,[...]])

 *  Parameters:   (">" input, "!" modified, "W" workspace, "<" output)
      (>) flags		(Int ) Ignored. 
      (!) status	(Long int *) As per ERR_REP.
      (>) format    	(Char *) A format statement.  See the description
			of the C printf function.
      (>) arg		(assorted) Formating arguments.  Set the description
			of the C printf function.


 *  Include files: Ers.h

 *  External functions used:

 *  External values used: 
	none

 *  Prior requirements:
	none 

 *  Support: Tony Farrell, AAO

 *-

 *  History:
      06-Oct-1992 - TJF - Original version
 */
/*
 *  Most unix machines do not yet have strarg.h, only the old varargs.h
 *  As a result, they can't handle the new style variable argument
 *  handling. Note that if ERS_STDARG_OK is true, then so is ERS_PROTOTYPES_OK
 */
#ifdef ERS_STDARG_OK	    
    extern void ErsRep (ERS_CONST_ARGVAL int flags, long int *status,
			ERS_CONST_ARGPNT char * fmt,...)
#else
    extern void ErsRep (va_alist)
    va_dcl
#endif
{
    DECLARE_INTEGER(fstat);
#   pragma nostandard
    DECLARE_CHARACTER(nullstr,1);
    DECLARE_CHARACTER(mess,ERS_C_LEN);
#   pragma standard

    va_list args;
    char message[1000];

#   ifdef ERS_STDARG_OK
	va_start(args,fmt);
#   else
        long int *status;
	int  flags;
	char *fmt;
        va_start(args);
        flags = va_arg(args, int);
        status = va_arg(args, long int *);
	fmt = va_arg(args, char *);
#   endif

    if (flags && ERS_M_NOFMT)
	strncpy(message,fmt,ERS_C_LEN);
    else
    {
        vsprintf(message,fmt,args);
	message[999] = '\0';
    }
/*
 *  Report the formated message
 */
    fstat = *status;
    cnf_exprt(message,mess,ERS_C_LEN);
    cnf_exprt("",nullstr,1);
    F77_EXTERNAL_NAME(err_rep)( CHARACTER_ARG(nullstr),
				CHARACTER_ARG(mess),
				INTEGER_ARG(&fstat)
				TRAIL_ARG(nullstr) TRAIL_ARG(mess) );
    *status = fstat;
}


/*
 *+			E r s O u t

 *  Function name:
      ErsOut

 *  Function:
      Report and output an Error message.


 *  Description:
	Starlink version of ErsOut.  flags argument is ignored

 *  Language:
      C

 *  Call:
      (Void) = ErsOut (flags, status, format, [arg ,[...]])

 *  Parameters:   (">" input, "!" modified, "W" workspace, "<" output)
      (>) flags		(Int ) Ignored
      (!) status	(Long int *) As per ERR_REP followed by ERR_FLUSH
      (>) format    	(Char *) A format statement.  See the description
			of the C printf function.
      (>) arg		(assorted) Formating arguments.  Set the description
			of the C printf function.


 *  Include files: Ers.h

 *  External functions used:
 *  External values used: 
	none

 *  Prior requirements:
	none 

 *  Support: Tony Farrell, AAO

 *-

 *  History:
      06-Oct-1992 - TJF - Original version
 */
/*
 *  Most unix machines do not yet have strarg.h, only the old varargs.h
 *  As a result, they can't handle the new style variable argument
 *  handling. Note that if ERS_STDARG_OK is true, then so is ERS_PROTOTYPES_OK
 */
#ifdef ERS_STDARG_OK	    
    extern void ErsOut (ERS_CONST_ARGVAL int flags, long int *status, 
			ERS_CONST_ARGPNT char * fmt,...)
#else
    extern void ErsOut (va_alist)
    va_dcl
#endif
{
    DECLARE_INTEGER(fstat);
#   pragma nostandard
    DECLARE_CHARACTER(nullstr,1);
    DECLARE_CHARACTER(mess,ERS_C_LEN);
#   pragma standard

    va_list args;
    char message[1000];

#   ifdef ERS_STDARG_OK
	va_start(args,fmt);
#   else
        long int *status;
	int  flags;
	char *fmt;
        va_start(args);
        flags = va_arg(args, int);
        status = va_arg(args, long int *);
	fmt = va_arg(args, char *);
#   endif

    if (flags && ERS_M_NOFMT)
	strncpy(message,fmt,ERS_C_LEN);
    else
    {
        vsprintf(message,fmt,args);
	message[999] = '\0';
    }

/*
 *  Report the formated message and flush it
 */

    fstat = *status;
    cnf_exprt(message,mess,ERS_C_LEN);
    cnf_exprt("",nullstr,1);
    F77_EXTERNAL_NAME(err_rep)( CHARACTER_ARG(nullstr),
				CHARACTER_ARG(mess),
				INTEGER_ARG(&fstat)
				TRAIL_ARG(nullstr) TRAIL_ARG(mess) );
    F77_EXTERNAL_NAME(err_flush)(INTEGER_ARG(&fstat));
    *status = fstat;

}



/*
 *+			E r s P u s h

 *  Function name:
      ErsPush

 *  Function:
	 Increase Error context level

 *  Description:
	Starlink version of ErsPush.  Calls ERR_MARK.

 *  Language:
      C

 *  Call:
      (Void) = ErsPush ()

 *  Parameters:   (">" input, "!" modified, "W" workspace, "<" output)
	    none


 *  Include files: Ers.h

 *  External functions used:
	    none

 *  External values used: 
	    none

 *  Prior requirements:
	    none 

 *  Support: Tony Farrell, AAO

 *-

 *  History:
      06-Oct-1992 - TJF - Original version
 */
#ifdef ERS_PROTOTYPES_OK
    extern void ErsPush(void)
#else
    extern void ErsPush()
#endif
{
    F77_EXTERNAL_NAME(err_mark)();
}




/*
 *+			E r s P o p

 *  Function name:
      ErsPop

 *  Function:
	 Decrease Error context level

 *  Description:
	Starlink version of ErsPop.  Calls ERR_RLSE.

 *  Language:
      C

 *  Call:
      (Void) = ErsPop ()

 *  Parameters:   (">" input, "!" modified, "W" workspace, "<" output)


 *  Include files: Ers.h

 *  External functions used:
	    none

 *  External values used: 
	    none

 *  Prior requirements:
	    none 

 *  Support: Tony Farrell, AAO

 *-

 *  History:
      06-Oct-1992 - TJF - Original version
 */
#ifdef ERS_PROTOTYPES_OK
    extern void ErsPop(void)
#else
    extern void ErsPop()
#endif
{
    F77_EXTERNAL_NAME(err_rlse)();
}   



/*
 *+			E r s A n n u l

 *  Function name:
      ErsAnnul

 *  Function:
	 Annul all Error messages in the current context.

 *  Description:
	Starlink version of ErsAnnul.  Calls ERR_ANNUL

 *  Language:
      C

 *  Call:
      (Void) = ErsAnnul (status)

 *  Parameters:   (">" input, "!" modified, "W" workspace, "<" output)
	(<) status	(Long int *) As per ERR_ANNUL.


 *  Include files: Ers.h

 *  External functions used:
	    none

 *  External values used: 
	    none

 *  Prior requirements:
	    none 

 *  Support: Tony Farrell, AAO

 *-

 *  History:
      06-Oct-1992 - TJF - Original version
 */
#ifdef ERS_PROTOTYPES_OK
    extern void ErsAnnul(long int * status)
#else
    extern void ErsAnnul(status)
    long int * status;
#endif
{
    DECLARE_INTEGER(fstat);
    fstat = *status;
    F77_EXTERNAL_NAME(err_annul)( INTEGER_ARG(&fstat) );
    *status = fstat;
}   

/*
 *+			E r s F l u s h

 *  Function name:
      ErsFlush

 *  Function:
	 Flush all error messages at the current context.

 *  Description:
	Starlink version of ErsFlush.  Calls ERR_FLUSH.

 *  Language:
      C

 *  Call:
      (Void) = ErsFlush (status)

 *  Parameters:   (">" input, "!" modified, "W" workspace, "<" output)
	(!) status	(Long int *) As per ERR_FLUSH.

 *  Include files: Ers.h

 *  External functions used:
	    none

 *  External values used: 
	    none

 *  Prior requirements:
	    none 

 *  Support: Tony Farrell, AAO

 *-

 *  History:
      06-Oct-1992 - TJF - Original version
 */
#ifdef ERS_PROTOTYPES_OK
    extern void ErsFlush(long int * status)
#else
    extern void ErsFlush(status)
    long int * status;
#endif
{
    DECLARE_INTEGER(fstat);
    fstat = *status;
    F77_EXTERNAL_NAME(err_flush)( INTEGER_ARG(&fstat) );
    *status = fstat;
}   


