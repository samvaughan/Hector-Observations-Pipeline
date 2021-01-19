
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
	
	When not under VxWorks, this uses vsnprintf().

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
	    fioFormatV (VxWorks) Do a C style format.

 *  External values used: 
	    none

 *  Prior requirements:
	    none 

 *  Support: Tony Farrell, AAO

 *-

 *  Copyright (c) Anglo-Australian Telescope Board, 1995.
    Not to be used for commercial purposes without AATB permission.
  
 *  History:
      23-Nov-1992 - TJF - Original version, based on Berkeley Unix
		    vprintf.c, version 5.47, 22-Mar-1991.
      27-Nov-1992 - TJF - Under VxWorks, use fioFormatV.
      29-Sep-1993 - TJF - Add Sccs id
      29-Apr-1994 - TJF - Solaris 2 does not have isinf, so use !finite
      21-Oct-1994 - TJF - Osf does not have isinf either - it appears to have
				been droped from ieee.  We use finite instead.
      08-Dec-1995 - TJF - Under solaris 2 we need to include ieeefp.h to
			get the definition of finite.  In addition, it appears
			isnan has been replaced by isnand, so we define a maro,
      18-Oct-1996 - TJF - VxWorks version was not correctly returning
			the result of the format.
      13-Aug-1997 - TJF - Support WIN32.
      27-May-1998 - TJF - Define _BSD_SOURCE under linux such that we get
                          all the necessary prototypes.
      25-Oct-2016 - TJF - We want isfinite() rather the finite(),
                             finite will disappear.
      18-Aug-2020 - TJF - Remove old BSD implementation and just call
                            vsnprintf() and change the result.  
      {@change entry@}
 
 
 *   @(#) $Id: ACMM:DramaErs/ersvsprintf.c,v 3.19 09-Dec-2020 17:17:02+11 ks $
 
 */


#define ERS_MAIN			/* Indication to Ers.h		*/
#ifndef DNOCONST
#   define DNOCONST 1
#endif


static char *rcsId="@(#) $Id: ACMM:DramaErs/ersvsprintf.c,v 3.19 09-Dec-2020 17:17:02+11 ks $";
static void *use_rcsId = (0 ? (void *)(&use_rcsId) : (void *) &rcsId);




#include "Ers.h"

#include <stdarg.h>
#include <stdio.h>

#ifndef VxWorks

extern int ErsVSPrintf(
    const int maxLength, 
    char *string , 
    const char * const fmt0, 
    va_list ap) 
{
    /*
     * vsnprintf() is the modern way of doing this. The
     * only problem is that it returns the number of characters
     * that would have been written if we exceed the buffer, rather
     * then EOF. We catch that and return EOF.  Note, it does not
     * count the trailing new-line, so >= is the correct test.
     */
    int result = vsnprintf(string, maxLength, fmt0, ap);

    if (result >= maxLength)
        return EOF;

    return result;


}
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
static int Ers___SPrintf (
    char *ptr,
    int  inLength,
    OutBufType *out)
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
extern int ErsVSPrintf(
    const int maxLength, 
    char *string , 
    const char * DCONSTR fmt0, 
    va_list ap) 
{
     register int i;
     OutBufType out;
     out.string = string;
     out.Length = maxLength;
     out.Next   = 0;
     if ((i = fioFormatV(fmt0, ap, Ers___SPrintf, (int)&out)) == ERROR)
	return(EOF);
     else
	return(i);
    
}
#endif
