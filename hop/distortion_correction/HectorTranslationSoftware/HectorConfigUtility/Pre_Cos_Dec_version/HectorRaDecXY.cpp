//
//                    H e c t o r  R a  D e c  X Y . c p p
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
//
//     For more details, see the comments at the start of HectorRaDecXY.h
//
//  Author(s): Keith Shortridge, K&V  (Keith@KnaveAndVarlet.com.au)
//
//  History:
//     12th May 2019.  Original version. KS.
//     15th Jul 2019.  Added GetError(). Fixed problem with undeclared Status
//                     variable. StatusToText() is now a method of the class, which
//                     it hadn't been before - I'd missed the HectorRaDecXY:: KS.
//      4th Sep 2020.  Added comments to each routine describing the parameters,
//                     in particular spelling out the units for each. Added
//                     code to calculate the telecentricity correction. KS.
//     12th Sep 2020.  Added the mechanical offset values supplied by Julia Bryant
//                     to the value calculated by TelecentricityOffset(). KS.
//     29th Sep 2020.  Modified the calculation in TelecentricityOffset() to match
//                     the latest values in the MagnetCasingsTilt.xslx spreadsheet
//                     provided by Julia Bryant. KS.
//     12th Sep 2020.  Added DisableTelecentricity() and DisableMechOffset(). KS.
//     15th Jan 2021.  Added ThermalOffset() and calculation of the thermal
//                     expansion correction. KS.
//     16th Jan 2021.  Added use of a DebugHandler to control debugging. KS.
//                     XY2RaDec() now allows for the zone boundary issue, where
//                     the zone based on the X,Y position is not that based on
//                     the offset X,Y position. Revised the comments about
//                     how this works. KS.
//     24th Feb 2021.  No code changes, but comments modified to explain that
//                     XY2RaDec() can be used for SkyFibres, but that the
//                     telecentricity and mech offset calculations do not apply
//                     to the SkyFibres, so these need to be disabled before
//                     XY2RaDec() is called to calculate the Ra,Dec position
//                     for a Sky fibre. KS.
//      9th Jun 2021.  Added code to support the linearity correction that Tony
//                     has explained is needed. Initialise() now takes a string
//                     giving the linearity file path and reads it into I_Lin.
//                     RaDec2XY() now calls TdfXy2pos() to apply the correction.
//                     DisableLin() has been added, setting I_EnableLin to allow
//                     the correction to be disabled for diagnostic purposes.
//                     Calls to TdfPos2XY() have been added to XY2RaDec() and to
//                     TeleCorrToRaDec(), just before calls to TdfXy2rd() to
//                     handle the reverse correction. KS.
//     11th Jun 2021.  Tidied up the code by introducing the packaged routines
//                     Pos2RaDec() and RaDec2Pos() each bundling the linearity
//                     and coordinate conversion operations into a single call.
//                     This fixes a bug where the telecentricity correction was
//                     applying a linearity correction to coordinates already
//                     corrected for linearity, which had been obscured by the
//                     more complicated code structure. KS.
//     13th Jun 2021.  Replaced TeleCorrToRaDec() and TeleCorrToXY() with,
//                     respectively TeleCorrFromXY(0) and TeleCorrFromRaDec().
//     15th Jun 2021.  Corrected the telemetry offset code, which was not
//                     respecting the signs of the X,Y positions, and so was
//                     only working in the X>0,Y>0 quadrant. Revised the
//                     calling sequence for ThermalOffset() so it returns
//                     corrected values rather than delta values. Added "Trace"
//                     "DiffMax", and "TraceOne" debug options. Added the
//                     GetModel() routine. KS.
//     16th Jun 2021.  Introduced a proper integrated calculation for the
//                     thermal correction instead of the linear approximation
//                     that didn't reverse precisely. Not because it mattered,
//                     but I wanted to understand what was going on. KS.
//     26th Jul 2021.  Made ThermalOffset() static and public, adding a DebugHandler arg.
//                     Created TelecentricityOffsetS() method, a static version
//                     of TelecentricityOffset(), passing through various extra
//                     arguments.  Created static version of TeleCorrFromRaDec()
//                     (passing through extra rguments). The intent here is to
//                     allow the basic functionality to be available to the
//                     2dF control task and the fpcal program.  They will 
//                     reimplement some bits of the conversion (as needed) but
//                     need access to these components.
//     23rd Nov 2021.  Following some confusion abput the precise orientation
//                     of the XY coordinate system, introduced I_RotXyMatrix to
//                     allow experimentation with rotating the coordinates. KS.
//

#include "HectorRaDecXY.h"

//  Slalib is needed because the telecentricity code does some of its own
//  calculations using SLA calls.

#include "slalib.h"
#include "slamac.h"

// ----------------------------------------------------------------------------------

//                          C o n s t r u c t o r
//
//  This is the only constructor. I don't like initialising things in constructors
//  as it makes it harder to report errors. So all this does is initialise the
//  various instance variables.

HectorRaDecXY::HectorRaDecXY (void) : I_Debug("RaDec")
{
   I_Initialised = false;
   I_Mjd = 0.0;
   I_Dut = 0.0;
   I_AtmosTemp = 0.0;
   I_Press = 0.0;
   I_Humid = 0.0;
   I_CenWave = 0.0;
   I_ObsWave = 0.0;
   I_RobotTemp = 0.0;
   I_ObsTemp = 0.0;
   I_DistFilePath = "";
   I_ErrorText = "";
   I_EnableTelecentricity = true;
   I_EnableMechOffset = true;
   I_EnableLin = true;
   
   //  The default rotation matrix is the identity matrix (as is its inverse,
   //  of course).
   
   I_RotXyMat[0] = 1.0;
   I_RotXyMat[1] = 0.0;
   I_RotXyMat[2] = 0.0;
   I_RotXyMat[3] = 1.0;
   I_RotXyInv[0] = 1.0;
   I_RotXyInv[1] = 0.0;
   I_RotXyInv[2] = 0.0;
   I_RotXyInv[3] = 1.0;

   //  We hard-code the coefficient of expansion for the plate, which is
   //  assumed to be invar. This is in microns/metre per degree C.
   
   I_CTE = 1.30;
   
   //  Set the list of debug levels currently supported by the Debug handler.
   //  If new calls to I_Debug.Log() or I_Debug.Logf() are added, the levels
   //  they use need to be included in this list.
   
   I_Debug.LevelsList("Diff,Offsets,Temp,DiffMax,Trace,TraceOne");
   
}

// ----------------------------------------------------------------------------------

//                            D e s t r u c t o r
//
//  The destructor releases any resources used by the code.

HectorRaDecXY::~HectorRaDecXY ()
{
   //  At the moment, there aren't any.
}

// ----------------------------------------------------------------------------------

//                        S e t  D e b u g  L e v e l s
//
//  Control the level of debug output. See the DebugHandler documentation
//  for details of the general format of a Levels string - a series of comma-
//  separated specifications, all of the form [subsystem.]level with
//  'subsystem." being optional. If subsystem is omitted or set to "RaDec"
//  or a wildcard that matches "RaDec" (matching the constructor for I_Debug),
//  then the specified level of debugging will be active and I_Debug.Log()
//  calls specifying that level will be active. Search the code for such
//  calls to see which levels are supported.

void HectorRaDecXY::SetDebugLevels (const std::string& Levels)
{
   I_Debug.SetLevels(Levels);
   
   //  TraceOne is implemented by turning on "trace" and then turning it off
   //  after the first conversion has been performed.
   
   if (I_Debug.Active("TraceOne")) I_Debug.SetLevels("Trace");
}

// ----------------------------------------------------------------------------------

//                            I n i t i a l i s e
//
//  Initialises the conversion routines, passing all the information needed
//  about the observation and the 2dF distortion.
//
//  CenRa       RA of the centre of the field plate in radians.
//  CenDec      Dec of the centre of the field plate in radians.
//  Mjd         UTC observing date and time as a modified julian date.
//  Dut         Delta UT (UT1 - UTC) in seconds.
//  AtmosTemp   Atmospheric observing temperature in degrees Kelvin.
//  Press       Observing atmospheric pressure in mB.
//  Humid       Atmospheric relative humidity (0 to 1.0).
//  CenWave     Telescope pointing wavelength in microns.
//  ObsWave     Observing wavelength in microns.
//  RobotTemp   Temperature of plate when configured by robot (deg K)
//  ObsTemp     Temperature of plate during observation (deg K)
//  RotXYMat    Rotation matrix to apply to XY plate positions (4 elements).
//  DisFilePath The path for the 2dF distortion file to use.
//  LinFilePath The path for the 2dF linearity file to use.

bool HectorRaDecXY::Initialise (
   double CenRa, double CenDec, double Mjd, double Dut, double AtmosTemp,
   double Press, double Humid, double CenWave, double ObsWave,
   double RobotTemp, double ObsTemp, double RotXyMat[],
   const std::string& DistFilePath, const std::string& LinFilePath)
{
   
   bool ReturnOK = false;
   
   StatusType Status = STATUS__OK;
   
   I_Initialised = false;
   
   I_CenRa = CenRa;
   I_CenDec = CenDec;
   I_Mjd = Mjd;
   I_Dut = Dut;
   I_AtmosTemp = AtmosTemp;
   I_Press = Press;
   I_Humid = Humid;
   I_CenWave = CenWave;
   I_ObsWave = ObsWave;
   I_RobotTemp = RobotTemp;
   I_ObsTemp = ObsTemp;
   I_DistFilePath = DistFilePath;
   I_LinFilePath = LinFilePath;
   
   //  Try to read the distortion file and the linearity file.
   
   TdfGetDist(const_cast<char*>(DistFilePath.c_str()),&I_Dist,&Status);
   if (Status != STATUS__OK) {
      I_ErrorText = "Unable to open 2dF distortion file " + DistFilePath;
   } else {
      TdfGetLin(const_cast<char*>(LinFilePath.c_str()),&I_Lin,&Status);
      if (Status != STATUS__OK) {
         I_ErrorText = "Unable to open 2dF distortion file " + LinFilePath;
      }
   }
   if (Status == STATUS__OK) {
   
      //  Initialise the tdfxy conversion routines. This combines all the supplied
      //  data into a single XY parameter structure (I_XYPars) that can be passed
      //  to the conversion routines. Note the set of dummy zero values passed
      //  as the AAT telescope model and ADC parameters. (TdfXYInit() declares all
      //  these as 'unused'.) I_CenRa and I_CenDec we just remember for later.
      
      TdfXyInit (Mjd,Dut,AtmosTemp,Press,Humid,CenWave,ObsWave,
                 0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,
                 I_Dist,&I_XYPars,&Status);
   }
   if (Status == STATUS__OK) {
   
      //  Copy the rotation matrix into the instance array variable I_RotXyMat,
      //  and calculate its inverse.
      
      if (RotXyMat) {
         for (int I = 0; I < 4; I++) I_RotXyMat[I] = RotXyMat[I];
         double Det = RotXyMat[0] * RotXyMat[3] - RotXyMat[1] * RotXyMat[2];
         if (Det == 0.0) {
            I_ErrorText =
               "XY rotation matrix has a zero determinant. Cannot be inverted.";
         } else {
            I_RotXyInv[0] = RotXyMat[3] / Det;
            I_RotXyInv[1] = -1.0 * RotXyMat[1] / Det;
            I_RotXyInv[2] = -1.0 * RotXyMat[2] / Det;
            I_RotXyInv[3] = RotXyMat[0] / Det;
         }
      }
   }
   if (Status != STATUS__OK) {
      if (I_ErrorText == "") {
         std::string StatusText = StatusToText(Status);
         if (StatusText == "") StatusText = "Unexpected error";
         I_ErrorText =
            "Failed to initialise XT conversion routines - " + StatusText;
      }
   } else {
      I_Initialised = true;
      ReturnOK = true;
   }
   
   return ReturnOK;
}

// ----------------------------------------------------------------------------------

//                       S e t  O b s  W a v e l e n g t h

//  Changes the observing wavelength being used. This needs to be called before
//  working out a position for a hexabundle or guide fibre that is operating at
//  a different wavelength to that used for the last conversion.
//
//  ObsWave     Observing wavelength in microns.

bool HectorRaDecXY::SetObsWavelength (double ObsWave)
{
   bool ReturnOK = false;
   
   if (!I_Initialised) {
      I_ErrorText =
         "Cannot modify observing wavelength - Conversion routines not initialised";
   } else {
   
      StatusType Status = STATUS__OK;
      if (ObsWave != I_ObsWave) {
         I_ObsWave = ObsWave;
         TdfXyInit (I_Mjd,I_Dut,I_AtmosTemp,I_Press,I_Humid,I_CenWave,ObsWave,
                 0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,
                 I_Dist,&I_XYPars,&Status);
         if (Status != STATUS__OK) {
            std::string StatusText = StatusToText(Status);
            if (StatusText == "") StatusText = "Unexpected error";
            I_ErrorText =
               "Failed to initialise XT conversion routines - " + StatusText;
            I_Initialised = false;
         } else {
            ReturnOK = true;
         }
      }
   }
   return ReturnOK;
}

// ----------------------------------------------------------------------------------

//                            G e t  M o d e l
//
//  Returns an array of double values that form the model used for the 2dF
//  coordinate conversions. These values are intended to be included in the
//  output Hector file where they can be used for diagnostics and to improve
//  the calibration.This routine is passed an array to receive the parameters,
//  and the number of parameters it has space for. It returns the actual number
//  of parameters. If there is a problem getting the model parameters, or if
//  the array passed is not large enough, this routine returns false, with an
//  error description in I_ErrorText. This cannot be called until Initialise()
//  has been called successfully.
//
//  Pars     Array to receive parameter values.
//  MaxPars  Number of double values that Pars can hold.
//  NumPars  Actual number of parameters.

bool HectorRaDecXY::GetModel ( double Pars[], int MaxPars, int* NumPars)
{
   bool ReturnOK = false;
   
   if (!I_Initialised) {
      I_ErrorText =
         "Cannot get model parameters - Conversion routines not initialised";
   } else {
      if (MaxPars < TDFXY_NUM_PARS) {
         I_ErrorText = "Cannot get model parameters - array too small";
      } else {
         StatusType Status = STATUS__OK;
         TdfModelPars Model;
         TdfGetModel (I_Dist,I_Lin,&Model,&Status);
         if (Status != STATUS__OK) {
            std::string StatusText = StatusToText(Status);
            if (StatusText == "") StatusText = "Unexpected error";
            I_ErrorText = "Failed to get model parameters - " + StatusText;
         } else {
            for (int I = 0; I < TDFXY_NUM_PARS; I++) {
               Pars[I] = Model.p[I];
            }
            *NumPars = TDFXY_NUM_PARS;
            ReturnOK = true;
         }
      }
   }
   return ReturnOK;
}

// ----------------------------------------------------------------------------------

//                            X Y  2  R a  D e c
//
//  Converts X,Y coordinates on the field plate to an apparent RA,Dec position
//  on the sky. This is an X,Y position at which a Hector magnet is placed, and
//  so takes into account not only the standard 2dF coordinate conversion, but
//  also the offsets due to telecentricity and magnet offset, and the thermal
//  correction required. This routine can also be used to calculate the Ra,Dec
//  position corresponding to the X,Y position of a Hector sky fibre, but
//  only if the telecentricity and magnet offsets are disabled before this is
//  called, as the sky fibre positions used for Hector already allow for
//  such effects.
//
//  X       Field plate X coordinate in microns.
//  Y       Field plate Y coordinate in microns.
//  Ra      Calculated apparent RA in radians.
//  Dec     Calculated apparent Dec in radians.

bool HectorRaDecXY::XY2RaDec (
   double X, double Y, double* Ra, double* Dec)
{
   bool ReturnOK = false;
   
   if (!I_Initialised) {
      I_ErrorText =
         "Cannot convert X,Y to Ra,Dec - Conversion routines not initialised";
   } else {
   
      //  Apply the rotation matrix introduced to allow for experimentation
      //  with the plate coordinate axes.
      
      I_Debug.Logf ("Trace","In XY2RaDec, X Y %f %f",X,Y);
      double XRot,YRot;
      XRot = (X * I_RotXyMat[0]) + (Y * I_RotXyMat[1]);
      YRot = (X * I_RotXyMat[2]) + (Y * I_RotXyMat[3]);
      I_Debug.Logf ("Trace","In XY2RaDec, axis rotation gives X, Y %f %f\n",
                                                                  XRot,YRot);
   
      //  The X,Y positions will be those set by the robot during configuration,
      //  but these will need to be modified to allow for thermal expansion.
      
      double CorrX,CorrY;
      ThermalOffset (XRot,YRot,I_ObsTemp,I_RobotTemp,I_CTE,&CorrX,&CorrY,&I_Debug);
      double LocalX = CorrX;
      double LocalY = CorrY;
      I_Debug.Logf ("Trace","In XY2RaDec, Thermal offset: %f %f X Y now %f %f",
                                    CorrX - XRot,CorrY - YRot,CorrX,CorrY);

      //  Now apply the telecentricity correction.

      //  The sequence performed by TeleCorrFromXY() is actually quite
      //  messy. It's being passed X and Y and an initial estimate at Ra,Dec.
      //  It's going to use that initial Ra,Dec position to work out the
      //  Hector magnet zone and hence the offset in X & Y. It will then
      //  apply that offset to the X,Y value it's passed and call TdfXy2rd()
      //  again to get a new Ra,Dec that has taken the magnet offset into
      //  account. Not only that, it will then use that new Ra,Dec value to
      //  redetermine the zone, to see if applying the offset has moved the
      //  resulting Ra,Dec position into a different zone (which happens very
      //  rarely, for positions right on the zone boundaries). If so, it will
      //  recalculate the offset on the basis of that changed zone - and that
      //  will involve yet another call to TdfXy2rd().
      
      if (TeleCorrFromXY (LocalX,LocalY,&CorrX,&CorrY)) {
         I_Debug.Logf ("Trace",
                  "In XY2RaDec, TeleCorrfromXY %f %f X Y now %f %f",
                           CorrX - LocalX,CorrY - LocalY,CorrX,CorrY);
         LocalX = CorrX;
         LocalY = CorrY;

         //  Convert this X,Y position (as corrected) to the equivalent Ra,Dec.
      
         if (Pos2RaDec (LocalX,LocalY,Ra,Dec)) {
            I_Debug.Logf ("Trace",
               "In XY2RaDec, Finally, X Y %f %f gives Ra,Dec %f %f",
                                                  LocalX,LocalY,*Ra,*Dec);
            ReturnOK = true;
         }
      }
   }
   return ReturnOK;
}

// ----------------------------------------------------------------------------------

//                            R a  D e c  2  X Y
//
//  Converts an apparent RA,Dec position on the sky to X,Y coordinates on the
//  field plate. This is an X,Y position at which a Hector magnet should be
//  in order to observe a target at that specified Ra,Dec position, and
//  so takes into account not only the standard 2dF coordinate conversion, but
//  also the offsets due to telecentricity and magnet offset and the thermal
//  correction required.
//
//  Ra      Apparent RA in radians.
//  Dec     Apparent Dec in radians.
//  X       Calculated field plate X coordinate in microns.
//  Y       Calculated field plate Y coordinate in microns.

bool HectorRaDecXY::RaDec2XY (
   double Ra, double Dec, double* X, double* Y)
{
   bool ReturnOK = false;
   
   if (!I_Initialised) {
      I_ErrorText =
         "Cannot convert Ra,Dec to X,Y - Conversion routines not initialised";
   } else {
   
      //  Apply the standard 2dF coordinate conversions, including the
      //  linearity correction (if enabled, which it usually will be).
      
      if (RaDec2Pos (Ra,Dec,X,Y)) {
         I_Debug.Logf ("Trace",
                 "In RaDec2XY, RaDec2Pos: Ra, Dec %f %f, X Y %f %f",
                                                           Ra,Dec,*X,*Y);
      
         //  Apply the telecentricity and magnet offset correction. Note that
         //  this uses the Ra,Dec position of the target to determine the zone
         //  and hence the mechanical characteristics of the Hector magnet.
         //  There may be issues near a zone boundary, where a magnet for
         //  either zone might be used, but where each would need a different
         //  X,Y position, as their offsets would be different. However, this
         //  code always picks the zone strictly on the basis of the Ra,Dec
         //  position.
         
         double CorrX,CorrY;
         TeleCorrFromRaDec (Ra,Dec,*X,*Y,&CorrX,&CorrY);
         I_Debug.Logf ("Trace",
               "In RaDec2XY, TelleCorrFromRaDec: %f %f X Y now %f %f",
                                       CorrX - *X,CorrY - *Y,CorrX,CorrY);
         *X = CorrX;
         *Y = CorrY;

         //  The X,Y position will be that at observing time and at observing
         //  temp. We need the position the robot will use at configuration
         //  time.
         
         ThermalOffset (*X,*Y,I_RobotTemp,I_ObsTemp,I_CTE,&CorrX,&CorrY, &I_Debug);
         I_Debug.Logf ("Trace",
                "In RaDec2XY, Thermal offset: %f %f X Y now %f %f",
                                       CorrX - *X,CorrY - *Y,CorrX,CorrY);
         *X = CorrX;
         *Y = CorrY;
         
         //  Apply the inverse rotation matrix so this X,Y result is in the
         //  same coordinate orientation as is used for the sky fibre positions.
         
         double XRot = *X;
         double YRot = *Y;
         *X = (XRot * I_RotXyInv[0]) + (YRot * I_RotXyInv[1]);
         *Y = (XRot * I_RotXyInv[2]) + (YRot * I_RotXyInv[3]);
         I_Debug.Logf ("Trace",
            "In RaDec2XY, axis rotation %f %f X Y now %f %f",XRot,YRot,*X,*Y);

         //  Just for fun, convert that back to Ra Dec and compare. This at
         //  least checks if the coordinate conversion code can be reversed
         //  accurately enough.
         
         if (I_Debug.Active("Diff") || I_Debug.Active("DiffMax")) {
            double Ra2,Dec2;
            static double Mdiff = 0.0;
            double MdiffWas = Mdiff;
            XY2RaDec (*X,*Y,&Ra2,&Dec2);
            if (fabs(Ra2-Ra) > Mdiff) Mdiff = fabs(Ra2-Ra);
            if (fabs(Dec2-Dec) > Mdiff) Mdiff = fabs(Dec2-Dec);
            char Text[1024];
            snprintf (Text,sizeof(Text),
                  "Ra,Dec [%f,%f] -> [%f,%f] -> Ra2,Dec2 [%f,%f]"
                  " differences: [%f,%f] max diff %f (asec)",
                  Ra,Dec,*X,*Y,Ra2,Dec2,fabs(Ra2-Ra) * DR2D * 3600.0,
                         fabs(Dec2-Dec) * DR2D * 3600.0, Mdiff * DR2D * 3600.0);
            I_Debug.Log("Diff",std::string(Text));
            if (Mdiff > MdiffWas) {
               I_Debug.Logf("DiffMax","Maximum difference now %f (asec)",
                                                    Mdiff * DR2D * 3600.0);
            }
         }
         
         ReturnOK = true;
      }
   }
   if (I_Debug.Active("TraceOne")) I_Debug.UnsetLevels("Trace");

   return ReturnOK;
}

// ----------------------------------------------------------------------------------

//                    T e l e  C o r r  F r o m  R a  D e c
//
//  Given an apparent RA,Dec position on the sky and the corresponding X,Y
//  coordinates on the field plate calculated on the basis of 2dF distortion,
//  applies the telecentricity correction due to the various prism angles used
//  by Hector and returns the new X,Y values corrected for telecentricity.
//
//  Ra      Apparent RA in radians.
//  Dec     Apparent Dec in radians.
//  X       Calculated field plate X coordinate in microns.
//  Y       Calculated field plate Y coordinate in microns.
//  CorrX   X value with the telecontricity correction applied.
//  CorrY   Y value with the telecontricity correction applied.

void HectorRaDecXY::TeleCorrFromRaDec (
   double Ra, double Dec, double X, double Y, double* CorrX, double* CorrY) {


  /*
   * Invoke the static version passing through the member variable
   * values it needs.
   */
  TeleCorrFromRaDec(I_CenRa, I_CenDec, Ra, Dec, X, Y, CorrX, CorrY, 
                    I_EnableTelecentricity, I_EnableMechOffset, &I_Debug);
}

// ----------------------------------------------------------------------------------

//                    T e l e  C o r r  F r o m  R a  D e c
//
//  Static version of TeleCorrFromRaDec(), in which the field centre position
//   is specified and there is no dependence on an object of the class.  Used
//   to implement the non-static version and allows external access.
//
//  Given a field centre and apparent RA,Dec position on the sky and the corresponding X,Y
//  coordinates on the field plate calculated on the basis of 2dF distortion,
//  applies the telecentricity correction due to the various prism angles used
//  by Hector and returns the new X,Y values corrected for telecentricity.
//
//  CenRa   Apparent RA of field centre, radians.
//  CenDec  Apparent Dec of field centre, radians.
//  Ra      Apparent RA in radians.
//  Dec     Apparent Dec in radians.
//  X       Calculated field plate X coordinate in microns.
//  Y       Calculated field plate Y coordinate in microns.
//  CorrX   X value with the telecontricity correction applied.
//  CorrY   Y value with the telecontricity correction applied.
//  Debug   If non-null, the address of a debug handler.

void HectorRaDecXY::TeleCorrFromRaDec (
    double CenRa, double CenDec, double Ra, double Dec, 
    double X, double Y, double* CorrX, double* CorrY,
    bool TeleOffEnable,
    bool MechOffEnable,
    DebugHandler *Debug) 
{

   //  Given the Ra and Dec and the central Ra Dec of the field plate, work out
   //  the angle between the two positions. This angle determines the prism that
   //  will be used for this position.
   
   double AngleRad = slaDsep (CenRa, CenDec, Ra, Dec);
   
   //  And that in turn determines the offset.
   
   double Offset = TelecentricityOffsetS (AngleRad, nullptr, TeleOffEnable, MechOffEnable, Debug);
   
   //  The rest is simple trig.
   
   double R = sqrt((X * X) + (Y * Y));
   *CorrX = X * (R + Offset) / R;
   *CorrY = Y * (R + Offset) / R;
}


// ----------------------------------------------------------------------------------

//                               P o s 2 R a D e c
//
//  This routine packages up the calls to the two 2dF routines TdfPos2xy() and
//  TdfXy2rd(). This goes from a position on the field plate (which is specified
//  by an X,Y position, but one that has not been corrected for linearity) and
//  converts it first to a linearity-corrected X,Y position and then to a sky
//  position in Ra,Dec. It's convenient to package these up into one routine,
//  which also provides a possible place to add diagnostic output if needed.
//
//  If all goes well, this routine returns true. If there is an error, it
//  returns false and sets an error description into I_ErrorText.
//

bool HectorRaDecXY::Pos2RaDec (double X, double Y, double *Ra, double*Dec)
{
   bool ReturnOK = true;
   double LinCorrX = X;
   double LinCorrY = Y;
   if (I_EnableLin) TdfPos2xy (&I_Lin,X,Y,&LinCorrX,&LinCorrY);
   StatusType Status = STATUS__OK;
   TdfXy2rd (&I_XYPars,I_CenRa,I_CenDec,LinCorrX,LinCorrY,I_Mjd,Ra,Dec,&Status);
   if (Status != STATUS__OK) {
      ReturnOK = false;
      std::string StatusText = StatusToText(Status);
      if (StatusText == "") StatusText = "Unexpected conversion error";
      I_ErrorText = "Cannot convert X,Y to Ra,Dec - " + StatusText;
   }
   return ReturnOK;
}

// ----------------------------------------------------------------------------------

//                               R a D e c 2 P o s
//
//  This routine packages up the calls to the two 2dF routines TdfRd2xy() and
//  TdfXy2pos(). This goes from a position on the sky in Ra,Dec to an X,Y
//  position on the field plate (which is corrected for linearity) and finally
//  reverses the linearity correction to get an X,Y position. This is a pair
//  with Pos2RaDec() and each one performs the inverse of the operation
//  performed by the other. It's convenient to package each of these up into
//  one routine, which also provides a possible place to add diagnostic output
//  if needed.
//
//  If all goes well, this routine returns true. If there is an error, it
//  returns false and sets an error description into I_ErrorText.
//

bool HectorRaDecXY::RaDec2Pos (double Ra, double Dec, double *X, double *Y)
{
   bool ReturnOK = true;
   double LinCorrX;
   double LinCorrY;
   StatusType Status = STATUS__OK;
   TdfRd2xy (&I_XYPars,I_CenRa,I_CenDec,Ra,Dec,I_Mjd,&LinCorrX,&LinCorrY,
                                                                   &Status);
   *X = LinCorrX;
   *Y = LinCorrY;
   if (Status != STATUS__OK) {
      ReturnOK = false;
      std::string StatusText = StatusToText(Status);
      if (StatusText == "") StatusText = "Unexpected conversion error";
      I_ErrorText = "Cannot convert Ra,Dec to X,Y - " + StatusText;
   } else {
      if (I_EnableLin) TdfXy2pos (&I_Lin,LinCorrX,LinCorrY,X,Y);
   }
   return ReturnOK;
}

// ----------------------------------------------------------------------------------

//                      T e l e  C o r r  F r o m  X Y
//
//  Given X,Y coordinates on the field plate and the corresponding apparent RA,Dec
//  position on the sky calculated on the basis of 2dF distortion, calacultes the
//  telecentricity correction due to the various prism angles used by Hector and
//  returns the telecentricity corrected X,Y values.
//
//  X       Field plate X coordinate in microns.
//  Y       Field plate Y coordinate in microns.
//  CorrX   X value with the telecontricity correction applied.
//  CorrY   Y value with the telecontricity correction applied..

bool HectorRaDecXY::TeleCorrFromXY (
   double X, double Y, double* CorrX, double* CorrY) {

   bool ReturnOK = false;
   
   //  This sequence is a lot messier than that used by TeleCorrFromRaDec(), as
   //  we need the angle between the field centre and the position in question,
   //  and it's easiest to get that from the calculated Ra,Dec. It's complicated
   //  even more by the question of what happens on the boundary between two
   //  Hector zones, as this code assumes that it is the Ra,Dec position that
   //  determines the zone and hence the offset, so there is an element of
   //  iteration involved here.

   double Ra,Dec;
   if (Pos2RaDec (X,Y,&Ra,&Dec)) {

      //  Given the Ra and Dec and the central Ra Dec of the field plate, work out
      //  the angle between the two positions. This angle determines the prism that
      //  will be used for this position.
      
      double AngleRad = slaDsep (I_CenRa, I_CenDec, Ra, Dec);
      
      //  And that in turn determines the offset.
      
      int Zone = 0;
      double Offset = TelecentricityOffset (AngleRad,&Zone);
      
      //  Given that, we can now work out the X and Y positions in the field plate
      //  without the offset. Note that the offset is applied in the opposite
      //  direction to that used in TeleCorrFromRaDec().
      
      double R = sqrt (X * X + Y * Y);
      double NewY = Y * (R - Offset) / R;
      double NewX = X * (R - Offset) / R;

      double NewRa, NewDec;
      if (Pos2RaDec (NewX,NewY,&NewRa,&NewDec)) {

         //  This is where it gets a bit tricky. This new Ra,Dec may fall into a
         //  different Hector zone, giving a different offset. If so, then we have
         //  used the wrong offset (assuming that the Ra,Dec position determines
         //  the zone and hence the magnet type used by Hector), and have to
         //  recalculate on the basis of the new zone.

         AngleRad = slaDsep (I_CenRa, I_CenDec, NewRa, NewDec);
         int NewZone = 0;
         double NewOffset = TelecentricityOffset (AngleRad,&NewZone);
         if (NewZone != Zone) {
            I_Debug.Logf ("Offsets",
               "Telecentricity offset crosses boundary of Zones %d and %d",
                                                                 NewZone,Zone);
            R = sqrt (X * X + Y * Y);
            NewY = Y * (R - NewOffset) / R;
            NewX = X * (R - NewOffset) / R;
         }
         *CorrX = NewX;
         *CorrY = NewY;
         ReturnOK = true;
      }
   }
   
   return ReturnOK;
}

// ----------------------------------------------------------------------------------

//                  T e l e c e n t r i c i t y  O f f s e t
//
//  Calculates the offset in microns radially outward from a given field plate
//  position (specified in terms of the angle between the coordiantes it represents
//  and the field centre, so a point at the field centre is at an angle of zero,
//  one at the edge of the 1 degree field of the instrument will be at an angle
//  of 1.0 deg - but note that the angle is passed to this routine in radians, just
//  because all angles in the calls to HectorRaDecXY routines are in radians, and
//  consistency is a virtue). Note that the returned offset will always be -ve,
//  because there are two offsets involved, one outward and the other inward and
//  the inward is always larger. (Originally, the code ignored the inward offset,
//  which is why it was written to return an outward offset.) If ZonePtr is
//  passed as non-null, this should be the address of an int that will be set
//  to the Hector zone number corresponding to that angle.
//
//  AngleRad    The angle to the position in question, in radians.
//  ZonPtr      If non-null, return the calculated zone here

double HectorRaDecXY::TelecentricityOffset (double AngleRad, int* ZonePtr) {

  // Call the static version, passing through various values from the object.
  return TelecentricityOffsetS(AngleRad, ZonePtr, 
                               I_EnableTelecentricity,
                               I_EnableMechOffset,
                               &I_Debug);
                        
}

//                  T e l e c e n t r i c i t y  O f f s e t S
//
//  Static version of the above TelecentricityOffset().
//
//  Calculates the offset in microns radially outward from a given field plate
//  position (specified in terms of the angle between the coordiantes it represents
//  and the field centre, so a point at the field centre is at an angle of zero,
//  one at the edge of the 1 degree field of the instrument will be at an angle
//  of 1.0 deg - but note that the angle is passed to this routine in radians, just
//  because all angles in the calls to HectorRaDecXY routines are in radians, and
//  consistency is a virtue). Note that the returned offset will always be -ve,
//  because there are two offsets involved, one outward and the other inward and
//  the inward is always larger. (Originally, the code ignored the inward offset,
//  which is why it was written to return an outward offset.) If ZonePtr is
//  passed as non-null, this should be the address of an int that will be set
//  to the Hector zone number corresponding to that angle.
//
//  AngleRad       The angle to the position in question, in radians.
//  ZonPtr         If non-null, return the calculated zone here
//  TeleOffEnable  Enable the Telecentricity correction.
//  MechOffEnable  Enable the mechanical offset correction.
//  Debug      If non-null, the address of a debug handler.

double HectorRaDecXY::TelecentricityOffsetS (
    double AngleRad, int* ZonePtr,
    bool TeleOffEnable,
    bool MechOffEnable,
    DebugHandler *Debug) {


   //  In Peter Gillingham's document 2dF_distortion_prism_effects.pdf, there is a
   //  table and a graph showing the offsets as they vary depending on the field
   //  angle, in four distinct bands corresponding to the four prism angles used.
   //  The minimum offset is 31 microns, increasing to between 116 and 118 microns
   //  in the outermost band. There is an additional mechanical offset connected
   //  with the position of the top centre of the prism compared to the centre of
   //  the magnet position, which comes from a table supplied by Julia Bryant - in
   //  an e-mail to me (KS) dated 11 Sep 2020. The two offsets need to be combined
   //  together to give the total offset. This was clarified somewhat in an Excel
   //  spreadsheet from Julia, 14/9/20, MagnetCasingsTilt.xlsx, which makes clear
   //  that what I call here the machanical offset, and which is described in the
   //  spreadsheet as a horizontal offset, in an offset radially inwards, while
   //  what I call here the optical offset and which the spreadsheet calls Peter's
   //  "Hector minus 2dF um" figure is a shift radially outwards. Note that the
   //  calculations in the code use values in microns throughout.
   
   int Zone = 0;
   double Offset = 31.0;
   double MechOffset = 1173.0;
   double AngleDeg = AngleRad * DR2D;
   if (AngleDeg < 0.396) {
      //  In the angle range 0 to 0.396 degrees, the prism angle offset is a
      //  constant 31 microns, and the mechanical offset is 0.1173mm.
      Offset = 31.0;
      MechOffset = 117.3;
      Zone = 1;
   } else if (AngleDeg < 0.627) {
      //  In the angle range 0.396 to 0.627 degrees, the prism angle offset is a
      //  constant 64 microns, and the mechanical offset is 0.2382mm.
      Offset = 64.0;
      MechOffset = 238.2;
      Zone = 2;
   } else if (AngleDeg < 0.823) {
      //  In the angle range 0.627 to 0.823 degrees, the prism angle offset
      //  increases linearly between 94 and 95 microns, and the mechanical offset
      //  is 0.3468mm.
      Offset = 94.0 + (95.0 - 94.0) * (AngleDeg - 0.627) / (0.823 - 0.627);
      MechOffset = 346.8;
      Zone = 3;
   } else if (AngleDeg < 1.0) {
      //  In the angle range 0.823 to 1.0 degrees, the prism angle offset decreases
      //  linearly between 118 and 116 microns and the mechanical offset is 0.4529mm.
      Offset = 118.0 + (116.0 - 118.0) * (AngleDeg - 0.823) / (1.0 - 0.823);
      MechOffset = 452.9;
      Zone = 4;
   } else {
      //  This shouldn't happen, but we trap it anyway.
      Offset = 116.0;
      MechOffset = 452.9;
   }
   
   //  We normally make use of both offsets, but it is possible to disable
   //  each independently. Note that the mechanical offset operates in the
   //  opposite drection to the telecentricity offset.
   
   double FinalOffset = 0.0;
   if (TeleOffEnable) FinalOffset = Offset;
   if (MechOffEnable) FinalOffset -= MechOffset;

   //  This may be a useful diagnostic to enable
   
   if ((Debug)&&Debug->Active("Offsets")) {
      char Text[1024];
      snprintf (Text,sizeof(Text),
              "Angle %.4f deg, Zone %d, Tele offset: %8.2f mu%s,"
              " Mech offset: %8.2f mu%s",
                AngleDeg,Zone,Offset, (TeleOffEnable ? "" : " (disabled)"),
                MechOffset, (MechOffEnable ? "" : " (disabled)"));
      Debug->Log("Offsets",std::string(Text));
   }
   
   //  Return the calculated zone, if that was wanted.
   
   if (ZonePtr) *ZonePtr = Zone;

   return FinalOffset;
}

// ----------------------------------------------------------------------------------

//                        T h e r m a l  O f f s e t
//
//  Static member
//
//  Calculates the change in X,Y position on the plate due to the difference
//  between the temperature of the plate at a given tempreature and the
//  the temperature of the plate at a target temperature.
//
//  X          Plate position in X (with 0,0 at the plate centre) in microns.
//  Y          Plate position in Y (with 0,0 at the plate centre) in microns.
//  Temp       Current emperature of plate, in Deg K.
//  TargetTemp Target temperature of plate, in Deg K.
//  CTE        Plate coefficient of thermal expansion (microns/metre per deg C).
//  CorrX      Corrected plate position in X.
//  CorrY      Corrected plate position in Y.
//  Debug      If non-null, the address of a debug handler.
//             
//
//  Note this routine can be used for both the forward and reverse corrections
//  from Ra,Dec to X,Y and Yx,Y back to Ra,Dec, but the current and target
//  temperatures need to be reversed in the two cases.
//
void HectorRaDecXY::ThermalOffset (
    double X, double Y, double Temp,
    double TargetTemp, double CTE, double* CorrX, double* CorrY,
    DebugHandler *Debug)
{
   static const double MicronsPerMetre = 1.0e6;
   double DeltaT = Temp - TargetTemp;
   double R = sqrt((X * X) + (Y * Y));
   double Alpha = CTE / MicronsPerMetre;
   
   //  We could use the simple approximation
   //  Offset = R * Alpha * DeltaT
   //  to get the expansion of something currently R long when the temp changes
   //  by DeltaT. But because R is changing constantly as the temp changes, this
   //  is only an approximation, and doesn't give the same value (with the sign
   //  changed) when the temperatures are reversed. Instead, we can solve the
   //  integral equation, which involves the integral of 1/L wrt L (which is
   //  ln(L), which is where the exp() comes from). This is too hard for C++
   //  comments, and I got most of this from
   //  https://www.miniphysics.com/uy1-thermal-expansion.html
   //  because I'd forgotten most of my integral calculus. But this expression
   //  does reverse properly.
   
   double Offset = R * (exp(Alpha * DeltaT) - 1);
   
   *CorrX = X * (R + Offset) / R;
   *CorrY = Y * (R + Offset) / R;
   if ((Debug)&&(Debug->Active("Temp"))) {
      char Text[1024];
      snprintf (Text,sizeof(Text),
         "X,Y [%f,%f], Temp %.2f (K) Target temp %.2f (K), DeltaX,Y [%f,%f] mu",
                                     X,Y,Temp,TargetTemp,*CorrX - X,*CorrY - Y);
      Debug->Log("Temp",std::string(Text));
   }
}

// ----------------------------------------------------------------------------------

//                            G e t  E r r o r
//
//  Returns a description of the latest error.

std::string HectorRaDecXY::GetError (void) {
   return I_ErrorText;
}

// ----------------------------------------------------------------------------------

//                 D i s a b l e  T e l e c e n t r i c i t y
//
//  Can be used to disable (or re-enable) the application of the telecentricity
//  offset for diagnostic purposes. By default this is enabled. If this routine
//  is called with the Disable argument set true, then it will be disabled.
//  Calling this with Disable set false will re-enable it. The routine always
//  returns the previous setting.
//
//  Note that this routine is called 'Disable' to emphasise that the offset
//  is usually enabled, and the instance variable used is called 'Enable'
//  because that seems to emphasise the same thing.

bool HectorRaDecXY::DisableTelecentricity (bool Disable) {

   bool Previous = !I_EnableTelecentricity;
   I_EnableTelecentricity = !Disable;
   return Previous;
}

// ----------------------------------------------------------------------------------

//                 D i s a b l e  M e c h  O f f s e t
//
//  Can be used to disable (or re-enable) the application of the mechanical
//  offset for diagnostic purposes. By default this is enabled. If this routine
//  is called with the Disable argument set true, then it will be disabled.
//  Calling this with Disable set false will re-enable it. The routine always
//  returns the previous setting.
//
//  Note that this routine is called 'Disable' to emphasise that the offset
//  is usually enabled, and the instance variable used is called 'Enable'
//  because that seems to emphasise the same thing.

bool HectorRaDecXY::DisableMechOffset (bool Disable) {

   bool Previous = !I_EnableMechOffset;
   I_EnableMechOffset = !Disable;
   return Previous;
}

// ----------------------------------------------------------------------------------

//                         D i s a b l e  L i n
//
//  Can be used to disable (or re-enable) the application of the linearity
//  correction for diagnostic purposes. By default this is enabled. If this
//  routine is called with the Disable argument set true, then it will be
//  disabled. Calling this with Disable set false will re-enable it. The
//  routine always returns the previous setting.
//
//  Note that this routine is called 'Disable' to emphasise that the offset
//  is usually enabled, and the instance variable used is called 'Enable'
//  because that seems to emphasise the same thing.

bool HectorRaDecXY::DisableLin (bool Disable) {

   bool Previous = !I_EnableLin;
   I_EnableLin = !Disable;
   return Previous;
}



// ----------------------------------------------------------------------------------

//                       S t a t u s  T o  T e x t
//
//  Converts a status code from the TdfXy routines to a text string. If the code
//  is not recognised, an empty string is returned.

const std::string HectorRaDecXY::StatusToText (StatusType Status)
{
   std::string Text = "";
   
   //  This is crude, and comes direct from the .msg file. But it's easier than
   //  using a more complex Ers routine.
   
   if (Status == TDFXY__ILLRA) Text = "Illegal RA";
   else if (Status == TDFXY__ILLDEC) Text = "Illegal Dec";
   else if (Status == TDFXY__ZDERR) Text = "ZD greater than 70 degrees";
   else if (Status == TDFXY__SPRINTF) Text =
                      "Format string exceeds available string length (ErsSPrintf)";
   else if (Status == TDFXY__DIRCREERR) Text = "Error creating directory";
   else if (Status == TDFXY__SLA_ERR_1) Text =
                                       "slaDs2tp failed - star too far from Axis.";
   else if (Status == TDFXY__SLA_ERR_2) Text =
                                      "slaDs2tp failed - Antistar on tanget plane";
   else if (Status == TDFXY__SLA_ERR_3) Text =
                                     "slaDs2tp failed - Antistar to far from axis";
   else if (Status == TDFXY__SLA_ERR_UNKN ) Text =
                                         "slaDs2tp failed - Unexpected error code";
   else if (Status == TDFXY__ILLMJD) Text =
            "Illegal Mean Julian Date (MJD) specified - object cannot be observed";
   else if (Status == TDFXY__ILLTEMP) Text =
                           "Illegal temperature supplied - outside expected range";
   else if (Status == TDFXY__ILLPRES) Text =
                              "Illegal pressure supplied - outside expected range";
   else if (Status == TDFXY__ILLHUMI) Text =
                              "Illegal humidity supplied - outside expected range";
   else if (Status == TDFXY__ILLWAVE) Text =
                            "Illegal wavelength supplied - outside expected range";
   else if (Status == TDFXY__ILLSCALE) Text =
                                     "Illegal scale - cannot have a scale of zero";
   else if (Status == TDFXY__ILLFIELD) Text =
                                   "Illegal field plate number - must be 0,1 or 2";
   else if (Status == TDFXY__MAP_FILE_OPEN) Text =
                                   "Failed to open FPI to Sky distortion map file";
   else if (Status == TDFXY__MAP_FILE_READ) Text =
                                   "Failed to read FPI to Sky distortion map file";
   else Text = "";
   
   return Text;
}


// ----------------------------------------------------------------------------------

/*                        P r o g r a m m i n g  N o t e s

   o  The tdfxy routines report errors using ErsRep(). This is fine, apart from
      the slightly 'sticky lump' aspects, but it does complicate error reporting
      a little in a non-DRAMA context.
 
   o  The telecentricity correction code is simple enough when working out the
      correction from an Ra,Dec, as you just work out the angle from the Ra,Dec
      and PG's work gives you the correction in microns immediately. From an
      X.Y position, what I'm doing here is working out Ra,Dec from the X,Y
      without allowing for the offset, then getting the offset on the basis of
      that Ra,Dec, applying the offset to the X,Y values and then recalcuating
      the Ra,Dec. Added later: PG seems to think the way to get the angle is
      from the 2dF code, and that is what I'm doing here.
 
   o  I have some reservations about how the Zone is being calculated. The
      Hector discussions have all talked about 'angle' but this isn't very
      clearly defined. I've taken it as the angle between the Ra,Dec position
      of the centre of the plate and that of the target. This seems to fit
      with what PG said about getting the angle from the 2dF code. There
      are issues near the zone boundaries. My first stab at the 'diff' test
      hit one signle case where a target was close enough to a zone boundary
      that the zone calculated from the uncorrected X,Y position differed
      from that calculted from the corrected X,Y position - ie the target
      is basically in a different zone to the magnet position that reaches
      it! It does seem to me that getting the magnets in the right place may
      depend on the configuration code making the same assumptions as this
      code about the zone calculation and hence which specific magnet to assign
      to a target. I suspect the coordinate conversion code should include the
      assumed zone in the output file to make this clear.
 
   o  I had orginally assumed that XY2RaDec() - which allows for the various
      magnet offsets - was what was needed for the calculation of the Ra,Dec
      equivalent to a sky fibre position, but it is now (24/2/21) clear that
      the sky fibres should not have the telecentricity or mechanical
      offsets applied. They do need the thermal expansion correction. This
      means XY2RaDec() can be used for SkyFibres, but only if the offset
      calculations are disabled beforehand - and conveniently, there are the
      DisableTelecentricity() and DisableMechOffset() calls now available
      to allow just that. (This isn't quite the way I'd have structured the
      code if I'd realised this at first, but it's a pretty simple solution
      to the problem.)
 
   o  Note that normally, the most important use for XY2RaDec() is for
      SkyFibres, but it's convenient to have it work for the other, magnet
      based, fibres as well, as this allows XY2RaDec() to be used to check
      the RaDec2XY() results by reversing the calculation.
 
   o  I have not checked that the positions calculated by the program are actually
      correct, and frankly, I'm not sure how to do that!
 
   o  The linearity code was added rather late in the piece, and this may show.

   o  In RaDec2XY(), Mdiff should really be an instance variable. Using a
      static is a deplorable shortcut - but it is just a diagnostic.
 
   o  I finally got the thermal expansion calculation to reverse exactly -
      you can see this from a trace with diff or diffmax specified as debug
      options. I still see a discrepancy in the hundredths of a micron for
      the telecentricity corrections when they're reversed, and it would be
      nice to look into that, but I don't think it's very productive.

*/
