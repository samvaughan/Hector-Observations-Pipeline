#ifndef SAMICONSTSINC
#define SAMICONSTSINC
/*                                  s a m i _ c o n s t s

 * Module name:
      sami_consts.h

 * Function:
	Include file that defines the a number of SAMI constants.
      
 * Description:
        SAMI is an AAT Instrument which implements a number of IFU
        probes on the AAT Prime Focus.

        This file defines a number of constants used in the system.  In
        particular, it attempts to define these in a way that does
        not bring in other systems.  The were originally declared 
        in sami_position.h, but had to be duplicated by samigencfg.C
        to avoid that program needing to link 2dFctrlTask files.
        

 * Language:
      C++

 * Support: Tony Farrell, AAO

 * History:
      23-Feb-2015 - TJF - Start histroy.


 * "@(#) $Id: ACMM:sami/sami_consts.h,v 1.29 23-Feb-2016 12:26:11+11 tjf $"

 */


namespace SAMI {

    const unsigned FibresPerIFU=61;  /* Number of fibres per SAMI IFU */
    const unsigned FibresPerSky=1;   /* Number of fibres per SAMI Sky probe */

    // Define the number of probes of each times  Fiducial = Guide.
    const unsigned MaxFiducialProbes = 1;
    const unsigned MaxObjectProbes = 13;
    const unsigned MaxSkyProbes = 26;
    const unsigned MaxImgGuideProbes = 5; // To accommodate the new imaging guide bundles.

    /*
     * Simple in-line function to return the total number of SAMI fibres.  
     */
    inline int TotalFibres() {
        return ((FibresPerIFU*MaxObjectProbes) + (FibresPerSky * MaxSkyProbes));
    }

    const unsigned MaxProbes=MaxFiducialProbes+MaxObjectProbes+MaxSkyProbes;

}

#endif
