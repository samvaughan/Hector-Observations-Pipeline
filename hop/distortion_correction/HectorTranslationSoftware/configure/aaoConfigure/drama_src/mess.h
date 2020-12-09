#ifndef MESSH
#define MESSH 1
#ifdef __cplusplus
extern "C" {
#endif



/*+			M E S S . H

 *  Module name:
      MESS.H

 *  Function:
      Portable message handling routines, include file.

 *  Description:

 *  Language:
      C

 *  Support: Tony Farrell, AAO

 *-

 *  Copyright (c) Anglo-Australian Telescope Board, 1995.
    Not to be used for commercial purposes without AATB permission.
  
 * Sccs Id:     mess.h, Release 1.11, 03/24/98

 * "@(#) $Id: ACMM:aaoConfigure/drama_src/mess.h,v 12.17 14-Mar-2016 10:42:34+11 tjf $"
  
 *  History:
      27-Aug-1992 - TJF - Original version
      31-Aug-1994 - TJF - char * should be const char *
      {@change entry@}
 */
typedef struct MessTabStruct {
	    long int number;	    /* Message number	*/
	    DCONSTV char * name;	    /* Message name	*/
	    DCONSTV char * text;	    /* Message text	*/
	    } MessTabType;

typedef struct MessFacStruct {
	    long int facnum;	    /* Facility number  */
	    DCONSTV char * facname;	    /* Facility name	*/
	    MessTabType *mestable; /* Messages for this facility */
	    struct MessFacStruct *next;  /* Next in list    */
	    } MessFacType;

#define MESS_M_TEXT 0x01
#define MESS_M_IDENT 0x02
#define MESS_M_SEVERITY 0x04
#define MESS_M_FACILITY 0x08

/*
 * Under Windows, we may need to export or import the functions, depending
 * on if we are building the DLL or linking against it.
 */
#ifdef WIN32
#ifdef DRAMA_DLL  /* Am building DRAMA DLL */
#define MESSEXTERN extern __declspec(dllexport)
#elif defined(DRAMA_STATIC) /* Building a static library */
#define MESSEXTERN extern
#else           /* Am building application with DLL */
#define MESSEXTERN extern __declspec(dllimport)
#endif

#else
#define MESSEXTERN extern
#endif


MESSEXTERN int MessFacility (StatusType code);
MESSEXTERN int MessSeverity (StatusType code);
MESSEXTERN int MessNumber (StatusType code);

MESSEXTERN void MessPutFacility (MessFacType *facility);
MESSEXTERN void MessPutFlags (DCONSTV int flags);
MESSEXTERN int MessGetMsg (DCONSTV StatusType msgid, int flag,
			DCONSTV int buflen, char  *buffer);

MESSEXTERN int MessFacilityKnown(int facilityNum);

MESSEXTERN int MessStatusToSysExit(DCONSTV StatusType code);

#define STS_K_WARNING 0
#define STS_K_SUCCESS 1
#define STS_K_ERROR 2
#define STS_K_INFO 3
#define STS_K_SEVERE 4
#define STS_M_SEVERITY 7
#define STS_M_NOTSEVERITY (~STS_M_SEVERITY)

#ifdef __cplusplus
}
#endif
#endif
