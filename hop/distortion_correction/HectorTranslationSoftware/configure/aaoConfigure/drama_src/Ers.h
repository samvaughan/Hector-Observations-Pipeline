/*
 *                   E r s . h     ( standalone version )
 *
 *  Ers is the error reporting system used by the AAO's DRAMA data 
 *  acquisition system. This means that it is used by almost all of the
 *  various sub-systems used by DRAMA, such as the IMP message system
 *  and the SDS structured data system.
 *
 *  In some cases, these DRAMA sub-systems are used in a standalone
 *  mode. When this happens, the users of the standalone sub-system do
 *  not need the whole DRAMA infrastructure, but do need a version of
 *  Ers that is independent of the rest of DRAMA. That is what this
 *  file and the ers_imp.c file provide. 
 *
 * "@(#) $Id: ACMM:aaoConfigure/drama_src/Ers.h,v 12.17 14-Mar-2016 10:42:34+11 tjf $"
 *
 *  Modified:
 *     19th Oct 1998. Changed type of Status arg in ErsRep and ErsOut
 *                    definitions. KS/AAO.
 *      1st Sep 1999. Added constant definitions. These aren't strictly
 *                    needed by IMP, but this file is occasionally used for
 *                    other sub-systems that need a standalone Ers, and some
 *                    of these need these definitions. KS/AAO.
 *     10th Sep 2000. Added temporary definition of IMPZ_INT4 for Linux. 
 *                    Needs a more general definition. KS/AAO.
 *     21st Sep 2000. Made the IMPZ_INT4 definition a little more general,
 *                    KS/AAO.
 *     16th Jul 2001. Added use of ERSINC and __IMP__. KS/AAO.
 *     16th Aug 2001. Slight extension of header comments. KS/AAO.
 *     17th Aug 2001. Added minimal ErsVSprintf() declaration. KS/AAO.
 *     19th Aug 2001. Added const to declaration of ErsRep(), ErsOut(). KS/AAO.
 *      8th Aug 2002. Added Mac OS X. KS/AAO.
 *     19th Aug 2003. Added DFLOAT_OK definition. CR/AAO
 *     19th Aug 2015. Add ErsRepNF.  TJF/AAO.
 */
 
#ifndef ERSINC
#define ERSINC

#ifdef VMS
#include <varargs.h>
#else
#include <stdarg.h>
#ifndef macintosh
#include <sys/types.h>
#endif
#endif

/*  A few definitions from DRAMA code, to allow the standard DRAMA
 *  versions of ersvsprintf and erssprintf to compile using this version of
 *  Ers.h.
 */

#ifndef DRAMAINC
#define DPUBLIC
#ifndef VMS
#define DSTDARG_OK
#endif
#define DCONSTV
#define DCONSTR
#define DPRIVATE static
#define DVOIDP void*
#define DVOID void
#endif

/* Following are used in ersvsprintf.c */
#define DFLOAT_OK
#define ERS_MAXEXP 308
#define ERS_MAXFRACT 39

void ErsStart();
void ErsStop();
void ErsPush();
void ErsAnnul();
void ErsFlush();
void ErsClear();
void ErsPop();

/*  IMPZ_INT4 is normally defined by the IMP system-dependent include 
 *  files. It is supposed to be a 32-bit signed integer quantity. The 
 *  following complex set of tests is a copy of those used by DRAMA to 
 *  define StatusType, which IMPZ_INT4 should match. This section is only
 *  needed for the case where this file is being used by a separate,
 *  non-IMP, system.  If we are compiling IMP code, we bypass this.
 */

#ifndef __IMP__
 
/* VAX */
#if defined(vax) || defined(__vax__) || defined(__VAX)
typedef long int IMPZ_INT4;

/*
 * Alpha VMS seems to use 32 bit long, while Alpha OSF uses 64bit longs.
 * Both use 32bit int's, so that is what we will use.
 */
#elif defined(__alpha)
typedef int IMPZ_INT4;

/* Decstation (mips compiler)    */
#elif defined(ultrix) && defined(mips)
typedef long int IMPZ_INT4;

/* Decstation (ansi compiler)   */
#elif defined(__ultrix__) && defined(__mips__)
typedef long int IMPZ_INT4;

/* Sparc (sun4) (what about their new 64 bit machine) */
#elif defined(sparc) || defined(__sparc__) || defined(__sparc)
typedef long int IMPZ_INT4;

/* MetroWerks CodeWarrior compiling for a PowerPC - ie a PowerMac */
#elif defined(__MWERKS__) && defined(__POWERPC__)
typedef long int IMPZ_INT4;

/* Macintosh OS X - there may be a better test */
#elif defined(__macosx__)
typedef long int IMPZ_INT4;

/* Linux architectures.  Think this is always 32 bits */
#elif defined(__linux__)
typedef int IMPZ_INT4;

/* Intel 80x86 cpus - not sure if this always applies   */
#elif defined(M_I86)
typedef long int IMPZ_INT4;

#elif defined(WIN32)
typedef long int IMPZ_INT4;

#elif defined(MC68020)||defined(MC68010)||defined(MC68030)||defined(MC68040)
typedef long int IMPZ_INT4;

#elif defined(_HPUX_SOURCE)
typedef int IMPZ_INT4;

/* Have to comment this out when compiling under WIN32 */
#elif defined(CPU)
#if CPU==MC68020 || CPU==MC68010 || CPU==MC68030 || CPU==MC68040
typedef long int IMPZ_INT4;
#endif

/*
 *  Fallback on a long int, if we don't know the system.
 */
#else
typedef long int IMPZ_INT4;
#endif

#endif

/*  And finally, our ErsRep and ErsOut definitions */

/*  gcc allows us to define the printf-like routines ErsRep() and ErsOut()
 *  elegantly, and we take advantage of this.
 */
 
#ifdef __GNUC__

void ErsRep (int Flags, IMPZ_INT4 *Status, const char *String, ...)
                           __attribute__ ((format (printf, 3, 4)));
void ErsOut (int Flags, IMPZ_INT4 *Status, const char *String, ...)
                           __attribute__ ((format (printf, 3, 4)));
void ErsRepNF (int Flags, IMPZ_INT4 *Status, const char *String);
#else

#ifdef VMS

void ErsRep();              /* VMS version */
void ErsRepNF();              /* VMS version */
void ErsOut();              /* VMS version */

#else

void ErsRep(int flags, IMPZ_INT4* status, char* fmt, ...);
void ErsRepNF(int flags, IMPZ_INT4* status, char* string);
void ErsOut(int flags, IMPZ_INT4* status, char* fmt, ...); 

#endif

#endif

#ifdef DSTDARG_OK
    DPUBLIC int ErsSPrintf (DCONSTV int maxLength, char *string, 
                                     DCONSTV char * DCONSTR fmt,...);
#else
    DPUBLIC int ErsSPrintf ();
#endif

int ErsVSPrintf();

/*
 *  Constants
 */

#define ERS_C_LEN 200           /* Maximum length of reported messages */
#define ERS_C_MAXMSG 30         /* Maximum number of reported messages */

#define ERS_M_NOFMT (1<<0)      /* Message flag masks           */
#define ERS_M_HIGHLIGHT (1<<1)
#define ERS_M_BELL (1<<2)
#define ERS_M_ALARM (1<<3)
#define ERS_M_BROADCAST (1<<4)

/*  The following is needed to allow ImpErsOut to be defined in the 
 *  imp.h file. It is lifted more or less verbatim from the Ers code.
 */

typedef struct {
   long      mesStatus;            /* Status of message    */
   unsigned int context;           /* Context message was written at */
   int    flags;                   /* Message flags        */
   char   message[ERS_C_LEN];      /* The formated message */
} ErsMessageType;

typedef DVOID (*ErsOutRoutineType)(
                    DVOIDP outArg,                    /* Supplied to ErsStart */
                    unsigned int count,               /* Number of messages */
                    DCONSTV ErsMessageType messages[],/* Array of messages */
                    IMPZ_INT4* status);

#endif
                    
/*  End of standalone Ers.h file */
