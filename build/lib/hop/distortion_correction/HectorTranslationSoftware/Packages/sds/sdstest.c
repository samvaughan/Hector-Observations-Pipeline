
/*  SDS Test program  */


/*
 * History:
 *     05-Apr-2004 - TJF - Merged in changes from Nick Rees@JACH.  Note
 *                         that the use of "U" to create large unsigned
 *                         constants is likely to cause problems under
 *                         older VMS CC.
 *     11-Nov-2004 - TJF - Drop 64 bit arrays, but properly test 64 bit scalars. 
 *                         Reformat somewhat.
 *     17-Feb-2006 - TJF - Add support for reading sds_bigendian.sds and
 *                          sds_littleendian.sds  -CheckFile() function.  
 *                         Add PrintErrorCode() function - through it is
 *                          not used much yet.
 *     02-Mar-2007 - TJF - Fix bug resulting from UINT64 being defined
 *                          incorrectly for cases where long is 64 bits.
 *     09-Sep-2009 - TJF - Add  CheckMacros function, used to check sdsport.h.
 *     02-Nov-2011 - TJF - Convert all error status codes to strings for
 *                           output and use fprintf(stderr) rather then 
 *                           printf() to ensure we see them.
 *     10-Feb-2014 - TJF - Add checks of new SdsCloneId().
 */


static const char *rcsId="@(#) $Id: ACMM:sds/sdstest.c,v 3.94 09-Dec-2020 17:15:24+11 ks $";
static void *use_rcsId = (0 ? (void *)(&use_rcsId) : (void *) &rcsId);


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "drama.h"
#include "status.h"
#include "mess.h"
#include "sds.h"
#include "sds_err_msgt.h"


typedef struct block
{
    char c1;
    double d1;
    char c2;
    float f1;
    char c3;
    INT32 l1;
    short s1;
    UINT32 ul1;
    char c4;
    unsigned short us1;
} block;

 


void SdsTestPut(SdsIdType topid, StatusType *status)
{
    SdsIdType charid1,charid2;
    SdsIdType byteid1,byteid2;
    SdsIdType ubyteid1,ubyteid2;
    SdsIdType shortid1,shortid2;
    SdsIdType ushortid1,ushortid2;
    SdsIdType intid1,intid2;
    SdsIdType uintid1,uintid2;
    SdsIdType i64id1/*,i64id2*/;
    SdsIdType ui64id1/*,ui64id2*/;
    SdsIdType floatid1,floatid2;
    SdsIdType doubleid1,doubleid2;
    SdsIdType sid;
    SdsIdType ushortid3;
    int i,j;
    char cdata[260];
    char bdata[128];
    unsigned char ubdata[512];
    short sdata[256];
    unsigned short usdata[660];
    INT32 idata[256];
    UINT32 uidata[40];
    INT64 i64data;
    UINT64 ui64data;
    float fdata[256];
    double ddata[20];
    /*char cdatas;*/
    static block block1 = {'A', 1.234567890, 'B', 3.1415926, 'q', 99999999, -32767, 
                           4000000000U, 'z', (unsigned short)65535 };


/*  Put Data test  */

    SdsFind(topid,"Char",&charid1,status);
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
    SdsFind(topid,"I64",&i64id1,status);
    /*SdsFind(topid,"I64s",&i64id2,status);*/
    SdsFind(topid,"UI64",&ui64id1,status);
    /*SdsFind(topid,"UI64s",&ui64id2,status);*/
    SdsFind(topid,"Float",&floatid1,status);
    SdsFind(topid,"Floats",&floatid2,status);
    SdsFind(topid,"Double",&doubleid1,status);
    SdsFind(topid,"Doubles",&doubleid2,status);
    SdsFind(topid,"Structure",&sid,status);
    if (*status != SDS__OK)
    {
        fprintf(stderr,"Structure navigation error in TestPut - %s\n",SdsErrorCodeString(*status));
        fprintf(stderr,"Test aborting\n");
        return;
    }
    SdsFindByName(topid, "us1", 0, &ushortid3, status);
    if (*status != SDS__OK)
    {
        fprintf(stderr,"SdsFindByPath failed in TestPut - %s\n",SdsErrorCodeString(*status));
        fprintf(stderr,"Test aborting\n");
        return;
    }  

    SdsPutExtra(sid,10,"Changed!!!",status);
    if (*status != SDS__OK) 
    {
        fprintf(stderr,"Error putting extra information field - %s\n",SdsErrorCodeString(*status));
        fprintf(stderr,"Test aborting\n");
        return;
    }
    SdsPut(charid1,1,0,"Q",status);
    if (*status != SDS__OK) 
    {
        fprintf(stderr,"Error putting char scalar - %s\n",SdsErrorCodeString(*status));
        fprintf(stderr,"Test aborting\n");
        return;
    }
    for (i=0;i<260;i+=26)
        for (j=0;j<26;j++)
            cdata[i+j] = 'a'+j;
    SdsPut(charid2,256,0,cdata,status);      
    if (*status != SDS__OK) 
    {
        fprintf(stderr,"Error putting char array - %s\n",SdsErrorCodeString(*status));
        fprintf(stderr,"Test aborting\n");
        return;
    }
    bdata[0]= -127;
    SdsPut(byteid1,1,0,bdata,status);
    if (*status != SDS__OK) 
    {
        fprintf(stderr,"Error putting byte scalar - %s\n",SdsErrorCodeString(*status));
        fprintf(stderr,"Test aborting\n");
        return;
    }
    for (i=0;i<128;i++)
        bdata[i]=i;
    SdsPut(byteid2,128,0,bdata,status);      
    if (*status != SDS__OK) 
    {
        fprintf(stderr,"Error putting byte array - %s\n",SdsErrorCodeString(*status));
        fprintf(stderr,"Test aborting\n");
        return;
    }
    ubdata[0]= 255;
    SdsPut(ubyteid1,1,0,ubdata,status);
    if (*status != SDS__OK) 
    {
        fprintf(stderr,"Error putting ubyte scalar - %s\n",SdsErrorCodeString(*status));
        fprintf(stderr,"Test aborting\n");
        return;
    }
    for (i=0;i<512;i++)
        ubdata[i]=i % 256;
    SdsPut(ubyteid2,512,0,ubdata,status);      
    if (*status != SDS__OK) 
    {
        fprintf(stderr,"Error putting ubyte array - %s\n",SdsErrorCodeString(*status));
        fprintf(stderr,"Test aborting\n");
        return;
    }
    sdata[0]= -32767;
    SdsPut(shortid1,2,0,sdata,status);
    if (*status != SDS__OK) 
    {
        fprintf(stderr,"Error putting short scalar - %s\n",SdsErrorCodeString(*status));
        fprintf(stderr,"Test aborting\n");
        return;
    }
    for (i=0;i<256;i++)
        sdata[i]=i*111;
    SdsPut(shortid2,256*2,0,sdata,status);      
    if (*status != SDS__OK) 
    {
        fprintf(stderr,"Error putting short array - %s\n",SdsErrorCodeString(*status));
        fprintf(stderr,"Test aborting\n");
        return;
    }
    usdata[0]= 65535;
    SdsPut(ushortid1,2,0,usdata,status);
    if (*status != SDS__OK) 
    {
        fprintf(stderr,"Error putting ushort scalar - %s\n",SdsErrorCodeString(*status));
        fprintf(stderr,"Test aborting\n");
        return;
    }
    for (i=0;i<660;i++)
        usdata[i]=i*11;
    SdsPut(ushortid2,660*2,0,usdata,status);      
    if (*status != SDS__OK) 
    {
        fprintf(stderr,"Error putting ushort array - %s\n",SdsErrorCodeString(*status));
        fprintf(stderr,"Test aborting\n");
        return;
    }
    idata[0]= -2147483647;
    SdsPut(intid1,4,0,idata,status);
    if (*status != SDS__OK) 
    {
        fprintf(stderr,"Error putting int scalar - %s\n",SdsErrorCodeString(*status));
        fprintf(stderr,"Test aborting\n");
        return;
    }
    for (i=0;i<256;i++)
        idata[i]= -i*111111;
    SdsPut(intid2,256*4,0,idata,status);      
    if (*status != SDS__OK) 
    {
        fprintf(stderr,"Error putting int array - %s\n",SdsErrorCodeString(*status));
        fprintf(stderr,"Test aborting\n");
        return;
    }
    uidata[0]= 4294967295U;
    SdsPut(uintid1,4,0,uidata,status);
    if (*status != SDS__OK) 
    {
        fprintf(stderr,"Error putting uint scalar - %s\n",SdsErrorCodeString(*status));
        fprintf(stderr,"Test aborting\n");
        return;
    }
    for (i=0;i<40;i++)
        uidata[i]= i*111111;
    SdsPut(uintid2,40*4,0,uidata,status);      
    if (*status != SDS__OK) 
    {
        fprintf(stderr,"Error putting uint array - %s\n",SdsErrorCodeString(*status));
        fprintf(stderr,"Test aborting\n");
        return;
    }
    i64data = SdsSetI64(-100,20);
    SdsPut(i64id1,8,0,&i64data,status);
    if (*status != SDS__OK) 
    {
        fprintf(stderr,"Error putting i64 scalar - %s\n",SdsErrorCodeString(*status));
        fprintf(stderr,"Test aborting\n");
        return;
    }    
    /* 
     * The number we put above should work out as
     *    -429496729580
     */
#ifdef LONG__64
    printf("64 BIT data item is %ld (long is 64 bit)",i64data);
#if 1
    if (i64data != -429496729580l)
    {
        fprintf(stderr,"\nFailure constructing 64 bit signed integer, should be -429496729580\n");
        *status = SDS__TESTERR;
        return;
    }
    printf(" - which is right\n");
#else
    printf("\n");
#endif
#elif defined( NATIVE__INT64 )
    printf("64 BIT long item is %lld (native 64 bit (long long))",i64data);
#if 1
    if (i64data != -429496729580ll)
    {
        fprintf(stderr,"\nFailure constructing 64 bit signed integer, should be -429496729580 \n");
        *status = SDS__TESTERR;
        return;
    }
    printf(" - which is right\n");
#else
    printf("\n");
#endif
#else
    {
        double x = -100.0*SDS__ULONG_DIVSOR_D + 20.0; /* As done in sdsutil.c */
        printf("64 BIT long item is %.15g  (no long/native 64 bit)", x);
        if (x != -429496729580.0)
        {
            fprintf(stderr,"\nFailure constructing 64 big signed integer, should be -429496729580 \n");
            *status = SDS__TESTERR;
            return;
        }
        printf(" - which is right\n");
    }
#endif



    ui64data= SdsSetUI64(100,30);
    SdsPut(ui64id1,8,0,&ui64data,status);
    if (*status != SDS__OK) 
    {
        fprintf(stderr,"Error putting ui64 scalar - %s\n",SdsErrorCodeString(*status));
        fprintf(stderr,"Test aborting\n");
        return;
    }  

/*
 *  The number we put above should be 429496729630
 */
#ifdef LONG__64
    printf("64 BIT unsigned long item is %lu (long is 64 bit)",ui64data);
    if (ui64data !=  429496729630ul)
    {
        fprintf(stderr,"\nFailure constructing 64 bit unsigned integer, should be 429496729630\n");
        *status = SDS__TESTERR;
        return;
    }
    printf(" - which is right\n");
#elif defined( NATIVE__INT64 )
    printf("64 BIT unsigned long item is %llu (native 64 bit (unsigned long long)\n",ui64data);
    if (ui64data != 429496729630ull)
    {
        fprintf(stderr,"\nFailure constructing 64 bit unsigned integer, should be 429496729630\n");
        *status = SDS__TESTERR;
        return;
    }
    printf(" - which is right\n");
#else
    {
        double x = 100.0*SDS__ULONG_DIVSOR_D + 30.0; /* As done in sdsutil.c */
        printf("64 BIT unsigned long item is %.15g  (no long/native 64 bit)", x);
        if (x != 429496729630.0)
        {
            fprintf(stderr,"\nFailure constructing 64 big unsigned integer, should be 429496729630\n");
            *status = SDS__TESTERR;
            return;
        }
        printf(" - which is right\n");
    }
#endif




    fdata[0]= 3.1415926;
    SdsPut(floatid1,4,0,fdata,status);
    if (*status != SDS__OK) 
    {
        fprintf(stderr,"Error putting float scalar - %s\n",SdsErrorCodeString(*status));
        fprintf(stderr,"Test aborting\n");
        return;
    }
    for (i=0;i<256;i++)
        fdata[i]= i*1.23456789;
    SdsPut(floatid2,256*4,0,fdata,status);      
    if (*status != SDS__OK) 
    {
        fprintf(stderr,"Error putting float array - %s\n",SdsErrorCodeString(*status));
        fprintf(stderr,"Test aborting\n");
        return;
    }
    ddata[0]= 3.141592653589793238462643;
    SdsPut(doubleid1,8,0,ddata,status);
    if (*status != SDS__OK) 
    {
        fprintf(stderr,"Error putting double scalar - %s\n",SdsErrorCodeString(*status));
        fprintf(stderr,"Test aborting\n");
        return;
    }
    for (i=0;i<20;i++)
        ddata[i]= i;
    SdsPut(doubleid2,20*8,0,ddata,status);      
    if (*status != SDS__OK) 
    {
        fprintf(stderr,"Error putting double array - %s\n",SdsErrorCodeString(*status));
        fprintf(stderr,"Test aborting\n");
        return;
    }
    SdsPut(sid,sizeof(block),0,&block1,status);      
    if (*status != SDS__OK) 
    {
        fprintf(stderr,"Error putting structure - %s\n",SdsErrorCodeString(*status));
        fprintf(stderr,"Test aborting\n");
        return;
    }
}   



void SdsTestGet(SdsIdType topid, StatusType *status)

{
    SdsIdType charid1,charid2;
    SdsIdType byteid1,byteid2;
    SdsIdType ubyteid1,ubyteid2;
    SdsIdType shortid1,shortid2;
    SdsIdType ushortid1,ushortid2;
    SdsIdType intid1,intid2;
    SdsIdType uintid1,uintid2;
    SdsIdType i64id1/*,i64id2*/;
    SdsIdType ui64id1/*,ui64id2*/;
    SdsIdType floatid1,floatid2;
    SdsIdType doubleid1,doubleid2;
    SdsIdType undefDouble;
    SdsIdType sid;
    int i,j;
    char cdata[260];
    char bdata[128];
    unsigned char ubdata[512];
    short sdata[256];
    unsigned short usdata[660];
    INT32 idata[256];
    UINT32 uidata[40];
    float fdata[256];
    double ddata[20];
    char cdatas;
    unsigned long actlen;
    block block1;
    INT32 *ptr;
    void *vptr;
    char cext[11];
    INT64 i64data;
    UINT64 ui64data;
    long          i64high;
    unsigned long ui64high;
    unsigned long i64low;


/*  Get Data test  */

    if (*status != SDS__OK) return;
    SdsFind(topid,"junk",&charid1,status);
    if (*status != SDS__NOITEM)
        printf("SdsFind - failed to get NOITEM error, got - %s\n",SdsErrorCodeString(*status));
    *status = SDS__OK;
    SdsFind(topid,"Char",&charid1,status);
    SdsFind(charid1,"junk",&charid2,status);
    if (*status != SDS__NOTSTRUCT)
        printf("SdsFind - failed to get NOTSTRUCT error, got - %s\n",SdsErrorCodeString(*status));
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
    SdsFind(topid,"I64",&i64id1,status);
    /*SdsFind(topid,"I64s",&i64id2,status);*/
    SdsFind(topid,"UI64",&ui64id1,status);
    /*SdsFind(topid,"UI64s",&ui64id2,status);*/
    SdsFind(topid,"Float",&floatid1,status);
    SdsFind(topid,"Floats",&floatid2,status);
    SdsFind(topid,"Double",&doubleid1,status);
    SdsFind(topid,"Doubles",&doubleid2,status);
    SdsFind(topid,"undefDouble",&undefDouble,status);
    SdsFind(topid,"Structure",&sid,status);
    if (*status != SDS__OK)
    {
        fprintf(stderr,"Structure navigation error in TestGet - %s\n",SdsErrorCodeString(*status));
        fprintf(stderr,"Test aborting\n");
        return;
    }
    SdsGetExtra(sid,10,cext,&actlen,status);
    cext[10] = 0;
    if ((*status != SDS__OK) || (strcmp(cext,"Changed!!!") != 0)) 
    {
        fprintf(stderr,"Error getting extra information field - %s\n",SdsErrorCodeString(*status));
        fprintf(stderr,"%s", cext); fprintf(stderr,"\n");
        *status = SDS__TESTERR;
        fprintf(stderr,"Test aborting\n");
        return;
    }

    SdsGet(charid1,1,0,&cdatas,&actlen,status);
    if ((*status != SDS__OK) || (cdatas != 'Q')) 
    {
        fprintf(stderr,"Error getting char scalar %d %c\n",*status,cdatas);
        fprintf(stderr,"Test aborting\n");
        if (*status == SDS__OK) *status = SDS__TESTERR;
        return;
    }
    for (i=0;i<256;i++) cdata[i]=' ';
    SdsGet(charid2,256,0,cdata,&actlen,status);      
    if (*status != SDS__OK) 
    {
        fprintf(stderr,"Error getting char array - %s\n",SdsErrorCodeString(*status));
        fprintf(stderr,"Test aborting\n");
        return;
    }
    for (i=0;i<260;i+=26)
        for (j=0;j<26;j++)
        {
            if ((cdata[i+j] != ('a'+j)) && ((i+j) < 256))
            {
                fprintf(stderr,"Error getting char array %c %d\n",cdata[i+j],i+j);
                fprintf(stderr,"Test aborting\n");
                *status = SDS__TESTERR;
                return;
            }
        }
    bdata[0]= 0;
    SdsGet(byteid1,1,0,bdata,&actlen,status);
    if ((*status != SDS__OK) || (bdata[0] != -127)) 
    {
        fprintf(stderr,"Error getting byte scalar %d %d\n",bdata[0],*status);
        fprintf(stderr,"Test aborting\n");
        if (*status == SDS__OK) *status = SDS__TESTERR;
        return;
    }
    for (i=0;i<128;i++)
        bdata[i]=0;
    SdsGet(byteid2,128,0,bdata,&actlen,status);      
    if (*status != SDS__OK) 
    {
        fprintf(stderr,"Error getting byte array - %s\n",SdsErrorCodeString(*status));
        fprintf(stderr,"Test aborting\n");
        return;
    }
    for (i=0;i<128;i++)
        if (bdata[i] != i)
        {
            fprintf(stderr,"Error getting byte array %d %d\n",i,bdata[i]);
            fprintf(stderr,"Test aborting\n");
            *status = SDS__TESTERR;
            return;
        }
    ubdata[0]= 0;
    SdsGet(ubyteid1,1,0,ubdata,&actlen,status);
    if ((*status != SDS__OK) || (ubdata[0] != 255)) 
    {
        fprintf(stderr,"Error getting ubyte scalar %ld %u\n",(long)status,ubdata[0]);
        fprintf(stderr,"Test aborting\n");
        if (*status == SDS__OK) *status = SDS__TESTERR;
        return;
    }
    for (i=0;i<512;i++)
        ubdata[i]=0;
    SdsGet(ubyteid2,512,0,ubdata,&actlen,status);      
    if (*status != SDS__OK) 
    {
        fprintf(stderr,"Error getting ubyte array - %s\n",SdsErrorCodeString(*status));
        fprintf(stderr,"Test aborting\n");
        return;
    }
    for (i=0;i<512;i++)
        if (ubdata[i] != (i % 256))
        {
            fprintf(stderr,"Error getting ubyte array %d %d\n",i,ubdata[i]);
            fprintf(stderr,"Test aborting\n");
            *status = SDS__TESTERR;
            return;
        }
    sdata[0]= 0;
    SdsGet(shortid1,2,0,sdata,&actlen,status);
    if ((*status != SDS__OK) || (sdata[0] != -32767)) 
    {
        fprintf(stderr,"Error putting short scalar %d %d\n",sdata[0],*status);
        fprintf(stderr,"Test aborting\n");
        if (*status == SDS__OK) *status = SDS__TESTERR;
        return;
    }
    for (i=0;i<256;i++)
        sdata[i]=0;
    SdsGet(shortid2,256*2,0,sdata,&actlen,status);      
    if (*status != SDS__OK) 
    {
        fprintf(stderr,"Error putting short array - %s\n",SdsErrorCodeString(*status));
        fprintf(stderr,"Test aborting\n");
        return;
    }
    for (i=0;i<256;i++)
        if (sdata[i] != i*111)
        {
            fprintf(stderr,"Error putting short array %d %d\n",i,sdata[i]);
            fprintf(stderr,"Test aborting\n");
            *status = SDS__TESTERR;
            return;
        }
    usdata[0]= 65535;
    SdsGet(ushortid1,2,0,usdata,&actlen,status);
    if ((*status != SDS__OK) || (usdata[0] != 65535)) 
    {
        fprintf(stderr,"Error getting ushort scalar %ld %u\n",(long)status,usdata[0]);
        fprintf(stderr,"Test aborting\n");
        if (*status == SDS__OK) *status = SDS__TESTERR;
        return;
    }
    for (i=0;i<660;i++)
        usdata[i]=0;
    SdsGet(ushortid2,660*2,0,usdata,&actlen,status);      
    if (*status != SDS__OK) 
    {
        fprintf(stderr,"Error getting ushort array - %s\n",SdsErrorCodeString(*status));
        fprintf(stderr,"Test aborting\n");
        return;
    }
    for (i=0;i<660;i++)
        if (usdata[i] != i*11)
        {
            fprintf(stderr,"Error getting ushort array %d %u\n",i,usdata[i]);
            fprintf(stderr,"Test aborting\n");
            *status = SDS__TESTERR;
            return;
        }
    idata[0]= 0;
    SdsGet(intid1,4,0,idata,&actlen,status);
    if ((*status != SDS__OK) || (idata[0] != -2147483647)) 
    {
        fprintf(stderr,"Error getting int scalar %d %d\n",idata[0],*status);
        fprintf(stderr,"Test aborting\n");
        *status = SDS__TESTERR;
        return;
    }
    SdsPointer(intid1,&vptr,&actlen,status);
    ptr = (INT32*)vptr;
    if ((*ptr != -2147483647) || (*status != SDS__OK))
    {
        fprintf(stderr,"Error getting int scalar via pointer %d %d\n",*ptr,*status);
        fprintf(stderr,"Test aborting\n");
        if (*status == SDS__OK) *status = SDS__TESTERR;
        return;
    }
    SdsFlush(intid1,status);
    for (i=0;i<256;i++)
        idata[i]= 0;
    SdsGet(intid2,256*4,0,idata,&actlen,status);      
    if (*status != SDS__OK) 
    {
        fprintf(stderr,"Error getting int array - %s\n",SdsErrorCodeString(*status));
        fprintf(stderr,"Test aborting\n");
        return;
    }
    for (i=0;i<256;i++)
        if (idata[i] != -i*111111)
        {
            fprintf(stderr,"Error getting int array %d %d\n",i,idata[i]);
            fprintf(stderr,"Test aborting\n");
            *status = SDS__TESTERR;
            return;
        }
    uidata[0]= 0;
    SdsGet(uintid1,4,0,uidata,&actlen,status);
    if ((*status != SDS__OK) || uidata[0] != 4294967295U) 
    {
        fprintf(stderr,"Error getting uint scalar %d %d\n",uidata[0],*status);
        fprintf(stderr,"Test aborting\n");
        if (*status == SDS__OK) *status = SDS__TESTERR;
        return;
    }
    for (i=0;i<40;i++)
        uidata[i]= i*111111;
    SdsGet(uintid2,40*4,0,uidata,&actlen,status);      
    if (*status != SDS__OK) 
    {
        fprintf(stderr,"Error putting uint array - %s\n",SdsErrorCodeString(*status));
        fprintf(stderr,"Test aborting\n");
        return;
    }
    for (i=0;i<40;i++)
        if (uidata[i] != (UINT32)i*111111)
        {
            fprintf(stderr,"Error putting uint array %d %u\n",i,uidata[i]);
            fprintf(stderr,"Test aborting\n");
            *status = SDS__TESTERR;
            return;
        }
    fdata[0]= 3.1415926;
    SdsGet(floatid1,4,0,fdata,&actlen,status);
    if ((*status != SDS__OK) || (fdata[0]>3.141593) || (fdata[0]<3.141592)) 
    {
        fprintf(stderr,"Error getting float scalar %d %g\n",*status,fdata[0]);
        fprintf(stderr,"Test aborting\n");
        if (*status == SDS__OK) *status = SDS__TESTERR;
        return;
    }
    for (i=0;i<256;i++)
        fdata[i]= 0.0;
    SdsGet(floatid2,256*4,0,fdata,&actlen,status);      
    if (*status != SDS__OK) 
    {
        fprintf(stderr,"Error getting float array - %s\n",SdsErrorCodeString(*status));
        fprintf(stderr,"Test aborting\n");
        return;
    }
    for (i=0;i<256;i++)
        if ((fdata[i] > i*1.2345685) || (fdata[i] < i*1.234567))
        {
            fprintf(stderr,"Error getting float array %d %g\n",i,fdata[i]);
            fprintf(stderr,"Test aborting\n");
            if (*status == SDS__OK) *status = SDS__TESTERR;
            return;
        }
    ddata[0]= 0.0;
    SdsGet(doubleid1,8,0,ddata,&actlen,status);
    if ((*status != SDS__OK) || (ddata[0] > 3.141592653590) ||
        (ddata[0] < 3.141592653589)) 
    {
        fprintf(stderr,"Error getting double scalar %d %g\n",*status,ddata[i]);
        fprintf(stderr,"Test aborting\n");
        if (*status == SDS__OK) *status = SDS__TESTERR;
        return;
    }
    for (i=0;i<20;i++)
        ddata[i]= i;
    SdsGet(doubleid2,20*8,0,ddata,&actlen,status);      
    if (*status != SDS__OK) 
    {
        fprintf(stderr,"Error putting double array - %s\n",SdsErrorCodeString(*status));
        fprintf(stderr,"Test aborting\n");
        return;
    }
    for (i=0;i<20;i++)
        if ((ddata[i] > (i + 1e-10)) || (ddata[i] < (i - 1e-10)))
        {
            fprintf(stderr,"Error putting double array - %s\n",SdsErrorCodeString(*status));
            fprintf(stderr,"Test aborting\n");
            *status = SDS__TESTERR;
            return;
        }



    SdsGet(i64id1,8,0,&i64data,&actlen, status);
    if (*status != SDS__OK) 
    {
        fprintf(stderr,"Error getting i64 scalar - %s\n",SdsErrorCodeString(*status));
        fprintf(stderr,"Test aborting\n");
        return;
    }    
    SdsGet(ui64id1,8,0,&ui64data,&actlen,status);
    if (*status != SDS__OK) 
    {
        fprintf(stderr,"Error getting ui64 scalar - %s\n",SdsErrorCodeString(*status));
        fprintf(stderr,"Test aborting\n");
        return;
    }  
    SdsGetI64(i64data, &i64high, &i64low);
    if ((i64high != -100)||(i64low != 20))
    {
        fprintf(stderr,"Error getting i64 scalar - value is wrong\n");
        fprintf(stderr," Expected (-100, 20), got %ld, %ld\n", i64high, i64low);
        fprintf(stderr," SdsSetI64()/SdsGetI64() are not working correctly\n");
        fprintf(stderr,"Test aborting\n");
        *status = SDS__TESTERR;
        return;

    }
    SdsGetUI64(ui64data, &ui64high, &i64low);
    if ((ui64high != 100)||(i64low != 30))
    {
        fprintf(stderr,"Error getting ui64 scalar - value is wrong\n");
        fprintf(stderr,"Expected (100, 30), got %ld, %ld\n", ui64high, i64low);
        fprintf(stderr," SdsSetUI64()/SdsGetUI64() are not working correctly\n");
        fprintf(stderr,"Test aborting\n");
        *status = SDS__TESTERR;
        return;

    }


    SdsGet(undefDouble,8,0,ddata,&actlen,status);
    if (*status != SDS__UNDEFINED)
    {
        fprintf(stderr,"Error getting undefined double scalar %d %g\n",*status,ddata[i]);
        fprintf(stderr,"Test aborting\n");
        if (*status == SDS__OK) *status = SDS__TESTERR;
        return;
    }
    else 
        *status = SDS__OK;

    if ( SdsIsDefined( undefDouble, status ) || *status != SDS__OK )
    {
        fprintf(stderr,"Error using SdsIsDefined on undefined item - %s\n",SdsErrorCodeString(*status));
        fprintf(stderr,"Test aborting\n");
        if (*status == SDS__OK) *status = SDS__TESTERR;
        return;
    }

    if ( !SdsIsDefined( doubleid1, status ) || *status != SDS__OK )
    {
        fprintf(stderr,"Error using SdsIsDefined on defined item - %s\n",SdsErrorCodeString(*status));
        fprintf(stderr,"Test aborting\n");
        if (*status == SDS__OK) *status = SDS__TESTERR;
        return;
    }

    if ( SdsIsDefined( topid, status ) || *status != SDS__NOTPRIM )
    {
        fprintf(stderr,"Error using SdsIsDefined on structured item - %s\n",SdsErrorCodeString(*status));
        fprintf(stderr,"Test aborting\n");
        if (*status == SDS__OK) *status = SDS__TESTERR;
        return;
    }
    else 
        *status = SDS__OK;

    SdsGet(sid,sizeof(block),0,&block1,&actlen,status);      
    if (*status != SDS__OK) 
    {
        fprintf(stderr,"Error getting structure - %s\n",SdsErrorCodeString(*status));
        fprintf(stderr,"Test aborting\n");
        return;
    }
    if ((block1.c1 != 'A') || (block1.d1 > 1.234567891) || (block1.d1 <
                                                            1.234567889) || (block1.c2 != 'B') || (block1.f1 < 3.141592) ||
        (block1.f1 > 3.141593) || (block1.c3 != 'q') || (block1.l1 != 99999999)
        || (block1.s1 != -32767) || (block1.ul1 != 4000000000U) ||
        (block1.c4 != 'z') || (block1.us1 != 65535))     
    {
        fprintf(stderr,"Error getting structure - %s\n",SdsErrorCodeString(*status));
        fprintf(stderr,"Test aborting\n");
        *status = SDS__TESTERR;
        return;
    }

}

static void CheckMacros()
{
    printf("Will output all macros used by \"sdsport.h\"\n");
#ifdef MAKE_VXWORKS
    printf("MAKE_VXWORKS=%d\n", MAKE_VXWORKS);
#else
    printf("MAKE_VXWORKS undefined\n");
#endif
#ifdef VxWorks
    printf("VxWorks=%d\n", VxWorks);
#else
    printf("VxWorks undefined\n");
#endif



#ifdef _WRS_VXWORKS_5_X
    printf("_WRS_VXWORKS_5_X=%d\n", _WRS_VXWORKS_5_X);
#else
    printf("_WRS_VXWORKS_5_X undefined\n");
#endif

#ifdef __cplusplus
    printf("__cplusplus=%ld\n", (long)(__cplusplus));
#else
    printf("__cplusplus undefined\n");
#endif

#ifdef __STDC__
    printf("__STDC__=%d\n", __STDC__);
#else
    printf("__STDC__ undefined\n");
#endif

#ifdef VAXC
    printf("VAXC=%d\n", VAXC);
#else
    printf("VAXC undefined\n");
#endif
#ifdef VAX
    printf("VAX=%d\n", VAX);
#else
    printf("VAX undefined\n");
#endif

#ifdef M_I86
    printf("M_I86=defined\n");
#else
    printf("M_I86 undefined\n");
#endif


#ifdef __i386__
    printf("__i386__=%d\n", __i386__);
#else
    printf("__i386__ undefined\n");
#endif
#ifdef __x86_64
    printf("__x86_64=%d\n", __x86_64);
#else
    printf("__x86_64 undefined\n");
#endif

#ifdef WIN32
    printf("WIN32=%d\n", WIN32);
#else
    printf("WIN32 undefined\n");
#endif

#ifdef macintosh
    printf("macintosh=%d\n", macintosh);
#else
    printf("macintosh undefined\n");
#endif

#ifdef VMS
    printf("VMS=%d\n", VMS);
#else
    printf("VMS undefined\n");
#endif

#ifdef __G_FLOAT
    printf("__G_FLOAT=%d\n", __G_FLOAT);
#else
    printf("__G_FLOAT undefined (VMS Float type)\n");
#endif


#ifdef __vms
    printf("__vms=%d\n", __vms);
#else
    printf("__vms undefined\n");
#endif

#ifdef __alpha
    printf("__alpha=%d\n", __alpha);
#else
    printf("__alpha undefined\n");
#endif

#ifdef __AlphaAXP
    printf("__AlphaAXP=%d\n", __AlphaAXP);
#else
    printf("__AlphaAXP undefined\n");
#endif

#ifdef __32BITS  /* Defined by DECC */
    printf("__32BITS=%d\n", __32BITS);
#else
    printf("__32BITS undefined (DECC macro)\n");
#endif

#ifdef __INITIAL_POINTER_SIZE  /* Defined by DECC */
    printf("__INITIAL_POINTER_SIZE=%d\n", __INITIAL_POINTER_SIZE);
#else
    printf("__INITIAL_POINTER_SIZE undefined (DECC macro)\n");
#endif




#ifdef __GNUC__
    printf("__GNUC__=%d\n", __GNUC__);
#else
    printf("__GNUC__ undefined\n");
#endif

#ifdef __GNUC_MINOR__
    printf("__GNUC_MINOR__=%d\n", __GNUC_MINOR__);
#else
    printf("__GNUC_MINOR__ undefined\n");
#endif


#ifdef SDS_GNUC_NO_LONG_LONG
    printf("SDS_GNUC_NO_LONG_LONG=%d\n", SDS_GNUC_NO_LONG_LONG);
#else
    printf("SDS_GNUC_NO_LONG_LONG undefined\n");
#endif

#ifdef _MSC_VER
    printf("_MSC_VER=%d\n", _MSC_VER);
#else
    printf("_MSC_VER undefined\n");
#endif

#ifdef __LP64__
    printf("__LP64__=%d\n", __LP64__);
#else
    printf("__LP64__ undefined\n");
#endif

#ifdef _LP64
    printf("_LP64=%d\n", _LP64);
#else
    printf("_LP64 undefined\n");
#endif

#ifdef UINT64_MAX
    printf("UINT64_MAX defined\n");
#else
    printf("UINT64_MAX undefined\n");
#endif

#ifdef __STDC__
    printf("__STDC__=%d\n", __STDC__);
#else
    printf("__STDC__ undefined\n");
#endif

#ifdef SDS__64_BECAUSE
    printf("Have native 64 bit integer types because:%s\n", SDS__64_BECAUSE);
#else
    printf("Do not have native 64 bit integer types - am emulating where possible\n");
#endif





    /*
     * * * * * * * * * * * * * * * * * * * * * * * * * * * *
     */

    printf("\nChecking sizes of types\n");
    printf("Size of int=%ld, size of long int = %ld, size of pointer = %ld (bytes))\n",
           (long)sizeof(int), (long)sizeof(long int), (long)sizeof(void *));
    if (sizeof(INT32) < 4)
    {
        fprintf(stderr,"SDS ERROR:SDS INT32 type is too small.  %ld bits, must be at least 32\n",
                (long)(sizeof(INT32)*8));
        exit(1);
    }
    if (sizeof(UINT32) < 4)
    {
        fprintf(stderr,"SDS ERROR:SDS UINT32 type is too small.  %ld bits, must be at least 32\n",
                (long)(sizeof(INT32)*8));
        exit(1);
    }
    if (sizeof(INT64) < 8)
    {
        fprintf(stderr,"SDS ERROR:SDS INT64 type is too small.  %ld bits, must be at least 64\n",
                (long)(sizeof(INT32)*8));
        exit(1);
    }
    if (sizeof(UINT64) < 8)
    {
        fprintf(stderr,"SDS ERROR:SDS UINT64 type is too small.  %ld bits, must be at least 64\n",
                (long)(sizeof(INT32)*8));
        exit(1);
    }  

    /*
     * * * * * * * * * * * * * * * * * * * * * * * * * * * *
     */


    printf("Now outputing results of \"sdsport.h\"\n");
#ifdef INT64_ALREADY_DEFINED
    printf("The INT64 type was already defined by the OS (likely VxWorks build)\n");
#else
    printf("The INT64 type was NOT already defined by the OS (normally ok)\n");
#endif

#if SDS_BIGEND
    printf("sdsport.h thinks this machine is big endian.  Fix sdsport.h if not\n");
#else
    printf("sdsport.h thinks this machine is little endian.  Fix sdsport.h if not\n");
#endif    

#ifdef NATIVE__INT64
    printf("Compiler supports a native 64 bit integer (NATIVE__INT64 defined)\n");
#else
    printf("Compiler does not supports a native 64 bit integer (NATIVE__INT64 undefined)\n");
#endif

#ifdef LONG__64
    printf("The \"long int\" type is 64 bits (LONG__64 defined)\n");
    if (sizeof(long int) != 8)
    {
        fprintf(stderr, "SDS ERROR:LONG__64 macro defined, long int type is %ld not 64 bits\n",
                (long)(sizeof(long int)*8));
        exit(1);
    }
#else
    printf("The \"long int\" type is 32 bits (LONG__64 undefined)\n");
    if (sizeof(long int) != 4)
    {
        fprintf(stderr, "SDS ERROR:LONG__64 macro undefined, long int type is %ld, not 32 bits\n",
                (long)(sizeof(long int)*8));
        exit(1);
    }

#endif
    printf("Check of \"sdsport.h\" complete\n\n");


}

static void CheckFile(const char *name,  StatusType *status);
static void PrintErrorCode(StatusType status);

int main()


{
    SdsIdType topid,id1,id2,id3,id4,id5,id6,pid,idc,ide;
    SdsIdType charid1,charid2;
    SdsIdType byteid1,byteid2;
    SdsIdType ubyteid1,ubyteid2;
    SdsIdType shortid1,shortid2;
    SdsIdType ushortid1,ushortid2;
    SdsIdType intid1,intid2;
    SdsIdType uintid1,uintid2;
    SdsIdType floatid1,floatid2;
    SdsIdType doubleid1,doubleid2,undefDouble;
    SdsIdType i64id1/*,i64id2*/;
    SdsIdType ui64id1/*,ui64id2*/;
    SdsIdType sid,tid;
    SdsIdType idq,idcop,idex,idetop,id9;
    StatusType status;
    unsigned long dims[8];
/*
  int i,j;
  unsigned long actlen;
*/
    unsigned long size;
    void *buffer,*buffer2;
    void *buf2;
    void *buf3;
    void *junk;
    unsigned long osize;

    int abort;

    /*
     * Allow us to translate error codes
     */
    MessPutFacility(&MessFac_SDS);



    /*
     * Check sdsport.h
     */
    CheckMacros();

    /*
     * The error messages below help explain what this code is doing.
     */ 


    /*
     * Check build
     */
    status = SDS__OK;
    SdsCheckBuild(&status);
    if (status != SDS__OK)
    {
        fprintf(stderr, "Failed build consistency check\n");
        fprintf(stderr, "Test aborting\n");
        return -1;
    }



    /*SdsSetWatch(-1, 0, 0, 0, 0, 0, &status);*/

    abort = 0;
    CheckFile("sds_vmsalpha.sds", &status);
    if (status != SDS__OK)
    {
       fprintf(stderr,"Failed to read file written by VMS/Alpha machine\n");
       PrintErrorCode(status);
       abort = 1;
       status = STATUS__OK;
    }
    else 
        printf("Read sds_vmsalpha.sds file ok\n");


    CheckFile("sds_bigendian.sds", &status);
    if (status != SDS__OK)
    {
       fprintf(stderr,"Failed to read file written by big-endian byte order IEEE Floats, machine\n");
       PrintErrorCode(status);
       abort = 1;
       status = STATUS__OK;
    }
    else
        printf("Read sds_bigendian.sds file ok\n");
    CheckFile("sds_littleendian.sds", &status);
    if (status != SDS__OK)
    {
       fprintf(stderr,"Failed to read file written by little-endian byte order, IEEE Floats,  machine\n");
       PrintErrorCode(status);
       abort = 1;
       status = STATUS__OK;
    }
    else
        printf("Read sds_littleendian.sds file ok\n");

    if (abort)
    {
        fprintf(stderr,"File read error aborts test\n");
        return 1;
    }

    /*
     * Checking for various errors.
     */
    SdsNew(-1,"Top",0,NULL,SDS_STRUCT,0,NULL,&topid,&status);
    if (status != SDS__BADID)
        fprintf(stderr,"SdsNew - Failed to get BADID error - %s\n",SdsErrorCodeString(status));
    status = SDS__OK;



    SdsNew(0,"12345678901234567890",0,NULL,SDS_STRUCT,0,NULL,&topid,&status);
    if (status != SDS__LONGNAME)
        fprintf(stderr,"SdsNew - Failed to get LONGNAME error - %s\n",SdsErrorCodeString(status));
    status = SDS__OK;
    SdsNew(0,"Top",129,NULL,SDS_STRUCT,0,NULL,&topid,&status);
    if (status != SDS__EXTRA)
        fprintf(stderr,"SdsNew - Failed to get EXTRA error - %s\n",SdsErrorCodeString(status));
    status = SDS__OK;
    SdsNew(0,"Top",0,NULL,999,0,NULL,&topid,&status);
    if (status != SDS__INVCODE)
        fprintf(stderr,"SdsNew - Failed to get INVCODE error - %s\n",SdsErrorCodeString(status));
    status = SDS__OK;
    SdsNew(0,"Top",0,NULL,SDS_STRUCT,8,NULL,&topid,&status);
    if (status != SDS__INVDIMS)
        fprintf(stderr,"SdsNew - Failed to get INVDIMS error - %s\n",SdsErrorCodeString(status));
    status = SDS__OK;
    SdsNew(0,"Top",0,NULL,SDS_STRUCT,-1,NULL,&topid,&status);
    if (status != SDS__INVDIMS)
        fprintf(stderr,"SdsNew - Failed to get INVDIMS error - %s\n",SdsErrorCodeString(status));
    status = SDS__OK;
    SdsNew(0,"Top",0,NULL,SDS_INT,0,NULL,&pid,&status);
    if (status != SDS__OK)
        fprintf(stderr,"SdsNew - Failure on primitive creation - %s\n",SdsErrorCodeString(status));
    status = SDS__OK;
    SdsNew(pid,"Top",0,NULL,SDS_INT,0,NULL,&id1,&status);
    if (status != SDS__NOTSTRUCT)
        fprintf(stderr,"SdsNew - Failure to get NOTSTRUCT error - %s\n",SdsErrorCodeString(status));
    status = SDS__OK;

    /* Now actually create some stuff */

    SdsNew(0,"Top",0,NULL,SDS_STRUCT,0,NULL,&topid,&status);
    if (status != SDS__OK)
    {
        fprintf(stderr,"SdsNew - Failure on structure creation %ld\n",(long)status);
        fprintf(stderr,"Test aborting\n");
        return(-1);
    }
    else
        printf("Structure creation test completed \n");
 
/*  Primitive creation as elements of a structure  */

    SdsNew(topid,"Char",0,NULL,SDS_CHAR,0,NULL,&charid1,&status);
    if (status != SDS__OK)
    {
        fprintf(stderr,"Error creating character scalar - %s\n",SdsErrorCodeString(status));
        fprintf(stderr,"Test aborting\n");
        return(-1);
    }
    dims[0] = 256;
    SdsNew(topid,"Chars",0,NULL,SDS_CHAR,1,dims,&charid2,&status);
    if (status != SDS__OK)
    {
        fprintf(stderr,"Error creating character array - %s\n",SdsErrorCodeString(status));
        fprintf(stderr,"Test aborting\n");
        return(-1);
    }
    SdsNew(topid,"Byte",0,NULL,SDS_BYTE,0,NULL,&byteid1,&status);
    if (status != SDS__OK)
    {
        fprintf(stderr,"Error creating byte scalar - %s\n",SdsErrorCodeString(status));
        fprintf(stderr,"Test aborting\n");
        return(-1);
    }
    dims[0] = 2;
    dims[1] = 2;
    dims[2] = 2;
    dims[3] = 2;
    dims[4] = 2;
    dims[5] = 2;
    dims[6] = 2;
    SdsNew(topid,"Bytes",0,NULL,SDS_BYTE,7,dims,&byteid2,&status);
    if (status != SDS__OK)
    {
        fprintf(stderr,"Error creating byte array - %s\n",SdsErrorCodeString(status));
        fprintf(stderr,"Test aborting\n");
        return(-1);
    }
    SdsNew(topid,"Ubyte",0,NULL,SDS_UBYTE,0,NULL,&ubyteid1,&status);
    if (status != SDS__OK)
    {
        fprintf(stderr,"Error creating ubyte scalar - %s\n",SdsErrorCodeString(status));
        fprintf(stderr,"Test aborting\n");
        return(-1);
    }
    dims[0] = 256;
    dims[1] = 2;
    SdsNew(topid,"Ubytes",0,NULL,SDS_UBYTE,2,dims,&ubyteid2,&status);
    if (status != SDS__OK)
    {
        fprintf(stderr,"Error creating ubyte array - %s\n",SdsErrorCodeString(status));
        fprintf(stderr,"Test aborting\n");
        return(-1);
    }
    SdsNew(topid,"Short",0,NULL,SDS_SHORT,0,NULL,&shortid1,&status);
    if (status != SDS__OK)
    {
        fprintf(stderr,"Error creating short scalar - %s\n",SdsErrorCodeString(status));
        fprintf(stderr,"Test aborting\n");
        return(-1);
    }
    dims[0] = 256;
    SdsNew(topid,"Shorts",0,NULL,SDS_SHORT,1,dims,&shortid2,&status);
    if (status != SDS__OK)
    {
        fprintf(stderr,"Error creating short array - %s\n",SdsErrorCodeString(status));
        fprintf(stderr,"Test aborting\n");
        return(-1);
    }
    SdsNew(topid,"Ushort",0,NULL,SDS_USHORT,0,NULL,&ushortid1,&status);
    if (status != SDS__OK)
    {
        fprintf(stderr,"Error creating ushort scalar - %s\n",SdsErrorCodeString(status));
        fprintf(stderr,"Test aborting\n");
        return(-1);
    }
    dims[0] = 20;
    dims[1] = 11;
    dims[2] = 3;
    SdsNew(topid,"Ushorts",0,NULL,SDS_USHORT,3,dims,&ushortid2,&status);
    if (status != SDS__OK)
    {
        fprintf(stderr,"Error creating ushort array - %s\n",SdsErrorCodeString(status));
        fprintf(stderr,"Test aborting\n");
        return(-1);
    }
    SdsNew(topid,"Int",0,NULL,SDS_INT,0,NULL,&intid1,&status);
    if (status != SDS__OK)
    {
        fprintf(stderr,"Error creating int scalar - %s\n",SdsErrorCodeString(status));
        fprintf(stderr,"Test aborting\n");
        return(-1);
    }
    dims[0] = 256;
    SdsNew(topid,"Ints",0,NULL,SDS_INT,1,dims,&intid2,&status);
    if (status != SDS__OK)
    {
        fprintf(stderr,"Error creating int array - %s\n",SdsErrorCodeString(status));
        fprintf(stderr,"Test aborting\n");
        return(-1);
    }
    SdsNew(topid,"Uint",0,NULL,SDS_UINT,0,NULL,&uintid1,&status);
    if (status != SDS__OK)
    {
        fprintf(stderr,"Error creating uint scalar - %s\n",SdsErrorCodeString(status));
        fprintf(stderr,"Test aborting\n");
        return(-1);
    }
    dims[0] = 4;
    dims[1] = 1;
    dims[2] = 5;
    dims[3] = 2;
    SdsNew(topid,"Uints",0,NULL,SDS_UINT,4,dims,&uintid2,&status);
    if (status != SDS__OK)
    {
        fprintf(stderr,"Error creating uint array - %s\n",SdsErrorCodeString(status));
        fprintf(stderr,"Test aborting\n");
        return(-1);
    }
    SdsNew(topid,"I64",0,NULL,SDS_I64,0,NULL,&i64id1,&status);
    if (status != SDS__OK)
    {
        fprintf(stderr,"Error creating int64 scalar - %s\n",SdsErrorCodeString(status));
        fprintf(stderr,"Test aborting\n");
        return(-1);
    }
    /*
     * 64 bit signed array
     *
     * Don't bother with this as yet - The Put and Get test don't use it 
     *
    dims[0] = 128;
    SdsNew(topid,"I64s",0,NULL,SDS_I64,1,dims,&i64id2,&status);
    if (status != SDS__OK)
    {
        fprintf(stderr,"Error creating int64 array - %s\n",SdsErrorCodeString(status));
        fprintf(stderr,"Test aborting\n");
        return(-1);
    }
    */
    SdsNew(topid,"UI64",0,NULL,SDS_UI64,0,NULL,&ui64id1,&status);
    if (status != SDS__OK)
    {
        fprintf(stderr,"Error creating uint64 scalar - %s\n",SdsErrorCodeString(status));
        fprintf(stderr,"Test aborting\n");
        return(-1);
    }
    /*
     * 64 bit unsigned array
     *
     * Don't bother with this as yet - The Put and Get test don't use it 
     *
    dims[0] = 4;
    dims[1] = 2;
    SdsNew(topid,"UI64s",0,NULL,SDS_UI64,2,dims,&ui64id2,&status);
    if (status != SDS__OK)
    {
        fprintf(stderr,"Error creating ui64 array - %s\n",SdsErrorCodeString(status));
        fprintf(stderr,"Test aborting\n");
        return(-1);
    }
    */
    SdsNew(topid,"Float",0,NULL,SDS_FLOAT,0,NULL,&floatid1,&status);
    if (status != SDS__OK)
    {
        fprintf(stderr,"Error creating float scalar - %s\n",
               SdsErrorCodeString(status));
        fprintf(stderr,"Test aborting\n");
        return(-1);
    }
    dims[0] = 256;
    SdsNew(topid,"Floats",0,NULL,SDS_FLOAT,1,dims,&floatid2,&status);
    if (status != SDS__OK)
    {
        fprintf(stderr,"Error creating float array - %s\n",
               SdsErrorCodeString(status));
        fprintf(stderr,"Test aborting\n");
        return(-1);
    }
    SdsNew(topid,"Double",0,NULL,SDS_DOUBLE,0,NULL,&doubleid1,&status);
    if (status != SDS__OK)
    {
        fprintf(stderr,"Error creating double scalar - %s\n",
               SdsErrorCodeString(status));
        fprintf(stderr,"Test aborting\n");
        return(-1);
    }
    dims[0] = 20;
    SdsNew(topid,"Doubles",0,NULL,SDS_DOUBLE,1,dims,&doubleid2,&status);
    if (status != SDS__OK)
    {
        fprintf(stderr,"Error creating double array %s\n",
               SdsErrorCodeString(status));
        fprintf(stderr,"Test aborting\n");
        return(-1);
    }
    SdsNew(topid,"undefDouble",0,NULL,SDS_DOUBLE,0,NULL,&undefDouble,&status);
    if (status != SDS__OK)
    {
        fprintf(stderr,"Error creating undefined double - %s\n",
               SdsErrorCodeString(status));
        fprintf(stderr,"Test aborting\n");
        return(-1);
    }

    SdsNew(topid,"Structure",10,"Extra Text",SDS_STRUCT,0,NULL,&sid,&status);
    SdsNew(sid,"c1",0,NULL,SDS_CHAR,0,NULL,&tid,&status);
    SdsNew(sid,"d1",0,NULL,SDS_DOUBLE,0,NULL,&tid,&status);
    SdsNew(sid,"c2",0,NULL,SDS_CHAR,0,NULL,&tid,&status);
    SdsNew(sid,"f1",0,NULL,SDS_FLOAT,0,NULL,&tid,&status);
    SdsNew(sid,"c3",0,NULL,SDS_CHAR,0,NULL,&tid,&status);
    SdsNew(sid,"l1",0,NULL,SDS_INT,0,NULL,&tid,&status);
    SdsNew(sid,"s1",0,NULL,SDS_SHORT,0,NULL,&tid,&status);
    SdsNew(sid,"ul1",0,NULL,SDS_UINT,0,NULL,&tid,&status);
    SdsNew(sid,"c4",0,NULL,SDS_CHAR,0,NULL,&tid,&status);
    SdsNew(sid,"us1",0,NULL,SDS_USHORT,0,NULL,&tid,&status);
    if (status != SDS__OK)
    {
        fprintf(stderr,"Error creating structure - %s\n",SdsErrorCodeString(status));
        fprintf(stderr,"Test aborting\n");
        return(-1);
    }

 
    printf("Primitive creation test completed\n");

/*  Structure array creation test  */

    dims[0]=2;
    dims[1]=3;
    SdsNew(topid,"Strarray",0,NULL,SDS_STRUCT,2,dims,&id1,&status);
    if (status != SDS__OK)
    {
        fprintf(stderr,"Error creating structure array - %s\n",SdsErrorCodeString(status));
        fprintf(stderr,"Test aborting\n");
        return(-1);
    }
    SdsCell(topid,2,dims,&id2,&status);
    if (status != SDS__NOTARRAY)
    {
        fprintf(stderr,"SdsCell - Failed to get NOTARRAY error - %s\n",SdsErrorCodeString(status));
        fprintf(stderr,"Test aborting\n");
        return(-1);
    }
    status = SDS__OK;
    SdsCell(id1,3,dims,&id2,&status);
    if (status != SDS__INDEXERR)
    {
        fprintf(stderr,"SdsCell - Failed to get INDEXERR error - %s\n",SdsErrorCodeString(status));
        fprintf(stderr,"Test aborting\n");
        return(-1);
    }
    status = SDS__OK;
    SdsCell(id1,2,dims,&id2,&status);
    if (status != SDS__OK)
    {
        fprintf(stderr,"Error in Sds Cell - %s\n",SdsErrorCodeString(status));
        fprintf(stderr,"Test aborting\n");
        return(-1);
    }

    printf("Structure array creation test completed\n");

    SdsTestPut(topid,&status);
    if (status != SDS__OK) return(-1);
  
    SdsTestGet(topid,&status);
    if (status != SDS__OK) return(-1);

    printf("Put and Get test completed\n");

    SdsCopy(topid,&id3,&status);
    if (status != SDS__OK)
    {
        fprintf(stderr,"Error Copying an SDS object - %s\n",
               SdsErrorCodeString(status));
        fprintf(stderr,"Test aborting\n");
        return(-1);
    }

    SdsInsert(id2,id3,&status);
    if (status != SDS__OK)
    {
        fprintf(stderr,"Error Inserting into a structure %s\n",
               SdsErrorCodeString(status));
        fprintf(stderr,"Test aborting\n");
        return(-1);
    }

    SdsExtract(id2,&status);
    if (status != SDS__ILLDEL)
    {
        fprintf(stderr,"SdsExtract - failed to get ILLDEL error - got %s\n",
               SdsErrorCodeString(status));
        fprintf(stderr,"Test aborting\n");
        return(-1);
    }
    status = SDS__OK;

    SdsExtract(id3,&status);
    if (status != SDS__OK)
    {
        fprintf(stderr,"Error Extracting from a structure - %s\n",
               SdsErrorCodeString(status));
        fprintf(stderr,"Test aborting\n");
        return(-1);
    }


    SdsInsert(id2,id3,&status);

    SdsTestGet(id3,&status);
    if (status != SDS__OK) return(-1);

    printf("Copy and Insert test completed\n");


    SdsCloneId(topid, &id6, &status);
    if (status != SDS__OK)
    {
        fprintf(stderr,"Error in SdsCloneId for internal data %s\n", 
                SdsErrorCodeString(status));
        fprintf(stderr,"Test aborting\n");
        return(-1);
    }
    SdsTestGet(id6,&status);
    if (status != SDS__OK) return(-1);

    printf("Clone of internal ID test completed\n");

    SdsSize(topid,&size,&status);
    osize = size;
    buffer = (void*)malloc(size);
    SdsExport(topid,size,buffer,&status);
    if (status != SDS__OK)
    {
        fprintf(stderr,"Error Exporting a structure - %s\n",
               SdsErrorCodeString(status));
        fprintf(stderr,"Test aborting\n");
        return(-1);
    }

    junk = (void*)malloc(20);
    SdsAccess(junk,&id4,&status);
    if ((status != SDS__NOTSDS) && (status != SDS__VERSION))
    {
        fprintf(stderr,"SdsAccess - failed to get NOTSDS or VERSION status - got %s\n",SdsErrorCodeString(status));
        if (status == STATUS__OK) status = SDS__TESTERR;
        fprintf(stderr,"Test aborting\n");
        return(-1);
    }
    status = SDS__OK;

    SdsAccess(buffer,&id4,&status);
    if (status != SDS__OK)
    {
        fprintf(stderr,"Error Accessing a structure - %s\n",SdsErrorCodeString(status));
        fprintf(stderr,"Test aborting\n");
        return(-1);
    }
    SdsTestGet(id4,&status);
    if (status != SDS__OK) return(-1);

    printf("Export and Access test completed\n");

    SdsCloneId(id4, &id6, &status);
    if (status != SDS__OK)
    {
        fprintf(stderr,"Error in SdsCloneId of exported data %s\n", 
                SdsErrorCodeString(status));
        fprintf(stderr,"Test aborting\n");
        return(-1);
    }
    SdsTestGet(id6,&status);
    if (status != SDS__OK) return(-1);
    printf("Clone of exported data test completed\n");




    SdsNew(0,"Test",0,0,SDS_STRUCT,0,0,&idq,&status);
    SdsCopy(topid,&idcop,&status);
    SdsInsert(idq,idcop,&status);
    SdsSize(idq,&size,&status);
    buf2 = (void*)malloc(size);
    SdsExport(idq,size,buf2,&status);
    SdsAccess(buf2,&idex,&status);
    SdsFind(idex,"Top",&idetop,&status);
    SdsSize(idetop,&size,&status);
    buf3 = (void*)malloc(size);
    SdsExport(idetop,size,buf3,&status);
    SdsAccess(buf3,&id9,&status);
    SdsTestGet(id9,&status);
    if (status != SDS__OK) return(-1);
  
    printf("Export and Access test for substructure completed\n");




    SdsSize(id4,&size,&status);
    if (status != SDS__OK)
    {
        fprintf(stderr,"Error with external SdsSize %ld\n",(long)status);
        fprintf(stderr,"Test aborting\n");
        return(-1);
    }
    if (size != osize)
    {
        fprintf(stderr,"Error in external SdsSize %ld\n",(long)size);
        fprintf(stderr,"Test aborting\n");
        return(-1);
    }
    buffer2 = (void*)malloc(size);
    SdsExport(id4,size,buffer2,&status);
    if (status != SDS__OK)
    {
        fprintf(stderr,"Error Exporting an external structure - %s\n",
               SdsErrorCodeString(status));
        fprintf(stderr,"Test aborting\n");
        return(-1);
    }
    SdsAccess(buffer2,&ide,&status);
    if (status != SDS__OK)
    {
        fprintf(stderr,"Error Accessing a structure - %s\n",
               SdsErrorCodeString(status));
        fprintf(stderr,"Test aborting\n");
        return(-1);
    }
    SdsTestGet(ide,&status);
    if (status != SDS__OK) return(-1);

    SdsFind(id4,"Chars",&idc,&status);
    SdsSize(idc,&size,&status);
    SdsFind(topid,"Chars",&idc,&status);
    SdsSize(idc,&osize,&status);
    if (status != SDS__OK)
    {
        fprintf(stderr,"Error with external SdsSize %ld\n",(long)status);
        fprintf(stderr,"Test aborting\n");
        return(-1);
    }
    if (size != osize)
    {
        fprintf(stderr,"Error in external SdsSize %ld\n",(long)size);
        fprintf(stderr,"Test aborting\n");
        return(-1);
    }


    SdsFind(id4,"Structure",&idc,&status);
    SdsSize(idc,&size,&status);
    SdsFind(topid,"Structure",&idc,&status);
    SdsSize(idc,&osize,&status);
    if (status != SDS__OK)
    {
        fprintf(stderr,"Error with external SdsSize for structure - %s\n",
               SdsErrorCodeString(status));
        fprintf(stderr,"Test aborting\n");
        return(-1);
    }
    if (size != osize)
    {
        fprintf(stderr,"Error in external SdsSize for structure %ld %ld\n",
               (long)size,(long)osize);
        fprintf(stderr,"Test aborting\n");
        return(-1);
    }


    SdsFind(id4,"Strarray",&idc,&status);
    SdsSize(idc,&size,&status);
    SdsFind(topid,"Strarray",&idc,&status);
    SdsSize(idc,&osize,&status);
    if (status != SDS__OK)
    {
        fprintf(stderr,"Error with external SdsSize for structure array - %s\n",
               SdsErrorCodeString(status));
        fprintf(stderr,"Test aborting\n");
        return(-1);
    }
    if (size != osize)
    {
        fprintf(stderr,"Error in external SdsSize for structure array %ld %ld\n",
               (long)size,(long)osize);
        fprintf(stderr,"Test aborting\n");
        return(-1);
    }

    SdsImport(buffer,&id5,&status);
    if (status != SDS__OK)
    {
        fprintf(stderr,"Error Importing a structure - %s\n",
               SdsErrorCodeString(status));
        fprintf(stderr,"Test aborting\n");
        return(-1);
    }
    SdsTestGet(id5,&status);
    if (status != SDS__OK)
    {
        fprintf(stderr,"SdsTestGet() failed\n");
        return(-1);
    }
    printf("Import test completed\n");

    SdsDelete(id4,&status);
    if (status != SDS__EXTERN)
    {
        fprintf(stderr,"SdsDelete - failed to get EXTERN error %s\n",
               SdsErrorCodeString(status));
        return(-1);
    }
    status = SDS__OK;

    SdsDelete(id2,&status);
    if (status != SDS__ILLDEL)
    {
        fprintf(stderr,"SdsDelete - failed to get ILLDEL error %s\n",
               SdsErrorCodeString(status));
        return(-1);
    }
    status = SDS__OK;

    SdsDelete(id3,&status);
    if (status != SDS__OK)
    {
        fprintf(stderr,"Error in SdsDelete - %s\n",
               SdsErrorCodeString(status));
        fprintf(stderr,"Test aborting\n");
        return(-1);
    }
    dims[0]=4;
    SdsResize(id3,1,dims,&status);
    if (status != SDS__BADID)
    {
        fprintf(stderr, "SdsResize - failed to get BADID error - %s\n",
               SdsErrorCodeString(status));
        fprintf(stderr,"Test aborting\n");
        return(-1);
    }
    status = SDS__OK;
    SdsResize(id1,1,dims,&status);
    if (status != SDS__OK)
    {
        fprintf(stderr,"Error in SdsResize - %s\n", SdsErrorCodeString(status));
        fprintf(stderr,"Test aborting\n");
        return(-1);
    }
    SdsRename(id1,"Renamed",&status);
    if (status != SDS__OK)
    {
        fprintf(stderr,"Error in SdsRename %s\n",SdsErrorCodeString(status));
        fprintf(stderr,"Test aborting\n");
        return(-1);
    }

    SdsWrite(topid,"test.sds",&status);
    if (status != SDS__OK)
    {
        fprintf(stderr,"Error in SdsWrite - %s\n",SdsErrorCodeString(status));
        fprintf(stderr,"Test aborting\n");
        return(-1);
    }
    SdsList(topid,&status);
    if (status != SDS__OK)
    {
        fprintf(stderr,"Error in SdsList %s\n", SdsErrorCodeString(status));
        fprintf(stderr,"Test aborting\n");
        return(-1);
    }

    printf("sdstest:Things look good\n");
    return(0);
}
  
#define INT_VAL 10
#define FLOAT_VAL 30.0
#define DOUBLE_VAL 20.0
/*
 * This function reads the specified file and looks for the values
 * named Integer, Float and Double and checks the have the values
 * defined above.  Name normally referes to a file containing those
 * values, but written on a little or big endian machine.  This function
 * checks we can read from both formats successfully.
 */
static void CheckFile(const char *name,  StatusType *status)
{
    SdsIdType id=0;
    SdsIdType tid=0;
    INT32 i;
    double d;
    float f;
    unsigned long int actlen;
    if (*status != SDS__OK) return;

    SdsRead(name, &id, status);
    if (*status != SDS__OK)
    {
        fprintf(stderr, "Failed to read file \"%s\" - %s\n", 
                name, SdsErrorCodeString(*status));
        return;
    }

    /* Check we can read Integer from the file */
    SdsFind(id, "Integer", &tid, status);
    SdsGet(tid, sizeof(i), 0, (void *)&i, &actlen, status);
    if (*status != SDS__OK)
    {
        fprintf(stderr, "Failed to read Integer from file %s - %s\n", 
               name, SdsErrorCodeString(*status));
        return;
    }
    else if (actlen != sizeof(i))
    {
        *status = SDS__TESTERR;
        fprintf(stderr, "When reading Integer from file %s, length was %ld rather then %ld\n", 
              name, actlen, (long)sizeof(i));
        return;
    }
    else if (i != INT_VAL)
    {
        *status = SDS__TESTERR;
        fprintf(stderr, "When reading Integer from file %s, value was %ld rather then %d\n", 
              name, (long)i, INT_VAL);
        return;
    }
    SdsFreeId(tid, status);

    /* Check we can read Float from the file */
    SdsFind(id, "Float", &tid, status);
    SdsGet(tid, sizeof(f), 0, (void *)&f, &actlen, status);
    if (*status != SDS__OK)
    {
        fprintf(stderr,"Failed to read Float from file %s\n", name);
        return;
    }
    else if (actlen != sizeof(f))
    {
        *status = SDS__TESTERR;
        fprintf(stderr,"When reading Float from file %s, length was %ld rather then %ld\n", 
              name, actlen, (long)sizeof(f));
        return;
    }
    else if (f != FLOAT_VAL)
    {
        *status = SDS__TESTERR;
        fprintf(stderr, "When reading Float from file %s, value was %g rather then %g\n", 
              name, f, FLOAT_VAL);
        return;
    }
    SdsFreeId(tid, status);

    /* Check we can read Double from the file */
    SdsFind(id, "Double", &tid, status);
    SdsGet(tid, sizeof(d), 0, (void *)&d, &actlen, status);
    if (*status != SDS__OK)
    {
        fprintf(stderr,"Failed to read Double from file %s\n", name);
        return;
    }
    else if (actlen != sizeof(d))
    {
        *status = SDS__TESTERR;
        fprintf(stderr,"When reading Double from file %s, length was %ld rather then %ld\n", 
              name, actlen, (long)sizeof(d));
        return;
    }
    else if (d != DOUBLE_VAL)
    {
        *status = SDS__TESTERR;
        fprintf(stderr,"When reading Double from file %s, value was %g rather then %g\n", 
              name, d, DOUBLE_VAL);
        return;
    }
    SdsFreeId(tid, status);

    SdsReadFree(id, status);
    SdsFreeId(id, status);
}
 
static void PrintErrorCode(StatusType status)
{
    char buffer[200];
    MessGetMsg(status, 0, sizeof(buffer), buffer);
    fprintf(stderr,"Status code = \"%s\" - %s\n", buffer, SdsErrorCodeString(status));
}

 
