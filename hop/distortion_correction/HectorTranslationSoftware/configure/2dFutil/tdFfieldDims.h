/*+           T D F P T

 *  Module name:
      tdFfieldDims.h

 *  Function:
      Include file containing the dimensions for the 2dF field plates.

 *  Description:
 
 *  Language:
      C

 *  Support: James Wilcox, AAO.

 *-

 *  History:
      16-Jun-1993 - JW  - Original
      20-Sep-1996 - TJF - Change OUTR_FILED_RAD from 300000 to 273500 which
			seems to be more realistic.
      28-Sep-1996 - TJF - 274500 is not write for OUTR_FIELD_RAD, goto to
      				275000 as a temp fix util we install the
      				correct algrothim.
      02-Oct-1996 - TJF - Comment out OUTR_FIELD_RA and add QUADRANT_RADIUS
			INSIDE_RADIUS OUTSIDE_RADIUS HALF_GUIDE_EXPAN  and
			tdFforbidden function definition.  These combine
			to do a better job of determining where the
			thing can go.
      26-Mar-1997 - TJF - Remove FP_GRIP_X_PARK FP_GRIP_Y_PARK FP_GRIP_Z_PARK
			FP_GRIP_J_PARK FP_GRIP_T_PARK FP_FPI_X_PARK
			FP_FPI_Y_PARK  FP_PLATE0_PARK FP_PLATE1_PARK 
			and FP_TUM_PARK since there are an equivalent set
			without the FP.   Correctly set up gantry park
			positions in the other set.
     06-May-1997 - TJF - Add FP_PIV_DO.
     17-Jun-1998 - TJF - up FPI_XY_FR_MAX and GRIP_XY_FR_MAX from 400000 to
			600000
     15-Mar-2000 - TJF - Add FIELD_RAD_INC_PARK.
     08-Jun-2004 - TJF - Reduce FIELD_RAD from 256000 to 251000 - as
                         the former value was allowing fibres to be
                         placed off the physical plate.
     04-Sep-2004 - TJF - FPI_X_PARK  and FPI_Y_PARK were wrong, they had
                         never been set correctly.  This has only been picked
                         up due to turning DPR feedback on on the FPI
                         combined with checking the positions before a tumble.
     14-Dec-2004 - TJF - 2dF is moving from 404 to 400 fibres.  Define
                         the macro NUM_PIVOTS_2DF with this value, to be
                         used throughout the 2dF software.  Also add
                         NUM_FIDS_2DF and NUM_FIELDS_2DF.
     11-Jan-2005 - TJF - Drop include of tdFpmac.h, to remove dependency
                          of this library on 2dFpmac library.
     16-Feb-2005 - TJF - Change FIELD_RAD from 251000 to 253000 after
                          checking on robot.
     31-Jul-2006 - TJF - Add NSMASK_INNER_RAD.
     08-Aug-2011 - TJF - Increase XY feedrate max from 600000 to 1000000 after 
                           robot changes.
     31-Mar-2014 - TJF - Add Z_CLEAN special Z code.
      {@change entry@}

 *     @(#) $Id: ACMM:2dFutil/tdFfieldDims.h,v 5.7 14-Mar-2016 09:20:47+11 tjf $
  
 */

#ifndef __TDFFIELDDIMS_H__
#define __TDFFIELDDIMS_H__

/*#include "tdFpmacDPR.h"  - Removed, to remove 2dFpmac dependency */

#ifdef __cplusplus
 extern "C" {
#endif

#define NUM_PIVOTS_2DF 400  /* Number of pivots in 2dF */
#define NUM_FIDS_2DF   21   /* Number of fiducials on each 2dF field */
#define NUM_FIELDS_2DF 2    /* Number of 2dF fields */


#define TUMBLER_RAD      410000  /* Tumbler radius                           */

#define FIELD_RAD        253000  /* Usable field plate radius                */
#define FIELD_RAD_INC_PARK 285000 /* Field radius include allowance for
                                     the park positions */
#define NSMASK_INNER_RAD FIELD_RAD /* Nod and shuffle mask inner radius */

/*
 *  The following set are taking from drawing E3956, 4-Jul-1996
 */ 

#define QUADRANT_RADIUS	277000	/* Radius of a quadrant of the field plate */
#define INSIDE_RADIUS	270430	/* Radius of largest circle which is inside */
				/* area which can be accessed by the gripper */
#define OUTSIDE_RADIUS  347300  /* Radius of smallest circule which is 	*/
				/* outside the retractor ring		*/
#define HALF_GUIDE_EXPAN 2250	/* The amount added between the four quadrants*/
				/* to account for the guide fibres	*/

/*
 * These have not been determined as yet 
 */
#define JAW_HWP		12220	/* Jaw half with +ve theta */
#define JAW_HWM		8300	/* Jaw half with -ve theta */	
#define JAW_LENGTH 	5250	/* Jaw length		*/



#define FIELD_RAD_SQRD 6.5536e10 /* Usable field plate radius, squared            */

#define FID_DIA             100  /* Fiducial diameter (microns)                   */
#define FID_CLEAR_SQRD  1000000  /* Clearence for fiducial mark squared - mic^2   */

#define PLATE_HOLE0_X    175000  /* Position of the mounting holes for the...     */
#define PLATE_HOLE0_Y    175000  /* ...field plate (microns)                      */
#define PLATE_HOLE1_X   -175000
#define PLATE_HOLE1_Y   -175000
#define PLATE_HOLE_DIA     6000  /* Diameter of the screw hole (microns)          */

#define XMIN            -285000  /* X-axis minimum value (microns)                */
#define XMAX             285000  /* X-axis maximum value (microns)                */
#define YMIN            -285000  /* Y-axis minimum value (microns)                */
#define YMAX             285000  /* Y-axis maximum value (microns)                */
#define ZMIN                  0  /* Z-axis minimum value (microns)                */
#define ZMAX              60000  /* Z-axis maximum value (microns)                */

/* Special valuues used with Z to arguments to the gripper to indicate
   predefined values */
#define Z_UP		-1	/* Height safe for crossing retractors */
#define Z_DOWN		-2	/* Height for putting fibre on plate	*/
#define Z_CARRY		-3	/* Height for carrying fibres		*/
#define Z_CAM		-4	/* Height for viewing fibres		*/
#define Z_CAM_FID	-5	/* Height for fibre fiducials		*/
#define Z_IT		-6 	/* Iteration Z				*/
#define Z_PARK		-7	/* Z park position			*/
#define Z_CLEAN         -8      /* Z height for cleaning                */
#define Z_SP_MIN	-8	/* Most negative of these values	*/

#define JAWMIN                0  /* JAW-axis minimum value (microns)              */
#define JAWMAX             8000  /* JAW-axis maximum value (microns)              */
#define THETAMIN        (-2*PI)  /* THETA-axis minimum value (radians)            */
#define THETAMAX         (2*PI)  /* THETA-axis minimum value (radians)            */
#define PHIMIN        (-PI/360)  /* PHI-axis (plate rotation) minimum (radians)   */
#define PHIMAX         (PI/360)  /* PHI-axis (plate rotation) maximum (radians)   */
#define TUMMIN              -45  /* Tumbler-axis minimum (degrees)                */
#define TUMMAX              225  /* Tumbler-axis maximum (degrees)                */


#define GRIP_PARKED     (GRIP_A_X | GRIP_A_Y | GRIP_A_Z)  /* Axes required to ... */
#define FPI_PARKED      (FPI_A_X | FPI_A_Y)               /* ... park gantry      */

#define GRIP_X_PARK      -260000  /* Gripper X axis park position (microns)        */
#define GRIP_Y_PARK      -260000  /* Gripper Y axis park position (microns)        */
#define GRIP_Z_PARK       50000  /* Gripper Z axis park position (microns)        */
#define GRIP_JAW_PARK         0  /* Gripper JAW axis park position (microns)      */
#define GRIP_THETA_PARK       0  /* Gripper THETA axis park position (microns)    */
#define FPI_X_PARK       331000  /* FPI X axis park position (microns)            */
#define FPI_Y_PARK      -290000     /* FPI X axis park position (microns)            */
#define PHI_PARK              0  /* Plate rotators park position (radians)        */
#define TUM_PARK              0  /* Park position of the tumbler axis (radians)   */

#define JAW_CLOSED            0  /* Approximate position of jaw when closed       */

#define FPI_XY_FR_MIN         1  /* FPI XY axes minimum feedrate value (?)        */
#define FPI_XY_FR_MAX   1000000  /* FPI XY axes maximum feedrate value (?)        */
#define PLATE_FR_MIN          1  /* Plate rotators minimum feedrate value (?)     */
#define PLATE_FR_MAX    3000000  /* Plate rotators maximum feedrate value (?)     */
#define TUMBLER_FR_MIN        1  /* Tumbler minimum feedrate value (?)            */
#define TUMBLER_FR_MAX  3000000  /* Tumbler maximum feedrate value (?)            */
#define GRIP_XY_FR_MIN        1  /* GRIP XY axes minimum feedrate value (?)       */
#define GRIP_XY_FR_MAX  1000000  /* GRIP XY axes maximum feedrate value (?)       */
#define GRIP_Z_FR_MIN         1  /* GRIP Z axis minimum feedrate value (?)        */
#define GRIP_Z_FR_MAX    200000  /* GRIP Z axis maximum feedrate value (?)        */
#define THETA_FR_MIN          1  /* GRIP theta axis minimum feedrate value (?)    */
#define THETA_FR_MAX    9000000  /* GRIP theta axis maximum feedrate value (?)    */
#define JAW_FR_MIN            1  /* GRIP jaw axis minimum feedrate value (?)      */
#define JAW_FR_MAX       200000  /* GRIP jaw axis maximum feedrate value (?)      */

#define FP_PIV_DO	13000	/* Pivot distance outside field plate */
extern int tdFforbidden ( double x, double y,
		double theta,
		double rq,
		double ri,
		double ro,
		double hw,
		double jhwp,
		double jhwm,
		double jl,
		double clearance,
		int *inside,
		int *outside);
#ifdef __cplusplus
}
#endif
#endif
