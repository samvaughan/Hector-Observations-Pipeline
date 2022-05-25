/*
*+                                s d s d u m p

 * Function name:
      sdspoke

 * Function:
   	Utility to insert data in to an SDS file.
   	
 *  Synopsis:
 	sdspoke [-1] component1 [component2...] 
   
 * Description:
     Read the Sds file and a list of element/value pairs from the
     stdin.  Poke the values as specified.  Note, if data item is
     not an array, treat it as a one element array.
   
      Example usages: 
          sdspoke < file.dat [-1] tdFconstants.sds field0.graspX

   file.dat has lines like:
   0    42
   1    54
   42  -99

   This would set graspX[0] = 42, graspX[1]=54, graspX[42]=-99

   If flag '-1' is specified array indices assumed to start from one 
   instead of zero.

   Remember to subtract one when needed.



 * Language:
      C
      
 * Author:  Karl Glazebrook, AAO. 
 * Support: Tony Farrell, AAO.

 *     @(#) $Id: ACMM:sds/sdspoke.c,v 3.94 09-Dec-2020 17:15:24+11 ks $
 *-

 * History:
      06-Dec-1996 - KGB - Original version
      28-Jan-1997 - TJF - Incorporated into SDS with comments reformated.
      				Remove global status. 
      04-Feb-1997 - TJF - dims and actval varlables should be unsigned.
      13-Feb-1997 - TJF - Include stdlib.h for malloc and free functions.
      21-Jan-2001 - TJF - Insert compilation fixes from N.Rees, JACH.
      18-Mar-2003 - TJF - More compilation warning fixes.
      17-Aug-2006 - TJF - Replace dimensions of 7 by SDS_C_MAXARRAYDIMS

 */



static const char *rcsId="@(#) $Id: ACMM:sds/sdspoke.c,v 3.94 09-Dec-2020 17:15:24+11 ks $";
static void *use_rcsId = (0 ? (void *)(&use_rcsId) : (void *) &rcsId);


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "sds.h"

#define BAD 1   /* Error exit status */

static void errcheck (StatusType status);

int main(int argc, char* argv[]) {

 int i, ioff;
 SdsIdType id,id2, topid;
 char name[32], *tok;
 long ndims,nvals;
 unsigned long dims[SDS_C_MAXARRAYDIMS],actlen;
 SdsCodeType code;
 StatusType status = STATUS__OK;

 /* Check number of args */

 if (argc<3 || argc>4) {
    printf("Usage: sdspoke <file.dat [-1] file.sds [compt1.compt...]\n");
    exit(1);
 }

 /* Loop for '-1' flag */

 ioff = 0;
 if (strcmp(argv[1],"-1") == 0) {
    ioff = 1;
    argv++;
    argc--;
 }
 
 SdsRead(argv[1],&id,&status); /* Slurp  file */
 errcheck(status);
 topid = id;

 /* Split name1.name2.name3 up */

 tok = strtok(argv[2],".");
 while(tok != NULL) { /*  Loop over components */
     SdsFind(id, tok, &id2, &status);
     errcheck(status);
     id = id2;
     tok = strtok(NULL,".");
  }

 SdsInfo(id,name,&code,&ndims, dims, &status);
 errcheck(status);
 nvals = 1;
 for(i=0; i<ndims; i++) 
    nvals *= dims[i];

 /* Now set data in the appropriate way */	

 switch (code) {

 case SDS_STRUCT:
    { 
      fprintf(stderr, "Error: Component is a structure\n");
      exit(BAD);
      break;
    }
 case SDS_CHAR:
 case SDS_UBYTE: 
    { unsigned char *array = (unsigned char *) malloc(nvals*sizeof(*array));
      long val;
      SdsGet(id,nvals*sizeof(*array),0,(void*)array,&actlen,&status);
      errcheck(status);
      while ( scanf("%d %ld", &i, &val)!=EOF ) { /* Read Data */
         i -= ioff;
         if (i>=nvals || i<0) {
            fprintf(stderr, 
                "Unable to set element %d in array of size %ld\n", i, nvals);
            exit(BAD);
          }
          array[i] = (unsigned char)val;
       }
      SdsPut(id,nvals*sizeof(*array),0,(void*)array,&status); 
      errcheck(status);
      break;
    }
 case SDS_USHORT:
    { unsigned short *array = (unsigned short *) malloc(nvals*sizeof(*array));
      long val;
      SdsGet(id,nvals*sizeof(*array),0,(void*)array,&actlen,&status);
      errcheck(status);
      while ( scanf("%d %ld", &i, &val)!=EOF ) { /* Read Data */
         i -= ioff;
         if (i>=nvals || i<0) {
            fprintf(stderr, 
                "Unable to set element %d in array of size %ld\n", i, nvals);
            exit(BAD);
          }
          array[i] = (unsigned short)val;
       }
      SdsPut(id,nvals*sizeof(*array),0,(void*)array,&status); 
      errcheck(status);
      break;
    }
 case SDS_SHORT:
    { short *array = (short *) malloc(nvals*sizeof(*array));
      long val;
      SdsGet(id,nvals*sizeof(*array),0,(void*)array,&actlen,&status);
      errcheck(status);
      while ( scanf("%d %ld", &i, &val)!=EOF ) { /* Read Data */
         i -= ioff;
         if (i>=nvals || i<0) {
            fprintf(stderr, 
                "Unable to set element %d in array of size %ld\n", i, nvals);
            exit(BAD);
          }
          array[i] = (short)val;
       }
      SdsPut(id,nvals*sizeof(*array),0,(void*)array,&status); 
      errcheck(status);
      break;
    }
 case SDS_UINT:
    { UINT32 *array = (UINT32 *) malloc(nvals*sizeof(*array));
      long val;
      SdsGet(id,nvals*sizeof(*array),0,(void*)array,&actlen,&status);
      errcheck(status);
      while ( scanf("%d %ld", &i, &val)!=EOF ) { /* Read Data */
         i -= ioff;
         if (i>=nvals || i<0) {
            fprintf(stderr, 
                "Unable to set element %d in array of size %ld\n", i, nvals);
            exit(BAD);
          }
          array[i] = (UINT32) val;
       }
      SdsPut(id,nvals*sizeof(*array),0,(void*)array,&status); 
      errcheck(status);
      break;
    }
 case SDS_INT:
    { INT32 *array = (INT32 *) malloc(nvals*sizeof(*array));
      long val;
      SdsGet(id,nvals*sizeof(*array),0,(void*)array,&actlen,&status);
      errcheck(status);
      while ( scanf("%d %ld", &i, &val)!=EOF ) { /* Read Data */
         i -= ioff;
         if (i>=nvals || i<0) {
            fprintf(stderr, 
                "Unable to set element %d in array of size %ld\n", i, nvals);
            exit(BAD);
          }
          array[i] = (INT32) val;
       }
      SdsPut(id,nvals*sizeof(*array),0,(void*)array,&status); 
      errcheck(status);
      break;
    }
 case SDS_FLOAT:
    { float *array = (float *) malloc(nvals*sizeof(*array));
      double val;
      SdsGet(id,nvals*sizeof(*array),0,(void*)array,&actlen,&status);
      errcheck(status);
      while ( scanf("%d %lf", &i, &val)!=EOF ) { /* Read Data */
         i -= ioff;
         if (i>=nvals || i<0) {
            fprintf(stderr, 
                "Unable to set element %d in array of size %ld\n", i, nvals);
            exit(BAD);
          }
          array[i] = (float)val;
       }
      SdsPut(id,nvals*sizeof(*array),0,(void*)array,&status); 
      errcheck(status);
      break;
    }
 case SDS_DOUBLE:
    { double *array = (double *) malloc(nvals*sizeof(*array));
      double val;
      SdsGet(id,nvals*sizeof(*array),0,(void*)array,&actlen,&status);
      errcheck(status);
      while ( scanf("%d %lf", &i, &val)!=EOF ) { /* Read Data */
         i -= ioff;
         if (i>=nvals || i<0) {
            fprintf(stderr, 
                "Unable to set element %d in array of size %ld\n", i, nvals);
            exit(BAD);
          }
          array[i] = (double) val;
       }
      SdsPut(id,nvals*sizeof(*array),0,(void*)array,&status); 
      errcheck(status);
      break;
    }
 case SDS_I64:
    { 
      fprintf(stderr, "Error: can not handle INT64 type\n");
      exit(BAD);
      break;
    }
 case SDS_UI64:
    { 
      fprintf(stderr, "Error: can not handle UINT64 type\n");
      exit(BAD);
      break;
    }
 default:
     printf("Error: unknown SDS data type: code = %d\n",(int)code);


 } /* End switch */

 SdsWrite(topid, argv[1], &status); /* Rewrite file */
 errcheck(status);

 return 0;
}

/* Check for SDS errors */

static void errcheck (StatusType status) {
    if (status!=SDS__OK) 
    {
        if (((status == SDS__FREAD)||(status == SDS__FOPEN))&&
            (errno != 0))
            perror("    File Read Error:");
        else
            fprintf(stderr, "Error: %s\n", SdsErrorCodeString(status));
        exit(1);
    }

}
