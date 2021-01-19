

static char *rcsId="@(#) $Id: ACMM:sds/Testing/readtest_vxworks.c,v 3.94 09-Dec-2020 17:15:54+11 ks $";
static void *use_rcsId = ((void)&use_rcsId,(void *) &rcsId);


#include "sds.h"

typedef struct block
  {
    char c1;
    double d1;
    char c2;
    float f1;
    char c3;
    long l1;
    short s1;
    unsigned long ul1;
    char c4;
    unsigned short us1;
  } block;



SdsTestGet(SdsIdType topid, long *status)

{
  SdsIdType charid1,charid2;
  SdsIdType byteid1,byteid2;
  SdsIdType ubyteid1,ubyteid2;
  SdsIdType shortid1,shortid2;
  SdsIdType ushortid1,ushortid2;
  SdsIdType intid1,intid2;
  SdsIdType uintid1,uintid2;
  SdsIdType floatid1,floatid2;
  SdsIdType doubleid1,doubleid2;
  SdsIdType sid;
  int i,j;
  char cdata[260];
  char bdata[128];
  unsigned char ubdata[512];
  short sdata[256];
  unsigned short usdata[660];
  long idata[256];
  unsigned long uidata[40];
  float fdata[256];
  double ddata[20];
  char cdatas;
  unsigned long actlen;
  block block1;
  long *ptr;
  void *vptr;
  char cext[11];

/*  Get Data test  */

  if (*status != SDS__OK) return;
  SdsFind(topid,"junk",&charid1,status);
  if (*status != SDS__NOITEM)
    printf("SdsFind - failed to get NOITEM error %d\n",*status);
  *status = SDS__OK;
  SdsFind(topid,"Char",&charid1,status);
  SdsFind(charid1,"junk",&charid2,status);
  if (*status != SDS__NOTSTRUCT)
    printf("SdsFind - failed to get NOTSTRUCT error %d\n",*status);
  *status = SDS__OK;
  SdsFind(topid,"Chars",&charid2,status);
  SdsFind(topid,"Byte",&byteid1,status);
  SdsFind(topid,"Bytes",&byteid2,status);
  SdsFind(topid,"Ubyte",&ubyteid1,status);
  SdsFind(topid,"Ubytes",&ubyteid2,status);
  SdsFind(topid,"Short",&shortid1,status);
  SdsFind(topid,"Shorts",&shortid2,status);
  SdsFind(topid,"Ushort",&ushortid1,status);
  SdsFind(topid,"Ushorts",&ushortid2,status);
  SdsFind(topid,"Int",&intid1,status);
  SdsFind(topid,"Ints",&intid2,status);
  SdsFind(topid,"Uint",&uintid1,status);
  SdsFind(topid,"Uints",&uintid2,status);
  SdsFind(topid,"Float",&floatid1,status);
  SdsFind(topid,"Floats",&floatid2,status);
  SdsFind(topid,"Double",&doubleid1,status);
  SdsFind(topid,"Doubles",&doubleid2,status);
  SdsFind(topid,"Structure",&sid,status);
  if (*status != SDS__OK)
    {
      printf("Structure navigation error in TestGet %d\n",*status);
      printf("Test aborting\n");
      return;
    }
  SdsGetExtra(sid,10,cext,&actlen,status);
  cext[10] = 0;
  if ((*status != SDS__OK) || (strcmp(cext,"Changed!!!") != 0)) 
    {
      printf("Error getting extra information field %d\n",*status);
      printf(cext); printf("\n");
      *status = 2;
      printf("Test aborting\n");
      return;
    }

  SdsGet(charid1,1,0,&cdatas,&actlen,status);
  if ((*status != SDS__OK) || (cdatas != 'Q')) 
    {
      printf("Error getting char scalar %d %c\n",*status,cdatas);
      printf("Test aborting\n");
      *status = 2;
      return;
    }
  for (i=0;i<256;i++) cdata[i]=' ';
  SdsGet(charid2,256,0,cdata,&actlen,status);      
  if (*status != SDS__OK) 
    {
      printf("Error getting char array %d\n",*status);
      printf("Test aborting\n");
      return;
    }
  for (i=0;i<260;i+=26)
    for (j=0;j<26;j++)
      {
        if ((cdata[i+j] != ('a'+j)) && ((i+j) < 256))
          {
            printf("Error getting char array %c %d\n",cdata[i+j],i+j);
            printf("Test aborting\n");
            *status = 2;
            return;
          }
      }
  bdata[0]= 0;
  SdsGet(byteid1,1,0,bdata,&actlen,status);
  if ((*status != SDS__OK) || (bdata[0] != -127)) 
    {
      printf("Error getting byte scalar %d %d\n",bdata[0],*status);
      printf("Test aborting\n");
      *status = 2;
      return;
    }
  for (i=0;i<128;i++)
    bdata[i]=0;
  SdsGet(byteid2,128,0,bdata,&actlen,status);      
  if (*status != SDS__OK) 
    {
      printf("Error getting byte array %d\n",*status);
      printf("Test aborting\n");
      return;
    }
  for (i=0;i<128;i++)
    if (bdata[i] != i)
      {
        printf("Error getting byte array %d %d\n",i,bdata[i]);
        printf("Test aborting\n");
        *status = 2;
        return;
      }
  ubdata[0]= 0;
  SdsGet(ubyteid1,1,0,ubdata,&actlen,status);
  if ((*status != SDS__OK) || (ubdata[0] != 255)) 
    {
      printf("Error getting ubyte scalar %d %u\n",status,ubdata[0]);
      printf("Test aborting\n");
      *status = 2;
      return;
    }
  for (i=0;i<512;i++)
    ubdata[i]=0;
  SdsGet(ubyteid2,512,0,ubdata,&actlen,status);      
  if (*status != SDS__OK) 
    {
      printf("Error getting ubyte array %d\n",*status);
      printf("Test aborting\n");
      return;
    }
  for (i=0;i<512;i++)
    if (ubdata[i] != (i % 256))
      {
        printf("Error getting ubyte array %d %d\n",i,ubdata[i]);
        printf("Test aborting\n");
        *status = 2;
        return;
      }
  sdata[0]= 0;
  SdsGet(shortid1,2,0,sdata,&actlen,status);
  if ((*status != SDS__OK) || (sdata[0] != -32767)) 
    {
      printf("Error putting short scalar %d %d\n",sdata[0],*status);
      printf("Test aborting\n");
      *status = 2;
      return;
    }
  for (i=0;i<256;i++)
    sdata[i]=0;
  SdsGet(shortid2,256*2,0,sdata,&actlen,status);      
  if (*status != SDS__OK) 
    {
      printf("Error putting short array %d\n",*status);
      printf("Test aborting\n");
      return;
    }
  for (i=0;i<256;i++)
    if (sdata[i] != i*111)
      {
        printf("Error putting short array %d %d\n",i,sdata[i]);
        printf("Test aborting\n");
        *status = 2;
        return;
      }
  usdata[0]= 65535;
  SdsGet(ushortid1,2,0,usdata,&actlen,status);
  if ((*status != SDS__OK) || (usdata[0] != 65535)) 
    {
      printf("Error getting ushort scalar %d %u\n",status,usdata[0]);
      printf("Test aborting\n");
      *status = 2;
      return;
    }
  for (i=0;i<660;i++)
    usdata[i]=0;
  SdsGet(ushortid2,660*2,0,usdata,&actlen,status);      
  if (*status != SDS__OK) 
    {
      printf("Error getting ushort array %d\n",*status);
      printf("Test aborting\n");
      return;
    }
  for (i=0;i<660;i++)
    if (usdata[i] != i*11)
      {
        printf("Error getting ushort array %d %u\n",i,usdata[i]);
        printf("Test aborting\n");
        *status = 2;
        return;
      }
  idata[0]= 0;
  SdsGet(intid1,4,0,idata,&actlen,status);
  if ((*status != SDS__OK) || (idata[0] != -2147483647)) 
    {
      printf("Error getting int scalar %d %d\n",idata[0],*status);
      printf("Test aborting\n");
      *status = 2;
      return;
    }
  SdsPointer(intid1,&vptr,&actlen,status);
  ptr = (long*)vptr;
  if ((*ptr != -2147483647) || (*status != SDS__OK))
    {
      printf("Error getting int scalar via pointer %d %d\n",*ptr,*status);
      printf("Test aborting\n");
      *status = 2;
      return;
    }
  SdsFlush(intid1,status);
  for (i=0;i<256;i++)
    idata[i]= 0;
  SdsGet(intid2,256*4,0,idata,&actlen,status);      
  if (*status != SDS__OK) 
    {
      printf("Error getting int array %d\n",*status);
      printf("Test aborting\n");
      return;
    }
  for (i=0;i<256;i++)
    if (idata[i] != -i*111111)
      {
        printf("Error getting int array %d %d\n",i,idata[i]);
        printf("Test aborting\n");
        *status = 2;
        return;
      }
  uidata[0]= 0;
  SdsGet(uintid1,4,0,uidata,&actlen,status);
  if ((*status != SDS__OK) || uidata[0] != 4294967295) 
    {
      printf("Error getting uint scalar %d %d\n",uidata[0],*status);
      printf("Test aborting\n");
      *status = 2;
      return;
    }
  for (i=0;i<40;i++)
    uidata[i]= i*111111;
  SdsGet(uintid2,40*4,0,uidata,&actlen,status);      
  if (*status != SDS__OK) 
    {
      printf("Error putting uint array %d\n",*status);
      printf("Test aborting\n");
      return;
    }
  for (i=0;i<40;i++)
    if (uidata[i] != i*111111)
      {
        printf("Error putting uint array %d %u\n",i,uidata[i]);
        printf("Test aborting\n");
        *status = 2;
        return;
      }
  fdata[0]= 3.1415926;
  SdsGet(floatid1,4,0,fdata,&actlen,status);
  if ((*status != SDS__OK) || (fdata[0]>3.141593) || (fdata[0]<3.141592)) 
    {
      printf("Error getting float scalar %d %g\n",*status,fdata[0]);
      printf("Test aborting\n");
      *status = 2;
      return;
    }
  for (i=0;i<256;i++)
    fdata[i]= 0.0;
  SdsGet(floatid2,256*4,0,fdata,&actlen,status);      
  if (*status != SDS__OK) 
    {
      printf("Error getting float array %d\n",*status);
      printf("Test aborting\n");
      return;
    }
  for (i=0;i<256;i++)
    if ((fdata[i] > i*1.2345685) || (fdata[i] < i*1.234567))
      {
        printf("Error getting float array %d %g\n",i,fdata[i]);
        printf("Test aborting\n");
        *status = 2;
        return;
      }
  ddata[0]= 0.0;
  SdsGet(doubleid1,8,0,ddata,&actlen,status);
  if ((*status != SDS__OK) || (ddata[0] > 3.141592653590) ||
    (ddata[0] < 3.141592653589)) 
    {
      printf("Error getting double scalar %d %g\n",*status,ddata[i]);
      printf("Test aborting\n");
      *status = 2;
      return;
    }
  for (i=0;i<20;i++)
    ddata[i]= i;
  SdsGet(doubleid2,20*8,0,ddata,&actlen,status);      
  if (*status != SDS__OK) 
    {
      printf("Error putting double array %d\n",*status);
      printf("Test aborting\n");
      return;
    }
  for (i=0;i<20;i++)
    if ((ddata[i] > (i + 1e-10)) || (ddata[i] < (i - 1e-10)))
      {
        printf("Error putting double array %d\n",*status);
        printf("Test aborting\n");
        *status = 2;
        return;
      }
  SdsGet(sid,sizeof(block),0,&block1,&actlen,status);      
  if (*status != SDS__OK) 
    {
      printf("Error getting structure %d\n",*status);
      printf("Test aborting\n");
      return;
    }
  if ((block1.c1 != 'A') || (block1.d1 > 1.234567891) || (block1.d1 <
     1.234567889) || (block1.c2 != 'B') || (block1.f1 < 3.141592) ||
     (block1.f1 > 3.141593) || (block1.c3 != 'q') || (block1.l1 != 99999999)
     || (block1.s1 != -32767) || (block1.ul1 != 4000000000) ||
     (block1.c4 != 'z') || (block1.us1 != 65535))     
    {
      printf("Error getting structure %d\n",*status);
      printf("Test aborting\n");
      *status = 2;
      return;
    }

}



main(int argc, char* argv)

{
  long status;
  long id;
 
  if (argc > 1)
    {
      status = SDS__OK;
      SdsRead(argv,&id,&status);
      SdsTestGet(id,&status);
      if (status != SDS__OK)
        {
          printf("Error in readtest %x\n",status);
          return;
        }
      SdsList(id,&status);
    }
}
