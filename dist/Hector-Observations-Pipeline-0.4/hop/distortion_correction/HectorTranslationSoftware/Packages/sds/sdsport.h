/*  SDS Portability Include file                              */
/*                                                           */
/*  Extracted from sds.h on 25/Jun/04 to cleanup that file  */
/*                                                           */
/*  @(#) $Id: ACMM:sds/sdsport.h,v 3.94 09-Dec-2020 17:15:24+11 ks $ */
/*                                                           */
/*                                                           */
/*                                                           */
#ifndef _SDS_PORT_INCLUDE_
#define _SDS_PORT_INCLUDE_ 1

/*
 * When compiling under the ESO environment, MAKE_VXWORKS is defined
 * but VxWorks is not.  Correct things.
 */
#if defined(MAKE_VXWORKS) && (!(defined(VxWorks)))
#define VxWorks 1
#endif

#ifdef VxWorks
#include "vxWorks.h"

/*
 * It appears this macro is defined when we are using VxWorks 5.5 (and hopefully, higher)
 * It is not defined for VxWorks 5.4 or 5.2 (don't know about 5.3, but suspect not).
 * In VxWorks 5.5 INT64 is already defined, so we set the INT64_ALREADY_DEFINED
 * macro to trigger this.
 */
#ifdef _WRS_VXWORKS_5_X
#define INT64_ALREADY_DEFINED 1
#endif


#endif

/*
 * For Solaris we need this to get the extra bits - such as INT64_MAX
 */
#ifndef __EXTENSIONS__
#define __EXTENSIONS__ 1
#endif

#include <limits.h>  /* To look for UINT64_MAX */

/*
 *   Is const acceptable on this machine.
 */
#if defined(__cplusplus) || defined(__STDC__) || defined (VAXC)
#     define SDSCONST const
#else
#     define SDSCONST  /* */
#endif


/*  Machine architecture information  
 *
 *  See also definitions in sds.c for formats of each indivdual data type  *
 */

/* 
 * Are integers big endian or little endian  
 */
#ifdef VAX
#define   SDS_BIGEND 0
#elif defined MIPSEL
#define   SDS_BIGEND 0

#elif defined(M_I86) || defined(__i386__)
#ifndef M_I86
#define M_I86 1
#endif
#define   SDS_BIGEND 0

#elif defined(__x86_64)
#define    SDS_BIGEND 0

#elif defined __alpha
#define   SDS_BIGEND 0
#else
#define   SDS_BIGEND 1
#endif
/*
 * Under Windows, we may need to export or import the functions, depending
 * on if we are building the DLL or linking against it.
 */
#ifdef WIN32
#ifdef DRAMA_DLL  /* Am building DRAMA DLL */
#define SDSEXTERN extern __declspec(dllexport)
#define SDSCLASS  __declspec(dllexport) class
#elif defined(DRAMA_STATIC) /* Building a static library */
#define SDSEXTERN extern
#define SDSCLASS  class
#else           /* Am building application with DLL */
#define SDSEXTERN extern __declspec(dllimport)
#define SDSCLASS  __declspec(dllimport) class
#endif

#else
#define SDSEXTERN extern
#define SDSCLASS  class
#endif



/*
 *  32 bit integer types - On most machines  int  is a 32 bit
 *  integer. However on some compilers long int is required to give
 *  a 32 bit integer. We can't use long int in all cases because this
 *  will sometimes give 64 bits (e.g. on Alpha). Under VxWorks, these values
 *  are already defined
  */


#ifdef macintosh
   typedef unsigned long int UINT32;
   typedef long int INT32;
#elif !defined(VxWorks)
   typedef unsigned int UINT32;
   typedef int INT32;
#endif


/* 
 * We have two define's to indicate support for 64 bit integers. If
 * the compiler supports a 64 bit integer natively NATIVE__INT64 is
 * defined and we expect INT64 and UINT64 to be defined to be the 64
 * bit integer types. If the compiler additionally uses the 64 bit
 * integer as it's default 'long int' type, then we also define 
 * LONG__64 so we can use %ld in printf statements, and the Arg* 
 * integer routines use 64 bit longs 
 *
 * SDS__ULONG_DIVSOR is a constant value used by SdsGetI64() etc. and
 * should represent the value 1^32 in a form acceptable to the compiler.
 * It is only needed if NATIVE__INT64 is defined.  SDS__ULONG_DIVSOR_D is similar, but
 * is a floating point and is needed all the time.
 */

#define SDS__ULONG_DIVSOR_D 4294967296.0

/*
 * On alpha machines - long is a 64 bit integer, except if the
 * __32BITS macro is defined and has the value 1, in which case it is 32 bits.
 */

#ifdef __alpha

#    define LONG__64 1
#    define SDS__ULONG_DIVSOR 4294967296ul

#    ifdef __32BITS
#        if __32BITS
#            undef LONG__64
#            undef SDS__ULONG_DIVSOR
#        endif
#    endif
#endif

/*
 * Try to work out if we are running with a version of GNU C that supports
 * long long.  This is the case for GNUC 2.95 (atleast) onwards.  
 *
 * For testing purposes, if the macro SDS_GNUC_NO_LONG_LONG is defined
 * at this point, then we don't set SDS_GNUC_WITH_LONG_LONG - we default
 * to use other approaches.
 */
#ifdef __GNUC__ 
# if __GNUC_MINOR__ >= 95 || __GNUC__ > 2
#    ifndef SDS_GNUC_NO_LONG_LONG
#        define SDS_GNUC_WITH_LONG_LONG 1
#    endif
# endif
#endif

/*
 * For Microsoft C, if we include basetsd.h, we get INT64 and UINT64.
 */
#ifdef _MSC_VER
#include <basetsd.h>   
#define NATIVE__INT64 1
#define SDS__ULONG_DIVSOR 4294967296ull  /* Have not confirmed this format*/
#define SDS__64_BECAUSE "am using Microsoft C, which defined INT64 and UINT64"

/*
 * GNU Seems to set __LP64__ if longs and pointers are 64 bits.  Some other systems
 * use _LP64
 */
#elif defined(__LP64__)
#   define NATIVE__INT64 1
#   define LONG__64 1
#   define SDS__ULONG_DIVSOR 4294967296ul
    typedef long int INT64;
    typedef  unsigned long UINT64;
#   define SDS__64_BECAUSE "__LP64__ macro is defined."

#elif defined(_LP64)
#   define NATIVE__INT64 1
#   define LONG__64 1
#   define SDS__ULONG_DIVSOR 4294967296ul
    typedef long int INT64;
    typedef  unsigned long UINT64;
#   define SDS__64_BECAUSE "_LP64 macro is defined."


#elif defined(SDS_GNUC_WITH_LONG_LONG)
/*
 * We are using GNU C with support for "long long".
 */
#   define NATIVE__INT64 1
#   define SDS__ULONG_DIVSOR 4294967296ull
/*
 * We need to define the INT64 and UINT64 types.  We don't do this
 * if the macro INT64_ALREADY_DEFINED is defined.  
 *
 * The __extension__ prefix tells GCC not to warn about the use of
 * a non-standard item (long long is not in C89).
 *
 * Typically, this macro is defined for Vxworks 5.5 on onwards by putting 
 * the definition "#define VxWorksDefinesINT64" in the drama_local.cf.
 */
#   ifndef INT64_ALREADY_DEFINED
        __extension__ typedef  long long INT64;
        __extension__ typedef  unsigned long long UINT64;
#       define SDS__64_BECAUSE "_GCC with long long available."
#   else
#       define SDS__64_BECAUSE "_GCC with long long available, types defined by VxWorks"
#   endif

/* 
 * In some other cases (Solaris cc, probably later versions of the C standard)
 * UINT64_MAX should be defined if "unsigned long long" is ok 
 */
#elif defined(UINT64_MAX)

#   define NATIVE__INT64 1
#   define SDS__ULONG_DIVSOR 4294967296ull /*Have not confirmed this format */

    typedef  long long INT64;
    typedef  unsigned long long UINT64;

#   define SDS__64_BECAUSE "UINT64_MAX macro is defined"

#elif defined (LONG__64)
/*
 * If we get here, then INT64 and UINT64 are not yet defined, but if
 * LONG__64 is defined, then "long" is 64 bits.
 */
#   define NATIVE__INT64 1
    typedef long int INT64;
    typedef  unsigned long UINT64;
#   define SDS__ULONG_DIVSOR 4294967296ul

#   define SDS__64_BECAUSE "LONG__64 macro previously defined"

#elif defined (__alpha)
/* 
 *  Only get here for DEC Alpha compiler in 32 bit mode. In this
 *  case we can use "long long" to get 32 bit ints.
 */
#   define NATIVE__INT64 1
#   define SDS__ULONG_DIVSOR 4294967296ull /*Have not confirmed this format */

    typedef  long long INT64;
    typedef  unsigned long long UINT64;

#   define SDS__64_BECAUSE "32 bit mode under alpha - 64 bit is long-long"
   
#else
/*
 * There is no native 64 bit integer.  Create a type just to hold
 * these - but we won't be able to use it in many operations.
 */
    typedef struct INT64
    {
        /** Most significant 32 bits of a 64 bit integer */
        UINT32 i1;
        /** Least significant 32 bits of a 64 bit integer */
        UINT32 i2;
    } INT64;

    typedef struct UINT64
    {
        /** Most significant 32 bits of a 64 bit unsigned integer */
        UINT32 i1;
        /** Least significant 32 bits of a 64 bit integer */
        UINT32 i2;
    } UINT64;
#endif 




/*
 * Define the SDSUNUSED and SDSDEPRECATED macros.  These refer to
 * GCC features (also available in CLANG).  We want to use them
 * if we can.  Even in GCC, they are not always available (Note, AAO
 * still has GCC 2.9x for VxWorks compilations, in 2016).
 *
 * This is based on DRAMA's drama.h file, DramaConfig sub-system
 * version 5.13.
 * 
 */ 
#if defined(__GNUC__) || defined(GNUC)
#   define SDSUNUSED             /* Default definition */
#   define SDSDEPRECATED         /* Default definition */

    /* If we don't hve __GNUC_MINOR__ we have a very old version */
#   ifdef __GNUC_MINOR__

    /* SDSUNUSED only works for versions after 2.7.   */
#   if __GNUC_MINOR__ >= 7 || __GNUC__ > 2
#       undef  SDSUNUSED
#       define SDSUNUSED __attribute__ ((unused))
#   endif


    /* SDSDEPRECATED only works at version 3.1 */
#   if  __GNUC__ > 3 || ((__GNUC__ == 3) && (__GNUC_MINOR__ >= 1))
#       undef  SDSDEPRECATED
#       define SDSDEPRECATED __attribute__ ((deprecated))
#   endif


#   endif /* defined  __GNUC_MINOR__ */
#else
#   define SDSUNUSED /* */
#   define SDSDEPRECATED /* */
#endif





#endif /* ! defined _SDS_PORT_INCLUDE_ */


