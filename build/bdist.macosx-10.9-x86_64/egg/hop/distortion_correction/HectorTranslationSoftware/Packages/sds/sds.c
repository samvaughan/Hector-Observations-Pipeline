/*                                                  */
/*            Self-Defining Data System             */
/*                                                  */
/*            Jeremy Bailey  1991 Oct               */
/*                                                  */
/*                                                  */
/*            Version 2.3  2000  Mar                */


/*     @(#) $Id: ACMM:sds/sds.c,v 3.94 09-Dec-2020 17:15:25+11 ks $                        */
/*                                                           */
/*                                                           */
/*                                                           */
/*    Copyright (c)  Anglo-Australian Telescope Board, 1991-2000.  */
/*    Permission granted for use for non-commercial purposes.      */

static const char *rcsId="@(#) $Id: ACMM:sds/sds.c,v 3.94 09-Dec-2020 17:15:25+11 ks $";
static void *use_rcsId = (0 ? (void *)(&use_rcsId) : (void *) &rcsId);


#define _SDS_BUILD  /* Used to enable extra calls in sds.h */

#include "sds.h"
#include "errno.h"


/*
 * Semaphore usage.
 *
 *  There are two types of semaphore systems used, dependent on the 
 *  system we are compiling for.  First there is the ID only semaphore
 *  system - Sds___IdSemTake()/Sds___IdSemGive().  These are used for
 *  VxWorks only at this point.  They are used to provide a lock
 *  around creating IDs.  The presumption of this system is that IDs
 *  are shared between takes, but once an ID is created, it is only
 *  used within one task and we are otherwise safe.  This has proven 
 *  a reliable approach for DRAMA tasks running on VxWorks where an
 *  ID is used by only one task, and that task only has one thread of
 *  control.
 *
 *  The other approach is what is known as the "F" semaphore, "F"
 *  being for "full access lock" semaphore - the calls
 *  Sds___FSemTake()/Sds___FSemGive().  These are used to fully lock
 *  SDS access - every "extern" call within this module uses them
 *  to lock any changes to SDS structures.  This approach is needed
 *  on machines using threads where any thread may access a given
 *  SDS ID.  This approach was introduced very late in the history of
 *  SDS, with the introduction of DRAMA2.  These semaphores are 
 *  implemented as a posix mutex when running under Linux or MacOsX.    
 *  When using this approach, the ID semaphore calls will check
 *  (using assert()) that the semaphore is taken.   There is also
 *  a call - Sds___FSEmCheckTake() which should be invoked by any
 *  static SDs function which should only be invoked when the
 *  semaphore is taken.
 * 
 *    Possible issues:
 *       1. The initialization of the semaphore is done when the
 *           first call which takes it is made.  In theory, this
 *           can go wrong badly if two threads invoke it at the
 *           same time.  But since the main use is in DRAMA tasks and
 *           DRAMA initialization will invoke it before starting threads,
 *           we are safe in normal usage.
 *
 *       2. Dead locks due to use of a second lock within the task.
 *           For example, the DRAMA 2 task wide lock.  Ideally in
 *           a DRAMA 2 task, we should only use the DRAMA 2 task wide
 *           lock to lock SDS.  This will require interface functions
 *           and DRAMA 2 telling SDS what functions to call as part
 *           of a task starting up.  ** To be implemented **
 */


/*
 * all modern linux and apple machines support posix threads.
 *  But if SDS_NO_POSIX_THREADS is defined, don't use them.
 */
#if defined(__APPLE__) || defined(__linux__)
#ifndef SDS_NO_POSIX_THREADS
#define POSIX_THREADS
#endif
#endif

#ifdef VxWorks
#include "semLib.h"

/*
 *  Semaphore to control allocation of identifiers for VxWorks real
 * time operating system  
 */

static SEM_ID   idSemaphore;


static int Sds___IdSemTake(StatusType *status)
{
    STATUS   semStatus;
    if (*status != STATUS__OK) return 0;

    semStatus = semTake(idSemaphore,600);

    if (semStatus != OK) 
    {
        *status = SDS__TIMEOUT;
        return 0;
    }
    return 1;

}
#define Sds___IdSemGive()  semGive(idSemaphore)

/*
 * VxWorks is not supporting full semaphore system.
 */
static int Sds___FSemTake(StatusType *status)
{
    /* This must return 0 on bad status - presumed by callers */
    if (*status != STATUS__OK) return 0;
    return 1;
}
#define Sds___FSemGive() /* */
#define Sds___FSemCheckTaken() /* */



#elif defined(POSIX_THREADS)
#warning "POSIX thread support enabled"
/*
 * POSIX Thread semaphore.  When we have posix threads we
 * must wrap semaphores around all operations.
 */
#include <pthread.h>
#include <assert.h>

static pthread_mutex_t 	id_mutex;


static int fSem_Init = 0;
static int fSem_Taken = 0;
/*
 * Initialise our semaphore - actually as posix recursive mutex.
 */
static void Sds___FSemInit(StatusType *status)
{
    int result = 0;
    /*
     * We want a recursive mutex.  So we need to create
     * an mutex attribute structure which we will use to
     * initialise the mutex.
     */
    pthread_mutexattr_t attr;
    if ((result = pthread_mutexattr_init(&attr)) != 0)
    {
        errno = result; 
        *status = SDS__SEM_INIT_ERR;
        return;
    }
    if ((result = pthread_mutexattr_settype(
             &attr, PTHREAD_MUTEX_RECURSIVE)) != 0)
    {
        errno = result; 
        *status = SDS__SEM_INIT_ERR;
        return;
    }
    
    if ( pthread_mutex_init( &id_mutex, &attr ) )
    {
        errno = result;
        *status = SDS__SEM_INIT_ERR;
        return;
    }
    /* The attribute structure can now be destroyed */
    pthread_mutexattr_destroy(&attr);

    /*
     * Indicate we have been initialized and are NOT taken.
     */
    fSem_Init = 1;
    fSem_Taken = 0;
}

/* 
 * Take the semaphore (posix mutex).  Returns 1 if taken, 0 on error.
 */
static int Sds___FSemTake(StatusType *status)
{
    int result;

    if (*status != STATUS__OK) return 0;

    if (fSem_Init == 0)
        Sds___FSemInit(status);
    assert(fSem_Init);
    
    if ((result = pthread_mutex_lock(&id_mutex)) != 0)
    {
        errno = result;
        *status = SDS__SEM_TAKE_ERR;
    }
    /* Increment count of number of times taken */
    ++fSem_Taken;
    return 1;
}
/*
 * Give the semaphore (posix mutex).  Note, we may not actually
 * release it if we have taken it multiple times.
 * 
 */
static void Sds___FSemGive()
{
    assert(fSem_Init);
    --fSem_Taken;
    assert(fSem_Taken >= 0);
    pthread_mutex_unlock(&id_mutex);
}


/*
 * We don't use ID semaphores if we have Posix threads - we use the F
 * semaphores above.  But - since in every case where we take a Id 
 * semaphore we should already have taken the F semaphore, we check that.
 */
#define CHECK_SEMS
#ifdef CHECK_SEMS
static int Sds___IdSemTake(StatusType *status SDSUNUSED)
{
    assert(fSem_Taken);
    return 1;
}
static void Sds___IdSemGive()
{
    assert(fSem_Taken);
}
/*
 * Check the F semaphore is taken, assert if not.  This should be
 * invoked from anywhere where it is presumed the semaphore is taken,
 * e.g. the static functions.
 */
#define Sds___FSemCheckTaken() assert(fSem_Taken)

#else
#warning "Strongly suggest enabling checking of semaphores"
#define Sds___IdSemTake(_s_) (1) /* _s_ */
#define Sds___IdSemGive() /* */
#define Sds___FSemCheckTaken() /* */
#endif

#else
#warning "SDS is not supporting any locking features - cant use with threading!!"
/*
 * No semaphore case.
 */
#define Sds___IdSemTake(_s_) (1) /* _s_ */
#define Sds___IdSemGive() /* */

static int Sds___FSemTake(StatusType *status)
{
    /* This must return 0 on bad status - presumed by callers */
    if (*status != STATUS__OK) return 0;
    return 1;
}
#define Sds___FSemGive() /* */
#define Sds___FSemCheckTaken() /* */

#endif


#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define SDS_SARRAY   10     /*  Code for structure array  */
#define SDS_NCODES   13     /*  Number of type codes  */


/*    Initial number of handles to allocate   */

#define NHANDLES    100

/*    Default number of structure items       */

#define NITEMS      20

/*    Number of ID blocks per section of ID array  */

#define NIDS        2000

/*    Maximum number of extra bytes      */

#define MAX_EXTRA   128

/*    Maximum number of dimensions   */
 
#define MAXDIMS  SDS_C_MAXARRAYDIMS

/*    Format codes   */

#define  BIGEND    0            /* Big endian integers */
#define  LITTLEEND 1            /* Little endian integers */
#define  IEEE      0            /* IEEE floating point format */
#define  VAXF      1            /* VAX floating point format */
#define  VAXD      VAXF         /* VAX D floating point format  */
#define  IEEEBS    2            /* IEEE floating point format byte swapped */
#define  VAXG      3            /* VAX G floating point format  */

#if 0
/*
 * Used in debugging code to convert format types to strings for output
 */
static char * Sds___LocalFormatToStr(int code, int format)
{
    if ((code == SDS_FLOAT)||(code == SDS_DOUBLE))
    {
        switch (format)
        {
        case 0:
            return "IEEE (0)";
        case 1:
            return "VAXF/VAXD (1)";
        case 2:
            return "IEEEBS (2)";
        case 3:
            return "VAXG (3)";
        default:
        {
            static char buff[80];
            sprintf(buff,"<invalid>(%d)", format);
            return buff;
        }
        }
    }
    else
    {
          
        switch (format)
        {
        case 0:
            return "BIGEND (0)";
        case 1:
            return "LITTLEEND (1)";
        default:
        {
            static char buff[80];
            sprintf(buff,"<invalid>(%d)", format);
            return buff;
        }
        }
    }
}
#endif

/*     Number of bytes in each format   */

static long nbytes[SDS_NCODES] = {0,1,1,1,2,2,4,4,4,8,0,8,8};


/*
 *  The CC$gfloat macro is defined on VAXC systems.  If true it indicates
 *  G floating point is being used.  Under DEC C, this is replaced by 
 *  __G_FLOAT
 */
#ifdef VMS
#ifdef CC$gfloat
#ifndef __G_FLOAT
#define __G_FLOAT CC$gfloat
#endif
#endif
#endif

/*     local format   */

#if defined(VAX) && __G_FLOAT

/*  VAX with G floating point format  */

#define VAX_G_DOUBLE 1
#define VAX_FLOAT 1

   static long  local_format[SDS_NCODES] = {0,0,0,LITTLEEND,LITTLEEND,LITTLEEND,
                                       LITTLEEND,LITTLEEND,VAXF,VAXG,0,LITTLEEND,LITTLEEND};

#elif defined(VAX)

   /*  VAX  */

#define VAX_D_DOUBLE 1
#define VAX_FLOAT 1

   static long  local_format[SDS_NCODES] = {0,0,0,LITTLEEND,LITTLEEND,LITTLEEND,
                                       LITTLEEND,LITTLEEND,VAXF,VAXD,0,LITTLEEND,LITTLEEND};

#elif defined(__alpha) && defined(__vms)

/*  Alpha VMS  */

#ifdef __G_FLOAT

#define VAX_G_DOUBLE 1
#define VAX_FLOAT 1
   static long  local_format[SDS_NCODES] = {0,0,0,LITTLEEND,LITTLEEND,LITTLEEND,
                                       LITTLEEND,LITTLEEND,VAXF,VAXG,0,LITTLEEND,LITTLEEND};

#elif __IEEE_FLOAT
   static long  local_format[SDS_NCODES] = {0,0,0,LITTLEEND,LITTLEEND,LITTLEEND,
                                   LITTLEEND,LITTLEEND,IEEEBS,IEEEBS,0,LITTLEEND,LITTLEEND};

#else

#define VAX_D_DOUBLE 1
#define VAX_FLOAT 1
   static long  local_format[SDS_NCODES] = {0,0,0,LITTLEEND,LITTLEEND,LITTLEEND,
                                       LITTLEEND,LITTLEEND,VAXF,VAXD,0,LITTLEEND,LITTLEEND};

#endif  /* Alpha VMS */


#elif defined(__alpha) 

   /*  Dec Alpha  OSF */

   static long  local_format[SDS_NCODES] = {0,0,0,LITTLEEND,LITTLEEND,LITTLEEND,
                                   LITTLEEND,LITTLEEND,IEEEBS,IEEEBS,0,LITTLEEND,LITTLEEND};
#elif defined MIPSEL

   /*  MIPS cpu with little endian byte order (Decstation)  */

   static long  local_format[SDS_NCODES] = {0,0,0,LITTLEEND,LITTLEEND,LITTLEEND,
                                  LITTLEEND,LITTLEEND,IEEEBS,IEEEBS,0,LITTLEEND,LITTLEEND};
#elif defined M_I86 || defined __x86_64

   /*  Intel 80x86 cpus (including the 64 bit machines)  */
   static long  local_format[SDS_NCODES] = {0,0,0,LITTLEEND,LITTLEEND,LITTLEEND,
                                   LITTLEEND,LITTLEEND,IEEEBS,IEEEBS,0,LITTLEEND,LITTLEEND};

#else

   /*  All others - e.g. SPARC, 680x0 etc. (Big endian, IEEE floats) */

   static long  local_format[SDS_NCODES] = {0,0,0,BIGEND,BIGEND,BIGEND,
                                       BIGEND,BIGEND,IEEE,IEEE,0,BIGEND,BIGEND};
#endif

#ifdef VAX_G_DOUBLE
double MTH$CVT_D_G(double * d);
#endif

#ifdef VAX_D_DOUBLE
double MTH$CVT_G_D(double * d);
#endif



#define  TRUE 1
#define  FALSE 0

#ifndef VxWorks
typedef char    BOOL;
#endif

/*  handle type - handles (which are pointers to pointers) are used to provide
    the links between blocks of structures. The advantage of using a handle
    rather than a direct pointer, is that items can easily be moved about
    with it only being necessary to update a single pointer through which
    everything else accesses the object   */

typedef void   *hptr;
typedef hptr   *handle;

typedef hptr   handle_block[NHANDLES];

typedef char   Sdscode;
typedef char   fmt_code;

/*  Structure block - blocks of this type are used to represent structures */

typedef struct sblock
  {  long maxitems;              /* max number of items  */
     handle self;                /* handle to this block */
     handle parent;              /* handle of parent block */
     Sdscode code;               /* type code  */
     fmt_code format;            /* format code */
     char  name[SDS_C_NAMELEN];  /* item name */
     long   nextra;              /* number of extra bytes */
     char  extra[MAX_EXTRA];     /* extra information */
     short    nitems;            /* number of items */
     handle   item[NITEMS];      /* item handles */
  }  sblock;

/* primitive block - blocks of this type are used to represent primitive items
                     and structure arrays   */

typedef struct pblock
  {  long maxitems;              /* max number of items  */
     handle self;                /* handle to this block */
     handle parent;              /* handle to parent block */
     Sdscode code;               /* type code  */
     fmt_code format;            /* format code */
     char  name[SDS_C_NAMELEN];  /* item name */
     long   nextra;              /* number of extra bytes */
     char  extra[MAX_EXTRA];     /* extra information */
     handle data;                /* handle to data */
     short    ndims;             /* number of dimensions */
     unsigned long   dims[MAXDIMS];       /* dimensions */
  }  pblock;


/*  idblock - a block of this type exists for each identifier allocated.
 *  If the item is internal this contains a handle to the item. If the
 *  item is external it contains a pointer to the item, and also a pointer
 *  to the origin of the block (the latter is needed since pointers in
 *  external items are relative to the origin of the block). 
 */

typedef struct idblock
  { handle   block;              /* handle of block (or pointer to the item
                                     if it is external)  */
    INT32     external;           /* TRUE if item is external */ 
    INT32     free;               /* TRUE if block is to be freed when the id is freed; */
    INT32*    origin;             /* origin of block for external item */
  }  idblock;


static SdsIdType id_number = 0;         /* number of last id allocated  */
static idblock *idarrayptr;        /* pointer to array of identifiers  */
static SdsIdType id_array_size = 0;     /* current size of array of identifiers */
static SdsIdType unused_ids = 0;        /* number of free ids before end of array */


/*
 * Watch info - set by SdsSetWatch(), to allow events to be watched for
 * debugging purposes.
 */
static SdsIdType           SdsWatchID = 0;
static SdsWatchRoutineType SdsWatchRoutine = 0;
static void *              SdsWatchClientData = 0;


SDSEXTERN INT64 SdsSetI64(long high, unsigned long low)

/*  Set value of a 64 bit integer, high contains the high order 32 bits
    low contains the low order 32 bits  */

{
#ifdef NATIVE__INT64 /* Also ok for LONG__64 */
    return((INT64)high*(INT64)(SDS__ULONG_DIVSOR)+low);
#elif defined BIGEND
    INT64 temp;
    temp.i1 = high;
    temp.i2 = low;
    return temp;
#else
    INT64 temp;
    temp.i1 = low;
    temp.i2 = high;
    return temp;
#endif
}

SDSEXTERN UINT64 SdsSetUI64(unsigned long high, unsigned long low)

/*  Set value of a 64 bit unsigned integer, high contains the high order 32 bits
    low contains the low order 32 bits  */

{
#ifdef NATIVE__INT64 /* Also ok for LONG__64 */
    return((UINT64)high*(UINT64)(SDS__ULONG_DIVSOR)+low);
#elif defined BIGEND
    UINT64 temp;
    temp.i1 = high;
    temp.i2 = low;
    return temp;
#else
    UINT64 temp;
    temp.i1 = low;
    temp.i2 = high;
    return temp;
#endif
}

SDSEXTERN void SdsGetI64(INT64 i64, long *high, unsigned long *low)

/*  Get value of a 64 bit integer, as high and low
order 32 bits  */

{
#if defined(NATIVE__INT64)
    UINT64 t = SDS__ULONG_DIVSOR;
    if (i64 < 0)
    {
        *high = (long) (-1*i64 / t) + 1;
        *high *= -1;
        *low = (unsigned long) (i64 % t);
    }
    else
    {
        *high = (long) (i64 / t);
        *low = (unsigned long) (i64 % t);
    }
  

#elif defined BIGEND
    *high = i64.i1;
    *low = i64.i2;
#else
    *high = i64.i2;
    *low = i64.i1;
#endif
}

SDSEXTERN void SdsGetUI64(UINT64 i64, unsigned long *high, unsigned long *low)

/*  Get value of a 64 bit unsigned integer, as high and low
order 32 bits  */

{
#if defined(NATIVE__INT64)
    UINT64 t = SDS__ULONG_DIVSOR;
    *high = (unsigned long) (i64 / t);
    *low = (unsigned long) (i64 % t);
#elif defined BIGEND
    *high = i64.i1;
    *low = i64.i2;
#else
    *high = i64.i2;
    *low = i64.i1;
#endif
}

/*
 * Get a signed 64 bit item into a double.  Could lose precision but
 * useful for debugging and printing purposes on machines without 64
 * bit support.
 * 
 */
SDSEXTERN double SdsGetI64toD(INT64 i64)
{
#if defined(NATIVE__INT64)
    return (double)(i64);

#else
    long high;
    unsigned long low;
    SdsGetI64(i64,&high,&low);
    if (high < 0)
    {
        /* Complicated for negative case, this is
         * the Reverse of code in SdsGetI64() but
         * converting to double
         */
        double highPart = (double)(high+1)*SDS__ULONG_DIVSOR_D;
        return (highPart - SDS__ULONG_DIVSOR + low);
    }
    else
        return ((double)high*SDS__ULONG_DIVSOR_D+(double)(low));
#endif

}

/*
 * Get a signed 64 bit item into a double.  Could lose precision but
 * useful for debugging and printing purposes on machines without 64
 * bit support.
 */
SDSEXTERN double SdsGetUI64toD(UINT64 i64)
{
#if defined(NATIVE__INT64)
    return (double)(i64);

#else
    unsigned long high;
    unsigned long low;
    SdsGetUI64(i64,&high,&low);
    return ((double)high*SDS__ULONG_DIVSOR_D+(double)(low));
#endif

}



typedef struct UI64 {
   UINT32 i1;
   UINT32 i2;
} UI64;

                                   
static UINT32 swap4(BOOL gbyteswap, UINT32 i)

/*  
 *    swap4 - swap the byte order of a long integer if
 *    gbyteswap is true
 */

{
   UINT32 temp;

   if (gbyteswap)
     {
       temp = (i<<24) | ((i<<8) & 0xFF0000) | ((i>>8) & 0xFF00) | (i>>24);
       return(temp);
     }
   else
     return(i);
}
#if 0  /* This does not appear to be used - TJF, Dec 2016 */
static UI64 swap8(BOOL gbyteswap, UI64 i)

/*  
 *    swap8 - swap the byte order of a 64 bit integer if
 *    gbyteswap is true
 */

{
   UI64 temp;
  
   if (gbyteswap)
     {
        temp.i1 = swap4(gbyteswap,i.i2);
        temp.i2 = swap4(gbyteswap,i.i1);
        return(temp);
     }
   else
     return(i);
}
#endif

    
static unsigned short swap2(BOOL gbyteswap, unsigned short i)

/*  
 *    swap2 - swap the byte order of a short integer if
 *    gbyteswap is true.
 */

{
   short temp;

   if (gbyteswap)
     {
       temp = (i<<8) | (i>>8);
       return(temp);
     }
   else
     return(i);
}
    


/*
 *  Sds__convert - Apply number format conversion to a block of data.
 *
 *  This function converts bewteen little endian and big endian integers,
 *  and between the various floating point formats.
 *
 *  The conversion is always to the local format 
 *    i.e. from  format  to  local_format[code]. 
 */

static void Sds__convert(long code, long format, INT32 *data, long length)
{
    union F4
    {  float f; UINT32 l; unsigned char b[4]; };
    union D8
    {  double d; UINT32 l[2]; unsigned char b[8];};

    INT32 *longbuf;           /*  Pointer to buffer as a long  */
    UI64 *llbuf;              /*  Pointer to buffer as a 64 bit integer */
    short *shortbuf;          /*  Pointer to buffer as a short  */
    union F4 *floatbuf;       /*  Pointer to buffer as a float  */
    union D8 *doublebuf;      /*  Pointer to buffer as a double  */
    long i;                   /*  Loop counter  */
    UINT32 tl;                /*  long temporary value   */
    UI64 tll;                 /*  64 bit temporary value  */
    unsigned short ts;        /*  short temporary value  */
    union F4 tf,tf2;          /*  float temporary values  */
    union D8 td,td2;          /*  double temporary values  */
    unsigned long e;          /*  exponent of floating point number  */


    Sds___FSemCheckTaken();
#if 0
    printf("Sds__convert invoked on code of %s, format %s, length %ld. Local format %s\n",
           SdsTypeToStr(code,1), 
           Sds___LocalFormatToStr(code, format), 
           length,  
           Sds___LocalFormatToStr(code, local_format[code]));
#endif

    if ((code == SDS_I64) || (code == SDS_UI64))
    {

/*  Convert 64 bit integer types by byte swapping  */

        llbuf = (UI64*)data;
        for(i=0;i<((length)/8);i++)
        {
            tl = llbuf[i].i1;
            tll.i2 = (tl<<24) | ((tl<<8) & 0xFF0000) | 
                ((tl>>8) & 0xFF00) | (tl>>24);
            tl = llbuf[i].i2;
            tll.i1 = (tl<<24) | ((tl<<8) & 0xFF0000) | 
                ((tl>>8) & 0xFF00) | (tl>>24);
            llbuf[i] = tll;
        }
    }  
    if ((code == SDS_INT) || (code == SDS_UINT))
    {

/*  Convert long integer types by byte swapping  */

        longbuf = (INT32*)data;
        for(i=0;i<((length)/4);i++)
        {
            tl = longbuf[i];
            longbuf[i] = (tl<<24) | ((tl<<8) & 0xFF0000) | 
                ((tl>>8) & 0xFF00) | (tl>>24);
        }
    }
    else if ((code == SDS_SHORT) || (code == SDS_USHORT))
    {

/*  Convert short integer types by byte swapping  */

        shortbuf = (short*)data;
        for(i=0;i<((length)/2);i++)
        {
            ts = shortbuf[i];
            shortbuf[i] = (ts<<8) | (ts>>8);
        }
    }
    else if (code == SDS_FLOAT)
#if defined(VAX_FLOAT)
    {
#if 0
        printf("Sds__convert:Native is VAX_FLOAT\n");
#endif

/*  Convert to VAX floating point type from IEEE or byte-swapped IEEE  */
/*  NOTE - This code is based on the assumption that it is running on a
    VAX (or VMS Alpha), as will always be the case in SDS, 
    since conversion is always to the local format. 
    It is therefore included in a conditional compilation
    as it is not needed on other architectures.                        */

        if (format == IEEE || format == IEEEBS) 
        {
            floatbuf = (union F4*)data;
            for (i=0;i<((length)/4);i++)
            {
                tf = floatbuf[i];    

/*  Swap bytes as required to get correct order for VAX  */

                if (format == IEEE)
                {
                    tf2.b[0] = tf.b[1];
                    tf2.b[1] = tf.b[0];
                    tf2.b[2] = tf.b[3];
                    tf2.b[3] = tf.b[2];
                }
                else
                {
                    tf2.b[0] = tf.b[2];
                    tf2.b[1] = tf.b[3];
                    tf2.b[2] = tf.b[0];
                    tf2.b[3] = tf.b[1];
                }

/*  Extract the exponent  */

                e = tf2.l & 0x7F80;
                if (e == 0)

/*  The exponent is zero (plus or minus zero or a denormalized number)   */

                    if ((tf2.l & 0x60) != 0)

/*  Check for a denormalized number which can still be represented in VAX
    format - Such cases will have an exponent of either 1 or 2 in
    VAX format and the fraction needs shifting left by 1 or 2  */

                    {
                        e = (tf2.l & 0x40) ? 2 : 1;
                        tf.l = ((tf2.l & 0xFFFF007F) << (3-e)) |
                            (tf2.b[3] >> (5+e));
                        tf.l = tf.l | ((e << 7) & 0x180) | (tf2.l & 0x8000);
                        floatbuf[i] = tf;
                    } 
                    else 

/*  Otherwise set result to zero   */

                        floatbuf[i].f = 0.0;
                else if (e == 0x7F80 || e == 0x7F00)

/*  If the exponent is 254 or 255 the number is a NaN or Infinity or is too
    large to be represented in VAX format, so set result to a bad value.  */

                    floatbuf[i].l = 0xFFFFFFFF;
                else

/*  Otherwise the conversion can be handled by a simple multiplication
    by 4 to account for the difference in exponent bias, and radix point
    position   */

                    floatbuf[i].f = tf2.f*4.0;
            }
        }
    }
#else
    {
#if 0
        printf("Sds__convert:Native is NOT VAX_FLOAT\n");
#endif

        if (format == IEEE || format == IEEEBS)
        {

/*  Conversion from IEEE to byte swapped IEEE  */

            longbuf = (INT32*)data;
            for(i=0;i<((length)/4);i++)
            {
                tl = longbuf[i];
                longbuf[i] = (tl<<24) | ((tl<<8) & 0xFF0000) | 
                    ((tl>>8) & 0xFF00) | (tl>>24);
            }
        }
        else if (format == VAXF)
        {

/*  Conversion from VAX to IEEE or byte swapped IEEE  */
/*  NOTE - We have to be careful here because the machine
    could have either little endian or big endian byte order. The
    assumption in this code is that byte swapped IEEE is always
    on machines with little endian integers, and normal IEEE on
    machines with big endian integers  */

            floatbuf = (union F4*)data;
            for (i=0;i<((length)/4);i++)
            {
                tf = floatbuf[i];    

/*  Swap bytes as required to get correct order   */

                if (local_format[code] == IEEEBS)
                {
                    tf2.b[0] = tf.b[2];
                    tf2.b[1] = tf.b[3];
                    tf2.b[2] = tf.b[0];
                    tf2.b[3] = tf.b[1];
                }
                else
                {
                    tf2.b[0] = tf.b[1];
                    tf2.b[1] = tf.b[0];
                    tf2.b[2] = tf.b[3];
                    tf2.b[3] = tf.b[2];
                }

/*  Extract the exponent  */

                e = tf2.l & 0x7F800000;

/*  If the exponent is zero the resulting value is zero. Also check
    the number for a bad value. Otherwise the conversion can be accomplished
    by multiplying by 0.25.   */

                if (e == 0) 
                    floatbuf[i].f = 0.0;
                else if (tf2.l == 0xFFFFFFFF)
                {
                    tf2.l = 0xFFFFFFFF;
                    tf2.b[1] = 0x7F;
                    floatbuf[i] = tf2;
                }
                else
                    floatbuf[i].f = (float) (tf2.f*0.25);
            }
        }
    }
#endif
    else if (code == SDS_DOUBLE)
#if defined(VAX_D_DOUBLE)
    {
#if 0
        printf("Sds__convert:Native is VAX_D_DOUBLE\n");
#endif

/*  Convert to VAX D floating point type from IEEE or byte-swapped IEEE  */

        if (format == IEEE || format == IEEEBS) 
        {
            doublebuf = (union D8*)data;
            for (i=0;i<((length)/8);i++)
            {
                td = doublebuf[i];

/*  Swap bytes if required to get correct order  */

                if (format == IEEEBS)
                {
                    td2.l[1] = (td.l[0]<<24) | ((td.l[0]<<8) & 0xFF0000) | 
                        ((td.l[0]>>8) & 0xFF00) | (td.l[0]>>24);
                    td2.l[0] = (td.l[1]<<24) | ((td.l[1]<<8) & 0xFF0000) | 
                        ((td.l[1]>>8) & 0xFF00) | (td.l[1]>>24);
                    td = td2;
                }
                e = ((td.b[0] & 0x7F) << 4) | ((td.b[1] >> 4) & 0xF);

/*  If the exponent is zero (plus or minus zero or a denormalized number)
    set the result to zero. Also set zero for any biased exponent less than
    894 which is too small to be represented in VAX format. Set a bad value
    for anything with biased exponent greater than 1150, which is either
    too large for the VAX representation or is a NaN or Infinity.   */

                if (e <= 894) 
                    doublebuf[i].d = 0.0;
                else if (e >= 1150)
                {
                    td2.l[0] = 0xFFFFFFFF;
                    td2.l[1] = 0xFFFFFFFF;
                    doublebuf[i] = td2;
                }
                else
                {

/*  The number is a normalized IEEE value within the range that can be
    represented on the VAX. Decrement the exponent by 894 to give
    corresponding VAX value and reassemble the number.    */

                    e -= 894;
                    td2.b[0] = ((e<<7) & 0x80) | ((td.b[1] & 0xF) << 3)
                        | ((td.b[2] >> 5) & 0x7);
                    td2.b[1] = ((e>>1) & 0x7F) | (td.b[0] & 0x80);
                    td2.b[2] = ((td.b[3] << 3) & 0xF8) |((td.b[4] >> 5) & 0X7);
                    td2.b[3] = ((td.b[2] << 3) & 0xF8) |((td.b[3] >> 5) & 0X7);
                    td2.b[4] = ((td.b[5] << 3) & 0xF8) |((td.b[6] >> 5) & 0X7);
                    td2.b[5] = ((td.b[4] << 3) & 0xF8) |((td.b[5] >> 5) & 0X7);
                    td2.b[6] = ((td.b[7] << 3) & 0xF8);
                    td2.b[7] = ((td.b[6] << 3) & 0xF8) |((td.b[7] >> 5) & 0X7);
                    doublebuf[i] = td2;
                }
            }
        }
        else if (format == VAXG)
        {
  
/*  Conversion from VAX G floating to VAX D floating  */

            doublebuf = (union D8*)data;
            for(i=0;i<((length)/8);i++)
            {
                td = doublebuf[i];

/*  Check that the exponent is in the range that can be 
    represented as a D floating value - if it is too small
    return 0, otherwise set to bad value  */            

                e = ((td.b[1] & 0x7F) << 4) | ((td.b[0] >> 4) & 0xF);
            
                if (e == 0 || (e < 896)) 
                    doublebuf[i].d = 0.0;
                else if (e >= 1151)
                {
                    td2.l[0] = 0xFFFFFFFF;
                    td2.l[1] = 0xFFFFFFFF;
                    doublebuf[i] = td2;
                }
                else

/*  Otherwise use the RTL routine to do the conversion  */

                    doublebuf[i].d = MTH$CVT_G_D(&(td.d));
            }
	}    
    }
#elif defined(VAX_G_DOUBLE)
    {
#if 0
        printf("Sds__convert:Native is VAX_G_DOUBLE\n");
#endif

/*  Convert to VAX G floating point type from IEEE or byte-swapped IEEE  */

        if (format == IEEE || format == IEEEBS) 
        {
            doublebuf = (union D8*)data;
            for (i=0;i<((length)/8);i++)
            {
                td = doublebuf[i];
#if 0
                printf("Sds__convert:td.b  %x %x %x %x %x %x %x %x\n",
                       td.b[0], td.b[1], td.b[2], td.b[3],
                       td.b[4], td.b[5], td.b[6], td.b[7]);
#endif

/*  Swap bytes if required to get correct order  */

                if (format == IEEE)
                {
                    td2.b[0] = td.b[1];
                    td2.b[1] = td.b[0];
                    td2.b[2] = td.b[3];
                    td2.b[3] = td.b[2];
                    td2.b[4] = td.b[5];
                    td2.b[5] = td.b[4];
                    td2.b[6] = td.b[7];
                    td2.b[7] = td.b[6];
                }
                else
                {
                    td2.b[0] = td.b[6];
                    td2.b[1] = td.b[7];
                    td2.b[2] = td.b[4];
                    td2.b[3] = td.b[5];
                    td2.b[4] = td.b[2];
                    td2.b[5] = td.b[3];
                    td2.b[6] = td.b[0];
                    td2.b[7] = td.b[1];
                }
                /*
                 * Grab the exponent.
                 */
                e = ((td2.b[0] & 0x7F) << 4) | ((td2.b[1] >> 4) & 0xF);
#if 0
                printf("Sds__convert:td2.b %x %x %x %x %x %x %x %x\n",
                       td2.b[0], td2.b[1], td2.b[2], td2.b[3],
                       td2.b[4], td2.b[5], td2.b[6], td2.b[7]);



                printf("Sds__convert:e =%ld (%ld | %ld)\n",
                       e, ((td2.b[0] & 0x7F) << 4),  ((td2.b[1] >> 4) & 0xF));

#endif

/*  If the exponent is zero (plus or minus zero or a denormalized number)
    set the result to zero. Set a bad value
    for anything with biased exponent of >= 2046, which is a NaN or Infinity
    or is too large to be represented   */

                if (e == 0) 
                    doublebuf[i].d = 0.0;
                else if (e >= 2046)
                {
                    td2.l[0] = 0xFFFFFFFF;
                    td2.l[1] = 0xFFFFFFFF;
                    doublebuf[i] = td2;
                }
                else
                {

/* Otherwise we can handle the conversion simply by multiplying by 4
   to allow for the difference in exponent bias and radix point position  */

                    doublebuf[i].d = td2.d*4.0;
                }

            }
        }
        else if (format == VAXD)
        {
  
/*  Conversion from VAX D floating to VAX G floating  */

            doublebuf = (union D8*)data;
            for(i=0;i<((length)/8);i++)
            {
                td = doublebuf[i];

/*  Use the RTL routine - we don't have to check the range as any
    D-floating number is in the range that can be represented in G-floating */

                doublebuf[i].d = MTH$CVT_D_G(&(td.d));
            }
	}    
    }
#else
    {
#if 0
        printf("Sds__convert:Native is IEEE/IEEEBS\n");
#endif
        if (format == IEEE || format == IEEEBS)
        {

/*  Conversion from IEEE to byte swapped IEEE  */

            doublebuf = (union D8*)data;
            for(i=0;i<((length)/8);i++)
            {
                td = doublebuf[i];
                td2.l[1] = (td.l[0]<<24) | ((td.l[0]<<8) & 0xFF0000) | 
                    ((td.l[0]>>8) & 0xFF00) | (td.l[0]>>24);
                td2.l[0] = (td.l[1]<<24) | ((td.l[1]<<8) & 0xFF0000) | 
                    ((td.l[1]>>8) & 0xFF00) | (td.l[1]>>24);
                doublebuf[i] = td2;
            }
        }
        else if (format == VAXD)
        {

/*  Conversion from VAX D-Floating to IEEE or byte swapped IEEE  */

            doublebuf = (union D8*)data;
            for (i=0;i<((length)/8);i++)
            {
                td = doublebuf[i];    

/*  Check for bad value and return correpsonding IEEE bad value  */

                if ((td.l[0] == 0xFFFFFFFF) && (td.l[1] == 0xFFFFFFFF))
                {
                    td2.b[1] = 0xEF;
                }
                else
                {

/*  Extract the exponent  */

                    e = ((td.b[1]<<1) & 0xFE) | ((td.b[0]>>7) & 0x1) ;
                    if (e != 0)
                    {

/*  If the exponent is non-zero, increment it by 894 and reassemble
    the resulting IEEE value   */

                        e += 894;
                        td2.b[0] = (unsigned char) ((td.b[1] & 0x80) | ((e >> 4) & 0x7F));
                        td2.b[1] = (unsigned char) (((td.b[0]>>3) & 0xF) |((e<<4) & 0xF0));
                        td2.b[2] = ((td.b[0]<<5) & 0xE0) |((td.b[3]>>3) &0x1F);
                        td2.b[3] = ((td.b[3]<<5) & 0xE0) |((td.b[2]>>3) &0x1F);
                        td2.b[4] = ((td.b[2]<<5) & 0xE0) |((td.b[5]>>3) &0x1F);
                        td2.b[5] = ((td.b[5]<<5) & 0xE0) |((td.b[4]>>3) &0x1F);
                        td2.b[6] = ((td.b[4]<<5) & 0xE0) |((td.b[7]>>3) &0x1F);
                        td2.b[7] = ((td.b[7]<<5) & 0xE0) |((td.b[6]>>3) &0x1F);
                    }
                    else

/*  If the exponent is zero the result is zero   */

                        td2.d = 0.0;
                }
                if (local_format[code] == IEEEBS)
                {

/*  Byte swap the result if necessary  */

                    td.l[1] = (td2.l[0]<<24) | ((td2.l[0]<<8) & 0xFF0000) 
                        | ((td2.l[0]>>8) & 0xFF00) | (td2.l[0]>>24);
                    td.l[0] = (td2.l[1]<<24) | ((td2.l[1]<<8) & 0xFF0000) 
                        | ((td2.l[1]>>8) & 0xFF00) | (td2.l[1]>>24);
                    doublebuf[i] = td;
                }
                else
                    doublebuf[i] = td2;
            }
        }
        else if (format == VAXG)
        {

/*  Conversion from VAX G-Floating to IEEE or byte swapped IEEE  */

            doublebuf = (union D8*)data;
            for (i=0;i<((length)/8);i++)
            {
                td = doublebuf[i];    

/*  Check for bad value and return correpsonding IEEE bad value  */

                if ((td.l[0] == 0xFFFFFFFF) && (td.l[1] == 0xFFFFFFFF))
                {
                    td2 = td;
                    if (local_format[code] == IEEE) 
                        td2.b[1] = 0xEF;
                    else
                        td2.b[6] = 0xEF;
                    doublebuf[i] = td2; 
                }
                else
                {

/*  Swap bytes if required to get correct order  */

                    if (local_format[code] == IEEE)
                    {
                        td2.b[0] = td.b[1];
                        td2.b[1] = td.b[0];
                        td2.b[2] = td.b[3];
                        td2.b[3] = td.b[2];
                        td2.b[4] = td.b[5];
                        td2.b[5] = td.b[4];
                        td2.b[6] = td.b[7];
                        td2.b[7] = td.b[6];
                    }
                    else
                    {
                        td2.b[0] = td.b[6];
                        td2.b[1] = td.b[7];
                        td2.b[2] = td.b[4];
                        td2.b[3] = td.b[5];
                        td2.b[4] = td.b[2];
                        td2.b[5] = td.b[3];
                        td2.b[6] = td.b[0];
                        td2.b[7] = td.b[1];
                    }
  
/*  Handle the conversion by multiplying by 0.25 to allow for the
    difference in exponent bias and radix point position  */

                    doublebuf[i].d = 0.25*td2.d;
                }

            }
        }
    }
#endif
}


/*
 *  Sds__get_new_id  -  Allocate a new identifier. Return the id number as
 *                   the function result, and return a pointer to the
 *                   id block in *idptr.
 */
static void Sds__get_new_id(idblock* *idptr, SdsIdType *id, StatusType *status)
{
    idblock *temp;
    SdsIdType i;

    if (*status != STATUS__OK) return;

    *id = 0;
#ifdef VxWorks
    {
        STATUS   semStatus;

        /*  If running under VxWorks create the semaphore used for controlling
            access to the id allocation system  */
        
        if (id_array_size == 0)
        {
            idSemaphore = semMCreate(SEM_Q_PRIORITY | SEM_DELETE_SAFE |
                                     SEM_INVERSION_SAFE);
            if (idSemaphore == NULL)
            {
                *status = SDS__SEM_INIT_ERR;
                return;
            }
        }
        
        /*  If running under VxWorks take the semaphore   */
        
        semStatus = semTake(idSemaphore,600);
        if (semStatus != OK) 
        {
            *status = SDS__SEM_TAKE_ERR;
            return;
        }
    }

#endif

/*  See if there are unused entries in the current id array  */

    if (unused_ids > 0)
    {
        for (i = 1; i <= id_number; i++) 
        {
            if (idarrayptr[i].block == NULL ) 
            {
                *idptr = &(idarrayptr[i]);

/*  Set .block component to mark the id as used - The actual value is
    irrelevant as long as it is not NULL  */

                idarrayptr[i].block = (handle)(*idptr);
                unused_ids--;
                Sds___IdSemGive();
                idarrayptr[i].free = FALSE;
                Sds___WatchEvent(i, SDS_WATCH_EVENT_NEWID, "Sds__get_new_id");
                *id = i;
                return;
            }
        }
    }
         
    id_number=id_number+1;

/*  Check of the id array has room for any more identifiers  */

    if (id_number >= id_array_size)
    {

/*  If no room we have to create a larger id array and copy the contents
    of the old array into it  */

        id_array_size = id_array_size+NIDS;
        temp = (idblock*)SdsMalloc(sizeof(idblock)*id_array_size);
        if (temp == 0)
        {
            *status = SDS__NOMEM;
            return;
        }
        if (id_array_size > NIDS)
        {
            for (i=1;i<id_array_size-NIDS;i++)
                temp[i] = idarrayptr[i];
#ifndef VxWorks
            SdsFree(idarrayptr);
#endif
        }
        idarrayptr = temp;
    }

/*  then return the new id number and the pointer  */

    *idptr = &(idarrayptr[id_number]);

/*  Set .block component to mark the id as used - The actual value is
    irrelevant as long as it is not NULL  */

    idarrayptr[id_number].block = (handle)(*idptr);
    *id = id_number;

    Sds___IdSemGive();

    idarrayptr[*id].free = FALSE;

    Sds___WatchEvent(*id, SDS_WATCH_EVENT_NEWID, "Sds__get_new_id");

    return;

}



/*
 * Access an SDS Id.  
 *  The ID is validated (in range and allocated) 
 */
static void Sds__get_id(
    SdsIdType id,   /*>> The ID to access */
    handle *block,  /*<< handle of block (or pointer to the item if it is external*/
    INT32 *external,/*<< TRUE if item is external */ 
    INT32 **origin, /*<< Origin of block for external item*/
    StatusType *status)

{

    Sds___FSemCheckTaken();  /* If we are using the Full semaphore, it
                              * should be taken at this point */

    if (*status != SDS__OK) return;

    if (!Sds___IdSemTake(status))
    {
        /*  The semaphore take might have failed because no ids have 
            yet been created and so the semaphore has not been created
            In this case the id must be bad and we should return BADID
            status  */

        if ((id <= 0) || (id > id_number))
            *status = SDS__BADID;
        return;
    }
    /*
     * From this point on - must always exit via "Exit" label.  To ensure
     * semaphore is released.
     */

    if ((id <= 0) || (id > id_number))
    {
        *status = SDS__BADID;
        goto Exit;
    }
    if (idarrayptr[id].block == NULL)   
    {
        *status = SDS__BADID;
        goto Exit;
        return;
    }
    if (!(idarrayptr[id].external))
        if (*(idarrayptr[id].block) == NULL)   
        {
            *status = SDS__BADID;

            goto Exit;
        }

    *block = idarrayptr[id].block;
    *external = idarrayptr[id].external;
    *origin = idarrayptr[id].origin;


Exit:  /* Common return - to release semaphore on exit */
    Sds___IdSemGive(); 

}


/*
 * Wrapper around taking the F semaphore and getting an ID, 
 * a very common combination.
 *
 *  If this returns with *status = STATUS__OK, then the
 *   semaphore is taken.  Otherwise the semaphore is not
 *   taken.
 */
static void Sds__FSemTake_get_id(
    SdsIdType id,   /*>> The ID to access */
    handle *block,  /*<< handle of block (or pointer to the item if it is external*/
    INT32 *external,/*<< TRUE if item is external */ 
    INT32 **origin, /*<< Origin of block for external item*/
    StatusType *status)
{
    if (!Sds___FSemTake(status)) 
    {
        return;  /* Semaphore failure */
    }

    Sds__get_id(id,block,external,origin,status);

    if (*status != STATUS__OK)
    {
        /* ID failure, must give the semaphore before returning */
        Sds___FSemGive();
    }
}

/*
 * Given an ID - fill in the details.
 */
static void Sds__put_id( 
    SdsIdType id,    /* ID number */
    handle block,    /* handle of block (or pointer to the item if it is external)*/
    INT32 external,  /* TRUE if item is external */
    INT32 *origin,   /* origin of block for external item */
    StatusType *status)

{

    Sds___FSemCheckTaken();  /* If we are using the Full semaphore, it
                              * should be taken at this point */


    if (*status != SDS__OK) return;

    if (!Sds___IdSemTake(status)) return;

    if ((id <= 0) || (id > id_number))
    {
        *status = SDS__BADID;
        Sds___IdSemGive();
        return;
    }
    if (idarrayptr[id].block == NULL)   
    {
        *status = SDS__BADID;

        Sds___IdSemGive();
        return;
    }

    idarrayptr[id].block = block;
    idarrayptr[id].external = external;
    idarrayptr[id].origin = origin;
    idarrayptr[id].free = 0;

    Sds___IdSemGive();
}




/*
 *  Sds__import_id - Given an id number return a pointer to the id block
 */
static idblock *Sds__import_id(SdsIdType id, StatusType * SDSCONST status)
{
    Sds___FSemCheckTaken();  /* If we are using the Full semaphore, it
                              * should be taken at this point */


    if (*status != SDS__OK) return(NULL);
    if ((id <= 0) || (id > id_number))
    {
        *status = SDS__BADID;
        return(NULL);
    }
    if (idarrayptr[id].block == NULL)   
    {
        *status = SDS__BADID;
        return(NULL);
    }
    if (!(idarrayptr[id].external))
        if (*(idarrayptr[id].block) == NULL)   
        {
            *status = SDS__BADID;
            return(NULL);
        }
    return(&(idarrayptr[id]));
}




static void * null_handle = NULL;
/*
 *   Sds__get_handle - Allocate a new handle
 */
static handle Sds__get_handle(void)
{
    handle temp;


    temp = (handle)SdsMalloc(sizeof(void*));
/*  printf("new handle %d \n",(long)temp);*/
    return(temp);

}


/*
 * Used to make an SDS id as requiring that the data be free-ed (by 
 * calling Sds__Free()).
 */
SDSEXTERN void Sds__MarkFree(SdsIdType id)

{
    idblock *idb;
    StatusType status = STATUS__OK;

    if (!Sds___FSemTake(&status)) return;
    if (!Sds___IdSemTake(&status)) return; /* Won't do anything if
                                            * FSemTake does take a
                                            * semaphore 
                                            */

    idb = Sds__import_id(id,&status);
    idb->free = TRUE;

    Sds___IdSemGive();
    Sds___FSemGive();
}

/*
 * Free the data associated with an Sds id.
 */
SDSEXTERN void Sds__Free(SdsIdType id, StatusType * SDSCONST status)

{
    idblock *idb;

    if (*status != SDS__OK) return;

    if (!Sds___FSemTake(status)) return;
    if (!Sds___IdSemTake(status)) return; /* Won't do anything if
                                           * FSemTake() does take a
                                           * semaphore 
                                           */

    idb = Sds__import_id(id,status);
    if (*status != SDS__OK) goto Exit;

    if (idb->free) SdsFree(idb->origin);
    idb->free = 0;


Exit:
    Sds___IdSemGive();
    Sds___FSemGive();

}




static void Sds__extend_struct(sblock **sb,StatusType * SDSCONST status)

/*
 *  Sds__extend_struct - extend an sblock when the space for structure items
 *                       has filled up
 */

{
    sblock *newsb;
    long n;
    long i;

    Sds___FSemCheckTaken();  /* If we are using the Full semaphore, it
                              * should be taken at this point */

    if(*status != SDS__OK) return;

/*  Create a new block which has room for NITEMS more handles than the
 *  present one
 */
    n = (*sb)->maxitems;
    newsb = (sblock*) SdsMalloc(sizeof(sblock)+(n*sizeof(handle)));
    if (newsb == NULL)
    {
        *status = SDS__NOMEM;
        return;
    }
/*
 *  Copy over contents
 */

    *newsb = **sb;
    for (i=NITEMS;i<n;i++)
    {
        newsb->item[i] = (*sb)->item[i];
    }
/*
 *  Update the handle to point to new block
 */

    *(newsb->self) = (void*)newsb;
     
    newsb->maxitems = n+NITEMS;
    SdsFree(*sb);
    *sb = newsb;

}



/*
 *  Sds__create_sblock - Create a new strucure block
 */
static sblock* Sds__create_sblock(SDSCONST char *name, long nextra, SDSCONST char *extra, long items)
{
    sblock* sb;
    long j;
    long n;

    Sds___FSemCheckTaken();  /* If we are using the Full semaphore, it
                              * should be taken at this point */
/*
 *  sblocks are allocated with space for (n+1) * NITEMS items.
 *  work out what n has to be (NITEMS items are included in the
 *  definition of sblock, and n more blocks are added on the
 *  end if required)
 */

    if (items > NITEMS) 
    {
        n = items/NITEMS;
        if ((items % NITEMS) == 0) n--;
    }
    else
        n = 0;
/*
 *  Allocate the space
 */
    sb = (sblock*)SdsMalloc(sizeof(sblock) + (n*NITEMS*sizeof(handle)));
    if (sb == NULL) return(NULL);
/*
 *  Initialize the fields of the block
 */
    strcpy(sb->name,name);
    sb->maxitems = (n+1)*NITEMS;
    sb->code = SDS_STRUCT;
    sb->format = 0;
    sb->nitems = 0;
    sb->nextra = nextra;
    for (j=0;j<nextra;j++) sb->extra[j] = extra[j];
    return(sb);
}



/*				S d s C r e a t e

 *  Function name:
      SdsCreate

 *  Function:
      Create a new structure

 *  Description:
      Creates a new empty internal structure and returns an identifier
      to it.
      
 *  Language:
      C

 *  Declaration:
      void SdsCreate(char *name, long nextra, char *extra, SdsIdType *id, long *status)
      
 *  Parameters:   (">" input, "!" modified, "W" workspace, "<" output)
      (>) name      (char*) The name of the structure to create.
                            The name should be of maximum length 16
                            characters including the terminating null.
      (>) nextra    (long)  The number of bytes of extra information to
                            be included (maximum 128).
      (>) extra     (char*) The extra information to be included with the item.
                            nextra bytes from here are copied into the
                            structure.
      (<) id        (SdsIdType*) Identifier to the created structure.
      (!) status    (long*) Modified status. Possible failure codes are:
                            SDS__NOMEM => Insufficient memory for creation
                            SDS__LONGNAME => name is too long

 *  Prior requirements:
      None.

 *  Support: Jeremy Bailey, {AAO}

 *  Version date: 18-Oct-91
 *
 */
SDSEXTERN void SdsCreate(
    SDSCONST char *name, 
    long nextra, 
    SDSCONST char *extra, 
    SdsIdType *id, 
    StatusType * SDSCONST status)
{
    /* No semaphore take needed as SdsNew() does that */
   SdsNew(0,name,nextra,extra,SDS_STRUCT,0,NULL,id,status);
}

/*
 *  Sds__info_ext - handle the external case for the Sdsinfo function
 */
static void Sds__info_ext(INT32 *data, char *name, long *code, long *ndims, 
                   unsigned long *dims, StatusType * SDSCONST status)
{
    INT32 pos;
    int i;

    Sds___FSemCheckTaken();  /* If we are using the Full semaphore, it
                              * should be taken at this point */

    if (*status != SDS__OK) return;
 
    *code = ((char*)data)[0];
    strncpy(name,((char*)data)+4,SDS_C_NAMELEN);
    if (*code != SDS_STRUCT)
    {
        *ndims = ((short*)data)[1];
        pos = 5;
        if (*code != SDS_SARRAY) pos++;
        for (i=0;i<*ndims;i++)
            dims[i] = data[pos+i];
    }
    else
        *ndims = 0;
    if (*code == SDS_SARRAY) *code = SDS_STRUCT;
}  
   



/*+				S d s I n f o

 *  Function name:
      SdsInfo

 *  Function:
      Return information about an object

 *  Description:
      Given the identifier to an object, return the name, type code and
      dimensions of the object.
      
 *  Language:
      C

 *  Declaration:
      void SdsInfo(SdsIdType id, char *name, SdsCodeType *code, long *ndims, 
               unsigned long *dims, StatusType * SDSCONST status)
    
 *  Parameters:   (">" input, "!" modified, "W" workspace, "<" output)
      (>) id        (SdsIdType) The identifier to the data object.
      (<) name      (char*) The name of the data object. A pointer to a
                            character string with space for at least
                            16 characters should be used.
      (<) code      (SdsCodeType*) The type code for the object. One of
                            the following values (defined in sds.h):
                               SDS_STRUCT => Structure
                               SDS_CHAR   => Character
                               SDS_BYTE   => Signed byte
                               SDS_UBYTE  => Unsigned byte
                               SDS_SHORT  => Signed short integer
                               SDS_USHORT => Unsigned short integer
                               SDS_INT    => Signed long integer
                               SDS_UINT   => Unsigned long integer
                               SDS_I64    => Signed 64 bit integer
                               SDS_UI64   => Unsigned 64 bit integer
                               SDS_FLOAT  => Floating point
                               SDS_DOUBLE => Double precision floating point
      (<) ndims     (long*) The number of dimensions if the object
                            is a primitive or structure array.
      (<) dims      (unsigned long*) The dimensions of the data. An array of
                            size at least 7 should be allowed to receive this.
      (!) status    (StatusType*) Modified status. Possible failure codes are:
                            SDS__BADID => The identifier is invalid

 *  Prior requirements:
      None.

 *  Support: Jeremy Bailey, {AAO}

 *  Version date: 18-Oct-91
 *-
 */

SDSEXTERN void SdsInfo(SdsIdType id, char *name, SdsCodeType *code, long *ndims, unsigned long *dims, StatusType * SDSCONST status)

{
    /*idblock *idptr;*/
    handle block;
    INT32 external;
    INT32* origin;
    sblock *sb;
    pblock *pb;
    long i;

    /* Take semaphore and access id */
    Sds__FSemTake_get_id(id,&block,&external,&origin,status);
    if (*status != SDS__OK) return;
    /*
     * From this point on - must always exit via "Exit" label.  To ensure
     * semaphore is released.
     */


    if (external)
    {


/*  If the object is external call the routine to handle the external case */

        Sds__info_ext((INT32*)(block),name,code,ndims,dims,status);
        goto Exit;
    }

/*  Otherwise get a pointer to the sblock  */

    sb = (sblock*)(*block);

/*  Copy the name and type code  */

    strcpy(name,sb->name);
    if (sb->code == SDS_SARRAY)
        *code = SDS_STRUCT;
    else
        *code = sb->code;

/*  If it is not a structure, get the dimensions   */

    if (sb->code != SDS_STRUCT)
    {
        pb = (pblock*)sb;
        *ndims = pb->ndims;
        for (i=0;i<pb->ndims;i++)  dims[i] = pb->dims[i];
    }   
    else
        *ndims = 0;

Exit:
    Sds___FSemGive();
}

/*+				S d s I s D e f i n e d

 *  Function name:
      SdsIsDefined

 *  Function:
      Return whether a primitive object is defined

 *  Description:
      Given the identifier to an object, return whether its data is defined.
      
 *  Language:
      C

 *  Declaration:
      int SdsIsDefined(SdsIdType id, StatusType * SDSCONST status)
    
 *  Parameters:   (">" input, "!" modified, "W" workspace, "<" output)
      (>) id        (SdsIdType) The identifier to the data object.
      (!) status    (StatusType*) Modified status. Possible failure codes are:
                            SDS__BADID => The identifier is invalid
                            SDS__NOTPRIM => The identifier is not a primitive item

 *  Returned:
      SdsIsDefined  (int) True if data is defined. False otherwise.

 *  Prior requirements:
      None.

 *  Support: Jeremy Bailey, {AAO}

 *  Version date: 18-Oct-91
 *-
 */

SDSEXTERN int SdsIsDefined(SdsIdType id, StatusType * SDSCONST status)
{
    pblock *pb;
    handle block;
    INT32 external;
    INT32 *origin;
    char code;
    int isData = 0;    

    Sds__FSemTake_get_id(id,&block,&external,&origin,status);
    if (*status != SDS__OK) return 0;
    /*
     * From this point on, must give semaphore before
     * returning.
     */

    if (external)
    {
        code  = ((char*)block)[0];
        isData = ((((INT32 *)block)[5]) != 0);
    }
    else
    {
        pb = (pblock*)(*block);
        code = pb->code;
        isData = (pb->data != NULL);
    }

    if (code == SDS_STRUCT || code == SDS_SARRAY) 
    {
        *status = SDS__NOTPRIM;
        isData = 0;
    }
    
    Sds___FSemGive();
    return isData;

}

/*
 * Validate arguments to SdsNew().
 */
static void Sds__ValidateNewArgs(
    SDSCONST char * SDSCONST name, 
    SDSCONST long nextra, 
    SDSCONST SdsCodeType code,
    long ndims, 
    StatusType * SDSCONST status
    )
{
    if (*status != STATUS__OK) return;

    /*
     * check length of string  
     */
    if (strlen(name) > (unsigned)(SDS_C_NAMELEN-1))      
    {
        *status = SDS__LONGNAME;
        return;
    }

    /*
     * check number of extra bytes  
     */
    if (nextra > MAX_EXTRA)
    {
        *status = SDS__EXTRA;
        return;
    }

    /*  
     * check code  
     */
    if ((code > SDS_NCODES) || (code<0) || (code == SDS_SARRAY))
    {
        *status = SDS__INVCODE;
        return;
    }

    /*
     * check dimensions  
     */
    if ((ndims > MAXDIMS) || (ndims<0))
    {
        *status = SDS__INVDIMS;
        return;
    }

}


/*+				  S d s N e w

 *  Function name:
      SdsNew

 *  Function:
      Create a new object

 *  Description:
      Creates a new component in an existing internal structure or a
      new top level object. A top level object is created by specifying
      a parent_id of zero. The new object can be a structure, a structure 
      array, or a primitive. A structure array is specified by means of a 
      type code of SDS_STRUCT and a non-zero number for ndims. If the type 
      code is SDS_STRUCT and ndims is zero an ordinary structure is created. 
      A primitive type is specified by the appropriate type code.
      
 *  Language:
      C

 *  Declaration:
      void SdsNew(SdsIdType parent_id, char *name, long nextra, char *extra, 
         SdsCodeType code, long ndims, unsigned long *dims, SdsIdType *id, 
         StatusType * SDSCONST status)
      
 *  Parameters:   (">" input, "!" modified, "W" workspace, "<" output)
      (>) parent_id (SdsIdType) The identifier of the structure to
                            which the object is to be added. Use a 
                            value of zero to create a new top level object.
      (>) name      (char*) The name of the object to create.
                            The name should be of maximum length 16
                            characters including the terminating null.
      (>) nextra    (long)  The number of bytes of extra information to
                            be included (maximum 128).
      (>) extra     (char*) The extra information to be included with the item.
                            nextra bytes from here are copied into the
                            structure.
      (>) code      (SdsCodeType) The type code for the item to be created. One of
                            the following values (defined in sds.h):
                               SDS_STRUCT => Structure
                               SDS_CHAR   => Character
                               SDS_BYTE   => Signed byte
                               SDS_UBYTE  => Unsigned byte
                               SDS_SHORT  => Signed short integer
                               SDS_USHORT => Unsigned short integer
                               SDS_INT    => Signed long integer
                               SDS_UINT   => Unsigned long integer
                               SDS_I64    => Signed 64 bit integer
                               SDS_UI64   => Unsigned 64 bit integer
                               SDS_FLOAT  => Floating point
                               SDS_DOUBLE => Double precision floating point
                               
      (>) ndims     (long)  Number of dimensions for the item. Zero
                            to create a scalar item.
      (>) dims      (unsigned long*) Array of dimensions for the item. Should be of
                            size at least ndims. A NULL pointer may be used
                            if the item is a scalar.
      (<) id        (SdsIdType*) Identifier to the created object.
      (!) status    (StatusType*) Modified status. Possible failure codes are:
                            SDS__BADID => Invalid identifier
                            SDS__NOMEM => Insufficient memory for creation
                            SDS__LONGNAME => name is too long
                            SDS__EXTRA => Too much extra data
                            SDS__INVCODE => Invalid type code 
                            SDS__INVDIMS => Invalid dimensions
                            SDS__NOTSTRUCT => Parent is not a structure
                            SDS__EXTERN => Parent is external

 *  Support: Jeremy Bailey, {AAO}

 *  Version date: 18-Oct-91
 *-
 */

SDSEXTERN void SdsNew(
    SdsIdType parent_id, 
    SDSCONST char *name, 
    long nextra, 
    SDSCONST char *extra, 
    SdsCodeType code,
    long ndims, 
    SDSCONST unsigned long *dims, 
    SdsIdType *id, 
    StatusType * SDSCONST status)

{
    sblock *parentStruct=0;            /* pointer to parent structure */
    pblock *primBlock;            /* pointer to primitive being created */
    sblock *structBlock;           /* pointer to structure being created */
    long ncomps;           /* number of components in structure array */
    handle *hblock;        /* pointer to block of handles to structure array */
    sblock *structArrayCompBlock;           /* pointer to structure array components sblock */
    BOOL structure;        /* True if we are creating a structure */
    int n=0;
    int i;
    BOOL toplevel;         /* TRUE for a top level object  */
    handle block;
    INT32 external;
    INT32 *origin;
    idblock *idptr;


    if (*status != SDS__OK) return;

    *id = 0;

    /*  Validate arguments */
    Sds__ValidateNewArgs(name, nextra, code, ndims, status);
    if (*status != STATUS__OK) return;

    if (!Sds___FSemTake(status)) return;
    /*
     * From this point on - must always exit via "Exit" label.  To ensure
     * semaphore is released.
     */

    /* import parent  */

    if (parent_id == 0) 
    {
        toplevel = TRUE;
    }
    else
    {
        Sds___WatchEvent(parent_id, SDS_WATCH_EVENT_NEW, "SdsNew");
        toplevel = FALSE;
        Sds__get_id(parent_id,&block,&external,&origin,status);
        if (*status != SDS__OK) return;
        if (external)
        {
            *status = SDS__EXTERN;
            goto Exit;
        }
        parentStruct = (sblock*)(*block);

/*  check that it is a structure  */

        if (parentStruct->code != SDS_STRUCT)
        {
            *status = SDS__NOTSTRUCT;
            goto Exit;
        }
    }



/*  create new block - if it is a structure we create an sblock but also
 *  copy the pointer as a pblock* - many of the fields are identical and
 *  can be accessed through this.
 *
 *  if it is a primitive or a structure array we create a pblock
 */

    if ((code == SDS_STRUCT) && (ndims == 0))
    {
        structBlock = (sblock*)SdsMalloc(sizeof(sblock));
        if (structBlock == NULL)
        {
            *status = SDS__NOMEM;
            goto Exit;
        }
        primBlock = (pblock*) structBlock;
        structure = TRUE;
    }
    else
    {
        primBlock = (pblock*)SdsMalloc(sizeof(pblock));
        if (primBlock == NULL)
        {
            *status = SDS__NOMEM;
            goto Exit;
        }
        structBlock = NULL;
        structure = FALSE;
    }

/* fill in fields */

    if (toplevel) 
        primBlock->parent = NULL;
    else
        primBlock->parent = block;
    primBlock->code = (char) code;
    strcpy(primBlock->name,name);
    primBlock->ndims = (short) ndims;
    if (ndims != 0)
    {
        for (i=0;i<ndims;i++)  primBlock->dims[i]=dims[i];
    }
    primBlock->nextra = nextra;
    for (i=0;i<nextra;i++)
    {
        primBlock->extra[i] = extra[i];
    }
    if (! structure) 
    {
        primBlock->data = NULL;
        primBlock->format = (fmt_code) local_format[code];
    }
    else
    {
        structBlock->maxitems = NITEMS;
        structBlock->nitems = 0;
        structBlock->format = 0;
    }

/* extend the parent block if necessary */

    if (!toplevel)
    {
        n = parentStruct->nitems;
        if (n >= parentStruct->maxitems) Sds__extend_struct(&parentStruct,status);
        if (*status != SDS__OK) return;
        parentStruct->nitems = n+1;
    }

/* get a handle for the new component */

    primBlock->self = Sds__get_handle();
    if ((primBlock->self) == NULL)
    {
        *status = SDS__NOMEM;
        goto Exit;
    }
    *(primBlock->self) = (void*)primBlock;
    if (!toplevel) parentStruct->item[n] = primBlock->self;

/* get an id for the new component  */

    Sds__get_new_id(&idptr,id, status);

/* Fill in ID details */
    Sds__put_id(*id,primBlock->self,FALSE,NULL,status);

    if (*status != SDS__OK) goto Exit;

/*  if it is a structure array we have to create an array of handles
 *  to point to the component sblocks of the array  
 */

    if ((code == SDS_STRUCT) && (ndims != 0)) 
    {
        primBlock->code = SDS_SARRAY;
        ncomps = 1;
        for(i=0;i<ndims;i++)  ncomps = ncomps*dims[i];
        if (ncomps == 0) ncomps = 1;
        hblock = (handle*)SdsMalloc(ncomps*sizeof(handle));
        if (hblock == 0)
        {
            *status = SDS__NOMEM;
            goto Exit;
        }
        primBlock->data = (handle)hblock;
/*
 *  Now create and initialize an sblock for each component of the 
 *  structure array
 */
        for(i=0;i<ncomps;i++)
        {
            structArrayCompBlock = Sds__create_sblock(name,nextra,extra,(long)NITEMS);
            if (structArrayCompBlock == NULL) 
            {
                *status = SDS__NOMEM;
                goto Exit;
            }
            structArrayCompBlock->parent = primBlock->self;
            structArrayCompBlock->self = Sds__get_handle();
            if ((structArrayCompBlock->self) == NULL)
            {
                *status = SDS__NOMEM;
                goto Exit;
            }
            *(structArrayCompBlock->self) = (void*)structArrayCompBlock;
            hblock[i] = structArrayCompBlock->self;
        }       
    }
Exit:
    Sds___FSemGive();
}


/*+				 S d s N u m I t e m s

 *  Function name:
      SdsNumItems

 *  Function:
     Return the number of components in a structure.
 *  Description:
     Return the number of components in a structure, that is, the
       maximum index value to SdsIndex.
 
 *  Language:
      C

 *  Declaration:
      void SdsNumItems(SdsIdType id, long *numItems,  
             StatusType * SDSCONST status)
      
 *  Parameters:   (">" input, "!" modified, "W" workspace, "<" output)
      (>) id (SdsIdType) Identifier of the structure.
      (>) numItems     (long)  The number of components in the struture.
      (!) status    (StatusType*) Modified status. Possible failure codes are:
                            SDS__NOMEM => Insufficient memory for creation
                            SDS__NOTSTRUCT => parent_id not a structure
           

 *  Support: Tony Farrell, {AAO}

 *  Version date: 25-Feb-2015
 
 *  Modified:
       25-Feb-2015 -  KS - Removed the unused idptr variable.
 *-
 */

SDSEXTERN void SdsNumItems(SdsIdType id, long *numItems, 
                           StatusType * SDSCONST status)

{
    sblock *sb=0;        /*  Pointer to structure block  */
    long code;           /*  Type code of parent  */
    INT32 *data=0;       /*  Pointer to external data block  */
    long nitems=0;       /*  Number of items in structure  */
    handle block;
    INT32 external;
    INT32 *origin;

    Sds__FSemTake_get_id(id,&block,&external,&origin,status);
    if (*status != SDS__OK) return;
    /*
     * From this point on - must give Semaphore before returning.
     */
    if (external)
    {

/*  In the external case get the data block pointer, type code and number of
    items   */

        data = (INT32*)(block);
        code = ((char*)data)[0];
        nitems = ((short*)data)[1];
    }
    else
    {

/*  In the internal case get the sblock pointer, the type code, and the number
    of items  */

        sb = (sblock*)(*block);
        code = sb->code;
        nitems = sb->nitems;
    }

/*  If it is not a structure return an error   */

    if (code != SDS_STRUCT)
    {
        *status = SDS__NOTSTRUCT;
        nitems = 0;
    }

    *numItems = nitems;
    Sds___FSemGive();

}



/*+				 S d s I n d e x

 *  Function name:
      SdsIndex

 *  Function:
      Find a structure component by position

 *  Description:
      Given the index number of a component in a structure, return an
      identifier to the component. 
      
 *  Language:
      C

 *  Declaration:
      void SdsIndex(SdsIdType parent_id, long index, SdsIdType *id, 
             StatusType * SDSCONST status)
      
 *  Parameters:   (">" input, "!" modified, "W" workspace, "<" output)
      (>) parent_id (SdsIdType) Identifier of the structure.
      (>) index     (long)  Index number of the component to be returned.
                            Items in a structure are numbered in order of 
                            creation starting with one.
      (<) id        (SdsIdType*) Identifier to the component.
      (!) status    (StatusType*) Modified status. Possible failure codes are:
                            SDS__NOMEM => Insufficient memory for creation
                            SDS__NOTSTRUCT => parent_id not a structure
                            SDS__NOITEM => No item with that index number


 *  Support: Jeremy Bailey, {AAO}

 *  Version date: 18-Oct-91
 *-
 */

SDSEXTERN void SdsIndex(SdsIdType parent_id, long index, 
              SdsIdType *id, StatusType * SDSCONST status)

{
    sblock *sb=0;        /*  Pointer to structure block  */
    idblock *idptr;      /*  Pointer to id block of component  */
    long code;           /*  Type code of parent  */
    INT32 *data=0;          /*  Pointer to external data block  */
    long nitems;         /*  Number of items in structure  */
    handle block;
    INT32 external;
    INT32 *origin;



    /* Take semaphore and import parent id */
    Sds__FSemTake_get_id(parent_id,&block,&external,&origin,status);
    if (*status != SDS__OK) return;
    /*
     * From this point on - must always exit via "Exit" label.  To ensure
     * semaphore is released.
     */

    *id = 0;

    if (external)
    {

        /*
         * In the external case get the data block pointer, type code
         *  and number of items   
         */
        data = (INT32*)(block);
        code = ((char*)data)[0];
        nitems = ((short*)data)[1];
    }
    else
    {

        /* 
         * In the internal case get the sblock pointer, the type code,
         * and the number of items  
         */

        sb = (sblock*)(*block);
        code = sb->code;
        nitems = sb->nitems;
    }

    /*  If it is not a structure return an error   */

    if (code != SDS_STRUCT)
    {
        *status = SDS__NOTSTRUCT;
        goto Exit;
    }

    if ((index <= nitems) && (index > 0))
    {

        /*  Get a new id block for the component   */

        Sds__get_new_id(&idptr, id, status);

        /*  Fill in the id block with the appropriate component  */

        if (external)
            Sds__put_id(*id,(handle)(origin+data[4+index]),
                        external,origin,status);
        else
            Sds__put_id(*id,sb->item[index-1],external,origin,status);
    }
    else
    {
        *status = SDS__NOITEM;
        goto Exit;
    }
Exit:
    Sds___FSemGive();


}




/*
 * Handle the external case of the SdsFind function
 */
static void Sds__find_ext(INT32* origin, INT32 *data, 
                   SDSCONST char *name, SdsIdType *id, 
                   StatusType * SDSCONST status)
{
    idblock *idptr;     
    BOOL found;  
    int index;
    INT32* item;
    int nitems;

    Sds___FSemCheckTaken();  /* If we are using the Full semaphore, it
                              * should be taken at this point */

    if (((char*)data)[0] != SDS_STRUCT)
    {
        *status = SDS__NOTSTRUCT;
        return;
    }
    found = FALSE;
    nitems = ((short*)data)[1];

/*  Loop over the items in the structure until a match with the name
    is found or the end of the list is reached   */

    for(index=0;(index<nitems)&&(!found);index++)
    {
        item = origin + data[5+index];
        if (name[0] == ((char*)item)[4])
        {
            if(strcmp(name,((char*)item)+4) == 0)
	    {
                Sds__get_new_id(&idptr, id, status);
                Sds__put_id(*id,(handle)item,TRUE,origin,status);

                if (*status != SDS__OK) return;
                found = TRUE;
    	    }
        }
    }
    if (! found)
    {
        *status = SDS__NOITEM;
        return;
    }
}



/*+				S d s F i n d

 *  Function name:
      SdsFind

 *  Function:
      Find a structure component by name

 *  Description:
      Given the name of a component in a structure, return an
      identifier to the component. 
      
 *  Language:
      C

 *  Declaration:
      void SdsFind(SdsIdType parent_id, char *name, SdsIdType *id, 
         StatusType * SDSCONST status)
      
 *  Parameters:   (">" input, "!" modified, "W" workspace, "<" output)
      (>) parent_id (SdsIdType) Identifier of the structure.
      (>) name      (char*) Name of the component to be found.
      (<) id        (SdsIdType*) Identifier to the component.
      (!) status    (StatusType*) Modified status. Possible failure codes are:
                            SDS__NOMEM => Insufficient memory for creation
                            SDS__NOTSTRUCT => parent_id not a structure
                            SDS__NOITEM => No item with that name


 *  Support: Jeremy Bailey, {AAO}

 *  Version date: 18-Oct-91
 *-
 */

SDSEXTERN void SdsFind(SdsIdType parent_id, SDSCONST char *name, SdsIdType *id, StatusType * SDSCONST status)

{
    sblock *sb;
    idblock *idptr;
    BOOL found;
    int index;
    sblock *pb;
    handle block;
    INT32 external;
    INT32 *origin;

    /* Take semaphore and import parent id */
    Sds__FSemTake_get_id(parent_id,&block,&external,&origin,status);
    if (*status != SDS__OK) return;

    /*
     * From this point on - must always exit via "Exit" label.  To ensure
     * semaphore is released.
     */
    *id = 0;

    if (external)
    {
        Sds__find_ext(origin,(INT32*)(block),name,id,status);
        goto Exit;
    }
    sb = (sblock*)(*block);
    if (sb->code != SDS_STRUCT)
    {
        *status = SDS__NOTSTRUCT;
        goto Exit;
    }
    found = FALSE;

/*  Loop over structure items until a match with the name is found or until
    the end of the list is reached   */

    for(index=0;(index<(sb->nitems))&&(!found);index++)
    {
        pb = (sblock*)(*(sb->item[index]));
        if (name[0] == (pb->name)[0])
        {
            if(strcmp(name,pb->name) == 0)
	    {
                Sds__get_new_id(&idptr, id, status);
                Sds__put_id(*id,sb->item[index],external,origin,status);

                if (*status != SDS__OK) goto Exit;
                found = TRUE;
	    }
        }
    }
    if (! found)
    {
        *status = SDS__NOITEM;
        goto Exit;
    }
Exit:
    Sds___FSemGive();
}



static void Sds__pointer_ext(INT32 *origin, INT32 *ptr, void **data, 
                             unsigned long *length, StatusType *status)

/*
 *   Handle the external case of the SdsPointer function
 */

{
    long i;
    long code;
    long format;
    long ndims;
    unsigned long dims[SDS_C_MAXARRAYDIMS];
    INT32 dptr;


    Sds___FSemCheckTaken();  /* If we are using the Full semaphore, it
                              * should be taken at this point */

    code = ((char*)ptr)[0];
    if (code == SDS_STRUCT) 
    {
        *status = SDS__NOTPRIM;
        return;
    }
/*
 * If the data is undefined return an error
 */
    dptr = ptr[5];
    if (dptr == 0)
    {
        *status = SDS__UNDEFINED;
        *length = 0;
        *data = NULL;
        return;
    }
/*
 *  Calculate length of the data
 */

    ndims = ((short*)ptr)[1];
    *length = nbytes[code];
    for(i=0;i<ndims;i++) 
    {
        dims[i] = ptr[6+i];
        *length = (*length)*dims[i];
    }
/*
 *  Convert the data if necessary
 */
    format = ((char*)ptr)[1];
    if (format != local_format[code]) 
    {
        Sds__convert(code,format,origin+dptr,*length);
        ((char*)ptr)[1] = (char) local_format[code];
    }
    *data = (void*)(origin+dptr);

}
       


/*+				S d s P o i n t e r

 *  Function name:
      SdsPointer

 *  Function:
      Get a pointer to the data of a primitive item

 *  Description:
      Return a pointer to the data of a primitive item. Also return the length
      of the item. If the data item is undefined and the object is internal
      storage for the data will be created.

      SdsPointer can only be used with primitive items, not with structures.

      If necessary (e.g. if the data originated on a machine with different
      architecture) the data for the object is converted (in place) from the 
      format stored in the data item to that required for the local machine

      If the data pointed to by the pointer is updated by a calling program,
      the program should then call the function SdsFlush to ensure that the
      data is updated in the original structure. This is necessary because
      implementations on some machine architectures may have to use a copy
      of the data rather than the actual data when returning a pointer.
      
 *  Language:
      C

 *  Declaration:
      void SdsPointer(SdsIdType id, void **data, unsigned long *length, 
              StatusType * SDSCONST status)
      
 *  Parameters:   (">" input, "!" modified, "W" workspace, "<" output)
      (>) id        (SdsIdType)  Identifier of the primitive item.
      (<) data      (void**) Address of a variable to hold the pointer.
      (<) length    (unsigned long*) Length of the data.
      (!) status    (StatusType*) Modified status. Possible failure codes are:
                            SDS__NOMEM => Insufficient memory for creation
                            SDS__NOTPRIM => Not a primitive item
                            SDS__UNDEFINED => Data undefined, and object external


 *  Support: Jeremy Bailey, {AAO}

 *  Version date: 18-Oct-91
 *-
 */


SDSEXTERN void SdsPointer(SdsIdType id, void **data, unsigned long *length, 
                          StatusType * SDSCONST status)

{
    pblock *pb;
    long i;
    handle block;
    INT32 external;
    INT32 *origin;

    Sds__FSemTake_get_id(id,&block,&external,&origin,status);
    if (*status != SDS__OK) return;
    /*
     * From this point on - must always exit via "Exit" label.  To ensure
     * semaphore is released.
     */
    *data = 0;
    *length = 0;

    if (external)
    {
        Sds__pointer_ext(origin,(INT32*)(block),data,length,status);
        goto Exit;
    }
    pb = (pblock*)(*block);
    if (pb->code == SDS_STRUCT) 
    {
        *status = SDS__NOTPRIM;
        goto Exit;
    }
/*
 *  Calculate length of the data
 */

    *length = nbytes[(int)pb->code];
    for(i=0;i<(pb->ndims);i++)  *length = (*length)*(pb->dims[i]);
/*
 * If the data is undefined create it
 */
    if (pb->data == NULL)
    {

/* Create the data block  */

        *data = (void*) SdsMalloc(*length);
        if (*data == NULL) 
        {
            *status = SDS__NOMEM;
            goto Exit;
        }

/* Create a handle to it */

        pb->data = Sds__get_handle();
        if (pb->data == NULL) 
        {
            *status = SDS__NOMEM;
            goto Exit;
        }

/* Set handle to point to data */

        *(pb->data) = *data;
        pb->format = (char) local_format[(int)pb->code];
    }
    else

/* Data is already there so return value in handle */

    {
        *data = *(pb->data);
    }
/*
 *  Convert the data if necessary
 */
    if (pb->format != local_format[(int)pb->code]) 
    {
        Sds__convert((long)(pb->code),(long)(pb->format),
                     (INT32*)(*(pb->data)),*length);
        pb->format = (char) local_format[(int)pb->code];
    }
Exit:
    Sds___FSemGive();

}

 

/*+				S d s F l u s h

 *  Function name:
      SdsFlush

 *  Function:
      Flush data updated via a pointer

 *  Description:
      If a primitive data item is accessed via SdsPointer, and the data
      array updated via the returned pointer, then SdsFlush must be called
      to ensure that the data is updated in the original structure.

      This must be done since implementations on some machine architectures
      may have to use a copy of the data rather than the actual data when
      returning a pointer.
      
 *  Language:
      C

 *  Declaration:
      void SdsFlush(SdsIdType id, StatusType * SDSCONST status)
      
 *  Parameters:   (">" input, "!" modified, "W" workspace, "<" output)
      (>) id        (SdsIdType)  Identifier of the primitive item.
      (!) status    (StatusType*) Modified status. Possible failure codes are:
                            SDS__BADID   => Identifier invalid
                            SDS__NOTPRIM => Not a primitive item


 *  Support: Jeremy Bailey, {AAO}

 *  Version date: 7-Feb-92
 *-
 */


SDSEXTERN void SdsFlush(SdsIdType id SDSUNUSED, 
                        StatusType * SDSCONST status SDSUNUSED)
{
    /* Null implementation for all current versions */
}


/*
 *  Adjust ptr to match the alignment requirements for an item of type
 *  code.
 */
static void Sds__align(
    unsigned long *ptr, 
    SdsCodeType code, 
    char* data SDSUNUSED)
{

/*  The test structure below is used to determine the alignment requirements
    for a given type on the current architecture   */

    typedef struct test
    {
        INT32 s1;
        char c1;
        short s2;
        INT32 l1;
        char c2;
        INT32 l2;
        float f1;
        char c3;
        float f2;
        double d1;
        char c4;
        double d2;
        INT64 j1;
        char c5;
        INT64 j2;
    }  test;

    test t;
    short alignment;
    short adjustment;
    short temp;


/*  Determine the alignment required for type code  */

    switch (code)
    {
    case SDS_SHORT:
    case SDS_USHORT:
        alignment = ((char*)(&t.s2))-(&t.c1);
        break;
    case SDS_INT:
    case SDS_UINT:
        alignment = ((char*)(&t.l2))-(&t.c2);
        break;
    case SDS_FLOAT:
        alignment = ((char*)(&t.f2))-(&t.c3);
        break;
    case SDS_DOUBLE:
        alignment = ((char*)(&t.d2))-(&t.c4);
        break;
    case SDS_I64:
    case SDS_UI64:
        alignment = ((char*)(&t.j2))-(&t.c5);
        break;
    default:
        return;
    }

/*  if ptr does not have the correct alignment, adjust it by incrementing
    it to match the required alignment   */

    temp = (short)(*ptr % alignment);
    if (temp == 0) return;
    adjustment = alignment-temp;
    *ptr = *ptr+adjustment;
    return;
}

/*
 *  Handle the internal primitive case of the SdsGet function 
 */
static void Sds__get_int(pblock *pb, unsigned long length, unsigned long offset, 
                         void* data, unsigned long *actlen, StatusType * SDSCONST status)


{
    long i;

    Sds___FSemCheckTaken();  /* If we are using the Full semaphore, it
                              * should be taken at this point */
/*
 * If the data is undefined return an error
 */
    if (pb->data == NULL)
    {
        *status = SDS__UNDEFINED;
        *actlen = 0;
        return;
    }
/*
 *  Calculate length of the data
 */

    *actlen = nbytes[(int) pb->code];
    for(i=0;i<(pb->ndims);i++)  *actlen = (*actlen)*(pb->dims[i]);
  
/*
 *  Convert the data if necessary
 */
    if (pb->format != local_format[(int)pb->code]) 
    {
        Sds__convert((long)(pb->code),(long)(pb->format),
                     (INT32*)(*(pb->data)),*actlen);
        pb->format = (char) local_format[(int)pb->code];
    }

    *actlen = *actlen-(offset*nbytes[(int)pb->code]);
    if (length < *actlen) *actlen = length;
/*
 *  Copy the data
 */
    if (pb->ndims == 0)
    {
        if (*actlen == 4) 
            *(INT32*)data = *((INT32*)*(pb->data)+offset);
        else if (*actlen == 8)
        {
            *(INT32*)data = *((INT32*)*(pb->data)+2*offset);
            *((INT32*)data+1) = *((INT32*)*(pb->data)+2*offset+1);
        }
        else if (*actlen == 2)
            *(short*)data = *((short*)*(pb->data)+offset);
        else if (*actlen == 1)
            *(char*)data = *((char*)*(pb->data)+offset);
    }
    else
        memcpy(data,((char*)*(pb->data))+offset*nbytes[(int)pb->code],*actlen);
}


       

/*
 *  Handle the external primitive case of the SdsGet function
 */
static void Sds__get_ext(INT32 *origin, INT32 *ptr, unsigned long length, 
                         unsigned long offset, void *data, unsigned long *actlen, 
                         StatusType * SDSCONST status)
{
    int i;
    long code;
    long format;
    long ndims;
    unsigned long dims[SDS_C_MAXARRAYDIMS];
    INT32 dptr;

    Sds___FSemCheckTaken();  /* If we are using the Full semaphore, it
                              * should be taken at this point */



    code = ((char*)ptr)[0];
    if (code == SDS_STRUCT) 
    {
        *status = SDS__NOTPRIM;
        return;
    }
/*
 * If the data is undefined return an error
 */
    dptr = ptr[5];
    if (dptr == 0)
    {
        *status = SDS__UNDEFINED;
        *actlen = 0;
        return;
    }
/*
 *  Calculate length of the data
 */

    ndims = ((short*)ptr)[1];
    *actlen = nbytes[code];
    for(i=0;i<ndims;i++) 
    {
        dims[i] = ptr[6+i];
        *actlen = (*actlen)*dims[i];
    }
  
/*
 *  Convert the data if necessary
 */
    format = ((char*)ptr)[1];
    if (format != local_format[code]) 
    {
        Sds__convert(code,format,origin+dptr,*actlen);
        ((char*)ptr)[1] = (char) local_format[code];
    }

    *actlen = *actlen-(offset*nbytes[code]);
    if (length < *actlen) *actlen = length;
/*
 *  Copy the data
 */
    if (ndims == 0)
    {
        if (*actlen == 4) 
            *(INT32*)data = *((INT32*)(origin+dptr)+offset);
        else if (*actlen == 8)
        {
            *(INT32*)data = *((INT32*)(origin+dptr)+2*offset);
            *((INT32*)data+1) = *((INT32*)(origin+dptr)+2*offset+1);
        }
        else if (*actlen == 2)
            *(short*)data = *((short*)(origin+dptr)+offset);
        else if (*actlen == 1)
            *(char*)data = *((char*)(origin+dptr)+offset);
    }
    else
        memcpy(data,((char*)(origin+dptr)+offset*nbytes[code]),*actlen);
}
       



/*
 *  Handle the internal structure case of the SdsGet function
 */
static void Sds__get_struct(sblock* sb, unsigned long length, void *data, 
                            unsigned long *actlen, StatusType * SDSCONST status)
{
    int i;
    int ncomps;
    pblock *pb;
    handle *hblock;
    unsigned long ilen;

    Sds___FSemCheckTaken();  /* If we are using the Full semaphore, it
                              * should be taken at this point */

    if (sb->code == SDS_STRUCT)
    {

/*  If it is a structure get all the component of the structure in sequence  */

        for (i=0;i<sb->nitems;i++)
            Sds__get_struct((sblock*)(*(sb->item[i])),length,data,
                            actlen,status);
    }
    else if (sb->code == SDS_SARRAY)
    {

/*  If it is a structure array get all the components in sequence  */

        pb = (pblock*)sb;
        hblock = (handle*)(pb->data);
        ncomps = 1;
        for (i=0;i<pb->ndims;i++) ncomps = ncomps*pb->dims[i];
        for (i=0;i<ncomps;i++)
            Sds__get_struct((sblock*)(*(hblock[i])),length,data,actlen,status);
    } 
    else
    {

/*  If it is a primitive get the data, copying it to the output buffer
    after the current value of actlen bytes - then increment actlen.
    Ensure correct alignment by calling Sds__align to adjust actlen to
    give the correct alignment for the current type.   */ 

        pb = (pblock*)sb;
        Sds__align(actlen,pb->code,(char*)data);
        if (*actlen > length)
        {
            *actlen = length;
            return;
        }
        Sds__get_int(pb,length-*actlen,(long)0,(void*)(((char*)data)+(*actlen)),
                     &ilen,status);
        if (*status != SDS__OK) return;
        *actlen += ilen;
        if (*actlen >= length) return;
    }
}

/*
 *  Handle the external structure case of the SdsGet function
 */
static void Sds__get_struct_ext(
    INT32 *origin, INT32 *ptr, unsigned long length, void *data, 
    unsigned long *actlen, StatusType *status)
{
    int i;
    int ncomps;
    int nitems;
    unsigned long ilen;
    long code;

    Sds___FSemCheckTaken();  /* If we are using the Full semaphore, it
                              * should be taken at this point */


    code = ((char*)ptr)[0];
    nitems = ((short*)ptr)[1];
    if (code == SDS_STRUCT)
    {

/*  If it is a structure get all the component of the structure in sequence  */

        for (i=0;i<nitems;i++)
            Sds__get_struct_ext(origin,origin+ptr[5+i],length,
                                data,actlen,status);
    }
    else if (code == SDS_SARRAY)
    {

/*  If it is a structure array get all the components in sequence  */

        ncomps = 1;
        for (i=0;i<nitems;i++) ncomps = ncomps*ptr[5+i];
        for (i=0;i<ncomps;i++)
        {
            Sds__get_struct_ext(origin,origin+ptr[5+nitems+i],
                                length,data,actlen,status);
        }     
    } 
    else
    {

/*  If it is a primitive get the data, copying it to the output buffer
    after the current value of actlen bytes - then increment actlen.
    Ensure correct alignment by calling Sds__align to adjust actlen to
    give the correct alignment for the current type.   */ 

        Sds__align(actlen,code,(char*)data);
        if (*actlen > length)
        {
            *actlen = length;
            return;
        }           
        Sds__get_ext(origin,ptr,length-*actlen,(long)0,
                     (void*)(((char*)data)+(*actlen)),&ilen,status);
        if (*status != SDS__OK) return;
        *actlen += ilen;
        if (*actlen >= length) return;
    }
}






/*+				  S d s G e t 

 *  Function name:
      SdsGet

 *  Function:
      Read the data from an object

 *  Description:
      The object may be a primitive item or a structure or structure array.
      Read the data from an item into a buffer. If the object is primitive 
      data is transferred starting at the position in the item specified by 
      offset, until the buffer if filled, or the end of the data array is 
      reached.

      If the object is a structure or structure array, the data from all its
      primitive components are copied into the buffer in order of their
      position in the structure. Alignment adjustments are made as necessary 
      to match the alignment of an C struct equivalent to the SDS structure.
      (Since these alignment requirements are machine dependent the actual
      sequence of bytes returned could be different on different machines).
      In the structure or structure array case the offset parameter is
      ignored.

      If necessary (e.g. if the data originated on a machine with different
      architecture) the data for the object is converted (in place) from the 
      format stored in the data item to that required for the local machine.
      
 *  Language:
      C

 *  Declaration:
      void SdsGet(SdsIdType id, unsigned long length, unsigned long offset, 
             void *data, unsigned long *actlen, StatusType * SDSCONST status)
      
 *  Parameters:   (">" input, "!" modified, "W" workspace, "<" output)
      (>) id        (SdsIdType) Identifier of the object.
      (>) length    (unsigned long)  Length in bytes of the buffer supplied to receive
                            the data.
      (>) offset    (unsigned long)  Offset into the data object at which to start
                            reading data. The offset is measured in units of
                            the size of each individual item in the array -
                            e.g. 4 bytes for an INT or 8 bytes for a DOUBLE.
                            The offset is zero to start at the beginning of the
                            array. This parameter is ignored if the object
                            is a structure or structure array.
      (<) data      (void*) Buffer to receive the data.
      (<) actlen    (unsigned long*) Actual number of bytes transferred.
      (!) status    (StatusType*) Modified status. Possible failure codes are:
                            SDS__BADID => Invalid identifier
                            SDS__UNDEFINED => Data undefined


 *  Support: Jeremy Bailey, {AAO}

 *  Version date: 18-Oct-91
 *-
 */



SDSEXTERN void SdsGet(
    SdsIdType id, 
    unsigned long length, 
    unsigned long offset, 
    void *data, 
    unsigned long *actlen, 
    StatusType * SDSCONST status)

{
    pblock *pb;
    handle block;
    INT32 external;
    INT32 *origin;

    Sds__FSemTake_get_id(id,&block,&external,&origin,status);
    if (*status != SDS__OK) return;
    /*
     * From this point on - must always exit via "Exit" label.  To ensure
     * semaphore is released.
     */
    if (external)
    {
        if (((char*)(block))[0] == SDS_STRUCT ||
            ((char*)(block))[0] == SDS_SARRAY)
        {

/*  External structure or structure array  */

            *actlen = 0;
            Sds__get_struct_ext(origin,(INT32*)(block),length,
                                data,actlen,status);
            goto Exit;
        } 

/*  External Primitive  */

        Sds__get_ext(origin,(INT32*)(block),length,offset,
                     data,actlen,status);
        goto Exit;
    }
    pb = (pblock*)(*block);
    if (pb->code == SDS_STRUCT || pb->code == SDS_SARRAY) 
    {

/*  Internal structure or structure array  */

        *actlen = 0;
        Sds__get_struct((sblock*)pb,length,data,actlen,status);
        goto Exit;
    }

/*  Internal primitive  */

    Sds__get_int(pb,length,offset,data,actlen,status);
Exit:
    Sds___FSemGive();


}



       

/*
 *  Handle external primitive case of the SdsPut function
 */
static void Sds__put_ext(
    INT32 *origin, 
    INT32 *ptr, 
    unsigned long length, 
    unsigned long offset, 
    SDSCONST void *data, 
    unsigned long *actlen, 
    StatusType * SDSCONST status)
{
    int i;
    long code;
    long format;
    long ndims;
    unsigned long dims[SDS_C_MAXARRAYDIMS];
    INT32 dptr;

    Sds___FSemCheckTaken();  /* If we are using the Full semaphore, it
                              * should be taken at this point */


    code = ((char*)ptr)[0];
    if (code == SDS_STRUCT) 
    {
        *status = SDS__NOTPRIM;
        return;
    }
/*
 * If the data is undefined return an error
 */
    dptr = ptr[5];
    if (dptr == 0)
    {
        *status = SDS__UNDEFINED;
        return;
    }
/*
 *  Calculate length of the data
 */

    ndims = ((short*)ptr)[1];
    *actlen = nbytes[code];
    for(i=0;i<ndims;i++) 
    {
        dims[i] = ptr[6+i];
        *actlen = (*actlen)*dims[i];
    }
  
    if (length > (*actlen-offset*nbytes[code]))
    {
        length = *actlen-offset*nbytes[code];
    }

/*
 *  Convert the data if necessary
 */
    format = ((char*)ptr)[1];
    if ((format != local_format[code]) && (length < *actlen)) 
    {
        Sds__convert(code,format,origin+dptr,*actlen);
        ((char*)ptr)[1] = (char) local_format[code];
    }

/*
 *  Copy the data
 */
    if (ndims == 0)
    {
        if (length == 4) 
            *((INT32*)(origin+dptr)+offset) = *(INT32*)data;
        else if (length == 8)
        {
            *((INT32*)(origin+dptr)+2*offset) = *(INT32*)data;
            *((INT32*)(origin+dptr)+2*offset+1) = *((INT32*)data+1);
        }
        else if (length == 2)
            *((short*)(origin+dptr)+offset) = *(short*)data;
        else if (length == 1)
            *((char*)(origin+dptr)+offset) = *(char*)data;
    }
    else
        memcpy(((char*)(origin+dptr)+offset*nbytes[code]),data,length);
    *actlen = length;
}



static void Sds__put_int(
    pblock *pb, 
    unsigned long length, 
    unsigned long offset,
    SDSCONST void *data, 
    unsigned long *actlen, 
    StatusType * SDSCONST status)
       

{
    int i;
    void *Sdsdata;

    Sds___FSemCheckTaken();  /* If we are using the Full semaphore, it
                              * should be taken at this point */


    *actlen = nbytes[(int)pb->code];
    for(i=0;i<(pb->ndims);i++)  *actlen = (*actlen)*(pb->dims[i]);
    if (length > (*actlen-offset*nbytes[(int)pb->code]))
    {
        length = *actlen-offset*nbytes[(int)pb->code];
    }

/*
 * If the data is undefined create it
 */
    if (pb->data == NULL)
    {

/* Create the data block  */

        Sdsdata = (void*)SdsMalloc(*actlen);
        if (Sdsdata == NULL) 
        {
            *status = SDS__NOMEM;
            return;
        }

/* Create a handle to it */

        pb->data = Sds__get_handle();
        if (pb->data == NULL) 
        {
            *status = SDS__NOMEM;
            return;
        }

/* Set handle to point to data */

        *(pb->data) = Sdsdata;
        pb->format = (char) local_format[(int)pb->code];
    }

/* If the data was previously defined, and the format is not the
   local format and we are not writing all the data, then the
   data have to be converted to the local format  */

    else if ((pb->format != local_format[(int)pb->code]) &&
             (length < *actlen))
    {
        Sds__convert((long)(pb->code),(long)(pb->format),
                     (INT32*)(*(pb->data)),*actlen);
        pb->format = (char) local_format[(int)pb->code];
    }
/*
 *  Copy the data
 */
    if (pb->ndims == 0)
    {
        if (length == 4) 
            *((INT32*)(*(pb->data))+offset) = *(INT32*)data;
        else if (length == 8)
        {
            *((INT32*)(*(pb->data))+2*offset) = *(INT32*)data;
            *((INT32*)(*(pb->data))+2*offset+1) = *((INT32*)data+1);
        } 
        else if (length == 2)
            *((short*)(*(pb->data))+offset) = *(short*)data;
        else if (length == 1)
            *((char*)(*(pb->data))+offset) = *(char*)data;
    }
    else
        memcpy(((char*)*(pb->data)+offset*nbytes[(int)pb->code]),data,length);
    *actlen = length;
}
       


/*
 *  Handle the internal structure case of the SdsPut function
 */
static void Sds__put_struct(
    sblock* sb, 
    unsigned long length, 
    SDSCONST void *data, 
    unsigned long *pos, 
    StatusType * SDSCONST status)
{
    int i;
    int ncomps;
    pblock *pb;
    handle *hblock;
    unsigned long ilen;

    Sds___FSemCheckTaken();  /* If we are using the Full semaphore, it
                              * should be taken at this point */

    if (sb->code == SDS_STRUCT)
    {

/*  If it is a structure put all the component of the structure in sequence  */

        for (i=0;i<sb->nitems;i++)
            Sds__put_struct((sblock*)(*(sb->item[i])),length,data,pos,status);
    }
    else if (sb->code == SDS_SARRAY)
    {

/*  If it is a structure array put all the components in sequence  */

        pb = (pblock*)sb;
        hblock = (handle*)(pb->data);
        ncomps = 1;
        for (i=0;i<pb->ndims;i++) ncomps = ncomps*pb->dims[i];
        for (i=0;i<ncomps;i++)
            Sds__put_struct((sblock*)(*(hblock[i])),length,data,pos,status);
    } 
    else
    {

/*  If it is a primitive put the data, copying it from the input buffer
    after the current value of actlen bytes - then increment pos.
    Ensure correct alignment by calling Sds__align to adjust pos to
    give the correct alignment for the current type.   */ 

        pb = (pblock*)sb;
        Sds__align(pos,pb->code,(char*)data);
        if (*pos > length) return;
        Sds__put_int(pb,length-*pos,(long)0,(void*)(((char*)data)+(*pos)),
                     &ilen,status);
        if (*status != SDS__OK) return;
        *pos += ilen;
        if (*pos >= length) return;
    }
}

/*
 *  Handle the external structure case of the SdsPut function
 */
static void Sds__put_struct_ext(
    INT32 *origin, 
    INT32 *ptr, 
    unsigned long length, 
    SDSCONST void *data, 
    unsigned long *pos, 
    StatusType * SDSCONST status)
{
    int i;
    int ncomps;
    int nitems;
    unsigned long ilen;
    long code;

    Sds___FSemCheckTaken();  /* If we are using the Full semaphore, it
                              * should be taken at this point */

    code = ((char*)ptr)[0];
    nitems = ((short*)ptr)[1];
    if (code == SDS_STRUCT)
    {

/*  If it is a structure get all the component of the structure in sequence  */

        for (i=0;i<nitems;i++)
            Sds__put_struct_ext(origin,origin+ptr[5+i],length,
                                data,pos,status);
    }
    else if (code == SDS_SARRAY)
    {

/*  If it is a structure array get all the components in sequence  */

        ncomps = 1;
        for (i=0;i<nitems;i++) ncomps = ncomps*ptr[5+i];
        for (i=0;i<ncomps;i++)
            Sds__put_struct_ext(origin,origin+ptr[5+nitems+i],
                                length,data,pos,status);
    } 
    else
    {

/*  If it is a primitive get the data, copying it from the input buffer
    after the current value of pos bytes - then increment pos.
    Ensure correct alignment by calling Sds__align to adjust pos to
    give the correct alignment for the current type.   */ 

        Sds__align(pos,code,(char*)data);
        if (*pos > length) return;
        Sds__put_ext(origin,ptr,length-*pos,(long)0,
                     (void*)(((char*)data)+(*pos)),&ilen,status);
        if (*status != SDS__OK) return;
        *pos += ilen;
        if (*pos >= length) return;
    }
}





/*+				S d s P u t

 *  Function name:
      SdsPut

 *  Function:
      Write data to an object.

 *  Description:
      Write data into an object. The object may be a primitive item or a
      structure or structure array.

      If the object is a structure or structure array, the data from the
      the buffer is copied into its primitive components in order of their
      position in the structure. Alignment adjustments are made as necessary 
      to match the alignment of a C struct equivalent to the SDS structure.
      In the structure or structure array case the offset parameter is
      ignored.

      If the object is primitive data is transferred starting
      at the position in the item specified by offset. 

      If the data was previously undefined memory for the data is allocated at
      this time.

      If the data is too long to fit into the object, it will be truncated.
      
 *  Language:
      C

 *  Declaration:
      void SdsPut(SdsIdType id, unsigned long length, unsigned long offset, 
             void *data, long *status)
      
 *  Parameters:   (">" input, "!" modified, "W" workspace, "<" output)
      (>) id        (SdsIdType) Identifier of the primitive item.
      (>) length    (unsigned long)  Length in bytes of the buffer containing
                            the data.
      (>) offset    (unsigned long)  Offset into the data object at which to start
                            writing data. The offset is measured in units of
                            the size of each individual item in the array -
                            e.g. 4 bytes for an INT or 8 bytes for a DOUBLE.
                            The offset is zero to start at the beginning of the
                            array.
      (>) data      (void*) Buffer containing the data.
      (!) status    (StatusType*) Modified status. Possible failure codes are:
                            SDS__NOMEM => Insufficient memory for creation
                            SDS__NOTPRIM => Not a primitive item
                            SDS__UNDEFINED => Data undefined, and object external


 *  Support: Jeremy Bailey, {AAO}

 *  Version date: 18-Oct-91
 *-
 */

SDSEXTERN void SdsPut(
    SdsIdType id, 
    unsigned long length, 
    unsigned long offset, 
    SDSCONST void *data, 
    StatusType * SDSCONST status)

{
    pblock *pb;
    unsigned long pos;
    handle block;
    INT32 external;
    INT32 *origin;

    /* Take semaphore and access id */
    Sds__FSemTake_get_id(id,&block,&external,&origin,status);
    if (*status != SDS__OK) return;
    /*
     * From this point on - must always exit via "Exit" label.  To ensure
     * semaphore is released.
     */
    if (external)
    {
        if (((char*)(block))[0] == SDS_STRUCT ||
            ((char*)(block))[0] == SDS_SARRAY)
        {

/*  External structure or structure array  */

            pos = 0;
            Sds__put_struct_ext(origin,(INT32*)(block),length,
                                data,&pos,status);
            goto Exit;
        } 

/*  External Primitive   */

        Sds__put_ext(origin,(INT32*)(block),length,offset,
                     data,&pos,status);
        goto Exit;
    }
    pb = (pblock*)(*block);
    if ((pb->code == SDS_STRUCT) ||
        (pb->code == SDS_SARRAY)) 
    {
        pos = 0;
        Sds__put_struct((sblock*)pb,length,data,&pos,status);
        goto Exit;
    }
    pos = 0;
    Sds__put_int(pb,length,offset,data,&pos,status);

Exit:
    Sds___FSemGive();
}


static void Sds__cell_ext(
    INT32 *origin, 
    INT32 *data, 
    unsigned long nindices, 
    SDSCONST unsigned long *indices, 
    SdsIdType *id, 
    StatusType * SDSCONST status)

{
    idblock *idptr;
    unsigned long size,addr;
    unsigned long i;
    unsigned long ndims;
    long pos;
    unsigned long dims[SDS_C_MAXARRAYDIMS];

    Sds___FSemCheckTaken();  /* If we are using the Full semaphore, it
                              * should be taken at this point */
/*
 *  Check that input is a structure array
 */
    if (((char*)data)[0] != SDS_SARRAY)
    {
        *status = SDS__NOTARRAY;
        return;
    }
/*
 *  Check that the indices are OK
 */
    ndims = ((short*)data)[1];
    pos = 5;
    if ((nindices != ndims) && 
        (nindices != 1))
    {
        *status = SDS__INDEXERR;
        return;
    }
    if (nindices != 1)
    {  
        for (i=0;i<nindices;i++) 
        {
            dims[i] = data[pos];
            pos++;
            if ((indices[i] > dims[i]) || (indices[i] == 0))
            {
                *status = SDS__INDEXERR;
                return;
            }
        }
    }
    else
    {
        size = 1;
        for (i=0;i<ndims;i++)
        {
            size = size * data[pos];
            pos++;
        }  
        dims[0] = size;  /* Addition by NPR@JACH, inserted by TJF 2-Apr-04 */
        if ((indices[0] > size) || (indices[0] == 0))
        {
            *status = SDS__INDEXERR;
            return;
        }
    }
    size = 1;
    addr = 0;
    for (i=0;i<nindices;i++)
    {
        addr = addr+size*(indices[i]-1);
        size = size*dims[i];
    }
    Sds__get_new_id(&idptr, id, status);
    Sds__put_id(*id,(handle)(origin + data[pos+addr]),TRUE,origin,status);

}
  

/*+				 S d s C e l l

 *  Function name:
      SdsCell

 *  Function:
      Find component of a structure array

 *  Description:
      Given the indices to a component of a structure array, return an
      identifier to the component.
      
 *  Language:
      C

 *  Declaration:
      void SdsCell(SdsIdType array_id, long nindices, unsigned long *indices, 
               SdsIdType *id, StatusType * SDSCONST status)
      
 *  Parameters:   (">" input, "!" modified, "W" workspace, "<" output)
      (>) array_id  (SdsIdType) Identifier of the structure array.
      (>) nindices  (long)  Number of indices supplied in the
                            array indices. This should be one or the same as
                            the number of dimensions of the array.
      (>) indices   (unsigned long*)  An array of length nindices containing 
                            the indices to the component of the structure
                            array. If nindicies is 1, then treat the 
                            structure array as having only one dimension even
                            if it has more.
      (<) id        (SdsIdType*) The identifier of the component.
      (!) status    (StatusType*) Modified status. Possible failure codes are:
                            SDS__NOMEM => Insufficient memory for creation
                            SDS__NOTARRAY => Not a structure array.
                            SDS__INDEXERR => Indices invalid.


 *  Support: Jeremy Bailey, {AAO}

 *  Version date: 18-Oct-91
 *-
 */



SDSEXTERN void SdsCell(SdsIdType array_id, long nindices, 
	     SDSCONST unsigned long *indices, 
             SdsIdType *id, StatusType * SDSCONST status)

{
    pblock *pb;
    handle *hblock;
    idblock *idptr;
    unsigned long size,addr;
    long i;
    handle block;
    INT32 external;
    INT32 *origin;

    /* Take semaphore and access id */
    Sds__FSemTake_get_id(array_id,&block,&external,&origin,status);
    if (*status != SDS__OK) return;
    /*
     * From this point on - must always exit via "Exit" label.  To ensure
     * semaphore is released.
     */
    if (external)
    {
        Sds__cell_ext(origin,(INT32*)(block),nindices,
                      indices,id,status);
        goto Exit;
    }
    pb = (pblock*)(*block);
/*
 *  Check that input is a structure array
 */
    if (pb->code != SDS_SARRAY)
    {
        *status = SDS__NOTARRAY;
        goto Exit;
    }
/*
 *  Check that the indices are OK
 */
    if ((nindices != pb->ndims) && (nindices != 1))
    {
        *status = SDS__INDEXERR;
        goto Exit;
    }
    if (nindices != 1)
    {
        for (i=0;i<nindices;i++) 
        {
            if ((indices[i] > pb->dims[i]) || (indices[i] == 0))
            {
                *status = SDS__INDEXERR;
                goto Exit;
            }
        }
    }
    else
    {
        size = 1;
        for (i=0;i<(pb->ndims);i++) size = size * pb->dims[i];
        if ((indices[0] > size) || (indices[0] == 0))
        {
            *status = SDS__INDEXERR;
            goto Exit;
        }
    }
    size = 1;
    addr = 0;
    for (i=0;i<nindices;i++)
    {
        addr = addr+size*(indices[i]-1);
        size = size*pb->dims[i];
    }
    hblock = (handle*)(pb->data);
    Sds__get_new_id(&idptr, id, status);
    Sds__put_id(*id,hblock[addr],FALSE,origin,status);

Exit:
    Sds___FSemGive();

}
  


/*+				 S d s I n s e r t C e l l

 *  Function name:
      SdsInsertCell

 *  Function:
      Insert object into a structure array

 *  Description:
      Insert a top level object into a structure array at a position specified by its
      indices. Delete any object that is currently at that position.
      
 *  Language:
      C

 *  Declaration:
      void SdsInsertCell(SdsIdType array_id, long nindices, unsigned long *indices, 
               SdsIdType id, StatusType * SDSCONST status)
      
 *  Parameters:   (">" input, "!" modified, "W" workspace, "<" output)
      (>) array_id  (SdsIdType) Identifier of the structure array.
      (>) nindices  (long)  Number of indices supplied in the
                            array indices. This should be 1 or the same as
                            the number of dimensions of the array.
      (>) indices   (unsigned long*)  An array of length nindices containing 
                            the indices to the component of the structure
                            array.  If nindicies is 1, then treat the 
			    structure array as having only one dimension even
			    if it has more.
      (>) id        (SdsIdType) The identifier of the component to be inserted.
      (!) status    (StatusType*) Modified status. Possible failure codes are:
                            SDS__EXTERN => Structure array or object is external.
                            SDS__NOTARRAY => Not a structure array.
                            SDS__INDEXERR => Indices invalid.
                            SDS__NOTTOP => Not a top level object.


 *  Support: Jeremy Bailey, {AAO}

 *  Version date: 24-Aug-96
 *-
 */

static void Sds__delete(sblock *sb);
static void Sds__invalidateIdsFor(handle sb);

SDSEXTERN void SdsInsertCell(
    SdsIdType array_id, 
    long nindices, 
    SDSCONST unsigned long *indices, 
    SdsIdType id, 
    StatusType * SDSCONST status)

{
    pblock *pb;
    sblock *sb;
    handle *hblock;
    unsigned long size,addr;
    long i;
    handle block;
    INT32 external;
    INT32 *origin;

    /* Take semaphore and access id */
    Sds__FSemTake_get_id(array_id,&block,&external,&origin,status);
    if (*status != SDS__OK) return;
    /*
     * From this point on - must always exit via "Exit" label.  To ensure
     * semaphore is released.
     */
    Sds___WatchEvent(id, SDS_WATCH_EVENT_INSERT_CELL, "SdsInsertCell");
    if (external)
    {
        *status = SDS__EXTERN;
        goto Exit;
    }
    pb = (pblock*)(*block);
/*
 *  Check that input is a structure array
 */
    if (pb->code != SDS_SARRAY)
    {
        *status = SDS__NOTARRAY;
        goto Exit;
    }
/*
 *  Check that the indices are OK
 */
    if ((nindices != pb->ndims) && (nindices != 1))
    {
        *status = SDS__INDEXERR;
        goto Exit;
    }
    if (nindices != 1)
    {  
        for (i=0;i<nindices;i++) 
        {
            if ((indices[i] > pb->dims[i]) || (indices[i] == 0))
            {
                *status = SDS__INDEXERR;
                goto Exit;
            }
        }
    }
    else
    {
        size = 1;
        for (i=0;i<(pb->ndims);i++) size = size * pb->dims[i];
        if ((indices[0] > size) || (indices[0] == 0))
        {
            *status = SDS__INDEXERR;
            goto Exit;
        }
    }
    size = 1;
    addr = 0;
    for (i=0;i<nindices;i++)
    {
        addr = addr+size*(indices[i]-1);
        size = size*pb->dims[i];
    }
    hblock = (handle*)(pb->data);
    Sds__get_id(id,&block,&external,&origin,status);
    if (*status != SDS__OK) goto Exit;
    if (external)
    {
        *status = SDS__EXTERN;
        goto Exit;
    }
    sb = (sblock*)(*block);
    if (sb->parent != NULL)
    {
        *status = SDS__NOTTOP;
        goto Exit;
    }
    if (hblock[addr] != NULL)
    {
        /*
         * Delete the block and then ensure any ID's
         * referring to it are invalided
         */
        Sds__delete(*hblock[addr]);
        Sds__invalidateIdsFor(hblock[addr]);

    }   
    hblock[addr] = block;
    sb->parent = pb->self;

Exit:
    Sds___FSemGive();

}
  

static void Sds__delete(sblock *sb)
   
{
    pblock *pb;
    long i;
    long ncomps;
    handle *hblock;

    Sds___FSemCheckTaken();  /* If we are using the Full semaphore, it
                              * should be taken at this point */
    if (sb->code == SDS_STRUCT)
    {
        for (i=0;i<sb->nitems;i++)
        {
            /*
             * Delete the block and then ensure any ID's
             * referring to it are invalided
             */
            Sds__delete((sblock*)(*(sb->item[i])));
            Sds__invalidateIdsFor(sb->item[i]);
        }
        if (sb->self)
        {
            SdsFree(sb->self);
            sb->self = NULL;
        } 
        SdsFree(sb);
    }
    else if (sb->code == SDS_SARRAY)
    {
        pb = (pblock*)sb;
        hblock = (handle*)(pb->data);
        ncomps = 1;
        for (i=0;i<pb->ndims;i++) ncomps = ncomps*pb->dims[i];
        for (i=0;i<ncomps;i++)
        {
            /*
             * Delete the block and then ensure any ID's
             * referring to it are invalided
             */
            Sds__delete((sblock*)(*(hblock[i])));
            Sds__invalidateIdsFor(hblock[i]);
        }
        if (pb->self)
            SdsFree(pb->self); 
        pb->self = NULL;
        SdsFree(hblock);
        SdsFree(sb);
    }
    else
    {
        pb = (pblock*)sb;
        if (pb->data != NULL)
        {
            SdsFree (*(pb->data));
            *pb->data = NULL; /* Shouldn't really be needed due to next
                               * line, but just to be sure */
            SdsFree (pb->data);
            pb->data = NULL;
        }
        if (pb->self)
            SdsFree(pb->self);  
        pb->self = NULL;
        SdsFree(pb);
    }
}       

/*
 * Find all IDs referring to a given block and invalidate them.
 *
 * This method is invoked by callers of Sds__delete().  It must ensure
 * that given a particular block has been deleted, that all ID's that
 * refer to that block are invalidated by pointing them to the null handle.
 * 
 */
static void Sds__invalidateIdsFor(handle block)
{
    SdsIdType id;
    for (id = 1; id <= id_number ; ++id)
    {       
        if (idarrayptr[id].block == block)
        {
            idarrayptr[id].block = &null_handle;
            idarrayptr[id].external = FALSE;
            idarrayptr[id].origin = NULL;
            idarrayptr[id].free = 0;
        }
    }
  
}



/*+				S d s D e l e t e

 *  Function name:
      SdsDelete

 *  Function:
      Delete an object

 *  Description:
      Delete an object, freeing any memory associated with it. Subsequent
      attempts to access the object through any identifier associated with
      it will return the SDS__BADID status. A structure array element cannot 
      be deleted. An attempt to do so will result in the SDS__ILLDEL status.

     Deleting an object does not free the memory associated with the
     identifier referencing it. This memory can be freed with the 
     SdsFreeId function.

      
 *  Language:
      C

 *  Declaration:
      void SdsDelete(SdsIdType id, long *status)
      
 *  Parameters:   (">" input, "!" modified, "W" workspace, "<" output)
      (>) id        (SdsIdType)  Identifier of the object to be deleted.
      (!) status    (StatusType*) Modified status. Possible failure codes are:
                            SDS__BADID => The identifier is invalid.
                            SDS__EXTERN => Object is external.
                            SDS__ILLDEL => Object cannot be deleted.


 *  Support: Jeremy Bailey, {AAO}

 *  Version date: 18-Oct-91
 *-
 */

SDSEXTERN void SdsDelete(SdsIdType id, StatusType * SDSCONST status)

{
    sblock *sb;
    sblock *parent;
    long index;
    handle block;
    INT32 external;
    INT32 *origin;

    /* Take semaphore and access id */
    Sds__FSemTake_get_id(id,&block,&external,&origin,status);
    if (*status != SDS__OK) return;
    /*
     * From this point on - must always exit via "Exit" label.  To ensure
     * semaphore is released.
     */
    Sds___WatchEvent(id, SDS_WATCH_EVENT_DELETE, "SdsDelete");
    if (external)
    {
        *status = SDS__EXTERN;
        goto Exit;
    }
    sb = (sblock*)(*block);
    if ((sb->parent) != NULL)
    {

/*  Find the parent of the object to be deleted  */

        parent = (sblock*)(*(sb->parent));
        if ((parent->nitems == 0) || (parent->code == SDS_SARRAY))
        {
            *status = SDS__ILLDEL;
            goto Exit;
        }
        index = 0;

/*  Loop through the parents item list to find the item to be deleted  */

        while (*(parent->item[index]) != sb)
        {
            index++;
            if (index >= (parent->nitems))
            {

/*  Can't find the item - must be a structure array component for
    which deletion is illegal  */

                *status = SDS__ILLDEL;
                goto Exit;
            }
        }
        
/*  Shift up following items in list by one and reduce item count by one */

        /*A change by NPR@JACH so the "-1" added here, merged by TJF,2-Apr-04*/
        for (;index < (parent->nitems-1);index++)
            parent->item[index] = parent->item[index+1];
        (parent->nitems)--;
    }

/*  Now go ahead and delete the object  */

    Sds__delete(sb);

/*  And ensure any ID that points to this is invalided */
    Sds__invalidateIdsFor(block);

Exit:
    Sds___FSemGive();

}


static void Sds__size(
    sblock *sb, 
    UINT32 *defn,   /* Size of definition returned here */
    UINT32 *data,   /* Size of data returned here */ 
    int define)     /* presume undefined is defined ? */

{
    UINT32 tdata;
    UINT32 tdefn;
    pblock *pb;
    int ncomps;
    handle *hblock;
    long i;

    Sds___FSemCheckTaken();  /* If we are using the Full semaphore, it
                              * should be taken at this point */
    if (sb->code == SDS_STRUCT)
    {
        *data=0;
        *defn=22 + 4*sb->nitems + sb->nextra;
        for (i=0;i<sb->nitems;i++)
        {
            Sds__size((sblock*)(*(sb->item[i])),&tdefn,&tdata,define);
            *data += tdata;
            *defn += tdefn;
        }
    }
    else if (sb->code == SDS_SARRAY)
    {
        pb = (pblock*)sb;
        hblock = (handle*)(pb->data);
        ncomps=1;
        for (i=0;i<pb->ndims;i++) ncomps = ncomps*pb->dims[i];
        *defn = 22+4*pb->ndims+pb->nextra+4*ncomps;
        *data = 0;
        for (i=0;i<ncomps;i++)
        {
            Sds__size((sblock*)(*(hblock[i])),&tdefn,&tdata,define);
            *data += tdata;
            *defn += tdefn;
        }
    }
    else
    {
        pb = (pblock*)sb;
        *defn = 26 + 4*pb->ndims + pb->nextra;
        ncomps = 1;
        for (i=0;i<pb->ndims;i++) ncomps = ncomps*pb->dims[i];
        if ((pb->data == NULL) && (!define))
            *data = 0;
        else
        {
            *data = ncomps*nbytes[(int)pb->code];
            if ((pb->code == SDS_DOUBLE) || (pb->code == SDS_I64) ||
                (pb->code == SDS_UI64)) *data += 4;
        }
    }
    while ((*defn % 4) != 0) (*defn)++;
    while ((*data % 4) != 0) (*data)++;
}


static void Sds__size_ext(
    INT32 *origin, 
    INT32 *edata, 
    UINT32 *defn,   /* Size of definition returned here */
    UINT32 *data,   /* Size of data returned here */
    int define)    /* presume defined ? */

{
    UINT32 tdata;
    UINT32 tdefn;
    int ncomps;
    char code;
    int i;
    int nextra;
    int nitems;
    INT32 pos;

    Sds___FSemCheckTaken();  /* If we are using the Full semaphore, it
                              * should be taken at this point */
    code = ((char*)edata)[0];
    nitems = ((short*)edata)[1];
    if (code == SDS_STRUCT)
    {
        nextra = ((short*)edata)[10+2*nitems];
        *data=0;
        *defn=22 + 4*nitems + nextra;
        for (i=0;i<nitems;i++)
        {
            Sds__size_ext(origin,origin+(edata[5+i]),&tdefn,&tdata,define);
            *data += tdata;
            *defn += tdefn;
        }
    }
    else if (code == SDS_SARRAY)
    {
        ncomps=1;
        pos = 5;
        for (i=0;i<nitems;i++) 
        {
            ncomps = ncomps*edata[pos];
            pos++;
        }
        nextra = ((short*)edata)[2*(pos+ncomps)];
        *defn = 22+4*nitems+nextra+4*ncomps;
        *data = 0;
        for (i=0;i<ncomps;i++)
        {
            Sds__size_ext(origin,origin+(edata[5+nitems+i]),&tdefn,&tdata,define);
            *data += tdata;
            *defn += tdefn;
        }
    }
    else
    {
        ncomps = 1;
        pos = 6;
        for (i=0;i<nitems;i++) 
        {
            ncomps = ncomps*edata[pos];
            pos++;
        }
        nextra = ((short*)edata)[2*pos];
        *defn = 26 + 4*nitems + nextra;
        if ((edata[5] == 0) && (!define))
            *data = 0;
        else
        {
            *data = ncomps*nbytes[(int)code];
            if ((code == SDS_DOUBLE) || (code == SDS_I64) ||
                (code == SDS_UI64)) *data += 4;
        }
    }
    while ((*defn % 4) != 0) (*defn)++;
    while ((*data % 4) != 0) (*data)++;
}


/*+				  S d s S i z e

 *  Function name:
      SdsSize

 *  Function:
      Find the buffer size needed to export an object

 *  Description:
      Return the size which will be needed for a buffer into which the object
      can be exported using the SdsExport function.
      
 *  Language:
      C

 *  Declaration:
      void SdsSize(SdsIdType id, unsigned long *bytes, StatusType * SDSCONST status)
      
 *  Parameters:   (">" input, "!" modified, "W" workspace, "<" output)
      (>) id        (SdsIdType)  Identifier of the object.
      (<) bytes     (unsigned long*) Size in bytes of required buffer.
      (!) status    (StatusType*) Modified status. Possible failure codes are:
                            SDS__BADID => The identifier is invalid.
                            SDS__EXTERN => Object is external.

 *  Support: Jeremy Bailey, {AAO}

 *  Version date: 18-Oct-91
 *-
 */


SDSEXTERN void SdsSize(SdsIdType id, unsigned long *bytes, StatusType * SDSCONST status)

{
    sblock *sb;
    UINT32 data;
    UINT32 defn;
    handle block;
    INT32 external;
    INT32 *origin;

    /* Take semaphore and access id */
    Sds__FSemTake_get_id(id,&block,&external,&origin,status);
    if (*status != SDS__OK) return;
    /*
     * From this point on - must release semaphore before exiting.
     */
    if (external)
    {
        if ((INT32*)block == origin+4)
        {
            *bytes = ((INT32*)origin)[1];
        }
        else
        {
            Sds__size_ext(origin,(INT32*)(block),&defn,&data,0);
            *bytes = 16+data+defn;
        }
    }
    else
    {
        sb = (sblock*)(*block);
        Sds__size(sb,&defn,&data,0);
        *bytes = 16+data+defn;
    }

    Sds___FSemGive();

}


/*+	                 S d s S i z e D e f i n e d

 *  Function name:
      SdsSizeDefined

 *  Function:
      Find the buffer size needed to export using SdsExportDefined

 *  Description:
      Return the size which will be needed for a buffer into which the object
      can be exported using the SdsExportDefined function.
      
 *  Language:
      C

 *  Declaration:
      void SdsSizeDefined(SdsIdType id, unsigned long *bytes, StatusType * SDSCONST status)
      
 *  Parameters:   (">" input, "!" modified, "W" workspace, "<" output)
      (>) id        (SdsIdType)  Identifier of the object.
      (<) bytes     (unsigned long*) Size in bytes of required buffer.
      (!) status    (StatusType*) Modified status. Possible failure codes are:
                            SDS__BADID => The identifier is invalid.
                            SDS__EXTERN => Object is external.

 *  Support: Jeremy Bailey, {AAO}

 *  Version date: 13-Jul-98
 *-
 */


SDSEXTERN void SdsSizeDefined(SdsIdType id, unsigned long *bytes, StatusType * SDSCONST status)

{
    sblock *sb;
    UINT32 data;
    UINT32 defn;
    handle block;
    INT32 external;
    INT32 *origin;

    /* Take semaphore and access id */
    Sds__FSemTake_get_id(id,&block,&external,&origin,status);
    if (*status != SDS__OK) return;
    /*
     * From this point on - must release semaphore before exiting.
     */
    if (external)
    {
        if ((INT32*)block == origin+4)
        {
            *bytes = ((INT32*)origin)[1];
        }
        else
        {
            Sds__size_ext(origin,(INT32*)(block),&defn,&data,1);
            *bytes = 16+data+defn;
        }
    }
    else
    {
        sb = (sblock*)(*block);
        Sds__size(sb,&defn,&data,1);
        *bytes = 16+data+defn;
    }

    Sds___FSemGive();


}


static void Sds__export_ext(
    INT32 *origin, INT32 *edata, 
    INT32 *data, INT32 *defn_pos,
    INT32 *data_pos, int define)

{
    /*INT32 tdata;*/
    /*INT32 tdefn;*/
    int ncomps;
    char code;
    long i;
    int nextra;
    int nitems;
    INT32 pos;
    unsigned long size;
    INT32 temp_pos;

    Sds___FSemCheckTaken();  /* If we are using the Full semaphore, it
                              * should be taken at this point */
    code = ((char*)edata)[0];
    nitems = ((short*)edata)[1];
    if (code == SDS_STRUCT)
    {
        nextra = ((short*)edata)[10+2*nitems];
        temp_pos = *defn_pos+5;
        size = 5 + nitems + (2+nextra)/4;
        if (((2+nextra) % 4) != 0) size++;
        memcpy(((char*)data)+(4*(*defn_pos)),((char*)edata),4*size);
        *defn_pos = *defn_pos + size;
        for (i=0;i<nitems;i++)
        {
            data[temp_pos] = *defn_pos;
            temp_pos++;
            Sds__export_ext(origin,origin+(edata[5+i]),data,
                            defn_pos,data_pos,define);
        }
    }
    else if (code == SDS_SARRAY)
    {
        ncomps=1;
        pos = 5;
        for (i=0;i<nitems;i++) 
        {
            ncomps = ncomps*edata[pos];
            pos++;
        }
        temp_pos = *defn_pos+5+nitems;
        nextra = ((short*)edata)[2*(pos+ncomps)];
        size = 5+nitems+(2+nextra)/4+ncomps;
        if (((2+nextra) % 4) != 0) size++;
        memcpy(((char*)data)+(4*(*defn_pos)),((char*)edata),4*size);
        *defn_pos = *defn_pos + size;
        for (i=0;i<ncomps;i++)
        {
            data[temp_pos] = *defn_pos;
            temp_pos++;
            Sds__export_ext(origin,origin+(edata[5+nitems+i]),data,defn_pos,
                            data_pos,define);
        }
    }
    else
    {
        ncomps = 1;
        pos = 6;
        temp_pos = *defn_pos+5;
        for (i=0;i<nitems;i++) 
        {
            ncomps = ncomps*edata[pos];
            pos++;
        }
        nextra = ((short*)edata)[2*pos];
        size = 6 + nitems + (2+nextra)/4;
        if (((2+nextra) % 4) != 0) size++;
        memcpy(((char*)data)+(4*(*defn_pos)),((char*)edata),4*size);
        *defn_pos = *defn_pos + size;
        if ((edata[5] == 0) && (!define))
            data[temp_pos] = 0;
        else
        {
            data[temp_pos] = *data_pos;
            if ((code == SDS_DOUBLE) || (code == SDS_I64) || 
                (code == SDS_UI64)) size += 4;
            size = ncomps*nbytes[(int)code];
            while ((size % 4) != 0) size++;
            if (edata[5] != 0)
                memcpy(((char*)data)+(4*(*data_pos)),((char*)(origin+edata[5])),size);
            *data_pos += size/4;
        }
    }
}



static void Sds__export(
    sblock *sb, 
    INT32 *data, 
    INT32 *defn_pos, 
    INT32 *data_pos,
    int define)

{
    pblock *pb;
    INT32 temp_pos;
    int i;
    int ncomps;
    handle *hblock;
    INT32 size;
    BOOL space;

    Sds___FSemCheckTaken();  /* If we are using the Full semaphore, it
                              * should be taken at this point */

    ((char*)data)[4*(*defn_pos)] = sb->code;
    ((char*)data)[(4*(*defn_pos))+1] = sb->format;
    strcpy(((char*)data)+(4*(*defn_pos))+4,sb->name);
    if (sb->code == SDS_STRUCT)
    {
        ((short*)data)[2*(*defn_pos)+1] = sb->nitems;
        temp_pos = *defn_pos + 5;
        *defn_pos = *defn_pos + 5 + sb->nitems + (2+sb->nextra)/4;
        if (((2+sb->nextra) % 4) != 0) (*defn_pos)++;
        for (i=0;i<sb->nitems;i++)
        {
            data[temp_pos] = *defn_pos;
            temp_pos++;
            Sds__export((sblock*)(*(sb->item[i])),data,defn_pos,data_pos,define);
        }
        ((short*)data)[2*temp_pos] = (short) sb->nextra;
        if (sb->nextra != 0)
            memcpy(((char*)data)+(4*temp_pos)+2,sb->extra,sb->nextra);
    }
    else if (sb->code == SDS_SARRAY)
    {
        pb = (pblock*)sb;
        ((short*)data)[2*(*defn_pos)+1] = pb->ndims;
        *defn_pos = *defn_pos + 5;
        ncomps = 1;
        for (i=0;i<pb->ndims;i++)
        {
            ncomps = ncomps * pb->dims[i];
            data[*defn_pos] = pb->dims[i];
            (*defn_pos)++;
        }
        temp_pos = *defn_pos;
        *defn_pos = *defn_pos + ncomps + (2+pb->nextra)/4;
        if (((2+pb->nextra) % 4) != 0) (*defn_pos)++;
        hblock = (handle*)(pb->data);
        for (i=0;i<ncomps;i++)
        {
            data[temp_pos] = *defn_pos;
            temp_pos++;
            Sds__export((sblock*)(*(hblock[i])),data,defn_pos,data_pos,define);
        }
        ((short*)data)[2*temp_pos] = (short) pb->nextra;
        if (pb->nextra != 0)
            memcpy(((char*)data)+(4*temp_pos)+2,pb->extra,pb->nextra);
    }
    else
    {
        pb = (pblock*)sb;
        ((short*)data)[2*(*defn_pos)+1] = pb->ndims;
        *defn_pos = *defn_pos + 5;
        if ((pb->data == NULL) && (!define))
            data[*defn_pos] = 0;
        temp_pos = *defn_pos;
        (*defn_pos)++;
        ncomps = 1;
        for (i=0;i<pb->ndims;i++)
        {
            ncomps = ncomps * pb->dims[i];
            data[*defn_pos] = pb->dims[i];
            (*defn_pos)++;
        }
        if ((pb->data != NULL) || (define))
        {
            size = nbytes[(int)pb->code] * ncomps;
            space = FALSE;
            if ((pb->code == SDS_DOUBLE) || (pb->code == SDS_I64) ||
                (pb->code == SDS_UI64))
            {
                if (((*data_pos) % 2) != 0)
                { 
                    *data_pos += 1;
                    space = TRUE;
                }
            }
            data[temp_pos] = *data_pos;
            if (pb->data != NULL) 
                memcpy(((char*)data)+((*data_pos)*4),*(pb->data),size);
            *data_pos += size/4;
            if (((pb->code == SDS_DOUBLE) || (pb->code == SDS_I64) ||
                 (pb->code == SDS_UI64)) && (! space))
                *data_pos += 1;
        }
        else
            size = 0;    /* Data is undefined - fix from N.Rees, 17-Jan-2002 */

        if ((size % 4) != 0) (*data_pos)++;
        ((short*)data)[2*(*defn_pos)] = (short) pb->nextra;
        if (pb->nextra != 0)
            memcpy(((char*)data)+(4*(*defn_pos))+2,pb->extra,pb->nextra);
        *defn_pos = *defn_pos + (2+pb->nextra)/4;
        if (((2+pb->nextra) % 4) != 0) (*defn_pos)++;
    }
}
        

/*+				S d s E x p o r t

 *  Function name:
      SdsExport

 *  Function:
      Export an object into an external buffer

 *  Description:
      Export an object into an external buffer.

      Once exported an object can be moved around in memory, written to
      a file etc., and subsequently returned to the SDS system either by using
      SdsImport to import it back into the system, or SdsAccess, to access it
      as an external object.

      The original internal version of the
      object continues to exist, in addition to the external copy. All
      identifiers to the object continue to refer to the original internal
      copy.
      
      With SdsExport, any undefined primitive data items occupy no
      space in the exported item, and cannot be written or read
      until the item is reimported. This enables the creation of
      compact templates for structures which may contain large arrays.
      If this behaviour is not wanted use SdsExportDefined, which allocates
      full space in the external structure for undefined primitive
      items.

      The length of the buffer required for SdsExport can be determined
      by a call to SdsSize.
      
 *  Language:
      C

 *  Declaration:
      void SdsExport(SdsIdType id, unsigned long length,  
                    void *data, StatusType * SDSCONST status)
      
 *  Parameters:   (">" input, "!" modified, "W" workspace, "<" output)
      (>) id        (SdsIdType)  Identifier of the structure to be exported.
      (>) length    (unsigned long) Size in bytes of the buffer.
      (<) data      (void*) The buffer into which the object will be
                          exported.
      (!) status    (StatusType*) Modified status. Possible failure codes are:
                            SDS__BADID => The identifier is invalid.
                            SDS__TOOLONG => The object is too large for the buffer
                            SDS__EXTERN => The object is external.

 *  Support: Jeremy Bailey, {AAO}

 *  Version date: 18-Oct-91
 *-
 */
        

SDSEXTERN void SdsExport(
    SdsIdType id, 
    unsigned long length, 
    void *data, 
    StatusType * SDSCONST status)

{
    INT32 defn_pos;
    INT32 data_pos;
    UINT32 defn_size;
    UINT32 data_size;
    sblock *sb;
    UINT32 size;
    handle block;
    INT32 external;
    INT32 *origin;

    /* Take semaphore and access id */
    Sds__FSemTake_get_id(id,&block,&external,&origin,status);
    if (*status != SDS__OK) return;
    /*
     * From this point on - must always exit via "Exit" label.  To ensure
     * semaphore is released.
     */
    if (external)
    {
        if ((INT32*)block == origin+4)
        {
            size = ((UINT32*)origin)[1];
            if (length < size)
            {
                *status = SDS__TOOLONG;
                goto Exit;
            }
            memcpy((char*)data,(char*)origin,size);
        }
        else 
        {
            Sds__size_ext(origin,((INT32*)block),&defn_size,
                          &data_size,0);
            if (length < (16+defn_size+data_size))
            {
                *status = SDS__TOOLONG;
                goto Exit;
            }
            defn_pos = 0;
            data_pos = 4+defn_size/4;
            if (local_format[SDS_INT] == BIGEND)
                ((INT32*)data)[defn_pos] = 0;
            else
                ((INT32*)data)[defn_pos] = ~0;
            defn_pos++;
            ((INT32*)data)[defn_pos] = 16+defn_size+data_size;
            defn_pos++;
            ((INT32*)data)[defn_pos] = SDS_VERSION;
            defn_pos++;
            ((INT32*)data)[defn_pos] = 16+defn_size;
            defn_pos++;

            Sds__export_ext(origin,(INT32*)(block),
                            (INT32*)data,&defn_pos,&data_pos,0);
        }
        goto Exit;
    }
    sb = (sblock*)(*block);
    Sds__size(sb,&defn_size,&data_size,0);
    if (length < (16+defn_size+data_size))
    {
        *status = SDS__TOOLONG;
        goto Exit;
    }
    defn_pos = 0;
    data_pos = 4+defn_size/4;
    if (local_format[SDS_INT] == BIGEND)
        ((INT32*)data)[defn_pos] = 0;
    else
        ((INT32*)data)[defn_pos] = ~0;
    defn_pos++;
    ((INT32*)data)[defn_pos] = 16+defn_size+data_size;
    defn_pos++;
    ((INT32*)data)[defn_pos] = SDS_VERSION;
    defn_pos++;
    ((INT32*)data)[defn_pos] = 16+defn_size;
    defn_pos++;

    Sds__export(sb,(INT32*)data,&defn_pos,&data_pos,0);

Exit:
    Sds___FSemGive();


}


/*+		       S d s E x p o r t D e f i n e d

 *  Function name:
      SdsExportDefined

 *  Function:
      Export an object into an external buffer 

 *  Description:
      Export an object into an external buffer.

      Once exported an object can be moved around in memory, written to
      a file etc., and subsequently returned to the SDS system either by using
      SdsImport to import it back into the system, or SdsAccess, to access it
      as an external object.

      The original internal version of the
      object continues to exist, in addition to the external copy. All
      identifiers to the object continue to refer to the original internal
      copy.
      
      SdsExportDefined allocates space in the external item for undefined
      data items, so that these can have their values filled in later
      by an SdsPut (or SdsPointer) to the external item.

      The length of the buffer required for SdsExportDefined can be determined
      by a call to SdsSizeDefined.
      
 *  Language:
      C

 *  Declaration:
      void SdsExportDefined(SdsIdType id, unsigned long length,  
                    void *data, StatusType * SDSCONST status)
      
 *  Parameters:   (">" input, "!" modified, "W" workspace, "<" output)
      (>) id        (SdsIdType)  Identifier of the structure to be exported.
      (>) length    (unsigned long) Size in bytes of the buffer.
      (<) data      (void*) The buffer into which the object will be
                          exported.
      (!) status    (StatusType*) Modified status. Possible failure codes are:
                            SDS__BADID => The identifier is invalid.
                            SDS__TOOLONG => The object is too large for the buffer
                            SDS__EXTERN => The object is external.

 *  Support: Jeremy Bailey, {AAO}

 *  Version date: 13-Jul-98
 *-
 */
        

SDSEXTERN void SdsExportDefined(
    SdsIdType id, 
    unsigned long length, 
    void *data, 
    StatusType * SDSCONST status)

{
    INT32 defn_pos;
    INT32 data_pos;
    UINT32 defn_size;
    UINT32 data_size;
    sblock *sb;

    handle block;
    INT32 external;
    INT32 *origin;

    /* Take semaphore and access id */
    Sds__FSemTake_get_id(id,&block,&external,&origin,status);
    if (*status != SDS__OK) return;
    /*
     * From this point on - must always exit via "Exit" label.  To ensure
     * semaphore is released.
     */
    if (external)
    {
        
        Sds__size_ext(origin,((INT32*)block),&defn_size,
                      &data_size,1);
        if (length < (16+defn_size+data_size))
        {
            *status = SDS__TOOLONG;
            goto Exit;
        }
        defn_pos = 0;
        data_pos = 4+defn_size/4;
        if (local_format[SDS_INT] == BIGEND)
            ((INT32*)data)[defn_pos] = 0;
        else
            ((INT32*)data)[defn_pos] = ~0;
        defn_pos++;
        ((INT32*)data)[defn_pos] = 16+defn_size+data_size;
        defn_pos++;
        ((INT32*)data)[defn_pos] = SDS_VERSION;
        defn_pos++;
        ((INT32*)data)[defn_pos] = 16+defn_size;
        defn_pos++;

        Sds__export_ext(origin,(INT32*)(block),
                        (INT32*)data,&defn_pos,&data_pos,1);
        
        goto Exit;
    }
    sb = (sblock*)(*block);
    Sds__size(sb,&defn_size,&data_size,1);
    if (length < (16+defn_size+data_size))
    {
        *status = SDS__TOOLONG;
        goto Exit;
    }
    defn_pos = 0;
    data_pos = 4+defn_size/4;
    if (local_format[SDS_INT] == BIGEND)
        ((INT32*)data)[defn_pos] = 0;
    else
        ((INT32*)data)[defn_pos] = ~0;
    defn_pos++;
    ((INT32*)data)[defn_pos] = 16+defn_size+data_size;
    defn_pos++;
    ((INT32*)data)[defn_pos] = SDS_VERSION;
    defn_pos++;
    ((INT32*)data)[defn_pos] = 16+defn_size;
    defn_pos++;

    Sds__export(sb,(INT32*)data,&defn_pos,&data_pos,1);
Exit:
    Sds___FSemGive();

}



static void Sds__import(
    INT32 *data, 
    INT32 pos, 
    sblock* *sbptr, 
    BOOL bs, 
    StatusType * SDSCONST status)

{
    long code;
    long format;
    int nitems;
    char name[SDS_C_NAMELEN];
    long nextra;
    char extra[MAX_EXTRA];
    sblock *sub;
    handle *hblock;
    int ncomps;
    INT32 temp_pos;
    long i;
    pblock *pb;
    sblock *sb;
    INT32 dptr;
    INT32 size;
    void *Sdsdata;

    Sds___FSemCheckTaken();  /* If we are using the Full semaphore, it
                              * should be taken at this point */
    code = ((char*)data)[4*pos];
    format = ((char*)data)[4*pos+1];
    nitems = swap2(bs,((short*)data)[2*pos+1]);
    strncpy(name,((char*)data)+(4*pos)+4,16);
    pos += 5;

    if (code == SDS_STRUCT)
    {
        temp_pos = pos;
        pos += nitems;
        nextra = swap2(bs,((short*)data)[2*pos]);
        memcpy(extra,((char*)data)+(4*pos)+2,nextra);
        sb = Sds__create_sblock(name,nextra,extra,nitems);
        if (sb == NULL)
        {
            *status = SDS__NOMEM;
            return;
        }
        (sb)->self = Sds__get_handle();
        if (((sb)->self) == NULL)
        {
            *status = SDS__NOMEM;
            return;
        }
        *(sb->self) = (void*)sb;
        sb->nitems = nitems;
        for(i=0;i<nitems;i++)
        {
            Sds__import(data,swap4(bs,data[temp_pos]),&sub,bs,status);
            sb->item[i] = sub->self;
            sub->parent = sb->self;
            temp_pos++;
        }
        *sbptr = sb;
    }
    else if (code == SDS_SARRAY) 
    {
        pb = (pblock*)SdsMalloc(sizeof(pblock));
        if (pb == NULL)
        {
            *status = SDS__NOMEM;
            return;
        }
        pb->code = (char) code;
        pb->format = 0;
        strcpy(pb->name,name);
        ncomps = 1;
        for (i=0;i<nitems;i++)
        {
            pb->dims[i] = swap4(bs,data[pos]);
            pos++;
            ncomps = ncomps*pb->dims[i];
        }
        pb->ndims = nitems;
        temp_pos = pos;
        pos += ncomps;
        pb->nextra = swap2(bs,((short*)data)[2*pos]);
        memcpy(pb->extra,((char*)data)+(4*pos)+2,pb->nextra);
        pb->self = Sds__get_handle();
        if ((pb->self) == NULL)
        {
            *status = SDS__NOMEM;
            return;
        }
        *(pb->self) = (void*)pb;
        hblock = (handle*)SdsMalloc(ncomps*sizeof(handle));
        if (hblock == 0)
        {
            *status = SDS__NOMEM;
            return;
        }
        pb->data = (handle)hblock;
        for(i=0;i<ncomps;i++)
        {
            Sds__import(data,swap4(bs,data[temp_pos]),&sub,bs,status);
            sub->parent = pb->self;
            temp_pos++;
            hblock[i] = sub->self;
        }
        *sbptr = (sblock*)pb;
    }
    else
    {
        pb = (pblock*)SdsMalloc(sizeof(pblock));
        if (pb == NULL)
        {
            *status = SDS__NOMEM;
            return;
        }
        pb->code = (char) code;
        pb->format = (char) format;
        strcpy(pb->name,name);
        ncomps = 1;
        dptr = swap4(bs,data[pos]);
        pos++;
        for (i=0;i<nitems;i++)
        {
            pb->dims[i] = swap4(bs,data[pos]);
            pos++;
            ncomps = ncomps*pb->dims[i];
        }
        pb->ndims = nitems;
        pb->nextra = swap2(bs,((short*)data)[2*pos]);
        memcpy(pb->extra,((char*)data)+(4*pos)+2,pb->nextra);
        pb->self = Sds__get_handle();
        if ((pb->self) == NULL)
        {
            *status = SDS__NOMEM;
            return;
        }
        *(pb->self) = (void*)pb;
        if (dptr != 0)
        {
            size = nbytes[code]*ncomps;
            Sdsdata = (void*)SdsMalloc(size);
            if (Sdsdata == NULL)
            {
                *status = SDS__NOMEM;
                return;
            }
            pb->data = Sds__get_handle();
            if (pb->data == NULL)
            {
                *status = SDS__NOMEM;
                return;
            }
            *(pb->data) = Sdsdata;
            memcpy(*(pb->data),data+dptr,size);
        }
        else
            pb->data = NULL;
        *sbptr = (sblock*)pb;
    }
}
       


/*+				S d s I m p o r t

 *  Function name:
      SdsImport

 *  Function:
      Import an object from an external buffer

 *  Description:
      Import an object from an external buffer and return an identifier
      to the internal copy created. The object must have
      been previously exported using SdsExport.

      The original external version of the
      structure continues to exist, in addition to the internal copy. 

      A fully dynamic internal structure is created in which all SDS
      operations are valid. However, to merely access the data in an object
      SdsAccess can be used in place of SdsImport.

 *  Language:
      C

 *  Declaration:
      void SdsImport(void *data, SdsIdType *id, StatusType * SDSCONST status)
      
 *  Parameters:   (">" input, "!" modified, "W" workspace, "<" output)
      (>) data      (void*) The buffer from which the object will be
                          imported.
      (<) id        (SdsIdType*)  Identifier of the new internal object.
      (!) status    (StatusType*) Modified status. Possible failure codes are:
                            SDS__NOTSDS => Not a valid sds object.
                            SDS__NOMEM => Insufficient memory.

 *  Support: Jeremy Bailey, {AAO}

 *  Version date: 18-Oct-91
 *-
 */
        

SDSEXTERN void SdsImport(
    SDSCONST void *data, 
    SdsIdType *id, 
    StatusType * SDSCONST status)

{
    INT32 pos;
    sblock *sb;
    idblock *idptr;
    BOOL bs;

    if (*status != STATUS__OK) return;

    if (((INT32*)data)[0] == 0)
        bs = (local_format[SDS_INT] == LITTLEEND);
    else if (((INT32*)data)[0] == ~0)
        bs = (local_format[SDS_INT] == BIGEND);
    else
    {
        *status = SDS__NOTSDS;
        return;
    }
   
/*  Check SDS version of data is OK  */

    if (swap4(bs,((INT32*)data)[2]) != SDS_VERSION)
    {
        *status = SDS__VERSION;
        return;
    }
    if (!Sds___FSemTake(status)) return;
    /*
     * From this point on - must always exit via "Exit" label.  To ensure
     * semaphore is released.
     */


  
    pos = 4;   
    Sds__import((INT32*)data, pos, &sb, bs, status);
    sb->parent = NULL;
    if (*status != SDS__OK) goto Exit;

    Sds__get_new_id(&idptr, id, status);
    Sds__put_id(*id,sb->self,FALSE,NULL,status);

Exit:
    Sds___FSemGive();

}
   
static void Sds__swap_pointers(INT32* origin, INT32* data, BOOL bs)

{
    int nitems;
    int i;
    long code;
    INT32 pos;
    long ncomps;
    INT32 ptr;
    long dims;
   
    code = ((char*)data)[0];
    nitems = ((short*)data)[1] = swap2(bs,((short*)data)[1]);
    pos = 5;
    if (code == SDS_STRUCT)
    {
        for (i=0;i<nitems;i++)
        {
            ptr = data[pos+i] = swap4(bs,data[pos+i]);
            Sds__swap_pointers(origin,origin+ptr,bs);
        }
        pos += nitems;
        ((short*)data)[2*pos] = swap2(bs,((short*)data)[2*pos]);
    }
    else if (code == SDS_SARRAY)
    {
        ncomps = 1;
        for (i=0;i<nitems;i++)
        {
            dims = data[pos+i] = swap4(bs,data[pos+i]);
            ncomps = ncomps*dims;
        }
        pos += nitems;
        for (i=0;i<ncomps;i++)
        {
            ptr = data[pos+i] = swap4(bs,data[pos+i]);
            Sds__swap_pointers(origin,origin+ptr,bs);
        }
        pos += ncomps;
        ((short*)data)[2*pos] = swap2(bs,((short*)data)[2*pos]);
    }
    else
    {
        data[pos] = swap4(bs,data[pos]);
        pos++;
        for (i=0;i<nitems;i++)
            data[pos+i] = swap4(bs,data[pos+i]);
        pos += nitems;
        ((short*)data)[2*pos] = swap2(bs,((short*)data)[2*pos]);
    }
}  
       


/*+				  S d s A c c e s s

 *  Function name:
      SdsAccess

 *  Function:
      Return an identifier to an external object

 *  Description:
      Make an external object (one exported by SdsExport) accessible
      to SDS by returning an identifier to it.

      Any SDS operations which do not change the structure of the object
      may be performed on the external object. These include navigation
      operations (SdsFind, SdsIndex, SdsCell), data access operations
      (SdsGet, SdsPut, SdsPointer) and inquiry operations (SdsInfo).

      Operations which are not permitted on an external object are those
      which add or remove components (SdsNew, SdsDelete), or write
      operations (SdsPut or SdsPointer) to data items which are currently 
      undefined.

      Unlike SdsImport, SdsAccess does not make a copy of the object. The 
      object is accessed in place in the original buffer.

 *  Language:
      C

 *  Declaration:
      void SdsAccess(void *data, SdsIdType *id, StatusType * SDSCONST status)
      
 *  Parameters:   (">" input, "!" modified, "W" workspace, "<" output)
      (>) data      (void*) The buffer containing the object to be
                          accessed.
      (<) id        (long*)  Identifier of the external object.
      (!) status    (StatusType*) Modified status. Possible failure codes are:
                            SDS__NOTSDS => Not a valid SDS object.
                            SDS__NOMEM => Insufficient memory.

 *  Support: Jeremy Bailey, {AAO}

 *  Version date: 18-Oct-91
 *-
 */
        


SDSEXTERN void SdsAccess(
    void *data, 
    SdsIdType *id, 
    StatusType * SDSCONST status)

{
    /*
     *  See the SDS Document (sds.tex), section "SDS Data Fomat" for a description
     *  of the format of the data.
     */
    idblock *idptr;
    BOOL bs;        /* Do we need to byte swap? */


    /*
     * First longword in the header indicates the byte order. 0 for big endian,
     * FFFFFFFF for little Endian.  The flags LITTLEEND and BIGEND indicate 
     * the local format.
     *
     * "bs" indicates that we need to byte swap.
     */ 
    if (*status != 0) return;
    if (((INT32*)data)[0] == 0)
        bs = (local_format[SDS_INT] == LITTLEEND);
    else if (((INT32*)data)[0] == ~0)
        bs = (local_format[SDS_INT] == BIGEND);
    else
    {
        *status = SDS__NOTSDS;
        return;
    }

    /*
     * Second longword is the length in bytes.
     */
    ((INT32*)data)[1] = swap4(bs,((INT32*)data)[1]);
    /*
     * Third longword is the SDS version.
     */
    ((INT32*)data)[2] = swap4(bs,((INT32*)data)[2]);

/*  Check SDS version of data is OK  */


    if ((((INT32*)data)[2]) != SDS_VERSION)
    {
        *status = SDS__VERSION;
        return;
    }
  

    if (!Sds___FSemTake(status)) return;
    /*
     * From this point on - must always exit via "Exit" label.  To ensure
     * semaphore is released.
     */


    Sds__get_new_id(&idptr, id, status);
    Sds__put_id(*id,(handle)(((INT32*)data)+4),TRUE,(INT32*)data,status);

    if (*status != STATUS__OK) goto Exit;

    if (bs) 
    {
        Sds__swap_pointers((INT32*)data,((INT32*)data)+4,bs);
        ((INT32*)data)[0] = ~(((INT32*)data)[0]);
    }
Exit:
    Sds___FSemGive();

}
   



/*+				S d s E x t r a c t

 *  Function name:
      SdsExtract

 *  Function:
      Extract an object from a structure

 *  Description:
      Extract an object from a structure. The extracted object becomes a 
      new independent top level object. The object is deleted from
      the original structure.

      The identifier must not be that of a structure array component.
      
      If the identifier is already that of a top level object, then the
      function does nothing.

 *  Language:
      C

 *  Declaration:
      void SdsExtract(SdsIdType id, StatusType * SDSCONST status)
      
 *  Parameters:   (">" input, "!" modified, "W" workspace, "<" output)
      (>) id        (SdsIdType)  Identifier of the object to be extracted.
      (!) status    (StatusType*) Modified status. Possible failure codes are:
                            SDS__BADID => The identifier is invalid.
                            SDS__ILLDEL => Object cannot be extracted.
                            SDS__EXTERN => The object is external.


 *  Support: Jeremy Bailey, {AAO}

 *  Version date: 23-Oct-91
 *-
 */

SDSEXTERN void SdsExtract(SdsIdType id, StatusType * SDSCONST status)

{
    sblock *sb;
    sblock *parent;
    long index;
    handle block;
    INT32 external;
    INT32 *origin;


    /* Take semaphore and access id */
    Sds__FSemTake_get_id(id,&block,&external,&origin,status);
    if (*status != SDS__OK) return;
    /*
     * From this point on - must always exit via "Exit" label.  To ensure
     * semaphore is released.
     */
    if (external)
    {
        *status = SDS__EXTERN;
        return;
    }
    Sds___WatchEvent(id, SDS_WATCH_EVENT_EXTRACT, "SdsExtract");
    sb = (sblock*)(*block);
    if ((sb->parent) != NULL)
    {

/*  Find the parent of the object to be extracted  */

        parent = (sblock*)(*(sb->parent));
        index = 0;
        if ((parent->nitems == 0) || (parent->code == SDS_SARRAY))
        {
            *status = SDS__ILLDEL;
            goto Exit;
        }

/*  Loop through the parents item list to find the item to be deleted  */

        while (*(parent->item[index]) != sb)
        {
            index++;
            if (index >= (parent->nitems))
            {

/*  Can't find the item - must be a structure array component for
    which deletion is illegal  */

                *status = SDS__ILLDEL;
                goto Exit;
            }
        }
        
/*  Shift up following items in list by one and reduce item count by one */

        /*A change by NPR@JACH so the "-1" added here, merged by TJF,2-Apr-04*/
        for (;index < (parent->nitems-1);index++)
            parent->item[index] = parent->item[index+1];
        (parent->nitems)--;
    }

    sb->parent = NULL;

Exit:
    Sds___FSemGive();

}




/*+				  S d s I n s e r t

 *  Function name:
      SdsInsert

 *  Function:
      Insert an existing object into a structure

 *  Description:
      An existing top level object is inserted into a structure.
      
 *  Language:
      C

 *  Declaration:
      void SdsInsert(SdsIdType parent_id, SdsIdType id, StatusType * SDSCONST status)
      
 *  Parameters:   (">" input, "!" modified, "W" workspace, "<" output)
      (>) parent_id (SdsIdType)  The identifier of the structure to
                            which the component is to be added.
      (>) id        (SdsIdType)  The identifier of the object to be inserted.
      (!) status    (StatusType*) Modified status. Possible failure codes are:
                            SDS__BADID => Invalid identifier
                            SDS__NOTSTRUCT => Parent is not a structure
                            SDS__NOTTOP => Not a top level object
                            SDS__NOMEM => Insufficient memory
                            SDS__EXTERN => Object is external

 *  Support: Jeremy Bailey, {AAO}

 *  Version date: 23-Oct-91
 *-
 */

SDSEXTERN void SdsInsert(
    SdsIdType parent_id, 
    SdsIdType id, 
    StatusType * SDSCONST status)

{
    sblock *sb;            /* pointer to parent structure */
    sblock *stb;           /* pointer to new component */
    long n;
    handle block;
    INT32 external;
    INT32 *origin;

    /* Take semaphore and access parent id */
    Sds__FSemTake_get_id(parent_id,&block,&external,&origin,status);
    if (*status != SDS__OK) return;

    if (external)
    {
        *status = SDS__EXTERN;
        goto Exit;
    }
    Sds___WatchEvent(id, SDS_WATCH_EVENT_INSERT, "SdsInsert");
    sb = (sblock*)(*block);
    if (sb->code != SDS_STRUCT)
    {
        *status = SDS__NOTSTRUCT;
        goto Exit;
    }

/* import object  */

    Sds__get_id(id,&block,&external,&origin,status);
    if (*status != SDS__OK) goto Exit;
    if (external)
    {
        *status = SDS__EXTERN;
        goto Exit;
    }
    stb = (sblock*)(*block);
    if (stb->parent != NULL)
    {
        *status = SDS__NOTTOP;
        goto Exit;
    }

/*  Extend structure if necessary  */

    n = sb->nitems;
    if (n >= sb->maxitems) Sds__extend_struct(&sb,status);
    if (*status != SDS__OK) goto Exit;
    sb->nitems = n+1;

/*  Insert object */

    sb->item[n] = stb->self;
    stb->parent = sb->self;

Exit:
    Sds___FSemGive();


}

/*
 *  Return a copy of the object sb
 */
static void Sds__copy(
    sblock *sb, 
    sblock **sbc, 
    StatusType * SDSCONST status)
{
    sblock *sub;
    pblock *parb;
    pblock *pb;
    handle *hblock;
    handle *hblock1;
    long ncomps;
    long i;
    void *Sdsdata;
    long size;

    Sds___FSemCheckTaken();  /* If we are using the Full semaphore, it
                              * should be taken at this point */

    if (sb->code == SDS_STRUCT)

/*  Structure case  */

    {

/*  Create a new sblock  */

        *sbc = Sds__create_sblock(sb->name,sb->nextra,sb->extra,
                                  (long)(sb->nitems));
        if (*sbc == NULL)
        {
            *status = SDS__NOMEM;
            return;
        }

/*  Create a handle to the block  */

        (*sbc)->self = Sds__get_handle();
        if ((*sbc)->self == NULL)
        {
            *status = SDS__NOMEM;
            return;
        }
        *((*sbc)->self) = (void*)(*sbc);

/*  Copy the items in the structure   */

        (*sbc)->nitems = sb->nitems;
        for (i=0;i<sb->nitems;i++)
        {
            Sds__copy((sblock*)(*(sb->item[i])),&sub,status);
            (*sbc)->item[i] = sub->self;
            sub->parent = (*sbc)->self;
        }
    }
    else if (sb->code == SDS_SARRAY)
    {

/*  Structure array case  */

        parb = (pblock*)sb;

/*  Create a new pblock  */

        pb = (pblock*)SdsMalloc(sizeof(pblock));
        if (pb == NULL)
        {
            *status = SDS__NOMEM;
            return;
        }

/*  Copy contents of pblock  */

        pb->code = parb->code;
        pb->format = 0;
        strcpy(pb->name,parb->name);
        ncomps = 1;
        for (i=0;i<parb->ndims;i++)
        {
            pb->dims[i] = parb->dims[i];
            ncomps = ncomps*pb->dims[i];
        }
        pb->ndims = parb->ndims;
        pb->nextra = parb->nextra;
        if ( pb->nextra != 0 )
        {
            memcpy(pb->extra,parb->extra,pb->nextra);
	}

/*  Create a handle  */

        pb->self = Sds__get_handle();
        if ((pb->self) == NULL)
        {
            *status = SDS__NOMEM;
            return;
        }
        *(pb->self) = (void*)pb;

/*  Create a handle block for the array components  */

        hblock1 = (handle*)(parb->data);
        hblock = (handle*)SdsMalloc(ncomps*sizeof(handle));
        if (hblock == 0)
        {
            *status = SDS__NOMEM;
            return;
        }
        pb->data = (handle)hblock;

/*  Copy the array components   */

        for(i=0;i<ncomps;i++)
        {
            Sds__copy((sblock*)(*(hblock1[i])),&sub,status);
            sub->parent = pb->self;

/* The following 7 lines are commented out because they creates a memory leak 
   since the handle has already been allocated by the Sds__copy - NPR - 23/1/01 */ 
/* 
   sub->self = Sds__get_handle();
   if (sub->self == NULL)
   {
   *status = SDS__NOMEM;
   return;
   }
   *(sub->self) = (void*)sub;
   */
            hblock[i] = sub->self;
        }
        *sbc = (sblock*)pb;
    }
    else
    {

/*  Primitive case  */

        parb = (pblock*)sb;

/*  Create a pblock  */

        pb = (pblock*)SdsMalloc(sizeof(pblock));
        if (pb == NULL)
        {
            *status = SDS__NOMEM;
            return;
        }

/*  Copy contents of pblock  */

        pb->code = parb->code;
        pb->format = parb->format;
        strcpy(pb->name,parb->name);
        ncomps = 1;
        for (i=0;i<parb->ndims;i++)
        {
            pb->dims[i] = parb->dims[i];
            ncomps = ncomps*pb->dims[i];
        }
        pb->ndims = parb->ndims;
        pb->nextra = parb->nextra;
        if ( pb->nextra != 0 )
        {
            memcpy(pb->extra,parb->extra,pb->nextra);
        }

/*  Create a handle  */

        pb->self = Sds__get_handle();
        if ((pb->self) == NULL)
        {
            *status = SDS__NOMEM;
            return;
        }
        *(pb->self) = (void*)pb;

/*  Copy the data if there is any  */

        if (parb->data != NULL)
        {
            size = nbytes[(int)pb->code]*ncomps;
            Sdsdata = (void*)SdsMalloc(size);
            if (Sdsdata == NULL)
            {
                *status = SDS__NOMEM;
                return;
            }
            pb->data = Sds__get_handle();
            if (pb->data == NULL)
            {
                *status = SDS__NOMEM;
                return;
            }
            *(pb->data) = Sdsdata;
            memcpy(*(pb->data),*(parb->data),size);
        }
        else
            pb->data = NULL;
        *sbc = (sblock*)pb;
    }
}
       


/*+				S d s C o p y

 *  Function name:
      SdsCopy

 *  Function:
      Make a copy of an object

 *  Description:
      Make a copy of an object and return an identifier to the copy.
      The copy is a new top level object, the original object is
      unchanged by the operation.

      The object being copied can be either external or internal.
      The copy is always an internal object.

 *  Language:
      C

 *  Declaration:
      void SdsCopy(SdsIdType id, SdsIdType *copy_id, StatusType * SDSCONST status)
      
 *  Parameters:   (">" input, "!" modified, "W" workspace, "<" output)
      (>) id        (SdsIdType)  Identifier of the object to be copied.
      (<) copy_id   (SdsIdType*) Identifier of the copy.
      (!) status    (StatusType*) Modified status. Possible failure codes are:
                            SDS__BADID => The identifier is invalid.
                            SDS__NOMEM => Insuficient memory.

 *  Support: Jeremy Bailey, {AAO}

 *  Version date: 23-Oct-91
 *-
 */


SDSEXTERN void SdsCopy(SdsIdType id, SdsIdType *copy_id, 
                       StatusType * SDSCONST status)


{
    sblock *sb;
    idblock *copyptr;
    handle block;
    INT32 external;
    INT32 *origin;

    /* Take semaphore and access id */
    Sds__FSemTake_get_id(id,&block,&external,&origin,status);
    if (*status != SDS__OK) return;
    /*
     * From this point on - must always exit via "Exit" label.  To ensure
     * semaphore is released.
     */
    Sds___WatchEvent(id, SDS_WATCH_EVENT_COPY, "SdsCopy");


    if (external)
    {

/*  If it is an external object we can use Sds__import to do the copy,
    but we have to turn byte swapping off, because this will already have
    been done by SdsAccess.   */

        Sds__import(origin,(INT32)((INT32*)(block)-origin),
                    &sb,FALSE,status);
    }
    else

/*  Otherwise it is an internal to internal copy done by Sds__copy  */

        Sds__copy((sblock*)(*block),&sb,status);

/*  Set parent to NULL - no parent  */

    sb->parent = NULL;
    if (*status != SDS__OK) goto Exit;

/*  Create an id block and set it to point to the object  */

    Sds__get_new_id(&copyptr, copy_id, status);
    Sds__put_id(*copy_id,sb->self,FALSE,NULL,status);

Exit:
    Sds___FSemGive();

}



/*+				S d s R e s i z e

 *  Function name:
      SdsResize

 *  Function:
      Change the dimensions of an array.

 *  Description:
     Change the number and/or size of the dimensions of an array. This
     operation can be performed on primitive arrays or structure arrays.
     Note that SDS_RESIZE does not move the data elements in the storage
     representing the array, so there is no guarantee that after resizing
     the array the same data will be found at the same array index 
     positions as before resizing, though this will be the case for
     simple changes such as a change in the last dimension only.

     If the size of a primitive array is increased the contents of the
     extra locations is undefined. Decreasing the size causes the data
     beyond the new limit to be lost.

     If a structure array is extended the new elements created will be empty
     structures. If a structure array is decreased in size, the lost elements
     and all their components will be deleted.

 *  Language:
      C

 *  Declaration:
      void SdsResize(SdsIdType id, long ndims, unsigned long *dims, 
              StatusType * SDSCONST status)
      
 *  Parameters:   (">" input, "!" modified, "W" workspace, "<" output)
      (>) id        (SdsIdType) Identifier of the object to be resized.
      (>) ndims     (long)  New number of dimensions.
      (>) dims      (unsigned long*) Array of dimensions.
      (!) status    (StatusType*) Modified status. Possible failure codes are:
                            SDS__BADID => The identifier is invalid.
                            SDS__NOMEM => Insuficient memory.
                            SDS__EXTERN => Object is external.
                            SDS__NOTARR => Object is not an array.
                            SDS__INVDIMS => Dimensions invalid.

 *  Support: Jeremy Bailey, {AAO}

 *  Version date: 23-Oct-91
 *-
 */

SDSEXTERN void SdsResize(
    SdsIdType id, 
    long ndims, 
    SDSCONST unsigned long *dims, 
    StatusType * SDSCONST status)

{
    pblock *pb;
    handle *hblock;
    handle *newhblock;
    long oldsize,newsize;
    sblock *sab;
    void* dptr;
    long i;
    handle block;
    INT32 external;
    INT32 *origin;

    /* Take semaphore and access id */
    Sds__FSemTake_get_id(id,&block,&external,&origin,status);
    if (*status != SDS__OK) return;
    /*
     * From this point on - must always exit via "Exit" label.  To ensure
     * semaphore is released.
     */
    if (external)
    {
        *status = SDS__EXTERN;
        goto Exit;
    }
    Sds___WatchEvent(id, SDS_WATCH_EVENT_RESIZE, "SdsResize");
    pb = (pblock*)(*block);
    if (pb->code == SDS_STRUCT)
    {   
        *status = SDS__NOTARR;
        goto Exit;
    }

/*  check dimensions  */

    if ((ndims > MAXDIMS) || (ndims<0))
    {
        *status = SDS__INVDIMS;
        goto Exit;
    }

    oldsize = 1;
    for (i=0;i<pb->ndims;i++) oldsize = oldsize*pb->dims[i];
    pb->ndims = (short) ndims;
    newsize = 1;
    for (i=0;i<pb->ndims;i++)
    {
        pb->dims[i] = dims[i];
        newsize = newsize*dims[i];
    }
    if (pb->code == SDS_SARRAY) 
    {
        hblock = (handle*)(pb->data);
        if (newsize < oldsize)
        {
            for (i=newsize;i<oldsize;i++)
            {
                /*
                 * Delete the block and then ensure any ID's
                 * referring to it are invalided
                 */
                Sds__delete((sblock*)(*(hblock[i])));
                Sds__invalidateIdsFor(hblock[i]);
            }

        }
        else if (newsize > oldsize)
        {
            newhblock = (handle*)SdsMalloc(newsize*sizeof(handle));
            if (newhblock == NULL)
            {
                *status = SDS__NOMEM;
                goto Exit;
            }
            for (i=0;i<oldsize;i++) newhblock[i] = hblock[i];
            for (i=oldsize;i<newsize;i++)
            { 
                sab = Sds__create_sblock(pb->name,pb->nextra,pb->extra,
                                         (long)NITEMS);
                if (sab == NULL) 
                {
                    *status = SDS__NOMEM;
                    goto Exit;
                }
                sab->parent = pb->self;
                sab->self = Sds__get_handle();
                if ((sab->self) == NULL)
                {
                    *status = SDS__NOMEM;
                    goto Exit;
                }
                *(sab->self) = (void*)sab;
                newhblock[i] = sab->self;
            }
            pb->data = (handle)newhblock;
            SdsFree(hblock);
        }
    }
    else
    {
        if ((pb->data == NULL) || (newsize <= oldsize))
            goto Exit;
        dptr = (void*)SdsMalloc(newsize*nbytes[(int)pb->code]);
        if (dptr == NULL)
        {
            *status = SDS__NOMEM;
            goto Exit;
        }
        memcpy(dptr,*(pb->data),
               ((newsize>oldsize) ? oldsize : newsize)*nbytes[(int)pb->code]);
        SdsFree(*(pb->data));
        *(pb->data) = dptr;
    }
Exit:
    Sds___FSemGive();
}




/*+				S d s R e n a m e

 *  Function name:
      SdsRename

 *  Function:
      Change the name of an object.

 *  Description:
      Specify a new name for an object.

 *  Language:
      C

 *  Declaration:
      void SdsRename(SdsIdType id, char* name, StatusType * SDSCONST status)
      
 *  Parameters:   (">" input, "!" modified, "W" workspace, "<" output)
      (>) id        (SdsIdType)  Identifier of the object to be renamed.
      (>) name      (char*) New name for the object. This should
                            have a maximum length of 16 characters including
                            the terminating null.
      (!) status    (long*) Modified status. Possible failure codes are:
                            SDS__BADID => The identifier is invalid.
                            SDS__LONGNAME => The name is too long.

 *  Support: Jeremy Bailey, {AAO}

 *  Version date: 24-Oct-91
 *-
 */

SDSEXTERN void SdsRename(
    SdsIdType id, 
    SDSCONST char *name, 
    StatusType * SDSCONST status)

{
    sblock *sb;
    handle block;
    INT32 external;
    INT32 *origin;
   
    /* Take semaphore and access id */
    Sds__FSemTake_get_id(id,&block,&external,&origin,status);
    if (*status != SDS__OK) return;
    /*
     * From this point on - must always exit via "Exit" label.  To ensure
     * semaphore is released.
     */
    if (strlen(name) > (SDS_C_NAMELEN-1))      /* -1 allows for null */
    {
        *status = SDS__LONGNAME;
        goto Exit;
    }
    if (external)
    {
        strcpy((char*)(block)+4,name);
    }
    else
    {
        sb = (sblock*)(*block);
        strcpy(sb->name,name);
    }
Exit:
    Sds___FSemGive();
}


/*
 *  External case of SdsPutExtra
 */
static void Sds__PutExtra_ext(
    INT32 *data, 
    long nextra, 
    SDSCONST char *extra, 
    StatusType * SDSCONST status)


{
    long code;
    long nitems;
    long ncomps;
    INT32 pos;
    long i;
    long oldnextra;

    Sds___FSemCheckTaken();  /* If we are using the Full semaphore, it
                              * should be taken at this point */
    code = ((char*)data)[0];
    nitems = ((short*)data)[1];

/* Determine position of extra field in data - This depends on the
   type of the object, and the number of components / dimensions  */

    if (code == SDS_STRUCT)
    {
        pos = 5+nitems;
    }
    else if (code == SDS_SARRAY)
    {
        ncomps = 1;
        for (i=0;i<nitems;i++)
            ncomps = ncomps*data[5+i];
        pos = 5 + nitems + ncomps;
    }
    else
    {
        pos = 6+nitems;
    }
    oldnextra = ((short*)data)[2*pos];
    if (nextra > oldnextra)
    {
        *status = SDS__EXTRA;
        return;
    }
    ((short*)data)[2*pos] = (short) nextra;
    memcpy(((char*)data)+(4*pos)+2,extra,nextra);
}   





/*+			  	S d s P u t E x t r a

 *  Function name:
      SdsPutExtra

 *  Function:
      Write to the extra information field of an object.

 *  Description:
      Write a specified number of bytes to the extra information field
      of an object. A maximum of 128 bytes may be written to an internal
      object. It is permissible to write to the extra information field
      of an external object, but the number of bytes written must not
      exceed the number originally in the object.

 *  Language:
      C

 *  Declaration:
      void SdsPutExtra(SdsIdType id, long nextra, char* extra, 
                    StatusType * SDSCONST status)

 *  Parameters:   (">" input, "!" modified, "W" workspace, "<" output)
      (>) id        (SdsIdType) Identifier of the object
      (>) nextra    (long)  Number of bytes of extra information.
      (>) extra     (char*) The extra information to be included.
                            nextra bytes are copied into the object.
      (!) status    (StatusType*) Modified status. Possible failure codes are:
                            SDS__BADID => The identifier is invalid.
                            SDS__EXTRA => Too much extra data.

 *  Support: Jeremy Bailey, {AAO}

 *  Version date: 24-Oct-91
 *-
 */

SDSEXTERN void SdsPutExtra(
    SdsIdType id, 
    long nextra, 
    SDSCONST char *extra, 
    StatusType * SDSCONST status)

{
    sblock *sb;
    handle block;
    INT32 external;
    INT32 *origin;
   
    /* Take semaphore and access id */
    Sds__FSemTake_get_id(id,&block,&external,&origin,status);
    if (*status != SDS__OK) return;
    /*
     * From this point on - must always exit via "Exit" label.  To ensure
     * semaphore is released.
     */
    if (nextra > MAX_EXTRA)
    {
        *status = SDS__EXTRA;
        goto Exit;
    }
    if (external)
    {
        Sds__PutExtra_ext((INT32*)(block),nextra,extra,status);
    }
    else
    {
        sb = (sblock*)(*block);
        memcpy(sb->extra,extra,nextra);
        sb->nextra = nextra;
    }
Exit:
    Sds___FSemGive();


}


/*
 *  External case of SdsGetExtra and SdsGetExtraLen().  The later
 *   is indicated by "extra" being null and in which case, length
 *   is ignored.
 */
static void Sds__GetExtra_ext(
    INT32 *data, 
    long length, 
    char *extra, 
    unsigned long *actlen, 
    StatusType * SDSCONST status)



{
    long code;
    long nitems;
    long ncomps;
    INT32 pos;
    long i;
    long nextra;

    Sds___FSemCheckTaken();  /* If we are using the Full semaphore, it
                              * should be taken at this point */

    if (*status != SDS__OK) return;

    code = ((char*)data)[0];
    nitems = ((short*)data)[1];

/* Determine position of extra field in data - This depends on the
   type of the object, and the number of components / dimensions  */

    if (code == SDS_STRUCT)
    {
        pos = 5+nitems;
    }
    else if (code == SDS_SARRAY)
    {
        ncomps = 1;
        for (i=0;i<nitems;i++)
            ncomps = ncomps*data[5+i];
        pos = 5 + nitems + ncomps;
    }
    else
    {
        pos = 6+nitems;
    }
    nextra = ((short*)data)[2*pos];
    if (extra)
    {
        /*
         * Copy the extra data.
         */
        *actlen = ((nextra > length) ? length : nextra);
        memcpy(extra,((char*)data)+(4*pos)+2,*actlen);
    } 
    else
    {
        /*
         * If we have no pointer to write the extra data to, then
         * we just want the length of it.
         */

        *actlen = nextra;
    }
}   






/*+			  	S d s G e t E x t r a

 *  Function name:
      SdsGetExtra

 *  Function:
      Read from the extra information field of an object.

 *  Description:
      Read bytes from the extra information field of an object. Bytes are
      copied until the supplied buffer is filled up or until all bytes
      in the field are copied. 

 *  Language:
      C

 *  Declaration:
      void SdsGetExtra(SdsIdType id, long length, char* extra, 
          unsigned long* actlen, StatusType * SDSCONST status)

 *  Parameters:   (">" input, "!" modified, "W" workspace, "<" output)
      (>) id        (SdsIdType) Identifier of the object
      (>) length    (long)  Length of buffer to receive data.
      (<) extra     (char*) Buffer to receive the extra information
                            copied from the object.
      (<) actlen    (unsigned long*) actual number of bytes copied.
      (!) status    (StatusType*) Modified status. Possible failure codes are:
                            SDS__BADID => The identifier is invalid.

 *  Support: Jeremy Bailey, {AAO}

 *  Version date: 24-Oct-91
 *-
 */

SDSEXTERN void SdsGetExtra(
    SdsIdType id, 
    long length, 
    char *extra, 
    unsigned long *actlen, 
    StatusType * SDSCONST status)

{
    sblock *sb;
    handle block;
    INT32 external;
    INT32 *origin;
   
    /* Take semaphore and access id */
    Sds__FSemTake_get_id(id,&block,&external,&origin,status);
    if (*status != SDS__OK) return;
    /*
     * From this point on - must always release semaphore before existing
     */
    if (external)
    {
        Sds__GetExtra_ext((INT32*)(block),length,extra,actlen,status);
    }
    else
    {
        sb = (sblock*)(*block);
        *actlen = ((sb->nextra > length) ? length : sb->nextra);
        memcpy(extra,sb->extra,*actlen);
    }
    Sds___FSemGive();
}




/*+			S d s I s E x t e r n a l

 *  Function name:
      SdsIsExternal

 *  Function:
      Enquire whether an object is external

 *  Description:
      Enquire whether an object is external

 *  Language:
      C

 *  Declaration:
      void SdsIsExternal(SdsIdType id, int *external, StatusType * SDSCONST status)

 *  Parameters:   (">" input, "!" modified, "W" workspace, "<" output)
      (>) id        (SdsIdType) Identifier of the object
      (<) external  (int *) Non zero if the object is external
      (!) status    (StatusType*) Modified status. Possible failure codes are:
                            SDS__BADID => The identifier is invalid.

 *  Support: Jeremy Bailey, {AAO}

 *  Version date: 13-Jul-98
 *-
 */

SDSEXTERN void SdsIsExternal(SdsIdType id, int * ext, 
                             StatusType * SDSCONST status)

{

   handle block;
   INT32 external;
   INT32 *origin;
   
    /* Take semaphore and access id */
    Sds__FSemTake_get_id(id,&block,&external,&origin,status);
    if (*status != SDS__OK) return;
    /*
     * From this point on - must always release semaphore before existing
     */
    *ext = external;
    Sds___FSemGive();

}


/*+			S d s E x t e r n I n f o

 *  Function name:
      SdsExternInfo

 *  Function:
      Return the address of an external object

 *  Description:
      Return the address of an external SDS object given its id.
      This is the address which was given to SdsAccess.

 *  Language:
      C

 *  Declaration:
      void SdsExternInfo(SdsIdType id, void ** address, StatusType * SDSCONST status)

 *  Parameters:   (">" input, "!" modified, "W" workspace, "<" output)
      (>) id        (SdsIdType) Identifier of the object
      (<) address   (void **) Address of object
      (!) status    (StatusType*) Modified status. Possible failure codes are:
                            SDS__BADID => The identifier is invalid.
                            SDS__EXTERN => Not an external object.

 *  Support: Jeremy Bailey, {AAO}

 *  Version date: 13-Jul-98
 *-
 */

SDSEXTERN void SdsExternInfo(
    SdsIdType id, 
    void ** address, 
    StatusType * SDSCONST status)

{

   handle block;
   INT32 external;
   INT32 *origin;
   
    /* Take semaphore and access id */
    Sds__FSemTake_get_id(id,&block,&external,&origin,status);
    if (*status != SDS__OK) return;
    /*
     * From this point on - must always release semaphore before existing
     */
    if (external)
        *address = origin;
    else
        *status = SDS__EXTERN;

    Sds___FSemGive();

}



/*
 *  Sds__import2 - Given an id number return a pointer to the id block
 *                 Don't give an error if the object has been deleted
 */
static idblock *Sds__import2(
    long id, 
    StatusType * SDSCONST status)


{
    if (*status != SDS__OK) return(NULL);

    Sds___FSemCheckTaken();  /* If we are using the Full semaphore, it
                              * should be taken at this point */


    if ((id <= 0) || (id > id_number))
    {
        *status = SDS__BADID;
        return(NULL);
    }
    if (idarrayptr[id].block == NULL)
    {
        *status = SDS__BADID;
        return(NULL);
    }
    return(&(idarrayptr[id]));
}





/*+			  	S d s F r e e I d

 *  Function name:
      SdsFreeId

 *  Function:
      Free an identifier, so that its associated memory may be reused.

 *  Description:
      Each identifier allocated by SDS uses memory. To avoid excessive
      allocation of memory the SdsFreeId function can be used to free
      the memory associated with an identifer that is no longer needed.
      When this is done the memory will be reused by SDS for a subsequent
      identifier when needed.

 *  Language:
      C

 *  Declaration:
      void SdsFreeId(SdsIdType id, StatusType * SDSCONST status)

 *  Parameters:   (">" input, "!" modified, "W" workspace, "<" output)
      (>) id        (SdsIdType)  Identifier to be freed
      (!) status    (StatusType*) Modified status. Possible failure codes are:
                            SDS__BADID => The identifier is invalid.

 *  Support: Jeremy Bailey, {AAO}
 
 *  Version date: 23-Jan-92
 *-
 */

SDSEXTERN void SdsFreeId(SdsIdType id, StatusType * SDSCONST status)

{
   idblock *idptr;

   if (*status != STATUS__OK) return;


    if (!Sds___FSemTake(status)) return;
    if (!Sds___IdSemTake(status)) return; /* Won't do anything if
                                           * FSemTake() does do
                                           * something 
                                           */
    /*
     * From this point on - must always exit via "Exit" label.  To ensure
     * semaphore is released.
     */


   Sds___WatchEvent(id, SDS_WATCH_EVENT_FREE, "SdsFreeId");

   idptr = Sds__import2(id,status);
   if (*status != SDS__OK) goto Exit;
   
   idptr->block = NULL;
   unused_ids++;

Exit:
   Sds___IdSemGive();
   Sds___FSemGive();

}


/*
 * Debugging  routine - return details on ids.
 */
SDSEXTERN void SdsGetIdInfo( 
    INT32 * p_id_number,     /* number of last id allocated */
    INT32 * p_id_array_size, /* current size of array of 
                                identifiers */
    INT32 * p_unused_ids)    /* number of free ids before end
                                of array */
{
    *p_id_number = id_number;
    *p_id_array_size = id_array_size;
    *p_unused_ids = unused_ids;
}





/*+			  	S d s S e t W a t c h 

 *  Function name:
      SdsSetWatch

 *  Function:
      Set a watch routine to watch for events on a given SDS ID.

 *  Description:
      This feature is still somewhat experimental.  It provides the ablity
      to watch certain happenings within SDS for a given SDS ID.  For example,
      if a given SDS ID is being free-ed incorrectly - often caused by a
      double free,. then you can specify the SDS ID number to watch and
      arrange for the specified function to be invoked when that ID is free-ed.

      You specify the ID, a routine to be invoked and the client data item.

 *  Language:
      C

 *  Declaration:
      void SdsSetWatch(SdsIdType id, watchRoutine, clientData,
                       oldId, oldWatchRoutine, oldClientData, status)

 *  Parameters:   (">" input, "!" modified, "W" workspace, "<" output)
      (>) id        (SdsIdType)  Identifier to be watched.  If 0, then
                          cancel watching.  If -1, then watch all IDs.  Note, it
                          is possible to watch an ID which is not currently in
                          use, in that case, you will get a  SDS_WATCH_EVENT_NEWID
                          event when starts being used. 
      (>) watchRoutine (SdsWatchRoutineType) The routine to be invoked when
                         the watch is triggered.  If not specified, then
                         a message is written to stderr.
      (>) clientData  (void *) Passed through to watchRoutine as its client data
                           item.
      (<) oldId       (SdsIdType *) If non-null, the previous id being watched 
                            is returned here.  Zero if there was none.
      (<) oldWatchRoutine (SdsWatchRoutineType *) If non-null, any old watch
                           routine address is returned here.
      (<) oldClientData (void **) If non-null, any old watch routine client
                           data is returned here.
      (!) status    (StatusType*) Modified status. 

 *  Function Prototypes Used:
         typedef int (*SdsWatchRoutineType)(SdsIdType id, 
                       void * client_data, int event);

 *  Parameters:   (">" input, "!" modified, "W" workspace, "<" output)
      (>) id          (SdsIdType) The ID which triggered the watch event.
      (>) client_data (void *)    As supplied to SdsSetWatch().
      (>) event    (int)       Indicates the event type.  The following event types
                               are supported.  Others might be added later and code 
                               should allow for this.

                                  SDS_WATCH_EVENT_FREE        => SdsFree or SdsFreeIdAndCheck. 
                                  SDS_WATCH_EVENT_DELETE      => SdsDelete.
                                  SDS_WATCH_EVENT_READFREE    => SdsReadFree.
                                  SDS_WATCH_EVENT_NEW         => SdsNew (on parent).
                                  SDS_WATCH_EVENT_COPY        => SdsCopy.
                                  SDS_WATCH_EVENT_INSERT      => SdsInsert.
                                  SDS_WATCH_EVENT_INSERT_CELL => SdsInsertCell.
                                  SDS_WATCH_EVENT_EXTRACT     => SdsExtract.
                                  SDS_WATCH_EVENT_RESIZE      => SdsResize
                                  SDS_WATCH_EVENT_NEWID       => Any function that creates an id.  "id" is the new SDS ID number.


 *  Support: Tony Farrell, {AAO}
 
 *  Version date: 14-Sep-2004
 *-
 */

SDSEXTERN void SdsSetWatch(SdsIdType id,
                 SdsWatchRoutineType watchRoutine,
                 void * clientData,
                 SdsIdType *oldId,
                 SdsWatchRoutineType *oldWatchRoutine,
                 void ** oldClientData,
                 StatusType * SDSCONST status)
{
    if (!Sds___FSemTake(status)) return;

    if (oldId)
        *oldId = SdsWatchID;
    if (oldWatchRoutine)
        *oldWatchRoutine = SdsWatchRoutine;
    if (oldClientData)
        *oldClientData = SdsWatchClientData;

    if (id)
    {
        SdsWatchID         = id;
        SdsWatchRoutine    = watchRoutine;
        SdsWatchClientData = clientData;
    }
    else
    {
        SdsWatchID         = 0;
        SdsWatchRoutine    = 0;
        SdsWatchClientData = 0;
    }
    Sds___FSemGive();
}


/*
 * This function is used to check for watch events and to act on them if need be.
 */
SDSEXTERN void Sds___WatchEvent(
    const SdsIdType id,   /* SDS ID event was for. */
    const int eventFlag,  /* Flag indicating event type - SDS_WATCH_EVENT_<...> */
    const char * const sourceFunc) /* Source function of watch event */

{
    if (SdsWatchID &&((id == SdsWatchID)||(SdsWatchID == -1)))
    {
        if (SdsWatchRoutine)
            (*SdsWatchRoutine)(id, SdsWatchClientData, eventFlag);
        else
            fprintf(stderr, "%s:Watch Event on ID %d\n", sourceFunc, (int)id);
    }

}



/*+			  	S d s F r e e I d A n d C h e c k

 *  Function name:
      SdsFreeIdAndCheck

 *  Function:
      Free an identifier, so that its associated memory may be reused. 

 *  Description:
      Each identifier allocated by SDS uses memory. To avoid excessive
      allocation of memory the SdsFreeId function can be used to free
      the memory associated with an identifer that is no longer needed.
      When this is done the memory will be reused by SDS for a subsequent
      identifier when needed.

      This version of SdsFreeId() will check that any data items have
      been deleted (SdsDelete()) or free-ed (SdsReadFree()) if needed.

 *  Language:
      C

 *  Declaration:
         void SdsFreeIdAndCheck(char *mes, SdsIdType id, StatusType * SDSCONST status)

 *  Parameters:   (">" input, "!" modified, "W" workspace, "<" output)
      (>) mes       (cosnt char *) If non-zero, then a message is written to
                            stderr when a problem is detector (rather then just setting
                            bad status).  This string will be part of that message.  The
                            message indicates the SDS ID number and the exact reason
                            the error was triggered.
      (>) id        (SdsIdType)  Identifier to be freed
      (!) status    (StatusType*) Modified status. Possible failure codes are:
                            SDS__BADID => The identifier is invalid.
                            SDS__CHK_FREE => something has not been cleaned up.

 *  Support: Tony Farrell, {AAO}
 
 *  Version date: 25-Sep-2008
 *-
 */

SDSEXTERN void SdsFreeIdAndCheck(
    const char *mes,
    SdsIdType id, 
    StatusType * SDSCONST status)

{
    idblock *idptr;
    /* pblock *pb1; */
    /* pblock *pb2; */
   

    if (!Sds___FSemTake(status)) return;
    if (!Sds___IdSemTake(status)) return; /* Won't do anything if
                                           * FSemTake() does do
                                           * something 
                                           */

    Sds___WatchEvent(id, SDS_WATCH_EVENT_FREE, "SdsFreeIdChk");

    idptr = Sds__import2(id,status);
    if (*status != SDS__OK) goto Exit;
    /* pb1 = (pblock *)(idptr->block); */
    /* pb2 = (pblock *)(*(idptr->block)); */
    if ((idptr->external))
    {
        if (idptr->free)
        {
            *status = SDS__CHK_FREE;
            if (mes)
                fprintf(stderr,"SdsFreeIdAndCheck:%s:SDS ID %d being free-ed without SdsReadFree() having been run\n",
                        mes, (int)id);
        }
    }
    else if (*(idptr->block) != NULL)
    {
        /* A top level non-external item - if we don't have a null block pointer, complain */
        if (((pblock *)(*idptr->block))->parent == NULL)
        {
            *status = SDS__CHK_FREE;
            if (mes)
                fprintf(stderr,"SdsFreeIdAndCheck:%s:SDS ID %d being free-ed without SdsDelete() having been run\n",
                        mes, (int)id);
        }
        /* Note - it is intended that the SDS item be free-ed anyway */
    }

   
    idptr->block = NULL;
    unused_ids++;

Exit:
    Sds___IdSemGive();
    Sds___FSemGive();

}

/*+			  	S d s L i s t I n U s e

 *  Function name:
      SdsListInUse

 *  Function:
      Debugging routine - list in use SDS ID's to stderr.

 *  Description:
      This is a simple debugging routine which will list the currently
      in use SDS id numbers to stderr (on one line).

 *  Language:
      C

 *  Declaration:
         void SdsListInUse()

 *  Support: Tony Farrell, {AAO}
 
 *  Version date: 02-Nov-2009
 *-
 */
SDSEXTERN void SdsListInUse()
{
    int i;
    StatusType ignore = STATUS__OK;
    if (!Sds___FSemTake(&ignore)) return;

    fprintf(stderr,"IDs in use:");
    for (i = 1; i <= id_number; i++) 
    {
        if (idarrayptr[i].block != NULL )
        {
            fprintf(stderr,"%d ", i);
        }
    }
    fprintf(stderr,"\n");

    Sds___FSemGive();

}

/*+			  	S d s C h e c k B u i l d

 *  Function name:
      SdsCheckBuild

 *  Function:
      Debugging routine - checks the SDS build against a user build.

 *  Description:
      This function will check that the calling code is using the same
      values for various items defined in sdsport.h (the SDS
      portability layer) as were used when sds itself was built.  

      Problems could occur due to different compilers or compiler options.

 *  Language:
      C

 *  Declaration:
         void SdsCheckBuild(status)

 *  Implementation Note:
      SdsCheckBuild() is a macro defined in sds.h.  The macro invokes
      Sds___CheckBuild() to do the actual check.  The macro passes in the
      values the calling code has for various things defined in sdsport.h  This
      function compares them to what this module was built with.  If they are 
      incompatible, it outputs details to stderr and returns bad status.
 
      This function is invoked by the sdstest.c test program which is built by the
      DRAMA but can be invoked by other user code if needed.

 *  Support: Tony Farrell, {AAO}
 
 *  Version date: 02-Nov-2009
 *-
 */
SDSEXTERN void Sds___CheckBuild(
    int bigend,
    int long_64,
    int native_64,
    StatusType *status)
{
    int err = 0;
    if (*status != STATUS__OK) return;

    if (bigend != SDS_BIGEND)
    {
        ++err;
        fprintf(stderr,"SdsCheckBuild() caller has SDS_BIGEND = %d, SDS library compiled with SDS_BIGEND = %d\n",
                bigend, SDS_BIGEND);
    }
#ifdef LONG__64
    if (!long_64)
    {
        ++err;
        fprintf(stderr,"SdsCheckBuild() caller has LONG__64 undefined, SDS library compiled with LONG__64 defined\n");
    }
#else
    if (long_64)
    {
        ++err;
        fprintf(stderr,"SdsCheckBuild() caller has LONG__64 defined, SDS library compiled with LONG__64 undefined\n");
    }
#endif

#ifdef NATIVE__INT64
    if (!native_64)
    {
        ++err;
        fprintf(stderr,"SdsCheckBuild() caller has NATIVE_INT64 undefined, SDS library compiled with NATIVE_INT64 defined\n");
    }
#else
    if (native_64)
    {
        ++err;
        fprintf(stderr,"SdsCheckBuild() caller has NATIVE_INT64 defined, SDS library compiled with NATIVE_INT64 undefined\n");
    }
#endif
    if (err)
    {
        fprintf(stderr, "    SdsCheckBuild() caller has a different envrionment to built SDS\n");
        fprintf(stderr, "    sdsport.h problem?  Different compiler used?\n");
        *status = SDS__TESTERR;
    }
}



/*+				S d s C l o n e I d

 *  Function name:
      SdsCloneId

 *  Function:
      Given an SDS id, return a new SDS item pointing to the same item.

 *  Description:
      Validates the given SDS id and creates a new one pointing to the
      same item.

      This is usefully in cases such as C++ copy/assignment operators.  Whilst
      relatively cheap, it does have some cost in terms of time, particularly
      if lots of Id's are being released.
      
 *  Language:
      C

 *  Declaration:
      void SdsCloneId(SdsIdType source_id, SdsIdType *new_id, 
         StatusType * SDSCONST status)
      
 *  Parameters:   (">" input, "!" modified, "W" workspace, "<" output)
      (>) source_id (SdsIdType) Identifier to clone.
      (<) id        (SdsIdType*) New id returned here.
      (!) status    (StatusType*) Modified status. Possible failure codes are:
                            SDS__NOMEM => Insufficient memory for creation
                            SDS__BADID => parent_id is invalid.


 *  Support: Tony Farrell, {AAO}

 *  Version date: 10-Feb-2014
 *-
 */

SDSEXTERN void SdsCloneId(
    SdsIdType source_id,
    SdsIdType * SDSCONST new_id,
    StatusType * SDSCONST status)
{
    idblock *idptr;
    handle block;
    INT32 external;
    INT32 *origin;

    /* Take semaphore and access id */
    Sds__FSemTake_get_id(source_id,&block,&external,&origin,status);
    if (*status != SDS__OK) return;
    /*
     * From this point on - must release semaphore before exiting.
     */
    /*
     * Grab a new id.
     */
    Sds__get_new_id(&idptr, new_id, status);
    /*
     * Make it our clone.
     */
    Sds__put_id(*new_id,block,external,origin,status);

    Sds___FSemGive();
}



/* * * * * */



/*+				S d s G e t N a m e

 *  Function name:
      SdsGetName

 *  Function:
      Return the name of an SDS object.

 *  Description:
      Given the identifier to an object, return the name.
      
 *  Language:
      C

 *  Declaration:
      void SdsInfo(SdsIdType id, char *name, StatusType * SDSCONST status)
    
 *  Parameters:   (">" input, "!" modified, "W" workspace, "<" output)
      (<) name      (char*) The name of the data object. A pointer to a
                            character string with space for at least
                            SDS_C_NAMELEN characters should be used.
      (!) status    (StatusType*) Modified status. Possible failure codes are:
                            SDS__BADID => The identifier is invalid

 *  Prior requirements:
      None.

 *  Support: Tony Farrell, {AAO}
 *-
 */

SDSEXTERN void SdsGetName(
    SdsIdType id, 
    char *name,
    StatusType * SDSCONST status)

{
    /*idblock *idptr;*/
    handle block;
    INT32 external;
    INT32* origin;

    /* Take semaphore and access id */
    Sds__FSemTake_get_id(id,&block,&external,&origin,status);
    if (*status != SDS__OK) return;
    /*
     * From this point on - must release semaphore before exiting.
     */
    if (external)
    {
        /* External item */
        INT32 *data = (INT32 *)block;  /* External data pointer */
        strncpy(name,((char*)data)+4,SDS_C_NAMELEN);

    }
    else
    {
        /* Internal item */
        /*  Otherwise get a pointer to the sblock  */
        sblock *sb = (sblock*)(*block);

        /*  Copy the name   */

        strcpy(name,sb->name);
    }
    Sds___FSemGive();
}


/*+				S d s G e t C o d e

 *  Function name:
      SdsGetCode

 *  Function:
      Return the code of an object

 *  Description:
      Given the identifier to an object, return the type code.
      
 *  Language:
      C

 *  Declaration:
      void SdsGetCode(SdsIdType id, SdsCodeType *code, StatusType * SDSCONST status)
    
 *  Parameters:   (">" input, "!" modified, "W" workspace, "<" output)
      (>) id        (SdsIdType) The identifier to the data object.
      (<) code      (SdsCodeType*) The type code for the object. One of
                            the following values (defined in sds.h):
                               SDS_STRUCT => Structure
                               SDS_CHAR   => Character
                               SDS_BYTE   => Signed byte
                               SDS_UBYTE  => Unsigned byte
                               SDS_SHORT  => Signed short integer
                               SDS_USHORT => Unsigned short integer
                               SDS_INT    => Signed long integer
                               SDS_UINT   => Unsigned long integer
                               SDS_I64    => Signed 64 bit integer
                               SDS_UI64   => Unsigned 64 bit integer
                               SDS_FLOAT  => Floating point
                               SDS_DOUBLE => Double precision floating point
      (!) status    (StatusType*) Modified status. Possible failure codes are:
                            SDS__BADID => The identifier is invalid

 *  Prior requirements:
      None.

 *  Support: Tony Farrell, {AAO}

 *-
 */

SDSEXTERN void SdsGetCode(
    SdsIdType id, 
    SdsCodeType *code, 
    StatusType * SDSCONST status)

{
    /*idblock *idptr;*/
    handle block;
    INT32 external;
    INT32* origin;

    /* Take semaphore and access id */
    Sds__FSemTake_get_id(id,&block,&external,&origin,status);
    if (*status != SDS__OK) return;
    /*
     * From this point on - must release semaphore before exiting.
     */
    if (external)
    {
        /* External item */
        INT32 *data = (INT32 *)block;  /* External data pointer */
        *code = ((char*)data)[0];
    }
    else
    {
        /* Internal item */
        /* Get a pointer to the sblock  */
        sblock *sb = (sblock*)(*block);

        /*  Copy type code  */
        *code = sb->code;
    }
    if (*code == SDS_SARRAY)
        *code = SDS_STRUCT;

    Sds___FSemGive();
}

/*+				S d s G e t D i m s

 *  Function name:
      SdsGetDims

 *  Function:
      Return dimensions of an SDS  object

 *  Description:
      Given the identifier to an object, return the dimensions of the object.
      
 *  Language:
      C

 *  Declaration:
      void SdsGetDims(SdsIdType id, long *ndims, 
                      unsigned long *dims, StatusType * SDSCONST status)
    
 *  Parameters:   (">" input, "!" modified, "W" workspace, "<" output)
      (>) id        (SdsIdType) The identifier to the data object.
      (<) ndims     (long*) The number of dimensions if the object
                            is a primitive or structure array.
      (<) dims      (unsigned long*) The dimensions of the data. An array of
                            size at least SDS_C_MAXARRAYDIMS (7) should be 
                            allowed to receive this.
      (!) status    (StatusType*) Modified status. Possible failure codes are:
                            SDS__BADID => The identifier is invalid

 *  Prior requirements:
      None.

 *  Support: Tony Farrell, {AAO}
 *-
 */

SDSEXTERN void SdsGetDims(
    SdsIdType id, 
    long *ndims, 
    unsigned long *dims, 
    StatusType * SDSCONST status)

{
    /*idblock *idptr;*/
    handle block;
    INT32 external;
    INT32* origin;

    /* Take semaphore and access id */
    Sds__FSemTake_get_id(id,&block,&external,&origin,status);
    if (*status != SDS__OK) return;
    /*
     * From this point on - must release semaphore before exiting.
     */
    if (external)
    {
        /* External item */
        INT32 *data = (INT32 *)block;  /* External data pointer */
        SdsCodeType code = ((char*)data)[0];
        if (code != SDS_STRUCT)
        {
            int i;
            INT32 pos = 5;
            *ndims = ((short*)data)[1];
            if (code != SDS_SARRAY) pos++;
            for (i=0;i<*ndims;i++)
                dims[i] = data[pos+i];
        }
        else
            *ndims = 0;

    }
    else
    {

        /* Internal item */
       /*  Get a pointer to the sblock  */
        sblock *sb = (sblock*)(*block);
        
        /*  If it is not a structure, get the dimensions   */
        
        if (sb->code != SDS_STRUCT)
        {
            pblock *pb = (pblock*)sb;
            int i;
            *ndims = pb->ndims;
            for (i=0;i<pb->ndims;i++)  dims[i] = pb->dims[i];
        }   
        else
            *ndims = 0;
    }
    Sds___FSemGive();
}



/*+			  	S d s G e t E x t r a L e n

 *  Function name:
      SdsGetExtraLen

 *  Function:
       Return the length of any extra information field of an object.

 *  Description:
       Return the length of any extra information field of an object.

 *  Language:
      C

 *  Declaration:
      void SdsGetExtraLen(SdsIdType id, unsigned long *length, 
                          StatusType * SDSCONST status)

 *  Parameters:   (">" input, "!" modified, "W" workspace, "<" output)
      (>) id        (SdsIdType) Identifier of the object
      (<) length    (unsigned long*) length of extra data. Zero if none.
      (!) status    (StatusType*) Modified status. Possible failure codes are:
                            SDS__BADID => The identifier is invalid.

 *  Support: Tony Farrell, {AAO}

 *-
 */

SDSEXTERN void SdsGetExtraLen(
    SdsIdType id, 
    unsigned long *length, 
    StatusType * SDSCONST status)

{
    handle block;
    INT32 external;
    INT32 *origin;
   
    /* Take semaphore and access id */
    Sds__FSemTake_get_id(id,&block,&external,&origin,status);
    if (*status != SDS__OK) return;
    /*
     * From this point on - must release semaphore before exiting.
     */
    if (external)
    {
        /*
         * External item.  If we call Sds__GetExtra_ext without
         * any data pointer, we just get the length in the actlen parameter.
         */
        Sds__GetExtra_ext((INT32*)(block),0,0,length,status);
    }
    else
    {
        sblock *sb = (sblock*)(*block);
        *length = sb->nextra;
    }
    Sds___FSemGive();}


