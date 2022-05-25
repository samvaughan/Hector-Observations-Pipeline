#ifndef ERSINC
#define ERSINC
#ifdef __cplusplus
extern "C" {
#endif


/*			E r s . h

 *  Module name:
      Ers.h

 *  Function:
	Function header for the Ers routines

 *  Description:
     Should be included by all files using the Ers routines.

 *  Language:
      C

 *  Support: Tony Farrell, AAO

 *  Copyright (c) Anglo-Australian Telescope Board, 1995.
    Not to be used for commercial purposes without AATB permission.
  
 *     @(#) $Id: ACMM:DramaErs/Ers.h,v 3.19 09-Dec-2020 17:17:02+11 ks $
  

 *  History:
      04-Aug-1992 - TJF - Original version
      25-Sep-1992 - TJF - Update comments
      06-Oct-1992 - TJF - Rewrite for complete Ers package.
      04-Aug-1993 - TJF - maxsize argument to ErsSPrintf needs a type
      28-Sep-1993 - TJF - Use GNUC attribute to flag the ers calls
			  as printf style.  Use drama.h for configuration
			  stuff.

      29-Sep-1993 - TJF - Add Sccs id
      06-Mar-1994 - TJF - Add Task Id stuff.
      05-Feb-1995 - TJF - Add BROADCAST flag
      06-Aug-1996 - TJF - Add const to strings arguments of ErsVSPrintf
      30-May-2001 - TJF - Add ErsSetLogRoutine.
      15-Jun-2001 - TJF - Add ErsGetTaskId().
      04-Mar-2014 - TJF - Add ErsGetAtCtx()
      18-Aug-2015 - TJF - Add ErsRepNF() and make ERS_M_NOFMT as deprecated using
                            _Pragma (GCC and CLang).
      03-Sep-2015 - TJF - The previous _Pragma spec was wrong, picked up by CLang.  Fixed.
      25-Oct-2016 - TJF - Add ERS___M___NOFMT, to allow internal code to remove the NOFMT
	                       flag etc. without picking up deprecated warnings.
      {@change entry@}
 
 
 */

#ifdef ERS_STANDALONE
/*
 *  DRAMA macros and types used by Ers.  They are defined here when we
 *  are building ers standalone.
 */
#define DUNUSED 
#define STATUS__OK 0
#define DFLOAT_OK
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
typedef long int StatusType;
#define StatusOkP(_value_)  (*(_value_) == STATUS__OK)
#else
/*
 *  Include the drama.h file for configuration macros. 
 */

#include "drama.h"

#include "status.h"		/* For StatusType etc	*/
#endif

#include <stdarg.h>
#include <stdio.h>

/*
 *  Get around problems in Sparc include files, they are not ANSI compatible
 */
#if defined(__sparc__) && !defined(sparc)
#define sparc 1
#endif

/*
 *  Floating point stuff.  Only used in ErsVSPrintf.
 */
#ifdef DFLOAT_OK
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

/*
 * ERS_M_NOFMT should no longer be used. If calling ErsRep() with it, replace
 * by a call to ErsRepNF().  This avoids compiler warnings about passing 
 * unknown strings as format strings. 
 *
 *  We use _Pragma (C99) and only do this test for GCC (which also does Apple 
 * CLang) since we don't otherwise have a way of being sure it works.  But
 * we don't do it for ers.c (ERS_MAIN defined) since we still need to support
 * it in the implementatoins.
 */
#if defined(__GNUC__) && !defined(ERS_MAIN)
#define ERS_M_NOFMT _Pragma ("GCC warning \"'ERS_M_NOFMT' macro is deprecated, use ErsRepNF() function instead.\"") (1<<0) 
#else
#define ERS_M_NOFMT (1<<0)	/* Message flag masks		*/
#endif
#define ERS___M___NOFMT (1<<0)	/* Message flag masks - special version for internal code, no deprecrated check	*/
#define ERS_M_HIGHLIGHT (1<<1)
#define ERS_M_BELL (1<<2)
#define ERS_M_ALARM (1<<3)
#define ERS_M_BROADCAST (1<<4)


/* 
 *  This structure is used to store details of a message
 */
typedef struct {
	    StatusType mesStatus;	    /* Status of message    */
	    unsigned int context;	    /* Context message was written at */
	    int    flags;		    /* Message flags	    */
	    char   message[ERS_C_LEN];	    /* The formated message */
	    } ErsMessageType;

typedef void * ErsTaskIdType;

/* 
 *  This type is that required for log routines - called on each call to 
 *  ErsRep with details of a single message.
 * 
 *  The argument "logArg" is a user value supplied when ErsStart is called.
 *  It enables the user to pass any appropriate value to the log routine.
 */
typedef void (*ErsLogRoutineType)(
		    void * logArg,	    /* Supplied to ErsStart  */
		    const ErsMessageType * message,/* The message  */
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
		    const ErsMessageType messages[],/* Array of messages */
		    StatusType * status);


/*
 *  Function prototypes.
 *
 */
extern void ErsRep(const int flags, StatusType * status, 
		       const char * string , ...)
#ifdef __GNUC__
	__attribute__ ((format (printf, 3, 4)))
#endif
		;
extern void ErsOut(const int flags, StatusType * status, 
		          const char * string, ...)
#ifdef __GNUC__
	__attribute__ ((format (printf, 3, 4)))
#endif
		;
extern int  ErsSPrintf(const int maxLength, 
			char *string, 
			const char * fmt,...)
#ifdef __GNUC__
	__attribute__ ((format (printf, 3, 4)))
#endif
		;


extern ErsTaskIdType ErsStart(
		ErsOutRoutineType outRoutine,
		void * outArg,
		ErsLogRoutineType logRoutine,
		void * logArg,
		StatusType * status);
extern void ErsStop(StatusType * status);
extern void ErsPush(void);
extern void ErsAnnul(StatusType * status);
extern void ErsFlush(StatusType * status);
extern void ErsClear(StatusType * status);
extern void ErsPop(void);
extern void ErsSetLogRoutine(
    ErsLogRoutineType logRoutine,
    void * logArg,
    ErsLogRoutineType *oldLogRoutine,
    void * *oldLogArg,
    StatusType * status);

extern ErsTaskIdType ErsGetTaskId(StatusType *status);
extern void ErsEnableTask(ErsTaskIdType TaskId,
			    ErsTaskIdType * SavedTaskId);
extern void ErsRestoreTask(ErsTaskIdType TaskId);
extern void ErsGetAtCtx(unsigned *count, ErsMessageType **messageArray,
                          StatusType * status);

extern void ErsRepNF(const int flags, StatusType * status, 
		       const char * string);


extern int ErsVSPrintf(
		 int maxLength, 
		 char *string , 
		 const char * fmt0, 
		 va_list ap);



#ifdef __cplusplus
}
#endif

#endif
