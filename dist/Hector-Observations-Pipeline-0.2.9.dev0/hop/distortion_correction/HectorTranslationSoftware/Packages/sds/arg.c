/*			A R G 

 *  Module name:
      ARG
 
 *  Function:
      Argument library.

 *  Description:

 *  Language:
      C

 *  Support: Tony Farrell, AAO

 *

 *     @(#) $Id: ACMM:sds/arg.c,v 3.94 09-Dec-2020 17:15:24+11 ks $

 *  History:
      09-Dec-1992 - TJF - Added ArgCvt and ArgLook to original Arg module.
			  Rewrote original arg routines using ArgCvt.
      15-Aug-1994 - TJF - Make Ansi DBL_DIG and FLT_DIG macros available on
			  sun for use in conversion of real's to string.
      30-Aug-1994 - TJF - Improving selection of floating point macros
      07-Aug-1995 - TJF - Correctly support (or at least attempt to)
      				64 bit machines.
      
      29-Apr-1996 - TJF - Ensure that SdsFreeId works even if ArgCvt
      			fails.
      16-May-1997 - TJF - Replace all calls to malloc and free with
      				SdsMalloc and SdsFree to help debug
      				memory problems.  These are defined to
      				malloc and free in sds.h
      14-Apr-1998 - TJF - Add ArgFind.
      31-Aug-1999 - KS  - Added macintosh support.
      21-Jan-2002 - TJF - Add changes from Nick Rees (JACH) which reduce
                          warning messages in some cases.
      12-Sep-2000 - KS  - Modified test for use of float.h to include Gnu C,
                          ditto definitions of strtod() etc.
      20-Aug-2001 - KS  - Modified code to compile cleanly under gcc with the
                          -ansi -Wall flags set.
      12-May-2003 - TJF - Merge in KS's changes of 31-Aug-1999, 12-Sep-2000
                          and 20-Aug-2001 into main code.
                          Comment out entirely the prototypes for
                             strtod, strtol and strtoul.  They should not
                             be needed on any supported machine any more.
      02-Dec-2003 - TJF - Modified Arg__SdsList() such that it does not fail
                          if ArgToString() returns ARG__CNVERR, but instead
                          will just return the truncated bit.
      02-Apr-2004 - TJF - Add use of NATIVE__64 instead of LONG__64 and
                          add support for WIN32.   Make use of new 
                          SdsIsDefined() routine in ArgCvt().
                          Changes from Nick Rees@JACH.
      25-Jun-2004 - TJF - Add ArgIntToString() and ArgUIntToString() which
                          are used for format variables of type
                          ARG_MAX_INT_TYPE and ARG_MAX_UINT_TYPE, allowing
                          us to select the correct format specifier easily.
       2-Mar-2007 - TJF - Fix bug in conversion of unsigned 32/64 bit items
                            to strings when long is 64 bits (missing break).
      02-Nov-2011 - TJF - The 07-Aug-1995 change in ArgCvt() was a strange
                           one and has proved faulty.  Instead Puti/Putui/Geti/Getui
                           now set the type of the SDS item corresponding to
                           long/unsigned long appropiately based on the value
                           of the LONG_64 macro.
 */
/*
 *  Include files
 */


#if defined(VxWorks)
#    include <vxWorks.h>
#    include <memLib.h>
#    include <stdioLib.h>
#else
#    include <stdio.h>
#endif


static const char *rcsId="@(#) $Id: ACMM:sds/arg.c,v 3.94 09-Dec-2020 17:15:24+11 ks $";
static void *use_rcsId = (0 ? (void *)(&use_rcsId) : (void *) &rcsId);


#include <stdlib.h>
#include <string.h>


/*
#if (defined(unix) || defined(__unix__)) && !defined(hpux) && !defined(solaris2) && !defined(DPOSIX_1) && !defined(__alpha)
     double strtod(char * npnt, char ** endpnt);
     long int  strtol(char * npnt, char ** endpnt, int base);
     unsigned long int  strtoul(char * npnt, char ** endpnt, int base);
#endif
*/


/*
 *   Mac's (prior to OS 10 at least) don't have strdef, where we get
 *   NULL from.
 */
#ifdef macintosh
#ifndef NULL
#define NULL 0
#endif
#else
#include <stddef.h>
#endif


#include <errno.h>
#include <ctype.h>
#include "status.h"
#include "Ers.h"
#include "sds.h"
#include "arg.h"


/*
 * DUNUSED Flag which under GCC, prevents warnings about an argument being
 * unused - should be added where we know it will be unused.  We will
 * have picked this up already in DRAMA builds (from Ers.h I think),
 * but not necessarily in other builds (e.g. configure), so
 * define it if we need to.
 */
#ifndef DUNUSED
#if defined(__GNUC__) || defined(GNUC)
#   define DUNUSED /* Default definition */
#   ifdef __GNUC_MINOR__
#   if __GNUC_MINOR__ >= 7
#       undef  DUNUSED
#       define DUNUSED __attribute__ ((unused))
#   endif
#   endif
#else
#   define DUNUSED /* */
#endif
#endif /* DUNUSED */



/*
 *  The following values are taken from the Sds manual, except for
 *  the 64bit case of INT and UNIT, which have simply been calculated.
 */
#define ARG_MAX_BYTE  127
#define ARG_MIN_BYTE -127

#define ARG_MAX_UBYTE 255
#define ARG_MIN_UBYTE 0

#define ARG_MAX_SHORT  32767
#define ARG_MIN_SHORT -32767

#define ARG_MAX_USHORT 65535
#define ARG_MIN_USHORT 0

#ifdef NATIVE__INT64    /* We have a native 64 bit type */
#ifdef LONG__64         /* long is the 64 bit type */
#    define ARG_MAX_INT  9223372036854775807L
#    define ARG_MIN_INT -9223372036854775807L
#    define ARG_MAX_UINT 18446744073709551615UL
#else                   /* long-long is presumed to be the 64 bit type */
#    define ARG_MAX_INT  9223372036854775807LL
#    define ARG_MIN_INT -9223372036854775807LL

#    define ARG_MAX_UINT 18446744073709551615ULL
#endif
#    define ARG_MIN_UINT 0
#    define ARG_MAX_INT_TYPE  INT64
#    define ARG_MAX_UINT_TYPE UINT64
#else                   /* No native 64  bit type */
#    define ARG_MAX_INT  2147483647L
#    define ARG_MIN_INT -2147483647L

#    define ARG_MAX_UINT 4294967295UL
#    define ARG_MIN_UINT 0

#    define ARG_MAX_INT_TYPE  long int
#    define ARG_MAX_UINT_TYPE unsigned long int
#endif

/*
 *  These values define the lengths of strings required to represents the
 *  largest float and integer values.
 */
#define ARG_FLT_STRLEN 25	/* Based vax D/G type float	*/
#define ARG_INT_STRLEN 23	/* Based on a 64 bit integer	*/

/*
 *  Get floating point macros.  float.h was not available on older 
 *   machines (Such as SunOs - I don't know what else).  But we don't
 *   have SunOs machines any more so lets just assume it is always
 *   available.
 */
#if 1
#    include <float.h>
#else
/*
 *  Try for values.h -> Sun Os, maybe other machines.
 *
 *  Sun include files are not ansi compatibile.  We must ensure the
 *  sun macro is defined otherwise values.h won't work.
 */
#    if defined(__sun__) && !defined(sun)
#        define sun __sun__
#    endif
/*
 *  The values we require are in values.h, but not with the ANSI names.
 *  Define the ansi names here.
 */
#    include <values.h>
#    define FLT_MAX MAXFLOAT
#    define FLT_MIN MINFLOAT
#    define FLT_DIG FSIGNIF
#    define DBL_DIG FSIGNIF
#endif

/*
 *  This type is used by the conversion routines.
 */

enum ClassCode { tCinteger,  tCuinteger, tCreal, tCstring, tCother };
typedef enum ClassCode ClassType;


/*
 * These functions format an integer of the type ARG_MAX_INT_TYPE
 * or ARG_MAX_UINT_TYPE into a string.  
 */
static char * ArgIntToString(ARG_MAX_INT_TYPE intVal, char *string)
{
#ifdef LONG__64
    sprintf(string, "%ld", intVal);
#elif defined( NATIVE__INT64)
    sprintf(string, "%lld", intVal);
#else
    sprintf(string, "%ld", intVal);
#endif   
    return string;
}
static char * ArgUIntToString(ARG_MAX_UINT_TYPE intVal, char *string)
{
#ifdef LONG__64
    sprintf(string, "%lu", intVal);
#elif defined( NATIVE__INT64)
    sprintf(string, "%llu", intVal);
#else
    sprintf(string, "%lu", intVal);
#endif   
    return string;

}
 


/*
 *  Internal Function, name:
      ArgCvt___IntClass

 *  Description:
      Convert from the source class to Integer class. 

 *  History:
      04-Dec-1992 - TJF - Original version
      20-Aug-2001 - KS  - Added casts to int in isdigit() calls.
      25-Jun-2004 - TJF - Format integers through ArgIntToString.
      {@change entry@}
 */

static ARG_MAX_INT_TYPE ArgCvt___IntClass (
		ClassType SrcClass,	    /* Source class	    */
		ARG_MAX_INT_TYPE intVal,    /* If source is int	    */
                ARG_MAX_UINT_TYPE uintVal,  /* If source is uint    */
		double realVal,		    /* If source is real    */
		char * sVal,		    /* If source is string  */
		StatusType * ARGCONST status)
{
    if (*status != SDS__OK) return 0;

    switch (SrcClass)
    {
	case tCinteger:
	    return(intVal);
	case tCuinteger:
	    if (uintVal > ARG_MAX_INT)
	    {
                char s[ARG_INT_STRLEN];
		*status = ARG__CNVERR;
		ErsRep(0,status,
                       "ArgCvt range error converting \"%s\" to Integer",
                       ArgUIntToString(uintVal,s));
		return(ARG_MAX_INT);
	    }
	    else
		return( (ARG_MAX_INT_TYPE) uintVal);
	case tCreal:
	    if ((realVal > ARG_MAX_INT)||(realVal < ARG_MIN_INT))
	    {
		*status = ARG__CNVERR;
		ErsRep(0,status,
			      "ArgCvt range error converting \"%g\" to Integer",
			      realVal);
		return(0);
	    }
	    else
		return( (ARG_MAX_INT_TYPE)realVal);
	case tCstring:
	{
	    ARG_MAX_INT_TYPE l;
	    char *end;
	    l = strtol(sVal,&end,0);

/*
 *	    If the number is not an integer, try as a double
 */
	    if (*end !='\0')
	    {
		realVal = strtod(sVal,&end);
	        if ((realVal > ARG_MAX_INT)||(realVal < ARG_MIN_INT))
		    errno = ERANGE;
		else
		    l = (ARG_MAX_INT_TYPE) realVal;
	    }
/*
 *	    Check for a range error, 
 */
	    if (errno == ERANGE)
	    {
		*status = ARG__CNVERR;
		ErsRep(0,status,
			"ArgCvt range error converting \"%s\" to Integer",sVal);
	    }
/*
 *	    Otherwise, if we have not got the end of the string, assume
 *	    it is not a valid number.
 */
	    else if ((*end !='\0')&&(!isspace((int)*end)))
	    {
		*status = ARG__CNVERR;
		ErsRep(0,status,
			"ArgCvt cannot convert \"%s\" to Integer",sVal);
	    }
	    return(l);
        }
	default:
	{
	    *status = ARG__CNVERR;
	    ErsRep(0,status,"ArgCvt unknown source type");
	    return(0);
	}
    }  /* Switch on source class */
}


/*
 *  Internal Function, name:
      ArgCvt___UintClass

 *  Description:
      Convert from source class to Unsigned Integer class.

 *  History:
      04-Dec-1992 - TJF - Original version
      20-Aug-2001 - KS  - Added casts to int in isdigit() calls.
     {@change entry@}
 */

static ARG_MAX_UINT_TYPE ArgCvt___UintClass (
			    ClassType SrcClass,
			    ARG_MAX_INT_TYPE intVal,
			    ARG_MAX_UINT_TYPE uintVal,
			    double realVal,
			    char * sVal,
			    StatusType * ARGCONST status)
{
    if (*status != SDS__OK) return 0;
    switch (SrcClass)
    {
	case tCinteger:
	    if (intVal < ARG_MIN_UINT)
	    {
		*status = ARG__CNVERR;
		ErsRep(0,status,
			"ArgCvt range error converting \"%ld\" to Unsigned Integer",
			(long) intVal);
		return(ARG_MIN_UINT);
	    }
	    else
		return((ARG_MAX_UINT_TYPE) intVal);

	case tCuinteger: /* nothing to do	*/
	    return(uintVal);

	case tCreal:
	    if ((realVal > ARG_MAX_UINT)||(realVal < ARG_MIN_UINT))
	    {
		*status = ARG__CNVERR;
		ErsRep(0,status,
			"ArgCvt range error converting \"%g\" to Unsigned Integer",
			realVal);
	        return(0);
	    }
	    else
		return((ARG_MAX_UINT_TYPE) realVal);
		    
	case tCstring:
	{
	    ARG_MAX_UINT_TYPE u;
	    char *end;
	    u = strtoul(sVal,&end,0);
/*
 *	    If the number is not an unsigned integer, try as a double
 */
	    if (*end !='\0')
	    {
		realVal = strtod(sVal,&end);
	        if ((realVal > ARG_MAX_UINT)||(realVal < ARG_MIN_UINT))
		    errno = ERANGE;
		else
		    u = (ARG_MAX_UINT_TYPE) realVal;
	    }
/*
 *	    Check for a range error, 
 */
	    if (errno == ERANGE)
	    {
		*status = ARG__CNVERR;
		ErsRep(0,status,
			"ArgCvt range error converting \"%s\" to Unsigned Integer",sVal);
	    }
/*
 *	    Otherwise, if we have not got the end of the string, assume
 *	    it is not a valid number.
 */
	    else if ((*end !='\0')&&(!isspace((int)*end)))
	    {
		*status = ARG__CNVERR;
		ErsRep(0,status,
			"ArgCvt cannot convert \"%s\" to Unsigned Integer",sVal);
	    }
	    return(u);
	}
	default:
	{
	    *status = ARG__CNVERR;
	    ErsRep(0,status,"ArgCvt unknown source type");
	    return(0);
	}
    }  /* Switch on source class */
}

/*
 *  Internal Function, name:
      ArgCvt___RealClass

 *  Description:
      Convert from Source class to Real class.

 *  History:
      04-Dec-1992 - TJF - Original version
      20-Aug-2001 - KS  - Added casts to int in isdigit() calls.
      {@change entry@}
 */

static double ArgCvt___RealClass (
			    ClassType SrcClass,
			    ARG_MAX_INT_TYPE intVal,
			    ARG_MAX_UINT_TYPE uintVal,
			    double realVal,
			    char * sVal,
			    StatusType * ARGCONST status)
{
    if (*status != SDS__OK) return 0;
    switch (SrcClass)
    {
	case tCinteger:
	    return((double) intVal);
	case tCuinteger:
#ifdef _MSC_VER
	    return ((double) ((ARG_MAX_INT_TYPE) uintVal));
#else
	    return ((double) uintVal);
#endif
	case tCreal:
	    return (realVal);
	case tCstring:
	{
	    double r;
	    char *end;
	    r = strtod(sVal,&end);
/*
 *	    Check for a range error, 
 */
	    if (errno == ERANGE)
	    {
		*status = ARG__CNVERR;
		ErsRep(0,status,
			"ArgCvt range error converting \"%s\" to Real",sVal);
	    }
/*
 *	    Otherwise, if we have not got the end of the string, assume
 *	    it is not a valid number.
 */
	    else if ((*end !='\0')&&(!isspace((int)*end)))
	    {
		*status = ARG__CNVERR;
		ErsRep(0,status,"ArgCvt cannot convert \"%s\" to Real",sVal);
	    }
	    return (r);
        }
	default:
	{
	    *status = ARG__CNVERR;
	    ErsRep(0,status,"ArgCvt unknown source type");
	    return(0);
	}
    }  /* Switch on source class */
}


/*
 *  Internal Function, name:
      ArgCvt___ToString

 *  Description:
      Convert from any type to a string.  All parts of the conversion to
      a string are handled here.

      With the exception of String to String, we just use ErsSPrintf to
      do the conversion.  In the String to String case, we just do a copy.

 *  History:
      04-Dec-1992 - TJF - Original version
      15-Aug-1994 - TJF - For conversion of float and double, use the
			maximum number of digits of precision available on
			the machine.
      07-Aug-1995 - TJF - Support machines with 64bit longs.
      17-Apr-1997 - TJF - Add realsAreSim format allowing us to convert
      			float and double to simpler formats (6 and 10 digits
      			of precision instead of the maximum number).
      16-Aug-2001 - KS  - Added one cast to placate gcc -Wall.
      05-Jul-2018 - TJF - Use new SdsGetI64toD() and SdsGetUI64toD()
                          to support conversion of 64 bit values with
                          high byte non-zero on machines without
                          native 64 bit (long time coming).
 */

static void ArgCvt___ToString (
			    int realsAreSim, /*True to use simple real formats*/
			    ARGCONST void * SrcAddr,
			    SdsCodeType SrcType,
			    char * DstAddr,
			    unsigned long int    DstLen,
			    StatusType * ARGCONST status)
{
    int result = 0;
    switch (SrcType)
    {
	case SDS_CHAR:
	case SDS_BYTE:
	    result = ErsSPrintf(DstLen,DstAddr,"%d",*(char *)SrcAddr);
	    break;
	case SDS_UBYTE:
	    result = ErsSPrintf(DstLen,DstAddr,"%u",*(unsigned char *)SrcAddr);
	    break;
	case SDS_SHORT:
	    result = ErsSPrintf(DstLen,DstAddr,"%d",*(short *)SrcAddr);
	    break;
	case SDS_USHORT:
	    result = ErsSPrintf(DstLen,DstAddr,"%u",*(unsigned short *)SrcAddr);
	    break;
  	case SDS_INT:
	    result = ErsSPrintf(DstLen,DstAddr,"%ld",(long)(*(INT32 *)SrcAddr));
	    break;
	case SDS_UINT:
	    result = ErsSPrintf(DstLen,DstAddr,"%lu",(unsigned long)(*(UINT32 *)SrcAddr));
	    break;
	    
#       ifdef LONG__64
	/* We are on a machine with 64 bit longs, safe to use them directly */
  	case SDS_I64:
	    result = ErsSPrintf(DstLen,DstAddr,"%ld",*(long int *)SrcAddr);
	    break;
	case SDS_UI64:
	    result = ErsSPrintf(DstLen,DstAddr,"%lu",*(unsigned long int *)SrcAddr);
            break;
#       elif defined(NATIVE__INT64)
	/* We are on a machine with explict 64 bit INT64's, safe to use 
         * them directly  - through we go through the ArgIntToString() and
         * ArgUIntToString() functions to ensure we get the format 
         * right
         */
  	case SDS_I64:
        {
            char s[ARG_INT_STRLEN];
	    result = ErsSPrintf(DstLen,DstAddr,"%s",
                                ArgIntToString(*(INT64 *)SrcAddr,s));
	    break;
        }
	case SDS_UI64:
        {
            char s[ARG_INT_STRLEN];
	    result = ErsSPrintf(DstLen,DstAddr,"%s",
                                ArgUIntToString(*(UINT64 *)SrcAddr,s));
            break;
        }
#	else
	/* This machine does not have 64 bits longs.  If the high word
           is not zero, we must use floating point conversion. */
  	case SDS_I64:
  	{
  	    long high;
	    unsigned long low;
  	    SdsGetI64(*(INT64 *)SrcAddr,&high,&low);
  	    if (high != 0)
  	    {
                result = ErsSPrintf(DstLen, DstAddr,"%.15g ",
                                    SdsGetI64toD(*(INT64 *)SrcAddr));
  	    }
  	    else
            {
	        result = ErsSPrintf(DstLen,DstAddr,"%ld",low);
            }
	    break;
	}
  	case SDS_UI64:
  	{
	    unsigned long high,low;
  	    SdsGetUI64(*(UINT64 *)SrcAddr,&high,&low);
  	    if (high != 0)
  	    {
               result = ErsSPrintf(DstLen, DstAddr,"%.15g ",
                                    SdsGetUI64toD(*(UINT64 *)SrcAddr));

  	    }
  	    else
            {
	        result = ErsSPrintf(DstLen,DstAddr,"%ld",(unsigned long)low);
            }
	    break;
	}
#       endif
	case SDS_FLOAT:
	{
/*
 *	    If the realsAreSim flag is set, we use 6 digits or precisions,
 *	    otherwise we use the maximum number of digits of precision which is 
 *	    represented FLG_DIG.  To do this, we must create the format string
 *	    dynamically.
 */
	    char format[10];
	    ErsSPrintf(sizeof(format),format,"%%.%dg",realsAreSim ? 6 :FLT_DIG);
	    result = ErsSPrintf(DstLen,DstAddr,format,*(float *)SrcAddr);
	    break;
	}
	case SDS_DOUBLE:
	{
/*
 *	    If the realsAreSim flag is set, we use 10 digits or precisions,
 *	    otherwise use the maximum number of digits of precision which is 
 *	    represented DBL_DIG.  To do this, we must create the format string
 *	    dynamically.
 */
	    char format[10];
	    ErsSPrintf(sizeof(format),format,"%%.%dg",realsAreSim ? 10:DBL_DIG);
	    result = ErsSPrintf(DstLen,DstAddr,format,*(double *)SrcAddr);
	    break;
	}
	case ARG_STRING:
	case ARG_STRING2:
	    strncpy(DstAddr,(char *)SrcAddr,DstLen);
	    if ((strlen((char *)SrcAddr)+1) > DstLen)
		result = EOF;
	    break;
	default:
	    *status = ARG__CNVERR;
	    ErsRep(0,status,"ArgCvt unknown source type code = \"%ld\"",
                                                       (long)SrcType);
	    break;
    }
/*
 *  Handle conversion error.
 */
    if (result == EOF)
    {
	*status = ARG__CNVERR;
	ErsRep(0,status,
	"ArgCvt error converting to string, destination length too small");
    }
}


/*
 *+			A R G C V T

 *  Function name:
      ArgCvt

 *  Function:
	Convert from one scaler SDS type to Another.

 *  Description:
	This routine meets requirements for a general type conversion
	between Sds scaler types.  There are three classes of scalars

		1. Signed Integers
		2. Unsigned Integers
		3. Real (floating point) values

	Within each class, Sds can represent various types.  For example-
	Sds supports SDS_BYTE, SDS_SHORT SDS_INT and SDS_I64 versions of 
	the Signed integer class.  The difference is the number of bytes 
	required for
	each one.
	
	This routine does a three part conversion-
	    1. Convert the source type to the largest type of the same class
		    (char, short -> long int)
		    (unsigned char, unsigned short -> unsigned long int)
		    (float -> double )
		   
 
            2. Convert the value above to the largest type of the destination
		class.

	    3. Convert the value in 2 to the actual destination type.

        Range errors are possible during the conversions.


	The source/destination for a conversion can be the address of 
	a value of the appropiate type or it may be an Sds item.

	When specifing the address of the value, you must specify the
	type. (SDS_CHAR, SDS_INT, SDS_UINT etc.  (Not SDS_STRUCT)).
	Additional type codes - ARG_STRING/ARG_STRING2 - can be speicifed 
	indicating the source or destination is a null terminated string while 
	should/will contain a representation of the number.  (If both source
	and destination are strings, then a simple string copy is done.)
	A ARG_STRING2 type differs in only when the source is a FLOAT or
	DOUBLE value.  When ARG_STRING is used, the maximum number of 
	decimal digits is retrieved whilst when ARG_STRING2 is used, 6 is used
	for FLOAT and 10  for double.  (Note that a type of SDS_CHAR 
	represents a single character, not a string of characters)  
	
	
	To specify an Sds item as the source/destination, supply the
	address of the Sds id of the item in the appropiate address argument.
	Suppy ARG_SDS as the corresponding type code.  The Sds id must
	describe a Scaler item, expect if it is a one dimensional 
	character array.  In this case, it is considered a character string.
	Source strings must be null terminated.

	Invalid conversions result in status being set to ARG__CNVERR and
	an error being reported using ErsRep.

	The ranges of integer types are determined by the range acceptable to
	SDS.  The ranges of real types are determined by the archecture
	on which the machine is running.
      

	Types of SDS_INT and SDS_UINT indicate the relevant item is a long
	int (which may be 32 or 64 bits, depending upon the machine and
	compiler being used).  Note that if the machine does not support
	64 bits integers then 64bit values with the high 32bits set to
	non-zero values cannot be handled - an error is returned.  

 *  Language:
      C

 *  Call:
      (Void) = ArgCvt (SrcAddr, SrcType, DstType, DstAddr, DstLen, status)

 *  Parameters:   (">" input, "!" modified, "W" workspace, "<" output)
      (>) SrcAddr	(void *)     Address of the source data or of an
					Sds item id.
      (>) SrcType	(SdsCodeType) Type of the source data.
      (>) DstType	(SdsCodeType) Type of the destination data.
      (>) DstAddr	(void *)     Address of the destination or of an
					Sds item id.
      (>) DstLen	(int)	     Length of the destination in bytes.  
			    If DstType  is ARG_SDS, then this is ignored.
      (!) status	(StatusType *) Modified status. 

 *  Include files: Arg.h

 *  External functions used:
	ErsRep	    (Ers)   Report an error.
	ErsSPrintf  (Ers)   Format a string into a buffer.
	strtol	    (CRTL)  Convert a decimal string to a long.
	strtoul	    (CRTL)  Convert a decimal string to an unsigned long.
	strtod	    (CRTL)  Convert a decimal string to a double.
	strncpy	    (CRTL)  Copy one string to another
	strlen	    (CRTL)  Get the length of a string.

 *  External values used:
      none

 *  Prior requirements:
      none

 *  Support: Tony Farrell, AAO

 *-

 *  History:
      04-Dec-1992 - TJF - Original version
      07-Aug-1995 - TJF - Correctly support 64bit integers.
      16-Nov-1995 - TJF - Tidy up 64 Bit integer stuff to avoid compilation
			warnings under solaris 2.
      17-Apr-1997 - TJF - Support ARG_STRING2 destination format.
      14-Aug-1997 - TJF - Fix error where we were not correctly handling
			failure in SdsInfo.
      16-Aug-2001 - KS  - Added various casts to placate gcc -Wall.
      25-Jun-2004 - TJF - Format integers through ArgIntToString/ArgUIntToString.
      14-Mar-2008 - TJF - Only call SdsResize() on strings if we don't have
                            sufficent space -  avoid calling it (and hence free/malloc)
                            unnecessarly.
      01-Dec-2011 - TJF - There is code which if it finds LONG__64 set, then
                            it will change the source/destination type to the
			    64 bit values.  This is causing problems and seems
			    to be wrong - I don't understand why it was here.
			    Disabled.
      12-Jul-2018 - TJF - Drop DstClass variable, was not being used.
 */

extern void ArgCvt ( 
		    ARGCONST void * SrcAddr,
		    SdsCodeType SrcType,
		    SdsCodeType DstType,
		    void * DstAddr,
		    unsigned long int DstLen , 
		    StatusType * ARGCONST status)
{	
    ClassType SrcClass = tCother;   /* Source type class	*/

    int	    SavedErrno;		    /* Saved value of Errno	*/
/*
 *  These values are used to hold the value being converted during the
 *  conversion.  The INT/UINT ones use the largest natural type we may see, 
 *  expected to be at least 64 bits if NATIVE__INT64 is defined.
 *
 */
    ARG_MAX_INT_TYPE intVal = 0;
    ARG_MAX_UINT_TYPE uintVal = 0;
    double realVal = 0.0;
    char *sVal = 0;
/*
 *  Sds id's for source and destination if available
 */
    SdsIdType	SrcSdsId = 0;
    SdsIdType	DstSdsId = 0;

    unsigned long dims[SDS_C_MAXARRAYDIMS];
    long ndims;
    char tname[SDS_C_NAMELEN];

/*
 *  Start of code.
 */
    if (*status != SDS__OK) return;

/*
 *  If the Source Type is ARG_SDS, then SrcAddr is the address of an 
 *  Sds Id.  Check the data type is appropiate set SrcAddr to point to it.
 */
    if (SrcType == ARG_SDS)
    {
	unsigned long int length;
	SrcSdsId = *(SdsIdType *)SrcAddr;

	SdsInfo(SrcSdsId,
		tname,
		&SrcType,
		&ndims,
		dims,
		status);

	if (*status != SDS__OK)
	    ;	/* Do Nothing */

	else if ((SrcType != SDS_STRUCT) && !SdsIsDefined( SrcSdsId,status))
            *status = SDS__UNDEFINED;
	else if ((SrcType == SDS_CHAR)&&(ndims == 1))
	{
/*
 *	    If we have a one dimensional character array, then
 *	    see if it is null terminated.  If not, we don't consider it
 *	    a character array.
 */ 
	    int i;
	    SdsPointer(SrcSdsId,(void **)&SrcAddr,&length,status);
	    if (*status == SDS__OK)
	    {
	        for (i = length-1 ; (i >= 0) && ((char *)(SrcAddr))[i]; --i);
/*
 *	        If i is less then zero then we don't have a null terminated 
 *              string assume is it a character array which we can't convert.
 */
	        if (i < 0)
		    *status = ARG__NOTSCALAR;
	        else 
/*
 *		We have a null terminated character array, consider it a 
 *		string and use it.
 */
		    SrcType = ARG_STRING;
	    }
	}
/*
 *	If the type is scaler, we can use it.
 */
	else if ((SrcType != SDS_STRUCT)&&(ndims == 0))
        {
	    SdsPointer(SrcSdsId,(void **)&SrcAddr,&length,status);
        }

	else
	    *status = ARG__NOTSCALAR;
    }
#   if 0  /* ifdef LONG__64   ** Disabled - see comment below ***/
/*
 *  On machines with 64bit long integers, SDS_INT and SDS_UINT will indicate
 *  the source is a 64 bit long, so convert them.  This is not necessary
 *  for items within Sds structures (above) since they are really 32 bits.
 *
 *  TJF - 1/Dec/2011 - This is wrong.  SDS_INT and SDS_UINT always indicate
 *                      32 bit or smaller values.  Code disabled.  It is the values
 *                      that we might be passed which are an issue.  They should be
 *                      INT32 or UINT32 to get this right.
 */ 
    else if (SrcType == SDS_INT)
        SrcType = SDS_I64;
    else if (SrcType == SDS_UINT)
        SrcType = SDS_UI64;
#   endif
/*
 *  If the Destination type is ARG_SDS, then the destination is an Sds item.
 *  DstAddr is a pointer to the address.  We check we can use it and then
 *  set DstAddr to point to it.  We also retreive the length for DstLen.
 */

    if ((DstType == ARG_SDS)&&(*status == SDS__OK))
    {
	DstSdsId = *(SdsIdType *)DstAddr;
	SdsInfo(DstSdsId,
		tname,
		&DstType,
		&ndims,
		dims,
		status);

	if (*status != SDS__OK)
	    ;	/* Do Nothing */
/*
 *	A one dimensional character array is treated as a string.  We
 *	Must resize the destination string to the appropriate length. 
 *	When the source is a string, this is the length of the source
 *	string, otherwise, it is an appropiate length for a scaler quatity.
 */
	else if ((DstType == SDS_CHAR)&&(ndims == 1))
	{
            unsigned long origDims = dims[0];

	    if ((SrcType == ARG_STRING)||(SrcType == ARG_STRING2))
		dims[0] = strlen(SrcAddr)+1;
	    else if ((SrcType == SDS_FLOAT)||(SrcType == SDS_DOUBLE))
		dims[0] = ARG_FLT_STRLEN;
	    else
		dims[0] = ARG_INT_STRLEN;

            if (dims[0] > origDims) {
                SdsResize(DstSdsId,1,dims,status);
            }
	    SdsPointer(DstSdsId,&DstAddr,&DstLen,status);
	    DstType = ARG_STRING;

	}
/*
 *	If the type is scaler, we can use it.
 */
	else if ((DstType != SDS_STRUCT)&&(ndims == 0))
	    SdsPointer(DstSdsId,&DstAddr,&DstLen,status);

	else
	    *status = ARG__NOTSCALAR;
    }
#   if 0  /* ifdef LONG__64   ** Disabled - see comment below ***/
/*
 *  On machines with 64bit long integers, SDS_INT and SDS_UINT will indicate
 *  the destination is a 64 bit long, so convert them.  This is not necessary
 *  for items within Sds structures (above) since they are really 32 bits.
 *
 *  TJF - 1/Dec/2011 - This is wrong.  SDS_INT and SDS_UINT always indicate
 *                      32 bit or smaller values.  Code disabled.  It is the values
 *                      that we might be passed which are an issue.  They should be
 *                      INT32 or UINT32 to get this right.
 */ 
    else if (DstType == SDS_INT)
        DstType = SDS_I64;
    else if (DstType == SDS_UINT)
        DstType = SDS_UI64;
#   endif

/*
 *  Short circuit on any error in the above code
 */
    if (*status != SDS__OK) return;

/*
 *  Converting to a string.  Things are a bit different, just call the
 *  routine to do it and return.
 */
    if ((DstType == ARG_STRING)||(DstType == ARG_STRING2))
    {
	ArgCvt___ToString(DstType == ARG_STRING2, 
			SrcAddr,SrcType,(char *)DstAddr,DstLen,status);
	if (SrcSdsId)
	    SdsFlush(SrcSdsId,status);
	if (DstSdsId)
	    SdsFlush(DstSdsId,status);
	return;
    }
/*
 *  We use some routines which set errno, but return our own status, so
 *  we save the current errno and restore it on exit.
 */
    SavedErrno = errno;
    errno = 0;

/*
 *  Work out the class of the source (integer, uinteger, real, string) and
 *  convert the source to the largest size type of that class.
 */    
    switch (SrcType)
    {
	case SDS_CHAR:		/* Byte and Char are the same	    */
	case SDS_BYTE:
	{
	    intVal = *(char *)SrcAddr;
	    SrcClass = tCinteger;
	    break;
	}
	case SDS_UBYTE:		/* Unsigned byte (actually unsigned char */
	{
	    uintVal = *(unsigned char *)SrcAddr;
	    SrcClass = tCuinteger;
	    break;
	}
	case SDS_SHORT:
	{ 
	    intVal = *(short *)SrcAddr;
	    SrcClass = tCinteger;
	    break;
	}
	case SDS_USHORT:
	{
	    uintVal = *(unsigned short *)SrcAddr;
	    SrcClass = tCuinteger;
	    break;
	}
	case SDS_INT:
	{
	    intVal = *(INT32 *)SrcAddr;
	    SrcClass = tCinteger;
	    break;
	}
	case SDS_UINT:
	{
	    uintVal = *(UINT32 *)SrcAddr;
	    SrcClass = tCuinteger;
	    break;
	}
#	ifdef NATIVE__INT64
	/* This machine supports 64 bits longs */
	case SDS_I64:
	{
	    intVal = *(INT64 *)SrcAddr;
	    SrcClass = tCinteger;
	    break;
	}
	case SDS_UI64:
	{
	    uintVal = *(UINT64 *)SrcAddr;
	    SrcClass = tCuinteger;
	    break;
	}
	
#	else
	/* This machine does not support 64bit longs.  We must 
		extract the values and can only handle values with the
		msb = 0 */
	case SDS_I64:
	{
            long high;
	    unsigned long low;
            SdsGetI64(*(INT64 *)SrcAddr,&high,&low);
	    SrcClass = tCinteger;
            if ((high != 0)||(low > 2147483647L))
            {
               *status = ARG__CNVERR;
                 ErsRep(0,status,
                        "ArgCvt 64 bit long value too large for 32 bits.");
            }
	    else
		intVal = (long)low;
	    break;
	}
	case SDS_UI64:
	{
            unsigned long high;
            SdsGetUI64(*(UINT64 *)SrcAddr,&high,&uintVal);
            if (high != 0)
            {
               *status = ARG__CNVERR;
                 ErsRep(0,status,
                    "ArgCvt 64 bit unsigned long value too large for 32 bits.");
            }
	    SrcClass = tCuinteger;
	    break;
	}
#	endif
	case SDS_FLOAT:
	{
	    realVal = *(float *)SrcAddr;
	    SrcClass = tCreal;
	    break;
	}
	case SDS_DOUBLE:
	{
            memcpy((char *)&realVal,(char *)SrcAddr,sizeof(double));
	    /*realVal = *(double *)SrcAddr;*/
	    SrcClass = tCreal;
	    break;
	}
	case ARG_STRING:
	case ARG_STRING2:
	{
	    SrcClass = tCstring;
	    sVal = (char *)SrcAddr;
	    break;
	}
	default:
	{
	    *status = ARG__CNVERR;
	    ErsRep(0,status,"ArgCvt unknown source type code = \"%ld\"",
          (long)SrcType);
	    break;
	}
    }

/*
 *  Get the destination class and convert the value from the source call
 *  to that class.  We may get conversion errors here.
 */
    switch (DstType)
    {
/*
 *	Destination class is signed integers.
 */
	case SDS_CHAR:
	case SDS_BYTE:
	case SDS_SHORT:
	case SDS_INT:
	case SDS_I64:
	{
	    intVal = ArgCvt___IntClass(SrcClass,intVal,uintVal,realVal,sVal,status);
	    break;
	}
/*
 *	Destination Class is unsigned integers.
 */
	case SDS_UBYTE:
	case SDS_USHORT:
	case SDS_UINT:
	case SDS_UI64:
	{
	    uintVal = ArgCvt___UintClass(SrcClass,intVal,uintVal,realVal,sVal,status);
	    break;
	}
/*
 *	Destination Class is Real.
 */
	case SDS_FLOAT:
	case SDS_DOUBLE:
	{
	    realVal = ArgCvt___RealClass(SrcClass,intVal,uintVal,realVal,sVal,status);
	    break;
	}
/*
 *	Error, note - we don't get here for a destination type of ARG_STRING/2.
 */
	default:
	{
	    *status = ARG__CNVERR;
	    ErsRep(0,status,"ArgCvt unknown destination type code = \"%ld\"",
         (long)DstType);
	}
    }  /* Switch on destination type */



/*
 *  At this point, DstType indicates the output parameter class and the
 *  appropriate one of intVal, uintVal, floatVal.  A string destination
 *  does not get here.
 *
 *  We now go from the largest size of the destination class to the
 *  actual destination type.  We may get range errors here.  We also
 *  check we have sufficient space in the destination.
 */
    if (*status == SDS__OK)
    {
        switch (DstType)
        {
	    case SDS_CHAR:
	    case SDS_BYTE:  
	    {
		if (DstLen < sizeof(char))
		{
                    char s[ARG_INT_STRLEN];
		    *status = ARG__CNVERR;
		    ErsRep(0,status,
		   	"ArgCvt error converting \"%s\", destination length too small",
                           ArgIntToString(intVal, s));
		}
		else if ((intVal > ARG_MAX_BYTE)||(intVal < ARG_MIN_BYTE))
		{
                    char s[ARG_INT_STRLEN];
		    *status = ARG__CNVERR;
		    ErsRep(0,status,
			"ArgCvt range error converting \"%s\" to Byte",
                           ArgIntToString(intVal,s));
		}
		else
		    *((char *)DstAddr) = (char)intVal;
		break;
	    }
	    case SDS_UBYTE:
	    {
		if (DstLen < sizeof(unsigned char))
		{
                    char s[ARG_INT_STRLEN];
		    *status = ARG__CNVERR;
		    ErsRep(0,status,
		       "ArgCvt error converting \"%s\", destination length too small",
                           ArgUIntToString(uintVal,s));
		}
		else if (uintVal > ARG_MAX_UBYTE)
		{
                    char s[ARG_INT_STRLEN];
		    *status = ARG__CNVERR;
		    ErsRep(0,status,
			"ArgCvt range error converting \"%s\" to Unsigned Byte",
                           ArgUIntToString(uintVal,s));
		}
		else
		    *((unsigned char *)DstAddr) = (unsigned char)uintVal;
		break;
	    }
	    case SDS_SHORT:
	    {
		if (DstLen < sizeof(short))
		{
                    char s[ARG_INT_STRLEN];
		    *status = ARG__CNVERR;
		    ErsRep(0,status,
		        "ArgCvt error converting \"%s\", destination length too small",
                           ArgIntToString(intVal,s));
		}
		else if ((intVal > ARG_MAX_SHORT)||(intVal < ARG_MIN_SHORT))
		{
                    char s[ARG_INT_STRLEN];
		    *status = ARG__CNVERR;
		    ErsRep(0,status,
			   "ArgCvt range error converting \"%s\" to Short",
                           ArgIntToString(intVal,s));
		}
		else
		    *((short *)DstAddr) = (short)intVal;
		break;
	    }
	    case SDS_USHORT:
	    {
		if (DstLen < sizeof(unsigned short))
		{
                    char s[ARG_INT_STRLEN];
		    *status = ARG__CNVERR;
		    ErsRep(0,status,
		        "ArgCvt error converting \"%s\", destination length too small",
                           ArgUIntToString(uintVal,s));
		}
		else if (uintVal > ARG_MAX_USHORT)
		{
                    char s[ARG_INT_STRLEN];
		    *status = ARG__CNVERR;
		    ErsRep(0,status,
			       "ArgCvt range error converting \"%s\" to Unsigned Byte",
                           ArgUIntToString(uintVal,s ));
		}
		else
		    *((unsigned short *)DstAddr) = (unsigned short)uintVal;
		break;
	    }
	    case SDS_INT:
		if (DstLen < sizeof(INT32))
		{
                    char s[ARG_INT_STRLEN];
		    *status = ARG__CNVERR;
		    ErsRep(0,status,
		      "ArgCvt error converting \"%s\", destination length too small",
                           ArgIntToString(intVal,s));
		}
		else
		    *((INT32 *)DstAddr) = (INT32)intVal;
		break;
	    case SDS_UINT:
		if (DstLen < sizeof(UINT32))
		{
                    char s[ARG_INT_STRLEN];
		    *status = ARG__CNVERR;
		    ErsRep(0,status,
		       "ArgCvt error converting \"%s\", destination length too small",
                           ArgUIntToString(uintVal,s));
		}
		else
		    *((UINT32 *)DstAddr) = (UINT32)uintVal;
		break;
#	    ifdef NATIVE__INT64
	    /* 64 bits longs ok */
	    case SDS_I64:
		if (DstLen < sizeof(INT64))
		{
                    char s[ARG_INT_STRLEN];
		    *status = ARG__CNVERR;
		    ErsRep(0,status,
                           "ArgCvt error converting \"%s\", destination length too small",
                           ArgIntToString(intVal,s));
		}
		else
		    *((INT64 *)DstAddr) = (INT64)intVal;
		break;
	    case SDS_UI64:
		if (DstLen < sizeof(UINT64))
		{
                    char s[ARG_INT_STRLEN];
		    *status = ARG__CNVERR;
		    ErsRep(0,status,
		           "ArgCvt error converting \"%s\", destination length too small",
                           ArgUIntToString(uintVal,s));
		}
		else
		    *((UINT64 *)DstAddr) = (UINT64)uintVal;
		break;
#	    else
	    /* Destination is 64 bit integers but machine does not support 
				them.*/
	    case SDS_I64:
		if (DstLen < sizeof(INT64))
		{
		    *status = ARG__CNVERR;
		    ErsRep(0,status,
		       "ArgCvt error converting \"%ld\", destination length too small",
                 intVal);
		}
		else
		    *((INT64 *)DstAddr) = SdsSetI64(0,intVal);
		break;
	    case SDS_UI64:
		if (DstLen < sizeof(UINT64))
		{
		    *status = ARG__CNVERR;
		    ErsRep(0,status,
		      "ArgCvt error converting \"%lu\", destination length too small",
                                                              uintVal);
		}
		else
		    *((UINT64 *)DstAddr) = SdsSetUI64(0,uintVal);
		break;
#	    endif
	    case SDS_FLOAT:
	    {
		if (DstLen < sizeof(float))
		{
		    *status = ARG__CNVERR;
		    ErsRep(0,status,
		           "ArgCvt error converting \"%g\", destination length too small",realVal);
		}
		else if ((realVal > (double)FLT_MAX)||(realVal < (double)(FLT_MAX* -1.0)))
		{
		    *status = ARG__CNVERR;
		    ErsRep(0,status,
			"ArgCvt range error converting \"%g\" to float",realVal);
		}
		else
		    *((float *)DstAddr) = (float)realVal;
		break;
	    }
	    case SDS_DOUBLE:
		if (DstLen < sizeof(double))
		{
		    *status = ARG__CNVERR;
		    ErsRep(0,status,
		           "ArgCvt error converting \"%g\", destination length too small",realVal);
		}
		else
		    *((double *)DstAddr) = (double)realVal;
		break;
         } /* switch on destination type */
    }	/* status == OK */
/*
 *  If the source/destination is an Sds item, flush it.  (I feel it is not
 *  logical to flush a source, since we have not changed it, but according to
 *  the Sds document, it must be done.
 */
    if (SrcSdsId)
	SdsFlush(SrcSdsId,status);
    if (DstSdsId)
	SdsFlush(DstSdsId,status);

    errno = SavedErrno;
}


/*
 *+			A R G L O O  K

 *  Function name:
      ArgLook

 *  Function:
	Look at the contents of a string 
	and determined if it can be represented as a number.

 *  Description:
	A common requirement in user interfaces is to convert a
	string containing a number to the machine representation of	
	that number.  This routine determines if this is possible and
	if so, what type the string can be converted to.

	1. If the string has the format - [+|-]nnn, where nnn means any
	number of decimal digits, this routine beleives it can be 
	be represented as an integer.

	2. If the string has the format -  [+|-]nnn[.nnn][{e|E}[+/-]nnn],
	where nnn means nay number of decimal digites, this routine
	beleives it can be represented as a real.

	The actual ability to represent the number in a given machine type
	is dependent on the size of the number.

	If MinFlag is flase and the string has the format of 1, then DstType 
	is set to  SDS_INT, except if USFlag is set true, In which
	case DstType is set to SDS_UINT (unless the number is negative).

	If MinFlag is false and the string has the format of 2, then	 
	DstType is set to SDS_DOUBLE.

	If MinFlag is true, then the routine attempts to determine the
	smallest size of the required type which can be used to represent
	the number.  If base type (INT or REAL) is determined as per
	when MinFlag is false.  The system then tries to convert the
	value to a number of that type using ArgCvt.  If this is successfull,
	then it looks at the resulting values and determines the smallest
	type which can be used to represet the number.

	If the string is not a valid number (or when MinFlag is true, cannot be
	represented in the largest type) then DstType is set to ARG_STRING.

	Any precedding white space in the string is ignored, but
	trailing white space is not allowed.

 *  Language:
      C

 *  Call:
      (Void) = ArgLook (SrcAddr, USFlag, MinFlag, DstType , status)

 *  Parameters:   (">" input, "!" modified, "W" workspace, "<" output)
      (>) SrcAddr	(char *)     Address of the source data.
      (>) USFlag	(int)	If true, integers are unsigned unless
				they include a minus sign.  If false,
				integers are always signed.
      (>) MinFlag	(int)   If true, then find the smallest type of the
				appropiate class which can represent this
				time.  E.g., use float instead of double
				if possible.  This is a more expensive operation
				as ArgLook must invoke ArgCvt to do a conversion
				in order to determine this.
      (<) DstType	(SdsCodeType *)	     Type of the destination.
      (!) status	(Long int *) Modified status. 

 *  Include files: Arg.h

 *  External functions used:
	ErsPush	    (Ers)   Increment error context.
	ErsPop	    (Ers)   Decrement error context.
	ErsAnnul    (Ers)   Annull error messages.

	ArgCvt	    (Arg)   Convert a value from one type to another.

	isspace	    (CRTL)  Is a character a which space character.
	isdigit	    (CRTL)  Is a character a digit.

 *  External values used:
      none

 *  Prior requirements:
      none

 *  Support: Tony Farrell, AAO

 *-

 *  History:
      04-Dec-1992 - TJF - Original version
      20-Aug-2001 - KS  - Added casts to int in isdigit() calls.
      {@change entry@}
 */

extern void ArgLook ( 
		    char * SrcAddr,
		    int	   USFlag,
		    int	   MinFlag,
		    SdsCodeType  * DstType,
		    StatusType * ARGCONST status)
{
    int	     MinusFound = 0;
    char     *Source = SrcAddr;
     
	
    if (*status != SDS__OK) return;
/*
 *  The default type is a string.
 */
    *DstType = ARG_STRING;
/*
 *  Skip any spaces.
 */
    while(isspace((int)*Source))
	++Source;

/*
 *  Get any preceeding sign.
 */
    if (*Source == '-')
    {
	++MinusFound;
	++Source;    
    }
    else if (*Source == '+')
	++Source;    

/*
 *  If the next character is a digit, we may have a number - soak up all the
 *  numbers until the next non number.
 */
    if (isdigit((int)*Source))
    {
        while (isdigit((int)*Source))
	     ++Source;
/*
 *	If the next character is null, this is the end, we have an integer.
 */
        if (*Source == '\0')
	{
	    if ((USFlag)&&(!MinusFound))
		*DstType = SDS_UINT;
	    else
		*DstType = SDS_INT;

	}

/*
 *	If we have a decimal point, we may have a float.
 */
	else if (*Source == '.')
	{
	    ++Source;
	    if (isdigit((int)*Source))
	    {
	        while (isdigit((int)*Source))
		    ++Source;
/*
 *		If we have a null, we already have a valid float.
 */ 
		if (*Source == '\0')
		    *DstType = SDS_DOUBLE;
/*
 *		IF we have a e/E, we may have a float.  It may be followed by
 *		a sign, but must then be followed by numbers
 */
		else if ((*Source == 'e')||(*Source == 'E'))
		{
		    ++Source;
		    if ((*Source == '+')||(*Source == '+'))
			++Source;
		    if (isdigit((int)*Source))
		    {
			while (isdigit((int)*Source))
			    ++Source;
			if (*Source == '\0')
			    *DstType = SDS_DOUBLE;
		    }
		}
		    
	    }
	}
/*
 *	If we have a e/E, we may have a float, it may be followed by a sign
 *	but must then be followed by numbers.
 */
	else if ((*Source == 'e')||(*Source == 'E'))
	{
	    ++Source;
	    if ((*Source == '+')||(*Source == '+'))
		++Source;
	    if (isdigit((int)*Source))
	    {
		while (isdigit((int)*Source))
		    ++Source;
		if (*Source == '\0')
		    *DstType = SDS_DOUBLE;
	    }
	}

    }
/*
 *  If we want the mininum type of a type-class, we must do a conversion
 *  and look at the value we get.
 */
    if (MinFlag)
    {
	ErsPush();
	switch (*DstType)
	{
	    case SDS_INT:
	    {
		INT32 i;
		ArgCvt(SrcAddr,ARG_STRING,SDS_INT,&i,sizeof(i),status);
		if (*status != SDS__OK)
		    *DstType = ARG_STRING;
		else if ((i <= ARG_MAX_BYTE)&&(i >= ARG_MIN_BYTE))
		    *DstType = SDS_BYTE;
		else if ((i <= ARG_MAX_SHORT)&&(i >= ARG_MIN_SHORT))
		    *DstType = SDS_SHORT;
		break;
	    }
	    case SDS_UINT:
	    {
		UINT32 i;
		ArgCvt(SrcAddr,ARG_STRING,SDS_UINT,&i,sizeof(i),status);
		if (*status != SDS__OK)
		    *DstType = ARG_STRING;
		else if (i <= ARG_MAX_UBYTE)
		    *DstType = SDS_UBYTE;
		else if (i <= ARG_MAX_USHORT)
		    *DstType = SDS_USHORT;
		break;
	    }
	    case SDS_DOUBLE:
	    {
		double i;
		ArgCvt(SrcAddr,ARG_STRING,SDS_DOUBLE,&i,sizeof(i),status);
		if (*status != SDS__OK)
		    *DstType = ARG_STRING;
		else if ((i <= (double)FLT_MAX)&&(i >= (double)FLT_MIN))
		    *DstType = SDS_FLOAT;
		break;
	    }
	    default:
		break;
	}
	ErsAnnul(status);
	ErsPop();
    }
}







/*
 *+				 A r g N e w

 *  Function name:
      ArgNew

 *  Function:
      Create a new argument structure

 *  Description:
      Creates a new structure to hold arguments and return an
      identifier to it
      
 *  Language:
      C

 *  Declaration:
      void ArgNew(SdsIdType *id, long *status)
      
 *  Parameters:   (">" input, "!" modified, "W" workspace, "<" output)
      (<) id        (SdsIdType*) Identifier to the created structure
      (!) status    (StatusType*) Modified status. Possible failure codes are:
                            SDS__NOMEM => Insufficient memory for creation

 *  Support: Jeremy Bailey, {AAO}

 *  Version date: 26-Mar-92
 *-
 */

void ArgNew(SdsIdType *id, StatusType * ARGCONST status)

{
   SdsNew(0,"ArgStructure",0,NULL,SDS_STRUCT,0,NULL,id,status);
}

/*
 *+				 A r g F i n d

 *  Function name:
      ArgFind

 *  Function:
      Call SdsFind, but report any error using ErsRep.

 *  Description:
      This routine simply calls SdsFind, but if SdsFind returns a bad
      status, then this routine reports the name of the item
      SdsFind was trying to find.
      
 *  Language:
      C

 *  Declaration:
       void ArgFind(SdsIdType parent_id, char *name, SdsIdType *id,
         StatusType *  status)

 *  Parameters:   (">" input, "!" modified, "W" workspace, "<" output)
      (>) parent_id (SdsIdType) Identifier of the structure.
      (>) name      (char*) Name of the component to be found.
      (<) id        (SdsIdType*) Identifier to the component.
      (!) status    (StatusType*) Modified status. Possible failure codes are:
                            SDS__NOMEM => Insufficient memory for creation
                            SDS__NOTSTRUCT => parent_id not a structure
                            SDS__NOITEM => No item with that name


 *  Support: Tony Farrell, {AAO}

 *  See Also: SdsFind().

 *  Version date: 14-Apr-98
 *-
 */

void ArgFind(SdsIdType parent_id, ARGCONST char *name, SdsIdType *id, 
             StatusType * ARGCONST status)

{
    if (*status != STATUS__OK) return;
    SdsFind(parent_id, name, id, status);
    if (*status != STATUS__OK)
    {
        ErsRep(0,status,"Failed to find SDS item \"%s\"",
               name);
    }
}


/*+				 A r g P u t c

 *  Function name:
      ArgPutc

 *  Function:
      Put a character item into an argument structure

 *  Description:
      A character item is written into a named component within the
      specified argument structure. The component is created if it
      does not currently exist.
      
 *  Language:
      C

 *  Declaration:
      void ArgPutc(SdsIdType id, const char *name, char value, const StatusType * status)
      
 *  Parameters:   (">" input, "!" modified, "W" workspace, "<" output)
      (>) id        (SdsIdType) Identifier to the structure.
      (>) name      (char*) Name of the component to be written to.
      (>) value     (char)  Character value to be written.
      (!) status    (StatusType*) Modified status. Possible failure codes are:
                            SDS__NOMEM => Insufficient memory for creation
                            SDS__BADID => Invalid Identifier
                            ARG__NOTSCALAR => Item is not a scalar
                            ARG__CNVERR => Type conversion error

 *  Support: Jeremy Bailey, {AAO}

 *  Version date: 26-Mar-92
 *-
 */

void ArgPutc(SdsIdType id, ARGCONST char *name, char value, StatusType * ARGCONST status)

{
   SdsIdType tid=0;
   StatusType ignore = STATUS__OK;

   if (*status != SDS__OK) return;
   
/*  Try to find the component  */

   SdsFind(id,name,&tid,status);
   if (*status == SDS__NOITEM)
     {

/*  If not found create the item  */

       *status = SDS__OK;
       SdsNew(id,name,0,NULL,SDS_CHAR,0,NULL,&tid,status);
       SdsPut(tid,sizeof(char),0,&value,status);
     }
   else
       ArgCvt(&value,SDS_CHAR,ARG_SDS,&tid,0,status);

  SdsFreeId(tid,&ignore);
}

/*+				 A r g P u t u s

 *  Function name:
      ArgPutus

 *  Function:
      Put an unsigned short integer item into an argument structure

 *  Description:
      An unsigned short item is written into a named component within the
      specified argument structure. The component is created if it
      does not currently exist.
      
 *  Language:
      C

 *  Declaration:
      void ArgPutus(SdsIdType id, const char *name, unsigned short value, const StatusType * status)
      
 *  Parameters:   (">" input, "!" modified, "W" workspace, "<" output)
      (>) id        (SdsIdType) Identifier to the structure.
      (>) name      (char*) Name of the component to be written to.
      (>) value     (unsigned short) Unsigned short value to be written.
      (!) status    (StatusType*) Modified status. Possible failure codes are:
                            SDS__NOMEM => Insufficient memory for creation
                            SDS__BADID => Invalid Identifier
                            ARG__NOTSCALAR => Item is not a scalar
                            ARG__CNVERR => Type conversion error

 *  Support: Jeremy Bailey, {AAO}

 *  Version date: 27-Mar-92
 *-
 */

void ArgPutus(SdsIdType id, ARGCONST char *name, unsigned short value, StatusType * ARGCONST status)

{
   SdsIdType tid=0;
   StatusType ignore = STATUS__OK;
   
   if (*status != SDS__OK) return;
   
/*  Try to find the component  */

   SdsFind(id,name,&tid,status);
   if (*status == SDS__NOITEM)
     {

/*  If not found create the item  */

       *status = SDS__OK;
       SdsNew(id,name,0,NULL,SDS_USHORT,0,NULL,&tid,status);
       SdsPut(tid,sizeof(unsigned short),0,&value,status);
     }
   else
       ArgCvt(&value,SDS_USHORT,ARG_SDS,&tid,0,status);

  SdsFreeId(tid,&ignore);
}

/*+				 A r g P u t s

 *  Function name:
      ArgPuts

 *  Function:
      Put a short integer item into an argument structure

 *  Description:
      A short integer item is written into a named component within the
      specified argument structure. The component is created if it
      does not currently exist.
      
 *  Language:
      C

 *  Declaration:
      void ArgPuts(SdsIdType id, const char *name, short value, const StatusType * status)
      
 *  Parameters:   (">" input, "!" modified, "W" workspace, "<" output)
      (>) id        (SdsIdType) Identifier to the structure.
      (>) name      (char*) Name of the component to be written to.
      (>) value     (short) Short integer value to be written.
      (!) status    (StatusType*) Modified status. Possible failure codes are:
                            SDS__NOMEM => Insufficient memory for creation
                            SDS__BADID => Invalid Identifier
                            ARG__NOTSCALAR => Item is not a scalar
                            ARG__CNVERR => Type conversion error

 *  Support: Jeremy Bailey, {AAO}

 *  Version date: 27-Mar-92
 *-
 */

void ArgPuts(SdsIdType id, ARGCONST char *name, short value, StatusType * ARGCONST status)

{
   SdsIdType tid=0;
   StatusType ignore = STATUS__OK;

   if (*status != SDS__OK) return;
   
/*  Try to find the component  */

   SdsFind(id,name,&tid,status);
   if (*status == SDS__NOITEM)
     {

/*  If not found create the item  */

       *status = SDS__OK;
       SdsNew(id,name,0,NULL,SDS_SHORT,0,NULL,&tid,status);
       SdsPut(tid,sizeof(short),0,&value,status);
     }
   else
       ArgCvt(&value,SDS_SHORT,ARG_SDS,&tid,0,status);

  SdsFreeId(tid,&ignore);
}

/*+				 A r g P u t i

 *  Function name:
      ArgPuti

 *  Function:
      Put a integer item into an argument structure

 *  Description:
      An integer item is written into a named component within the
      specified argument structure. The component is created if it
      does not currently exist.
      
      Note, when this call has to create the item, it always creates
      32bit integer SDS items, regardless of the size of long int on
      the machine.  Use ArgPuti64 to create 64bit integer items.
            
 *  Language:
      C

 *  Declaration:
      void ArgPuti(SdsIdType id, const char *name, long value, const StatusType * status)
      
 *  Parameters:   (">" input, "!" modified, "W" workspace, "<" output)
      (>) id        (SdsIdType) Identifier to the structure.
      (>) name      (char*) Name of the component to be written to.
      (>) value     (long) Integer value to be written.
      (!) status    (StatusType*) Modified status. Possible failure codes are:
                            SDS__NOMEM => Insufficient memory for creation
                            SDS__BADID => Invalid Identifier
                            ARG__NOTSCALAR => Item is not a scalar
                            ARG__CNVERR => Type conversion error

 *  Support: Jeremy Bailey, {AAO}

 *  Version date: 27-Mar-92
 *-
 */

void ArgPuti(SdsIdType id, ARGCONST char *name, long value, StatusType * ARGCONST status)

{
   SdsIdType tid=0;
   StatusType ignore = STATUS__OK;

   if (*status != SDS__OK) return;

/*  Try to find the component  */

   SdsFind(id,name,&tid,status);
   if (*status == SDS__NOITEM)
     {

/*  If not found create the item  */

       INT32 val = value;
       *status = SDS__OK;
       SdsNew(id,name,0,NULL,SDS_INT,0,NULL,&tid,status);
       SdsPut(tid,sizeof(val),0,&val,status);
     }
   else
   {
       /*
        * If long is 64 bit, then LONG__64 marco has been defined by
        * SDS and we should use the SDS type SDS_I64 as our source type.
        */
#ifdef LONG__64
       int type = SDS_I64;
#else 
       int type = SDS_INT;
#endif
       ArgCvt(&value,type,ARG_SDS,&tid,0,status);
   }

   SdsFreeId(tid,&ignore);
}


/*+				 A r g P u t u

 *  Function name:
      ArgPutu

 *  Function:
      Put an unsigned integer item into an argument structure

 *  Description:
      An unsigned integer item is written into a named component within the
      specified argument structure. The component is created if it
      does not currently exist.

      Note, when this call has to create the item, it always creates
      32bit unsigned integer SDS items, regardless of the size of long int on
      the machine.  Use ArgPutui64 to create 64bit unsigned integer items.
       
 *  Language:
      C

 *  Declaration:
      void ArgPutu(SdsIdType id, const char *name, unsigned long value, const StatusType * status)
      
 *  Parameters:   (">" input, "!" modified, "W" workspace, "<" output)
      (>) id        (SdsIdType) Identifier to the structure.
      (>) name      (char*) Name of the component to be written to.
      (>) value     (unsigned long) Unsigned integer value to be written.
      (!) status    (StatusType*) Modified status. Possible failure codes are:
                            SDS__NOMEM => Insufficient memory for creation
                            SDS__BADID => Invalid Identifier
                            ARG__NOTSCALAR => Item is not a scalar
                            ARG__CNVERR => Type conversion error

 *  Support: Jeremy Bailey, {AAO}

 *  Version date: 27-Mar-92
 *-
 */

void ArgPutu(SdsIdType id, ARGCONST char *name, unsigned long value, StatusType * ARGCONST status)

{
   SdsIdType tid=0;
   StatusType ignore = STATUS__OK;


   if (*status != SDS__OK) return;
   
/*  Try to find the component  */

   SdsFind(id,name,&tid,status);
   if (*status == SDS__NOITEM)
     {

/*  If not found create the item  */
	UINT32 val = value;

       *status = SDS__OK;
       
       SdsNew(id,name,0,NULL,SDS_UINT,0,NULL,&tid,status);
       SdsPut(tid,sizeof(val),0,&val,status);
     }
   else
   {
      /*
        * If long is 64 bit, then LONG__64 marco has been defined by
        * SDS and we should use the SDS type SDS_U64 as our source type.
        */
#ifdef LONG__64
       int type = SDS_UI64;
#else 
       int type = SDS_UINT;
#endif
       ArgCvt(&value,type,ARG_SDS,&tid,0,status);
   }

  SdsFreeId(tid,&ignore);
}
/*+				 A r g P u t i 6 4

 *  Function name:
      ArgPuti64

 *  Function:
      Put a 64 bit integer item into an argument structure

 *  Description:
      A 64 bit integer item is written into a named component within the
      specified argument structure. The component is created if it
      does not currently exist.
      
 *  Language:
      C

 *  Declaration:
      void ArgPuti64(SdsIdType id, const char *name, INT64 value, const StatusType * status)
      
 *  Parameters:   (">" input, "!" modified, "W" workspace, "<" output)
      (>) id        (SdsIdType) Identifier to the structure.
      (>) name      (char*) Name of the component to be written to.
      (>) value     (INT64) 64 bit integer value to be written.
      (!) status    (StatusType*) Modified status. Possible failure codes are:
                            SDS__NOMEM => Insufficient memory for creation
                            SDS__BADID => Invalid Identifier
                            ARG__NOTSCALAR => Item is not a scalar
                            ARG__CNVERR => Type conversion error

 *  Support: Tony Farrell, {AAO}

 *  Version date: 10-Aug-95
 *-
 */

void ArgPuti64(SdsIdType id, ARGCONST char *name, INT64 value, StatusType * ARGCONST status)

{
   SdsIdType tid=0;
   StatusType ignore = STATUS__OK;

   if (*status != SDS__OK) return;

/*  Try to find the component  */

   SdsFind(id,name,&tid,status);
   if (*status == SDS__NOITEM)
     {

/*  If not found create the item  */

       *status = SDS__OK;
       SdsNew(id,name,0,NULL,SDS_I64,0,NULL,&tid,status);
       SdsPut(tid,sizeof(value),0,&value,status);
     }
   else
       ArgCvt(&value,SDS_I64,ARG_SDS,&tid,0,status);

  SdsFreeId(tid,&ignore);
}


/*+				 A r g P u t u 6 4

 *  Function name:
      ArgPutu64

 *  Function:
      Put an unsigned 64 bit integer item into an argument structure

 *  Description:
      An unsigned 64 bit integer item is written into a named component 
      within the specified argument structure. The component is created if it
      does not currently exist.

 *  Language:
      C

 *  Declaration:
      void ArgPutu64(SdsIdType id, const char *name, unsigned long value, const StatusType * status)
      
 *  Parameters:   (">" input, "!" modified, "W" workspace, "<" output)
      (>) id        (SdsIdType) Identifier to the structure.
      (>) name      (char*) Name of the component to be written to.
      (>) value     (UINT64) Unsigned integer value to be written.
      (!) status    (StatusType*) Modified status. Possible failure codes are:
                            SDS__NOMEM => Insufficient memory for creation
                            SDS__BADID => Invalid Identifier
                            ARG__NOTSCALAR => Item is not a scalar
                            ARG__CNVERR => Type conversion error

 *  Support: Tony Farrell, {AAO}

 *  Version date: 10-Aug-95
 *-
 */

void ArgPutu64(SdsIdType id, ARGCONST char *name, UINT64 value, StatusType * ARGCONST status)

{
   SdsIdType tid=0;
   StatusType ignore = STATUS__OK;

   if (*status != SDS__OK) return;
   
/*  Try to find the component  */

   SdsFind(id,name,&tid,status);
   if (*status == SDS__NOITEM)
     {

/*  If not found create the item  */
       *status = SDS__OK;
       
       SdsNew(id,name,0,NULL,SDS_UI64,0,NULL,&tid,status);
       SdsPut(tid,sizeof(value),0,&value,status);
     }
   else
       ArgCvt(&value,SDS_UI64,ARG_SDS,&tid,0,status);

  SdsFreeId(tid,&ignore);
}

/*+				 A r g P u t f

 *  Function name:
      ArgPutf

 *  Function:
      Put a floating point item into an argument structure

 *  Description:
      A floating point item is written into a named component within the
      specified argument structure. The component is created if it
      does not currently exist.
      
 *  Language:
      C

 *  Declaration:
      void ArgPutf(SdsIdType id, const char *name, float value, const StatusType * status)
      
 *  Parameters:   (">" input, "!" modified, "W" workspace, "<" output)
      (>) id        (SdsIdType) Identifier to the structure.
      (>) name      (char*) Name of the component to be written to.
      (>) value     (float) Floating point value to be written.
      (!) status    (StatusType*) Modified status. Possible failure codes are:
                            SDS__NOMEM => Insufficient memory for creation
                            SDS__BADID => Invalid Identifier
                            ARG__NOTSCALAR => Item is not a scalar
                            ARG__CNVERR => Type conversion error

 *  Support: Jeremy Bailey, {AAO}

 *  Version date: 27-Mar-92
 *-
 */

void ArgPutf(SdsIdType id, ARGCONST char *name, float value, StatusType * ARGCONST status)

{
   SdsIdType tid=0;
   StatusType ignore = STATUS__OK;

   if (*status != SDS__OK) return;
   
/*  Try to find the component  */

   SdsFind(id,name,&tid,status);
   if (*status == SDS__NOITEM)
     {

/*  If not found create the item  */

       *status = SDS__OK;
       SdsNew(id,name,0,NULL,SDS_FLOAT,0,NULL,&tid,status);
       SdsPut(tid,sizeof(float),0,&value,status);
     }
   else
       ArgCvt(&value,SDS_FLOAT,ARG_SDS,&tid,0,status);

  SdsFreeId(tid,&ignore);
}


/*+				 A r g P u t d

 *  Function name:
      ArgPutd

 *  Function:
      Put a double floating point item into an argument structure

 *  Description:
      A double item is written into a named component within the
      specified argument structure. The component is created if it
      does not currently exist.
      
 *  Language:
      C

 *  Declaration:
      void ArgPutd(SdsIdType id, const char *name, double value, const StatusType * status)
      
 *  Parameters:   (">" input, "!" modified, "W" workspace, "<" output)
      (>) id        (SdsIdType) Identifier to the structure.
      (>) name      (char*) Name of the component to be written to.
      (>) value     (double) Floating point value to be written.
      (!) status    (StatusType*) Modified status. Possible failure codes are:
                            SDS__NOMEM => Insufficient memory for creation
                            SDS__BADID => Invalid Identifier
                            ARG__NOTSCALAR => Item is not a scalar
                            ARG__CNVERR => Type conversion error

 *  Support: Jeremy Bailey, {AAO}

 *  Version date: 27-Mar-92
 *-
 */

void ArgPutd(SdsIdType id, ARGCONST char *name, double value, StatusType * ARGCONST status)

{
   SdsIdType tid=0;
   StatusType ignore = STATUS__OK;

   if (*status != SDS__OK) return;
   
/*  Try to find the component  */

   SdsFind(id,name,&tid,status);
   if (*status == SDS__NOITEM)
     {

/*  If not found create the item  */

       *status = SDS__OK;
       SdsNew(id,name,0,NULL,SDS_DOUBLE,0,NULL,&tid,status);
       SdsPut(tid,sizeof(double),0,&value,status);
     }
   else
       ArgCvt(&value,SDS_DOUBLE,ARG_SDS,&tid,0,status);

  SdsFreeId(tid,&ignore);
}


/*+			   A r g P u t S t r i n g

 *  Function name:
      ArgPutString

 *  Function:
      Put a character string item into an argument structure

 *  Description:
      A character string item is written into a named component within the
      specified argument structure. The component is created if it
      does not currently exist.
      
 *  Language:
      C

 *  Declaration:
      void ArgPutString(SdsIdType id, char *name, char *value, const StatusType * status)
      
 *  Parameters:   (">" input, "!" modified, "W" workspace, "<" output)
      (>) id        (SdsIdType) Identifier to the structure.
      (>) name      (char*) Name of the component to be written to.
      (>) value     (char*) Null terminated string to be written.
      (!) status    (StatusType*) Modified status. Possible failure codes are:
                            SDS__NOMEM => Insufficient memory for creation
                            SDS__BADID => Invalid Identifier
                            ARG__NOTSTRING => Item is not a string

 *  Support: Jeremy Bailey, {AAO}

 *  Version date: 28-Mar-92
 *-
 */

void ArgPutString(SdsIdType id, ARGCONST char *name, 
			ARGCONST char *value, StatusType * ARGCONST status)

{
   SdsIdType tid=0;
   StatusType ignore = STATUS__OK;

   if (*status != SDS__OK) return;
   
/*  Try to find the component  */

   SdsFind(id,name,&tid,status);
   if (*status == SDS__NOITEM)
     {

/*  If not found create the item  */

       unsigned long dims[SDS_C_MAXARRAYDIMS];
       *status = SDS__OK;
       dims[0] = strlen(value)+1;
       SdsNew(id,name,0,NULL,SDS_CHAR,1,dims,&tid,status);
       SdsPut(tid,dims[0],0,value,status);
     }
   else
       ArgCvt(value,ARG_STRING,ARG_SDS,&tid,0,status);
  SdsFreeId(tid,&ignore);
}



/*+				 A r g G e t c

 *  Function name:
      ArgGetc

 *  Function:
      Get a character item from an argument structure

 *  Description:
      A character item is read from a named component within the
      specified argument structure. The item is converted to character
      type if necessary. 
      
 *  Language:
      C

 *  Declaration:
      void ArgGetc(SdsIdType id, const char *name, char *value, const StatusType * status)
      
 *  Parameters:   (">" input, "!" modified, "W" workspace, "<" output)
      (>) id        (SdsIdType) Identifier to the structure.
      (>) name      (char*) Name of the component to be read from.
      (<) value     (char*)  Character variable to read into.
      (!) status    (StatusType*) Modified status. Possible failure codes are:
                            SDS__NOMEM => Insufficient memory for creation
                            SDS__BADID => Invalid Identifier
                            ARG__NOTSCALAR => Item is not a scalar
                            ARG__CNVERR => Type conversion error

 *  Support: Jeremy Bailey, {AAO}

 *  Version date: 27-Mar-92
 *-
 */

void ArgGetc(SdsIdType id, ARGCONST char *name, char *value, StatusType * ARGCONST status)

{
   SdsIdType tid=0;
   StatusType ignore = STATUS__OK;

   if (*status != SDS__OK) return;
   
/*  Try to find the component  */

   SdsFind(id,name,&tid,status);
   ArgCvt(&tid,ARG_SDS,SDS_CHAR,value,sizeof(*value),status);
   SdsFreeId(tid,&ignore);
}

/*+				 A r g G e t s

 *  Function name:
      ArgGets

 *  Function:
      Get a short integer item from an argument structure

 *  Description:
      A short integer item is read from a named component within the
      specified argument structure. The item is converted to short
      type if necessary. 
      
 *  Language:
      C

 *  Declaration:
      void ArgGets(SdsIdType id, const char *name, short *value, const StatusType * status)
      
 *  Parameters:   (">" input, "!" modified, "W" workspace, "<" output)
      (>) id        (SdsIdType) Identifier to the structure.
      (>) name      (char*) Name of the component to be read from.
      (<) value     (short*)  Short variable to read into.
      (!) status    (StatusType*) Modified status. Possible failure codes are:
                            SDS__NOMEM => Insufficient memory for creation
                            SDS__BADID => Invalid Identifier
                            ARG__NOTSCALAR => Item is not a scalar
                            ARG__CNVERR => Type conversion error

 *  Support: Jeremy Bailey, {AAO}

 *  Version date: 27-Mar-92
 *-
 */

void ArgGets(SdsIdType id, ARGCONST char *name, short *value, StatusType * ARGCONST status)

{
   SdsIdType tid=0;
   StatusType ignore = STATUS__OK;

   if (*status != SDS__OK) return;
   
/*  Try to find the component  */

   SdsFind(id,name,&tid,status);
   ArgCvt(&tid,ARG_SDS,SDS_SHORT,value,sizeof(*value),status);
   SdsFreeId(tid,&ignore);
}

/*+				 A r g G e t u s

 *  Function name:
      ArgGetus

 *  Function:
      Get an unsigned short integer item from an argument structure

 *  Description:
      An unsigned short integer item is read from a named component within the
      specified argument structure. The item is converted to unsigned short
      type if necessary. 
      
 *  Language:
      C

 *  Declaration:
      void ArgGetus(SdsIdType id, const char *name, unsigned short *value, const StatusType * status)
      
 *  Parameters:   (">" input, "!" modified, "W" workspace, "<" output)
      (>) id        (SdsIdType) Identifier to the structure.
      (>) name      (char*) Name of the component to be read from.
      (<) value     (unsigned short*)  Short variable to read into.
      (!) status    (StatusType*) Modified status. Possible failure codes are:
                            SDS__NOMEM => Insufficient memory for creation
                            SDS__BADID => Invalid Identifier
                            ARG__NOTSCALAR => Item is not a scalar
                            ARG__CNVERR => Type conversion error

 *  Support: Jeremy Bailey, {AAO}

 *  Version date: 27-Mar-92
 *-
 */

void ArgGetus(SdsIdType id, ARGCONST char *name, unsigned short *value, StatusType * ARGCONST status)

{
   SdsIdType tid=0;
   StatusType ignore = STATUS__OK;

   if (*status != SDS__OK) return;
   
/*  Try to find the component  */

   SdsFind(id,name,&tid,status);
   ArgCvt(&tid,ARG_SDS,SDS_USHORT,value,sizeof(*value),status);
   SdsFreeId(tid,&ignore);
}



/*+				 A r g G e t i

 *  Function name:
      ArgGeti

 *  Function:
      Get an integer item from an argument structure

 *  Description:
      A long integer integer item is read from a named component within the
      specified argument structure. The item is converted to long integer
      type if necessary. 
      
 *  Language:
      C

 *  Declaration:
      void ArgGeti(SdsIdType id, const char *name, long *value, const StatusType * status)
      
 *  Parameters:   (">" input, "!" modified, "W" workspace, "<" output)
      (>) id        (SdsIdType) Identifier to the structure.
      (>) name      (char*) Name of the component to be read from.
      (<) value     (long*) Long variable to read into.
      (!) status    (StatusType*) Modified status. Possible failure codes are:
                            SDS__NOMEM => Insufficient memory for creation
                            SDS__BADID => Invalid Identifier
                            ARG__NOTSCALAR => Item is not a scalar
                            ARG__CNVERR => Type conversion error

 *  Support: Jeremy Bailey, {AAO}

 *  Version date: 27-Mar-92
 *-
 */

void ArgGeti(SdsIdType id, ARGCONST char *name, long *value, StatusType * ARGCONST status)

{
    /*
     * If long is 64 bit, then LONG__64 marco has been defined by
     * SDS and we should use the SDS type SDS_I64 as our source type.
     */
#ifdef LONG__64
    int type = SDS_I64;
#else 
    int type = SDS_INT;
#endif

   SdsIdType tid=0;
   StatusType ignore = STATUS__OK;

   if (*status != SDS__OK) return;
   
/*  Try to find the component  */

   SdsFind(id,name,&tid,status);
   ArgCvt(&tid,ARG_SDS,type,value,sizeof(*value),status);
   SdsFreeId(tid,&ignore);
}


/*+				 A r g G e t u

 *  Function name:
      ArgGetu

 *  Function:
      Get an unsigned integer item from an argument structure

 *  Description:
      An unsigned long integer item is read from a named component within the
      specified argument structure. The item is converted to unsigned long 
      type if necessary. 
      
 *  Language:
      C

 *  Declaration:
      void ArgGetu(SdsIdType id, const char *name, unsigned long *value, const StatusType * status)
      
 *  Parameters:   (">" input, "!" modified, "W" workspace, "<" output)
      (>) id        (SdsIdType) Identifier to the structure.
      (>) name      (char*) Name of the component to be read from.
      (<) value     (unsigned long*) Unsigned long variable to read into.
      (!) status    (StatusType*) Modified status. Possible failure codes are:
                            SDS__NOMEM => Insufficient memory for creation
                            SDS__BADID => Invalid Identifier
                            ARG__NOTSCALAR => Item is not a scalar
                            ARG__CNVERR => Type conversion error

 *  Support: Jeremy Bailey, {AAO}

 *  Version date: 27-Mar-92
 *-
 */

void ArgGetu(SdsIdType id, ARGCONST char *name, unsigned long *value, StatusType * ARGCONST status)

{
    /*
     * If long is 64 bit, then LONG__64 marco has been defined by
     * SDS and we should use the SDS type SDS_UI64 as our source type.
     */
#ifdef LONG__64
    int type = SDS_UI64;
#else 
    int type = SDS_UINT;
#endif
   SdsIdType tid=0;
   StatusType ignore = STATUS__OK;
   
   if (*status != SDS__OK) return;
   
/*  Try to find the component  */

   SdsFind(id,name,&tid,status);
   ArgCvt(&tid,ARG_SDS,type,value,sizeof(*value),status);
   SdsFreeId(tid,&ignore);

}
/*+				 A r g G e t i 6 4

 *  Function name:
      ArgGeti64

 *  Function:
      Get a 64 bit integer item from an argument structure

 *  Description:
      A 64 bit integer integer item is read from a named component within the
      specified argument structure. The item is converted to 64 bit integer
      type if necessary. 
      
 *  Language:
      C

 *  Declaration:
      void ArgGeti64(SdsIdType id, const char *name, INT64 *value, const StatusType * status)
      
 *  Parameters:   (">" input, "!" modified, "W" workspace, "<" output)
      (>) id        (SdsIdType) Identifier to the structure.
      (>) name      (char*) Name of the component to be read from.
      (<) value     (INT64*) Long variable to read into.
      (!) status    (StatusType*) Modified status. Possible failure codes are:
                            SDS__NOMEM => Insufficient memory for creation
                            SDS__BADID => Invalid Identifier
                            ARG__NOTSCALAR => Item is not a scalar
                            ARG__CNVERR => Type conversion error

 *  Support: Jeremy Bailey, {AAO}

 *  Version date: 27-Mar-92
 *-
 */

void ArgGeti64(SdsIdType id, ARGCONST char *name, INT64 *value, StatusType * ARGCONST status)

{
   SdsIdType tid=0;
   StatusType ignore = STATUS__OK;

   if (*status != SDS__OK) return;
   
/*  Try to find the component  */

   SdsFind(id,name,&tid,status);
   ArgCvt(&tid,ARG_SDS,SDS_I64,value,sizeof(*value),status);
   SdsFreeId(tid,&ignore);
}


/*+				 A r g G e t u 6 4

 *  Function name:
      ArgGetu64

 *  Function:
      Get an unsigned 64bit integer item from an argument structure

 *  Description:
      An unsigned 64bit integer item is read from a named component within the
      specified argument structure. The item is converted to unsigned 64bit
      type if necessary. 
      
 *  Language:
      C

 *  Declaration:
      void ArgGetu64(SdsIdType id, const char *name, UINT64 *value, const StatusType * status)
      
 *  Parameters:   (">" input, "!" modified, "W" workspace, "<" output)
      (>) id        (SdsIdType) Identifier to the structure.
      (>) name      (char*) Name of the component to be read from.
      (<) value     (UINT64*) Unsigned 64bit variable to read into.
      (!) status    (StatusType*) Modified status. Possible failure codes are:
                            SDS__NOMEM => Insufficient memory for creation
                            SDS__BADID => Invalid Identifier
                            ARG__NOTSCALAR => Item is not a scalar
                            ARG__CNVERR => Type conversion error

 *  Support: Tony Farrell, {AAO}

 *  Version date: 10-Aug-1995
 *-
 */

void ArgGetu64(SdsIdType id, ARGCONST char *name, UINT64 *value, StatusType * ARGCONST status)

{
   SdsIdType tid=0;
   StatusType ignore = STATUS__OK;
   
   if (*status != SDS__OK) return;
   
/*  Try to find the component  */

   SdsFind(id,name,&tid,status);
   ArgCvt(&tid,ARG_SDS,SDS_UI64,value,sizeof(*value),status);
   SdsFreeId(tid,&ignore);

}

/*+				 A r g G e t f

 *  Function name:
      ArgGetf

 *  Function:
      Get a floating point item from an argument structure

 *  Description:
      A floating point item is read from a named component within the
      specified argument structure. The item is converted to float 
      type if necessary. 
      
 *  Language:
      C

 *  Declaration:
      void ArgGetf(SdsIdType id, const char *name, float *value, const StatusType *  status)
      
 *  Parameters:   (">" input, "!" modified, "W" workspace, "<" output)
      (>) id        (SdsIdType) Identifier to the structure.
      (>) name      (char*) Name of the component to be read from.
      (<) value     (float*) Float variable to read into.
      (!) status    (StatusType*) Modified status. Possible failure codes are:
                            SDS__NOMEM => Insufficient memory for creation
                            SDS__BADID => Invalid Identifier
                            ARG__NOTSCALAR => Item is not a scalar
                            ARG__CNVERR => Type conversion error

 *  Support: Jeremy Bailey, {AAO}

 *  Version date: 27-Mar-92
 *-
 */

void ArgGetf(SdsIdType id, ARGCONST char *name, float *value, StatusType * ARGCONST status)

{
   SdsIdType tid=0;
   StatusType ignore = STATUS__OK;

   if (*status != SDS__OK) return;
   
/*  Try to find the component  */

   SdsFind(id,name,&tid,status);
   ArgCvt(&tid,ARG_SDS,SDS_FLOAT,value,sizeof(*value),status);
   SdsFreeId(tid,&ignore);
}



/*+				 A r g G e t d

 *  Function name:
      ArgGetd

 *  Function:
      Get a double floating point item from an argument structure

 *  Description:
      A double floating point item is read from a named component within the
      specified argument structure. The item is converted to double 
      type if necessary. 
      
 *  Language:
      C

 *  Declaration:
      void ArgGetd(SdsIdType id, const char *name, double *value, const StatusType * status)
      
 *  Parameters:   (">" input, "!" modified, "W" workspace, "<" output)
      (>) id        (SdsIdType) Identifier to the structure.
      (>) name      (char*) Name of the component to be read from.
      (<) value     (double*) Double variable to read into.
      (!) status    (StatusType*) Modified status. Possible failure codes are:
                            SDS__NOMEM => Insufficient memory for creation
                            SDS__BADID => Invalid Identifier
                            ARG__NOTSCALAR => Item is not a scalar
                            ARG__CNVERR => Type conversion error

 *  Support: Jeremy Bailey, {AAO}

 *  Version date: 27-Mar-92
 *-
 */

void ArgGetd(SdsIdType id, ARGCONST char *name, double *value, StatusType * ARGCONST status)

{
   SdsIdType tid=0;
   StatusType ignore = STATUS__OK;

   if (*status != SDS__OK) return;
   
/*  Try to find the component  */

   SdsFind(id,name,&tid,status);
   ArgCvt(&tid,ARG_SDS,SDS_DOUBLE,value,sizeof(*value),status);
   SdsFreeId(tid,&ignore);

}
/*+		           A r g G e t S t r i n g

 *  Function name:
      ArgGetString

 *  Function:
      Get a character string item from an argument structure

 *  Description:
      A character string item is read from a named component within the
      specified argument structure. 
      
 *  Language:
      C

 *  Declaration:
      void ArgGetString(SdsIdType id, const char *name, long len,char *value, const StatusType *  status)
      
 *  Parameters:   (">" input, "!" modified, "W" workspace, "<" output)
      (>) id        (SdsIdType) Identifier to the structure.
      (>) name      (char*) Name of the component to be read from.
      (>) len       (long)  Length of buffer to receive string.
      (<) value     (char*) Character string buffer to read into.
      (!) status    (StatusType*) Modified status. Possible failure codes are:
                            SDS__NOMEM => Insufficient memory for creation
                            SDS__BADID => Invalid Identifier
                            ARG__NOTSTRING => Item is not a string

 *  Support: Jeremy Bailey, {AAO}

 *  Version date: 28-Mar-92
 *-
 */

void ArgGetString(SdsIdType id, ARGCONST char *name, long len, char *value, StatusType * ARGCONST status)

{
   SdsIdType tid=0;
   StatusType ignore = STATUS__OK;
 
   if (*status != SDS__OK) return;
   
/*  Try to find the component  */

   SdsFind(id,name,&tid,status);
   ArgCvt(&tid,ARG_SDS,ARG_STRING,value,len,status);
   SdsFreeId(tid,&ignore);
}


/*+		              A r g D e l e t e

 *  Function name:
      ArgDelete

 *  Function:
      Delete an argument structure

 *  Description:
      Delete an argument structure and free its identifier
      
 *  Language:
      C

 *  Declaration:
      void ArgDelete(SdsIdType id, const StatusType * status)
      
 *  Parameters:   (">" input, "!" modified, "W" workspace, "<" output)
      (>) id        (SdsIdType) Identifier to the structure to be deleted.
      (!) status    (StatusType*) Modified status. Possible failure codes are:
                            SDS__BADID => Invalid Identifier

 *  Support: Jeremy Bailey, {AAO}

 *  Version date: 7-Apr-92
 *-
 */

void ArgDelete(SdsIdType id, StatusType * ARGCONST status)

{
   SdsDelete(id,status);
   SdsFreeId(id,status);
}

/*
 *+			A r g T o S t r i n g

 *  Function name:
      ArgToString

 *  Function:
	Take an Sds structure and write it to a string.

 *  Description:
	The structure is examined recursively.  It scalar or string
	item found is written to the output string, with a space
	separating each item.  No structure is maintained to the data - each
        item is converted to a string and appended to the output string.

	The normal use of the function is to convert what is expected to
	be simple structures into something suitable for output to the
	user.
      

 *  Language:
      C

 *  Call:
      (Void) = ArgToString (id,maxlen,length,string,status)

 *  Parameters:   (">" input, "!" modified, "W" workspace, "<" output)
      (>) id        (SdsIdType) Identifier to the structure to be deleted.
      (>) maxlen	(Int)   Length of buffer to receive string.
      (!) length	(Int *)	Set to zero on entry.  On exit, will
			    contain the actual length of the string.
      (<) string	(Char *) Character string buffer to write to.
      (!) status	(Long int *) Modified status. 

 *  Include files: Arg.h

 *  External functions used:

 *  External values used:
      none

 *  Prior requirements:
      none

 *  Support: Tony Farrell, AAO

 *-

 *  History:
      28-Jan-1993 - TJF - Original version
      17-Apr-1997 - TJF - Can now handle arrays (both scaller and structure).
      04-Nov-2003 - TJF - Conversion of char array type was not correct
                          at the end of the string.  Fixed.
      11-Nov-2011 - TJF - tname should be SDS_C_NAMELEN characters long rather
                          then 15 (which is wrong).
      25-Jul-2017 - TJF - Was not null terminating the result in all cases, not sure
                            why I was getting away with this, picked up by
                            Google Sanitiser tools
      05-Jul-2018 - TJF - Was not outputting 64 bit negative items
                            correctly when part of an array.  Rework
                            the related code using the new
                            SdsGetI64toD() and  SdsGetUI64toD() and
                            providing proper support if we have native
                            64 bit integers.



         
      {@change entry@}
 */


void ArgToString (SdsIdType id,int maxlen,int *length,
		char *string, StatusType * ARGCONST status)

{
    char tname[SDS_C_NAMELEN];		/* temp name		 */	
    SdsCodeType tcode;		/* temp sds code	 */
    long ndims;			/* number of dimensions	 */
    unsigned long dims[SDS_C_MAXARRAYDIMS];	/* The dimensions	 */
    unsigned long actlen = 0;	/* Actual length of item */


    if (*status != SDS__OK) return;

/*
 *  If the buffer is already full, return.
 */

    if (*length >= maxlen) 
    {
	*length = maxlen;
	return;
    }
/*
 *  Find out item type.
 */
    SdsInfo(id,tname,&tcode,&ndims,dims,status);
    if (*status != SDS__OK) return;

/*
 *  If the item is a structure, recursively output it.
 */
    if ((tcode == SDS_STRUCT)&&(ndims == 0))
    {
	int ind = 1;
	while (*status == SDS__OK)
	{
/*
 *	    Index to the next item.
 */
	    SdsIdType nid = 0;
	    StatusType ignore = STATUS__OK;


	    SdsIndex(id,ind,&nid,status);

            if ((*status == STATUS__OK)&&((*length) >= maxlen))
            {
                /*
                 * We have no space for any more.
                 */
                string[maxlen-1] = 0;
                *length=maxlen;
                *status = ARG__CNVERR;
                ErsRep(0,status,
                       "ArgToString error converting to string, destination length too small (struct)");
            }

/*
 *	    Convert it.
 */
	    ArgToString(nid,maxlen,length,string,status);
/*
 *	    If everything ok, add a space if there is room.  If there is
 *          no room but we have more items, then the next one will fail.
 */
	    if (*status == SDS__OK)
	    {
	        ind++;
	        if (*length < maxlen)   /* Put a space between each one */
	    	    string[(*length)++] = ' ';
            }
            SdsFreeId(nid,&ignore);
		
	}
/*
 *	We only know we have examined all items when status is set to i
 *      SDS__NOITEM, so clear status, assuming we found at least one item.
 */
	if ((ind > 1)&&(*status == SDS__NOITEM))
	    *status = SDS__OK;
        if (*status == STATUS__OK)
            string[*length] = '\0';
    }
/*
 *  As a special case, handle character strings (1 dimensional character arrays)
 *  These are written as quoted strings.
 */
    else if ((ndims == 1)&&(tcode == SDS_CHAR))
    {
	int l = dims[0] ;               /* Length of item   */
        int overflow = 0;
	if (((*length)+l) > maxlen)     /* Note, *length is < maxlen, see above */
        {
            ++overflow;
	    l = maxlen - *length;       /* What we have space for */
        }
	    
	SdsGet(id,l,0,&string[*length],&actlen,status);
	*length += l;                   /* Total length */

        if (overflow)
        {
            /*
             * We overflowed - null terminate what we fetched, set error 
             * and return.
             */
            string[maxlen-1] = 0;
            *length=maxlen;
            *status = ARG__CNVERR;
            ErsRep(0,status,
           "ArgToString error converting to string, destination length too small (char array 1)");
        }
        else if (string[(*length)-1] == '\0')
            /* Last character in buffer is null, we can step back one */
	    --(*length);
        else if (*length < maxlen)
            /* Have space for null terminator - ensure we have it  */
            string[*length] = '\0';
        else 
        {
            /*
             * We don't have space for null terminator - consider this an
             * overflow.
             */
            string[maxlen-1] = 0;
            *length=maxlen;
            *status = ARG__CNVERR;
            ErsRep(0,status,
           "ArgToString error converting to string, destination length too small (char array 2)");

        }
    }
/*
 *  Structure arrays.
 */
    else if (tcode == SDS_STRUCT)
    {
	unsigned long tdims[SDS_C_MAXARRAYDIMS];    	
        tdims[0] = 1;
    	while (*status == STATUS__OK)
    	{
            SdsIdType tid;
    	    SdsCell(id,1,tdims,&tid,status);
/*
 *	    Convert it.
 */
	    ArgToString(tid,maxlen,length,string,status);
/*
 *	    If everything ok, add a space.
 */
	    if (*status == SDS__OK)
	    {
	        if (*length < maxlen)   /* Put a space between each one */
	    	    string[(*length)++] = ' ';
            }
 	    SdsFreeId(tid,status);
    	    ++tdims[0];
    	}
    	if (*status == SDS__INDEXERR) *status = STATUS__OK;
        if (*status == STATUS__OK)
            string[*length] = '\0';
    }
/*
 *  Scaler arrays.
 */
    else if (ndims != 0) 
    {
        long ncomps = 1;
        register unsigned i;
        for (i = 0 ; i <  (unsigned)ndims ; ++i)
            ncomps = ncomps * dims[i];
	if (tcode == SDS_CHAR)
	{
	    /* Can get character array data directly */
	    unsigned long actlen;
	    long space = maxlen-*length - 1;
	    if (space < ncomps) ncomps = space;
	    SdsGet(id,ncomps,0,(void *)&string[*length],&actlen,status);
	    string[*length+actlen] = '\0';
	    *length += strlen(&string[*length]);
	}
	else
	{
	    long offset;
	    int fmtResult = 1;
	    long space = maxlen-*length - 1;
	    for (offset = 0; (offset < ncomps)&&(space > 0)&&(fmtResult > 0) 
	    				; offset++)
	    {
	        switch (tcode)
	        {
	            case SDS_UBYTE:
	            {
	                 unsigned char data;
	                 SdsGet(id,sizeof(data),offset,(void *)&data,&actlen,status);
	                 fmtResult = ErsSPrintf(space,&string[*length],"%u ",
	                 						data);
	                 break;
	            }
	            case SDS_BYTE:
	            {
	                 char data;
	                 SdsGet(id,sizeof(data),offset,(void *)&data,&actlen,status);
	                 fmtResult = ErsSPrintf(space,&string[*length],"%d ",data);
	                 break;
	            }
	            case SDS_USHORT:
	            {
	                 unsigned short data;
	                 SdsGet(id,sizeof(data),offset,(void *)&data,&actlen,status);
	                 fmtResult = ErsSPrintf(space,&string[*length],"%u ",data);
	                 break;
	            }
	            case SDS_SHORT:
	            {
	                 short data;
	                 SdsGet(id,sizeof(data),offset,(void *)&data,&actlen,status);
	                 fmtResult = ErsSPrintf(space,&string[*length],"%d ",data);
	                 break;
	            }
	            case SDS_UINT:
	            {
	                 UINT32 data;
	                 SdsGet(id,sizeof(data),offset,(void *)&data,&actlen,status);
	                 fmtResult = ErsSPrintf(space,&string[*length],"%lu ",
	                 		(unsigned long)data);
	                 break;
	            }
	            case SDS_INT:
	            {
	                 INT32 data;
	                 SdsGet(id,sizeof(data),offset,(void *)&data,&actlen,status);
	                 fmtResult = ErsSPrintf(space,&string[*length],"%ld ",(long)data);
	                 break;
	            }
	            case SDS_UI64:
	            {
	                 UINT64 data;
                         char s[ARG_INT_STRLEN] DUNUSED; /* Unused
                                                          * unless
                                                          * second
                                                          * item here
                                                          * used */
	                 SdsGet(id,sizeof(data),offset,(void *)&data,&actlen,status);
#ifdef LONG_64
                         /* Long is 64 bits - can do this directly */
	                 fmtResult = ErsSPrintf(space,&string[*length],"%lu ",data);
#elif defined(NATIVE__INT64)
                         /*  We are on a machine with explict 64 bit
                          *  INT64's but long is not 64 bit.  Safe to use 
                          * them directly  - through we go through the
                          *  ArgUIntToString() functions to ensure we
                          *  get the  format right
                          */
                         fmtResult = ErsSPrintf(space,&string[*length],
                                                "%s ",
                                                ArgUIntToString(data,s));
#else
                         /*
                          * No 64 bit integer available, must convert
                          * to a double and format that.
                          */
	                 fmtResult = ErsSPrintf(space,&string[*length],"%.15g ",
                                                SdsGetUI64toD(data));
#endif
	                 break;
	            }
	            case SDS_I64:
	            {
                         INT64 data;
                         char s[ARG_INT_STRLEN] DUNUSED; /* Unused
                                                          * unless
                                                          * second
                                                          * item here
                                                          * used */
	                 SdsGet(id,sizeof(data),offset,(void *)&data,&actlen,status);
#ifdef LONG_64
                         /* Long is 64 bits - can do this directly */
	                 fmtResult = ErsSPrintf(space,&string[*length],"%ld ",
                                                data);
#elif defined(NATIVE__INT64)
                         /*  We are on a machine with explict 64 bit
                          *  INT64's but long is not 64 bit.  Safe to use 
                          * them directly  - through we go through the
                          *  ArgIntToString() functions to ensure we
                          *  get the  format right
                          */
                         fmtResult = ErsSPrintf(space,&string[*length],
                                                "%s ",
                                                ArgIntToString(data,s));
#else
                         /*
                          * No 64 bit integer available, must convert
                          * to a double and format that.
                          */
	                 fmtResult = ErsSPrintf(space,&string[*length],"%.15g ",
                                                SdsGetI64toD(data));
#endif
	                 break;
	            }
	            case SDS_FLOAT:
	            {
	                 float data;
	                 SdsGet(id,sizeof(data),offset,(void *)&data,&actlen,status);
	                 fmtResult = ErsSPrintf(space,&string[*length],"%g ",data);
	                 break;
	            }
	            case SDS_DOUBLE:
	            {
	                 double data;
	                 SdsGet(id,sizeof(data),offset,(void *)&data,&actlen,status);
	                 fmtResult = ErsSPrintf(space,&string[*length],"%g ",data);
	                 break;
	            }
	            default: ;
	        }
	        if (fmtResult < 0)
	            string[*length] = '\0';
	        else
	    	    *length += strlen(&string[*length]);
	        space = maxlen-*length - 1;
	    }
	    (*length)--;
	    string[*length] = '\0';
	}
        	
    }
    else    /* Primitive item	*/
    {
	ArgCvt(&id,ARG_SDS,ARG_STRING2,&string[*length],maxlen-*length,status);
	*length += strlen(&string[*length]);
    }
	 
}


/*+				 A r g S d s L i s t

 *  Function name:
      ArgSdsList

 *  Function:
 *  	List an Sds structure calling a user supplied callback.

 *  Description:
 	This routine does basically the same job as SdsList, but invokes
 	a user supplied callback routine to do the actual output.  This
 	allows the output to be directed anywhere required by the user,
 	not just to stdout as in SdsList.
 
 	The output routine is called for each line to be output.  Note that
 	although close to the output of SdsList, this output of this routine
 	is not exactly the same.
      
 *  Language:
      C

 *  Declaration:
      void ArgSdsList(id, buflen, buffer, func, client_data, status)
      
 *  Parameters:   (">" input, "!" modified, "W" workspace, "<" output)
      (>) id        (SdsIdType) Identifier to the structure.
      (>) buflen    (unsigned int) Length of buffer.  If zero, then malloc
      				a buffer of 400 bytes.  If lines exceed this
      				length, then they are truncated correctly
      				before calling the output routine.
      (>) buffer    (char *)	A buffer to be used by this routine to print
      				the output into.  It is passed directly to
      				the output function.   Since the length of 
      				each line can be quite long (particularly
      				with arrays) we allow the user to specify
      				how much of a line he is interested in.  For
      				example, if outputing to a terminal, you might
      				only specify an 80 byte buffer.  If not 
      				supplied, we will malloc buflen bytes.
      (>) func	    (ArgListFunctType) A function to called to output each
      				line.  It is passed the client data item,
      				the address of the buffer and a modified 
      				status item.
      				
      (>) client_data (void *) Passed directly to func.
      (!) status    (StatusType*) Modified status. 

 *  Support: Tony Farrell, AAO

 *  Version date: 17-Apr-97
 *-
 */
/*
 *  Prototype the functions called recursively.
 */
static void Arg___SdsList(int level, SdsIdType id,  unsigned buflen, 
		char *buffer,  ArgListFuncType func, void * client_data,
		  long sndims, unsigned long *sdims, 
		  StatusType * ARGCONST status);
static void Arg___PrintIt(unsigned buflen, char *buffer, ArgListFuncType func, 
		void *client_data, int level, ARGCONST char *name,
			SdsCodeType code,
			long ndims, ARGCONST unsigned long *dims,
			ARGCONST char *value,			
                        StatusType * ARGCONST status);
		  
/*
 *  The function itself.  Just handle the buffer and buflen arguments
 *  correctly before calling Arg___SdsList setting the level number to 0.
 */
extern void ArgSdsList(SdsIdType id, unsigned buflen, char *buffer, 
		 ArgListFuncType func, void * client_data,
		 StatusType * ARGCONST status)
		  
{
   if (*status != SDS__OK) return;
   if ((buflen == 0)||(buffer == 0))
   {
        if (buflen == 0) buflen = 400;
        if ((buffer = (void *)SdsMalloc(buflen)) == 0)
        { 
            *status = ARG__MALLOCERR;
            return;    
        }
        Arg___SdsList(0,id,buflen,buffer,func,client_data,0,0,status);
        SdsFree(buffer);
   }
   else
       Arg___SdsList(0,id,buflen,buffer,func,client_data,0,0,status);
}

/*
 *  Routine to rescursively list an Sds structure, using a supplied
 *  buffer and output routine.
 */

static void Arg___SdsList(int level, SdsIdType id,  unsigned buflen, 
		char *buffer,  ArgListFuncType func, void * client_data,
		  long sndims, unsigned long *sdims, 
		  StatusType * ARGCONST status)
{
    char name[SDS_C_NAMELEN];
    SdsCodeType code;
    long ndims;
    unsigned long dims[SDS_C_MAXARRAYDIMS];
 
    if (*status != STATUS__OK) return;    
/*
 *  Get details of this structure.
 */
    SdsInfo(id,name,&code,&ndims,dims,status);
    if ((code == SDS_STRUCT)&&(ndims == 0))
    {
/*
 *  	It is a structure item.
 */
    	int index = 1;
/*
 *      If sndims > 0, then this item is a structure array element. sndims
 *      and sdims specfiy the array index.  We format the array index
 *      into the name.  
 *      
 *      IF sndims = 0, just output the name.
 */

    	if (sndims > 0)
    	{
    	    char cellname[100];
    	    register unsigned i;
    	    sprintf(cellname,"%s[%ld",name,sdims[0]);
    	    for (i = 1; i < (unsigned)sndims ; ++ i)
    	    {
    	        char strtemp[20];
    	        sprintf(strtemp,",%ld",sdims[i]) ;      
    	        strcat(cellname,strtemp);
    	    }
    	    strcat(cellname,"]");
            Arg___PrintIt(buflen,buffer,func,client_data,level,cellname,
            		code,ndims,dims,"",status);
    	}
    	else
            Arg___PrintIt(buflen,buffer,func,client_data,level,name,
            		code,ndims,dims,"",status);

/*
 *  	Step through each element in the structure, recursively calling
 *  	this routine to list it.
 */
    	while (*status == STATUS__OK)
    	{
            SdsIdType tid;
    	    SdsIndex(id,index,&tid,status);
 	    Arg___SdsList(level+1,tid,buflen,buffer,func,client_data,0,0,status);
 	    SdsFreeId(tid,status);
    	    ++index;
    	}
    	if (*status == SDS__NOITEM) *status = STATUS__OK;

    }
    else if (code == SDS_STRUCT)
    {
/*
 *  	We are dealing with a structure array.  
 */
 	unsigned long index[SDS_C_MAXARRAYDIMS];
        unsigned long size[SDS_C_MAXARRAYDIMS];
 	long temp;
 	long ind=0;
 	SdsIdType nid;
 	register int i;
/*
 *  	Output the name and top level details.
 */

        Arg___PrintIt(buflen,buffer,func,client_data,level,name,code,ndims,
        						dims,"",status);
/*
 *  	Step through the cells, recursively listing the contents of each
 *  	structure.
 */
  	while (*status == STATUS__OK)
 	{
 	    size[0] = 1;
 	    for (i = 1; i < ndims ; ++i) 
                size[i] = dims[i-1]*size[i-1];
 	    temp = ind;
 	    for (i = ndims-1 ; i >= 0 ; i--)
 	    {
 	        index[i] = temp / size[i] + 1;
 	        temp = temp % size[i];    
 	    }
 	    SdsCell(id,ndims,index,&nid,status);
 	    Arg___SdsList(level+1,nid,buflen,buffer,func,client_data,
 	    			ndims,index,status);
 	    SdsFreeId(nid,status);
 	    ind++;
 	}
    	if (*status == SDS__INDEXERR) *status = STATUS__OK;
    }
    else if ((ndims > 0)&&(*status == STATUS__OK))
    {
/*
 *  	Scaler array element.  Allocate a buffer of the same size as the
 *  	supplied user's buffer.  Format the array into this buffer and then
 *  	call the output routine.
 */
        char *string;
        int length = 0;
        if ((string = (void *)SdsMalloc(buflen)) == 0)
        { 
            ErsRep(0, status, "Error mallocing memory in ArgSdsList()");
            *status = ARG__MALLOCERR;
            return;    
        }
        ErsPush();
    	ArgToString(id,buflen,&length,string,status);    
	if(*status == SDS__UNDEFINED)
	{
	    strncpy(string,"Undefined",buflen-1);
            ErsAnnul(status);
	   
        }
        else if (*status == ARG__CNVERR)
        {
            ErsAnnul(status);
        }
        ErsPop();
        Arg___PrintIt(buflen,buffer,func,client_data,level,name,code,ndims,
        					dims,string,status);
  	SdsFree(string);    	

    }
    else if (*status == STATUS__OK)
    {
/*
 *  	Single scaler item.  Convert to a string and then call the
 *  	output routine.
 */

        char string[50];
        int length = 0;
        ErsPush();
    	ArgToString(id,sizeof(string),&length,string,status);    
	if(*status == SDS__UNDEFINED)
	{
            
	    strcpy(string,"Undefined");
	    ErsAnnul(status);
        }
        ErsPop();
        Arg___PrintIt(buflen,buffer,func,client_data,level,name,code,ndims,
        					dims,string,status);
    }
}

/*
 *   Print it formats each line and then calls the user's output routine.
 */
static void Arg___PrintIt(unsigned buflen,char *buffer, ArgListFuncType func, 
		void *client_data, int level, ARGCONST char *name,
			SdsCodeType code,
			long ndims, ARGCONST unsigned long *dims,
			ARGCONST char *value,			
                        StatusType * ARGCONST status)
{
/*
 *  The comlexities here are handling arrays and character strings and
 *  handling truncation of the line.
 */

    char indicies[40];
    int fmtResult;
    if (*status != STATUS__OK) return;
    if (ndims > 0)
    {
/*
 *      Array item, we need to set up a string with the indicies.
 */
        register unsigned i;
        indicies[0] = '[';
        indicies[1] = '\0';
        for ( i = 0; i < (unsigned)ndims ; ++ i)
        {
            char v[20];
            if (i+1 == (unsigned)ndims)
                sprintf(v,"%ld",dims[i]);
            else
                sprintf(v,"%ld,",dims[i]);
            strcat(indicies,v);
        } 
/*
 *  	The data for a character array is enclosed in quotes
 *  	The data for a structure array is not output
 *  	The data for other array is enclosed in braces
 */

        if (code == SDS_CHAR)
            strcat(indicies,"] \"");
        else if (code == SDS_STRUCT)
            strcat(indicies,"]");
	else
            strcat(indicies,"] {");
    }
    else
        indicies[0] = '\0';
    
/*
 *   Format the string.  We have fixed lengths for the name and type areas.
 *   We use a variable length format string as the first item to set the
 *   indent.
 *   
 *   For character strings, the data is enclosed in quotes
 *   For structure arrays, data is not output
 *   For the reset, just do it.
 */

    if ((code == SDS_CHAR)&&(ndims > 0))
         fmtResult = ErsSPrintf(buflen,buffer,"%*s%-20s %-6s %s%s\"",
    			level*2,"",name,SdsTypeToStr(code,0), indicies, value);
    else if ((code == SDS_STRUCT)&&(ndims > 0))
        fmtResult = ErsSPrintf(buflen,buffer,"%*s%-20s %-6s %s",
    			level*2,"",name,SdsTypeToStr(code,0), indicies);    
    else
        fmtResult = ErsSPrintf(buflen,buffer,"%*s%-20s %-6s %s %s %s",
    			level*2,"",name,SdsTypeToStr(code,0), indicies,
    			value, (ndims > 0 ? "}" : ""));
/*
 *  If fmtResult < 0, it indicates we ran out of space.  We must now terminate
 *  the line nicely.
 */

    if (fmtResult < 0)
    {
        unsigned i;
        if (code == SDS_CHAR)
/*
 *  	    Finish character arrays with ..."
 */
            strcpy(&buffer[buflen-5],"...\"");
        else if ((ndims > 0)&&(code != SDS_STRUCT))
        {
/*
 *  	    Scaler array item. Terminate with ...}.  We need to step back
 *  	    to just after the last completed item.  We do this by finding
 *  	    the last space, but we must step back one more item if we
 *  	    don't have sufficent space.
 */
            for (i = buflen; buffer[i] != ' ' || i > (buflen-6); --i);
            strcpy(&buffer[i]," ...}");
        }
        else
        {
/*
 *  	    Other lines, terminate before last space.
 *  	    (Possibly more work is needed here)
 */
            for (i = buflen; buffer[i] != ' '; --i);
            strcpy(&buffer[i],"\n");
        }
    }
/*
 *  Output the line
 */
    if (func)
            (*func)(client_data,buffer,status);
    
}




/*+		              A r g C h e c k I t e m

 *  Function name:
      ArgCheckItem

 *  Function:
      Check if an SDS id's details matches the specified details.

 *  Description:
      A common requirement for the user of the SDS library is to check
      that a specified SDS item has the right format - type code,
      number of dimensions and dimensions.  This utility function
      implements such a check.
      
 *  Language:
      C

 *  Declaration:
      void ArgCheckItem(id, code, ndims, dims, status)
      
 *  Parameters:   (">" input, "!" modified, "W" workspace, "<" output)
      (>) id        (SdsIdType) Identifier to the structure to be checked.
      (>) code      (SdsCodeType) The SDS code we expect.  The normal
                                    SDS code values or ARG_STRING or -1.
                                    If ARG_STRING, we mean a SDS_CHAR with
                                    one extra dimension then specified in 
                                    ndims, which can be of any length.  If -1,
                                    then ignore the code.
      (>) ndims     (long)      The expected number of dimensions.  0 to 
                                    SDS_C_MAXARRAYDIMS (7) or a maximum
                                    one less then this if code=ARG_STIRNG.
                                    If = -1, then don't check ndims.
      (>) dims      (long [])  The expected dimensions.  Only used
                                    if ndims>0 or code=ARG_STRING.  Only ndims
                                    values are used (or ndims+1 if 
                                    code=ARG_STRING).  If the value is
                                    negative or zero, the value is ignored, 
                                    otherwise they must match.
                                    Note - long, not the unsigned long that
                                     SDS dimension arrays noramlly are.
                                     A null pointer can be supplied, which
                                     is equivalent of an array of -1 values.
      (!) status    (StatusType*) Modified status. Possible failure codes are:
                            as per SdsInfo and SDS__INVITEM.

 *  Support: Tony Farrell, {AAO}

 *-
 */
 
ARGEXTERN void ArgCheckItem(
    SdsIdType id, 
    SdsCodeType code, long ndims, 
    long * ARGCONST dims, 
    StatusType * ARGCONST status)
{
    char tname[SDS_C_NAMELEN];
    SdsCodeType tcode;
    long tndims;
    long i;
    unsigned long tdims[SDS_C_MAXARRAYDIMS];
    
    if (*status != STATUS__OK) return;

    SdsInfo(id, tname, &tcode, &tndims, tdims, status);
    if (*status != STATUS__OK) return;

    if ((code < ARG_STRING)||(code > SDS_C_MAXCODE))
    {
        *status = SDS__INVCODE;
        ErsRep(0, status, 
               "The SDS code of %d is invalid, must be between %d and %d",
               (int)code, ARG_STRING, SDS_C_MAXCODE);
    }
    /*
     * Convert ARG_STRING or ARG_STRING2 to SDS_CHAR with an increased number
     * of dimensions.
     */
    if ((code == ARG_STRING)||(code == ARG_STRING2))
    {
        if (ndims >= 0) ++ndims;
        code = SDS_CHAR;
    }

    if ((code != -1)&&(code != tcode))
    {
        *status = SDS__INVITEM;
        ErsRep(0, status, 
               "Expected SDS item code is %s, actual is %s",
               SdsTypeToStr(code, 1), SdsTypeToStr(tcode, 1));
    }
    if ((ndims >= 0)&&(ndims != tndims))
    {
        *status = SDS__INVITEM;
        ErsRep(0, status,  
               "Expected SDS item to have %ld dimensions, but it had %ld",
               ndims, tndims);
        
    }
    /*
     * If ndims was not specified or ndims not specified, return now.
     */
    if ((ndims <= 0)||(dims == 0))
        return;

    for (i = 0; i < ndims ; ++i)
    {
        if ((dims[i] >= 0)&&((unsigned long)(dims[i]) != tdims[i]))
        {
            *status = SDS__INVITEM;
            ErsRep(0, status,  
                   "Expected SDS item dimension %ld to be %ld but it is %ld",
                   i, dims[i], tdims[i]);
            
        }
    }
}

/*
 * Internal function - used to find a 1D array of the given number of dimensions
 * and SDS code type.  If it does not exist, it is created.
 */
static void Arg___FindOrCreate1DArray(
    const SdsIdType id,
    const char * const name,
    const int code,
    const unsigned long dims,
          SdsIdType * const arrayId,
    StatusType * const status)
{
    if (*status != STATUS__OK) return;
    *arrayId = 0;
    /*
     * Try to find the item.  If it does not exist, create it.
     */
    SdsFind(id,name,arrayId,status);
    if (*status == SDS__NOITEM)
    {
        *status = STATUS__OK;
        SdsNew(id, name, 0, NULL, code,  1,  &dims, arrayId, status);
    }
    else if (*status == STATUS__OK)
    {
        char tname[SDS_C_NAMELEN];
        long tndims;
        SdsCodeType tcode;
        unsigned long tdims[SDS_C_MAXARRAYDIMS];
        SdsInfo(*arrayId, tname, &tcode, &tndims, tdims, status);

        if (*status != STATUS__OK)
        {
            /* should not happen - SdsInfo should have worked */
            return;
        }

        if ((tndims != 1)||(tdims[0] < dims))
        {   
            *status = SDS__INVDIMS;
        }
        else if (tcode != code)
        {
            *status = SDS__INVCODE;
        }
        if (*status != STATUS__OK)
        {
            StatusType ignore = STATUS__OK;
            ErsRep(0, status, "Item named %s existed but was of wrong type", name);
            ErsRep(0, status, "Expected 1D array of size %ld elements, type %s",
                   dims, SdsTypeToStr(code, 1));
            if (tndims == 0)
            {
                ErsRep(0, status, "Got non-array item of type %s",
                       SdsTypeToStr(tcode, 1));
            }
            else if (tndims == 1)
            {
                ErsRep(0, status, "Got 1 D array of type %s, %ld elements",
                       SdsTypeToStr(tcode,1 ), tdims[0]);
            }
            else
            {
                ErsRep(0, status, "Got %ld D array of type %s",
                       tndims, SdsTypeToStr(tcode, 1));
            }   
            SdsFreeId(*arrayId, &ignore);
            *arrayId = 0;
        }
    }
    
}


/*+				 A r g P u t 1 D A r r a y D

 *  Function name:
      ArgPut1DArrayD

 *  Function:
      Put a 1D double array.

 *  Description:
     Put a 1D double array item into an ARG structure.  The item is created if
     it does not exist.  

     Does not convert items like the other Arg routines - if the item already exists
     it must be at least as long as required and of the same type.
     
            
 *  Language:
      C

 *  Declaration:
      void ArgPut1DArrayD(id, name, dim, data, status)
      
 *  Parameters:   (">" input, "!" modified, "W" workspace, "<" output)
      (>) id        (SdsIdType) Identifier to the structure.
      (>) name      (char*) Name of the component to be written to.
      (>) dim       (long)  Number of elements in array.  If array does not
                             already exist, it will be created with this number
                             of elements.  If it does exist, it must have at
                             least this number of elements
      (>) data      (double[]) The data to be inserted.
      (!) status    (StatusType*) Modified status. Possible failure codes are:
                            SDS__NOMEM => Insufficient memory for creation
                            SDS__BADID => Invalid Identifier
                            SDS__INVDIMS => Item exists but it not a 1 D array or
                                is 1 D but has less then "dim" elements.
                            SDS__INVTYPE => Item exists but it not of type SDS_DOUBLE.


 *  Support: Tony Farrell, {AAO}

 *-
 */



ARGEXTERN void ArgPut1DArrayD(
    const SdsIdType id,
    const char * const name,
    const unsigned long dim,
    const double data[],
    StatusType * const status)
{
    SdsIdType arrayId = 0;
    StatusType ignore = STATUS__OK;

    /*
     * Find or create the item in question.  
     */
    Arg___FindOrCreate1DArray(id, name, SDS_DOUBLE, dim, &arrayId, status);
    if (*status != STATUS__OK) return;
    
    /*
     * Insert the data.
     */
    SdsPut(arrayId, dim*sizeof(double), 0, (void *)(data), status);
    
    
    SdsFreeId(arrayId,&ignore);
}


/*+				 A r g P u t 1 D A r r a y D E l e m

 *  Function name:
      ArgPut1DArrayDElem

 *  Function:
      Put a 1D double array element.

 *  Description:
     Put an element 1D double array item into an ARG structure.  The item is 
     created  if it does not exist.  

     Does not convert items like the other Arg routines - if the item already exists
     it must be at least as long as required and of the same type.
     
            
 *  Language:
      C

 *  Declaration:
      void ArgPut1DArrayDElem(id, name, dim, index, data, status)
      
 *  Parameters:   (">" input, "!" modified, "W" workspace, "<" output)
      (>) id        (SdsIdType) Identifier to the structure.
      (>) name      (char*) Name of the component to be written to.
      (>) dim       (long)  Min number of elements in array.  If array does not
                             already exist, it will be created with this number
                             of elements.  If zero, use index+1.
      (>) index     (unsigned int)   Index of item to be inserted.
      (>) data      (double) The data to be inserted.
      (!) status    (StatusType*) Modified status. Possible failure codes are:
                            SDS__NOMEM => Insufficient memory for creation
                            SDS__BADID => Invalid Identifier
                            SDS__INVDIMS => Item exists but it not a 1 D array or
                                is 1 D but has less then "dim" elements.  Or index
                                is greater then dim-1.
                            SDS__INVTYPE => Item exists but it not of type SDS_DOUBLE.


 *  Support: Tony Farrell, {AAO}

 *-
 */



ARGEXTERN void ArgPut1DArrayDElem(
    const SdsIdType id,
    const char * const name,
          unsigned long dim,
    const unsigned int index,
    const double data,
    StatusType * const status)
{
    SdsIdType arrayId = 0;
    StatusType ignore = STATUS__OK;

    if (dim == 0) dim = index+1;

    if (index >= dim)
    {
        *status = SDS__INVDIMS;
        ErsRep(0, status, 
               "Tried to insert a item at index %d into an array of size %ld",
               index, dim);
    }
    /*
     * Find or create the item in question.  
     */
    Arg___FindOrCreate1DArray(id, name, SDS_DOUBLE, dim, &arrayId, status);
    if (*status != STATUS__OK) return;
    
    /*
     * Insert the data.
     */
    SdsPut(arrayId, sizeof(double), index, (void *)(&data), status);
    
    
    SdsFreeId(arrayId,&ignore);


}

/*+				 A r g P u t 1 D A r r a y I

 *  Function name:
      ArgPut1DArrayI

 *  Function:
      Put a 1D INT32 array.

 *  Description:
     Put a 1D INT32 array item into an ARG structure.  The item is created if
     it does not exist.  

     Does not convert items like the other Arg routines - if the item already exists
     it must be at least as long as required and of the same type.
     
            
 *  Language:
      C

 *  Declaration:
      void ArgPut1DArrayI(id, name, dim, data, status)
      
 *  Parameters:   (">" input, "!" modified, "W" workspace, "<" output)
      (>) id        (SdsIdType) Identifier to the structure.
      (>) name      (char*) Name of the component to be written to.
      (>) dim       (long)  Number of elements in array.  If array does not
                             already exist, it will be created with this number
                             of elements.  If it does exist, it must have at
                             least this number of elements
      (>) data      (INT32[]) The data to be inserted.
      (!) status    (StatusType*) Modified status. Possible failure codes are:
                            SDS__NOMEM => Insufficient memory for creation
                            SDS__BADID => Invalid Identifier
                            SDS__INVDIMS => Item exists but it not a 1 D array or
                                is 1 D but has less then "dim" elements.
                            SDS__INVTYPE => Item exists but it not of type SDS_DOUBLE.


 *  Support: Tony Farrell, {AAO}

 *-
 */



ARGEXTERN void ArgPut1DArrayI(
    const SdsIdType id,
    const char * const name,
    const unsigned long dim,
    const INT32 data[],
    StatusType * const status)
{
    SdsIdType arrayId = 0;
    StatusType ignore = STATUS__OK;

    /*
     * Find or create the item in question.  
     */
    Arg___FindOrCreate1DArray(id, name, SDS_INT, dim, &arrayId, status);
    if (*status != STATUS__OK) return;
    
    /*
     * Insert the data.
     */
    SdsPut(arrayId, dim*sizeof(INT32), 0, (void *)(data), status);
    
    
    SdsFreeId(arrayId,&ignore);
}


/*+				 A r g P u t 1 D A r r a y I E l e m

 *  Function name:
      ArgPut1DArrayIElem

 *  Function:
      Put a 1D INT32 array element.

 *  Description:
     Put an element 1D INT32 array item into an ARG structure.  The item is created 
     if it does not exist.  

     Does not convert items like the other Arg routines - if the item already exists
     it must be at least as long as required and of the same type.
     
            
 *  Language:
      C

 *  Declaration:
      void ArgPut1DArrayIElem(id, name, dim, index, data, status)
      
 *  Parameters:   (">" input, "!" modified, "W" workspace, "<" output)
      (>) id        (SdsIdType) Identifier to the structure.
      (>) name      (char*) Name of the component to be written to.
      (>) dim       (long)  Number of elements in array.  If array does not
                             already exist, it will be created with this number
                             of elements.  If it does exist, it must have at
                             least this number of elements.  If zero, use index+1.
      (>) index     (unsigned int)   Index of item to be inserted.
      (>) data      (INT32) The data to be inserted.
      (!) status    (StatusType*) Modified status. Possible failure codes are:
                            SDS__NOMEM => Insufficient memory for creation
                            SDS__BADID => Invalid Identifier
                            SDS__INVDIMS => Item exists but it not a 1 D array or
                                is 1 D but has less then "dim" elements.  Or index
                                is greater then dim-1.
                            SDS__INVTYPE => Item exists but it not of type SDS_DOUBLE.


 *  Support: Tony Farrell, {AAO}

 *-
 */



ARGEXTERN void ArgPut1DArrayIElem(
    const SdsIdType id,
    const char * const name,
          unsigned long dim,
    const unsigned int index,
    const INT32 data,
    StatusType * const status)
{
    SdsIdType arrayId = 0;
    StatusType ignore = STATUS__OK;

    if (dim == 0) dim = index+1;

    if (index >= dim)
    {
        *status = SDS__INVDIMS;
        ErsRep(0, status, 
               "Tried to insert a item at index %d into an array of size %ld",
               index, dim);
    }
    /*
     * Find or create the item in question.  
     */
    Arg___FindOrCreate1DArray(id, name, SDS_INT, dim, &arrayId, status);
    if (*status != STATUS__OK) return;
    
    /*
     * Insert the data.
     */
    SdsPut(arrayId, sizeof(INT32), index, (void *)(&data), status);
    
    
    SdsFreeId(arrayId,&ignore);
}

/*+				 A r g P u t 1 D A r r a y B

 *  Function name:
      ArgPut1DArrayB

 *  Function:
      Put a 1D "unsigned char" (SDS_UBYTE) array.

 *  Description:
     Put a 1D "unsigned char" array item into an ARG structure.  The item is created if
     it does not exist.  

     Does not convert items like the other Arg routines - if the item already exists
     it must be at least as long as required and of the same type.
     
            
 *  Language:
      C

 *  Declaration:
      void ArgPut1DArrayB(id, name, dim, data, status)
      
 *  Parameters:   (">" input, "!" modified, "W" workspace, "<" output)
      (>) id        (SdsIdType) Identifier to the structure.
      (>) name      (char*) Name of the component to be written to.
      (>) dim       (long)  Number of elements in array.  If array does not
                             already exist, it will be created with this number
                             of elements. If it does exist, it must have at
                             least this number of elements.  
      (>) data      (unsigned char[]) The data to be inserted.
      (!) status    (StatusType*) Modified status. Possible failure codes are:
                            SDS__NOMEM => Insufficient memory for creation
                            SDS__BADID => Invalid Identifier
                            SDS__INVDIMS => Item exists but it not a 1 D array or
                                is 1 D but has less then "dim" elements.
                            SDS__INVTYPE => Item exists but it not of type SDS_DOUBLE.


 *  Support: Tony Farrell, {AAO}

 *-
 */



ARGEXTERN void ArgPut1DArrayB(
    const SdsIdType id,
    const char * const name,
    const unsigned long dim,
    const unsigned char data[],
    StatusType * const status)
{
    SdsIdType arrayId = 0;
    StatusType ignore = STATUS__OK;

    /*
     * Find or create the item in question.  
     */
    Arg___FindOrCreate1DArray(id, name, SDS_UBYTE, dim, &arrayId, status);
    if (*status != STATUS__OK) return;
    
    /*
     * Insert the data.
     */
    SdsPut(arrayId, dim*sizeof(unsigned char), 0, (void *)(data), status);
    
    
    SdsFreeId(arrayId,&ignore);
}


/*+				 A r g P u t 1 D A r r a y B E l e m

 *  Function name:
      ArgPut1DArrayBElem

 *  Function:
      Put a 1D unsigned char (SDS_BYTE) array element.

 *  Description:
     Put an element 1D unsigned char array item into an ARG structure.  The item is 
     created  if it does not exist.  

     Does not convert items like the other Arg routines - if the item already exists
     it must be at least as long as required and of the same type.
     
            
 *  Language:
      C

 *  Declaration:
      void ArgPut1DArrayBElem(id, name, dim, index, data, status)
      
 *  Parameters:   (">" input, "!" modified, "W" workspace, "<" output)
      (>) id        (SdsIdType) Identifier to the structure.
      (>) name      (char*) Name of the component to be written to.
      (>) dim       (long)  Number of elements in array.  If array does not
                             already exist, it will be created with this number
                             of elements.  If it does exist, it must have at
                             least this number of elements.  If zero, use index+1.
      (>) index     (unsigned int)   Index of item to be inserted.
      (>) data      (unsigned char) The data to be inserted.
      (!) status    (StatusType*) Modified status. Possible failure codes are:
                            SDS__NOMEM => Insufficient memory for creation
                            SDS__BADID => Invalid Identifier
                            SDS__INVDIMS => Item exists but it not a 1 D array or
                                is 1 D but has less then "dim" elements.  Or index
                                is greater then dim-1.
                            SDS__INVTYPE => Item exists but it not of type SDS_DOUBLE.


 *  Support: Tony Farrell, {AAO}

 *-
 */



ARGEXTERN void ArgPut1DArrayBElem(
    const SdsIdType id,
    const char * const name,
          unsigned long dim,
    const unsigned int index,
    const unsigned char data,
    StatusType * const status)
{
    SdsIdType arrayId = 0;
    StatusType ignore = STATUS__OK;

    if (dim == 0) dim = index+1;

    if (index >= dim)
    {
        *status = SDS__INVDIMS;
        ErsRep(0, status, 
               "Tried to insert a item at index %d into an array of size %ld",
               index, dim);
    }
    /*
     * Find or create the item in question.  
     */
    Arg___FindOrCreate1DArray(id, name, SDS_UBYTE, dim, &arrayId, status);
    if (*status != STATUS__OK) return;
    
    /*
     * Insert the data.
     */ 
    SdsPut(arrayId, sizeof(unsigned char), index, (void *)(&data), status);
    
    
    SdsFreeId(arrayId,&ignore);
}

/*
 * Internal function - used to find a 1D array string of the given number of dimensions
 * and SDS code type.  If it does not exist, it is created.
 *
 * actdims and arrayId should only be considered value if *status == STATUS_OK, but
 * may be changed anyway.
 */
static void Arg___FindOrCreateString1DArray(
    const SdsIdType id,                /* Parent SDS item */
    const char * const name,           /* Name of item */
    const unsigned long slen,          /* Requested length of each string */
    const unsigned long dims,          /* Requested number of strings */
          SdsIdType * const arrayId,   /* SDS ID of array returned here */
          unsigned long actdims[],     /* Actual dimensions of array returned here 
                                          Must have at least SDS_C_MAXARRAYDIMS elements.*/
          StatusType * const status)
{
    if (*status != STATUS__OK) return;
    *arrayId = 0;
    /*
     * Try to find the item.  If it does not exist, create it.
     */
    SdsFind(id,name,arrayId,status);
    if (*status == SDS__NOITEM)
    {
        actdims[0] = slen;
        actdims[1] = dims;
        *status = STATUS__OK;
        SdsNew(id, name, 0, NULL, SDS_CHAR,  2,  actdims, arrayId, status);
    }
    else if (*status == STATUS__OK)
    {
        char tname[SDS_C_NAMELEN];
        long tndims;
        SdsCodeType tcode;
        SdsInfo(*arrayId, tname, &tcode, &tndims, actdims, status);

        if (*status != STATUS__OK)
        {
            /* should not happen - SdsInfo should have worked */
            return;
        }

        if ((tndims != 2)||(actdims[0] < slen)||(actdims[1]<dims))
        {   
            *status = SDS__INVDIMS;
        }
        else if (tcode != SDS_CHAR)
        {
            *status = SDS__INVCODE;
        }
        if (*status != STATUS__OK)
        {
            StatusType ignore = STATUS__OK;
            ErsRep(0, status, "Item named %s existed but was of wrong type", name);
            ErsRep(0, status, "Expected 1D array of size %ld elements, type %s",
                   dims, SdsTypeToStr(SDS_CHAR, 1));
            if (tndims == 0)
            {
                ErsRep(0, status, "Got non-array item of type %s",
                       SdsTypeToStr(tcode, 1));
            }
            else if (tndims == 2)
            {
                ErsRep(0, status, "Got 2 D array of type %s, %ld, %ld elements",
                       SdsTypeToStr(tcode,1 ), actdims[0], actdims[1]);
            }
            else
            {
                ErsRep(0, status, "Got %ld D array of type %s",
                       tndims, SdsTypeToStr(tcode, 1));
            }
            SdsFreeId(*arrayId, &ignore);
            *arrayId = 0;
        }
    }
    
}



/*+				 A r g P u t 1 D A r r a y S

 *  Function name:
      ArgPut1DArrayS

 *  Function:
      Put a 1D string array.

 *  Description:
     Put an a string array item into an ARG structure.  The item is created if
     it does not exist.  

     Does not convert items like the other Arg routines - if the item already exists
     it must be at least as long as required and of the same type and the size of each
     string must be the same.

     Note - we actually created a 2D SDS array of char to represent this array of 1D
     strings.  This is a convience feature.
     
            
 *  Language:
      C

 *  Declaration:
      void ArgPut1DArrayS(id, name, strlen, dim, data, status)
      
 *  Parameters:   (">" input, "!" modified, "W" workspace, "<" output)
      (>) id        (SdsIdType) Identifier to the structure.
      (>) name      (char*) Name of the component to be written to.
      (>) slen      (unsigned long) Length of each string item.  If the array does not
                             already exist, it will be created with this number of
                             characters in each string.  If it does exist, it must have
                             at least this number.  This should include the null terminated.
      (>) dim       (unsigned long)  Number of elements in array.  If array does not
                             already exist, it will be created with this number
                             of elements. If it does exist, it must have at
                             least this number of elements
      (>) data      (*char[]) The data to be inserted.  Array of dim pointers to null
                              terminated strings.  If the length of any of these including
                              their null terminator are longer then slen, then an error
                              is triggered.
      (!) status    (StatusType*) Modified status. Possible failure codes are:
                            SDS__NOMEM => Insufficient memory for creation
                            SDS__BADID => Invalid Identifier
                            SDS__INVDIMS => Item exists but it not a 1 D array or
                                is 1 D but has less then "dim" elements.
                            SDS__INVTYPE => Item exists but it not of type SDS_DOUBLE.
                            ARG__CNVERR => The length of an item in "data" plus the null
                                terminator is longer then slen
                            ARG__NOTSTRING => One of the values within data[] is null.
 *  Support: Tony Farrell, {AAO}

 *-
 */



ARGEXTERN void ArgPut1DArrayS(
    const SdsIdType id,
    const char * const name,
    const unsigned long slen,
    const unsigned long dim,
    const char *data[],
    StatusType * const status)
{
    SdsIdType arrayId = 0;
    StatusType ignore = STATUS__OK;
    unsigned long actdims[SDS_C_MAXARRAYDIMS]; /* Actual dims of array. */
    unsigned i;

    if (*status != STATUS__OK) return;


    /*
     * Validate the data array = each string including null terminator
     * must be no greater then slen in length.   Also be sure we actually
     * have strings.
     */
    for (i = 0; (i < dim)&&(*status == STATUS__OK) ; ++i)
    {
        if (data[i] == 0)
            *status = ARG__NOTSTRING;
        else if (strlen(data[i]) >= slen)
            *status = ARG__CNVERR;
    }
    /*
     * Find or create the item in question.  
     */
    Arg___FindOrCreateString1DArray(id, name, slen, dim, &arrayId, actdims, status);
    if (*status != STATUS__OK) return;

    /*
     * Now put each string in term.
     */
    for (i = 0; i < dim ; ++i)
    {
        unsigned len = strlen(data[i])+1; /* Length of string to put including null 
                                             terminator - validated above. */

        SdsPut(arrayId, len, actdims[0]*i, (void *)(data[i]), status);
    }

    
    SdsFreeId(arrayId, &ignore);

}


/*+				 A r g P u t 1 D A r r a y S E l e m

 *  Function name:
      ArgPut1DArraySElem

 *  Function:
      Put a 1D string array element.

 *  Description:
     Put an element of a 1D string array item into an ARG structure.  The item is 
     created  if it does not exist.  

     Does not convert items like the other Arg routines - if the item already exists
     it must be at least as long as required and of the same type.
     

            
 *  Language:
      C

 *  Declaration:
      void ArgPut1DArraySElem(id, name, dim, index, data, status)
      
 *  Parameters:   (">" input, "!" modified, "W" workspace, "<" output)
      (>) id        (SdsIdType) Identifier to the structure.
      (>) name      (char*) Name of the component to be written to.
      (>) slen    (unsigned long) Number of items in each string element. If the
                             item does not already exist, it will be created with this
                             number of character in each string. If it does exist, it must
                             have at least this number.  This length must include the
                             null terminator.  If zero, use the length of the string.
      (>) dim       (unsigned long)  Number of elements in array.  If array does not
                             already exist, it will be created with this number
                             of elements.  If it does exist, it must have at
                             least this number of elements.  IF zero, use index+1.
      (>) index     (unsigned int)   Index of item to be inserted.
      (>) data      (char *) The data to be inserted.
      (!) status    (StatusType*) Modified status. Possible failure codes are:
                            SDS__NOMEM => Insufficient memory for creation
                            SDS__BADID => Invalid Identifier
                            SDS__INVDIMS => Item exists but it not a 1 D array or
                                is 1 D but has less then "dim" elements.  Or index
                                is greater then dim-1.
                            SDS__INVTYPE => Item exists but it not of type SDS_DOUBLE.


 *  Support: Tony Farrell, {AAO}

 *-
 */



ARGEXTERN void ArgPut1DArraySElem(
    const SdsIdType id,
    const char * const name,
          unsigned long slen,
          unsigned long dim,
    const unsigned int index,
    const char * data,
    StatusType * const status)
{
    SdsIdType arrayId = 0;
    StatusType ignore = STATUS__OK;
    unsigned long actdims[SDS_C_MAXARRAYDIMS]; /* Actual dims of array. */
    unsigned long length = strlen(data) + 1;

    if (dim == 0) dim = index+1;
    if (slen == 0) slen = length;

    if (index >= dim)
    {
        *status = SDS__INVDIMS;
        ErsRep(0, status, 
               "Tried to insert a string at index %d into an array of size %ld",
               index, dim);
    }
    else if (length > slen) 
    {
        *status = SDS__INVDIMS;
        ErsRep(0, status, 
               "Tried to insert a string of length %ld into an array where they are a max of %ld",
               length, slen);

    }

    /*
     * Find or create the item in question.  
     */
    Arg___FindOrCreateString1DArray(id, name, slen, dim, &arrayId, actdims, status);
    if (*status != STATUS__OK) return;


    SdsPut(arrayId, length, actdims[0]*index, (void *)(data), status);

    SdsFreeId(arrayId, &ignore);

}

