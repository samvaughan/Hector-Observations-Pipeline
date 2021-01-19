//
//                    H e c t o r  R a  D e c  X Y . h
//
//  Function:
//     A small utility class that does RaDec <-> X,Y conversions for Hector
//
//  Description:
//     This defines a C++ class, HectorRaDecXY, that performs conversions between
//     sky positions in Ra,Dec and field plate positions in X,Y. It has methods
//     that convert Ra,Dec to X,Y and X,Y to Ra,Dec. It also has setup methods
//     that allow the other details needed for the conversion to be specified:
//     the field plate centre in Ra,Dec, the observing time, the observing
//     conditions (temperature, pressure), the current 2dF distortion model, etc.
//     This should be all that's needed in terms of coordinate conversion for
//     Hector's fibre allocation sub-system. The allocation code needs to be able
//     to work out where on the field plate the image of a target at given
//     Ra,Dec coordinates will appear, and needs to be able to work out the Ra,Dec
//     coordinates of a sky fibre at a given point on the field plate.
//
//  Setup information needed:
//     This lists the information that must be passed to the class in order to
//     initialise the conversion routines. After that, the conversion routines
//     need only be passed the Ra,Dec or X,Y position to be converted.
//
//     These describe the observing conditions:
//
//     CenRa     (double)  Apparent RA of field centre.
//     CenDec    (double)  Apparent Dec of field centre.
//     Mjd       (double)  UTC date and time as modified julian date.
//     Dut       (double)  Delta UT (UT1 - UTC) seconds.
//     Temp      (double)  Atmospheric temperature (K).
//     Press     (double)  Atmospheric pressure (mB).
//     Humid     (double)  Atmospheric relative humidity (0 to 1.0).
//     CenWave   (double)  Telescope pointing wavelength (microns).
//     ObsWave   (double)  Instrument observing wavelength (microns).
//
//     The 2dF distortion model is traditionally held in an SDS format file
//     in a standard AAO directory. There are different distortion files for
//     the different 2dF field plates. Just how this works for Hector is yet to
//     be determined, but for the moment we assume there is a file path (directory
//     and filename) that can be used to locate the file to be used.
//
//     DirFile   (string)  The path to the distortion SDS format file.
//
//     Note that the following are not required:
//     o For 2dF, always associated with the distortion SDS file for a 2dF plate
//     is a linear model SDS file. This describes the non-linearity of the
//     relationship between XY field plate positions to positioner encoder values.
//     The same correction will presumably be needed for Hector, but it is assumed
//     this will be built into the stand-alone robot XY positioner software, and
//     is not needed here.
//     o The 2dFutil coordinate conversion routines have a calling sequence that
//     allows them to be passed the AAT pointing parameters and ADC parameters.
//     However, in practice it was determined that the only ones of these that
//     mattered were the well-determined ME and MA pointing parameters and their
//     values were hard-coded into the 2dFutil code.
//
//     The observing wavelength is the one item that might be expected to change
//     during processing of a single set of targets, so the value passed to
//     Initialise() can be overriden by a call to SetObsWavelength(). Note
//     that this needs to be called, for example, when switching from working
//     out hexabundle positions to working out guide fibre positions. It may be
//     safest to call it for each fibre - if the wavelength hasn't changed, it
//     does nothing.
//
//  Packages used:
//     Most 2dF code works through the rather abstract FPIL interface, which
//     supports a number of different fibre instruments, including OxPoz, but
//     not including SAMI. Adding an instrument to FPIL is quite complex, and a
//     lot of the work required is to support the needs of the AAOConfigure program,
//     which is not used for Hector. Hector's needs are much simpler, and it is
//     easier to use the underlying 2dFutil library (which is what FPIL uses)
//     directly from this code, which is really just a very thin layer on top of
//     the 2dFutil coordinate conversion routines.
//
//  Remaining issues:
//     o For HECTOR there is a telecentric correction that needs to be applied, due
//       to the slope of the magnets (used to correct for the curvature of the
//       focal plane). The details of this are still to be incorporated into this
//       code.
//     o There may be issues connected with the thermal expansion of the field
//       plate that will need to be dealt with.
//
//  Author(s): Keith Shortridge, K&V  (Keith@KnaveAndVarlet.com.au)
//
//  History:
//     12th May 2019.  Original version. KS.
//     19th May 2019.  Added SetObsWavelength(). KS.
//      9th Jul 2019.  Added the ifdef..endef block for __HectorRaDecXY__. KS.
//      4th Sep 2020.  Added telecentricity correction routines. KS.
//
// ----------------------------------------------------------------------------------

#ifndef __HectorRaDecXY__
#define __HectorRaDecXY__

#include <string>

#include "sds.h"
#include "status.h"

#include "tdfxy.h"

class HectorRaDecXY {
public:
   //  Constructor
   HectorRaDecXY (void);
   //  Destructor
   ~HectorRaDecXY ();
   //  Initialisation
   bool Initialise (double CenRa, double CenDec, double Mjd, double Dut,
                     double Temp, double Press, double Humid, double CenWave,
                      double ObsWave, const std::string& DistFilePath);
   //  Modify the observing wavelength being used - eg when changing fibre types.
   bool SetObsWavelength (double ObsWave);
   //  Convert Ra,Dec to X,Y
   bool RaDec2XY (double Ra, double Dec, double* X, double* Y);
   //  Convert X,Y to ra,Dec
   bool XY2RaDec (double X, double Y, double* Ra, double* Dec);
   //  Get description of latest error
   std::string GetError (void);
private:
   //  Utility routine to convert status codes to text.
   const std::string StatusToText (StatusType Status);
   //  Calculates the telecentricity correction to an X,Y value.
   void TeleCorrToXY (double Ra, double Dec, double *X, double *Y);
   //  Calculates the telecentricity correction to an Ra,Dec value.
   void TeleCorrToRaDec (double X, double Y, double *Ra, double *Dec);
   //  Calculate telecentricity offset in microns from angle from plate centre.
   double TelecentricityOffset (double AngleRad);
   //  Flag set once Initialise() has been called successfully.
   bool I_Initialised;
   //  Field plate apparent central RA.
   double I_CenRa;
   //  Field plate apparent central Dec.
   double I_CenDec;
   //  UTC observing date and time as modified julian date
   double I_Mjd;
   //  Delta UT (UT1 - UTC) seconds
   double I_Dut;
   //  Atmospheric temperature (K)
   double I_Temp;
   //  Atmospheric pressure (mB)
   double I_Press;
   //  Atmospheric relative humidity (0 to 1.0)
   double I_Humid;
   //  Telescope pointing wavelength (microns)
   double I_CenWave;
   //  Observing wavelength (microns)
   double I_ObsWave;
   //  File path for distortion SDS file.
   std::string I_DistFilePath;
   //  Structure holding 2dF distortion values read from SDS file.
   TdfDistType I_Dist;
   //  Structure holding combined XY transformation parameters.
   TdfXyType I_XYPars;
   //  Description of last error, if any.
   std::string I_ErrorText;
};

#endif

// ----------------------------------------------------------------------------------

/*                        P r o g r a m m i n g  N o t e s

   o  It probably isn't necessary to keep instance variables that remember all
      the values passed to Initialise() - I_Mjd through I_DistFilePath. Although
      they could be useful for some diagnostic routine that showed the current
      setup parameters.
 
   o  I'm still not clear how best to handle the telecentric correction. I'm
      assuming it can be done just as an additional correction to the X,Y
      value calculated by TdfRd2xy() (or passed to TdfXy2rd()).
 
   o  There's also a suggestion that there is a slight telecentric correction
      due to the small 2dF prisms performed by the 2dF code. (This was hinted
      at in an e-mail from PG.) If so, I can't find it, but if it is there this
      will have to be overridden in some way.
 
   o  I have no idea if there is actually going to be a new distortion SDS file
      produced for Hector, or whether one of the existing 2dF files will do.
      The only complication would be if thers's some question of using the
      equivalent of the 'average' distortion calculated as an average of the
      values for the two 2dF field plates, in which case two file paths might
      need to be passed to the initialsie routine. Note the complex process
      run through by AccessModel() in tdffpilfull.c to see what this can look
      like.
 
   o  The field plate expands with temperature, and this will also need to be
      dealt with. I assume this, like the telecentric correction, is just an
      additional correction to the X,Y value based on the temperature (and will
      probably need some sort of expansion coefficients - might these be the same
      as for SAMI?)
*/
