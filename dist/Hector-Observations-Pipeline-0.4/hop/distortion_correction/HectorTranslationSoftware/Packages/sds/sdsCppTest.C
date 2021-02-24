/*                          s d s C p p T e s t . C
  
 * Program name:
        sdsCppTest.C

 * Function:
        A program to test the SDS C++ interfaces.
  
 * Description:
        This is intended to test the SDS C++ interface - it is early days
        yet on this, but hopefully all the bits will be tested at some stage.
  
        Unlike most of SDS, this program presumes we have DRAMA's
        CONFIG and MESSGEN sub-systems.

 * Language:
      C++
  
 * Support: Tony Farrell, AAO
  
 * History:
      15-Apr-2005 - TJF - Original version
      27-Apr-2005 - TJF - Fix bug shown up by Linux.
      16-Aug-2006 - TJF - Add use of DataAddressRW() and DataAddressRO() 
                            members to CheckArray;
      
  
 *  Copyright (c) Anglo-Australian Telescope Board, 2005
    Not to be used for commercial purposes without AATB permission.
  
 *     @(#) $Id: ACMM:sds/sdsCppTest.C,v 3.94 09-Dec-2020 17:15:24+11 ks $
  
 */


static const char *rcsId="@(#) $Id: ACMM:sds/sdsCppTest.C,v 3.94 09-Dec-2020 17:15:24+11 ks $";
static void *use_rcsId = (0 ? (void *)(&use_rcsId) : (void *) &rcsId);

#define DRAMA_ALLOW_CPP_STDLIB 1 /* Allow DRAMA to use C++ template library */
#include "drama.h"
#include "sds.h"
#include "mess.h"
#include "sds_err_msgt.h"
#include <iostream>
#include <string>
#include <vector>
#include <unistd.h>

#include "arg.h"

static void Construct(SdsId *topLevel, StatusType *status);
static void Examine(SdsId &topLevel, StatusType *status);

int main(int , const char *[]) 
{
    StatusType status = STATUS__OK;
    /*
     * Allow us to translate error codes
     */
    MessPutFacility(&MessFac_SDS);

#if 0
    {
        SDS_CHECK_IDS("main");
        SdsIdType id;
        ArgNew(&id, &status);
    }
#endif


    SdsId topLevel;

    /*
     * Construct a structure and write it out to a file.
     */
    Construct(&topLevel, &status);
    topLevel.Write("sdstest.sds", &status);
    if (status != STATUS__OK)
        std::cerr << "Failure constructing/writing file sdstest.sds"  
                  << std::endl;

    else
    {
        /*
         * Read the structure back in and examine it.
         */
        SdsId topLevel2("sdstest.sds", &status);

        Examine(topLevel, &status);

        /*
         * Get rid of the file.
         */
        unlink("sdstest.sds");
    }

    if (status != STATUS__OK)
    {
        char buffer[200];
        MessGetMsg(status, 0, sizeof(buffer), buffer);
        std::cerr << "   Status = " << buffer  << std::endl;
        return 1;
    }


    return 0;





}

/*
 * Fill an SDS array with a standard set of values - single dimensional
 * arrays only.
 *
 *  The value put in each array element is
 *    <sds type code a integer> + index + index*filler
 *
 * Class T should be a sub-class of SdsArrayAccessHelper
 * typename F should be the underlying C++ type.
 *
 */
template<class T, typename F> 
void fillArray(SdsId &id, F filler, StatusType *status)
{
    if (*status != STATUS__OK) return;
    unsigned long i;
    unsigned long count;

    T array;
    id.ArrayAccess(&array, status);

    count = array.Size();
    for (i = 0 ; i < count ; ++i)
    {
        array[i] = i + array.Code() + filler*i;
    }    
}

/*
 * Fill an SDS array with a standard set of values - any dimensional
 * array.
 *
 *  The value put in each array element is
 *    <sds type code a integer> + index + index*filler
 *
 * Class T should be a sub-class of SdsArrayAccessHelper
 * typename F should be the underlying C++ type.
 *
 * This could be used instead of fillArray (above) but we want to test both
 * interfaces.
 */
template<class T, typename F> 
void fillArrayMult(SdsId &id, F filler, StatusType *status)
{
    if (*status != STATUS__OK) return;
    unsigned long i;
    unsigned long count;
    long ndims;
    unsigned long dims[7];

    T array;
    id.ArrayAccess(&array, &ndims, dims, status);

    count = array.Size();
    for (i = 0 ; i < count ; ++i)
    {
        array[i] = i + array.Code() + filler*i;
    }    
}

/*
 * Construct the structure Examine() expects to see.
 */
static void Construct(SdsId *topLevelResult, StatusType *status)
{
    if (*status != STATUS__OK) return;

    
    /*
     * Construct a new top-level item.
     */
    SdsId id("SdsCppTest", SDS_STRUCT, status);
    
    unsigned long dims[7];

    /*
     * construct some contents.
     */
    SdsId item1(id, "ui64", SDS_UI64, status);
    SdsId item2(id, "i64",  SDS_I64,  status);

    dims[0] = 10;
    SdsId item3(id, "double", SDS_DOUBLE, 1, dims, status);
    dims[0] = 12;
    SdsId item4(id, "float",  SDS_FLOAT,  1, dims, status);

    dims[0] = 5;
    SdsId item5(id, "byte",   SDS_BYTE,   1, dims, status);
    SdsId item6(id, "ubyte",  SDS_UBYTE,  1, dims, status);

    dims[0] = 3;
    SdsId item7(id, "short",   SDS_SHORT,  1, dims, status);
    SdsId item8(id, "ushort",  SDS_USHORT,  1, dims, status);

    dims[0] = 10;
    dims[1] = 10;
    SdsId item9(id, "int",     SDS_INT,    2, dims, status);
    dims[0] = 20;
    dims[1] = 20;
    SdsId item10(id, "uint",    SDS_UINT,   2, dims, status);


    dims[0] = 100;
    dims[1] = 10;
    SdsId item11(id, "string_array",  SDS_CHAR,   2, dims, status);


    if (*status != STATUS__OK)
    {
        std::cerr << "Failure constructing sds structure"  
                  << std::endl;
        return;
    }

    /*
     * Put values into the 2 64 bit items.
     */
    UINT64 myu64 = SdsSetUI64(100,30); /*   429496729630 */
    INT64  my64  = SdsSetI64(-100,20); /*  -429496729580 */
    
    item1.Put(sizeof(myu64), (void *)&myu64, status);
    item2.Put(sizeof(my64),  (void *)&my64,  status);


    /*
     * For each of the standard arrays, we use create an appropiate array
     * access help and fill up the array with some pattern.  We use the
     * fillArray template function to make this easier.
     */
    fillArray<SdsArrayDouble, double>        (item3,  0.1, status);
    fillArray<SdsArrayFloat,  float>         (item4,  0.1, status);
    fillArray<SdsArrayByte,   signed char>   (item5,  0,   status);
    fillArray<SdsArrayUbyte,  unsigned char> (item6,  0,   status);

    fillArray<SdsArrayShort,  short>         (item7,  0,   status);
    fillArray<SdsArrayUshort, unsigned short>(item8,  0,   status);

    /*
     * As per fillArray but appropiate for multi dimentional arrays.
     */
    fillArrayMult<SdsArrayINT32,  INT32>     (item9,  0,   status);
    fillArrayMult<SdsArrayUINT32, UINT32>    (item10, 0,   status);


    if (*status != STATUS__OK)
    {
        std::cerr << "Failure filling sds structure"  
                  << std::endl;
    }    
    else
        // Return the result.
        topLevelResult->ShallowCopy(id, true);
}

/*
 * check an SDS array which should be filled with  a standard set of 
 * values - any dimensional arrays.
 *
 *  The value  in each array element should be
 *    <sds type code a integer> + index + index*filler
 *
 * Class T should be a sub-class of SdsArrayAccessHelper
 * typename F should be the underlying C++ type.
 *
 */
template<class T, typename F> 
void checkArray(
    SdsId &id,           /* Top level structure */
    const char *name,    /* Item name */
    F filler,            /* Expected filler item */
    StatusType *status)
{
    if (*status != STATUS__OK) return;

    SdsId item(id, name, status);
    if (*status != STATUS__OK) 
    {
        std::cerr << "Failure finding item \""  << name 
                  << "\" in top level structure"
                  << std::endl;
        return;
    }

    unsigned long i;
    unsigned long count;
    long ndims;
    unsigned long dims[7];

    T array;
    item.ArrayAccess(&array, &ndims, dims, status);

    if (*status != STATUS__OK)
    {
        std::cerr << "Failure accessing \"" 
                      << name
                      << "\" data item.  "  
                      << std::endl;
        return;
    }

    count = array.Size();
    for (i = 0 ; i < count ; ++i)
    {
        F x = i;  /* Need to ensure the types are right in the next line */
        F expected = x + (F)(array.Code()) + filler*x;
        if (array[i] != expected) 
        {
            std::cerr << "Failure setting/retriving \"" 
                      << name
                      << "\" data item."  
                      << std::endl
                      << "    Value of data item at index "
                      << i
                      << " is "
                      << array[i]
                      << " Instead of "
                      << expected
                      << std::endl;
            id.List(status);
            *status = SDS__TESTERR;
            return;
        }
    }    

    /*
     * Try the raw data access - both by Read/Write address and
     * Read Only addres.  Really only a compiler check.
     */
    F       *rawDataRW = array.DataAddressRW();
    const F *rawDataRO = array.DataAddressRO();
    rawDataRW[0] = rawDataRO[0];

}

/*
 * Examine the structure -is its contents as we expected (as contructed
 * by Contruct()). 
 */
static void Examine(SdsId &topLevel, StatusType *status)
{
    if (*status != STATUS__OK) return;
    //topLevel.List(status);

    /*
     * Check the unsigned 64 bit item.
     */
    SdsId item1(topLevel, "ui64", status);
    UINT64 u64;
    item1.Get(sizeof(u64), (void *)&u64, status);
    unsigned long u64high;
    unsigned long u64low;
    SdsGetUI64(u64, &u64high, &u64low);
    if ((*status == STATUS__OK)&&
        ((u64high != 100)||(u64low != 30)))
    {
         std::cerr << "Failure set/retriving unsigned 64 bit data item.  "  
                   << "Values are (" << u64high << "," << u64low 
                   << ") instead of (100, 30)"
                  << std::endl;
         *status = SDS__TESTERR;
    }
    
    /*
     * Check the signed 64 bit item.
     */
    SdsId item2(topLevel, "i64", status);
    INT64 i64;
    item2.Get(sizeof(i64), (void *)&i64, status);
    long          i64high;
    unsigned long i64low;
    SdsGetI64(i64, &i64high, &i64low);
    if ((*status == STATUS__OK)&&
        ((i64high != -100)||(i64low != 20)))
    {
         std::cerr << "Failure set/retriving signed 64 bit data item.  " 
                   << "Values are (" << i64high << "," << i64low 
                   << ") instead of (-100, 20)"
                  << std::endl;
         *status = SDS__TESTERR;
    }
    
    /*
     * For each of the standard arrays, we access the array and check its
     * value meets the pattern set by Construct() above.  We use the
     * checkArray template for this.
     */
    checkArray<SdsArrayDouble, double>        (topLevel, "double", 0.1, status);
    checkArray<SdsArrayFloat,  float>         (topLevel, "float",  0.1, status);

    checkArray<SdsArrayByte,   signed char>   (topLevel, "byte",   0,   status);
    checkArray<SdsArrayUbyte,  unsigned char> (topLevel, "ubyte",  0,   status);

    checkArray<SdsArrayShort,  short>         (topLevel, "short",  0,   status);
    checkArray<SdsArrayUshort, unsigned short>(topLevel, "ushort", 0,   status);

    checkArray<SdsArrayINT32,  INT32>         (topLevel, "int",    0,   status);
    checkArray<SdsArrayUINT32, UINT32>        (topLevel, "uint",   0,   status);

    if (*status != STATUS__OK)
    {
        std::cerr << "Failure examining file sdstest.sds"  
                  << std::endl;
        
    }
}
