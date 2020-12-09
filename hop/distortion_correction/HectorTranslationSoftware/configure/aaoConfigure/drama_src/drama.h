#ifndef DRAMAINC
#define DRAMAINC
#ifdef __cplusplus
extern "C" {
#endif

/*			D R A M A . H

 *  Module name:
     drama.h

 *  Function:
	Configuration dependent macros defined here.

 *  Description:
	Defines the following types in an appropriate maner for the target

		DVOID		void in compilers which support it, 
				int otherwise.
		DVOIDP		void * in compliers which support it,
				char * otherwise.
		DVOID_I		Use when fudging prototypes containing void.
				Sometimes the compilers supports void but the
				include files where set up for compilers which
				don't.  The _I version helps get arround this.
		DVOIDP_I	Use when fudging prototypes contianing void_i.
		DVOIDF		Return type for void functions.
				(obsolete, should replace with DVOID or 
                                 DVOID_I)


		DCONSTV		Make Value of routine arguments const
		DCONSTR		Make Value referened to by argument const
					e.g.
						CONSTV int CONSTR * number;

		DCONSTD		Definition of a Const value
		DCONST_I	A const for use in fudged prototypes.  Since the
				other DCONST values are not defined if DNOCONST
				is defined to be 1, this is usefull in 
				prototypes where it is necessary anyway.
				

		DINLINE		Function declaration if inlines is to be requested.
		DPUBLIC		For public functions
		DPRIVATE	For function private to a module.
		DINTERNAL	For functions public within a system but for the
					internal use of that system.

		DPUBLICVAR      For public variables.
	        DPUBLICCLASS    For public C++ classes.
	        DSTATICMEM      For public C++ static member functions.


	Defines the following macros to indicate

		DPROTOTYPES_OK	C Prototypes should be used
		DSTDARG_OK	Do we have the stdarg variable argument list 
                                stuff. If this is not defined, assume varargs 
				is used.
		DFUDGE_PROTOTYPES_UNIX 	Defined if the run time library include
				files do not define prototypes for unix system 
				calls but the compiler supports prototypes.  
				The setting of this may depend on if the Gnu C 
				fix includes script has been run.
		DFUDGE_PROTOTYPES_C  	Defined if the run time library include
				 files do not define prototypes for C run time 
				library calls but the compiler supports 
				prototypes.    The setting of this may depend 
				on if the Gnu C fix includes script has been run
		DUNIX		Defined to indicate we are compiling for unix
		DPOSIX_1	Defined to indicate we are compiling on a system
				with posix part one compatible include files.
		DPOSIX_4	Defined to indicate we are compiling on a system
				with posix part four complatible include files.

		DFLOAT_OK	Does the system include floating point support.
		DUNUSED         Used to mark a variable as delibrately
				unused.  Only has effect for gcc and is designed
				to make the gcc -Wunused warning not be
				generated for the variable in question.

		
	This header is designed to help avoid the use of compiler generated macros.
	The following compiler macros may actually be needed

		VMS  - to indicate VMS include files and VMS specific functions
		VAXC - to indicate we can use vaxc builtins.AA
		VAX  - to indicate a VAX.
		ALPHA - to indicate an ALPHA
		DECC - to indicate Dec C.
		VxWorks - to indicate VxWorks include files and specific functions.
		

 *  Language:
      C

 *  Support: Tony Farrell, AAO

 *

 * "@(#) $Id: ACMM:aaoConfigure/drama_src/drama.h,v 12.17 14-Mar-2016 10:42:34+11 tjf $"

 *  History:
      28-Sep-1993 - TJF - Original version
      26-Aug-1994 - TJF - Support solaris2.
      26-OCt-1994 - TJF - Support alpha/osf1.
      06-Apr-1995 - TJF - Add Version numbers
      07-Aug-1996 - TJF - Tidy up POSIX stuff and support of alpha/vms/decc
      09-Aug-1996 - TJF - Support C++ better.
      30-Jan-1997 - TJF - VxWorks 5.2, 5.3 supported as posix machines.
      11-Feb-1997 - TJF - DRAMA version now 1.0
      21-Feb-1997 - TJF - DECC is posix compatible.
      13-May-1997 - TJf - Add support for C++ portability problems.
      15-Aug-1997 - TJf - Support WIN32.
      02-Aug-1997 - TJF - Goto version 1.1
      06-Nov-1997 - TJF - Goto version 1.2. Linux support.
      09-Dec-1997 - TJF - Add DPUBLICVAR and DPUBLICCLASS items to support
			  DRAMA DLL under WIN32.
      02-Feb-1998 - TJF - Add DSTATICMEM to support DRAMA DLL under WIN32
      25-Mar-1998 - TJF - Add DUNUSED.
      11-May-1998 - TJF - DUNUSED is not always supported by GNU.  Add the
			  support to avoid it.
      28-May-1998 - TJF - Fix some commenting errors.

 * Sccs Id:     drama.h, Release 1.42, 06/24/98
  
 * Copyright (c) Anglo-Australian Telescope Board, 1995.
   Not to be used for commercial purposes without AATB permission.
   
 */
/*  
 *  Version numbers.
 */
#define DRAMA_VERSION "1.2"
#define DRAMA_MAJOR_VERSION 1
#define DRAMA_MINOR_VERSION 2

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * 
 *  We currently know about VAX (VMS), mips (ultrix), sparc (sunos 4.1) and
 *  sparc (sunos 5.3/solaris 2)  alpha/osf1.  
 *   We know about Gnu CC under mips (ultrix) 
 *  sparc (sunos) and VxWorks and the sun ansi complier under solaris 2.
 */
#if defined VxWorks		/* For basic VxWorks types */
#    include "vxWorks.h"
#endif

/*
 *  Get around problems in Sparc include files, they are not ANSI compatible
 */
#if defined(__sparc__) && !defined(sparc)
#define sparc 1
#endif
/*
 * Ensure we always have the macro's we use.  If Strict Ansi is enabled,
 *  then we only have the underscore versions but when on older complers,
 *  we don't have them at all.
 */
#if defined(__VMS) && !defined(VMS)
#define VMS 1
#endif

#if defined(__VAXC) && !defined(VAXC)
#define  VAXC 1
#endif

#if defined(__VAX) && !defined(VAX)
#define  VAX 1
#endif

#if defined(__ALPHA) && !defined(ALPHA)
#define  ALPHA 1
#ifndef __alpha
#define __alpha 1
#endif
#endif

#if defined(__DECC) && !defined(DECC)
#define  DECC 1
#endif

#if defined(__alpha) && !defined(ALPHA)
#ifndef ALPHA
#define ALPHA 1
#endif
#endif

/*
 *  DRAMA uses M_I86 to identify Intel x86 series.  Ensure it is
 *  defined if required
 */
#if defined(__i386__)
#ifndef M_I86
#define M_I86
#endif
#endif


/*
 *  Define this to pick up unix specfic include files and enable the use of
 *  unix specific functions.  Use VMS and VxWorks macros for VMS and VxWorks 
 *  specific stuff.
 */
#if defined(unix) || defined(__unix__)
#   define DUNIX
#endif

/*
 *
 *  Do we use prototypes.  We can use prototypes on everything with standard 
 *  ansi  C (__STDC__ defined).  We can also use them on VAXC, DECC and ultrix 
 *  and WIN32 (where __STDC__ won't be defined if compling borland)
 */
#if defined (VAXC)  || defined (DECC) || defined(__STDC__) || defined(ultrix)   || defined(__cplusplus) || defined(WIN32)
#   define DPROTOTYPES_OK
#endif


/*
 *  void - if we have prototypes, we should have void function returns.
 */
#if defined(DPROTOTYPES_OK)
#    define DVOIDF void
#    define DVOID void
#    define DVOIDP void *
#else
#    define DVOIDF int
#    define DVOID int
#    define DVOIDP char *
#endif

/*
 *   The caller can set this to stop the various const stuff being defined.  
 *   This may be necessary if using external routines which are not setup correctly.
 */
#ifndef DNOCONST
#    define DNOCONST 0
#endif

/*
 *    const - this is not supported on all compilers and some don't support it
 *    everywhere.
 */
#if DNOCONST
#   define DCONSTV /* */
#   define DCONSTR /* */
#   define DCONSTD /* */
/*
 *  STDC  does it all right, as should DEC C and any C++  compiler.
 *  and WIN32 (where __STDC__ won't be defined if compling borland)
 */
#elif defined(__STDC__) || defined(DECC) || defined(__cplusplus) || defined(WIN32)
#   define DCONSTV const
#   define DCONSTR const
#   define DCONSTD const
/*
 *  VAXC - does not support references to const items on argument lists.
 */
#elif defined (VAXC)
#   define DCONSTV const
#   define DCONSTR /* */
#   define DCONSTD const
/*
 *  The rest, assume the worst until we know otherwise.  
 */
#else
#   define DCONSTV /* */
#   define DCONSTR /* */
#   define DCONSTD /* */
#endif

/*
 *   Posix, i
 *      VxWorks 5.1/5.2/5.3 is the only system we currently know that is fully
 *	posix part 4 (the real time part) compiliant.  Solaris 2 and Dec 
 *	alpha machines(both osf and VMS) should be Posix 1 compiliant.
 */
#ifndef DPOSIX_4
#if defined(VxWorks_5_1) || defined(VxWorks_5_2) || defined(VxWorks_5_3)
#   define DPOSIX_4 1
#endif
#endif

#ifndef DPOSIX_1
#if defined(DPOSIX_4) || defined(solaris2) || defined(__alpha) || defined(DECC) || defined(WIN32)
#   define DPOSIX_1 1
#endif
#endif


/*
 *  If we are compiling with prototypes, but we don't have prototypes for
 *  run time functions, define this so they can be provided by dits.
 *  (The _UNIX version is for unix system functions, the _C version for 
 *   C run time library functions).  The _I versions of DVOID and DVOIDP
 *  are required on systems where the include files are for compilers which
 *  do not accept void, while the compiler does accept it.  They are only needed
 *  in fudged prototypes.
 */
#if defined DPROTOTYPES_OK && !defined(DPOSIX_1)
#   if defined(DUNIX) 
#        define DFUDGE_PROTOTYPES_UNIX
#        if defined(sun) || defined(__sun__)
#            define DFUDGE_PROTOTYPES_C
#        endif
#   endif

/*
 *  These for cases where you are using Gcc but fix_includes has
 *  not been run
 */
#ifdef DNOFIXED_INCLUDES
#   define DVOID_I int
#   define DVOIDP_I char *
#   define DCONST_I const
#else
/*
 *  Othewise, these should work.
 */
#   define DVOID_I void
#   define DVOIDP_I void *
#   define DCONST_I const
#endif

#else
#   define DVOID_I void
#   define DVOIDP_I void *
#   define DCONST_I const
#endif



/*
 *  Do we have the stdarg.h include file.  This is POSIX_1 but VMS and 
 *  VxWorks 5.0 also do.
 */

#ifdef DPROTOTYPES_OK
#   if defined(DPOSIX_1) || defined (VMS) || defined (VxWorks)
#        define DSTDARG_OK
#   endif
#endif


/*
 *  Is the inline keyword available ?
 *  Similary for the gcc unused attribute, but this is not always supported,
 *  base suppot on GNUC_MINOR macro.
 */
#if defined(__GNUC__) || defined(GNUC)
#   define DINLINE __inline__ static
#   define DUNUSED /* Default definition */
#   ifdef __GNUC_MINOR__
#   if __GNUC_MINOR__ >= 7
#       undef  DUNUSED
#       define DUNUSED __attribute__ ((unused))
#   endif
#   endif
#else
#   define DINLINE static
#   define DUNUSED /* */
#endif

/*
 *  Used to declare routines, variables and classes which are to be
 *  exported from DRAMA to user routines.   Particulary significant for
 *  WIN32, where we must export items from the DRAMA DLL.
 */
#ifdef WIN32
#ifdef DRAMA_DLL  /* Am building DRAMA DLL */
#define DPUBLIC    extern __declspec(dllexport)
#define DPUBLICVAR extern __declspec(dllexport)
#define DPUBLICCLASS class __declspec(dllexport) 
#define DSTATICMEM   static __declspec(dllexport) 
#elif defined(DRAMA_STATIC) /* Building a static library */
#define DPUBLIC    extern
#define DPUBLICVAR extern
#define DPUBLICCLASS  class
#define DSTATICMEM    static
#else		/* Am building application with DLL */
#define DPUBLIC    extern __declspec(dllimport)
#define DPUBLICVAR extern __declspec(dllimport)
#define DPUBLICCLASS class __declspec(dllimport) 
#define DSTATICMEM   static __declspec(dllimport) 
#endif

#else
#define DPUBLIC    extern
#define DPUBLICVAR extern
#define DPUBLICCLASS  class
#define DSTATICMEM    static
#endif

#define DINTERNAL  extern
#define DPRIVATE   static

/*
 *  Float point support.  All systems so far have it.
 */
#define DFLOAT_OK

#ifdef __cplusplus
}

/*
 *  C++ stuff.
 *  
 *  CPP_NOBOOL instead we don't have support for bool.  We must emulate it.
 */
#ifdef CPP_NOBOOL
typedef int bool;
#define false 0
#define true 1
#endif

/*
 *  If NRCONST_BUG is defined, then non-reference parameters which are
 *  defined to be a const must be consistant in both the prototype and
 *  definition.  We use D_NRCONST in the definition - the include file
 *  does not change.
 */
#ifdef NRCONST_BUG
#define D_NRCONST
#else
#define D_NRCONST const
#endif

#endif


#endif
