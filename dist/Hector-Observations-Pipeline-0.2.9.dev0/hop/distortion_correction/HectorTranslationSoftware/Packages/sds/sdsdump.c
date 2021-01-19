/*
*+                                s d s d u m p

 * Function name:
      sdsdump

 * Function:
        Dump part or all of an  sds file on stdout.

 *  Synopsis:
	sdsdump file [component[.component...]]

 * Description:
 	Use to dump all or part of an Sds file.

	   Example usages: 
	
	   (i)   sdsdump s22K15.sds unallocGuide.magnitude
	
	         - lists array 'magnitude' in structure unallocGuide.
	
	   (ii)  sdsdump s22K15.sds objects
	
	         - lists the contents of the structure 'objects'
	
	   (iii) sdsdump s22K15.sds
	
	         - lists all the structures
      

 * Language:
      C
      
 * Author:  Karl Glazebrook, AAO. 

 * Support: Tony Farrell, AAO.

 *     @(#) $Id: ACMM:sds/sdsdump.c,v 3.94 09-Dec-2020 17:15:24+11 ks $

 *-

 * History:
      11-Jun-1996 - KGB - Original version
      28-Jan-1997 - TJF - Incorporated into SDS with comments reformated.
      				Remove global status.
      04-Feb-1997 - TJF - dims and actlen variables should be unsigned.
      13-Feb-1997 - TJF - Include stdlib.h for malloc and free functions.
			  Include stdio.h for printf etc.
      21-Jan-2002 - TJF - Compilation warning fix from N.Rees (JACH).
      18-Feb-2003 - TJF - Fix compilation warnings.

 */


static const char *rcsId="@(#) $Id: ACMM:sds/sdsdump.c,v 3.94 09-Dec-2020 17:15:24+11 ks $";
static void *use_rcsId = (0 ? (void *)(&use_rcsId) : (void *) &rcsId);


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "sds.h"

static void errcheck (StatusType status);

int main(int argc, char* argv[]) {

  unsigned int i;
  SdsIdType id,id2;
  char name[32], *tok;
  long ndims;
  unsigned long dims[7];
  long nvals;
  unsigned long actlen;
  SdsCodeType code;
  StatusType status; /* Used in errcheck() */

  if (argc<2 || argc>3) {
     printf("Usage: sdsdump file.sds [compt1.compt...]\n");
     exit(1);
  }
 
 status = SDS__OK;
 SdsRead(argv[1],&id,&status); /* Slurp  file */
 errcheck(status);

 /* Split name1.name2.name3 up */

 if (argc==3) {
    tok = strtok(argv[2],".");
    while(tok != NULL) { /*  Loop over components */
        SdsFind(id, tok, &id2, &status);
        errcheck(status);
        id = id2;
        tok = strtok(NULL,".");
     }
 }

 SdsInfo(id,name,&code,&ndims, dims, &status);
 errcheck(status);
 nvals = 1;
 for(i=0; i<(unsigned)ndims; i++) 
    nvals *= dims[i];

 /* Now print in the appropriate way */	

 switch (code) {

 case SDS_STRUCT:
    { 
      SdsList(id, &status);
      if (status != SDS__OK) {
         printf("Error: accessing SDS structure\n");
         exit(1);
      }
      break;
    }
 case SDS_CHAR:
    { char *array = (char*) malloc(nvals*sizeof(*array));
      SdsGet(id,nvals*sizeof(*array),0,(void*)array,&actlen,&status);
      errcheck(status);
      if (ndims==1) 
         printf("%s\n",array);
      else if (ndims==2) {
         for(i=0; i<dims[1]; i++)
         printf("%s\n",array+dims[0]*i*sizeof(*array));
         free(array);
      }
      else {
         printf("Error: too many dimensions to char array\n");
         exit(1);
      }
      break;
    }
 case SDS_UBYTE: 
    { unsigned char *array = (unsigned char*) malloc(nvals*sizeof(*array));
      SdsGet(id,nvals*sizeof(*array),0,(void*)array,&actlen,&status);
      errcheck(status);
      for(i=0; i<actlen/sizeof(*array); i++) 
         printf("%ld\n",(long)array[i]);
      free(array);
      break;
    }
 case SDS_USHORT:
    { unsigned short *array = (unsigned short *) malloc(nvals*sizeof(*array));
      SdsGet(id,nvals*sizeof(*array),0,(void*)array,&actlen,&status);
      errcheck(status);
      for(i=0; i<actlen/sizeof(*array); i++) 
         printf("%ld\n",(long)array[i]);
      free(array);
      break;
    }
 case SDS_SHORT:
    { short *array = (short *) malloc(nvals*sizeof(*array));
      SdsGet(id,nvals*sizeof(*array),0,(void*)array,&actlen,&status);
      errcheck(status);
      for(i=0; i<actlen/sizeof(*array); i++) 
         printf("%ld\n",(long)array[i]);
      free(array);
      break;
    }
 case SDS_UINT:
    { UINT32 *array = (UINT32 *) malloc(nvals*sizeof(*array));
      SdsGet(id,nvals*sizeof(*array),0,(void*)array,&actlen,&status);
      errcheck(status);
      for(i=0; i<actlen/sizeof(*array); i++) 
         printf("%ld\n",(long)array[i]);
      free(array);
      break;
    }
 case SDS_INT:
    { INT32 *array = (INT32 *) malloc(nvals*sizeof(*array));
      SdsGet(id,nvals*sizeof(*array),0,(void*)array,&actlen,&status);
      errcheck(status);
      for(i=0; i<actlen/sizeof(*array); i++) 
         printf("%ld\n",(long)array[i]);
      free(array);
      break;
    }
 case SDS_FLOAT:
    { float *array = (float*) malloc(nvals*sizeof(*array));
      SdsGet(id,nvals*sizeof(*array),0,(void*)array,&actlen,&status);
      errcheck(status);
      for(i=0; i<actlen/sizeof(*array); i++) 
         printf("%.6g\n",array[i]);
      free(array);
      break;
    }
 case SDS_DOUBLE:
    { double *array = (double*) malloc(nvals*sizeof(*array));
      SdsGet(id,nvals*sizeof(*array),0,(void*)array,&actlen,&status);
      errcheck(status);
      for(i=0; i<actlen/sizeof(*array); i++) 
         printf("%.20g\n",array[i]);
      free(array);
      break;
    }

 case SDS_I64:
    { INT64 *array = (INT64 *) malloc(nvals*sizeof(*array));
      printf("Error: don't know how to print SDS_INT64 type\n");
      exit(1);
    }

 case SDS_UI64:
    { UINT64 *array = (UINT64 *) malloc(nvals*sizeof(*array));
      printf("Error: don't know how to print SDS_UINT64 type\n");
      exit(1);
    }

  default:
     printf("Error: unknown SDS data type: code = %d\n",(int)code);


 } /* End switch */

 return 0;
}

/* Check for SDS errors */

void errcheck (StatusType status) {


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
