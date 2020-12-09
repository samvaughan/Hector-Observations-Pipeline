/*
 * This file is a copy of fpDefines.h from the ESO ozpoz fp module.
 *
 * From the point of view of configure it will probably not change.
 * It is not practical to extract the "proper" module for use here.
 *
 */

/************************************************************************
* E.S.O. - VLT project
#
# "@(#) $Id: ACMM:aaoConfigureSystem/fpDefines.h,v 8.4 14-Mar-2016 11:10:51+11 tjf $" 
*
* fpDefines.h
*
* who       when        what
* --------  ----------  ----------------------------------------------
* dpopovic  24/06/2002  Due to robot misalignement at Comm2:
*                       fpPARK_MAX_RADIUS  444500.0-->445500.0, 
*                       fpLIMIT_R_MAX_ENC  436800.0-->437800.0
* dpopovic  09/05/2002  fpSPECTROGRAPH_TEST --> fpSPECTROGRAPH_NONE
* dpopovic  15/04/2002  #define fpMDL_NUM_PARS 20
* dpopovic  07/03/2002  Paranal: fpLIMIT_R_MAX_ENC=436800.0
* tfarrell  01/02/2002  Added fpPARK_MAX_RADIUS
* dpopovic  30/11/2001  fpHOME_POS_THETA=1759751.0 (was 1762000.0)
* dpopovic  01/06/2001  Addition of GIRAFFE SLIT defines
* dpopovic  22/03/2001  Addition of Observing modes
* dpopovic  06/02/2001  Exact number (21) of fiducials on plates 1 & 2
*                       Removed non-existing fpmod and fpmodServer names
* dpopovic  04/12/2000  Exact fibre numbers. They include blank retractor
*                       positions, as well.
*                       Exact fibre types.
*                       Exact plate numbers (4 plates).
* dpopovic  01/12/2000  More realistic fibre numbers
* kpd       06/07/2000  Added fpDB_NUM_FACB & fpDB_NUM_FACB_PLATE
* dp        16/05/2000  Re-ordered fpDB_FIBRE_TYPE_<...>
* dp        03/05/2000  created ie. replaced  'fpDatabase.h' file
*------------------------------------------------------------------------
*/
/***********************************************************************
 *           Definitions common to all FP modules                      *
 ***********************************************************************/


#ifndef FP_DEFINES_H
#define FP_DEFINES_H


/* some useful constants */
#ifndef M_PI
#define M_E           2.7182818284590452354
#define M_LOG2E       1.4426950408889634074
#define M_LOG10E      0.43429448190325182765
#define M_LN2         0.69314718055994530942
#define M_LN10        2.30258509299404568402
#define M_PI          3.14159265358979323846
#define M_PI_2        1.57079632679489661923
#define M_PI_4        0.78539816339744830962
#define M_1_PI        0.31830988618379067154
#define M_2_PI        0.63661977236758134308
#define M_2_SQRTPI    1.12837916709551257390
#define M_SQRT2       1.41421356237309504880
#define M_SQRT1_2     0.70710678118654752440
#define MAXDOUBLE     ((double)1.7e+308)
#endif


/* 
 * FP Constants 
*/

/* Units */
#define fpUNITS_USER        0
#define fpUNITS_ENCODER     1


#define fpPLATE_RADIUS        440000.0  /* plate radius */
#define fpPARK_MAX_RADIUS     445500.0  /* When we park on porch,radius in plate coords */
#define fpFIBRE_MAX_OFFSET    2000.0    /* maximum fibre grasp offset */


/*
 * Robot limits and home positions. In ENCODER units!
 * NOTE: This type of data is normally stored in DB, but for safety reasons,
 * we hard-coded it, as recommended at PAA A.
 */
#define fpLIMIT_R_MIN_ENC         -7500.0  
#define fpLIMIT_R_MAX_ENC        437800.0  
#define fpLIMIT_THETA_MIN_ENC     -1000.0
#define fpLIMIT_THETA_MAX_ENC   2520000.0
#define fpHOME_POS_R             423000.0
#define fpHOME_POS_THETA        1759751.0




/* Maximum number of fibres per plate */
#define fpNUM_MAX_MEDUSA        132  /* maximum number of MEDUSA fibres */
#define fpNUM_MAX_IFU           15   /* maximum number of IFU fibres */
#define fpNUM_MAX_IFU_SKY       15   /* maximum number of IFU Sky fibres */
#define fpNUM_MAX_ARGUS_SKY     15   /* maximum number of ARGUS Sky fibres */
#define fpNUM_MAX_FACB          4    /* maximum number of FACB fibres */
#define fpNUM_MAX_TEST          11   /* maximum number of TEST fibres */
#define fpNUM_MAX_UVES          8    /* maximum number of UVES fibres per plate */
#define fpNUM_MAX_BLANK         64   /* maximum number of blank retractor positions */

/* Teoretical maximum number of fibres per plate, including the UVES fibres and blank
   retractors. To be able to use this literal in .db files, we cannot define 
   fpNUM_MAX_FIBRES as a sum of the above literals. We have to add up the total number 
   of fibres by hand! */
#define fpNUM_MAX_FIBRES        264     

/* Number of allocations for fibres on one plate . 
 * Index 0 is not used so: button number = button index.
 * Button numbers are 1..fpNUM_MAX_FIBRES */
#define fpFIBRES_ARRAY_COUNT  (fpNUM_MAX_FIBRES+1) 

#define fpNUM_MAX_FIDUCIALS   30   /* maximum number of fiducials per plate */

/* Number of allocations for fiducials on one plate. 
 * Index 0 is not used so: fiducial number = fiducial index. 
 * Button numbers are 1..fpNUM_MAX_FIDUCIALS */
#define fpFIDUC_ARRAY_COUNT   (fpNUM_MAX_FIDUCIALS+1) 

#define fpNUM_PLATES          4    /* number of plates  */

/* Total maximum number of fibres and fiducials together per plate. */
#define fpNUM_MAX_FIB_FIDUC      (fpNUM_MAX_FIBRES+fpNUM_MAX_FIDUCIALS)
/* Total maximum number of allocations for fibres and fiducials together per plate. */   
#define fpFIB_FIDUC_ARRAY_COUNT  (fpFIBRES_ARRAY_COUNT+fpFIDUC_ARRAY_COUNT) 

/* 
 * Maximum number of moves required to reconfigure the plate.
 * The worst possible case would involve parking and positioning
 * of each fibre, ie 2 moves per fibre
 */
#define fpMOVES_LIST_SIZE        (2*fpNUM_MAX_FIBRES) 

#define fpMOVES_UVES_LIST_SIZE   (2*fpNUM_MAX_UVES) 

/* 
 * Buffer for command parameters and replies 
 * Used in <mod>SendCmd()
*/
#define fpMSG_BUF_LEN              128  


#define fpNOT_USED                 0
#define fpUSED                     1

#define fpNOT_GRIPPED              0
#define fpGRIPPED                  1

#define fpCURVED                   0
#define fpSTRAIGHT                 1

#define fpFIBRE                    0
#define fpBUTTON                   fpFIBRE  /* BUTTON == FIBRE, in most cases */ 
#define fpFIDUCIAL                 1

/*
 * Types of coordinates
 */
#define fpCOORD_PLATE              0
#define fpCOORD_ROBOT              1
#define fpCOORD_CAMERA             2

/* number of fiducials on each plate */
#define fpDB_NUM_FIDUC_PLATE_1     21
#define fpDB_NUM_FIDUC_PLATE_2     21
#define fpDB_NUM_FIDUC_PLATE_3     4
#define fpDB_NUM_FIDUC_PLATE_4     4

/* number of FACB fibres per plate */
#define fpDB_NUM_FACB_PLATE        4

/* total number of FACB fibres assuming four plates*/
#define fpDB_NUM_FACB              16

/* types of fibres / back illumination */
#define fpDB_FIBRE_TYPE_NONE       0
#define fpDB_FIBRE_TYPE_FIDUC      1
#define fpDB_FIBRE_TYPE_MEDUSA     2
#define fpDB_FIBRE_TYPE_IFU        3
#define fpDB_FIBRE_TYPE_IFU_SKY    4
#define fpDB_FIBRE_TYPE_ARGUS_SKY  5
#define fpDB_FIBRE_TYPE_FACB       6
#define fpDB_FIBRE_TYPE_TEST       7
#define fpDB_FIBRE_TYPE_UVES       8
#define fpDB_FIBRE_TYPE_BLANK      9

#define fpDB_FIBRE_TYPE_NUM        9 /* Total number of fibre illumination types. 
				      * This value excludes type _NONE */

#define fpCURRENT_FIBRE_TYPE       fpDB_FIBRE_TYPE_NONE

/* types of spectrographs */                                     
#define fpSPECTROGRAPH_NONE        0
#define fpSPECTROGRAPH_GIRAFFE     1
#define fpSPECTROGRAPH_UVES        2
#define fpSPECTROGRAPH_NEW         3


/* types of IO signals */
#define fpDB_ANALOG_INPUT          0
#define fpDB_ANALOG_OUTPUT         1
#define fpDB_DIGITAL_INPUT         2
#define fpDB_DIGITAL_OUTPUT        3

/* order of something */
#define fpORDER_FIRST              1
#define fpORDER_INTERMEDIATE       2
#define fpORDER_LAST               3

/* CI server names */
#define fpSERVER_NAME_ROBOT        "fprobServer"
#define fpSERVER_NAME_ROBOTMOTORS  "fprmServer"
#define fpSERVER_NAME_GRIPPER      "fpgrpServer"

/* LCU module names */
#define fpMODULE_NAME_ROBOT        "fprob"
#define fpMODULE_NAME_ROBOTMOTORS  "fprm"
#define fpMODULE_NAME_GRIPPER      "fpgrp"

/* How to handle image processing */
#define fpDONT_GRAB_IMAGE          0  /* we have got the image in RAM */
#define fpGRAB_IMAGE               1  /* we need to take image first  */

/* Calibration Lamps */
#define fpLAMP_NONE                0
#define fpLAMP_THORIUM             1
#define fpLAMP_NEON                2
#define fpLAMP_TUNGSTEN            3
#define fpLAMP_NASMYTH             4

#define fpUSE_GRIPPER              fpLAMP_NONE

/* Used to specify what the given position is applied to */
#define fpCENTRE_BUTTON            0
#define fpCENTRE_FIBRE             1


/* Flags for actions which should be performed on buttons */
#define fpACT_PARK             1   /* Park all fibres not used in configuration */
#define fpACT_PARK_IF_NEEDED   2   /* Park if have to and leave it parked */
#define fpACT_PARK_BUT_RESTORE 3   /* If we have to park the fibre to allow
				    * other fibre to be moved, we have to 
				    * place it back to its original 
				    * position */

/* Flags indicating how to handle UVES fibres in Delta_Task */
#define fpUVES_IGNORE                0  /* ignore UVES restrictions  */
#define fpUVES_CHECK                 1  /* check  UVES restrictions */

/* Types of button moves: move or park */
#define fpMOVE_TYPE_MOVE             1
#define fpMOVE_TYPE_PARK             2
#define fpMOVE_TYPE_VIA_PARK         3  /* Move via park position */

/* Position state of button: parked or not-parked */
#define fpPOS_STATE_NOT_PARKED       0
#define fpPOS_STATE_PARKED           1

/* Observing modes */
#define fpOBSMODE_GIRAFFE_MEDUSA     "GIRMED"
#define fpOBSMODE_GIRAFFE_ARGUS      "GIRARG"
#define fpOBSMODE_GIRAFFE_IFU        "GIRIFU"
#define fpOBSMODE_UVES               "UVES"
#define fpOBSMODE_UVES_SIM           "UVES_SIM"
#define fpOBSMODE_COMBINED_MEDUSA    "COMMED"
#define fpOBSMODE_COMBINED_ARGUS     "COMARG"
#define fpOBSMODE_COMBINED_IFU       "COMIFU"


/* GIRAFFE spectrograph SLIT positions */
#define fpGIRSLIT_MEDUSA_1   "Medusa1"
#define fpGIRSLIT_MEDUSA_2   "Medusa2"
#define fpGIRSLIT_IFU_1      "IFU1"
#define fpGIRSLIT_IFU_2      "IFU2"
#define fpGIRSLIT_ARGUS      "Argus"
#define fpGIRSLIT_LONGSLIT   "LongSlit"

/*
 * Number of telescope model parameters.
 * (see fpmdl.h).
*/
#define fpMDL_NUM_PARS        20


/*
 * Macros
 */



/* database READ/WRITE access variables */
#define fpVARS_DB_READ_WRITE \
    dbATTRTYPE    attrType; \
    dbTYPE        dataType[2]; \
    char          myBuf[2000]; \
    vltINT32      actual; \
    vltUINT16     recordCnt;

/* database WRITE access variables */
#define fpVARS_DB_WRITE \
    dbTYPE        dataType[2]; \
    char          myBuf[1000]; \
    vltINT32      actual;


/* 
 * Macro to read any SCALAR type from DB. 
 * For example, to read fpgrpStatus of type vltINT32, the call 
 * would look like:
 * 
 * fpVARS_DB_READ_WRITE; - declaration of neccessary variables
 * fpDB_READ_DIRECT(fpgrp, Status, vltINT32)
 * 
 * It is assumed that the following symbolic and absolute addresses are defined:
 * 
 * static const dbSYMADDRESS fpgrpDbStatus
 * static       dbDIRADDRESS fpgrpDbAStatus
 */
#define fpDB_READ_DIRECT(prefix, var, type) \
{ \
    if (dbReadDirect(& ## prefix ## DbA ## var, dataType, myBuf, sizeof(myBuf), \
		     &actual, &recordCnt, &attrType, error) != SUCCESS) \
        ERROR_RETURN( prefix ## ERR_DBREAD, __FILE__,  "%s",  prefix ## Db ## var); \
    memcpy(& ## prefix ## var, myBuf, sizeof(type)); \
}


/* 
 * Macro to write any SCALAR type from DB. 
 * For example, to write fpgrpStatus of type vltINT32, the call 
 * would look like:
 * 
 * fpVARS_DB_READ_WRITE; - declaration of neccessary variables
 * fpgrpStatus = <some value>
 * fpDB_WRITE_DIRECT(fpgrp, Status, dbINT32, vltINT32)
 * 
 * It is assumed that the following symbolic and absolute addresses are defined:
 * 
 * static       vltINT32     fpgrpStatus
 * static const dbSYMADDRESS fpgrpDbStatus
 * static       dbDIRADDRESS fpgrpDbAStatus
 */
#define fpDB_WRITE_DIRECT(prefix, var, db_type, type) \
{ \
    dataType[0] = db_type; \
    dataType[1] = 0; \
    memcpy (myBuf, & ## prefix ## var, sizeof(type)); \
    if (dbWriteDirect(& ## prefix ## DbA ## var, dataType, myBuf, sizeof(type), \
		      &actual, 1, dbSCALAR, error) == FAILURE ) \
	ERROR_RETURN(prefix ## ERR_DBWRITE, __FILE__,  "%s", prefix ## Db ## var); \
}



#endif 	/* !FP_DEFINES_H */

