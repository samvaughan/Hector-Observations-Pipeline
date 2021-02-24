/*			E r s. c

 *  Module name:
      Ers.c

 *  Function:
      Implements the Ers (Error reporting system) routines

 *  Description:comp
      ErsStart => Startup Error system
      ErsStop  => Shutdown Error system
      ErsRep => Report an Error message
      ErsOut => Report an Error message and flush .
      ErsPush => Increase Error context
      ErsPop  => Decrase Error context
      ErsAnnul => Annul messages at current Error context
      ErsFlush => Flush messages at current Error context
      ErsClear => Flush all messages and rest context to lowest level
      ErsOut => Outputs a Error message to the user interface.
      ErsGetTaskId  => Return a task id.
      ErsEnableTask => Enable ERS context for a given task.
      ErsRestoreTask => Retore ERS
      ErsSetLogRoutine => Set the logging routine.

 *  Language:
      C

 *  Support: Tony Farrell, AAO

 *  Copyright (c) Anglo-Australian Telescope Board, 1995.
    Not to be used for commercial purposes without AATB permission.
  

 *  History:
      04-Aug-1992 - TJF - Original version under Dits.
      06-Oct-1992 - TJF - Rewrite for complete Ers package.
      31-Mar-1993 - TJF - Got rid of ErsInitialise/ErsShutdown.  Add ErsClear.
      28-Sep-1993 - TJF - USe drama.h configuration stuff.
      29-Sep-1993 - TJF - Add Sccs id
      11-Feb-1993 - TJF - Catch reports while flushing. 
			   Ensure ErsClear returns status ok to stop drama
			   failing.
      16-May-1997 - TJF - Replace all calls to malloc and free with
      				ErsMalloc and ErsFree to help debug
      				memory problems.   This are in turn just
				defined to be malloc and free in the normal
				case.
      27-May-1998 - TJF - Fix linux compilation warnings.
      04-Mar-2014 - TJF - Add ErsGetAtCtx().
 

 *     @(#) $Id: ACMM:DramaErs/ers.c,v 3.19 09-Dec-2020 17:17:02+11 ks $
 
 */
/*
 *  Include files
 */
#define ERS_MAIN			/* Indication to Ers.h		*/

/*
 *
 */
#ifndef	 DNOCONST
#    define DNOCONST 1
#endif


static char *rcsId="@(#) $Id: ACMM:DramaErs/ers.c,v 3.19 09-Dec-2020 17:17:02+11 ks $";
static void *use_rcsId = (0 ? (void *)(&use_rcsId) : (void *) &rcsId);


#include "Ers.h" 
#include "Ers_Err.h" 

/*
 Define ErsMalloc and ErsFree.  These are normally just defined
   to malloc and free but sometimes when debugging we want our
   Plug versions
*/

#define ErsMalloc(_size) (void *)malloc(_size)
#define ErsFree(_where)   (void)free((void *)(_where))
/*
void * PlugMalloc (const char *,int size);
void  PlugFree(const char *,void * where);
#define ErsMalloc(_size) PlugMalloc("ERS",(_size))
#define ErsFree(_where)   PlugFree("ERS",(_where))
 */



/*
 *  We perfer stdarg.h, but not all machines have it 
 */
#include <stdarg.h>			/* Variable argument lists (ansi)*/
#ifdef VxWorks_5_0
#    include <string.h>
#    include <stdioLib.h>		/* Vxworks stdio		*/
#    include <memLib.h>			/* VxWorks malloc and free	*/
#else /* Posix/Ansi */
#    include <string.h>
#    include <stdio.h>
#    include <stdlib.h>
#endif

#ifdef VMS
#    include "strmac.h"			/* Must be after string.h	*/
#elif defined(VxWorks)
#    include <taskVarLib.h>		/* taskVar routines		*/
#    include <logLib.h>                 /* log routines */
#    include <taskLib.h>                /*  task routines */
#    include <intLib.h>                 /* int routines */
#endif

/*
 * intContext indicates if we are in interrupt context on VxWorks - define
 * a macro which does the same job on other machines so we can use this
 * call cleanly.
 */
#ifndef VxWorks
#define intContext() (0)
#endif
/*
 *  Fudge prorotypes for C run time library system functions 
 */
#ifdef DFUDGE_PROTOTYPES_C
     extern int fputs (char * str, FILE *file_ptr);
     extern int fprintf (FILE *file, char * format, ...);
#endif
     
/*
 *  Define the structure which holds the message table
 *
 *  context is incremented by ErsPush, decremented by ErsPop. Starts at 1.
 *
 *  current indicites the next message we can write too.Range 0 - ERS_C_MAXMSG-1
 */

typedef struct {
    ErsOutRoutineType outRoutine;	/* Message output routine	*/
    void * outArg;			/* Argument for output routine	*/  
    ErsLogRoutineType logRoutine;	/* Message logging routine	*/
    void * logArg;			/* Argument for logging routine	*/
    unsigned int context;		/* Current context level	*/
    unsigned int current;		/* Current message pointer	*/
    ErsMessageType messages[ERS_C_MAXMSG];  /* Messages			*/
    int flushing;			/* Indicates we are currently flushing*/
    int interrupt;			/* Are we in an interrupt handler */
    } ErsBlockType;

/*
 *  ErsBlock is the message table
 */
static ErsBlockType *ErsBlock = 0;

/*
 *+			E r s S t a r t

 *  Function name:
      ErsStart

 *  Function:
	Startup Error reporting system.


 *  Description:
	Startup the Error reporting system.  Calls may be made to the other
	Ers routines without calling this routine, but if this is done, then
	messages reported with ErsRep are written directly to the user standard
	Error output, not stored.

	

 *  Language:
      C

 *  Call:
      (ErsTaskIdType) = ErsStart (outRoutine, outArg, logRoutine, logArg, status)

 *  Parameters:   (">" input, "!" modified, "W" workspace, "<" output)
	(>) outRoutine (ErsOutRoutineType) If non zero, this routine will
			be called to output any messages output by ErsFlush.
	(>) outArg	(void *) Passed directly to outRoutine as its first
			argument and not examined by Ers.
	(>) logRoutine (ErsLogRoutineType) If non zero, this routine will
			be called to log any messages reported by ErsRep.
	(>) logArg	(void *) Passed directly to logRoutine as its first
			argument and not examined by Ers.
	(!) status	(StatusType *)Modified status.  routine returns
			immediately if non-zero.  If Ers is already active,
			it is set to ERS__ACTIVE.   An error allocating
			space for the table will set status to
			ERS__MALLOCERR.  Under VxWorks, an error
			setting up a task varable will cause it to be
			set to ERS__TASKVARERR.  


 *  Include files: Ers.h

 *  Function Value: A value which can be passed to ErsEnableTask from an
		interrupt handler.

 *  External functions used:
	    malloc	(Crtl)	Allocate memory.
	    taskVarAdd	(Crtl)  Add a task variable.
	    none

 *  External values used: 
	    none

 *  Prior requirements:
	    none 

 *  Support: Tony Farrell, AAO

 *-

 *  History:
      06-Oct-1992 - TJF - Original version
      06-Mar-1994 - TJF - Add interrupt variable and return ErsBlock.
 */
extern ErsTaskIdType ErsStart(
    ErsOutRoutineType outRoutine,
    void * outArg,
    ErsLogRoutineType logRoutine,
    void * logArg,
    StatusType * status)
{
    if (!StatusOkP(status)) return(0);

    if (ErsBlock)
    {
	*status = ERS__ACTIVE;/* Error, already active */
	return(0);
    }
    else
    {	
	
#       ifdef VxWorks
         if (taskVarAdd(0, (int *)&ErsBlock) != OK)
	 {
	    *status = ERS__TASKVARERR;
	    return(0);
	 }
         else
#       endif 
         {
	    if ((ErsBlock = (ErsBlockType *)ErsMalloc(sizeof(ErsBlockType))) == 
                                                                    (void *)0)
	    {
	        *status = ERS__MALLOCERR;
		return(0);
	    }
            else	/* Initialise the structure */
	    {
	/*	ErsBlockType * t = ErsBlock;*/
	/*	logMsg("ErsStart, block add = %x\n",(int)t,0,0,0,0,0);*/
		
	        ErsBlock->outRoutine = outRoutine;
	        ErsBlock->outArg = outArg;
	        ErsBlock->logRoutine = logRoutine;
	        ErsBlock->logArg = logArg;
	        ErsBlock->context = 1;
	        ErsBlock->current = 0;
	        ErsBlock->flushing = 0;
		ErsBlock->interrupt = 0;
		return(ErsBlock);
            }
         }
    }
}



/*
 *+			E r s S t o p

 *  Function name:
      ErsStop

 *  Function:
	Shutdown Error reporting system.


 *  Description:
	The major effect of this routine is that in any future calls to 
	ErsRep, the message is reported immediately to stderr.  The Output
	and logging routine supplied by the previous call to ErsStart are
	forgotton.

	The first thing this routine does is flush all pending messages.
	

 *  Language:
      C

 *  Call:
      (Void) = ErsStop (status)

 *  Parameters:   (">" input, "!" modified, "W" workspace, "<" output)
	(!) status	(StatusType *) Modified status.  routine returns
			immediately if non-zero.  If Ers is not active,
			it is set to ERS__NOTACTIVE.  Under VxWorks, an error
			deleting a task varable will cause it to be
			set to ERS__TASKVARERR.


 *  Include files: Ers.h

 *  External functions used:
	    ErsClear	(Ers)	Flush all outstanding errors.
	    free	(Crtl)	Release memory.
	    taskVarDelete (VxWorks) Delete a task variable.

 *  External values used: 
	    none

 *  Prior requirements:
	    none 

 *  Support: Tony Farrell, AAO

 *-

 *  History:
      06-Oct-1992 - TJF - Original version
 */
extern void ErsStop(StatusType * status)
{
/*	ErsBlockType * t = ErsBlock;*/
/*	logMsg("ErsStop, block add = %x\n",(int)t,0,0,0,0,0);*/



    ErsClear(status);
    if (!StatusOkP(status)) return;
/*
 *  Free the memory used for messages.
 */
    ErsFree(ErsBlock);
    ErsBlock = 0;
#   ifdef VxWorks
 	if (taskVarDelete(0, (int *)&ErsBlock) != OK)
	 *status = ERS__TASKVARERR;
#   endif
}



/*
 *  Internal Function, name:
      Ers___Report

 *  Description:
      Report an error message.  The message has already being formatted

 *  History:
      15-Oct-1992 - TJF - Original version
      18-Jan-1994 - TJF - Don't pass the NOFMT flag up, it has already
			been handled.
      06-Mar-1994 - TJF - Handle interrupt and flushing flags
      28-Jan-1997 - TJF - Format logMsg message ourselves to avoid it
      			accessing stack information.
      29-Apr-1998 - TJF - On buffer full, instead of outputing just this
                        messages, flush the lot.  This should help in cases
                        were ErsRep is being called in a error loop.
      19-Apr-2005 - TJF - Catch real interrupt context under VxWorks by
                       using intContext() - incase the caller gets it wrong.
 */

extern void Ers___Report (int flags,  const char * message, StatusType *status)
{
/*
 *  If we are not running, just report the message to stderr.
 */
    if (!ErsBlock)
    {
	fputs("!!",stderr);
	fputs(message,stderr);
	fputs("\n",stderr);
    }
    else if ((ErsBlock->interrupt)||(intContext()))
    {
#	ifdef VxWorks
/*
 *	    Since logMsg call arguments (other then the first one) are read
 *	    last, we must format the "message" into our own buffer first.
 */

	    char string[ERS_C_LEN+20] ;
	    ErsVSPrintf(sizeof(string),string,"!!ErsInIntterupt:%s\n",message);
	    logMsg(string,0,0,0,0,0,0);

            if (!ErsBlock->interrupt)
                logMsg("!!ErsInInterrupt::ErsEnableTask() call not current when a call to ERS was made from an interrupt handler - likely programming error\n",
                       0,0,0,0,0,0); 
#	else
	    fprintf(stderr,"!!ErsInInterrupt:%s\n",message);
#	endif

    }
    else if (ErsBlock->flushing)
    {
	fprintf(stderr,"!!Error Report While Flushing:%s\n",message);
    }  
    else if (ErsBlock->current < ERS_C_MAXMSG)
    {
/*
 *	Save at current context level, message number ErsBlock->current,
 *	which is incremented.
 */
	register int i = ErsBlock->current++;
	memcpy(ErsBlock->messages[i].message,message,ERS_C_LEN);
/*
 *	Ensure the message is null terminated and save the other details.
 */
	ErsBlock->messages[i].message[ERS_C_LEN-1] = '\0'; 
	ErsBlock->messages[i].mesStatus = *status;
	ErsBlock->messages[i].flags = flags & (~ERS_M_NOFMT); 
	ErsBlock->messages[i].context = ErsBlock->context;
/*
 *	If a log routine has been provided, call it.
 */
	if (ErsBlock->logRoutine)
	{
	    StatusType outstat = STATUS__OK;
	    ErsBlock->logRoutine(ErsBlock->logArg,
				& ErsBlock->messages[i],
				& outstat);
	}
    }
    else    /* Error, no more space	*/
    {
	register unsigned i;
        unsigned int context =  1;

/*
 *      Previously, we just reported the new message.  This works ok if
 *      the error is simply that too many messages are reported in a otherwise
 *      correct sequence.  But if the error involves recursion in error
 *      reporting, it is better that we clear the stack and ensure the
 *      original messages is output.
 */
	fprintf(stderr,"!!The ERS system error table is full, possible recursion in error reporting\n");


	fprintf(stderr,"! Here are all messages reported so far\n");
/*
 *	Output the messages.
 */
	for (i = 0; i < ErsBlock->current ; ++i)
	{
            if (ErsBlock->messages[i].context > context)
            {
                fputs("! **** ErsPush invoked before this message ****\n",
                      stderr);
                context = ErsBlock->messages[i].context;
            }
	    fputs("! ",stderr);
	    fputs(ErsBlock->messages[i].message,stderr);
	    fputs("\n",stderr); 
        }

	fputs("! ",stderr);
	fputs(message,stderr);
	fputs("\n! **** End of message flush ****\n",stderr);
	*status = ERS__NOSPACE;


/*
 *      Forget the messages and clear the buffer.
 */
	ErsBlock->current = 0;
    }
}



/*
 *+			E r s R e p

 *  Function name:
      ErsRep

 *  Function:
      Report an Error message.


 *  Description:
      According to the Error context, the Error message is either sent
      to the user or retained in the Error table.  The latter case allows
      the application to take further action before deciding if the
      user should receive the message.  On successfull completion, status
      is returned unchanged unless an Error occurs in this routine.

      Output will only occur if the context is 0, in which case ErsStart
      has not been called and the message will be output
      using {\tt fputs} to {\tt stderr}.

      If a logging routine was specified when ErsStart was called, then
      the logging routine is called after the message is formated.

      The format and it's arguments are the the same as used by the 
      \typeit{printf} C RTL function.

      The maximum length of stored messages is given by the macro
      ERS_C_LEN (currently 200 characters including the null terminator) 
      so the result of the format should be less then this value.  If the 
      format results in a string greater then this value it is truncated.

 *  Language:
      C

 *  Call:
      (Void) = ErsRep (flags, status, format, [arg ,[...]])

 *  Parameters:   (">" input, "!" modified, "W" workspace, "<" output)
      (>) flags		(Int ) Message flags.  These flags influence the
			operation of the Ers system and are also passed to
			the logging and output routines when these are called
			for this message.
			Bits 0 to 7 are reserved to the Ers system while bits 
			8 - 15 are available for logging systems.  Other bits
			should not be used since they may not be available on
			16bit machines. The following masks are defined and 
			may be ORed for the desired effect-

			ERS_M_NOFMT => Don't format the string.  Any formating
			arguments are ignored and the format string is used
			as specified.  Deprecated!!  Call ErsRepNF() instead
                        of specifying this flag to ErsRep() to avoid compiler
                        warnings.
			ERS_M_HIGHLIGHT => Suggest to the user interface that
			the message should be highlighted.
			ERS_M_BELL => Suggest to the user interface that the
			terminal bell (or an equivalent) should be rung when
			the message is output.
			ERS_M_ALARM => Suggest to the user interface that
			this is urgent message which should be acknowledged
			by the user.

 
      (!) status	(StatusType *) The routine works regardless of the
			value of status.  If the message table is full, then
			status is set to ERS__NOSPACE, otherwise, it is
			not touched.
      (>) format    	(Char *) A format statement.  See the description
			of the C printf function.
      (>) arg		(assorted) Formating arguments.  Set the description
			of the C printf function.


 *  Include files: Ers.h

 *  External functions used:
      va_start  (Crtl) Start a variable argument session.
      va_args   (Crtl) Get an argument.
      va_end    (Crtl) End a variable argument session.
      fputs     (Crtl) Output a string.
      fprint    (Crtl) Output a formated string.
      memcpy    (Crtl) Copy one string to another.
      logMsg    (VxWorks) Log a message.
      ErsVSPrintf (Ers) Safe format.

 *  External values used: 
        stderr  (Crtl) The standard error device.

 *  Prior requirements:
	none 

 *  Support: Tony Farrell, AAO

 *-

 *  History:
      06-Oct-1992 - TJF - Original version
 */

extern void ErsRep (const int flags, StatusType *status, 
                    const char * const fmt,...)
{
    va_list args;

    va_start(args,fmt);
/*
 *  If not formating, just report the string.
 */

    if (flags & ERS_M_NOFMT)
    {
	Ers___Report(flags,fmt,status);
    }
/*
 *  If formating, then do the format.  The Ers version of vsprintf is 
 *  safe - no matter what format is passed to it, it won't overwrite the 
 *  string. Then report the result.
 */
    else
    {
	char message[ERS_C_LEN];
        ErsVSPrintf(sizeof(message),message,fmt,args);
	Ers___Report(flags,message,status);
    }
}



/*
 *+			E r s R e p N F

 *  Function name:
      ErsRepNF

 *  Function:
      Report an Error message.  No formating.


 *  Description:
      According to the Error context, the Error message is either sent
      to the user or retained in the Error table.  The latter case allows
      the application to take further action before deciding if the
      user should receive the message.  On successfull completion, status
      is returned unchanged unless an Error occurs in this routine.

      Output will only occur if the context is 0, in which case ErsStart
      has not been called and the message will be output
      using {\tt fputs} to {\tt stderr}.

      If a logging routine was specified when ErsStart was called, then
      the logging routine is called after the message is formated.

      The format and it's arguments are the the same as used by the 
      \typeit{printf} C RTL function.

      The maximum length of stored messages is given by the macro
      ERS_C_LEN (currently 200 characters including the null terminator) 
      so the result of the format should be less then this value.  If the 
      format results in a string greater then this value it is truncated.

 *  Language:
      C

 *  Call:
      (Void) = ErsRepNF (flags, status, string)

 *  Parameters:   (">" input, "!" modified, "W" workspace, "<" output)
      (>) flags		(Int ) Message flags.  These flags influence the
			operation of the Ers system and are also passed to
			the logging and output routines when these are called
			for this message.
			Bits 0 to 7 are reserved to the Ers system while bits 
			8 - 15 are available for logging systems.  Other bits
			should not be used since they may not be available on
			16bit machines. The following masks are defined and 
			may be ORed for the desired effect-

			ERS_M_NOFMT => Ignored.
			ERS_M_HIGHLIGHT => Suggest to the user interface that
			the message should be highlighted.
			ERS_M_BELL => Suggest to the user interface that the
			terminal bell (or an equivalent) should be rung when
			the message is output.
			ERS_M_ALARM => Suggest to the user interface that
			this is urgent message which should be acknowledged
			by the user.

 
      (!) status	(StatusType *) The routine works regardless of the
			value of status.  If the message table is full, then
			status is set to ERS__NOSPACE, otherwise, it is
			not touched.
      (>) string    	(Char *) String to output.


 *  Include files: Ers.h

 *  External functions used:
      fputs     (Crtl) Output a string.
      fprint    (Crtl) Output a formated string.
      memcpy    (Crtl) Copy one string to another.
      logMsg    (VxWorks) Log a message.
      ErsVSPrintf (Ers) Safe format.

 *  External values used: 
        stderr  (Crtl) The standard error device.

 *  Prior requirements:
	none 

 *  Support: Tony Farrell, AAO

 *-

 *  History:
      18-Aug-2015 - TJF - Original version
 */
extern void ErsRepNF (const int flags, StatusType *status, 
			   const char * const string)
{
    Ers___Report(flags,string,status);
}



/*
 *+			E r s O u t

 *  Function name:
      ErsOut

 *  Function:
      Report an Error message.


 *  Description:
      Implements the equivalent of  a call to ErsRep followed by a 
      call to ErsFlush.

      The format and it's arguments are the the same as used by the 
      \typeit{printf} C RTL function.

 *  Language:
      C

 *  Call:
      (Void) = ErsOut (flags, status, format, [arg ,[...]])

 *  Parameters:   (">" input, "!" modified, "W" workspace, "<" output)
      (>) flags		(Int ) Message flags.  These flags influence the
			operation of the Ers system and are also passed to
			the logging and output routines when these are called
			for this message.
			Bits 0 to 7 are reserved to the Ers system while bits 
			8 - 15 are available for logging systems.  Other bits
			should not be used since they may not be available on
			16bit machines. The following masks are defined and 
			may be ORed for the desired effect-

			ERS_M_NOFMT => Don't format the string.  Any formating
			arguments are ignored and the format string is used
			as specified.  Deprecated!!  Call ErsRepNF() followed
                        by ErsFlush() instead of specifying this flag to 
                        ErsOut() to avoid compiler warnings.
			ERS_M_HIGHLIGHT => Suggest to the user interface that
			the message should be highlighted.
			ERS_M_BELL => Suggest to the user interface that the
			terminal bell (or an equivalent) should be rung when
			the message is output.
			ERS_M_ALARM => Suggest to the user interface that
			this is urgent message which should be acknowledged
			by the user.

 
	(!) status	(StatusType *) Set to zero unless the message table 
			is full, when status is set to ERS__NOSPACE, or
			an output error occurs, in which case it is set to
			the error code returned by the output routine.
      (>) format    	(Char *) A format statement.  See the description
			of the C printf function.
      (>) arg		(assorted) Formating arguments.  Set the description
			of the C printf function.


 *  Include files: Ers.h

 *  External functions used:
      va_start	(Crtl) Start a variable argument session.
      va_args	(Crtl) Get an argument.
      va_end	(Crtl) End a variable argument session.
      fputs	(Crtl) Output a string.
      fprint	(Crtl) Output a formated string.
      memcpy	(Crtl) Copy one string to another.
      logMsg    (VxWorks) Log a message.
      ErsVSPrintf (Ers) Safe format.
      ErsFlush	(Ers)	Flush error messages.

 *  External values used: 
        stderr  (Crtl) The standard error device.

 *  Prior requirements:
	none 

 *  Support: Tony Farrell, AAO

 *-

 *  History:
      06-Oct-1992 - TJF - Original version
      06-Mar-1994 - TJF - Ensure a flush is always done.
 */
extern void ErsOut (const int flags, StatusType *status, 
                    const char * const fmt,...)
{
    va_list args;

    va_start(args,fmt);
/*
 *  If not formating, just report the string.
 */

    if (flags & ERS_M_NOFMT)
	Ers___Report(flags,fmt,status);
/*
 *  If formating, then do the format.  The Ers version of vsprintf is 
 *  safe - no matter what format is passed to it, it won't overwrite the 
 *  string. Then report the result.
 */
    else
    {
	char message[ERS_C_LEN];
        ErsVSPrintf(sizeof(message),message,fmt,args);
	Ers___Report(flags,message,status);
    }
/*
 *  Flush the message.  IF status is NOSPACE, we return that status to the
 *  user.
 */
    if (*status != ERS__NOSPACE)
        ErsFlush(status);
    else
    {
	StatusType ignore = STATUS__OK;
	ErsFlush(&ignore);
    }
}



/*
 *+			E r s P u s h

 *  Function name:
      ErsPush

 *  Function:
	 Increase Error context level

 *  Description:
	 Begin a new Error reporting context so that delivery of subsequently 
	 reported Error messages is defered and the messages held in the
	 Error table.  A Subsequent call to {\tt ErsAnnul} or {\tt ErsFlush} will 
         only annul or flush the contexts of the Error table within this 
         new context.

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
      06-Mar-1994 - TJF - Handle interrupt and flushing flags
      19-Apr-2005 - TJF - Add check on interrupt context.
 */
extern void ErsPush(void)
{
    if (!ErsBlock)
	; /* not active */
    else if ((ErsBlock->interrupt)||(ErsBlock->flushing)||(intContext()))
        return; /* Nothing to do	*/
    else
/*
 *	Just increase the error context.
 */
	ErsBlock->context++;
}




/*
 *+			E r s P o p

 *  Function name:
      ErsPop

 *  Function:
	 Decrease Error context level

 *  Description:
	 Pops the Error message table context, returning the Error Message
	 Service to the previous context.  Note that any messages pending
	 output will be passed to this previous context, not annulled.

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
      06-Mar-1994 - TJF - Handle interrupt and flushing flags
      19-Apr-2005 - TJF - Add interrupt context check.
 */
extern void ErsPop(void)
{
    register int i;
    if (!ErsBlock)
	return;	/* Not active */
    else if ((ErsBlock->interrupt)||(ErsBlock->flushing)||(intContext()))
        return; /* Nothing to do	*/

    else if (ErsBlock->context > 1)
	ErsBlock->context--;
/*
 *  Go back though all the messages at or above the next context and set
 *  them to the new context
 */
    for (i = ErsBlock->current-1; 
	 (i > 0) &&(ErsBlock->messages[i].context > ErsBlock->context) ; 
	 i--)
	ErsBlock->messages[i].context = ErsBlock->context;

}   



/*
 *+			E r s A n n u l

 *  Function name:
      ErsAnnul

 *  Function:
	 Annul all Error messages in the current context.

 *  Description:
	 All pending messages at the current context are annulled, i.e. deleted.  
	 The context level does not change

 *  Language:
      C

 *  Call:
      (Void) = ErsAnnul (status)

 *  Parameters:   (">" input, "!" modified, "W" workspace, "<" output)
	(<) status	(StatusType *) Set to zero.


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
      06-Mar-1994 - TJF - Handle interrupt and flushing flags
      08-Sep-2003 - TJF - Log the annul.
      29-Mar-2004 - TJF - Can't auto-initialise the logMessage variable since
                          Solaris cc does not accept this (C standard does
                          not actually allow it I beleive)
      19-Apr-2005 - TJF - Add interrupt context check.
  
 */
extern void ErsAnnul(StatusType * status)
{
    register int i;
    *status = STATUS__OK;
    if (!ErsBlock)
	return ;/* Not active	*/
    else if ((ErsBlock->interrupt)||(ErsBlock->flushing)||(intContext()))
        return; /* Nothing to do	*/
    else if (ErsBlock->current == 0)
	return; /* No messages reported */


/*
 *  Find the last message at the next lower context by search back from
 *  the last message reported.
 * 
 */
    for (i = ErsBlock->current-1; 
	    (i >= 0)&&(ErsBlock->messages[i].context >= ErsBlock->context) ; 
	 --i);

/*
 *  If we are logging - log the annul.
 */
    if (ErsBlock->logRoutine)
    {
        StatusType outstat = STATUS__OK;
        int count = ErsBlock->current - i - 1;
	/* We need one of these for the logRoutine - can't auto init since
		it is not a contast (not accepted by all compilers
        */
        ErsMessageType logMessage;
	logMessage.mesStatus  = STATUS__OK;
	logMessage.context    = ErsBlock->context;
	logMessage.flags      = 0;
        sprintf(logMessage.message, "Annulling %d messages at context %d",
                count, ErsBlock->context);
        ErsBlock->logRoutine(ErsBlock->logArg,
                             &logMessage,
                             &outstat);
    }
/*
 *  Forget the messages by marking the spot just after the last message at 
 *  the next lowest context the new current spot.
 */
    ErsBlock->current = i+1;
}   

/*
 *+			E r s F l u s h

 *  Function name:
      ErsFlush

 *  Function:
	 Flush all error messages at the current context.

 *  Description:
	 All messages at the current context are written to the user.  The
	 context level does not change.

	 Messages are written using the output routine supplied by the
	 user when ErsStart was called. If no routine was supplied, then
	 the messages are written to the standard Error output device using the
	 C run time library.

 *  Language:
      C

 *  Call:
      (Void) = ErsFlush (status)

 *  Parameters:   (">" input, "!" modified, "W" workspace, "<" output)
	(!) status	(StatusType *) Set to zero, unless an output error
			occurs, in which case it is the error code returned
			by the output routine.


 *  Include files: Ers.h

 *  External functions used:
      fprintf	(Crtl) Formated print.
      fputs	(Crtl) Output a string
      logMsg	(VxWorks) Log a message

 *  External values used: 
        stderr  (Crtl) The standard error device.

 *  Prior requirements:
	    none 

 *  Support: Tony Farrell, AAO

 *-

 *  History:
      06-Oct-1992 - TJF - Original version
      06-Mar-1994 - TJF - Handle interrupt and flushing flags
      08-Sep-2003 - TJF - Log the flush.
      29-Mar-2004 - TJF - Can't auto-initialise the logMessage variable since
                          Solaris cc does not accept this (C standard does
                          not actually allow it I beleive)
      19-Apr-2005 - TJF - Check for interrupt context under Vxworks.
 */
extern void ErsFlush(StatusType * status)
{
    int i;
    unsigned count = 0;
  /*  ErsBlockType * t = ErsBlock;*/

    *status = STATUS__OK;

  /*  logMsg("ErsFlush, block add = %x\n",(int)t,0,0,0,0,0);*/

    if (!ErsBlock)
	return ;/* Not active	*/
    else if ((ErsBlock->interrupt)||(intContext()))
    {
#	ifdef VxWorks
	    logMsg("ErsFlush:Attempted flush in interrupt handler (%p, %d, %d)\n",
                   (long)ErsBlock, ErsBlock->interrupt,intContext(),0,0,0);
#	else
	    fprintf(stderr,"ErsFlush:Attempted flush in interrupt handler\n");
#	endif
	*status = ERS__INTHAND;
        return;
    }
    else if (ErsBlock->flushing)
    {
	fprintf(stderr,"ErsFlush:Attempted flush while flushing\n");
	*status = ERS__FLUSHING;
	return;
    }
    else if (ErsBlock->current == 0)
	return; /* No messages reported */

/*
 *  We have to do a flush!
 *
 *  Increment flushing count to ensure we don't modify the message stack 
 */
    ErsBlock->flushing++;
/*
 *  Find the last message at the next lower context by search back from
 *  the last message reported.
 * 
 */
    for (i = ErsBlock->current-1; 
	 (i >= 0) &&(ErsBlock->messages[i].context >= ErsBlock->context) ; 
	 i--);
/*
 *  Work out the number of messages (may be 0) and the begining of the list
 *  for output (i++)
 */
    i++;
    count = ErsBlock->current - i;

/*
 *  If we are logging - log the flush.
 */
    if (ErsBlock->logRoutine)
    {
        StatusType outstat = STATUS__OK;
	/* We need one of these for the logRoutine - can't auto init since
		it is not a contast (not accepted by all compilers
        */
        ErsMessageType logMessage;
	logMessage.mesStatus  = STATUS__OK;
	logMessage.context    = ErsBlock->context;
	logMessage.flags      = 0;
        sprintf(logMessage.message, "Flushing %d messages at context %d",
                count, ErsBlock->context);
        ErsBlock->logRoutine(ErsBlock->logArg,
                             &logMessage,
                             &outstat);
    }

/*
 *  Forget the messages by marking the spot just after the last at the next
 *  lowest context, the new current spot.
 */
    ErsBlock->current = i;
/*
 *  Output the  message using the user supplied routine if possible
 */
    if (ErsBlock->outRoutine)
	ErsBlock->outRoutine(ErsBlock->outArg,
			    count,
			    &ErsBlock->messages[i],
			    status);
    if ((!ErsBlock->outRoutine)||(!StatusOkP(status)))
    {
/*
 *	If no user supplied routine or output error, use fputs to stderr.
 */
	int first = 1;
/*
 *	If we got an error, note it.
 */
	if (!StatusOkP(status))
	    fprintf(stderr,"ErsFlush:Message output error %lx\n",
	        (long unsigned)*status);


	count += i;
	for (; i < (int)count ; ++ i)
	{
	    if (first)
		fputs("!!",stderr);
	    else
		fputs("! ",stderr);
	    fputs(ErsBlock->messages[i].message,stderr);
/*
 *	    If bell or alarm flags set, assume ascii terminal and ring bell.
 */
	    if ((ErsBlock->messages[i].flags & ERS_M_ALARM)||
	        (ErsBlock->messages[i].flags & ERS_M_BELL))
		fputs("\07",stderr);
	    fputs("\n",stderr);
	}
    }

/*
 *  Clear the flushing flag.
 */
    ErsBlock->flushing = 0;
}   




/*
 *+			E r s C l e a r

 *  Function name:
      ErsClear

 *  Function:
	 Flush all error messages at all contexts and reset to base context

 *  Description:
	 All messages are written to the user.  The
	 context level does not change is reset to the base context.

	 Messages are written using the output routine supplied by the
	 user when ErsStart was called. If no routine was supplied, then
	 the messages are written to the standard Error output device using the
	 C run time library.

 *  Language:
      C

 *  Call:
      (Void) = ErsClear (status)

 *  Parameters:   (">" input, "!" modified, "W" workspace, "<" output)
	(!) status	(StatusType *) Set to ERS__NOTACTIVE if ERS is not
			active.  Otherwise, as per ErsFlush.  Unlike ErsFlush,
			this routine does not work if status is bad on entry.

 *  Include files: Ers.h

 *  External functions used:
	ErsFlush	(Ers) Flush error messages.
        fprintf		(Crtl) Write a message.

 *  External values used: 
	stderr	(Crtl) The standard error device.

 *  Prior requirements:
	ErsStart should have been called.

 *  Support: Tony Farrell, AAO

 *-

 *  History:
      06-Oct-1992 - TJF - Original version
      06-Mar-1994 - TJF - Handle interrupt and flushing flags
      19-Apr-2005 - TJF - Check for VxWorks interrupt context.
 */
extern void ErsClear(StatusType * status)
{
    if (!StatusOkP(status)) return;

    if (!ErsBlock)
	*status = ERS__NOTACTIVE; /* Error, not active */
    else if ((ErsBlock->interrupt)||(intContext()))
	*status = ERS__INTHAND;
    else if (ErsBlock->flushing)
	*status = ERS__FLUSHING;
    else
    {
/*
 *	We set the context to 1 and flush the message buffer.  Setting the
 *	context to 1 ensures all messages are flushed.
 */
	ErsBlock->context = 1;
	ErsFlush(status);
/*
 *	Ignore any status problem.
 */
	if (*status != STATUS__OK)
	{
	    fprintf(stderr,"!!Error Clearing Ers Table, status = %lx\n",
		    (long unsigned)*status);
	    *status = STATUS__OK;
	}
    }
}

/*
 *+			E r s G e t T a s k I d

 *  Function name:
       ErsGetTaskId

 *  Function:
       Get ERS task id for use with ErsEnableTask.

 *  Description:    
      Some Systems, such as VxWorks based systems, run all programs in
      a common address space.  In such systems static and global variables
      can be seen by all tasks.  To allow tasks to have private copies of
      static and global variables it is possible to have such variables saved
      and restored during task context switching.  Ers uses this technique
      to store task specific information.   In such systems it is sometimes
      necessary to call Ers routines outside the context of a task (say
      in an interrupt routine), during which the task specific information 
      will be unavailable.  This routine is used in conjunction with
      ErsEnableTask and ErsRestoreTask to make the task specific information 
      available in such places.  

      This routine can be used where the task ID returned by ErsStart(3)
      is not avaiable.  It should be invoked in non-interrupt handler
      code to fetch the task id for passing to ErsEnableTask(3).


 *  Language:
      C

 *  Call:
      (ErsTaskIdType) = ErsGetTaskId (StatusType *status)

 *  Parameters:   (">" input, "!" modified, "W" workspace, "<" output)
              (!) status    (StatusType *)Modified status.

 *  Include files: Ers.h

 *  External functions used:
	None

 *  External values used:
	None

 *  Function Value: A value which can be passed to ErsEnableTask from an
		interrupt handler.   If status is non-zero, zero is returned.

 *  Prior requirements:
	ErsStart should have been called.

 *  Support: Tony Farrell, AAO

 *-

 *  History:
      15-Jun-2001 - TJF - Original version
 */
extern ErsTaskIdType ErsGetTaskId(StatusType * status)
{
    if (*status != STATUS__OK) return 0;
    return ErsBlock;
}


/*
 *+			E r s E n a b l e T a s k 

 *  Function name:
       ErsEnableTask

 *  Function:
       Enable Ers calls within an interrupt handler.

 *  Description:    
      Some Systems, such as VxWorks based systems, run all programs in
      a common address space.  In such systems static and global variables
      can be seen by all tasks.  To allow tasks to have private copies of
      static and global variables it is possible to have such variables saved
      and restored during task context switching.  Ers uses this technique
      to store task specific information.   In such systems it is sometimes
      necessary to call Ers routines outside the context of a task (say
      in an interrupt routine), during which the task specific information 
      will be unavailable.  This routine is used in conjunction with
      ErsStart and ErsRestoreTask to make the task specific information 
      available in such places.  

      This call is normally made at the begining of an interrupt handler.
      The argument should be a value previously returned by a call made to 
      ErsStart() when executing in normal task context.  After this
      call is made, Ers routines can be invoked although all that they do
      is report error using a method appropaite to the interrupt handler.


      The value returned by this function should be supplied to ErsRestoreTask
      after before you exit the interrupt handler to ensure the task that was
      interrupted is restored to its original state.
      
      This call is not neccessary or desirable on systems with process
      specific address spaces (VMS or UNIX).  On such systems it does nothing.

      Note that on VxWorks, if this function is called when not running
      at interrupt context, then taskLock() will be invoked to lock the
      current task as the running task - to avoid corruption issues.



 *  Language:
      C

 *  Call:
      (Void) = ErsEnableTask (TaskId, SavedTaskId)

 *  Parameters:   (">" input, "!" modified, "W" workspace, "<" output)
	(>) TaskId  (ErsTaskIdType)	A value returned by DitsGetTaskID.
	(<) SavedTaskId (ErsTaskIdType *) The value which is to be passed
				to ErsRestoreTask is put here.

 *  Include files: Ers.h

 *  External functions used:
	None

 *  External values used:
	None

 *  Prior requirements:
	ErsStart should have been called.

 *  Support: Tony Farrell, AAO

 *-

 *  History:
      06-Mar-1994 - TJF - Original version
      19-Apr-2005 - TJF - When not in interrupt context on VxWorks, lock
                          the task to avoid another task using the same
                          Ers block running at the same time.
 */
extern void ErsEnableTask(
    ErsTaskIdType TaskId DUNUSED,
    ErsTaskIdType *SavedTaskId)
{
/*
    ErsBlockType * t = ErsBlock;
    logMsg("ErsEnableTask, current = %x, new = %x\n",(int)t,(int)TaskId,0,0,0,0);
*/

#   ifdef VxWorks
        *SavedTaskId = ErsBlock;
        ErsBlock = TaskId;
        ++ErsBlock->interrupt;
        /*
         * If we are not in interrupt context, then lock the task to ensure
         * we don't get switched out an another task using the same block
         * gets switched in (won't corrupt ERS but may mess up the task as
         * it won't be able to use ERS).
         */
        if (intContext() == FALSE)
            taskLock();
#   else
	*SavedTaskId = 0;
#   endif
}

/*
 *+			E r s R e s t o r e T a s k 

 *  Function name:
      ErsRestoreTask

 *  Function:
       Restore the interrupted Task Id 

 *  Description:    
      Some Systems, such as VxWorks based systems, run all programs in
      a common address space.  In such systems static and global variables
      can be seen by all tasks.  To allow tasks to have private copies of
      static and global variables it is possible to have such variables saved
      and restored during task context switching.  Ers uses this technique
      to store tErsGetAtCtxask specific information.   In such systems it is sometimes
      necessary to call Ers routines outside the context of a task (say
      in an interrupt routine), during which the task specific information 
      will be unavailable.  This routine is used in conjunction with
      ErsStart and ErsEnableTask to make the task specific information 
      available in such places.  

      This call is made in interrupt handlers.  The argument should be the 
      value returned by a previous call to ErsEnableTask in the interrupt 
      handler.  After this call is made, Ers routines can no longer be used.

     This call is not neccessary or desirable on systems with process
      specific address spaces (VMS or UNIX).  On such systems it does nothing.


 *  Language:
      C

 *  Call:
      (Void) = ErsRestoreTask (TaskId)

 *  Parameters:   (">" input, "!" modified, "W" workspace, "<" output)
	(>) TaskId  (ErsTaskIdType)	A value returned by ErsEnableTask.

 *  Include files: Ers.h

 *  External functions used:
	None

 *  External values used:
	None

 *  Prior requirements:
	ErsStart should have been called.

 *  Support: Tony Farrell, AAO

 *-

 *  History:
      06-Mar-1994 - TJF - Original version
      19-Apr-2005 - TJF - When not in interrupt context on VxWorks, unlock
                          the task.
 */
extern void ErsRestoreTask(ErsTaskIdType TaskId DUNUSED)
    
{
/*
    ErsBlockType * t = ErsBlock;
    logMsg("ErsEnableTask, current = %x, new = %x\n",(int)t,(int)TaskId,0,0,0,0);
*/
#   ifdef VxWorks
        --ErsBlock->interrupt;
        ErsBlock = TaskId;

        /*
         * If we are not in interrupt context, then unlock the task.
         */
        if (intContext() == FALSE)
            taskUnlock();
#   endif
}




/*
 *+			E r s S e t L o g R o u t i n e

 *  Function name:
      ErsSetLogRoutine

 *  Function:
       Change the Ers log routine.


 *  Description:
       Allows us to change the routine used to log ERS messages.

 *  Language:
      C

 *  Call:
      (void) = ErsSetLogRoutine (logRoutine, logArg, 
                      oldLogRoutine, oldLogArg, status)

 *  Parameters:   (">" input, "!" modified, "W" workspace, "<" output)
	(>) logRoutine (ErsLogRoutineType) If non zero, this routine will
			be called to log any messages reported by ErsRep.
	(>) logArg	(void *) Passed directly to logRoutine as its first
			argument and not examined by Ers.
	(>) oldLogRoutine (ErsLogRoutineType *) The previous log routine
                        is returned here - if non-zero, the new routine should
                        call this routine after it has done its job.
	(>) oldLogArg	(void *) Should be passed as the first argument to
                        the old log routine.
        (!) status	(StatusType *)Modified status.  


 *  Include files: Ers.h

 *  External functions used: none

 *  External values used: 
	    none

 *  Prior requirements:
	    none 

 *  Support: Tony Farrell, AAO

 *-

 *  History:
      30-May-2001 - TJF - Original version
 */
extern void ErsSetLogRoutine(
    ErsLogRoutineType logRoutine,
    void * logArg,
    ErsLogRoutineType *oldLogRoutine,
    void * *oldLogArg,
    StatusType * status)
{
    if (!StatusOkP(status)) return;

    if (!ErsBlock)
	*status = ERS__NOTACTIVE; /* Error, not active */
    else 
    {	
        /*
         * Copy the old values and then use the new one
         */
        if (oldLogRoutine)
            *oldLogRoutine = ErsBlock->logRoutine;
        if (oldLogArg)
            *oldLogArg     = ErsBlock->logArg;
        ErsBlock->logRoutine = logRoutine;
        ErsBlock->logArg = logArg;
    }
}




/*
 *+			E r s G e t A t C t x

 *  Function name:
      ErsGetAtCtx

 *  Function:
         Access all ERS messages reported at the current contact.

 *  Description:

 *  Language:
      C

 *  Call:
      (Void) = ErsCtx (status)

 *  Parameters:   (">" input, "!" modified, "W" workspace, "<" output)
        (<) count       (unsigned int *) Number of message at the current
                            context.
        (<) messageArray (ErsMessageType **) Will be set to an array of
                            ErsMessageType of size at least "count". If
                            *count is set to zero, then this may be set
                            to a null pointer.
	(<) status	(StatusType *) Set to zero.


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
      04-Mar-2014 - TJF - Original version
  
 */
extern void ErsGetAtCtx(
    unsigned *count,
    ErsMessageType **messageArray,
    StatusType * status)
{
    register int i;
    *status = STATUS__OK;

    *count = 0;
    *messageArray = 0;

    if (!ErsBlock)
	return ;/* Not active	*/
    else if ((ErsBlock->interrupt)||(ErsBlock->flushing)||(intContext()))
        return; /* Nothing to do	*/
    else if (ErsBlock->current == 0)
	return; /* No messages reported */


/*
 *  Find the last message at the next lower context by search back from
 *  the last message reported.
 * 
 */
    for (i = ErsBlock->current-1; 
	    (i >= 0)&&(ErsBlock->messages[i].context >= ErsBlock->context) ; 
	 --i, ++(*count) );

    if ((*count) == 0) return;

    *messageArray = &ErsBlock->messages[i+1];
}   
