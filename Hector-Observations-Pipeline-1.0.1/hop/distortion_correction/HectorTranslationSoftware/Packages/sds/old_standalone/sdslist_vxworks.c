

static char *rcsId="@(#) $Id: ACMM:sds/old_standalone/sdslist_vxworks.c,v 3.94 09-Dec-2020 17:15:34+11 ks $";
static void *use_rcsId = ((void)&use_rcsId,(void *) &rcsId);


#include "sds.h"

main(int argc, char* argv)

{
  long status;
  SdsIdType id;
 
  if (argc > 1)
    {
      status = SDS__OK;
      SdsRead(argv,&id,&status);
      SdsList(id,&status);
    }
}
