#ifndef STATUSINC
#define STATUSINC
#ifdef __cplusplus
extern "C" {
#endif

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

      StatusType should correspond to the integer size used by the fortran
	compiler on this machine - or the smallest integer at least 32
	bits in size.

      The StatusOk and StatusOkP macros can be used in the following way
       if desired
	if StatusOk(i) 
	    ...
    

 *  Language:
      C

 *  Support: Tony Farrell, AAO

 *-

 *  Copyright (c) Anglo-Australian Telescope Board, 1995.
    Not to be used for commercial purposes without AATB permission.
  
 *     @(#) $Id: ACMM:sds/Standalone/status.h,v 3.94 09-Dec-2020 17:15:27+11 ks $

 *  History:
      23-Jun-1993 - TJF - Original version
      06-Jul-1993 - TJF - Fix bugs
      11-Apr-1994 - TJF - Support Alpha
      17-Aug-1994 - TJF - Remove the # from #error.  This bombs out with
			sun's CC even though that line should not be executed.
      26-Oct-1994 - TJF - support alpha_osf1
      05-Aug-1996 - TJF - Should use int for any alpha machine.
      18-Feb-1997 - TJF - DECC on VAX, we need to look for __VAX.
      13-Aug-1997 - TJF - Support WIN32
      03-May-1998 - TJF - StatusType under solaris changed from long int to
                          int to ensure it is correct on a 64 bit solaris 
			  machine.
      05-Jun-2000 - TJF - Support HP RISC machines.
      14-Mar-2003 - TJF - Support MacOsX using __APPLE__ tag.
      22-May-2006 - TJF - Move mac defintion to before intel tags are used,
	                   to avoid it becoming a long int on a intel mac.
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
#if defined(vax) || defined(__vax__) || defined(__VAX)
typedef long int StatusType;

/*
 * Alpha VMS seems to use 32 bit long, while Alpha osf uses 64bit longs.
 * Both use 32bit int's, so that is what we will use.
 */
#elif defined(__alpha)
typedef int StatusType;

/* Decstation (mips compiler	*/
#elif defined(ultrix) && defined(mips) 
typedef long int StatusType;

/* Decstation (ansi compiler)	*/
#elif defined(__ultrix__) && defined(__mips__) 
typedef long int StatusType;

/* Sparc (sun4) (int is 32 bits on both 32 bit and 64 bit sparcs) */
#elif defined(sparc) || defined(__sparc__) || defined(__sparc)
typedef int StatusType;

/* Linux architectures.  Think this is always 32 bits */
#elif defined(__linux__) 
typedef int StatusType;

#elif defined(__APPLE__)
typedef int StatusType;

/* Intel 80x86 cpus - not sure if this always applies	*/
#elif defined(M_I86) 
typedef long int StatusType;

#elif defined(WIN32)
typedef long int StatusType;

/* HP RISC Machine */
#elif defined(_PA_RISC1_1)
typedef long int StatusType;

#elif defined(MC68020)||defined(MC68010)||defined(MC68030)||defined(MC68040)
typedef long int StatusType;

/* Have to comment this out when compiling under WIN32 */
#elif defined(CPU)
#if CPU==MC68020 || CPU==MC68010 || CPU==MC68030 || CPU==MC68040 
typedef long int StatusType;


#endif


/*
 *  If StatusType is still not defined, will have to work out the appropriate
 *  symbols for a particular machine.
 */
#else
error "Status.h: Machine/Compiler/OS combination not known - status not defined"
#endif
#ifdef __cplusplus
}
#endif

#endif
