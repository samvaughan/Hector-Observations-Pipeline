#ifndef STATUSINC
#define STATUSINC
#ifdef __cplusplus
extern "C" {
#endif

/*+                     S T A T U S

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

 * "@(#) $Id: ACMM:aaoConfigure/drama_src/status.h,v 12.17 14-Mar-2016 10:42:34+11 tjf $"

 *  Copyright (c) Anglo-Australian Telescope Board, 1995.
    Not to be used foF - DECC on VAX, we need to look for __VAX.
      13-Aug-1997 - TJF - Support WIN32
      29-Aug-1999 - KS  - Support Power Macintosh.
      20-Sep-2000 - KS  - Support HP-UX.
      20-Aug-2001 - KS  - Corrected use of #error. 
      08-Aug-2002 - KS  - Added MacOSX.
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
/* VAX                          */
#if defined(vax) || defined(__vax__) || defined(__VAX)
typedef long int StatusType;

/*
 * Alpha VMS seems to use 32 bit long, while Alpha osf uses 64bit longs.
 * Both use 32bit int's, so that is what we will use.
 */
#elif defined(__alpha)
typedef int StatusType;

/* Decstation (mips compiler    */
#elif defined(ultrix) && defined(mips)
typedef long int StatusType;

/* Decstation (ansi compiler)   */
#elif defined(__ultrix__) && defined(__mips__)
typedef long int StatusType;

/* Sparc (sun4) (what about their new 64 bit machine) */
#elif defined(sparc) || defined(__sparc__) || defined(__sparc)
typedef long int StatusType;

/* MetroWerks CodeWarrior compiling for a PowerPC - ie a PowerMac */
#elif defined(__MWERKS__) && defined(__POWERPC__)
typedef long int StatusType;

/* Macintosh OS X - there may be a better test */
#elif defined(__macosx__)
typedef long int StatusType;

/* Linux architectures.  Think this is always 32 bits */
#elif defined(__linux__)
typedef int StatusType;

/* Intel 80x86 cpus - not sure if this always applies   */
#elif defined(M_I86)
typedef long int StatusType;

#elif defined(WIN32)
typedef long int StatusType;

#elif defined(MC68020)||defined(MC68010)||defined(MC68030)||defined(MC68040)
typedef long int StatusType;

#elif defined(_HPUX_SOURCE)
typedef int StatusType;

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
#error "Status.h: Machine/Compiler/OS combination not known - status undefined"
#endif
#ifdef __cplusplus
}
#endif

#endif
