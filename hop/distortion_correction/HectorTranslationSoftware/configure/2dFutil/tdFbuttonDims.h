/*+           T D F P T

 *  Module name:
      tdFbuttonDims.h

 *  Function:
      Include file containing the dimensions for the 2dF fibre buttons.

 *  Description:
 
 *  Language:
      C

 *  Support: James Wilcox, John Straede, AAO.

 *-

 *  History:
      16-Jun-1993 - JW  - Original (JOS 'collision' global variables converted to
                          constants, with jaw dimensions added).
      22-Jun-1993 - JW  - Removed all 3D references following decision to move to
                          single pivot level with fibres parallel to field plate.
      13-Dec-1994 - KS  - Increased EXTENSION value to allow fibres to reach
                          the field centre. Ditto EXT_SQRD.

      16-Nov-1995 - JW  - Increased EXTENSION value (again) to allow fibres to reach
                          the field centre. Ditto EXT_SQRD.
      12-Aug-1997 - TJF - Change MAX_PIV_ANGLE to 0.18, appropriate for
				full 400 fibres on a plate.
      26-Jan-1997 - TJF - Decreased EXTENSION value back to 283000, as the
			  value of 300000 was clearly too long.  Note, 
			  283000 allows for fibre being over the center but
			  allow 5mm of slack in the movement.  Ditto EXT_SQRD
      27-Oct-1997 - TJF - Change MAX_PIV_ANGLE back to 0.25 for commissioning
			  purposes.
      22-Jun-1999 - TJF - To allow for buttons which are incorrectly made, with
                          fibre projecting from the button too much, add
			  400um to each of BLE (700->1100), BTE (4700->5100)
			  and FVP (8700->9100)
      
      {@change entry@}

 *  @(#) $Id: ACMM:2dFutil/tdFbuttonDims.h,v 5.7 14-Mar-2016 09:20:47+11 tjf $ (mm/dd/yy)
 */

#ifndef __TDFBUTTONDIMS_H__
#define __TDFBUTTONDIMS_H__

/*
              PLAN VIEW OF BUTTON, PRISM, and JAWS.
              =====================================

      PLE         BLE                         BTE       FVP
       |           |                           |         |
       v           v                           v         v

 BHW->             +---------------------------+
                   |                           |
 PHW-> +-------+---+___________________________| 
       |       |   |                           +---------+        <-THW
       |   O   |   |                           |         +-------+
       |       |   |___________________________+---------+
       +-------+---+                           |
                   |                           |
           ^       +---------------------------+
           |
           |
        origin = centre of microprism

     where:

      BHW - button half width              PHW - prism half width
      BLE - button leading edge            PLE - prism leading edge
      BTE - button trailing edge

      FVP - fibre virtual pivot            THW - tail half width
*/


/*
 *  Note: all dimensions in microns unless stated.
 */
#define  BUTTON_FVP          9100        /* Fibre virtual pivot                   */
#define  BUTTON_PHW           600        /* Prism half width                      */
#define  BUTTON_PLE          -600        /* Prism leading edge                    */
#define  BUTTON_BHW          1000        /* Button half width                     */
#define  BUTTON_BLE          1100        /* Button leading edge                   */
#define  BUTTON_BTE          5100        /* Button trailing edge                  */
#define  BUTTON_THW           300        /* Tail half width                       */

#define  FIBRE_DIA            225        /* Diameter of fibre optics              */
#define  EXTENSION         283000        /* Maximum allowed extension             */
#define  EXT_SQRD           8.0089e10        /* Maximum extension, squared            */
#define  FIBRE_CLEAR          400        /* Minimum allowed button/fibre clear.   */
#define  BUTTON_CLEAR         400        /* Minimum allowed button/button clear.  */
#define  MAX_CLEARANCE       5000        /* Maximum allowed but or fib clearances */
#define  MAX_PIV_ANGLE      0.250        /* +/- max allowed piv/fib angle (rads)  */
#define  MAX_BUT_ANGLE      0.189        /* +/- max allowed but/fib angle (rads)  */

#define  HANDLE_WIDTH         500        /* Width of the button handle (microns)  */

#define  PIVOT_PROBE_DIST   10000        /* The distance between the pivot pos &..*/
                                         /* the prism when the button is parked   */

#endif
