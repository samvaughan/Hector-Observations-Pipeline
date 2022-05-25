
/*  Beginnings of an  Arg library test program  */


/*
 * History:
 *     5-Jul-2018 - TJF - Initial version, testing a problem with
 *                        ArgToString() and 64 bit signed values.
 */


static const char *rcsId="@(#) $Id: ACMM:sds/argtest.C,v 3.94 09-Dec-2020 17:15:55+11 ks $";
static void *use_rcsId = (0 ? (void *)(&use_rcsId) : (void *) &rcsId);


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "drama.h"
#include "status.h"
#include "mess.h"
#include "sds.h"
#include "arg.h"
#include "sds_err_msgt.h"
#include "arg_err_msgt.h"
#include "string"

/* Check the conversion of 64 bit values to strings, this was failing
 * for a long time due to a but in ArgToString()
 */
void Arg64BitToStringCheck(StatusType *status)
{
    /*
     * VAL1 is the first value we are playing with.  It was
     * a value that gave problems in TAIPAN code, but is just
     * one of a class that will give this problem. The UPPER and
     * LOWER bits are the same value, when extracted from a 64 bit
     * unsigned.
     *
     * VAL_STRING is the same value as a string, expected result from
     * ArgToString().
     */
#if 1  // Example which only uses the bottom half of the 64 bit item.
    const long VAL1 = -1922485; 
    const long VAL1_UPPER = -1;
    const unsigned long VAL1_LOWER = 0xffe2aa4b;
    const std::string VAL1_STRING = "-1922485 ";
    const std::string VAL1_STRING_ARRAY = "-1922485 -1922485 -1922485";
#else // Example with even larger value - needs > 32 bits.
    const long long VAL1 = -19224850000; 
    const long VAL1_UPPER = -5;
    const unsigned long VAL1_LOWER = 0x861c11b0;
    const std::string VAL1_STRING = "-19224850000 ";
    const std::string VAL1_STRING_ARRAY = "-19224850000 -19224850000 -19224850000";
#endif
    INT64 s64;
#if defined(NATIVE__INT64)
    s64 =  VAL1;
    printf("64 bit value is %lld\n", s64);
#else
    s64 = SdsSetI64(VAL1_UPPER, VAL1_LOWER);
#endif

    long high;
    unsigned long low;
    SdsGetI64(s64, &high, &low);

    printf("64 bit value converted to 2 32s gives us high=%ld, low=0x%lx\n",
           high, low);

    if (high != VAL1_UPPER)
    {
        fprintf(stderr,"Failed to correctly get high part of 64 bit value\n");
        *status = SDS__TESTERR;
        return;
    }
    if (low != VAL1_LOWER)
    {
        fprintf(stderr,"Failed to correctly get high part of 64 bit value\n");
        *status = SDS__TESTERR;
        return;
    }
#if 1

    if (high < 0)
    {
        printf("YYYY:  %lld  %lld\n",
               (SDS__ULONG_DIVSOR - low),
               ((SDS__ULONG_DIVSOR - low)*-1));
        double highPart = (double)(high+1)*SDS__ULONG_DIVSOR_D;
        printf("YYYY: %.20g\n", highPart);
        printf("YYYY: %.20g\n", highPart - SDS__ULONG_DIVSOR + low);
    }
    else
    {
        printf("XXX: %.10g\n",
               ((double)high*SDS__ULONG_DIVSOR_D+(double)(low)));
    }
#endif

    Arg topID;
    topID.New(status);
    topID.Put("64bit_item", s64, status);
    topID.List(status);
    
    char buffer[50];
    int length=0;
    topID.ToString(sizeof(buffer), &length, buffer, status);
    if (*status != STATUS__OK)
    {
        fprintf(stderr, 
                "Failure constructing and converting Arg 64 bit item\n");
        return;
    }
    printf("Arg 64 bit item converted back to string is \"%s\"\n",
           buffer);
    fflush(stdout);

    if (VAL1_STRING != std::string(buffer))
    {
        fprintf(stderr,"64 bit value did not converted to string correctly\n");
        *status = SDS__TESTERR;
        return;
    }
    topID.Delete(status);


    const unsigned NUM_ITEMS = 3;
    SdsIdType id = 0;
    unsigned long dims[1] = { NUM_ITEMS };
    SdsNew(0, "64BIT_ARRAY", 0, 0, SDS_I64, 1, dims, &id, status);
    
    INT64 i64array[NUM_ITEMS];
    for (unsigned i = 0; i < NUM_ITEMS ; ++i)
    {
        i64array[i] = VAL1;
    }
    SdsPut(id, sizeof(i64array), 0, (void *)(&i64array), status);

    SdsList(id, status);
    length = 0;
    ArgToString(id, sizeof(buffer), &length, buffer, status);

    if (*status != STATUS__OK)
    {
        fprintf(stderr, 
                "Failure constructing or converting 64 bit array item\n");
        return;
    }
    printf("Arg 64 array item converted back to string is \"%s\"\n",
           buffer);
    fflush(stdout);
    if (VAL1_STRING_ARRAY != std::string(buffer))
    {
        fprintf(stderr,"64 bit array value did not converted to string correctly\n");
        *status = SDS__TESTERR;
        return;
    }

    const long long VAL2 =  1922485; 
    const long long VAL3 =  19224850000; 
    const long long VAL4 = -1922485; 
    const long long VAL5 = -19224850000; 
    const unsigned long long VAL6 =  1922485; 
    const unsigned long long VAL7 =  19224850000; 
    
    printf("VAL2 - smaller pos case:%lld %.15g\n",
           VAL2, SdsGetI64toD(VAL2));
    printf("VAL3 - larger  pos case:%lld %.15g\n",
           VAL3, SdsGetI64toD(VAL3));
    printf("VAL4 - smaller neg case:%lld %.15g\n",
           VAL4, SdsGetI64toD(VAL4));
    printf("VAL5 - larger pos case:%lld %.15g\n",
           VAL5, SdsGetI64toD(VAL5));

    printf("VAL6 - smaller unsigned case:%llu %.15g\n",
           VAL6, SdsGetI64toD(VAL6));
    printf("VAL3 - larger  unsigned case:%llu %.15g\n",
           VAL7, SdsGetI64toD(VAL7));

}

static void PrintErrorCode(StatusType status);

extern int main()
{
    /*
     * Allow us to translate error codes
     */
    MessPutFacility(&MessFac_SDS);
    MessPutFacility(&MessFac_ARG);

    StatusType status = STATUS__OK;
 
    Arg64BitToStringCheck(&status);
    if (status != STATUS__OK)
    {
        fprintf(stderr,"argtest:Check of 64 bit SDS item conversion to string failed\n");
        PrintErrorCode(status);
        return(1);
    }

    printf("argtest:Things look good\n");
    return(0);
}
  

static void PrintErrorCode(StatusType status)
{
    char buffer[200];
    MessGetMsg(status, 0, sizeof(buffer), buffer);
    fprintf(stderr,"Status code = \"%s\" - %s\n", buffer, SdsErrorCodeString(status));
}

 
