#ifndef ARGINC
#define ARGINC
/*
 *  @(#) $Id: ACMM:sds/arg.h,v 3.94 09-Dec-2020 17:15:24+11 ks $
 */
/*
 * This file now uses DOXYGEN comments to generate the C++ web pages.
 * m4 macros of the form DCF(function) are used in the comments
 * to refer to DRAMA C function documention (see DramaHtml/Makefile,
 * DramaHtml/doxygen.config and DramaHtml/filter.m4 for detials)
 */

#ifdef __cplusplus

/*
 * If under C++ and this is allowed, then we can add functions which
 * require the C++ Standard Library (say under GCC 2.95.2 or later).
 */
#ifdef DRAMA_ALLOW_CPP_STDLIB
#include <string>
#endif

extern "C" {

#endif
#include "sds.h"
#include "arg_err.h"
#include "status.h"

#define ARG_SDS -1
#define ARG_STRING -2
#define ARG_STRING2 -3

/*
 *   Is const acceptable on this machine.
 */
#if defined(__cplusplus) || defined(__STDC__) || defined (VAXC)
#     define ARGCONST const
#else
#     define ARGCONST  /* */
#endif


/*
 * Under Windows, we may need to export or import the functions, depending
 * on if we are building the DLL or linking against it.
 */
#ifdef WIN32
#ifdef DRAMA_DLL  /* Am building DRAMA DLL */
#define ARGEXTERN __declspec(dllexport)
#elif defined(DRAMA_STATIC) /* Building a static library */
#define ARGEXTERN extern
#else           /* Am building application with DLL */
#define ARGEXTERN __declspec(dllimport)
#endif

#else
#define ARGEXTERN extern
#endif

/*
 * If defined, we are using the newer C++ overloads.
 */
#define ARG_NEWCPP_OVERLOADS 1


ARGEXTERN void ArgNew(SdsIdType *id, StatusType * ARGCONST status);
ARGEXTERN void ArgPutc(SdsIdType id, ARGCONST char *name, char value, 
			   StatusType * ARGCONST status);
ARGEXTERN void ArgPuts(SdsIdType id, ARGCONST char *name, short value,
			   StatusType * ARGCONST status);
ARGEXTERN void ArgPutus(SdsIdType id, ARGCONST char *name, unsigned short value,
			   StatusType * ARGCONST status);
ARGEXTERN void ArgPuti(SdsIdType id, ARGCONST char *name, long value, 
			   StatusType * ARGCONST status);
ARGEXTERN void ArgPuti64(SdsIdType id, ARGCONST char *name, INT64 value, 
			   StatusType * ARGCONST status);

ARGEXTERN void ArgPutu(SdsIdType id, ARGCONST char *name, unsigned long value, 
			   StatusType * ARGCONST status);

ARGEXTERN void ArgPutu64(SdsIdType id, ARGCONST char *name, UINT64 value,
			   StatusType * ARGCONST status);

ARGEXTERN void ArgPutf(SdsIdType id, ARGCONST char *name, float value,
			   StatusType * ARGCONST status);
ARGEXTERN void ArgPutd(SdsIdType id, ARGCONST char *name, double value,
			   StatusType * ARGCONST status);
ARGEXTERN void ArgPutString(SdsIdType id, ARGCONST char *name, ARGCONST char *value,
			   StatusType * ARGCONST status);
ARGEXTERN void ArgGetc(SdsIdType id, ARGCONST char *name, char *value,
			   StatusType * ARGCONST status);
ARGEXTERN void ArgGets(SdsIdType id, ARGCONST char *name, short *value,
			   StatusType * ARGCONST status);
ARGEXTERN void ArgGetus(SdsIdType id, ARGCONST char *name, unsigned short *value,
			   StatusType * ARGCONST status);
ARGEXTERN void ArgGeti(SdsIdType id, ARGCONST char *name, long *value,
			   StatusType * ARGCONST status);
ARGEXTERN void ArgGeti64(SdsIdType id, ARGCONST char *name, INT64 *value,
			   StatusType * ARGCONST status);

ARGEXTERN void ArgGetu(SdsIdType id, ARGCONST char *name, unsigned long *value,
			   StatusType * ARGCONST status);
ARGEXTERN void ArgGetu64(SdsIdType id, ARGCONST char *name, UINT64 *value,
			    StatusType * ARGCONST status);

ARGEXTERN void ArgGetf(SdsIdType id, ARGCONST char *name, float *value, 
			   StatusType * ARGCONST status);
ARGEXTERN void ArgGetd(SdsIdType id, ARGCONST char *name, double *value, 
			  StatusType * ARGCONST status);
ARGEXTERN void ArgGetString(SdsIdType id, ARGCONST char *name, long len, char *value, 
			  StatusType * ARGCONST status);
ARGEXTERN void ArgDelete(SdsIdType id, StatusType * ARGCONST status);


ARGEXTERN void ArgCvt (ARGCONST void * SrcAddr, SdsCodeType SrcType,  SdsCodeType DstType,
	     void * DstAddr, unsigned long int DstLen , 
			  StatusType  * ARGCONST status);
ARGEXTERN void ArgLook (char *SrcAddr, int USFlag, int MinFlag, SdsCodeType *DstType, 
			  StatusType * ARGCONST status);


ARGEXTERN void ArgToString (SdsIdType id,int maxlen,int *length,char *string,
			  StatusType * ARGCONST status);

typedef void (*ArgListFuncType)(
			void *client_data,
			ARGCONST char *line,			
                        StatusType * status);
ARGEXTERN void ArgSdsList(SdsIdType id, unsigned buflen, char *buffer, 
		 ArgListFuncType func, void * client_data,
		 StatusType * ARGCONST status);


ARGEXTERN void ArgFind(SdsIdType parent_id, ARGCONST char *name, 
                       SdsIdType *id, 
                       StatusType * ARGCONST status);

ARGEXTERN void ArgCheckItem(SdsIdType id, 
                            SdsCodeType code, long ndims, 
                            long * ARGCONST dims, 
                            StatusType * ARGCONST status);


/*
 * Methods for creating/setting array items.  We don't have the complete
 * set as yet.
 */
ARGEXTERN void ArgPut1DArrayD(SdsIdType id,
                              const char *name,
                              unsigned long dim,
                              const double data[],
                              StatusType * status);


ARGEXTERN void ArgPut1DArrayDElem(SdsIdType id,
                                  const char *name,
                                  unsigned long dim,
                                  unsigned int index,
                                  double data,
                                  StatusType * const status);
                              

ARGEXTERN void ArgPut1DArrayI(SdsIdType id,
                              const char *name,
                              unsigned long dim,
                              const INT32 data[],
                              StatusType * status);


ARGEXTERN void ArgPut1DArrayIElem(SdsIdType id,
                                  const char *name,
                                  unsigned long dim,
                                  unsigned int index,
                                  INT32 data,
                                  StatusType * const status);
                              
ARGEXTERN void ArgPut1DArrayB(SdsIdType id,
                              const char *name,
                              unsigned long dim,
                              const unsigned char data[],
                              StatusType * status);


ARGEXTERN void ArgPut1DArrayBElem(SdsIdType id,
                                  const char *name,
                                  unsigned long dim,
                                  unsigned int index,
                                  unsigned char data,
                                  StatusType * const status);



ARGEXTERN void ArgPut1DArrayS(SdsIdType id,
                              const char *name,
                              unsigned long slen,
                              unsigned long dim,
                              const char *data[],
                              StatusType * status);


ARGEXTERN void ArgPut1DArraySElem(SdsIdType id,
                                  const char *name,
                                  unsigned long slen,
                                  unsigned long dim,
                                  unsigned int index,
                                  const char *data,
                                  StatusType * const status);


#ifdef __cplusplus
} /* extern "C" */


/*
 * C++ only section.
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 *  Here is the a C++ interface to ARG.  No object modifications required since
 *  all code is inline.
 */
/**
 * A C++ Interface to the SDS Arg library.
 *
 * Arg provides simpler interfaces to constructing, putting and fetching data 
 * from simple SDS scaler or string items.
 *
 * Note that if the macro DRAMA_ALLOW_CPP_STDLIB is defined, then
 * it is presume that the C++ standard library is available and
 * relevant methods (such as those which use std::string types) are
 * compiled.  Otherwise they are not defined.
 *
 * @see SdsId
 * @see <A href="http://www.aao.gov.au/drama/doc/ps/sds.ps"  TARGET="_top">SDS Specification</A>
 * @see <A href="http://www.aao.gov.au/drama/doc/ps/drama_cpp.ps"  TARGET="_top">DRAMA C++ interface</A>
 * @see <A href="http://www.aao.gov.au/drama/doc/ps/dits_spec_5.ps"  TARGET="_top">DRAMA DITS Specification</A>
 *   
 */
class Arg : public SdsId {
    private:
        /* Assignment and Copy operators, made private to avoid misuse, but
           this is done in SdsId so would have been prohibited anyway
         */
        Arg& operator=(const Arg&); /* Assignment */
        Arg(const Arg&); /* Copy */
     public:
        /** Arg constructor to access an item created by C language SDS.
         *
         * This Arg constructor that takes an existing SDS id
         *      (C style) and creates an SdsId class object which referees
         *      to it.  You must explictly specify if the item is to be
         *      freed and deleted when the object's destructor is invoked, and
         *      what you do depends on when what other code may do with
         *      item.  del and readfree are mutually exclusive.  
         *
         * @param item The SdsIdType of the item.  If 0 (default), a SdsId 
         *             object which does not refer to any SDS item is created.
         *             This is normally only used when we want an argument
         *             to pass (by pointer) to a function which will use
         *             ShallowCopy() or DeepCopy() to set the ID or if you wish
         *             to use the New() method to create a new method later.
         * @param free Invoke DCF(SdsFreeId) on the ID when the object is  destroyed.
         *             Default is false.
         * @param del  Invoke DCF(SdsDelete) on the ID when the object is
         *              destroyed.  The default is false.
         * @param readfree Invoke DCF(SdsReadFree) on the ID when the object
         *              is destroyed.  The default is false.
         * 
    	 */        
	Arg (const SdsIdType item=0, const bool free=false, 
    	     const bool del = false, const bool readfree = false) : 
    	     		SdsId(item,free,del,readfree) {
	};
	/** Arg constructor which creates a new SDS structure.
         *
         * This Arg contructor creates a new top level SDS item.
         *
         * The first argument is just used to make this constructors argument 
         * list unique in all cases.
         *
         * @see DCF(ArgNew)
         * 
         *  @param flag   Ignored, but used to distingish this constructor
         *                 from other contructors.
         *  @param status Inherited status.
         *  @param name   The name for the structure.  The default is what
         *                is normally supplied by the DCF(ArgNew) function, but
         *                any name acceptable to SDS can be specified.
	 */
	Arg (bool /*flag*/, StatusType * const status, 
			const char * const name = "ArgStructure") : 
	    SdsId(name, SDS_STRUCT,status) {
	}
	
        /** Create a new Arg structure within this object.
         *
         * This method destorys the existing SDS object referred to by
         * this object and creates a new one.
         *
         * @see DCF(ArgNew)
         * 
         *  @param status Inherited status.
         *  @param name   The name for the structure.  The default is what
         *                is normally supplied by the DCF(ArgNew) function, but
         *                any name acceptable to SDS can be specified.
         */
	void New (StatusType * const status,
			const char * const name = "ArgStructure") {
            /*
             * We create the new item and then us SdsId::ShallowCopy to
             * copy it into this object.
             */
	    Arg tid(true,status, name);
	    ShallowCopy(tid,true);
	};

        /** Constructor which returns an id to a named item of another item 
         *
         * This constructor searches an existing SDS item for a item with
         * a specified name and creates a reference to it.  
         *
         * @see DCF(SdsFind)
         * @see DCF(ArgFind)
         *
         * @param source The SDS item to find name within.
         * @param name   The name of the SDS item to find.
         * @param status    Inherited status.
         *
         *  
         */
        Arg(const SdsId& source, const char * const name,
                StatusType * const status) :
                SdsId(0,false,false,false) {
            /*
             * This is confusing since what we really want is to do is SdsFind 
             * but we want the ErsRep reporting facilities of ArgFind, so we   
             * must create this item as a null item, call ArgFind using the    
             * arguments and then use ShallowCopy to copy the SdsIdType to this 
             * item 
             */
            SdsIdType tid=0;
            ArgFind(source,name,&tid,status);
            ShallowCopy(tid,true,false,false);
        }

        /** Convert the structure to a string
         *
         * This method attempts to convert the SDS structure maintained
         * by this Arg object into a string representation.  
         *
         * @see DCF(ArgToString)
         * @see Arg::List
         * 
         * @param maxlen  The maximum amount of space in the output string.
         * @param length  The length of the output string is returned here.
         * @param string  The buffer to write the structure too.
         * @param status    Inherited status.
         *
         */
	void ToString(int maxlen, int *length, char *string, 
				StatusType * const status)
	{
	    ArgToString(*this,maxlen,length,string,status);
	}
        /** Check an item has a required structure
         *
         * A common requirement for the user of the SDS library is to check
         * that a specified SDS item has the right format - type code,
         * number of dimensions and dimensions.  This utility function
         * implements such a check.
         *
         * @see DCF(ArgCheckItem)
         *
         * @param code    SDS Code we expect. The normal SDS code values,
         *                or ARG_STRING, in which case we mean SDS_CHAR but
         *                the ndims value is bumped by 1 automatically.  
         *                If specified as -1, then the code is ignored.
         * @param status  Inherited status.
         * @param ndims   The expected number of dimensions.  0 to 
         *                SDS_C_MAXARRAYDIMS (7) or a maximum one less then 
         *                this if code=ARG_STIRNG.
         *                If specified as -1, then don't check ndims.
         * @param dims    The expected dimensions.  Only used
         *                if ndims>0 or code=ARG_STRING.  Only ndims
         *                values are used (or ndims+1 if code=ARG_STRING).  
         *                If the value is negative or zero, the value is 
         *                ignored,  otherwise they must match.
         *                Note - long, not the unsigned long that
         *                SDS dimension arrays noramlly are.
         *                A null pointer can be supplied, which
         *                is equivalent of an array of -1 values.
         */
        void CheckItem(SdsCodeType code,
                       StatusType *status,
                       long ndims=-1, 
                       long * dims=0) 
        {
            ArgCheckItem(*this, code, ndims, dims, status);
        }
        

#       ifndef CPP_NOBOOL
        /** Insert a boolean value into a named component of the Arg structure.
         *
         * The named item is created in the current structure if it does
         * not already exist (of SDS type SDS_USHORT) and the specified
         * value inserted (after conversion to unsigned short).  If the name 
         * item already exists, then the value is converted to that type (if
         * possible) and inserted.
         *
         * Note that SDS does not directly support boolean values, so an
         * integer representation is used.
         *
         * @see DCF(ArgPutus)
         *
         * @param name The name of the item within the SDS structure.
         * @param value The value to be inserted.
         * @param status    Inherited status.
         *
         */
        void Put (const char *name, bool value, StatusType * const status) {
            unsigned short v = (int)value;
	    ArgPutus(*this,name,v,status);
	};
#	endif
        /** Insert a char value into a named component of the Arg structure.
         *
         * The named item is created in the current structure if it does
         * not already exist (of SDS type SDS_CHAR) and the specified
         * value inserted.  If the name item already exists, then the value is 
         * converted to that type (if possible) and inserted.
         *
         * @see DCF(ArgPutc)
         *
         * @param name The name of the item within the SDS structure.
         * @param value The value to be inserted.
         * @param status    Inherited status.
         *
         */
        void Put (const char *name, char value, StatusType * const status) {
	    ArgPutc(*this,name,value,status);
	};
        /** Insert a short value into a named component of the Arg structure.
         *
         * The named item is created in the current structure if it does
         * not already exist (of SDS type SDS_SHORT) and the specified
         * value inserted.  If the name item already exists, then the value is 
         * converted to that type (if possible) and inserted.
         *
         * @see DCF(ArgPuts)
         *
         * @param name The name of the item within the SDS structure.
         * @param value The value to be inserted.
         * @param status    Inherited status.
         *
         */
        void Put (const char *name, short value, StatusType * const status) {
	    ArgPuts(*this,name,value,status);
	};
        /** Insert an unsigned  short value into a named component of the Arg 
         *  structure.
         *
         * The named item is created in the current structure if it does
         * not already exist (of SDS type SDS_USHORT) and the specified
         * value inserted.  If the name item already exists, then the value is 
         * converted to that type (if possible) and inserted.
         *
         * @see DCF(ArgPutus)
         *
         * @param name The name of the item within the SDS structure.
         * @param value The value to be inserted.
         * @param status    Inherited status.
         *
         */
        void Put (const char *name, unsigned short value, 
					   StatusType * const status) {
	    ArgPutus(*this,name,value,status);
	};

        /** Insert a 32 bit integer value into a named component of the Arg 
         *  structure.
         *
         * The named item is created in the current structure if it does
         * not already exist (of SDS type SDS_INT) and the specified
         * value inserted.  If the name item already exists, then the value is 
         * converted to that type (if possible) and inserted.
         *
         * @see DCF(ArgPuti)
         *
         * @param name The name of the item within the SDS structure.
         * @param value The value to be inserted.
         * @param status    Inherited status.
         *
         */
        void Put (const char *name, INT32 value, StatusType * const status) {
	    ArgPuti(*this,name,value,status);
	};

        /** Insert an unsigned 32 bit integer into a named component of the 
         *  Arg structure.
         *
         * The named item is created in the current structure if it does
         * not already exist (of SDS type SDS_UINT) and the specified
         * value inserted.  If the name item already exists, then the value is 
         * converted to that type (if possible) and inserted.
         *
         * @see DCF(ArgPutu)
         *
         * @param name The name of the item within the SDS structure.
         * @param value The value to be inserted.
         * @param status    Inherited status.
         *
         */
        void Put (const char *name, UINT32 value, StatusType * const status) {
	    ArgPutu(*this,name,value,status);
	};
        /** Insert a 64 bit integer value into a named component of the Arg 
         * structure.
         *
         * The named item is created in the current structure if it does
         * not already exist (of SDS type SDS_INT64) and the specified
         * value inserted.  If the name item already exists, then the value is 
         * converted to that type (if possible) and inserted.
         *
         * @see DCF(ArgPuti64)
         *
         * @param name The name of the item within the SDS structure.
         * @param value The value to be inserted.
         * @param status    Inherited status.
         *
         */
        void Put (const char *name, INT64 value, StatusType * const status) {
	    ArgPuti64(*this,name,value,status);
	};
        /** Insert an unsigned 64 bit integer value into a named component 
         *  of the  Arg structure.
         *
         * The named item is created in the current structure if it does
         * not already exist (of SDS type SDS_UINT64) and the specified
         * value inserted.  If the name item already exists, then the value is 
         * converted to that type (if possible) and inserted.
         *
         * @see DCF(ArgPutui64)
         *
         * @param name The name of the item within the SDS structure.
         * @param value The value to be inserted.
         * @param status    Inherited status.
         *
         */
        void Put (const char *name, UINT64 value, StatusType * const status) {
	    ArgPutu64(*this,name,value,status);
	};
        /** Insert an  float size real value into a named component of the 
         *  Arg structure.
         *
         * The named item is created in the current structure if it does
         * not already exist (of SDS type SDS_FLOAT) and the specified
         * value inserted.  If the name item already exists, then the value is 
         * converted to that type (if possible) and inserted.
         *
         * @see DCF(ArgPutf)
         *
         * @param name The name of the item within the SDS structure.
         * @param value The value to be inserted.
         * @param status    Inherited status.
         *
         */
        void Put (const char *name, float value, StatusType * const status) {
	    ArgPutf(*this,name,value,status);
	};
        /** Insert an double size real value into a named component of the 
         *  Arg structure.
         *
         * The named item is created in the current structure if it does
         * not already exist (of SDS type SDS_DOUBLE) and the specified
         * value inserted.  If the name item already exists, then the value is 
         * converted to that type (if possible) and inserted.
         *
         * @see DCF(ArgPutd)
         *
         * @param name The name of the item within the SDS structure.
         * @param value The value to be inserted.
         * @param status    Inherited status.
         *
         */
        void Put (const char *name, double value, StatusType * const status) {
	    ArgPutd(*this,name,value,status);
	};
        /** Insert a char * string into a named component of the 
         *  Arg structure.
         *
         * The named item is created in the current structure if it does
         * not already exist (array of SDS type SDS_CHAR) and the specified
         * value inserted.  If the name item already exists, then the value is 
         * converted to that type (if possible) and inserted.
         *
         * @see DCF(ArgPutString)
         *
         * @param name The name of the item within the SDS structure.
         * @param value The value to be inserted.  Must be a null terminated string.
         *              Note that the null termination is considered part of the
         *              string to be inserted.
         * @param status    Inherited status.
         *
         */
        void Put (const char *name, const char *value, StatusType * const status) {
	    ArgPutString(*this,name,value,status);
	};


#       ifndef CPP_NOBOOL
        /** Fetch a boolean value from a named component of the Arg structure.
         *
         * If the named item exists within the ARG structure then
         * fetch its value and convert it to a bool, if possible.
         *
         * Note that SDS does not directly support boolean values, so an
         * integer representation is used internally and converted to bool.
         *
         * @see DCF(ArgGetus)
         *
         * @param name The name of the item within the SDS structure.
         * @param value The value returned is written here.
         * @param status    Inherited status.
         *
         */
	void Get (const char *name, bool * value, 
					      StatusType * const status)const {
	    unsigned short v;
	    ArgGetus(*this,name,&v,status);
	    *value =  v ? true : false;
	}
#	endif


       /** Insert an array of double into a named component of the 
         *  Arg structure.
         *
         * The named item is created in the current structure if it does
         * not already exist (array of SDS type SDS_DOUBLE) and the specified
         * value inserted.  
         *
         * This does not convert items like the other Arg routines - if the item 
         * already exists it must be at least as long as required and of the 
         * same type.
         *
         * @see DCF(ArgPut1DArrayD)
         *
         * @param name The name of the item within the SDS structure.
         * @param dim  Number of elements in array.  If array does not
         *                already exist, it will be created with this number
         *                of elements.  If it does exist, it must have at
         *                least this number of elements
         * @param data The data to be inserted. 
         * @param status    Inherited status.
         *
         */
        void Put (const char *name, const long dim, const double data[],
                  StatusType * const status) {
            ArgPut1DArrayD(*this, name, dim, data, status);
	};



       /** Insert an 1D double array element.
         *
         * The named item is created in the current structure if it does
         * not already exist (array of SDS type SDS_DOUBLE) and the specified
         * value inserted. 
         *
         * This does not convert items like the other Arg routines - if the item 
         * already exists it must be at least as long as required and of the 
         * same type.
         *
         * @see DCF(ArgPut1DArrayD)
         *
         * @param name The name of the item within the SDS structure.
         * @param dim  Number of elements in array.  If array does not
         *                already exist, it will be created with this number
         *                of elements.  If it does exist, it must have at
         *                least this number of elements.  If zero, use presume index+1.
         * @param index The index to the item within the SDS structure.
         * @param data The data item to be inserted. 
         * @param status    Inherited status.
         *
         */
        void Put (const char *name, const long dim, const unsigned index,
                  const double data, StatusType * const status) {
            ArgPut1DArrayDElem(*this, name, dim, index, data, status);
	};




       /** Insert an array of INT32 into a named component of the 
         *  Arg structure.
         *
         * The named item is created in the current structure if it does
         * not already exist (array of SDS type SDS_INT) and the specified
         * value inserted.  
         *
         * This does not convert items like the other Arg routines - if the item 
         * already exists it must be at least as long as required and of the 
         * same type.
         *
         * @see DCF(ArgPut1DArrayI)
         *
         * @param name The name of the item within the SDS structure.
         * @param dim  Number of elements in array.  If array does not
         *                already exist, it will be created with this number
         *                of elements.  If it does exist, it must have at
         *                least this number of elements
         * @param data The data to be inserted. 
         * @param status    Inherited status.
         *
         */
        void Put (const char *name, const long dim, const INT32 data[],
                  StatusType * const status) {
            ArgPut1DArrayI(*this, name, dim, data, status);
	};

       /** Insert an 1D INT32 array element.
         *
         * The named item is created in the current structure if it does
         * not already exist (array of SDS type SDS_INT) and the specified
         * value inserted. 
         *
         * This does not convert items like the other Arg routines - if the item 
         * already exists it must be at least as long as required and of the 
         * same type.
         *
         * @see DCF(ArgPut1DArrayIElem)
         *
         * @param name The name of the item within the SDS structure.
         * @param dim  Number of elements in array.  If array does not
         *                already exist, it will be created with this number
         *                of elements.  If it does exist, it must have at
         *                least this number of elements. If zero, use presume index+1.
         * @param index The index to the item within the SDS structure.
         * @param data The data item to be inserted. 
         * @param status    Inherited status.
         *
         */
        void Put (const char *name, const long dim, const unsigned index,
                  const INT32 data, StatusType * const status) {
            ArgPut1DArrayIElem(*this, name, dim, index, data, status);
	};




       /** Insert an array of unsigned char into a named component of the 
         *  Arg structure.
         *
         * The named item is created in the current structure if it does
         * not already exist (array of SDS type SDS_UBYTE) and the specified
         * value inserted.  
         *
         * This does not convert items like the other Arg routines - if the item 
         * already exists it must be at least as long as required and of the 
         * same type.
         *
         * @see DCF(ArgPut1DArrayB)
         *
         * @param name The name of the item within the SDS structure.
         * @param dim  Number of elements in array.  If array does not
         *                already exist, it will be created with this number
         *                of elements.  If it does exist, it must have at
         *                least this number of elements
         * @param data The data to be inserted. 
         * @param status    Inherited status.
         *
         */
        void Put (const char *name, const long dim, const unsigned char data[],
                  StatusType * const status) {
            ArgPut1DArrayB(*this, name, dim, data, status);
	};


       /** Insert an 1D unsigned char array element.
         *
         * The named item is created in the current structure if it does
         * not already exist (array of SDS type SDS_UBYTE) and the specified
         * value inserted. 
         *
         * This does not convert items like the other Arg routines - if the item 
         * already exists it must be at least as long as required and of the 
         * same type.
         *
         * @see DCF(ArgPut1DArrayBElem)
         *
         * @param name The name of the item within the SDS structure.
         * @param dim  Number of elements in array.  If array does not
         *                already exist, it will be created with this number
         *                of elements.  If it does exist, it must have at
         *                least this number of elements. If zero, use presume index+1.
         * @param index The index to the item within the SDS structure.
         * @param data The data item to be inserted. 
         * @param status    Inherited status.
         *
         */
        void Put (const char *name, const long dim, const unsigned index,
                  const unsigned char data, StatusType * const status) {
            ArgPut1DArrayBElem(*this, name, dim, index, data, status);
	};


      /** Insert an array of string into a named component of the 
         *  Arg structure.
         *
         * The named item is created in the current structure if it does
         * not already exist (array of SDS strings) and the specified
         * value inserted.  
         *
         * This does not convert items like the other Arg routines - if the item 
         * already exists it must be at least as long as required and of the 
         * same type.
         *
         * @see DCF(ArgPut1DArrayS)
         *
         * @param name The name of the item within the SDS structure.
         * @param slen Number of items in each string, including null 
         *                terminator.  If array does not
         *                already exist, it will be created with this number
         *                of characters in each string.  If it does exist, 
         *                it must have at least this number of characters
         *                in each string.
         * @param dim  Number of elements in array.  If array does not
         *                already exist, it will be created with this number
         *                of elements.  If it does exist, it must have at
         *                least this number of elements
         * @param data The data to be inserted. 
         * @param status    Inherited status.
         *
         */
        void Put (const char *name,  const long slen, const long dim, const char *data[],
                  StatusType * const status) {
            ArgPut1DArrayS(*this, name, slen, dim, data, status);
	};


       /** Insert an 1D  string array element.
         *
         * The named item is created in the current structure if it does
         * not already exist (array of strings) and the specified
         * value inserted. 
         *
         * This does not convert items like the other Arg routines - if the item 
         * already exists it must be at least as long as required and of the 
         * same type.
         *
         * @see DCF(ArgPut1DArraySElem)
         *
         * @param name The name of the item within the SDS structure.
         * @param slen Number of items in each string, including null 
         *                terminator.  If array does not
         *                already exist, it will be created with this number
         *                of characters in each string.  If it does exist, 
         *                it must have at least this number of characters
         *                in each string.  If zero, use length of data including
         *                null terminator.
         * @param dim  Number of elements in array.  If array does not
         *                already exist, it will be created with this number
         *                of elements.  If it does exist, it must have at
         *                least this number of elements. If zero, use presume index+1.
         * @param index The index to the item within the SDS structure.
         * @param data The data item to be inserted. 
         * @param status    Inherited status.
         *
         */
        void Put (const char *name, const long slen, const long dim, const unsigned index,
                  const char *data, StatusType * const status) {
            ArgPut1DArraySElem(*this, name, slen, dim, index, data, status);
	};


        /** Fetch a char value from a named component of the Arg structure.
         *
         * If the named item exists within the ARG structure then
         * fetch its value and convert it to a char, if possible.
         *
         * @see DCF(ArgGetc)
         *
         * @param name The name of the item within the SDS structure.
         * @param value The value returned is written here.
         * @param status    Inherited status.
         *
         */
	void Get (const char *name, char * value, StatusType * const status)const {
	    ArgGetc(*this,name,value,status);
	}
        /** Fetch a short integer value from a named component of the Arg structure.
         *
         * If the named item exists within the ARG structure then
         * fetch its value and convert it to a short integer, if possible.
         *
         * @see DCF(ArgGets)
         *
         * @param name The name of the item within the SDS structure.
         * @param value The value returned is written here.
         * @param status    Inherited status.
         *
         */
	void Get (const char *name, short * value, StatusType * const status)const {
	    ArgGets(*this,name,value,status);
	}
        /** Fetch an unsigned short integer value from a named component of the 
         * Arg structure.
         *
         * If the named item exists within the ARG structure then
         * fetch its value and convert it to an unsigned short integer, if possible.
         *
         * @see DCF(ArgGetus)
         *
         * @param name The name of the item within the SDS structure.
         * @param value The value returned is written here.
         * @param status    Inherited status.
         *
         */
	void Get (const char *name, unsigned short * value, 
					      StatusType * const status) const {
	    ArgGetus(*this,name,value,status);
	}
        /** Fetch a  32 bit integer value from a named component of the 
         * Arg structure.
         *
         * If the named item exists within the ARG structure then
         * fetch its value and convert it to a long integer, if possible.
         *
         * @see DCF(ArgGeti)
         *
         * @param name The name of the item within the SDS structure.
         * @param value The value returned is written here.
         * @param status    Inherited status.
         *
         */
	void Get (const char *name, INT32 * value, StatusType * const status ) const{
            long v; /* Since ArgGeti uses long */
	    ArgGeti(*this,name,&v,status);
            *value = v;
	}
        /** Fetch an unsigned 32 bit integer value from a named component 
         *  of the Arg structure.
         *
         * If the named item exists within the ARG structure then
         * fetch its value and convert it to an unsigned long integer, if possible.
         *
         * @see DCF(ArgGetu)
         *
         * @param name The name of the item within the SDS structure.
         * @param value The value returned is written here.
         * @param status    Inherited status.
         *
         */
	void Get (const char *name, UINT32 * value, 
                  StatusType * const status)const {
            unsigned long v; /* Since ArgGetu always uses long */
	    ArgGetu(*this,name,&v,status);
            *value = v;
	}
        /** Fetch a  64 bit integer value from a named component of the 
         * Arg structure.
         *
         * If the named item exists within the ARG structure then
         * fetch its value and convert it to a 64 bit integer, if possible.
         *
         * @see DCF(ArgGeti64)
         *
         * @param name The name of the item within the SDS structure.
         * @param value The value returned is written here.
         * @param status    Inherited status.
         *
         */
	void Get (const char *name, INT64 * value, StatusType * const status) const{
	    ArgGeti64(*this,name,value,status);
	}
        /** Fetch a  unsigned 64 bit integer value from a named component of the 
         * Arg structure.
         *
         * If the named item exists within the ARG structure then
         * fetch its value and convert it to a unsigned 64 bit integer, if possible.
         *
         * @see DCF(ArgGetu64)
         *
         * @param name The name of the item within the SDS structure.
         * @param value The value returned is written here.
         * @param status    Inherited status.
         *
         */
	void Get (const char *name, UINT64 * value, StatusType * const status)const{
	    ArgGetu64(*this,name,value,status);
	}

        /** Fetch a float size real value from a named component of the 
         * Arg structure.
         *
         * If the named item exists within the ARG structure then
         * fetch its value and convert it to a float, if possible.
         *
         * @see DCF(ArgGetf)
         *
         * @param name The name of the item within the SDS structure.
         * @param value The value returned is written here.
         * @param status    Inherited status.
         */
	void Get (const char *name, float * value, StatusType * const status) const{
	    ArgGetf(*this,name,value,status);
	}
        /** Fetch a double size real value from a named component of the 
         * Arg structure.
         *
         * If the named item exists within the ARG structure then
         * fetch its value and convert it to a double, if possible.
         *
         * @see DCF(ArgGetd)
         *
         * @param name The name of the item within the SDS structure.
         * @param value The value returned is written here.
         * @param status    Inherited status.
         */
	void Get (const char *name, double * value, StatusType * const status)const {
	    ArgGetd(*this,name,value,status);
	}
        /** Fetch a string value as a char * from a named component of the 
         * Arg structure.
         *
         * If the named item exists within the ARG structure then
         * fetch its value and convert it a char * value, if possible.
         *
         * @see DCF(ArgGetString)
         *
         * @param name The name of the item within the SDS structure.
         * @param len  The length available in value, in bytes.
         * @param value The value returned is written here.
         * @param status    Inherited status.
         */
	void Get (const char *name, long len, char * value,  
                                               StatusType * const status) const{
	    ArgGetString(*this,name,len,value,status);
	}	
        /** List an SDS structure through a print function.
         *
         * This static List function allows an SdsId/Arg structure to
         * be listed with a printing function.  It provides appropiate
         * access to the DCF(ArgSdsList) function
         *
         * @remark Should use a printing object rather then a printing function, 
         *         which would probably improve the format significantly.
         *
         * @see DCF(ArgSdsList)
         *
         * @param id           The SdsId item to be printed.
         * @param buflen       The length of the supplied buffer.
         * @param buffer       The buffer to format into.
         * @param func         The printing function.
         * @param client_data  Passed through to the printing function.
         * @param status    Inherited status.
         */
	static void List(const SdsId & id, unsigned buflen, char *buffer, 
		 ArgListFuncType func, void * client_data,
		 StatusType * ARGCONST status) {
	    ArgSdsList(id,buflen,buffer,func,client_data,status);
	}

/*
 *  
 *   	Arg hids some Sds functions, so we redeclare them here.
 */
        /** List the contents of the structure to standard output.
         *
         * @see DCF(SdsList)
         *
         * @param status    Inherited status.
         */
         void List(StatusType * const status) const {
            SdsList(*this,status);
        }

        /** List the contents of the structure to a file
         *
         * @see DCF(SdsList)
         *
         * @param status    Inherited status.
         */
         void List(FILE *to, StatusType * const status) const {
             SdsListTo(to, *this,status);
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
        void Get(const unsigned long length,
        	 void * const data,
        	 StatusType * const status,
        	 unsigned long *actlen = 0,
        	 const unsigned long offset=0) const {
            SdsId::Get(length,data,status,actlen,offset);
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
        void Put(const unsigned long length,
        	 void * const data,
        	 StatusType * const status,
        	 const unsigned long offset=0)
        {
            SdsId::Put(length,data,status,offset);
        }
         	                                     

#ifdef DRAMA_ALLOW_CPP_STDLIB
        /*
         * If DRAMA is allowed to use the C++ Standard Template library, then
         * we add these functions to arg.   All of these are about using
         * std::string instead of char[], and the only complicatef one
         * is the Get function which gets the value as a string.
         *
         */

        /** Constructor which returns an id to a named item of another item 
         *
         * This constructor searches an existing SDS item for a item with
         * a specified name and creates a reference to it.  
         *
         * @remarks Only available if the marcro DRAMA_ALLOW_CPP_STDLIB is defined.
         *
         * @see DCF(SdsFind)
         * @see DCF(ArgFind)
         *
         * @param source The SDS item to find name within.
         * @param name   The name of the SDS item to find.
         * @param status    Inherited status.
         *
         */
        Arg(const SdsId &source, 
            const std::string &name,
            StatusType *const status) :
                SdsId(0,false,false,false) {
            SdsIdType tid=0;
            ArgFind(source,name.c_str(),&tid,status);
            ShallowCopy(tid,true,false,false);
        }

        /** Insert a boolean value into a named component of the Arg structure.
         *
         * The named item is created in the current structure if it does
         * not already exist (of SDS type SDS_USHORT) and the specified
         * value inserted (after conversion to unsigned short).  If the name 
         * item already exists, then the value is converted to that type (if
         * possible) and inserted.
         *
         * Note that SDS does not directly support boolean values, so an
         * integer representation is used.
         *
         * @remarks Only available if the marcro DRAMA_ALLOW_CPP_STDLIB is defined.
         *
         * @see DCF(ArgPutus)
         *
         * @param name The name of the item within the SDS structure.
         * @param value The value to be inserted.
         * @param status    Inherited status.
         *
         */
        void Put (const std::string &name, bool value, StatusType * const status) {
            unsigned short v = (int)value;
	    ArgPutus(*this,name.c_str(),v,status);
	};
        /** Insert a char value into a named component of the Arg structure.
         *
         * The named item is created in the current structure if it does
         * not already exist (of SDS type SDS_CHAR) and the specified
         * value inserted.  If the name item already exists, then the value is 
         * converted to that type (if possible) and inserted.
         *
         * @remarks Only available if the marcro DRAMA_ALLOW_CPP_STDLIB is defined.
         *
         * @see DCF(ArgPutc)
         *
         * @param name The name of the item within the SDS structure.
         * @param value The value to be inserted.
         * @param status    Inherited status.
         *
         */
        void Put (const std::string &name, char value, StatusType * const status) {
	    ArgPutc(*this,name.c_str(),value,status);
	};
        /** Insert a short value into a named component of the Arg structure.
         *
         * The named item is created in the current structure if it does
         * not already exist (of SDS type SDS_SHORT) and the specified
         * value inserted.  If the name item already exists, then the value is 
         * converted to that type (if possible) and inserted.
         *
         * @remarks Only available if the marcro DRAMA_ALLOW_CPP_STDLIB is defined.
         *
         * @see DCF(ArgPuts)
         *
         * @param name The name of the item within the SDS structure.
         * @param value The value to be inserted.
         * @param status    Inherited status.
         *
         */
        void Put (const std::string &name,short value, StatusType * const status) {
	    ArgPuts(*this,name.c_str(),value,status);
	};
        /** Insert an unsigned  short value into a named component of the Arg 
         *  structure.
         *
         * The named item is created in the current structure if it does
         * not already exist (of SDS type SDS_USHORT) and the specified
         * value inserted.  If the name item already exists, then the value is 
         * converted to that type (if possible) and inserted.
         *
         * @remarks Only available if the marcro DRAMA_ALLOW_CPP_STDLIB is defined.
         *
         * @see DCF(ArgPutus)
         *
         * @param name The name of the item within the SDS structure.
         * @param value The value to be inserted.
         * @param status    Inherited status.
         *
         */
        void Put (const std::string &name, unsigned short value, 
					   StatusType * const status) {
	    ArgPutus(*this,name.c_str(),value,status);
	};
        /** Insert a 32 bit integer value into a named component of the 
         *   Arg structure.
         *
         * The named item is created in the current structure if it does
         * not already exist (of SDS type SDS_INT) and the specified
         * value inserted.  If the name item already exists, then the value is 
         * converted to that type (if possible) and inserted.
         *
         * @remarks Only available if the marcro DRAMA_ALLOW_CPP_STDLIB is defined.
         *
         * @see DCF(ArgPuti)
         *
         * @param name The name of the item within the SDS structure.
         * @param value The value to be inserted.
         * @param status    Inherited status.
         *
         */
        void Put (const std::string &name, INT32 value, 
                  StatusType * const status) {
	    ArgPuti(*this,name.c_str(),value,status);
	};
        /** Insert an unsigned 32 bit integer value into a named component 
         *  of the Arg structure.
         *
         * The named item is created in the current structure if it does
         * not already exist (of SDS type SDS_UINT) and the specified
         * value inserted.  If the name item already exists, then the value is 
         * converted to that type (if possible) and inserted.
         *
         * @remarks Only available if the marcro DRAMA_ALLOW_CPP_STDLIB is defined.
         *
         * @see DCF(ArgPutu)
         *
         * @param name The name of the item within the SDS structure.
         * @param value The value to be inserted.
         * @param status    Inherited status.
         *
         */
        void Put (const std::string &name, UINT32 value, 
                  StatusType * const status) {
	    ArgPutu(*this,name.c_str(),value,status);
	};

        /** Insert a 64 bit integer value into a named component of the Arg 
         * structure.
         *
         * The named item is created in the current structure if it does
         * not already exist (of SDS type SDS_INT64) and the specified
         * value inserted.  If the name item already exists, then the value is 
         * converted to that type (if possible) and inserted.
         *
         * @remarks Only available if the marcro DRAMA_ALLOW_CPP_STDLIB is defined.
         *
         * @see DCF(ArgPuti64)
         *
         * @param name The name of the item within the SDS structure.
         * @param value The value to be inserted.
         * @param status    Inherited status.
         *
         */
        void Put (const std::string &name, INT64 value, StatusType * const status) {
	    ArgPuti64(*this,name.c_str(),value,status);
	};
        /** Insert an unsigned 64 bit integer value into a named component of the 
         *  Arg structure.
         *
         * The named item is created in the current structure if it does
         * not already exist (of SDS type SDS_UINT64) and the specified
         * value inserted.  If the name item already exists, then the value is 
         * converted to that type (if possible) and inserted.
         *
         * @remarks Only available if the marcro DRAMA_ALLOW_CPP_STDLIB is defined.
         *
         * @see DCF(ArgPutu64)
         *
         * @param name The name of the item within the SDS structure.
         * @param value The value to be inserted.
         * @param status    Inherited status.
         *
         */
        void Put (const std::string &name, UINT64 value, StatusType * const status) {
	    ArgPutu64(*this,name.c_str(),value,status);
	};
        /** Insert an  float size real value into a named component of the 
         *  Arg structure.
         *
         * The named item is created in the current structure if it does
         * not already exist (of SDS type SDS_FLOAT) and the specified
         * value inserted.  If the name item already exists, then the value is 
         * converted to that type (if possible) and inserted.
         *
         * @remarks Only available if the marcro DRAMA_ALLOW_CPP_STDLIB is defined.
         *
         * @see DCF(ArgPutf)
         *
         * @param name The name of the item within the SDS structure.
         * @param value The value to be inserted.
         * @param status    Inherited status.
         *
         */
        void Put (const std::string &name, float value, StatusType * const status) {
	    ArgPutf(*this,name.c_str(),value,status);
	};
        /** Insert an double size real value into a named component of the 
         *  Arg structure.
         *
         * The named item is created in the current structure if it does
         * not already exist (of SDS type SDS_DOUBLE) and the specified
         * value inserted.  If the name item already exists, then the value is 
         * converted to that type (if possible) and inserted.
         *
         * @remarks Only available if the marcro DRAMA_ALLOW_CPP_STDLIB is defined.
         *
         * @see DCF(ArgPutd)
         *
         * @param name The name of the item within the SDS structure.
         * @param value The value to be inserted.
         * @param status    Inherited status.
         *
         */
        void Put (const std::string &name, double value, StatusType * const status) {
	    ArgPutd(*this,name.c_str(),value,status);
	};
        /** Insert a std::string into a named component of the 
         *  Arg structure.
         *
         * The named item is created in the current structure if it does
         * not already exist (array of SDS type SDS_CHAR) and the specified
         * value inserted.  If the name item already exists, then the value is 
         * converted to that type (if possible) and inserted.
         *
         * @remarks Only available if the marcro DRAMA_ALLOW_CPP_STDLIB is defined.
         *
         * @see DCF(ArgPutString)
         *
         * @param name The name of the item within the SDS structure.
         * @param value The value to be inserted.  The null terminated string
         *             extracted form this is what is inserted.
         * @param status    Inherited status.
         *
         */
        void Put(const char *name,
                 const std::string &value,
                 StatusType *status) {
            ArgPutString(*this, name, value.c_str(), status);
        }
        /** Insert a std::string into a named component of the 
         *  Arg structure.
         *
         * The named item is created in the current structure if it does
         * not already exist (array of SDS type SDS_CHAR) and the specified
         * value inserted.  If the name item already exists, then the value is 
         * converted to that type (if possible) and inserted.
         *
         * @remarks Only available if the marcro DRAMA_ALLOW_CPP_STDLIB is defined.
         *
         * @see DCF(ArgPutString)
         *
         * @param name The name of the item within the SDS structure.
         * @param value The value to be inserted.  The null terminated string
         *             extracted form this is what is inserted.
         * @param status    Inherited status.
         *
         */
        void Put(const std::string &name,
                 const std::string &value,
                 StatusType *status) {
            Put(name.c_str(), value, status);
        }
        /** Fetch a boolean value from a named component of the Arg structure.
         *
         * If the named item exists within the ARG structure then
         * fetch its value and convert it to a bool, if possible.
         *
         * Note that SDS does not directly support boolean values, so an
         * integer representation is used internally and converted to bool.
         *
         * @remarks Only available if the marcro DRAMA_ALLOW_CPP_STDLIB is defined.
         *
         * @see DCF(ArgGetus)
         *
         * @param name The name of the item within the SDS structure.
         * @param value The value returned is written here.
         * @param status    Inherited status.
         *
         */
	void Get (const std::string &name, bool * value, 
					      StatusType * const status) const{
	    unsigned short v;
	    ArgGetus(*this,name.c_str(),&v,status);
	    *value =  v ? true : false;
	}
        /** Fetch a char value from a named component of the Arg structure.
         *
         * If the named item exists within the ARG structure then
         * fetch its value and convert it to a char, if possible.
         *
         * @remarks Only available if the marcro DRAMA_ALLOW_CPP_STDLIB is defined.
         *
         * @see DCF(ArgGetc)
         *
         * @param name The name of the item within the SDS structure.
         * @param value The value returned is written here.
         * @param status    Inherited status.
         *
         */
	void Get (const std::string &name, char * value, StatusType * const status) const {
	    ArgGetc(*this,name.c_str(),value,status);
	}
        /** Fetch a short integer value from a named component of the Arg structure.
         *
         * If the named item exists within the ARG structure then
         * fetch its value and convert it to a short integer, if possible.
         *
         * @remarks Only available if the marcro DRAMA_ALLOW_CPP_STDLIB is defined.
         *
         * @see DCF(ArgGets)
         *
         * @param name The name of the item within the SDS structure.
         * @param value The value returned is written here.
         * @param status    Inherited status.
         *
         */
	void Get (const std::string &name, short * value, StatusType * const status) const {
	    ArgGets(*this,name.c_str(),value,status);
	}
        /** Fetch an unsigned short integer value from a named component of the 
         * Arg structure.
         *
         * If the named item exists within the ARG structure then
         * fetch its value and convert it to an unsigned short integer, if possible.
         *
         * @remarks Only available if the marcro DRAMA_ALLOW_CPP_STDLIB is defined.
         *
         * @see DCF(ArgGetus)
         *
         * @param name The name of the item within the SDS structure.
         * @param value The value returned is written here.
         * @param status    Inherited status.
         *
         */
	void Get (const std::string &name, unsigned short * value, 
					      StatusType * const status) const{
	    ArgGetus(*this,name.c_str(),value,status);
	}
        /** Fetch a  32 bit integer value from a named component of the 
         * Arg structure.
         *
         * If the named item exists within the ARG structure then
         * fetch its value and convert it to a long integer, if possible.
         *
         * @remarks Only available if the marcro DRAMA_ALLOW_CPP_STDLIB is defined.
         *
         * @see DCF(ArgGeti)
         *
         * @param name The name of the item within the SDS structure.
         * @param value The value returned is written here.
         * @param status    Inherited status.
         *
         */
	void Get (const std::string &name, INT32 * value, 
                  StatusType * const status) const {
            Get(name.c_str(), value, status);
	}
        /** Fetch a 32 bit unsigned integer value from a named component of 
         *  the Arg structure.
         *
         * If the named item exists within the ARG structure then
         * fetch its value and convert it to an unsigned long integer, if possible.
         *
         * @remarks Only available if the marcro DRAMA_ALLOW_CPP_STDLIB is defined.
         *
         * @see DCF(ArgGetu)
         *
         * @param name The name of the item within the SDS structure.
         * @param value The value returned is written here.
         * @param status    Inherited status.
         *
         */
	void Get (const std::string &name, UINT32 * value, 
                  StatusType * const status) const{
            Get(name.c_str(), value, status);
	}
        /** Fetch a  64 bit integer value from a named component of the 
         * Arg structure.
         *
         * If the named item exists within the ARG structure then
         * fetch its value and convert it to a 64 bit integer, if possible.
         *
         * @remarks Only available if the marcro DRAMA_ALLOW_CPP_STDLIB is defined.
         *
         * @see DCF(ArgGeti64)
         *
         * @param name The name of the item within the SDS structure.
         * @param value The value returned is written here.
         * @param status    Inherited status.
         *
         */
	void Get (const std::string &name, INT64 * value, StatusType * const status) const {
	    ArgGeti64(*this,name.c_str(),value,status);
	}
        /** Fetch a  unsigned 64 bit integer value from a named component of the 
         * Arg structure.
         *
         * If the named item exists within the ARG structure then
         * fetch its value and convert it to a unsigned 64 bit integer, if possible.
         *
         * @remarks Only available if the marcro DRAMA_ALLOW_CPP_STDLIB is defined.
         *
         * @see DCF(ArgGetu64)
         *
         * @param name The name of the item within the SDS structure.
         * @param value The value returned is written here.
         * @param status    Inherited status.
         */
	void Get (const std::string &name, UINT64 * value, StatusType * const status) const {
	    ArgGetu64(*this,name.c_str(),value,status);
	}
        /** Fetch a float size real value from a named component of the 
         * Arg structure.
         *
         * If the named item exists within the ARG structure then
         * fetch its value and convert it to a float, if possible.
         *
         * @remarks Only available if the marcro DRAMA_ALLOW_CPP_STDLIB is defined.
         *
         * @see DCF(ArgGetf)
         *
         * @param name The name of the item within the SDS structure.
         * @param value The value returned is written here.
         * @param status    Inherited status.
         */
	void Get (const std::string &name, float * value, StatusType * const status) const {
	    ArgGetf(*this,name.c_str(),value,status);
	}
        /** Fetch a double size real value from a named component of the 
         * Arg structure.
         *
         * If the named item exists within the ARG structure then
         * fetch its value and convert it to a double, if possible.
         *
         * @remarks Only available if the marcro DRAMA_ALLOW_CPP_STDLIB is defined.
         *
         * @see DCF(ArgGetd)
         *
         * @param name The name of the item within the SDS structure.
         * @param value The value returned is written here.
         * @param status    Inherited status.
         */
	void Get (const std::string &name, double * value, StatusType * const status) const {
	    ArgGetd(*this,name.c_str(),value,status);
	}
        /** Fetch a string value as a std::string from a named component of the 
         * Arg structure.
         *
         * If the named item exists within the ARG structure then
         * fetch its value and convert it a std::string value, if possible.
         *
         * Note that this does not work for non-char items array items which
         * require more then buflen bytes - since it is not possible to ensure
         * we resize the string appropiately.
         *
         * @remarks Only available if the marcro DRAMA_ALLOW_CPP_STDLIB is defined.
         *
         * @see DCF(ArgGetString)
         *
         * @param name The name of the item within the SDS structure.
         * @param value The value returned is written here.
         * @param status    Inherited status.
         * @param buflen The number of bytes to allow in the string for the
         *               result.  Only used if the underlying value is not 
         *               an array of SDS_CHAR
         */
        void Get(const char *name,
                       std::string *value,
                 StatusType *status,
                 int buflen = 100) const {
            
            /* Find the item we are interested in */
            Arg ItemId(*this, name, status);
            if (*status != STATUS__OK) return;

            char tname[SDS_C_NAMELEN];
            SdsCodeType code;
            long ndims;
            unsigned long dims[SDS_C_MAXARRAYDIMS];

            /* Get details on this item */
            ItemId.Info(tname, &code, &ndims, dims, status);
            if (*status != STATUS__OK) return;

            if ((code == SDS_CHAR)&&(ndims == 1))
            {
                /* Character array.  Ensure we have sufficent space */
                buflen = dims[0] + 1;
            }
            char *buffer;
            buffer = new char[buflen];
            if (buffer == 0)
            {
                *status = ARG__MALLOCERR;
                return;
            }
            ArgGetString(*this, name, buflen, buffer, status);
            if (*status == STATUS__OK)
                *value = buffer;
	    delete[] buffer;

        }

        /*
         * Interface to the above were the name string is a std::string.
         */
        /** Fetch a string value as a std::string from a named component of the 
         * Arg structure.
         *
         * If the named item exists within the ARG structure then
         * fetch its value and convert it a std::string value, if possible.
         *
         * Note that this does not work for non-char items array items which
         * require more then buflen bytes - since it is not possible to ensure
         * we resize the string appropiately.
         *
         * @remarks Only available if the marcro DRAMA_ALLOW_CPP_STDLIB is defined.
         *
         * @see DCF(ArgGetString)
         *
         * @param name The name of the item within the SDS structure.
         * @param value The value returned is written here.
         * @param status    Inherited status.
         * @param buflen The number of bytes to allow in the string for the
         *               result.  Only used if the underlying value is not 
         *               an array of SDS_CHAR
         */
        void Get(const std::string &name,
                       std::string *value,
                 StatusType *status,
                 int buflen = 100) const {
            Get(name.c_str(), value, status, buflen);
        }
#endif

}; /* class */


#endif

#endif
