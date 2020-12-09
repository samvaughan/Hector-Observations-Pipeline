#ifndef tdFIOMaps_h
#define tdFIOMaps_h
#ifdef __cplusplus
 extern "C" {
#endif
/*                           t d F I O M a p s
  
 * Module name:
      tdFIOMaps.h
  
 * Function:
        2dF I/O Bit maps of interest to various parts of the system
  
 * Description:
  	This module defines various 2dF I/O bit maps of interest to more then
  	one part of the system.      
  
 * Language:
      C
  
 * Support: Tony Farrell, AAO
  
 * Copyright (c) Anglo-Australian Telescope Board, 1997.
   Not to be used for commercial purposes without AATB permission.
  
 *     @(#) $Id: ACMM:2dFutil/tdFiomaps.h,v 5.7 14-Mar-2016 09:20:47+11 tjf $
  
 * History:
      11-Mar-1997 - TJF - Original version
      26-Mar-1997 - TJF - Add the values required for TASK_STATE.
      10-Jun-1998 - TJF - Add BACKILLAL flag.
  
 */

/*
 *  Definitions for use with IO_READ and IO_WRITE pmac functions, since
 *  they are required thoughout the system.
 */

#define BELT_TEN    1	/* Tumbler belt is tensioned */
#define BELT_LOOSE  2	/* Tumbler belt is loose     */
#define DETENT_OUT  4	/* Tumbler detent is out     */
#define DETENT_IN   8	/* Tumbler detent is in      */
#define TUM_SAFE BELT_LOOSE|DETENT_IN /* Combination indicates safe to more gantries */
/*
 *  These bits do not work as yet (10-Mar-1997)
 */ 
#define N2_OK       0x10 /* Nitrogen pressure is ok */
#define TUM_IN_POS  0x20 /* Tumbler is in position 0, 90 or 180 */
#define GANS_PARKED 0x40 /* Gantries are all in park positions */
#define ZSAFE       0x80 /* Z height is > 25mm, safe to traverse */

/*
 *  This is used to help upper level code work out what to do when 
 *  simulation is enabled.
 */
#define IO_READ_SIM 0x100 /* Simulation is enabled at level where flag was set*/



/*
 *    Check flags which the various tasks put in their TASK_STATE parameters.
 *    We can have eight of these (4 sets must fit in a 32 bit word )
 *
 */
#define PARKED	  1	/* Gantry parked, tumbler at 0 or 180, plates zeroed */
#define SAFE      2	/* Gantry won't interfere with observations	     */
#define LAMPS_ON  4     /* Gantry lamps on				     */
#define BACKILLAL 8     /* Backillumination always on                       */

#define FIDS_P0_ON 10   /* Plates task, plate 0 fiducials on		*/
#define FIDS_P1_ON 20   /* Plates task, plate 1 fiducials on		*/

/*
 * Determine the bits used in the tdFpt TASK_STATE parameter
 */
#define GRIP_STATE_OFFSET 0
#define GRIP_STATE_BITS  0xFF

#define FPI_STATE_OFFSET 8
#define FPI_STATE_BITS   0xFF00

#define TUM_STATE_OFFSET 16
#define TUM_STATE_BITS   0xFF0000

#define PLATES_STATE_OFFSET 24
#define PLATES_STATE_BITS   0xFF000000

/*
 *  Releated values, indicating the MOVE_CLEAR positions
 */
#define GRIP_CLEAR_X -246980    /* Position of gantry clear of plate */
#define GRIP_CLEAR_Y -264476    /* Equivalent of x/y when parked     */
#define GRIP_CLEAR_Z 49900

#define FPI_CLEAR_X   253513
#define FPI_CLEAR_Y  -240488



#ifdef __cplusplus
}
#endif
#endif
