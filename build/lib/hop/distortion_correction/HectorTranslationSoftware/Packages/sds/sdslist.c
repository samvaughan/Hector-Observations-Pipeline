/*
*+                                s d s l i s t

 * Function name:
      sdslist

 * Function:
      List  sds files on stdout.

 *  Synopsis:
	sdslist [file...]

 * Description:
	A listing of the contents of Sds objects contained in the specified
	files is generated on standard output.  The listing consists of
	the name, type, dimensions andvalue of each object in the structure.
	The hierarchical structure is indicated by indenting the listing for
	components at each level.

	For array objects, only the values for the first vew components
	are listed so that each listing fits on a single line.	
      

 * Language:
      C


 * Support: Jeremy Bailey, AAO

 *-


 *     @(#) $Id: ACMM:sds/sdslist.c,v 3.94 09-Dec-2020 17:15:24+11 ks $


 * History:
      24-Feb-95 - TJF - Header added for wflman support.
      30-Aug-99 - KS  - Added conditional code to get arguments on Mac.
      07-Jan-04 - TJF - main should be of type int.
      17-Aug-06 - TJF - Handle bad arguments correctly.


 */


static const char *rcsId="@(#) $Id: ACMM:sds/sdslist.c,v 3.94 09-Dec-2020 17:15:24+11 ks $";
static void *use_rcsId = (0 ? (void *)(&use_rcsId) : (void *) &rcsId);


#include "status.h"
#include "sds.h"
#include "sds_err.h"
#include <stdio.h>
#include <errno.h>

#if defined (__MWERKS__) && defined(macintosh)
#include <console.h>
#endif

int main(int argc, char* argv[])

{
    StatusType status;
    SdsIdType id;
  
#if defined (__MWERKS__) && defined(macintosh)
    argc = ccommand(&argv);
#endif

    if (argc == 2)
    {
        status = SDS__OK;
        SdsRead(argv[1],&id,&status);
        SdsList(id,&status);
        if (status != 0)
        {
            fprintf(stderr,"%s:Error reading file \"%s\"\n", argv[0], argv[1]);
            if (((status == SDS__FREAD)||(status == SDS__FOPEN))&&
                (errno != 0))
                perror("    File Read Error:");
            else if (status == SDS__NOTSDS)
                fprintf(stderr, "    SDS Read Error:Likely this is not an SDS file\n");
            else
                fprintf(stderr, "    %s\n", SdsErrorCodeString(status));
        }
    }
    else
    {
	fprintf(stderr, "%s:Usage:%s <filename>\n", argv[0], argv[0]);
        return 1;
    }
    if (status == 0)
        return 0;
    else
        return -1;
 
}
