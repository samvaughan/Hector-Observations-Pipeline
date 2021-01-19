/*
 *       t e s t g e n . c
 *  
 *  Function:
 *        Creats a simple Arg structure and writes it to the file argtest.sds.
 *
 *        As well as testing some of the arg code, this generates the
 *        test file used by sdstest.c
 *  
 *  Author: Tony Farrell
 *  
 *     @(#) $Id: ACMM:sds/testgen.c,v 3.94 09-Dec-2020 17:15:24+11 ks $
 */
 


static const char *rcsId="@(#) $Id: ACMM:sds/testgen.c,v 3.94 09-Dec-2020 17:15:24+11 ks $";
static void *use_rcsId = (0 ? (void *)(&use_rcsId) : (void *) &rcsId);


#include <stdio.h>
#include "status.h"
#include "sds.h"
#include "arg.h"

extern int main(void)

{
  SdsIdType id;          /* SDS identifier  */
  StatusType status;     /* Inherited Status Variable  */
  unsigned i = 1111;     /* 1111 so we can see 0 going into it */

#ifdef NATIVE__INT64
  INT64 L64 = 0;
#endif

  status = STATUS__OK;

/*  Create a structure  */

  ArgNew(&id,&status);

/*  EXTRA TESTS - to help check problem found on 64 bit machines */

   ArgCvt("0",ARG_STRING,SDS_UINT,&i,sizeof(i),&status);
   ArgPutu(id,"Argument1",i,&status);


#ifdef NATIVE__INT64
    ArgCvt("64",ARG_STRING,SDS_I64,&L64,sizeof(L64),&status);
    ArgPuti64(id,"Argument2", L64, &status);
#endif

/*  Create an integer component  */

  ArgPuti(id, "Integer", 10, &status);

/*  Create a double component */

  ArgPutd(id, "Double", 20.0, &status);

/*  Create a float component  */

  ArgPutf(id, "Float", 30, &status);

/*  Create a string component  */

  ArgPutString(id, "String", "Hi There", &status);
  
/*  List the results */
  
  SdsList(id,&status);

  SdsWrite(id, "argtest.sds", &status);

  if (status == STATUS__OK)
      return 0;

  fprintf(stderr,"SDS Error=%s\n",
          SdsErrorCodeString(status));
  
  return(1);

}
