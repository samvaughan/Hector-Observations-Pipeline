
/*
   This module defines a subset of the routines in the Drama Ers library
   sufficient to allow the Arg package to work. It includes a full
   definition of ErsSprintf, and a dummy version of ErsRep, ErsAnnul,
   ErsPush and ErsPop.

   Jeremy Bailey, AAO

   Aug-2-1993


*/


#define ERS_MAIN


static char *rcsId="@(#) $Id: ACMM:sds/old_standalone/ers.c,v 3.94 09-Dec-2020 17:15:47+11 ks $";
static void *use_rcsId = ((void)&use_rcsId,(void *) &rcsId);


#include "Ers.h"



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
			as specified. 
			ERS_M_HIGHLIGHT => Suggest to the user interface that
			the message should be highlighted.
			ERS_M_BELL => Suggest to the user interface that the
			terminal bell (or an equivalent) should be rung when
			the message is output.
			ERS_M_ALARM => Suggest to the user interface that
			this is urgent message which should be acknowledged
			by the user.

 
      (!) status	(Long int *) The routine works regardless of the
			value of status.  If the message table is full, then
			status is set to ERS__NOSPACE, otherwise, it is
			not touched.
      (>) format    	(Char *) A format statement.  See the description
			of the C printf function.
      (>) arg		(assorted) Formating arguments.  Set the description
			of the C printf function.


 *  Include files: Ers.h

 *  External functions used:
      va_start	(Crtl) Start a variable argument session.
      va_args	(Crtl) Get an argument.
      va_end	(Crtl) End a variable argument session.
      fputs	(Crtl) Output a string
      ErsVSPrintf (Ers) Safe format.

 *  External values used: 
	none

 *  Prior requirements:
	none 

 *  Support: Tony Farrell, AAO

 *-

 *  History:
      06-Oct-1992 - TJF - Original version
      {@change entry@}
 */
/*
 *  Most unix machines do not yet have strarg.h, only the old varargs.h
 *  As a result, they can't handle the new style variable argument
 *  handling.  Note that if ERS_STDARG_OK is true, then so is ERS_PROTOTYPES_OK
 * 
 */
#ifdef ERS_STDARG_OK	    
    extern void ErsRep (ERS_CONST_ARGVAL int flags, StatusType *status, 
			ERS_CONST_ARGPNT char * fmt,...)
#else
    extern void ErsRep (va_alist)
    va_dcl
#endif
{
    va_list args;

#   ifdef ERS_STDARG_OK
	va_start(args,fmt);
#   else
        StatusType *status;
	int  flags;
	char *fmt;
        va_start(args);
        flags = va_arg(args, int);
        status = va_arg(args, StatusType *);
	fmt = va_arg(args, char *);
#   endif
}


extern void ErsAnnul(StatusType *status)
{
}

extern void ErsPush()
{
}

extern void ErsPop()
{
}



/*
 *+			E r s S P r i n t f

 *  Function name:
      ErsSPrintf

 *  Function:
	A safe version of the C RTL sprintf function.


 *  Description:
	The standard C RTL version of sprintf is unsafe as nothing
	limits the length of the output string.  It is easy to overwrite
	the stack.  By providing a length argument string argument, this 
	routine implements a safe version of sprintf.

	See ErsVSPrintf() for more details.	

 *  Language:
      C

 *  Call:
	(int) = ErsSPrintf(length, string, format, args...)

 *  Parameters:   (">" input, "!" modified, "W" workspace, "<" output)
	(>) length  (int) The length of string.
	(<) string  (char *) The pointer to the output string	
	(>) format  (char *) A format specification
	(>) arg...  (anything) argument list 


 *  Function value:
	EOF indicates the format string exceeds 
	the length available, otherwise, the number of characters output.

 *  Include files: Ers.h, stdio.h


 *  External functions used:
	    ErsVSPrintf  (Ers) A save version of vsprintf.

 *  External values used: 
	    none

 *  Prior requirements:
	    none 

 *  Support: Tony Farrell, AAO

 *-

 *  History:
      23-Nov-1992 - TJF - Original version,
      {@change entry@}
 */
#ifdef ERS_STDARG_OK	    
    extern int ErsSPrintf (
			ERS_CONST_ARGVAL int maxLength, 
			char *string, 
			ERS_CONST_ARGPNT char * fmt,...)
#else
    extern int ErsSPrintf (va_alist)
    va_dcl
#endif
{
    va_list args;

#   ifdef ERS_STDARG_OK
	va_start(args,fmt);
#   else
	int maxLength;
	char *string;
	char *fmt;
        va_start(args);
        maxLength = va_arg(args, int);
        string = va_arg(args, char *);
	fmt = va_arg(args, char *);
#   endif
    
    return(ErsVSPrintf(maxLength,string,fmt,args));

}

/*
 *+			E r s V S P r i n t f

 *  Function name:
      ErsVSPrintf

 *  Function:
	A safe version of the C RTL vsprintf function.


 *  Description:
	The standard C RTL version of vsprintf is unsafe as nothing
	limits the length of the output string.  It is easy to overwrite
	the stack.  By providing a length argument string argument, this 
	routine implements a safe version of vsprintf.
	
	When not under VxWorks, this module is based upon the Berkeley Unix 
	vprintf.c module.  (based on version 5.47, 22-Mar-1991).  The header 
	for this file and the appropaite copyright appears below. 

	Under VxWorks, the fioFormatV routine is used.

 *  Language:
      C

 *  Call:
	(int) = ErsVSPrintf(length, string, format, arg)

 *  Parameters:   (">" input, "!" modified, "W" workspace, "<" output)
	(>) length  (int) The length of string.
	(<) string  (char *) The pointer to the output string	
	(>) format  (char *) A format specification
	(>) arg	    (va_list) Variable argument list 


 *  Function value:
	EOF indicates the format string exceeds the length available,
	otherwise, the number of characters output.

 *  Include files: Ers.h, stdio.h


 *  External functions used:
	    none

 *  External values used: 
	    none

 *  Prior requirements:
	    none 

 *  Support: Tony Farrell, AAO

 *-

 *  History:
      23-Nov-1992 - TJF - Original version, based on Berkeley Unix
		    vprintf.c, version 5.47, 22-Mar-1991.
      27-Nov-1992 - TJF - Under VxWorks, use fioFormatV.
      {@change entry@}
 */


/*
 *  Original Berkeley copyright notice.
 *
 * Copyright (c) 1990 The Regents of the University of California.
 * All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Chris Torek.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifdef ERS_STDARG_OK
#include <stdarg.h>
#else
#include <varargs.h>
#endif

#ifndef VxWorks
/*
 *
 * Berkley unix based version, for all systems except VxWorks.
 *
 * This code is large and complicated...
 */

#include <math.h>
#include <stdio.h>
#include <string.h>
#if defined(VxWorks)
#   include <vxWorks.h>
#endif
#ifdef ERS_FUDGE_PROTOTYPES_C
   extern char *memchr(ERS_CONST_ARGPNT char *s1, 
		       ERS_CONST_ARGVAL int c,
		       ERS_CONST_ARGVAL size_t size);
#endif


/* end of configuration stuff */

/*
 *  Copy a string to the output string. If the output string exceeds the
 *  length required, return false, otherwise true.
 */

#ifdef ERS_PROTOTYPES_OK
    static int Ers___SPrintf (
		ERS_CONST_ARGVAL int outLength,/* Max number allowed in string */
		ERS_CONST_ARGPNT char *ptr, /* Point to string to print	    */
		ERS_CONST_ARGVAL int inLength,/* Length of string to print    */
		char *string,		    /* Output string		    */
		int *outNext)		    /* Current length of string	    */
#else
    static int Ers___SPrintf (outLength, ptr, inLength, string, outNext)
		       ERS_CONST_ARGVAL int outLength;
		       ERS_CONST_ARGPNT char *ptr;
		       ERS_CONST_ARGVAL int inLength;
		       char *string;
		       int *outNext;
#endif
{
    register int i=0;
/*
 *  We use outLength-1 to ensure we leave space for the null terminator.
 */
    while ((*outNext < outLength-1)&&(i < inLength))
	string[(*outNext)++] = ptr[i++];

    string[*outNext] = '\0';	/* Null terminate	*/
/*
 *  If we managed to output everything, return 1, else 0.
 */
    if (i == inLength)
	return(1);
    else
	return(0);
}



#ifdef ERS_FLOAT_OK

#define	BUF		(ERS_MAXEXP+ERS_MAXFRACT+1)	/* + decimal point */
#define	DEFPREC		6

#ifdef ERS_PROTOTYPES_OK
    static int Ers___Cvt(
	double number,
	register int prec,
	int flags,
	char *signp,
	int fmtch,
	char *startp,
	char *endp);
#else
    static int Ers___Cvt();
#endif

#else

#define	BUF		40

#endif


/*
 * Macros for converting digits to letters and vice versa
 */
#define	to_digit(c)	((c) - '0')
#define is_digit(c)	((unsigned)to_digit(c) <= 9)
#define	to_char(n)	((n) + '0')

/*
 * Flags used during conversion.
 */
#define	LONGINT		0x01		/* long integer */
#define	LONGDBL		0x02		/* long double; unimplemented */
#define	SHORTINT	0x04		/* short integer */
#define	ALT		0x08		/* alternate form */
#define	LADJUST		0x10		/* left adjustment */
#define	ZEROPAD		0x20		/* zero (as opposed to blank) pad */
#define	HEXPREFIX	0x40		/* add 0x or 0X prefix */

#ifdef ERS_PROTOTYPES_OK
    extern int ErsVSPrintf(
		 ERS_CONST_ARGVAL int maxLength, 
		 char *string , 
		 ERS_CONST_ARGPNT char *fmt0, 
		 va_list ap) 
#else
    extern int ErsVSPrintf(maxLength, string , fmt0, ap) 
		 ERS_CONST_ARGVAL int maxLength;
		 char *string;
		 ERS_CONST_ARGPNT char *fmt0;
		 va_list ap;
#endif
{
	register char *fmt;	/* format string */
	register int ch;	/* character from fmt */
	register int n;		/* handy integer (short term usage) */
	register char *cp;	/* handy char pointer (short term usage) */
	register int flags;	/* flags as above */
	int ret;		/* return value accumulator */
	int width;		/* width from format (%8d), or 0 */
	int prec;		/* precision from format (%.3d), or -1 */
	char sign;		/* sign prefix (' ', '+', '-', or \0) */
#ifdef ERS_FLOAT_OK
	char softsign;		/* temporary negative sign for floats */
	double _double;		/* double precision arguments %[eEfgG] */
	int fpprec;		/* `extra' floating precision in [eEfgG] */
#endif
	unsigned long _ulong ;	/* integer arguments %[diouxX] */
	enum { OCT, DEC, HEX } base;/* base for [diouxX] conversion */
	int dprec;		/* a copy of prec if [diouxX], 0 otherwise */
	int fieldsz;		/* field size expanded by sign, etc */
	int realsz;		/* field size expanded by dprec */
	int size;		/* size of converted field or string */
	char *xdigs="";		/* digits for [xX] conversion */
	char buf[BUF];		/* space for %c, %[diouxX], %[eEfgG] */
	char ox[2];		/* space for 0x hex-prefix */

	int current;		/* Pointer to current length */

	/*
	 * Choose PADSIZE to trade efficiency vs size.  If larger
	 * printf fields occur frequently, increase PADSIZE (and make
	 * the initialisers below longer).
	 */
#define	PADSIZE	16		/* pad chunk size */
	static char blanks[PADSIZE] =
	 {' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' '};
	static char zeroes[PADSIZE] =
	 {'0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0'};

	/*
	 * BEWARE, these `goto error' on error, and PAD uses `n'.
	 */
#define	PRINT(ptr, len) { \
	if (!Ers___SPrintf(maxLength,(ptr),(len),(string),&current)) \
	    return(EOF);\
}
#define	PAD(howmany, with) { \
	if ((n = (howmany)) > 0) { \
		while (n > PADSIZE) { \
			PRINT(with, PADSIZE); \
			n -= PADSIZE; \
		} \
		PRINT(with, n); \
	} \
}
#define	FLUSH()

	/*
	 * To extend shorts properly, we need both signed and unsigned
	 * argument extraction methods.
	 */
#define	SARG() \
	(flags&LONGINT ? va_arg(ap, long) : \
	    flags&SHORTINT ? (long)(short)va_arg(ap, int) : \
	    (long)va_arg(ap, int))
#define	UARG() \
	(flags&LONGINT ? va_arg(ap, unsigned long) : \
	    flags&SHORTINT ? (unsigned long)(unsigned short)va_arg(ap, int) : \
	    (unsigned long)va_arg(ap, unsigned int))

	if ((maxLength < 0)||(string == 0)) 
	    return(EOF);

	fmt = (char *)fmt0;
	ret = 0;
	current = 0;

	/*
	 * Scan the format for conversions (`%' character).
	 */
	for (;;) {
		for (cp = fmt; (ch = *fmt) != '\0' && ch != '%'; fmt++)
			/* void */;
		if ((n = fmt - cp) != 0) {
			PRINT(cp, n);
			ret += n;
		}
		if (ch == '\0')
			goto done;
		fmt++;		/* skip over '%' */

		flags = 0;
		dprec = 0;
#ifdef ERS_FLOAT_OK
		fpprec = 0;
#endif
		width = 0;
		prec = -1;
		sign = '\0';

rflag:		ch = *fmt++;
reswitch:	switch (ch) {
		case ' ':
			/*
			 * ``If the space and + flags both appear, the space
			 * flag will be ignored.''
			 *	-- ANSI X3J11
			 */
			if (!sign)
				sign = ' ';
			goto rflag;
		case '#':
			flags |= ALT;
			goto rflag;
		case '*':
			/*
			 * ``A negative field width argument is taken as a
			 * - flag followed by a positive field width.''
			 *	-- ANSI X3J11
			 * They don't exclude field widths read from args.
			 */
			if ((width = va_arg(ap, int)) >= 0)
				goto rflag;
			width = -width;
			/* FALLTHROUGH */
		case '-':
			flags |= LADJUST;
			goto rflag;
		case '+':
			sign = '+';
			goto rflag;
		case '.':
			if ((ch = *fmt++) == '*') {
				n = va_arg(ap, int);
				prec = n < 0 ? -1 : n;
				goto rflag;
			}
			n = 0;
			while (is_digit(ch)) {
				n = 10 * n + to_digit(ch);
				ch = *fmt++;
			}
			prec = n < 0 ? -1 : n;
			goto reswitch;
		case '0':
			/*
			 * ``Note that 0 is taken as a flag, not as the
			 * beginning of a field width.''
			 *	-- ANSI X3J11
			 */
			flags |= ZEROPAD;
			goto rflag;
		case '1': case '2': case '3': case '4':
		case '5': case '6': case '7': case '8': case '9':
			n = 0;
			do {
				n = 10 * n + to_digit(ch);
				ch = *fmt++;
			} while (is_digit(ch));
			width = n;
			goto reswitch;
#ifdef ERS_FLOAT_OK
		case 'L':
			flags |= LONGDBL;
			goto rflag;
#endif
		case 'h':
			flags |= SHORTINT;
			goto rflag;
		case 'l':
			flags |= LONGINT;
			goto rflag;
		case 'c':
			*(cp = buf) = va_arg(ap, int);
			size = 1;
			sign = '\0';
			break;
		case 'D':
			flags |= LONGINT;
			/*FALLTHROUGH*/
		case 'd':
		case 'i':
			_ulong = SARG();
			if ((long)_ulong < 0) {
				_ulong = -_ulong;
				sign = '-';
			}
			base = DEC;
			goto number;
#ifdef ERS_FLOAT_OK
		case 'e':
		case 'E':
		case 'f':
		case 'g':
		case 'G':
			_double = va_arg(ap, double);

#ifndef VMS
			/* do this before tricky precision changes */
			/* (this checks for infinity and not a number) */
			/* (not necessary on VAX/VMS)		*/
			if (!finite(_double)) {
				if (_double < 0)
					sign = '-';
				cp = "Inf";
				size = 3;
				break;
			}
			if (isnan(_double)) {
				cp = "NaN";
				size = 3;
				break;
			}
#endif
			/*
			 * don't do unrealistic precision; just pad it with
			 * zeroes later, so buffer size stays rational.
			 */
			if (prec > ERS_MAXFRACT) {
				if ((ch != 'g' && ch != 'G') || (flags&ALT))
					fpprec = prec - ERS_MAXFRACT;
				prec = ERS_MAXFRACT;
			} else if (prec == -1)
				prec = DEFPREC;
			/*
			 * cvt may have to round up before the "start" of
			 * its buffer, i.e. ``intf("%.2f", (double)9.999);'';
			 * if the first character is still NUL, it did.
			 * softsign avoids negative 0 if _double < 0 but
			 * no significant digits will be shown.
			 */
			cp = buf;
			*cp = '\0';
			size = Ers___Cvt(_double, prec, flags, &softsign, ch,
			    cp, buf + sizeof(buf));
			if (softsign)
				sign = '-';
			if (*cp == '\0')
				cp++;
			break;
#endif
		case 'n':
			if (flags & LONGINT)
				*va_arg(ap, long *) = ret;
			else if (flags & SHORTINT)
				*va_arg(ap, short *) = ret;
			else
				*va_arg(ap, int *) = ret;
			continue;	/* no output */
		case 'O':
			flags |= LONGINT;
			/*FALLTHROUGH*/
		case 'o':
			_ulong = UARG();
			base = OCT;
			goto nosign;
		case 'p':
			/*
			 * ``The argument shall be a pointer to void.  The
			 * value of the pointer is converted to a sequence
			 * of printable characters, in an implementation-
			 * defined manner.''
			 *	-- ANSI X3J11
			 */
			/* NOSTRICT */
			_ulong = (unsigned long)va_arg(ap, void *);
			base = HEX;
			xdigs = "0123456789abcdef";
			flags |= HEXPREFIX;
			ch = 'x';
			goto nosign;
		case 's':
			if ((cp = va_arg(ap, char *)) == NULL)
				cp = "(null)";
			if (prec >= 0) {
				/*
				 * can't use strlen; can only look for the
				 * NUL in the first `prec' characters, and
				 * strlen() will go further.
				 */
				char *p = memchr(cp, 0, prec);

				if (p != NULL) {
					size = p - cp;
					if (size > prec)
						size = prec;
				} else
					size = prec;
			} else
				size = strlen(cp);
			sign = '\0';
			break;
		case 'U':
			flags |= LONGINT;
			/*FALLTHROUGH*/
		case 'u':
			_ulong = UARG();
			base = DEC;
			goto nosign;
		case 'X':
			xdigs = "0123456789ABCDEF";
			goto hex;
		case 'x':
			xdigs = "0123456789abcdef";
hex:			_ulong = UARG();
			base = HEX;
			/* leading 0x/X only if non-zero */
			if (flags & ALT && _ulong != 0)
				flags |= HEXPREFIX;

			/* unsigned conversions */
nosign:			sign = '\0';
			/*
			 * ``... diouXx conversions ... if a precision is
			 * specified, the 0 flag will be ignored.''
			 *	-- ANSI X3J11
			 */
number:			if ((dprec = prec) >= 0)
				flags &= ~ZEROPAD;

			/*
			 * ``The result of converting a zero value with an
			 * explicit precision of zero is no characters.''
			 *	-- ANSI X3J11
			 */
			cp = buf + BUF;
			if (_ulong != 0 || prec != 0) {
				/*
				 * unsigned mod is hard, and unsigned mod
				 * by a constant is easier than that by
				 * a variable; hence this switch.
				 */
				switch (base) {
				case OCT:
					do {
						*--cp = to_char(_ulong & 7);
						_ulong >>= 3;
					} while (_ulong);
					/* handle octal leading 0 */
					if (flags & ALT && *cp != '0')
						*--cp = '0';
					break;

				case DEC:
					/* many numbers are 1 digit */
					while (_ulong >= 10) {
						*--cp = to_char(_ulong % 10);
						_ulong /= 10;
					}
					*--cp = to_char(_ulong);
					break;

				case HEX:
					do {
						*--cp = xdigs[_ulong & 15];
						_ulong >>= 4;
					} while (_ulong);
					break;

				default:
					cp = "bug in vfprintf: bad base";
					size = strlen(cp);
					goto skipsize;
				}
			}
			size = buf + BUF - cp;
		skipsize:
			break;
		default:	/* "%?" prints ?, unless ? is NUL */
			if (ch == '\0')
				goto done;
			/* pretend it was %c with argument ch */
			cp = buf;
			*cp = ch;
			size = 1;
			sign = '\0';
			break;
		}

		/*
		 * All reasonable formats wind up here.  At this point,
		 * `cp' points to a string which (if not flags&LADJUST)
		 * should be padded out to `width' places.  If
		 * flags&ZEROPAD, it should first be prefixed by any
		 * sign or other prefix; otherwise, it should be blank
		 * padded before the prefix is emitted.  After any
		 * left-hand padding and prefixing, emit zeroes
		 * required by a decimal [diouxX] precision, then print
		 * the string proper, then emit zeroes required by any
		 * leftover floating precision; finally, if LADJUST,
		 * pad with blanks.
		 */

		/*
		 * compute actual size, so we know how much to pad.
		 * fieldsz excludes decimal prec; realsz includes it
		 */
#ifdef ERS_FLOAT_OK
		fieldsz = size + fpprec;
#else
		fieldsz = size;
#endif
		if (sign)
			fieldsz++;
		else if (flags & HEXPREFIX)
			fieldsz += 2;
		realsz = dprec > fieldsz ? dprec : fieldsz;

		/* right-adjusting blank padding */
		if ((flags & (LADJUST|ZEROPAD)) == 0)
			PAD(width - realsz, blanks);

		/* prefix */
		if (sign) {
			PRINT(&sign, 1);
		} else if (flags & HEXPREFIX) {
			ox[0] = '0';
			ox[1] = ch;
			PRINT(ox, 2);
		}

		/* right-adjusting zero padding */
		if ((flags & (LADJUST|ZEROPAD)) == ZEROPAD)
			PAD(width - realsz, zeroes);

		/* leading zeroes from decimal precision */
		PAD(dprec - fieldsz, zeroes);

		/* the string or number proper */
		PRINT(cp, size);

#ifdef ERS_FLOAT_OK
		/* trailing f.p. zeroes */
		PAD(fpprec, zeroes);
#endif

		/* left-adjusting padding (always blank) */
		if (flags & LADJUST)
			PAD(width - realsz, blanks);

		/* finally, adjust ret */
		ret += width > realsz ? width : realsz;

		FLUSH();	/* copy out the I/O vectors */
	}
done:
	FLUSH();
	return (ret);
	/* NOTREACHED */
}

#ifdef ERS_FLOAT_OK
#include <math.h>

#ifdef ERS_PROTOTYPES_OK
    static char * Ers___Exponent(register char *p,register int exp,int fmtch);
    static char * Ers___Round(
	double fract,
	int *exp,
	register char *start, 
	register char *end,
	char ch, 
	char *signp);
#else
    static char * Ers___Exponent(p, exp, fmtch);
    static char * Ers___Round(fract, exp, start, end, ch, signp);
#endif

#ifdef ERS_PROTOTYPES_OK
    static int Ers___Cvt(
	double number,
	register int prec,
	int flags,
	char *signp,
	int fmtch,
	char *startp,
	char *endp)
#else
    static int Ers___Cvt(number, prec, flags, signp, fmtch, startp, endp)
	double number;
	register int prec;
	int flags;
	char *signp;
	int fmtch;
	char *startp, *endp;
#endif
{
	register char *p, *t;
	register double fract;
	int dotrim, expcnt, gformat;
	double integer, tmp;

	dotrim = expcnt = gformat = 0;
	if (number < 0) {
		number = -number;
		*signp = '-';
	} else
		*signp = 0;

	fract = modf(number, &integer);

	/* get an extra slot for rounding. */
	t = ++startp;

	/*
	 * get integer portion of number; put into the end of the buffer; the
	 * .01 is added for modf(356.0 / 10, &integer) returning .59999999...
	 */
	for (p = endp - 1; integer; ++expcnt) {
		tmp = modf(integer / 10, &integer);
		*p-- = to_char((int)((tmp + .01) * 10));
	}
	switch (fmtch) {
	case 'f':
		/* reverse integer into beginning of buffer */
		if (expcnt)
			for (; ++p < endp; *t++ = *p);
		else
			*t++ = '0';
		/*
		 * if precision required or alternate flag set, add in a
		 * decimal point.
		 */
		if (prec || flags&ALT)
			*t++ = '.';
		/* if requires more precision and some fraction left */
		if (fract) {
			if (prec)
				do {
					fract = modf(fract * 10, &tmp);
					*t++ = to_char((int)tmp);
				} while (--prec && fract);
			if (fract)
				startp = Ers___Round(fract, (int *)NULL, startp,
				    t - 1, (char)0, signp);
		}
		for (; prec--; *t++ = '0');
		break;
	case 'e':
	case 'E':
eformat:	if (expcnt) {
			*t++ = *++p;
			if (prec || flags&ALT)
				*t++ = '.';
			/* if requires more precision and some integer left */
			for (; prec && ++p < endp; --prec)
				*t++ = *p;
			/*
			 * if done precision and more of the integer component,
			 * round using it; adjust fract so we don't re-round
			 * later.
			 */
			if (!prec && ++p < endp) {
				fract = 0;
				startp = Ers___Round((double)0, &expcnt, startp,
				    t - 1, *p, signp);
			}
			/* adjust expcnt for digit in front of decimal */
			--expcnt;
		}
		/* until first fractional digit, decrement exponent */
		else if (fract) {
			/* adjust expcnt for digit in front of decimal */
			for (expcnt = -1;; --expcnt) {
				fract = modf(fract * 10, &tmp);
				if (tmp)
					break;
			}
			*t++ = to_char((int)tmp);
			if (prec || flags&ALT)
				*t++ = '.';
		}
		else {
			*t++ = '0';
			if (prec || flags&ALT)
				*t++ = '.';
		}
		/* if requires more precision and some fraction left */
		if (fract) {
			if (prec)
				do {
					fract = modf(fract * 10, &tmp);
					*t++ = to_char((int)tmp);
				} while (--prec && fract);
			if (fract)
				startp = Ers___Round(fract, &expcnt, startp,
				    t - 1, (char)0, signp);
		}
		/* if requires more precision */
		for (; prec--; *t++ = '0');

		/* unless alternate flag, trim any g/G format trailing 0's */
		if (gformat && !(flags&ALT)) {
			while (t > startp && *--t == '0');
			if (*t == '.')
				--t;
			++t;
		}
		t = Ers___Exponent(t, expcnt, fmtch);
		break;
	case 'g':
	case 'G':
		/* a precision of 0 is treated as a precision of 1. */
		if (!prec)
			++prec;
		/*
		 * ``The style used depends on the value converted; style e
		 * will be used only if the exponent resulting from the
		 * conversion is less than -4 or greater than the precision.''
		 *	-- ANSI X3J11
		 */
		if (expcnt > prec || (!expcnt && fract && fract < .0001)) {
			/*
			 * g/G format counts "significant digits, not digits of
			 * precision; for the e/E format, this just causes an
			 * off-by-one problem, i.e. g/G considers the digit
			 * before the decimal point significant and e/E doesn't
			 * count it as precision.
			 */
			--prec;
			fmtch -= 2;		/* G->E, g->e */
			gformat = 1;
			goto eformat;
		}
		/*
		 * reverse integer into beginning of buffer,
		 * note, decrement precision
		 */
		if (expcnt)
			for (; ++p < endp; *t++ = *p, --prec);
		else
			*t++ = '0';
		/*
		 * if precision required or alternate flag set, add in a
		 * decimal point.  If no digits yet, add in leading 0.
		 */
		if (prec || flags&ALT) {
			dotrim = 1;
			*t++ = '.';
		}
		else
			dotrim = 0;
		/* if requires more precision and some fraction left */
		if (fract) {
			if (prec) {
				do {
					fract = modf(fract * 10, &tmp);
					*t++ = to_char((int)tmp);
				} while(!tmp);
				while (--prec && fract) {
					fract = modf(fract * 10, &tmp);
					*t++ = to_char((int)tmp);
				}
			}
			if (fract)
				startp = Ers___Round(fract, (int *)NULL, startp,
				    t - 1, (char)0, signp);
		}
		/* alternate format, adds 0's for precision, else trim 0's */
		if (flags&ALT)
			for (; prec--; *t++ = '0');
		else if (dotrim) {
			while (t > startp && *--t == '0');
			if (*t != '.')
				++t;
		}
	}
	return (t - startp);
}

#ifdef ERS_PROTOTYPES_OK
    static char * Ers___Round(
	double fract,
	int *exp,
	register char *start, 
	register char *end,
	char ch, 
	char *signp)
#else
    static char * Ers___Round(fract, exp, start, end, ch, signp)
	double fract;
	int *exp;
	register char *start, *end;
	char ch, *signp;
#endif
{
	double tmp;

	if (fract)
		(void)modf(fract * 10, &tmp);
	else
		tmp = to_digit(ch);
	if (tmp > 4)
		for (;; --end) {
			if (*end == '.')
				--end;
			if (++*end <= '9')
				break;
			*end = '0';
			if (end == start) {
				if (exp) {	/* e/E; increment exponent */
					*end = '1';
					++*exp;
				}
				else {		/* f; add extra digit */
				*--end = '1';
				--start;
				}
				break;
			}
		}
	/* ``"%.3f", (double)-0.0004'' gives you a negative 0. */
	else if (*signp == '-')
		for (;; --end) {
			if (*end == '.')
				--end;
			if (*end != '0')
				break;
			if (end == start)
				*signp = 0;
		}
	return (start);
}

#ifdef ERS_PROTOTYPES_OK
    static char * Ers___Exponent(register char *p,register int exp,int fmtch)
#else
    static char * Ers___Exponent(p, exp, fmtch)
	register char *p;
	register int exp;
	int fmtch;
#endif
{
	register char *t;
	char expbuf[ERS_MAXEXP];

	*p++ = fmtch;
	if (exp < 0) {
		exp = -exp;
		*p++ = '-';
	}
	else
		*p++ = '+';
	t = expbuf + ERS_MAXEXP;
	if (exp > 9) {
		do {
			*--t = to_char(exp % 10);
		} while ((exp /= 10) > 9);
		*--t = to_char(exp);
		for (; t < expbuf + ERS_MAXEXP; *p++ = *t++);
	}
	else {
		*p++ = '0';
		*p++ = to_char(exp);
	}
	return (p);
}
#endif
/*
 *	End of Non-VxWorks format
 */
#else
/*
 *	Under VxWorks, we call the FioFormatV routine
 */
#include <vxWorks.h>
#include <fioLib.h>

typedef struct {
		char * string;
		int    Length;
		int    Next;
			} OutBufType;
/*
 *	Ers___Sprintf is called by fioFormatV to output the character.  
 */
#ifdef ERS_PROTOTYPES_OK
    static int Ers___SPrintf (
		char *ptr,
		int  inLength,
		OutBufType *out)
#else
    static int Ers___SPrintf (ptr, inLength, out) 
		char *ptr;
		int  inLength;
		OutBufType *out;
#endif
{
    register int i=0;
/*
 *  We use outLength-1 to ensure we leave space for the null terminator.
 */
    while ((out->Next < out->Length-1)&&(i < inLength))
	out->string[(out->Next)++] = ptr[i++];

    out->string[out->Next] = '\0';	/* Null terminate	*/
/*
 *  If we managed to output everything, return 1, else 0.
 */
    if (i == inLength)
	return(OK);
    else
	return(ERROR);
	
}
#ifdef ERS_PROTOTYPES_OK
    extern int ErsVSPrintf(
		 ERS_CONST_ARGVAL int maxLength, 
		 char *string , 
		 ERS_CONST_ARGPNT char *fmt0, 
		 va_list ap) 
#else
    extern int ErsVSPrintf(maxLength, string , fmt0, ap) 
		 ERS_CONST_ARGVAL int maxLength;
		 char *string;
		 ERS_CONST_ARGPNT char *fmt0;
		 va_list ap;
#endif

{
     register i;
     OutBufType out;
     out.string = string;
     out.Length = maxLength;
     out.Next   = 0;
     if (i = (fioFormatV(fmt0, ap, Ers___SPrintf, (int)&out)) == ERROR)
	return(EOF);
     else
	return(1);
    
}
#endif
