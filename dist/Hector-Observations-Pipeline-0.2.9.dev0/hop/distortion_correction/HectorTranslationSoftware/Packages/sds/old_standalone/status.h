#ifndef STATUSINC
#define STATUSINC
/*+			S T A T U S 

 *  Module name:
      STATUS

 *  Function:
      Define StatusType StatusOk macro and STATUS__OK symbol

 *  Description:
      STATUS__OK is the value defines as an ok status.  The StatusOk macro
      will return True if its argument has the value STATUS__OK, false
      otherwise.  StatusOkP is the same is StatusOk except that its argument
      is a pointer to a status value. The StatusType type is a typedef of 
      the appropriate integer to use for status.

      The StatusOk and StatusOkP macros can be used in the following way
       if desired
	if StatusOk(i) 

        VERSION OF THIS FILE KEPT in SDS library for Standalone SDS release
        support.
	    ...
 *  @(#) $Id: ACMM:sds/old_standalone/status.h,v 3.94 09-Dec-2020 17:15:42+11 ks $

 *  Language:
      C

 *  Support: Tony Farrell, AAO

 *-

 *  History:
      23-Jun-1993 - TJF - Original version
      06-Jul-1993 - TJF - Fix bugs
      {@change entry@}
 */
#define STATUS__OK 0
#define StatusOk(_value_) ((_value_) == STATUS__OK)
#define StatusOkP(_value_)  (*(_value_) == STATUS__OK)

/*
 *  The definition of StatusType is the hardest bit.  It should be the
 *  smallest int which can take at least 32 bits, on the current
 *  implementation.
 */

/* VAX				*/
#if defined(vax) || defined(__vax__)
typedef long int StatusType;

/* Decstation (mips compiler	*/
#elif defined(ultrix) && defined(mips) 
typedef long int StatusType;

/* Decstation (ansi compiler)	*/
#elif defined(__ultrix__) && defined(__mips__) 
typedef long int StatusType;

/* Sparc (sun4) (what about their new 64 bit machine) */
#elif defined(sparc) || defined(__sparc__)  
typedef long int StatusType;

/* Intel 80x86 cpus - not sure if this always applies	*/
#elif defined(M_I86)
typedef long int StatusType;

#elif defined(MC68020)||defined(MC68010)||defined(MC68030)||defined(MC68040)
typedef long int StatusType;

#elif defined(__alpha)
typedef int StatusType;

#elif defined(CPU)
#if CPU==MC68020 || CPU==MC68010 || CPU==MC68030 || CPU==MC68040 
typedef long int StatusType;


#endif


#endif
/*
 *  If StatusType is still not defined, will have to work out the appropriate
 *  symbols for a particular machine.
 */
#endif
