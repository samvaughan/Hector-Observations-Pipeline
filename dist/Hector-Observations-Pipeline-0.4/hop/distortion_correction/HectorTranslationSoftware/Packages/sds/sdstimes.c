

static const char *rcsId="@(#) $Id: ACMM:sds/sdstimes.c,v 3.94 09-Dec-2020 17:15:24+11 ks $";
static void *use_rcsId = (0 ? (void *)(&use_rcsId) : (void *) &rcsId);


#include <stdio.h>
#include "status.h"
#include "sds.h"
/*
 *     @(#) $Id: ACMM:sds/sdstimes.c,v 3.94 09-Dec-2020 17:15:24+11 ks $   
 */

extern void timer_start(void);
extern void cpu_used(double *);
extern void elapsed_time(double *);

int main()

{

   StatusType status;
   SdsIdType topid;
   SdsIdType sid;
   SdsIdType id[1000];
   char itname[1000][9];
   int i,ii;
   double cpu,elapsed;
   long j,act;
   
   status=0;
   timer_start();
   SdsCreate("Test",0,0,&topid,&status);
   for (i=1;i<=100;i++)
     {
        sprintf(itname[i]," ITEM%3.3d",i);
        itname[i][0] = 'A';
        itname[i][0] = itname[i][0] + (i % 25);
        SdsNew(topid,itname[i],0,0,7,0,0,&sid,&status);
     }
   cpu_used(&cpu);
   elapsed_time(&elapsed);
   printf(" creating sds structure  -  cpu %f  elapsed %f \n",cpu,elapsed);
   printf(" %d \n",status);

/*  Find items test  */

   timer_start();
   for (ii=1;ii<=10;ii++)
   {
   for (i=1;i<=100;i++)
     {
        SdsFind(topid,itname[i],&id[i],&status);
     }
   }
   cpu_used(&cpu);
   elapsed_time(&elapsed);
   printf(" Finding items in structure  -  cpu %f  elapsed %f \n",cpu,elapsed);
   printf(" %d \n",status);

/*  Put data test   */

   timer_start();
   for (ii=1;ii<=100;ii++)
   {
   for (i=1;i<=100;i++)
     {
        SdsPut(id[i],4,0,&i,&status);
     }
   }
   cpu_used(&cpu);
   elapsed_time(&elapsed);
   printf(" Putting data  -  cpu %f  elapsed %f \n",cpu,elapsed);
   printf(" %d \n",status);

/*  Get data test   */

   timer_start();
   for (ii=1;ii<=100;ii++)
   {
   for (i=1;i<=100;i++)
     {
        SdsGet(id[i],4,0,&j,(void *)&act,&status);
     }
   }
   cpu_used(&cpu);
   elapsed_time(&elapsed);
   printf(" Getting data  -  cpu %f  elapsed %f \n",cpu,elapsed);
   printf(" %d \n",status);

   return 0;
}
