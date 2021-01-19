/*  SDS Include file                                         */
/*                                                           */
/*  Jeremy Bailey      1998  Jul 13                          */
/*                                                           */
/*     @(#) $Id: ACMM:sds/sds.h,v 3.94 09-Dec-2020 17:15:24+11 ks $                        */
/*                                                           */
/*                                                           */
/*                                                           */

/*
 * This file now uses DOXYGEN comments to generate the C++ web pages.
 * m4 macros of the form DCF(function) are used in the comments
 * to refer to DRAMA C function documention (see DramaHtml/Makefile,
 * DramaHtml/doxygen.config and DramaHtml/filter.m4 for detials)
 */
#define USE_INTERP_RESULT 1
#ifndef _SDS_INCLUDE_
#define _SDS_INCLUDE_ 1

#include "sdsport.h"
#include "status.h"
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

/** 
 * \def SDS_VERSION
 *
 * SDS version code    
 */
#define SDS_VERSION 100

/*    Status codes  */

#include "sds_err.h"

/*    Type codes   */

#define SDS_STRUCT   0              /*  Structure   */
#define SDS_CHAR     1              /*  Character   */
#define SDS_UBYTE    2              /*  Unsigned byte */
#define SDS_BYTE     3              /*  byte          */
#define SDS_USHORT   4              /*  Unsigned short */
#define SDS_SHORT    5              /*  short  */
#define SDS_UINT     6              /*  Unsigned int  */
#define SDS_INT      7              /*  int  */
#define SDS_FLOAT    8              /*  float  */
#define SDS_DOUBLE   9              /*  double  */
#define SDS_I64      11             /*  Int 64  */
#define SDS_UI64     12             /*  Unsigned Int 64  */
#define SDS_C_MAXCODE SDS_UI64      /* The maximum code value */

/*  Name length  */

#define SDS_C_NAMELEN 16

#define SDS_C_MAXARRAYDIMS 7

#define SDS_WATCH_EVENT_FREE        0
#define SDS_WATCH_EVENT_DELETE      1
#define SDS_WATCH_EVENT_READFREE    2
#define SDS_WATCH_EVENT_NEW         3
#define SDS_WATCH_EVENT_COPY        4
#define SDS_WATCH_EVENT_INSERT      5
#define SDS_WATCH_EVENT_INSERT_CELL 6
#define SDS_WATCH_EVENT_EXTRACT     7
#define SDS_WATCH_EVENT_RESIZE      8
#define SDS_WATCH_EVENT_NEWID       9

typedef long  SdsIdType;

typedef long  SdsCodeType;

typedef void (*SdsWatchRoutineType)(SdsIdType, void *, int reserved);

/*
 * Functions defined in sds.c.
 */
SDSEXTERN void SdsSetWatch(SdsIdType id,
                           SdsWatchRoutineType watchRoutine,
                           void * clientData,
                           SdsIdType *oldId,
                           SdsWatchRoutineType *oldWatchRoutine,
                           void ** oldClientData,
                           StatusType * SDSCONST status);

SDSEXTERN void SdsCreate(SDSCONST char *name, long nextra, SDSCONST char *extra, 
		SdsIdType *id, StatusType * SDSCONST status);

SDSEXTERN void SdsInfo(SdsIdType id,char *name, SdsCodeType *code, long *ndims, 
               unsigned long* dims, StatusType * SDSCONST status);
SDSEXTERN void SdsGetName(SdsIdType id, char *name, StatusType * SDSCONST status);
SDSEXTERN void SdsGetCode(SdsIdType id, SdsCodeType *code, 
                          StatusType * SDSCONST status);

SDSEXTERN void SdsGetDims(SdsIdType id, long *ndims, 
                          unsigned long *dims, StatusType * SDSCONST status);

SDSEXTERN void SdsNew(SdsIdType parent_id, SDSCONST char *name, long nextra, 
	       SDSCONST char *extra, SdsCodeType code, long ndims,  
	       SDSCONST unsigned long *dims, 
               SdsIdType *id, StatusType * SDSCONST status);

SDSEXTERN int SdsIsDefined(SdsIdType parent_id, StatusType * SDSCONST status);

SDSEXTERN void SdsIndex(SdsIdType parent_id, long index, SdsIdType *id, StatusType * SDSCONST status);
SDSEXTERN void SdsNumItems(SdsIdType id, long *numItems, StatusType * SDSCONST status);

SDSEXTERN void SdsFind(SdsIdType parent_id, SDSCONST char *name, SdsIdType *id, StatusType * SDSCONST status);

SDSEXTERN void SdsFindByPath(SdsIdType parent_id, SDSCONST char *name, SdsIdType *id, StatusType * SDSCONST status);

SDSEXTERN void SdsPointer(SdsIdType id, void **data, unsigned long *length, StatusType * SDSCONST status);

SDSEXTERN void SdsGet(SdsIdType id, unsigned long length, unsigned long offset, 
	void *data, unsigned long *actlen, StatusType * SDSCONST status);

SDSEXTERN void SdsPut(SdsIdType id, unsigned long length, unsigned long offset, 
		SDSCONST void *data,  StatusType * SDSCONST status);


SDSEXTERN void SdsCell(SdsIdType array_id, long nindices, 
		SDSCONST unsigned long *indices, 
                SdsIdType *id, StatusType * SDSCONST status);


SDSEXTERN void SdsInsertCell(SdsIdType array_id, long nindices, 
		SDSCONST unsigned long *indices, 
                SdsIdType id, StatusType * SDSCONST status);

SDSEXTERN void SdsDelete(SdsIdType id, StatusType * SDSCONST status);

SDSEXTERN void SdsSize(SdsIdType id, unsigned long *bytes, StatusType * SDSCONST status);
SDSEXTERN void SdsExport(SdsIdType id, unsigned long length, void *data, StatusType * SDSCONST status);

SDSEXTERN void SdsImport(SDSCONST void *data, SdsIdType *id, 
			StatusType * SDSCONST status);

SDSEXTERN void SdsAccess(void *data, SdsIdType *id, StatusType * SDSCONST status);

SDSEXTERN void SdsExtract(SdsIdType id, StatusType * SDSCONST status);

SDSEXTERN void SdsInsert(SdsIdType parent_id, SdsIdType id, StatusType * SDSCONST status);

SDSEXTERN void SdsCopy(SdsIdType id, SdsIdType *copy_id, StatusType * SDSCONST status);

SDSEXTERN void SdsCloneId(SdsIdType id, SdsIdType *clone_id, StatusType * SDSCONST status);

SDSEXTERN void SdsResize(SdsIdType id, long ndims, SDSCONST unsigned long *dims,
		 StatusType * SDSCONST status);

SDSEXTERN void SdsRename(SdsIdType id, SDSCONST char* name, StatusType * SDSCONST status);

SDSEXTERN void SdsGetExtra(SdsIdType id, long length, char* extra, unsigned long *actlen, 
StatusType * SDSCONST status);
SDSEXTERN void SdsGetExtraLen(SdsIdType id, 
                              unsigned long *length, StatusType * SDSCONST status);

SDSEXTERN void SdsPutExtra(SdsIdType id, long length, SDSCONST char *extra, 
					StatusType * SDSCONST status);
    
SDSEXTERN void SdsExportDefined(SdsIdType id, unsigned long length, void *data, StatusType * SDSCONST status);    
      
SDSEXTERN void SdsSizeDefined(SdsIdType id, unsigned long *bytes, StatusType * SDSCONST status);

SDSEXTERN void SdsIsExternal(SdsIdType id, int *external, StatusType * SDSCONST status);

SDSEXTERN void SdsGetExternInfo(SdsIdType id, void **data, StatusType * SDSCONST status);

SDSEXTERN void SdsListInUse();

/*
 *  Functions defined in sds.c, not intended to be public but needed by
 * sdsutil.c. sdsutil.c will define _SDS_UTIL_BUILD before this is
 * included, and sds.c will define _SDS_BUILD.
 */
#if defined(_SDS_UTIL_BUILD) || defined (_SDS_BUILD)
SDSEXTERN void Sds__MarkFree(SdsIdType id);

SDSEXTERN void Sds__Free(SdsIdType id, StatusType * SDSCONST status);

SDSEXTERN void Sds___WatchEvent(SdsIdType id, int eventFlag, const char * sourceFunc);

#endif /* defined  _SDS_UTIL_BUILD */

/*
 * Functions used to work with 64 bit data on  machines
 * without any native 64 bit type .
 *
 * Note - if you get a syntax error at this point, it is likely
 * that INT64 has not been defined.  See the relevant part of
 * sdsport.h. 
 */
SDSEXTERN INT64 SdsSetI64(long high, unsigned long low);

SDSEXTERN UINT64 SdsSetUI64(unsigned long high, unsigned long low);

SDSEXTERN void SdsGetI64(INT64 i64, long *high, unsigned long *low);

SDSEXTERN void SdsGetUI64(UINT64 i64, unsigned long *high, unsigned long *low);

SDSEXTERN double SdsGetI64toD(INT64 i64);

SDSEXTERN double SdsGetUI64toD(UINT64 i64);

/*
 * Debugging function.
 */
SDSEXTERN void SdsGetIdInfo( INT32 * pnum, INT32 * pasize, INT32 * puids );

SDSEXTERN void SdsFreeIdAndCheck(const char *mes, SdsIdType id, StatusType *status);


/*
 * These two macros used by SdsCheckBuild().
 */
#ifdef NATIVE__INT64
#define __SDS_NATIVE__INT64 1
#else
#define __SDS_NATIVE__INT64 0
#endif
#ifdef LONG__64
#define __SDS_LONG__64 1
#else
#define __SDS_LONG__64 0
#endif

extern void Sds___CheckBuild(int bigend, int long_64, int native_64, StatusType *status);
#define SdsCheckBuild(_status_) Sds___CheckBuild(SDS_BIGEND, __SDS_LONG__64, __SDS_NATIVE__INT64, (_status_));


/*
 * These ones are defined in sdsutil.c 
 */
SDSEXTERN void SdsList(SdsIdType id, StatusType * SDSCONST status);
SDSEXTERN void SdsListTo(FILE *to, SdsIdType id, StatusType * SDSCONST status);

SDSEXTERN void SdsWrite(SdsIdType id, SDSCONST char *filename, StatusType * SDSCONST status);

SDSEXTERN void SdsFreeId(SdsIdType id, StatusType * SDSCONST status);

SDSEXTERN void SdsFlush(SdsIdType id, StatusType * SDSCONST status);

SDSEXTERN void SdsRead(SDSCONST char *filename, SdsIdType *id, StatusType * SDSCONST status);

SDSEXTERN void SdsFillArray(SdsIdType array_id, SdsIdType elem_id,
                StatusType * SDSCONST status);


SDSEXTERN void SdsReadFree(SdsIdType id, StatusType * SDSCONST status);
SDSEXTERN SDSCONST char *SdsTypeToStr(SdsCodeType code, int mode);
SDSEXTERN void SdsFindByName( SdsIdType parent_id,  
                    SDSCONST char * name, 
                    unsigned int * skip, 
                    SdsIdType * id, 
                    StatusType * SDSCONST status );


SDSEXTERN SDSCONST char * SdsErrorCodeString(StatusType error);






struct SdsCheckStruct {
    INT32 _init_last_id;
    INT32 _init_current_array_size;
    INT32 _init_unused_ids;
    INT32 leakCount;
    INT32 _spare2;
    INT32 _spare3;
    INT32 _spare4;
};
typedef struct SdsCheckStruct SdsCheckType;

SDSEXTERN void SdsCheckInit(SdsCheckType *chkData, StatusType *status);
SDSEXTERN void SdsCheck(const char *mes, SdsCheckType *chkData, 
                        StatusType *status); 

/* End of functions defined in sdsutil.c */



#define SDS_COMP_MESS_ERROR 1
#define SDS_COMP_MESS_WARN  2

/*
 * Defined in sdsc.c.
 */
SDSEXTERN void SdsCompiler(SDSCONST char *string, int messages, int intaslong, SdsIdType *id, StatusType *status);




#if defined(SDS_CHECK_FREE)||defined(SDS_CHECK_FREE_W)
#ifndef SdsFreeId
#ifdef SDS_CHECK_FREE_W
#warning "Checking SdsFree operations"
#endif
/*
 * Note - the C++ version does its own thing below - see cleanup().
 */
#define SdsFreeId(_id_, _status) \
    { \
        if (*(_status) == STATUS__OK)                   \
        {                                               \
            SdsFreeIdAndCheck("", (_id_), (_status));    \
            if (*(_status) == SDS__CHK_FREE)             \
            {                                           \
                fprintf(stderr, "    At file %s, line %d\n", __FILE__, __LINE__); \
                fprintf(stderr, "    This warning was enabled by SDS_CHECK_FREE or SDS_CHECK_FREE_W macro\n"); \
                *(_status) = STATUS__OK;                \
            }                                           \
        }                                               \
    }
#endif
#endif



#ifdef __cplusplus

} /* extern "C" */

/*
 * C++ only section.
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *  Here is the a C++ interface to SDS. No object code is required since
 *  all code is inline.
 *
 */
/** Helper class for access to an SDS Scaler Array.
 *
 * This is Data helper class for the ArrayAccess methods of the
 * SdsId:: class.  The main point here is to set up a releationship between 
 * an SDS data type and the relevant underlying Sds code - SdsCodeType of
 * the SDS scalar array we wish to access.  Once the
 * relationship is set up (by creating an subclass using this template)
 * and a call is made to SdsId::ArrayAccess(), then the methods of this
 * class allow access to the data using the index operator.  
 *
 * @warning The array index operator will throw an NoDataException exception
 *          if there is no data to access or a InvalidIndexException exception
 *          if the index for the data is invalid.
 *
 * @see SdsId::ArrayAccess 
 *
 * Various defined Instantiations of this template are provided - for 
 * each SDS code associated with the a scalar. 
 *
 * @see SdsArrayChar
 * @see SdsArrayUbyte
 * @see SdsArrayByte
 * @see SdsArrayShort
 * @see SdsArrayUshort
 * @see SdsArrayINT32
 * @see SdsArrayUINT32
 * @see SdsArrayINT64
 * @see SdsArrayUINT64
 * @see SdsArrayFloat
 * @see SdsArrayDouble
  * 
 */
#ifdef DRAMA_ALLOW_CPP_STDLIB
#include <string>
template <typename T> class SdsArrayAccessHelper {
public:
    friend class SdsId;
    /* Some exceptions thrown from the subscript operator */
    /**
     * This class is thrown by the SdsArraAccessHelper index operator
     * when it is found there is no data to access.
     */
    class NoDataException {
        std::string detail;
    public:
        /** NoDataException Constructor 
         *
         * @param why  Information on why this exception was thrown
         */
        NoDataException(const char *why) :detail(why) { }
        /** Return reason why the exception was thrown
         *
         */
        std::string const & GetDetails() const { return detail; };

    };
    /**
     * This class is thrown by the SdsArrayAccessHelper index operator
     * when it is found the index is invalid.
     */
    class InvalidIndexException {
        std::string detail;
    public:
         /** InvalidIndexException Constructor 
         *
         * @param why  Information on why this exception was thrown
         */
        InvalidIndexException(const char *why) :detail(why) { }
        /** Return reason why the exception was thrown
         *
         */
        std::string const & GetDetails() const { return detail; };
    };

private:
    SdsCodeType code;         /* The SDS code of the item */
    unsigned long elements;   /* Number of element in the array */
    T* data;                  /* The array pointer is written here */

    /*
     * Help function used to check an index is valid.  Also checks
     * that we have some data. Throws expections on problems.
     */
    void CheckIndexAndData(unsigned long index) const {
        if (!data)
        {
            throw new NoDataException("Index %d, Code %d, elements %d - data is invalid");
        }
        if (index >= elements)
        {
            throw new InvalidIndexException("Index %d, Code %d, elements %d - index is invalid");
        }
    }


    /*
     * The remaining private items are accessed by SdsId as a friend class.
     */
    /*
     * Return the address of the address of the data being accessed  
     *
     * This is used by SdsId.AccessArray() to put the address of the
     * data item.
     */
    void ** Data() { return (void **)&data; }

    /*
     * Set the number of elements in the array.
     *
     * This is used by SdsId.AccessArray() set the number of elements in
     * the array.
     */
    void SetElements(unsigned long nelem) {
        elements = nelem;
    }


protected:
    /** Constructor - only available to sub-classes.
     *
     * Construct an SdsArrayAccessHelper item.  Only sub-classes can invoke
     * this.  
     * 
     * @param c  The SDS Code assoicated with the template type.
     */
    SdsArrayAccessHelper(SdsCodeType c) : code(c), elements(0), data(0) {
    }
public:
 
    /** Return the SDS type code of the item being accessed
     */
    SdsCodeType Code() const { return code; }
    /** Return the number of elements in the array.
     */
    unsigned long Size() const { return elements; }
    /** SDS array subscript operator (const version)
     *
     * Access the data using the subscript operator - the only way of
     * getting at the data at the moment.
     *
     * @exception SdsArrayAccessHelper::NoDataException  Normally indicates
     *   that the array access operation failed.
     * @exception SdsArrayAccessHelper::InvalidIndexException  The index
     *   is invalid.
     *
     * @param index The index to access
     * @return A reference to the value.
     */
    T const & operator[](const unsigned long index) const {
        CheckIndexAndData(index);
        return data[index];
    }
    /** SDS array subscript operator (non-const version)
     *
     * Set/Access the data using the subscript operator - the only way of
     * getting at the data at the moment.
     *
     * @exception SdsArrayAccessHelper::NoDataException::  Normally indicates
     *   that the array access operation failed.
     * @exception SdsArrayAccessHelper::InvalidIndexException::  The index
     *   is invalid.
     *
     * @param index The index to access
     * @return A reference to the value.
     */
    T  & operator[](const unsigned long index) {
        CheckIndexAndData(index);
        return data[index];
    }
    /** Access the data directly.  read/write.
     * 
     * Returns the address of the raw data.  The user is responsible for
     * ensuring that any accesses are within the correct range - not more 
     * then Size() elements.
     */
    T * DataAddressRW() {
        return data;
    }
    /** Access the data directly - read only.
     * 
     * Returns the address of the raw data.  The user is responsible for
     * ensuring that any accesses are within the correct range - not more 
     * then Size() elements.
     */
    const T * DataAddressRO() const {
        return data;
    }
};  /* SdsArrayAccessHelper */

/*
 * Classes for each SDS data type.
 */
/** This is the SdsArrayAccessHelper class Instantiationed for access ot 
 *  SDS_CHAR type items.
 *
 * @see SdsArrayAccessHelper
 *
 */
class SdsArrayChar : public SdsArrayAccessHelper<char> {
public:
    SdsArrayChar() : SdsArrayAccessHelper<char>(SDS_CHAR) {
    }

};
/** This is the SdsArrayAccessHelper class Instantiationed for access ot 
 *  SDS_UBYTE type items.
 *
 * @see SdsArrayAccessHelper
 *
 */
class SdsArrayUbyte : public SdsArrayAccessHelper<unsigned char> {
public:
    SdsArrayUbyte() : SdsArrayAccessHelper<unsigned char>(SDS_UBYTE) {
    }

};
/** This is the SdsArrayAccessHelper class Instantiationed for access ot 
 *  SDS_BYTE type items.
 *
 * @see SdsArrayAccessHelper
 *
 */
class SdsArrayByte : public SdsArrayAccessHelper<signed char> {
public:
    SdsArrayByte() : SdsArrayAccessHelper<signed char>(SDS_BYTE) {
    }

};


/** This is the SdsArrayAccessHelper class Instantiationed for access ot 
 *  SDS_USHORT type items.
 *
 * @see SdsArrayAccessHelper
 *
 */
class SdsArrayUshort : public SdsArrayAccessHelper<unsigned short> {
public:
    SdsArrayUshort() : SdsArrayAccessHelper<unsigned short>(SDS_USHORT) {
    }

};
/** This is the SdsArrayAccessHelper class Instantiationed for access ot 
 *  SDS_SHORT type items.
 *
 * @see SdsArrayAccessHelper
 *
 */
class SdsArrayShort : public SdsArrayAccessHelper<short> {
public:
    SdsArrayShort() : SdsArrayAccessHelper<short>(SDS_SHORT) {
    }

};



/** This is the SdsArrayAccessHelper class Instantiationed for access ot 
 *  SDS_INT type items.
 *
 * @see SdsArrayAccessHelper
 *
 */
class SdsArrayINT32 : public SdsArrayAccessHelper<INT32> {
public:
    SdsArrayINT32() : SdsArrayAccessHelper<INT32>(SDS_INT) {
    }

};
/** This is the SdsArrayAccessHelper class Instantiationed for access ot 
 *  SDS_UINT type items.
 *
 * @see SdsArrayAccessHelper
 *
 */
class SdsArrayUINT32 : public SdsArrayAccessHelper<UINT32> {
public:
    SdsArrayUINT32() : SdsArrayAccessHelper<UINT32>(SDS_UINT) {
    }
};


/** This is the SdsArrayAccessHelper class Instantiationed for access ot 
 *  SDS_I64 type items.
 *
 * @see SdsArrayAccessHelper
 *
 */
class SdsArrayINT64 : public SdsArrayAccessHelper<INT64> {
public:
    SdsArrayINT64() : SdsArrayAccessHelper<INT64>(SDS_I64) {
    }

};
/** This is the SdsArrayAccessHelper class Instantiationed for access ot 
 *  SDS_UI64 type items.
 *
 * @see SdsArrayAccessHelper
 *
 */
class SdsArrayUINT64 : public SdsArrayAccessHelper<UINT64> {
public:
    SdsArrayUINT64() : SdsArrayAccessHelper<UINT64>(SDS_UI64) {
    }
};

/** This is the SdsArrayAccessHelper class Instantiationed for access ot 
 *  SDS_DOUBLE type items.
 *
 * @see SdsArrayAccessHelper
 *
 */
class SdsArrayDouble : public SdsArrayAccessHelper<double> {
public:
    SdsArrayDouble() : SdsArrayAccessHelper<double>(SDS_DOUBLE) {
    }
};
/** This is the SdsArrayAccessHelper class Instantiationed for access ot 
 *  SDS_FLOAT type items.
 *
 * @see SdsArrayAccessHelper
 *
 */
class SdsArrayFloat : public SdsArrayAccessHelper<float> {
 public:
    SdsArrayFloat() : SdsArrayAccessHelper<float>(SDS_FLOAT) {
    }
};

#endif /* defined DRAMA_ALLOW_CPP_STDLIB */

/** 
 * A C++ Interface to the handling SDS structures.
 *
 * The class SdsId provides a C++ interface to the AAO DRAMA SDS Library.
 * See <A href="http://www.aao.gov.au/drama/doc/ps/sds.ps"  TARGET="_top">The SDS Specification</A> for more details on SDS itself.
 *
 * An assortment of constructors are provided. Some of these access 
 * components of structures, some
 * access external items and some create new SDS structures.  Each SdsId class
 * object contains an SDS id which refers to an SDS structure, rather then a 
 * complete SDS structure and more then on SdsId can refer to the one 
 * underlying SDS structure.  Additionally, details about what to do when the
 * object goes out of scope are maintained, ensuring the the destructor
 * cleans up correctly.  
 *
 * If any of the contructors fail, then the current
 * implementation will set the inherited status argument bad rather then 
 * throwing an exception.  The object is still constructed in this case but 
 * will refer to an invalid SDS id, causing any other methods to fail (by 
 * setting status bad).  If the inherited status is not equal to STATUS__OK
 * on entry to any method with an inherited status argument, then the
 * method returns immediately.
 *
 * The various methods of the class implement those SDS functions which do
 * not return new SDS id's (SDS functions which return new SDS id's are
 * implemented as constructors).
 *
 *  The destructor will normally delete the underlying SDS item and and free 
 *  the SDS id, when appropiate.  These operations are normally dependent on 
 *  how the item was constructed, but can be changed if required (which might
 *  be required if the underlying SDS id is to be passed to a C function).
 *        
 *  Assignment and copying of these items is prohibited by making the
 *  operators private.  This is done since it is not clear what
 *  the user will expect from some operations. (for example, in a copy
 *  constructor, should the item be copied with the C routine
 *  DCF(SdsCopy) or just a
 *  new id to the same item generated.  SDS does not even support the
 *  later.  Also, management of the inherited status is a problem). 
 *        
 *  You can pass items to subroutines by reference or pointer to get 
 *  around the copy problem.  A constructor which creates a copy of an 
 *  existing object can be used in the place of assignment in many cases.
 *  You can use the member functions ShallowCopy() (which just copies the 
 *  Sds id itself) and DeepCopy(), to get a copy in an explicit maner in other
 *  cases.
 * 
 * @see Arg
 * @see <A href="http://www.aao.gov.au/drama/doc/ps/sds.ps"  TARGET="_top">SDS Specification</A>
 * @see <A href="http://www.aao.gov.au/drama/doc/ps/drama_cpp.ps"  TARGET="_top">DRAMA C++ interface</A>
 * @see <A href="http://www.aao.gov.au/drama/doc/ps/dits_spec_5.ps"  TARGET="_top">DRAMA DITS Specification</A>
 *  
 */

SDSCLASS SdsId {
    private:
    	SdsIdType id;	/* Actual Sds id.  If 0, then no id is allocated */
    	struct {
    		bool free : 1;/* Set true to cause destructor to free the id*/
    		bool del  : 1;/* If free is true, destructor will do SdsDelete*/
    		bool readfree : 1;/* if free and del, to SdsReadFree */
    		} flags;

	/* Private routine to clean up, will be called by destructor and 
	   Shallow and Deep copy operations */

        /* If this check free code is not to be run, we don't want the from_where argument, it
           causes warnings in all using code.  This macro setup allows that
        */
#if defined(SDS_CHECK_FREE)||defined(SDS_CHECK_FREE_W)            
#define SDS__CHECK_ARG(_arg_) _arg_
#else
#define SDS__CHECK_ARG(_arg_) 
#endif


    	void CleanUp(const char * SDS__CHECK_ARG(from_where)) {
    	    /* If we have an id and the free flag is set */
	    /* In the future, we should consider do a throw if status goes */
	    /* bad, instead of just ignoring it			*/
    	    
    	    if ((id)&&(flags.free))
    	    {
    	    	/* If del and readfree, we need to do SdsReadFree */
    	        if ((flags.del)&&(flags.readfree))
    	        {
    		    StatusType ignore = STATUS__OK;
    		    SdsReadFree(id,&ignore);
    	        }
    	        /* If del and not readfree, we need to to SdsDelete */
    	        else if (flags.del)
    	        {
    		    StatusType ignore = STATUS__OK;
    		    SdsDelete(id,&ignore);
    	        }
    	        /* Free the id */
    		StatusType ignore = STATUS__OK;
#if defined(SDS_CHECK_FREE)||defined(SDS_CHECK_FREE_W)            
                SdsFreeIdAndCheck("", id, &ignore);
                if (ignore != STATUS__OK)
                {
                    fprintf(stderr, "SDS FreeId failure in C++ cleanup(), called from \"%s\" for id %d\n", from_where,(int)id);
                    fprintf(stderr, "    Status Code:%s\n", SdsErrorCodeString(ignore));
                    fprintf(stderr, "    SdsId object address is %p\n", (void *)(this));
                    fprintf(stderr, "    Sorry - don't have more exact location information\n");
                    fprintf(stderr, "    This warning was enabled by SDS_CHECK_FREE or SDS_CHECK_FREE_W macro\n");
                }
#else
    		SdsFreeId(id,&ignore);
#endif

            }
    	}
    	/* Assignment and Copy operators, made private to avoid misuse */	
    	SdsId& operator=(const SdsId& a); /* Assignment */
    	SdsId(const SdsId&); /* Copy */


#ifdef DRAMA_ALLOW_CPP_STDLIB
        /* This is used by the ArrayAccess methods to check the item
           is appropiate for the SdsArrayAccessHelper being used
        */
        void CheckArrayType(
            const SdsCodeType code,     /* Expected code */
            const int ndims,            /* Expected number of dims */
            unsigned long * const dims, /* Actual dims written here */
            StatusType * const status) {
            
            if (*status != STATUS__OK) return;
            char name[SDS_C_NAMELEN];
            SdsCodeType tcode;
            long tndims;
            /*
             * Get details on the object.
             */
            Info(name, &tcode, &tndims, dims, status);
            /*
             * Check the code and number of dimensions are correct.
             */
            if (tcode != code)
            {
                *status = SDS__TYPE;
                return;
            }
            if (tndims == 0)
            {
                *status = SDS__NOTARR;
                return;
            }
            if (tndims != ndims)
            {
                *status = SDS__INVDIMS;
                return;
            }
            
        }
        /*
         * A simplified access to  CheckArrayType, for a single dimensional
         * array.
         */
        void CheckArrayTypeSingleDim(
            const SdsCodeType code, 
            unsigned long * const nitems,  /* Actual number of items written here*/
            StatusType * const status) {
            
            unsigned long dims[SDS_C_MAXARRAYDIMS];
            CheckArrayType(code, 1, dims, status);
            if (*status != STATUS__OK) return;
            *nitems = dims[0];
            
        }
#endif /* defined DRAMA_ALLOW_CPP_STDLIB */

    public:
    	/**
         * Construct an SdsId item from an existing C language SDS id.
         *  
         *  This SdsId constructor that takes an existing SDS id
         *      (C style) and creates an SdsId class object which refers
         *      to that SDS id.  You must explictly specify if the item is to be
         *      free-ed and/or deleted when the object's destructor is invoked,
         *      and what you do depends on when what other code may do with
         *      item.  The del and readfree are mutually exclusive, through
         *      this is currently not checked in this implementation.
         *
         * @param item The SdsIdType of the item.  If 0 (the default), the 
         *             SdsId object constructed does not refer to any SDS item.
         *             This is normally only used when we want an argument
         *             to pass (by pointer) to a function which will use
         *             SdsId::ShallowCopy or SdsId::DeepCopy
         *             to set the ID.
         *
         * @param free Set true to invoke DCF(SdsFreeId) on the ID when the 
         *             object  is  destroyed.  The default is false.
         *
         * @param del  Set true to invoke DCF(SdsDelete) on the ID when 
         *             the object is destroyed.  The default is false.
         *
         * @param readfree Set true to invoke DCF(SdsReadFree) on the ID 
         *             when the object is destroyed.  The default is false.
         *
    	 */
    	SdsId(const SdsIdType item = 0, const bool free=false, 
    			const bool del = false, const bool readfree = false) :
    		id(item) {
    		 flags.free = free;
    		 flags.del = del; 
    		 flags.readfree = readfree;
    	}
    	/**
         * Constructor that accesses an exported SDS structure in a byte stream.
         *
         *  This constructor uses DCF(SdsImport) or DCF(SdsAccess)
         *              to get access an SDS structure stored in an
         *              external byte stream
         *
         * @see DCF(SdsImport).
         * @see DCF(SdsAccess).
         *
         *  @param data A pointer to a byte stream created with 
         *              DCF(SdsExport) or SdsId::Export .
         *
         *  @param status Inherited status.
         *  @param import If true, then import the structure such that is an
         *                internal SDS structure.  This allows the structure
         *                to be modified in any fashion, but is slower and 
         *                requires more memory.
         */
    	SdsId(void * const data, StatusType * const status, 
    		   const bool import = false) :
    		id(0) {
    	    flags.free = true;
    	    flags.readfree = false;
            if (import)
            {
    	        SdsImport(data,&id,status);
    	        flags.del = true;
    	    }
            else
            {
    	        SdsAccess(data,&id,status);
    	        flags.del = false; 
    	    }
    	}
    	/**
         *  Constructor that imports a previously exported SDS structure from a 
         *  const byte stream.
         *
         *  This constructor uses DCF(SdsImport)
         *              to get access an SDS structure stored in an
         *              const external byte stream.  Note that since
         *              the data is const, only a  DCF(SdsImport) is possible.
         *
         * @see DCF(SdsImport).
         *
         *  @param data A pointer to a byte stream created with 
         *              DCF(SdsExport) or SdsId::Export .
         *  @param status Inherited status.
         *
         */
    	SdsId(const void * const data, StatusType * const status) : id(0) {
    	    flags.free = true;
    	    flags.readfree = false;
    	    flags.del = true;
    	    SdsImport(data,&id,status);
    	}
    	/** Constructor which creates an SDS Id after reading a structure from 
	 * a file.
         *
         * The specified file should contain an SDS structure created
         * using DCF(SdsWrite) or SdsId::Write .
         *
         * @see SdsId::Write
         * @see DCF(SdsRead)
         *
         * @param filename Name of the file.
         * @param status Inherited status.
         *
         */
    	SdsId(const char * const filename, StatusType * const status) :
    		id(0) {
    	    flags.free = true;
    	    flags.del = true;
    	    flags.readfree = true;
    	    SdsRead(filename,&id,status);
    	}
    	/** Constructor which creates a new (non-array) child item.
         *
         * This contructor creates a new SDS item which is a child of
         * the specified SDS item in the specified SdsId object.
         *
         * @see DCF(SdsNew)
         *
         * @param parent_id The parent SDS item.
         * @param name      The name for the new SDS item.
         * @param code      The SDS code for the new item.
         * @param status    Inherited status.
         * @param nextra    The about of extra data, default to 0 (no extra
         *                  data).
         * @param extra     The extra data.
         */
    	SdsId(const SdsId &parent_id, const char * const name, 
    	    const SdsCodeType code,
    	    StatusType * const status, const long nextra = 0, 
    	    const char * const extra = 0) :
    		id(0) {
    	    flags.free = true;
    	    flags.del = false;
    	    flags.readfree = false;
     	    SdsNew(parent_id.id,name,nextra,extra,code,0,0,&id,status);
    	}    
    	/** Constructor which creates a new (non-array) top-level item.
         *
         * This constructor creates a new SDS top-level SDS item.
         *
         * @param name      The name for the new SDS item.
         * @param code      The SDS code for the new item.
         * @param status    Inherited status.
         * @param nextra    The about of extra data, default to 0 (no extra
         *                  data).
         * @param extra     The extra data.
         */
    	SdsId(const char * const name, const SdsCodeType code,
    	    StatusType * const status, const long nextra = 0, 
    	    const char * const extra = 0) :
    		id(0) {
    	    flags.free = true;
    	    flags.del = true;
    	    flags.readfree = false;
     	    SdsNew(0,name,nextra,extra,code,0,0,&id,status);
    	}    
    	/** Constructor which creates a new array child item.
         *
         * This contructor creates a new SDS structure array which is a child of
         * the specified SDS item in the specified SdsId object.
         *
         * @see DCF(SdsNew)
         *
         * @param parent_id The parent SDS item.
         * @param name      The name for the new SDS item.
         * @param code      The SDS code for the new item.
         * @param ndims     The number of dimensions, 1 through 7.
         * @param dims      An array giving each dimension.
         * @param status    Inherited status.
         * @param nextra    The about of extra data, default to 0 (no extra
         *                  data).
         * @param extra     The extra data.
         */
    	SdsId(const SdsId &parent_id, const char * const name, 
    	    const SdsCodeType code,
    	    const long ndims, const unsigned long *dims,
    	    StatusType * const status, const long nextra = 0, 
    	    const char * const extra = 0) :
    		id(0) {
    	    flags.free = true;
    	    flags.del = false;
    	    flags.readfree = false;
     	    SdsNew(parent_id.id,name,nextra,extra,code,ndims,dims,&id,status);
    	}    
    	/** Constructor which creates a new array top-level item.
         *
         * This constructor creates a new SDS top-level SDS structure array.
         *
         * @see DCF(SdsNew)
         *
         * @param name      The name for the new SDS item.
         * @param code      The SDS code for the new item.
         * @param ndims     The number of dimensions, 1 through 7.
         * @param dims      An array giving each dimension.
         * @param status    Inherited status.
         * @param nextra    The about of extra data, default to 0 (no extra
         *                  data).
         * @param extra     The extra data.
         */
    	/* Constructor which creates a new array top-level item */
    	SdsId(const char * const name, const SdsCodeType code,
    	    const long ndims, const unsigned long *dims,
    	    StatusType * const status, const long nextra = 0, 
    	    const char * const extra = 0) :
    		id(0) {
     	    flags.free = true;
    	    flags.del = true;
    	    flags.readfree = false;
    	    SdsNew(0,name,nextra,extra,code,ndims,dims,&id,status);
    	}    
    	
    	/** Constructor that returns a cell of an existing array id 
         *
         *  This constructor is used to access a specified cell of
         *  an existing SDS structure array.
         *
         * @see DCF(SdsCell).
         *
         * @param array_id   An ID which refers to a structure array.
         * @param nindicies  The number of indicies in the array
         * @param indicies   An array giving the indicies of the cell in question.
         * @param status    Inherited status.
         *
         */
    	SdsId(const SdsId &array_id, const long nindicies, 
    	    const unsigned long * const indicies, StatusType * const status) :
    		id(0) {
    	    flags.free = true;
    	    flags.del = false;
    	    flags.readfree = false;
     	    SdsCell(array_id.id, nindicies, indicies, &id,status);	
    	}
    	/** SdsId Copy constructor.
         *
         * This constructor is used to generate a copy of an existing SDS
         * structure.  This is a deep copy - a new internal SDS item
         * is created.
         *
         * @see DCF(SdsCopy).
         *
         * @param source The SDS item to be copied.
         * @param status    Inherited status.
         *
         */
    	SdsId(const SdsId& source, StatusType * const status):
    		id(0) {
    	    flags.free = true;
    	    flags.del = true;
    	    flags.readfree = false;
     	    SdsCopy(source.id, &id, status);
    	}
    	/** Constructor which returns a reference to a named item.
         *
         * This constructor searches an existing SDS item for a item with
         * a specified name and creates a reference to it.  
         *
         * @see DCF(SdsFind)
         *
         * @param source The SDS item to find name within.
         * @param name   The name of the SDS item to find.
         * @param status    Inherited status.
         *
         */
    	SdsId(const SdsId& source, const char * const name, 
    		StatusType * const status) :
    		id(0) {
    	    flags.free = true;
    	    flags.del = false;
    	    flags.readfree = false;
     	    SdsFind(source.id,name,&id,status);
    	}
    	
    	/** Constructor which returns an id to a structured item indexed 
    	 *  by position
         *
         *  This constructor uses the specified index to find an item within
         *  an SDS structure.
         *
         * @see DCF(SdsIndex)
         *
         * @param source The SDS item to index within.
         * @param index  The index of the item fo find.  Items in a structure are 
         *               numbered in order of creation starting with one.
         * @param status    Inherited status.
         *
         **/
    	SdsId(const SdsId& source, const long index, StatusType * const status):
    		id(0) {
    	    flags.free = true;
    	    flags.del = false;
    	    flags.readfree = false;
     	    SdsIndex(source.id,index,&id,status);
    	}
    	
    	/** SdsId  Destructor.  
         *
         * The DCF(SdsDelete)/ DCF(SdsReadFree) may be invoked on the 
         * underlying SDS id  as may DCF(SdsFreeId).  What is to happen 
         * depends on how the item was
         * constructed or how the flags were changed.  In general, the 
         * required result is achived.
         *
         * @see SdsId::SetFree()
         * @see SdsId::SetDelete()
         * @see SdsId::ClearDelete()
         * @see SdsId::Outlive()
         * @see DCF(SdsFreeId)
         * @see DCF(SdsDelete)
         * @see DCF(SdsReadFree)
         */
        /*Just invoke the private cleanup routine */
    	virtual ~SdsId() {
    	    CleanUp("SdsId destructor");
    	}
    	/* Modify flags */

        /** Indicate the underlying SDS item should be free-ed when the SdsId
         * object is destroyed.
         *
         * @see SdsId::OutLive()
         */
    	void SetFree() { flags.free = true; }
        /** Indicate the underlying SDS structure should be deleted when the
         * SdsId object is destroyed
         *
         * @see SdsId::OutLive()
         */
    	void SetDelete() { flags.del = true; }

        /** Indicate the underlying SDS structure should NOT be deleted when
         * the SdsId object is desroyed.
         *
         * This should be considered if the SDS object refered to was created
         * when the SdsId object is created and must outlive the object.
         *
         * @see SdsId::OutLive()
         */
    	void ClearDelete() { flags.del = false; }

    	/** Force the actual SDS ID to outlive the SdsId variable 
         *
         * This method should be invoked if it is determined that the
         * underlying SDS ID/Structure should outlive the SdsId object.  Typically,
         * this might be done if the underlying ID is passed to some other function
         *
         */
    	void Outlive() { flags.free = false; flags.del = false; 
    			   flags.readfree = false; }
    	
/*
 *	General operations, as per similar Sds operation.
 *	Currently only Delete, Get and Put are virtual.
 */
    	
    	/** Delete the SDS item.
         *
         * DCF(SdsDelete) is invoked on the SDS ID refered to by the Object.  
         * This
         * is generally used to delete items which are a component of other
         * objects.  The ID is also free-ed if the approiate flag is set
         *
         * After this method is invoked, this object is invalid and many methods
         * will now fail.  But you can use the copy methods to insert a new
         * item.
         *
         * @see DCF(SdsDelete)
         * @see DCF(SdsFreeId)
         *
         * @param status    Inherited status.
         */
    	virtual void Delete(StatusType * const status) {
    	    SdsDelete(id,status);
    	    if (*status == STATUS__OK)
            {
    	        flags.del = false;
                if (flags.free)
                {
                    SdsFreeId(id, status);
                    if (*status == STATUS__OK)
                    {
                        id = 0;
                        flags.free = false;
                    }
                }
            }
    	}
    	/** Export the SDS structure into a buffer.
         *
         * The SDS structure refered to by this SdsId object is exported
         * into the specified buffer.  
         *
         * Note that if any of the underlying data is not defined (say
         * because  you created a new item but have not yet put any data into it)
         * then the size will not include space for the data itself.  This
         * is acceptable to SDS but may not be what you expect.  To create
         * the required space in the exported object, use the SdsId::ExportDefined
         * method.
         *
         * @see DCF(SdsExport)
         * @see SdsId::ExportDefined
         *
         * @param length  The length of the buffer.  Should be the value
         *                returned by DCF(SdsSize) or SdsId::Size
         * @param data    The buffer to write the data into. Should be of at
         *                least the size specified by length.
         * @param status    Inherited status.
         *
         */
    	void Export(const unsigned long length, 
    		    void * const data,
    		    StatusType * const status) {
    	    SdsExport(id,length,data,status);
        }
    	/** Export the SDS structure into a buffer, defining any undefined data.
         *
         * The SDS structure refered to by this SdsId object is exported
         * into the specified buffer, with any undefined data defined.
         *
         * Data may be undefined if you created a new item but have not yet put any 
         * data into it.  The standard DCF(Export) method does not allow space
         * for this data, but this method does.
         *
         * @see DCF(SdsExportDefined)
         * @see SdsId::Export
         *
         * @param length  The length of the buffer.  Should be the value
         *                returned by DCF(SdsSizeDefined) or SdsId::SizeDefined
         * @param data    The buffer to write the data into. Should be of at
         *                least the size specified by length.
         * @param status    Inherited status.
         *
         */    	
        void ExportDefined(const unsigned long length, 
    		    void * const data,
    		    StatusType * const status) {
    	    SdsExportDefined(id,length,data,status);
        }
        /**  Extract the SDS structure from its parent.
         *
         * This method is normally applied to an SdsId object which refers
         * to a child of another SDS structure.  This method extracts the child
         * to create a new top-level SDS structure
         *
         * @see DCF(SdsExtract)
         * @see SdsId::Pointer
         * @see DCF(SdsPointer)
         *
         * @param status    Inherited status.
         */
    	void Extract(StatusType * const status) {
    	    SdsExtract(id,status);
    	    /* If we succeed, we need to modify the flags to appropiate for
    	       and indepent toplevel item */
    	    if (*status == STATUS__OK)
    	    {
    	        flags.free = true;
    	        flags.del = true;
    	        flags.readfree = false;
    	    }
        }
        /** Flush data modified by pointer.
         *
         * When the data of an SDS is modified via a pointer obtained by
         * DCF(SdsPointer) or SdsId::Pointer, then this method should be used
         * to ensure the data is updated correctly.  
         *
         * This is currently a null method for all machines on which SDS is 
         * currently implemented, but may be required in the future.
         *
         * @see DCF(SdsFlush)
         *
         * @param status    Inherited status.
         */
        void Flush(StatusType * const status) {
            SdsFlush(id,status);
        }
        /** Get data from an SDS item.
         *
         * This method accesses the data from an SDS item.  The data is copied
         * into the specified buffer.  
         *
         * Note that data from nested structures is also copied.  Please see
         * the SDS manual for more details on Get operations.
         *
         * @see DCF(SdsGet)
         * @see SdsId::Pointer
         * @see SdsId::Put
         * @see DCF(SdsPut)
         *
         * @param length    The amount of data to fetch.
         * @param data      The buffer to put the data info, must of of size 
         *                  length or greater.
         * @param status    Inherited status.
         * @param actlen    The actual amount of data retrived is written here.  If
         *                  not supplied, this information is not returned.
         * @param offset    An offset into the SDS structure to start fetch from, in
         *                  bytes
         */
        virtual void Get(const unsigned long length,
        	 void * const data,
        	 StatusType * const status,
        	 unsigned long *actlen = 0,
        	 const unsigned long offset=0) const
        {
            unsigned long myactlen;
            if (!actlen) actlen = &myactlen;
            SdsGet(id,length,offset,data,actlen,status);
        }
        /** Get extra data from an SDS item.
         *
         * This method accesses the extra data from an SDS item.  The data is copied
         * into the specified buffer.  
         *
         * @see DCF(SdsGetExtra)
         * @see SdsId::PutExtra
         * @see DCF(SdsPutExtra)
         *
         * @param length    The amount of data to fetch.
         * @param extra     The buffer to fetch the data into, must of of size 
         *                  length or greater.
         * @param status    Inherited status.
         * @param actlen    The actual amount of data retrived is written here.  If
         *                  not supplied, this information is not returned.
         */
        void GetExtra(const unsigned long length,
        	 char * const extra,
        	 StatusType * const status,
        	 unsigned long *actlen = 0) const
        {
            unsigned long myactlen;
            if (!actlen) actlen = &myactlen;
            SdsGetExtra(id,length,extra,actlen,status);
        }
        /** Return information about an SDS item.
         *
         * This method returns all the standard information about an SDS
         * item.
         *
         * @see DCF(SdsInfo)
         * @see SdsId::Code
         * @see SdsId::Dims
         * @see SdsId::Dims
         *
         * @param  name   The name of the SDS item is returned here.  There should 
         *                be space for SDS_C_NAMELEN (16) 16 bytes.
         * @param  code   The SDS item code is returned here..
         * @param  ndims  The number of dimensions is returned here, 
         *                0 if the item is not an array,
         *                otherwise, 1 through 7.
         * @param  dims   The dimensions are retuned here.  This should be 
         *                an array of 7 items.
         * @param status    Inherited status.
         */
        void Info(char * const name,
        	  SdsCodeType * const code,
        	  long * const ndims,
        	  unsigned long * const dims,
        	  StatusType * const status) const {
            SdsInfo(id,name,code,ndims,dims,status);
        }
        /* Give access to individual things normally returned by SdsInfo*/
        /** Return the name of the SDS item
         *
         * Just return the name of the underlying SDS item.  
         *
         * Note that if  you want more then
         * one of the items returned by SdsId::GetName, SdsId::Code and
         * SdsId::Dims, then use SdsId::Info, it is more efficent.
         *
         * @see SdsId::Info
         * @see DCF(SdsInfo)
         *
         * @param  name   The name of the SDS item is returned here.  There should 
         *                be space for SDS_C_NAMELEN (16) 16 bytes.
         * @param status    Inherited status.
         */
        void GetName(char * const name,
        	     StatusType * const status) const {
            SdsCodeType code;
            long ndims;
            unsigned long dims[SDS_C_MAXARRAYDIMS];
            SdsInfo(id,name,&code,&ndims,dims,status);
        }
        /** Return the code of the SDS item
         *
         * Just return the code of the underlying SDS item.  
         *
         * Note that if  you want more then
         * one of the items returned by SdsId::GetName, SdsId::Code and
         * SdsId::Dims, then use SdsId::Info, it is more efficent.
         *
         * @see SdsId::Info
         * @see DCF(SdsInfo)
         *
         * @param  code   The SDS item code is returned here..
         * @param status    Inherited status.
         *
         */
        void Code(SdsCodeType * const code,
        		StatusType * const status) const {
            char name[SDS_C_NAMELEN];
            long ndims;
            unsigned long dims[SDS_C_MAXARRAYDIMS];
            SdsInfo(id,name,code,&ndims,dims,status);
        }
        /** Return the dimensions of the SDS item
         *
         * Just return the dimensions of the underlying SDS item. 
         *
         * Note that if  you want more then
         * one of the items returned by SdsId::GetName, SdsId::Code and
         * SdsId::Dims, then use SdsId::Info, it is more efficent.
         *
         * @see SdsId::Info
         * @see DCF(SdsInfo)
         *
         * @param  ndims  The number of dimensions is returned here, 
         *                0 if the item is not an array,
         *                otherwise, 1 through 7.
         * @param  dims   The dimensions are retuned here.  This should be 
         *                an array of 7 items.
         * @param status    Inherited status.
         *
         */
         void Dims(long * const ndims, unsigned long * const dims,
        		StatusType * const status) const {
            char name[SDS_C_NAMELEN];
            SdsCodeType code;
            SdsInfo(id,name,&code,ndims,dims,status);
        }
         /** Insert an SDS object into this object 
          *
          * This method inserts an existing SDS structure into the
          * SDS structure refered to by this object.
          *
          * @see DCF(SdsInsert)
          *
          * @param to_insert The object to insert.
          * @param status    Inherited status.
          */
        void Insert(SdsId & to_insert, StatusType * const status) {
            SdsInsert(id,to_insert.id,status);
            /* If we succeed, the new child should not be deleted when
            	it's id is destroyed, so clear the flag */
            if (*status == STATUS__OK)
                to_insert.flags.del = false;
        }
        /** Insert an SDS object into this object, which is a structured array 
         *
         * This object inserts a top level object into a structure array at a 
         * position specified by its indices.  It deletes any object that is 
         * currently at that position.
         *
         * @see DCF(SdsInsertCell)
         * @see SdsId::FillArray
         *
         * @param to_insert The object to insert.
         * @param ndims     The number of dimensions to the array.  1 through 7.
         * @param dims      The cell indicies.
         * @param status    Inherited status.
         */
        void Insert(SdsId & to_insert, const long ndims,
			const unsigned long * const dims, 
			StatusType * const status) {
            SdsInsertCell(id,ndims,dims,to_insert.id,status);
            /* If we succeed, the new child should not be deleted when
            	it's id is destroyed, so clear the flag */
            if (*status == STATUS__OK)
                to_insert.flags.del = false;
        }

        /** Fill out the contents of this object, which is a structured array.
         *
         * This routine will fill out an array of structures item with the 
         * copies of a specified struture.  
         *
         * @see DCF(SdsFillArray)
         * @see SdsId::Insert(SdsId &to_insert, const long ndims, const unsigned long *const dims, StatusType *const status)
         *
         * @param elem      The item to copy into the structure.
         * @param status    Inherited status.
         */
	void FillArray(const SdsId &elem, StatusType * const status) {
	    SdsFillArray(id,elem.id,status);
	}

        /** Object a pointer to the data area of a primitive SDS item.
         *
         * Return a pointer to the data of a primitive item.  If the data item is 
         * undefined and the object is internal, storage for the data will be 
         * created.  This can only be used with primitive items, not with structures.
         * 
         * If necessary (e.g. if the data originated on a machine with different 
         * architecture) the data for the object is converted (in place) from 
         * the  format stored in the data item to that required for the local
         * machine
         *
         * If the data pointed to by the pointer is updated by a calling program, 
         * the program should then call the function DCF(SdsFlush)/ SdsId::Flush to 
         * ensure that the data is updated in the original structure. This is 
         * necessary because implementations on some machine architectures may 
         * have to use a copy of the data rather than the actual data when 
         * returning a pointer.
         *
         * @see DCF(SdsPointer)
         *
         * @param data The address of a item to return the pointer to.
         * @param status    Inherited status.
         * @param length    If specified, the length of the data is returned here.
         */
        void Pointer(void **data, StatusType * const status, 
        	     unsigned long * length = 0) {
            unsigned long mylength;
            if (!length) length = &mylength;
            SdsPointer(id,data,length,status);
        }

        /** Put data into an SDS item.
         *
         * This method put the data into an SDS item.  The data is copied
         * into the specified buffer.  
         *
         * Note that data from nested structures is also copied.  Please see
         * the SDS manual for more details on Put operations.
         *
         * @see DCF(SdsPut)
         * @see SdsId::Get
         * @see SdsId::Pointer
         * @see DCF(SdsGet)
         *
         * @param length    The amount of data to put.
         * @param data      The buffer of the data, must of of size length or
         *                  greater.
         * @param status    Inherited status.
         * @param offset    An offset into the SDS structure to start fetch from, in
         *                  bytes
         */
        virtual void Put(const unsigned long length,
        	 void * const data,
        	 StatusType * const status,
        	 const unsigned long offset=0)
        {
            SdsPut(id,length,offset,data,status);
        }
       /** Put extra data into an SDS item.
         *
         * This method puts the extra data from an SDS item.  The data is copied
         * into the specified buffer.  
         *
         * @see DCF(SdsPutExtra)
         * @see SdsId::GetExtra
         * @see DCF(SdsGetExtra)
         *
         * @param nextra    The amount of data to put
         * @param extra     The buffer of the data, must of of size length or
         *                  greater.
         * @param status    Inherited status.
         */
        void PutExtra(const long nextra,
        	 const char * const extra,
        	 StatusType * const status)
        {
            SdsPutExtra(id,nextra,extra,status);
        }
        /** Rename the SDS item.
         *
         * @see DCF(SdsRename)
         *
         * @param name The new name, must be less then SDS_C_NAMELEN (16) bytes
         *             including the null terminator.
         * @param status    Inherited status.
         *
         */	
        void Rename(const char * const name, 
        		StatusType * const status) {
            SdsRename(id,name,status);
        }
        /** Change the dimensions of an SDS array.
         *
         * This object must be an SDS array (either primitive or structured).  This
         * method resizes the arrary.  Note that it does not move the data elements 
         * in the storage representing the array, so there is no guarantee that 
         * after resizing the array the same data will be found at the same array 
         * index positions as before resizing, though this will be the case for 
         * simple changes such as a change in the last dimension only.
         *
         * If the size of a primitive array is increased the contents of the extra 
         * locations is undefined. Decreasing the size causes the data beyond the 
         * new limit to be lost.
         *
         * If a structure array is extended the new elements created will be 
         * empty structures. If a structure array is decreased in size, the lost 
         * elements and all their components will be deleted.
         *
         * @see DCF(SdsResize)
         *
         * @param ndims The number of dimensions
         * @param dims  The new dimensions
         * @param status    Inherited status.
         */
        void Resize(const long ndims, const unsigned long *dims, 
        		StatusType * const status) {
             SdsResize(id,ndims,dims,status);
        }
        /** Return the size of an SDS structure, as required for exporting.
         *   
         * @see SdsId::Export
         * @see DCF(SdsSize)
         *
         * @param bytes The number of bytes to allow for exporting this structure.
         * @param status    Inherited status.
         */
        void Size(unsigned long * const bytes, 
        	  StatusType * const status) const {
            SdsSize(id,bytes,status);
        }
         /** Return the size of an SDS structure, as required for exporting defined.
         *   
         * @see SdsId::ExportDefined
         * @see DCF(SdsSizeDefined)
         *
         * @param bytes The number of bytes to allow for exporting this structure
         *              when all elements are defined.
         * @param status    Inherited status.
         */
        void SizeDefined(unsigned long * const bytes, 
        	  StatusType * const status) const {
            SdsSizeDefined(id,bytes,status);
        }
        /** List the contents of the structure to standard output.
         *
         * @see DCF(SdsList)
         *
         * @param status    Inherited status.
         */
        void List(StatusType * const status) const {
            SdsList(id,status);
        }
        /** List the contents of the structure to a C file
         *
         * @see DCF(SdsList)
         *
         * @param status    Inherited status.
         */
        void List(FILE *to, StatusType * const status) const {
            SdsListTo(to, id,status);
        }

        /** Write the contents of the structure to a file.
         *
         * This method exports the SDS structure into a malloced buffer, before
         * writing that buffer to a the specified file.
         *
         * @see DCF(SdsWrite)
         *
         * @param filename  The name of the file.  Will be overwritten if it already
         *                  exists.
         * @param status    Inherited status.
         */
	void Write(const char * const filename,
		   StatusType * const status) const {
	    SdsWrite(id,filename,status);
	}        

        /** Determine if the SDS structure is external.
         *
         * This method is used to determine if the SDS object refered to by
         * this object is an external object.  Note that external objects
         * cannot have their structure modified.
         *
         * @see DCF(SdsIsExternal)
         *
         * @param external  A non-zero value is written here if the object is 
         *                  external
         * @param status    Inherited status.
         */
        void IsExternal(int * const external, 
		   StatusType * const status) const {
            SdsIsExternal(id, external, status);
        }
        /** Return the address of an external item.
         *
         * This method is used to determine the address of the buffer containing
         * an external item, if this item refers to an external item. This is
         * the value given the contructor which accessed this item.
         *
         * @see SdsId::SdsId(void *const data, StatusType *const status, const bool import)
         *
         * @param data The address is written here.
         * @param status    Inherited status.
         */
        void GetExternInfo(void **data, 
		   StatusType * const status) const {
            SdsGetExternInfo(id, data, status);
        }
	
	/** Operator which returns the underlying SDS id 
         *
         * This operator allows the SdsId object to be used in calls to 
         * functions which expect an SdsIdType argument.  Note that you 
         * must ensure that object is nether deleted or free-ed by whatever
         * uses it. See SdsId::COut if that might happen.
         *
         * @see SdsId::COut
         *
         * @return The underlying SDS ID value.
         */
	operator SdsIdType(void) const { return id; }
	
	/** Operator which tests if an item is valid.  
         *
         * @return True if an item appears to be valid (the underlying SDS ID 
         *         is non-zero).
         */
        /* Note, if there is no bool type, we can just use the above operator which 
           returns the Sds id. Similary for Borland and Sunpro CC (Version 6) which do not like
           this operator (ambigious with the above operator 
         */
#	if (!defined(CPP_NOBOOL))  && (!defined(BORLAND)) && (!defined(__SUNPRO_CC))
	operator bool(void) const { return (id != 0); }
#	endif
	
	/** Return this item as an SdsIdType for return to C code.
         *
         * This method will setup for and obtain details of this item for return 
         * of this  item using an SdsIdType.  Of particular note -
         * if outlives is true, the SdsIdType item returned by this is considered
         * to outlive this object and we leave the freeing of the id etc 
         * up to the caller, who can use the other variables to work out 
         * what to do.  If outlives is false, can  method  be used as an enquiry
         * for what is to happen when the destructor is invoked.
         *
         * @param outlives  If true, then the returned value will outlive this
         *                  object and this object is modified to reflect this.
         * @param free      If specified, set true to indicate that the destructor 
         *                  for this object would have invoked DCF(SdsFreeId) on the
         *                  underlying SDS ID.  If outlives is true, this won't 
         *                  now happen.
         * @param del      If specified, set true to indicate that the destructor 
         *                  for this object would have invoked DCF(SdsDelete) on the
         *                  underlying SDS ID.  If outlives is true, this won't 
         *                  now happen.
         * @param readfree If specified, set true to indicate that the destructor 
         *                  for this object would have invoked DCF(SdsReadFree) on the
         *                  underlying SDS ID.  If outlives is true, this won't 
         *                  now happen.
         *
         * @return The underlying SDS ID value.
         */
	SdsIdType COut(const bool outlives, bool * const free = 0,
		bool * const del= 0, bool * const readfree = 0) {
	    if (free)
	    	*free = flags.free;
	    if (del)
	    	*del  = flags.del;
	    if (readfree)
	    	*readfree = flags.readfree;
	    if (outlives)
	        flags.free = flags.del = flags.readfree = false;
	    return (SdsId::id);
	}
	/* Obsolete version 	*/
	/** Return this item as an SdsIdType for return to C code.
         *
         * @deprecated Please use the version which returns the value.
         *
         * This method will setup for and obtain details of this item for return 
         * of this  item using an SdsIdType.  Of particular note -
         * if outlives is true, the SdsIdType item returned by this is considered
         * to outlive this object and we leave the freeing of the id etc 
         * up to the caller, who can use the other variables to work out 
         * what to do. 
         *
         * @param outlives  If true, then the returned value will outlive this
         *                  object and this object is modified to reflect this.
         * @param item      the SDS item is returned here.
         * @param free      If specified, set true to indicate that the destructor 
         *                  for this object would have invoked DCF(SdsFreeId) on the
         *                  underlying SDS ID.  If outlives is true, this won't 
         *                  now happen.
         * @param del      If specified, set true to indicate that the destructor 
         *                  for this object would have invoked DCF(SdsDelete) on the
         *                  underlying SDS ID.  If outlives is true, this won't 
         *                  now happen.
         * @param readfree If specified, set true to indicate that the destructor 
         *                  for this object would have invoked DCF(SdsReadFree) on the
         *                  underlying SDS ID.  If outlives is true, this won't 
         *                  now happen.
         *
         * @return The underlying SDS ID value.
         */
	void COut(const bool outlives, SdsIdType *item, bool * const free = 0,
		bool * const del= 0, bool * const readfree = 0) {
	    *item = COut(outlives,free,del,readfree);
	}


        /** Find an SDS object and make available in the current SdsId object.
         *
         * This method will first clean up the SdsId object, as if the destructor
         * were run.  It then finds a new SDS object within the object
         * represented by "source" and makes that available in the object.
         *
         * The purpose of this method is for cases where an SdsId object must
         * be set up in a different spot from where the variable is declared.
         *
         * This method is meant to replace sequences where previously one would
         * have needed to create a new object, and then use ShallowCopy() to
         * copy its contents to another object.
         *
         * If the find fails or if status is non-zero on entry, then the
         * object will have been cleaned up and will refer to a null SDS 
         * object.
         *
         * @see SdsId::ShallowCopy
         * @sds DCF(SdsFind)
         *
         * @param source     The object containing the object of interest
         * @param name       The name of the SDS object of interest
         * @param status    Inherited status.
         *
         */
    	void Find(const SdsId& source, 
                  const char * const name, 
                  StatusType * const status) {
            CleanUp("SdsId::Find");	/* Run destructor on old id */

    	    flags.free = true;
    	    flags.del = false;
    	    flags.readfree = false;
     	    SdsFind(source.id,name,&id,status);
    	}

	/** Shallow copy from SdsId.  
         *
         * Copy another SdsId item into this object.  The existing SDS ID
         * refered to by this object is first tidied up correctly.  
         * After the copy, both the source
         * and this object refer to the same SDS structure.  The outlives
         * parameter must be set correctly to determine which one destroys the
         * underlying SDS structure.  
         *
         * Note that if outlives is true, then write access is required to
         * source - hence it cannot be const.
         *
         * @see SdsId::DeepCopy
         *
         * @param source The Sds object to copy.
         * @param outlives If true (default) this object is to outlive the source object.  
         */
	void ShallowCopy (SdsId & source, const bool outlives=true) {
	    if (this != &source)
	    {
	        CleanUp("SdsId::ShallowCopy 1");	/* Run destructor on old id */
	        id = source.id;
	        if (outlives)
	        {
                    /* If we will outlive the source, then we copy the source's 
                     * flags and set  the sources flags to false. 	
                     */

	    	    flags.free = source.flags.free;
	     	    flags.del  = source.flags.del;
	    	    flags.readfree  = source.flags.readfree;

	    	    source.flags.free = source.flags.del = 
	    	    			source.flags.readfree = false;
	        }
	        else
	        {
	    	    flags.free = flags.del = flags.readfree = false;
	        }
	    }
	}
	/** Replace the item refered to by this object by a deep copy of source
         *
         * The DCF(SdsCopy) function is used to create a copy.  
         *
         * @remarks If source is this item, then  DCF(SdsCopy) is only invoked
         *  if this is an external SDS item.  The result of such an operation
         *  is that "this" will always refer to something you can modify the
         *  structure of, after a call to DeepCopy().
         *
         * @param source The item to create a copy of.
         * @param status Inherited status.
         */
	void DeepCopy (const SdsId &source, StatusType * const status)
	{
	    if (this != &source)
	    {
	        CleanUp("SdsId::DeepCopy 1");	/* Run destructor code on old id */
	        id = 0;
	        SdsCopy(source.id,&id,status);
	        flags.free = true;
	        flags.del = true;
	        flags.readfree = false;
	    }
            else
            {
                /*
                 * this = &source;
                 *
                 * Deep copy of self.  If this item is external, then
                 * we actual do a copy - but we have to get the ordering
                 * right.  This ensures that DeepCopy() always gives us
                 * something we can modify the structure of.
                 */
                int external;
                SdsIsExternal(id, &external, status);
                if (external)
                {
                    /*
                     * copy this to a new SDS id..
                     */
                    SdsIdType newId;
                    SdsCopy(id,&newId,status);
                    
                    if (*status == STATUS__OK)
                    {
                        /*
                         * Run the contructor on "this" and then set up 
                         * this again.
                         */
                        CleanUp("SdsId::DeepCopy 2");
                        id = newId;
                        flags.free = true;
                        flags.del = true;
                        flags.readfree = false;
                        
                    }
                    
                }
            }
	}


        /** Shallow copy from SdsIdType.  
         *
         * Copy an SdsIdType item into this object.  The existing SDS ID
         * refered to by this object is first tidied up correctly.  
         * After the copy, both the source
         * and this object refer to the same SDS structure.  The caller must indicate
         * who has responsibility for cleaning up by setting appropiate flags.
         *
         *
         * @see SdsId::DeepCopy
         *
         * @param source   The Sds object to copy.
         * @param free     If set true, this object will free the SDS id when
         *                 it is destroyed.
         * @param del      If set true, this object will delete the underlying
         *                 SDS object when it is destroyed.
         * @param readfree If set true, this object will call DCF(SdsReadFree) when
         *                 the object is destroyed.
         */
	void ShallowCopy (const SdsIdType source, const bool free=false, 
    			const bool del = false, const bool readfree = false)
    	{
	    CleanUp("SdsId::ShallowCopy 2");	/* Run destructor on old id */
    	    flags.free = free;
    	    flags.del = del; 
    	    flags.readfree = readfree;
	    id = source;
	}
	/** Replace the item refered to by this object by a deep copy of source
         *
         * The DCF(SdsCopy) function is used to create a copy.
         *
         * @param source The item to create a copy of.
         * @param status Inherited status.
         */
	void DeepCopy (const SdsIdType source, StatusType *status)
	{
	    CleanUp("SdsId::DeepCopy 3");	/* Run destructor code on old id */
	    id = 0;
  	    SdsCopy(source,&id,status);
	    flags.free = true;
	    flags.del = true;
	    flags.readfree = false;
	}
#ifdef DRAMA_ALLOW_CPP_STDLIB
        /** Access the data of a single dimensional SDS array item.
         *
         * Accesses an array using an SdsArrayAccessHelper of the appropiate
         * type.  This version allows access to a one dimensional array of
         * any size.   Various other versions also exist.
         *
         * On return, "data[index]" can be used to access the array element
         * at the specified index.  See the SdsAccess ArrayHelper:: class 
         * for more information.
         *
         * This version uses the number of elements in the SDS array.
         *
         * In this example, SdsArrayShort is a sub-class of 
         * SdsArrayAccessHelper.
         * <pre>
         *    SdsId structure;
         *    ...
         *    SdsArrayShort value;
         *    structure.ArrayAccess(&value, status);
         *    unsigned long count = value.Size();
         *    if (*status == STATUS__OK)
         *    {
         *       for (unsigned long i = 0; i < count ; ++i)
         *         printf("Element %ld = %d\\n", i, (int)value[i]);
         *    }
         * </pre>
         *
         * @remarks Only available if the marcro DRAMA_ALLOW_CPP_STDLIB is defined.
         * @see SdsArrayAccessHelper
         *
         * @param data A pointer to an object which allows you to access the
         *        underlying SDS data.  It must be a sub-class of 
         *        SdsArrayAccessHelper appropiate for type T.
         * @param status  Inherited status.  In addition to the values
         *        returned by DCF(SdsInfo) and DCF(SdsPointer), the values
         *        SDS__TYPE, SDS__NOTARR and SDS__INVDIMS may be returned.
         */
        template <typename T> void ArrayAccess(
            SdsArrayAccessHelper<T> * const data,
            StatusType *const status) {
            
            unsigned long dims[SDS_C_MAXARRAYDIMS];
            CheckArrayTypeSingleDim(data->Code(), dims, status);
            if (*status != STATUS__OK) return;
            data->SetElements(dims[0]);
            
            Pointer(data->Data(), status); 
        }
        /** Access the data of a single dimensional SDS array
         *
         * Accesses an array using an SdsArrayAccessHelper of the appropiate
         * type.  This version allows access to a one dimensional array of
         * any size.   Various other versions also exist.
         *
         * On return, "data[index]" can be used to access the array element
         * at the specified index.
         *
         * In this version uses the number of elements in the SDS array must
         * match the specified number of elements, otherwise status is set
         * to SDS__INVDIMS.
         *
         * @remarks Only available if the marcro DRAMA_ALLOW_CPP_STDLIB 
         * is defined.
         *
         * In this example, SdsArrayShort is a sub-class of 
         * SdsArrayAccessHelper.
         * <pre>
         *    SdsId structure;
         *    ...
         *    SdsArrayShort value;
         *    structure.ArrayAccess(10, &value, status);
         *    if (*status == STATUS__OK)
         *    {
         *       for (unsigned long i = 0; i < 10 ; ++i)
         *         printf("Element %ld = %d\\n", i, (int)value[i]);
         *    }
         * </pre>
         *         *
         * @see SdsArrayAccessHelper
         *
         * @param expitems The number of items expected in the array.
         * @param data A pointer to an object which allows you to access the
         *        underlying SDS data.  It must be a sub-class of 
         *        SdsArrayAccessHelper appropiate for type T.
         * @param status  Inherited status.  In addition to the values
         *        returned by DCF(SdsInfo) and DCF(SdsPointer), the values
         *        SDS__TYPE, SDS__NOTARR and SDS__INVDIMS may be returned.
         */
        template <typename T> void ArrayAccess(
            const unsigned long expitems, /* Expected number of elements in array*/
            SdsArrayAccessHelper<T> * const data,
            StatusType *const status) {
            
            unsigned long dims[SDS_C_MAXARRAYDIMS];
            CheckArrayTypeSingleDim(data->Code(), dims, status);
            if (*status != STATUS__OK) return;
            if (dims[0] != expitems)
            {
                *status = SDS__INVDIMS;
                return;
            }
            data->SetElements(dims[0]);
            Pointer(data->Data(), status); 
        }
        /** Access the data of an SDS array
         *
         * Accesses an array using an SdsArrayAccessHelper of the appropiate
         * type.  This version allows access to an array of a specified
         * number of dimensions.   Various other versions also exist.
         *
         * On return, "data[index]" can be used to access the array element
         * at the specified index.  See the SdsArrayAccessHelper:: class 
         * for more information.  Note that Multidimensional arrays must 
         * be indexed by calculating the equivalent single dimensional index 
         * (otherwise the type definitions are too hard).
         *
         * In this example, SdsArrayShort is a sub-class of 
         * SdsArrayAccessHelper.
         * <pre>
         *    SdsId structure;
         *    ...
         *    unsigned long dims[SDS_C_MAXARRAYDIMS];
         *    SdsArrayShort value;
         *    structure.ArrayAccess(&value, 1, dims, status);
         *    if (*status == STATUS__OK)
         *    {
         *       for (unsigned long i = 0; i < dims[0] ; ++i)
         *         printf("Element %ld = %d\\n", i, (int)value[i]);
         *    }
         * </pre>
         *
         * @remarks Only available if the marcro DRAMA_ALLOW_CPP_STDLIB is 
         *          defined.
         * @see SdsArrayAccessHelper
         *
         * @param data A pointer to an object which allows you to access the
         *        underlying SDS data.  It must be a sub-class of 
         *        SdsArrayAccessHelper appropiate for type T.
         * @param ndims The number of dimensions there should be in the 
         *               structure.
         * @param dims The dimensions are written to this array. Must have
         *          space for 7 dimensions.
         * @param status  Inherited status.  In addition to the values
         *        returned by DCF(SdsInfo) and DCF(SdsPointer), the values
         *        SDS__TYPE, SDS__NOTARR and SDS__INVDIMS may be returned.
         */
        template <typename T> void ArrayAccess(
            SdsArrayAccessHelper<T> * const data,
            long ndims,
            unsigned long dims[],
            StatusType *const status) {
            
            if ((ndims < 1)||(ndims > 7))
            {
                *status = SDS__INVDIMS;
                return;
            }
            CheckArrayType(data->Code(), ndims, dims, status);
            if (*status != STATUS__OK) return;

            /*
             * work out the number of elements
             */
            unsigned long elements = dims[0];
            for (long i = 1; i < ndims ; ++i)
            {
                elements *= dims[i];
            }
            data->SetElements(elements);
            /*
             * Access the data.
             */
            Pointer(data->Data(), status); 
        }


        /** Access the data of an SDS array
         *
         * Accesses an array using an SdsArrayAccessHelper of the appropiate
         * type.  This version allows access to an array of any
         * dimensions and number of dimensions.  Various other versions 
         * also exist.
         *
         * On return, "data[index]" can be used to access the array element
         * at the specified index.  See the SdsArrayAccessHelper class 
         * for more information.  Note that mult-idimensional arrays must be 
         * indexed by calculating the single dimensional index (otherwise 
         * the type definitions are too hard).
         *
         * In this example, SdsArrayShort is a sub-class of 
         * SdsArrayAccessHelper.
         * <pre>
         *    SdsId structure;
         *    ...
         *    long ndims;
         *    unsigned long dims[SDS_C_MAXARRAYDIMS];
         *    SdsArrayShort value;
         *    structure.ArrayAccess(&value, &ndims, dims, status);
         *    unsigned long count = value.Size();
         *    if (*status == STATUS__OK)
         *    {
         *       for (unsigned long i = 0; i < count ; ++i)
         *         printf("Element %ld = %d\\n", i, (int)value[i]);
         *    }
         * </pre>
         *
         * @remarks Only available if the marcro DRAMA_ALLOW_CPP_STDLIB is 
         *          defined.
         *
         * @see SdsArrayAccessHelper
         *
         * @param data A pointer to an object which allows you to access the
         *        underlying SDS data.  It must be a sub-class of 
         *        SdsArrayAccessHelper appropiate for type T.
         * @param ndims The number of dimensions is written here.
         * @param dims The dimensions are written to this array. Must have
         *          space for 7 dimensions.
         * @param status  Inherited status.  In addition to the values
         *        returned by DCF(SdsInfo) and DCF(SdsPointer), the values
         *        SDS__TYPE, SDS__NOTARR and SDS__INVDIMS may be returned.
         */
        template <typename T> void ArrayAccess(
            SdsArrayAccessHelper<T> * const data,
            long *ndims,
            unsigned long dims[],
            StatusType *const status) {
            
            /*
             * A bit of a pain, we need to call Dims to get the number
             * of dimensions.
             */
            Dims(ndims, dims, status);
            /*
             * The use the above version where we know the number of dimenions
             */
            ArrayAccess(data, *ndims, dims, status);
        }



#endif /* defined DRAMA_ALLOW_CPP_STDLIB */


}; /* class SdsId */

SDSEXTERN const SdsId SdsNull;

#ifdef DRAMA_ALLOW_CPP_STDLIB
#include <stdio.h>
/** A class to check for SDS leaks
 *
 * Create one of these objects in your and when the destructor is run, it will
 * write a message to stderr if your code has allocated or released a net number of
 * SDS ID's since the contructor was run.
 *
 * The easiest way to use this via the SDS_CHECK_IDS macro.
 */
class  SdsIDChecker {
private:
    SdsCheckType _chkData;
    int _line;
    std::string _file;
    std::string _function;
public:
    /** Construct an SdsChecker object.
     *
     * When constructed, this object will run SdsCheckInit() and save the results.  When
     * the destructor is run, it will run SdsCheck() and write a mesage to stderr if
     * there was a net change in SDS id's which are allocated since the 
     *
     * @remarks Use the SDS_CHECK_IDS marco to invoke this (if only used once in a block.
     *
     * @remarks Only available if the marcro DRAMA_ALLOW_CPP_STDLIB is 
     *          defined.
     *
     * @param line  The line number the object is constructed as.  Assign to __LINE__.
     * @param file  The compilation file the object is constructed in. Assign to __FILE__.
     * @param function A function/method name to be used in the message.
     *
     */
    SdsIDChecker(int line, const char *file, const char *function) 
        : _line(line), _file(file), _function(function) {
        StatusType ignore = STATUS__OK;
        SdsCheckInit(&_chkData, &ignore);
    }
    /** SdsCheck destructor.
     * 
     * Will invoke SdsCheck() and output a message to stderr if the net number of SDS ID's
     * outstarnding have changed since the constructor was run.
     */
    virtual ~SdsIDChecker() {
        StatusType status = STATUS__OK;
        SdsCheck(0, &_chkData, &status);
        if (status == SDS__CHK_LEAK)
            fprintf(stderr,"Warning:SdsIDChecker:Function leaked SDS id's - function %s, file %s, check object at line %d\n", _function.c_str(), _file.c_str(), _line);
        else if (status == SDS__CHK_RELEASED)
            fprintf(stderr,"Warning:SdsIDChecker:Function released SDS id's - function %s, file %s, check object at line %d\n", _function.c_str(), _file.c_str(), _line);
    }
};
/** Check for SDS ID releases or leaks.
 *
 * This marco declears an object named _sds_id_checker of type SdsIDChecker.  It uses the
 * current line number and compilation file name when constructing the object.    This is
 * the easiest way of checking for SDS leaks in a C++ function.
 *
 * @remarks Only available if the marcro DRAMA_ALLOW_CPP_STDLIB is defined.
 *
 * @param _function_   A function/method name to be used in the message.  char *.
 *
 */
#define SDS_CHECK_IDS(_function_) SdsIDChecker _sds_id_checker(__LINE__, __FILE__, (_function_))
#endif /* defined DRAMA_ALLOW_CPP_STDLIB */

 
#endif

/* Define SdsMalloc and SdsFree.  These are normally just defined
   to malloc and free but sometimes when debugging we want different
   versions
 */
 
#define SdsMalloc(_size) (void *)malloc(_size)
#define SdsFree(_where)   (void)free((void *)(_where))

#endif


