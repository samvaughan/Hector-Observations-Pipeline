#ifndef ERSINC
#define ERSINC
/*			E r s . h

 *  Module name:
      Ers.h

 *  Function:
	Function header for the Ers routines

 *  Description:
     Should be included by all files using the Ers routines.

     Version of Ers.h kept in SDS library for building Standalone releases.

 *  Language:
      C

 *  Support: Tony Farrell, AAO

 *  

 *  @(#) $Id: ACMM:sds/old_standalone/Ers.h,v 3.94 09-Dec-2020 17:15:38+11 ks $

 *  History:
      04-Aug-1992 - TJF - Original version
      25-Sep-1992 - TJF - Update comments
      06-Oct-1992 - TJF - Rewrite for complete Ers package.
      {@change entry@}
 */

/*
 *  Get around problems in Sparc include files, they are not ANSI compatible
 */
#if defined(__sparc__) && !defined(sparc)
#define sparc 1
#endif

/*
 *  Configuration section
 * 
 *  We currently know about VAX (VMS), mips (ultrix), sparc (sunos).  We know
 *  about Gnu CC under mips (ultrix) and sparc (sunos).
 *
 *
 *  Do we use prototypes.  We can use prototypes on everything with standard ansi
 *  C (__STDC__ defined).  We can also use them on VAXC and ultrix.  
 */
#if defined (VAXC)  || defined(__STDC__) || defined(ultrix) || defined (__alpha)  
#   define ERS_PROTOTYPES_OK
#endif

/*
 *  Can we use const arguments to routines.  Only on VAXC and I think, standard
 *  C, we can use them all the time.  Under Gnu, we get some warnings when 
 *  compiling Dits itself due to Imp and C routines not being declared  
 *  correctly.  So don't declare const arguments for Gnu.
 *
 *	ERS_CONST_ARGVAL   - Use for const argument values
 *	ERS_CONST_ARGPNT   - USe for const argument pointers
 *	ERS_CONST_VAR	    - Use for const variables.
 *
 *  Note that ERS_MODULE is defined for in all Dits modules.
 */

#ifdef ERS_PROTOTYPES_OK
#   if defined (VAXC) 
#       define ERS_CONST_ARGVAL const
#       define ERS_CONST_ARGPNT const
#	define ERS_CONST_VAR const
#   elif defined(__GNUC__) && defined(ERS_MAIN)
#       define ERS_CONST_ARGVAL 
#       define ERS_CONST_ARGPNT 
#	define ERS_CONST_VAR const
#   elif defined(__STDC__)
#       define ERS_CONST_ARGVAL const
#       define ERS_CONST_ARGPNT const
#	define ERS_CONST_VAR const
#   else
#       define ERS_CONST_ARGVAL
#       define ERS_CONST_ARGPNT
#	define ERS_CONST_VAR
#   endif
#else
#   define ERS_CONST_ARGVAL
#   define ERS_CONST_ARGPNT
#   define ERS_CONST_VAR
#endif
/*
 *  Do we have the stdarg.h include file.  VMS and VxWorks do, but sun and ultrix
 *  do not.
 */

#ifdef ERS_PROTOTYPES_OK
#   if defined (VMS) || defined (VxWorks) || defined (__alpha)
#        define ERS_STDARG_OK
#   endif
#endif

/*
 *  If we are compiling with prototypes, but we don't have prototypes for
 *  run time functions, define this so they can be provided by ers.c.
 *  (The _UNIX version is for unix system functions, the C version for 
 *   _C run time library functions)
 */
#ifdef ERS_PROTOTYPES_OK
#   if defined(unix) || defined(__unix__)
#        define ERS_FUDGE_PROTOTYPES_UNIX
#   endif
#   ifdef solaris2
#   elif defined(sun) || defined(__sun__)
#        define ERS_FUDGE_PROTOTYPES_C
#   endif
#endif

/*
 *  Define this to pick up unix specfic include files.  Use VMS and VxWorks
 *  macros for VMS and VxWorks specific include files.
 */
#if defined(unix) || defined(__unix__)
#   define ERS_UNIX_INCLUDES
#endif

#include "status.h"		/* For StatusType etc	*/

/*
 *  Floating point stuff.  Only used in ErsVSPrintf.
 */

#define ERS_FLOAT_OK
/*
 *  Of the supported machines, only a sun not using gnu does not have float.h
 */
#ifdef ERS_FLOAT_OK
/*
 *  These values taken from bsd floatio.h
 */
#    define ERS_MAXEXP 308
#    define ERS_MAXFRACT 39
#endif

/*
 *  Constants
 */

#define ERS_C_LEN 200		/* Maximum length of reported messages */
#define ERS_C_MAXMSG 30		/* Maximum number of reported messages */

#define ERS_M_NOFMT (1<<0)	/* Message flag masks		*/
#define ERS_M_HIGHLIGHT (1<<1)
#define ERS_M_BELL (1<<2)
#define ERS_M_ALARM (1<<3)


/* 
 *  This structure is used to store details of a message
 */
typedef struct {
	    StatusType mesStatus;			    /* Status of message    */
	    unsigned int context;		    /* Context message was written at */
	    int    flags;			    /* Message flags	    */
	    char   message[ERS_C_LEN];		    /* The formated message */
	    } ErsMessageType;

#ifdef ERS_PROTOTYPES_OK
/* 
 *  This type is that required for log routines - called on each call to 
 *  ErsRep with details of a single message.
 * 
 *  The argument "logArg" is a user value supplied when ErsStart is called.
 *  It enables the user to pass any appropriate value to the log routine.
 */
typedef void (*ErsLogRoutineType)(
		    void * logArg,	    /* Supplied to ErsStart  */
		    ErsMessageType * message,/* The message  */
		    StatusType * status);
/*
 *  The type is that requried for the output routine - called to output 
 *  the messages to the user.  An array of message may be output by one
 *  call, with count being the number of message to output.
 *
 *  The argument "outArg" is a user value supplied when ErsStart is called.
 *  It enables the user to pass any appropriate value to the log routine.
 */
typedef void (*ErsOutRoutineType)(
		    void * outArg,	    /* Supplied to ErsStart */
		    unsigned int count,	    /* Number of messages   */
		    ErsMessageType messages[],/* Array of messages */
		    StatusType * status);

/*
 *  Function prototypes.
 *
 * 
 *  We can't define these prorotype in the Ers main module unless we have
 *  stdarg.h.
 */
#if !defined(ERS_MAIN) || defined(ERS__STDARG__OK)
    extern void ErsRep(ERS_CONST_ARGVAL int flags, StatusType *status, 
		       ERS_CONST_ARGPNT char *, ...);
    extern void ErsOut(ERS_CONST_ARGVAL int flags, StatusType *status, 
		       ERS_CONST_ARGPNT char *, ...);
    extern int  ErsSPrintf(ERS_CONST_ARGVAL int maxLength, 
			char *string, 
			ERS_CONST_ARGPNT char * fmt,...);

#endif

extern void ErsStart(
		ErsOutRoutineType outRoutine,
		void * outArg,
		ErsLogRoutineType logRoutine,
		void * logArg,
		StatusType * status);
extern void ErsStop(StatusType * status);
extern void ErsPush(void);
extern void ErsPop(void);
extern void ErsAnnul(StatusType * status);
extern void ErsFlush(StatusType * status);
extern void ErsClear(StatusType *status);


#ifdef ERS_STDARG_OK
#   include <stdarg.h>
#else
#   include <varargs.h>
#endif
extern int ErsVSPrintf(
		 ERS_CONST_ARGVAL int maxLength, 
		 char *string , 
		 ERS_CONST_ARGPNT char *fmt0, 
		 va_list ap);
#else
/* Don't use prorotypes */
typedef void (*ErsLogRoutineType)();
typedef void (*ErsOutRoutineType)();

extern void ErsRep();
extern void ErsOut();

extern void ErsStart();
extern void ErsStop();
extern void ErsPush();
extern void ErsPop();
extern void ErsAnnul();
extern void ErsFlush();
extern void ErsClear();

extern int ErsVSPrintf();
extern int ErsSPrintf();

#endif	



#endif
