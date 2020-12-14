/*
*+                                s d s l i s t p a t h

 * Function name:
      sdslistpath

 * Function:
      List a component of an sds file on stdout.

 *  Synopsis:
	sdslistpath file [component]

 * Description:
	A listing of the contents of Sds objects contained in the specified
	component of the specified file is generated on standard output.  
	
	The listing consists of the name, type, dimensions and value of 
	each object in the structure. The hierarchical structure is 
	indicated by indenting the listing for components at each level.

	For array objects, only the values for the first vew components
	are listed so that each listing fits on a single line.	

	The compoent name is a dot separated sequence of component names
	in the accepted by the function SdsFindByPath.      If a component
	name is not specified, the entire object is listed.

 * Language:
      C


 * Support: Tony Farrell, AAO

 *     @(#) $Id: ACMM:sds/sdslistpath.c,v 3.94 09-Dec-2020 17:15:24+11 ks $

 *-

 * History:
      30-Apr-96 - TJF - Original version.
      21-Jan-02 - TJF - Compilation fix from N.Rees inserted.


 */


static const char *rcsId="@(#) $Id: ACMM:sds/sdslistpath.c,v 3.94 09-Dec-2020 17:15:24+11 ks $";
static void *use_rcsId = (0 ? (void *)(&use_rcsId) : (void *) &rcsId);


#include "status.h"
#include "sds.h"
#include <stdio.h>
#include <errno.h>

int main(int argc, char* argv[])

{
  StatusType status;
  SdsIdType id;
 
  if (argc > 1)
  {
      status = SDS__OK;
      SdsRead(argv[1],&id,&status);
      if ((status == STATUS__OK)&&(argc > 2))
      	SdsFindByPath(id,argv[2],&id,&status);
      SdsList(id,&status);
      if (status != STATUS__OK)
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
  return 0;

}
