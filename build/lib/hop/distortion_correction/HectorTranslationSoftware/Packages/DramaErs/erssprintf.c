
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

 *  Copyright (c) Anglo-Australian Telescope Board, 1995.
    Not to be used for commercial purposes without AATB permission.
  
 *  History:
      23-Nov-1992 - TJF - Original version,
      29-Sep-1993 - TJF - Add Sccs id
      18-Aug-2020 - TJF - Rather then calling ErsVSPrintf(), call vsnprintf() which is
                            the modern standard for.
      {@change entry@}
 
 
 *   @(#) $Id: ACMM:DramaErs/erssprintf.c,v 3.19 09-Dec-2020 17:17:02+11 ks $
 
 */
#define ERS_MAIN			/* Indication to Ers.h		*/
#ifndef DNOCONST
#    define DNOCONST 1
#endif 


static char *rcsId="@(#) $Id: ACMM:DramaErs/erssprintf.c,v 3.19 09-Dec-2020 17:17:02+11 ks $";
static void *use_rcsId = (0 ? (void *)(&use_rcsId) : (void *) &rcsId);

#include "Ers.h"


extern int ErsSPrintf (
    const int maxLength, 
    char *string, 
    const char * const fmt,...)
{
    int result  = EOF;
    va_list args;
    va_start(args,fmt);
    
    /*return(ErsVSPrintf(maxLength,string,fmt,args));*/

    /*
     * vsnprintf() is the modern way of doing this. The
     * only problem is that it returns the number of characters
     * that would have been written if we exceed the buffer, rather
     * then EOF. We catch that and return EOF.  Note, it does not
     * count the trailing new-line, so >= is the correct test.
     */
    result = vsnprintf(string, maxLength, fmt, args);

    if (result >= maxLength)
        return EOF;

    return result;


}
