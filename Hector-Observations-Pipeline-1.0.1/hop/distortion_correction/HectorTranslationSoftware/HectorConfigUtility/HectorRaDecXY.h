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
//     For diagnostic purposes it is possible to disable the telecentricity,
//     linearity, and offset corrections, but these are enabled by default.
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
//  Author(s): Keith Shortridge, K&V  (Keith@KnaveAndVarlet.com.au)
//
//  History:
//     12th May 2019.  Original version. KS.
//     19th May 2019.  Added SetObsWavelength(). KS.
//      9th Jul 2019.  Added the ifdef..endef block for __HectorRaDecXY__. KS.
//      4th Sep 2020.  Added telecentricity correction routines. KS.
//     12th Sep 2020.  Added DisableTelecentricity() and DisableMechOffset().KS.
//     15th Jan 2021.  Added ThermalOffset() and associated variables. KS.
//     16th Jan 2021.  Added use of a DebugHandler to control debugging. KS.
//      9th Jun 2021.  Added the linearity data in I_Lin. Added I_LinFilePath to
//                     record the file name used. Initialise() now takes an
//                     argument giving the linearity file path. I_EnableLin
//                     and the routine DisableLin() added. KS.
//     11th Jun 2021.  Added Pos2RaDec() and RaDec2Pos(). KS.
//     13th Jun 2021.  Replaced TeleCorrToRaDec(), TeleCorrToXY() with,
//                     respectively TeleCorrFromXY(0), TeleCorrFromRaDec().
//                     Added GetModel(). KS.
//     23rd Nov 2021.  Introduced I_RotXyMatrix to allow experimentation with
//                     the XY coordinate system for the plate. Added RotXYMat
//                     to the Initialise() call. KS.
//
// ----------------------------------------------------------------------------------

#ifndef __HectorRaDecXY__
#define __HectorRaDecXY__

#include <string>

#include "sds.h"
#include "status.h"

#include "tdfxy.h"

#include "DebugHandler.h"

class HectorRaDecXY {
public:
   //  Constructor
   HectorRaDecXY (void);
   //  Destructor
   ~HectorRaDecXY ();
   //  Initialisation
   bool Initialise (double CenRa, double CenDec, double Mjd, double Dut,
         double AtmosTemp, double Press, double Humid, double CenWave,
         double ObsWave, double RobotTemp, double ObsTemp, double RotXyMat[],
         const std::string& DistFilePath, const std::string& LinFilePath);
   //  Get model details - required for the output file header
   bool GetModel (double Pars[], int MaxPars, int* NumPars);
   //  Modify the observing wavelength being used - eg when changing fibre types.
   bool SetObsWavelength (double ObsWave);
   //  Convert Ra,Dec to X,Y
   bool RaDec2XY (double Ra, double Dec, double* X, double* Y);
   //  Convert X,Y to ra,Dec
   bool XY2RaDec (double X, double Y, double* Ra, double* Dec);
   //  Control debugging.
   void SetDebugLevels (const std::string& Levels);
   //  Get description of latest error
   std::string GetError (void);
   //  Disable/re-enable the telecentricity correction - old setting returned.
   bool DisableTelecentricity (bool Disable);
   //  Disable/re-enable the mech offset correction - old setting returned.
   bool DisableMechOffset (bool Disable);
   //  Disable/re-enable the linearity correction - old setting returned.
   bool DisableLin (bool Disable);

   //  Calculates the telecentricity correction based on an Ra,Dec value.
   static void TeleCorrFromRaDec (double CenRa, double CenDec, 
                                  double Ra, double Dec, double X, double Y,
                                  double* CorrX, double* CorrY,
                                  bool TeleOffEnable = true,
                                  bool MechOffEnable = true,
                                  DebugHandler *Debug = nullptr);


   //  Calculate telecentricity offset in microns from angle from plate centre.
   static double TelecentricityOffsetS(double AngleRad, int* Zone = NULL,
                                       bool TeleOffEnable = true,
                                       bool MechOffEnable = true,
                                       DebugHandler *Debug = nullptr);

   //  Calculate change in position due to temperature difference
   static void ThermalOffset (double X, double Y, double Temp,
                              double TargetTemp, double CTE, double* CorrX, double* CorrY,
                               DebugHandler *Debug);


private:
   //  Utility routine to convert status codes to text.
   const std::string StatusToText (StatusType Status);
   //  Calculates the telecentricity correction based on an Ra,Dec value.
   void TeleCorrFromRaDec (double Ra, double Dec, double X, double Y,
                                             double* CorrX, double* CorrY);
   //  Calculates the telecentricity correction based on an X,Y value.
   bool TeleCorrFromXY (double X, double Y, double* CorrX, double* CorrY);
   //  Calculate telecentricity offset in microns from angle from plate centre.
   double TelecentricityOffset (double AngleRad, int* Zone = NULL);
   //  Applies both 2dF linearity and XY->RaDec corrections.
   bool Pos2RaDec (double X, double Y, double *Ra, double*Dec);
   //  Applies both 2dF RaDec -> XY and linearity corrections.
   bool RaDec2Pos (double Ra, double Dec, double *X, double *Y);
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
   //  Atmospheric temperature (K) for observation
   double I_AtmosTemp;
   //  Atmospheric pressure (mB)
   double I_Press;
   //  Atmospheric relative humidity (0 to 1.0)
   double I_Humid;
   //  Telescope pointing wavelength (microns)
   double I_CenWave;
   //  Observing wavelength (microns)
   double I_ObsWave;
   //  Temperature of plate when configured by robot (K)
   double I_RobotTemp;
   //  Temperature of plate during observation (K)
   double I_ObsTemp;
   //  Plate coefficient of thermal expansion (mircons/metre per deg C)
   double I_CTE;
   //  Flag enabling telecentricity correction - defaults to on.
   bool I_EnableTelecentricity;
   //  Flag enabling mechanical offset correction - defaults to on.
   bool I_EnableMechOffset;
   //  Flag enabling the linearity correction - defaults to on.
   bool I_EnableLin;
   //  File path for distortion SDS file.
   std::string I_DistFilePath;
   //  File path for linearityn SDS file.
   std::string I_LinFilePath;
   //  Structure holding 2dF distortion values read from SDS file.
   TdfDistType I_Dist;
   //  Structure holding 2dF linearity correction values read from SDS file.
   TdfLinType I_Lin;
   //  Structure holding combined XY transformation parameters.
   TdfXyType I_XYPars;
   //  A rotation matrix allowing X,Y positions to be flipped etc if needed.
   double I_RotXyMat[4];
   //  And its inverse.
   double I_RotXyInv[4];
   //  Description of last error, if any.
   std::string I_ErrorText;
   //  Debug handler used to control level of debugging
   DebugHandler I_Debug;
};

#endif

// ----------------------------------------------------------------------------------

/*                        P r o g r a m m i n g  N o t e s

   o  It probably isn't necessary to keep instance variables that remember all
      the values passed to Initialise() - I_Mjd through I_DistFilePath. Although
      they could be useful for some diagnostic routine that showed the current
      setup parameters.
 
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
