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
   
   //  We hard-code the coefficient of expansion for the plate, which is
   //  assumed to be invar. This is in microns/metre per degree C.
   
   I_CTE = 1.30;
   
   //  Set the list of debug levels currently supported by the Debug handler.
   //  If new calls to I_Debug.Log() or I_Debug.Logf() are added, the levels
   //  they use need to be included in this list.
   
   I_Debug.LevelsList("Diff,Offsets,Temp");
   
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
//  DisFilePath The path for the 2dF distortion file to use.

bool HectorRaDecXY::Initialise (
   double CenRa, double CenDec, double Mjd, double Dut, double AtmosTemp,
   double Press, double Humid, double CenWave, double ObsWave,
   double RobotTemp, double ObsTemp, const std::string& DistFilePath)
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
   
   //  Try to read the distortion file.
   
   TdfGetDist(const_cast<char*>(DistFilePath.c_str()),&I_Dist,&Status);
   if (Status != STATUS__OK) {
      I_ErrorText = "Unable to open 2dF distortion file " + DistFilePath;
   } else {
   
      //  Initialise the tdfxy conversion routines. This combines all the supplied
      //  data into a single XY parameter structure (I_XYPars) that can be passed
      //  to the conversion routines. Note the set of dummy zero values passed
      //  as the AAT telescope model and ADC parameters. (TdfXYInit() declares all
      //  these as 'unused'.) I_CenRa and I_CenDec we just remember for later.
      
      TdfXyInit (Mjd,Dut,AtmosTemp,Press,Humid,CenWave,ObsWave,
                 0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,
                 I_Dist,&I_XYPars,&Status);
      if (Status != STATUS__OK) {
         std::string StatusText = StatusToText(Status);
         if (StatusText == "") StatusText = "Unexpected error";
         I_ErrorText =
            "Failed to initialise XT conversion routines - " + StatusText;
      } else {
         I_Initialised = true;
         ReturnOK = true;
      }
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

//                            X Y  2  R a  D e c
//
//  Converts X,Y coordinates on the field plate to an apparent RA,Dec position
//  on the sky. This is an X,Y position at which a Hector magnet is placed, and
//  so takes into account not only the standard 2dF coordinate conversion, but
//  also the offsets due to telecentricity and magnet offset.
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
   
      //  The X,Y positions will be those set by the robot during configuration,
      //  but these will need to be modified to allow for thermal expansion.
      
      StatusType Status = STATUS__OK;
      double DeltaX,DeltaY;
      ThermalOffset (X,Y,I_RobotTemp,I_ObsTemp,I_CTE,&DeltaX,&DeltaY);
      double LocalX = X + DeltaX;
      double LocalY = Y + DeltaY;
      TdfXy2rd (&I_XYPars,I_CenRa,I_CenDec,LocalX,LocalY,
                                                  I_Mjd,Ra,Dec,&Status);
      if (Status != STATUS__OK) {
         std::string StatusText = StatusToText(Status);
         if (StatusText == "") StatusText = "Unexpected conversion error";
         I_ErrorText = "Cannot convert X,Y to Ra,Dec - " + StatusText;
      } else {
      
         //  Now apply the telecentricity correction.

         //  The sequence performed by TeleCorrToRaDec() is actually quite
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
         
         TeleCorrToRaDec (LocalX,LocalY,Ra,Dec);
         ReturnOK = true;
         
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
//  also the offsets due to telecentricity and magnet offset.
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
      StatusType Status = STATUS__OK;
      TdfRd2xy (&I_XYPars,I_CenRa,I_CenDec,Ra,Dec,I_Mjd,X,Y,&Status);
      if (Status != STATUS__OK) {
         std::string StatusText = StatusToText(Status);
         if (StatusText == "") StatusText = "Unexpected conversion error";
         I_ErrorText = "Cannot convert Ra,Dec to X,Y - " + StatusText;
      } else {
      
         //  Apply the telecentricity and magnet offset correction. Note that
         //  this uses the Ra,Dec position of the target to determine the zone
         //  and hence the mechanical characteristics of the Hector magnet.
         //  There may be issues near a zone boundary, where a magnet for
         //  either zone might be used, but where each would need a different
         //  X,Y position, as their offsets would be different. However, this
         //  code always picks the zone strictly on the basis of the Ra,Dec
         //  position.
         
         TeleCorrToXY (Ra,Dec,X,Y);
         
         //  The X,Y position will be that at observing time and at observing
         //  temp. We need the position the robot will use at configuration
         //  time.
         
         double DeltaX,DeltaY;
         ThermalOffset (*X,*Y,I_RobotTemp,I_ObsTemp,I_CTE,&DeltaX,&DeltaY);
         *X = *X - DeltaX;
         *Y = *Y - DeltaY;
         
         //  Just for fun, convert that back to Ra Dec and compare. This at
         //  least checks if the coordinate conversion code can be reversed
         //  accurately enough.
         
         if (I_Debug.Active("Diff")) {
            double Ra2,Dec2;
            static double Mdiff = 0.0;
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
         }
         
         ReturnOK = true;
      }
   }
   return ReturnOK;
}

// ----------------------------------------------------------------------------------

//                         T e l e  C o r r  T o  X Y
//
//  Given an apparent RA,Dec position on the sky and the corresponding X,Y
//  coordinates on the field plate calculated on the basis of 2dF distortion,
//  applies the telecentricity correction due to the various prism angles used
//  by Hector and modifies the calculated X,Y position accordingly.
//
//  Ra      Apparent RA in radians.
//  Dec     Apparent Dec in radians.
//  X       Calculated field plate X coordinate in microns (modified).
//  Y       Calculated field plate Y coordinate in microns (modified).

void HectorRaDecXY::TeleCorrToXY (double Ra, double Dec, double *X, double *Y) {

   //  Given the Ra and Dec and the central Ra Dec of the field plate, work out
   //  the angle between the two positions. This angle determines the prism that
   //  will be used for this position.
   
   double AngleRad = slaDsep (I_CenRa, I_CenDec, Ra, Dec);
   
   //  And that in turn determines the offset.
   
   double Offset = TelecentricityOffset (AngleRad);
   
   //  The rest is simple trig. It looks a bit messy because X and Y are actually
   //  the addresses of the X and Y values.
   
   double R = sqrt((*X * *X) + (*Y * *Y));
   *Y = *Y * (R + Offset) / R;
   *X = *X * (R + Offset) / R;
}

// ----------------------------------------------------------------------------------

//                      T e l e  C o r r  T o  R a  D e c
//
//  Given X,Y coordinates on the field plate and the corresponding apparent RA,Dec
//  position on the sky calculated on the basis of 2dF distortion, applies the
//  telecentricity correction due to the various prism angles used by Hector and
//  modifies the calculated Ra, Dec position accordingly.
//
//  X       Field plate X coordinate in microns.
//  Y       Field plate Y coordinate in microns.
//  Ra      Calculated apparent RA in radians (modified).
//  Dec     Calculated apparent Dec in radians (modified).

void HectorRaDecXY::TeleCorrToRaDec (double X, double Y, double *Ra, double *Dec) {

   //  This sequence is a lot messier than that used by TeleCorrToXY(), because
   //  we need the angle between the field centre and the position in question,
   //  and it's easiest to get that from the calculated Ra,Dec. It's complicated
   //  even more by the question of what happens on the boundary between two
   //  Hector zones, as this code assumes that it is the Ra,Dec position that
   //  determines the zone and hence the offset, so there is an element of
   //  iteration involved here.
   
   //  Given the Ra and Dec and the central Ra Dec of the field plate, work out
   //  the angle between the two positions. This angle determines the prism that
   //  will be used for this position.
   
   double AngleRad = slaDsep (I_CenRa, I_CenDec, *Ra, *Dec);
   
   //  And that in turn determines the offset.
   
   int Zone = 0;
   double Offset = TelecentricityOffset (AngleRad,&Zone);
   
   //  Given that, we can now work out the X and Y positions in the field plate
   //  without the offset. Note that the offset is applied in the opposite
   //  direction to that used in TeleCorrToXY().
   
   double R = sqrt (X * X + Y * Y);
   double NewY = Y * (R - Offset) / R;
   double NewX = X * (R - Offset) / R;
   
   //  And then we can work out the Ra, Dec corresponding to the new X,Y position.

   double NewRa, NewDec;
   StatusType Status = STATUS__OK;
   TdfXy2rd (&I_XYPars,I_CenRa,I_CenDec,NewX,NewY,I_Mjd,&NewRa,&NewDec,&Status);
   
   //  This is where it gets a bit tricky. This new Ra,Dec may fall into a
   //  different Hector zone, giving a different offset. If so, then we have
   //  used the wrong offset (assuming that the Ra,Dec position determines
   //  the zone and hence the magnet type used by Hector), and have to
   //  recalculate on the basis of the new zone.

   AngleRad = slaDsep (I_CenRa, I_CenDec, NewRa, NewDec);
   int NewZone = 0;
   double NewOffset = TelecentricityOffset (AngleRad,&NewZone);
   if (NewZone != Zone) {
      R = sqrt (X * X + Y * Y);
      NewY = Y * (R - NewOffset) / R;
      NewX = X * (R - NewOffset) / R;
      Status = STATUS__OK;
      TdfXy2rd (&I_XYPars,I_CenRa,I_CenDec,NewX,NewY,I_Mjd,&NewRa,&NewDec,
                                                                    &Status);
   }
   
   *Ra = NewRa;
   *Dec = NewDec;

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

double HectorRaDecXY::TelecentricityOffset (double AngleRad, int* ZonePtr) {

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
   if (I_EnableTelecentricity) FinalOffset = Offset;
   if (I_EnableMechOffset) FinalOffset -= MechOffset;

   //  This may be a useful diagnostic to enable
   
   if (I_Debug.Active("Offsets")) {
      char Text[1024];
      snprintf (Text,sizeof(Text),
              "Angle %.4f deg, Zone %d, Tele offset: %8.2f mu%s,"
              " Mech offset: %8.2f mu%s",
         AngleDeg,Zone,Offset,I_EnableTelecentricity ? "" : " (disabled)",
                        MechOffset,I_EnableMechOffset ? "" : " (disabled)");
      I_Debug.Log("Offsets",std::string(Text));
   }
   
   //  Return the calculated zone, if that was wanted.
   
   if (ZonePtr) *ZonePtr = Zone;

   return FinalOffset;
}

// ----------------------------------------------------------------------------------

//                        T h e r m a l  O f f s e t
//
//  Calculates the change in X,Y position on the plate due to the difference
//  between the temperature of the plate when configured by the robot and
//  the temperature of the plate at the time of the observation.
//
//  X         Plate position in X (with 0,0 at the plate centre) in microns.
//  Y         Plate position in Y (with 0,0 at the plate centre) in microns.
//  RobotTemp Temperature of plate when configured by the robot, in Deg K.
//  ObsTemp   Temperature of plate during the observation, in Deg K.
//  CTE       Plate coefficient of thermal expansion (microns/metre per deg C).
//  DeltaX    Set to the change in X in microns due to the temperature change.
//  DeltaY    Set to the change in Y in microns due to the temperature change.
//
//  Note: the position change returned in DeltaX,DeltaY is the change from
//  the robot temp position to the observing position. If the observing temp
//  is lower than the robot configuration temp (ie if RobotTemp > ObsTemp),
//  the plate will contract, and the values in DeltaX,DeltaY will be negative.

void HectorRaDecXY::ThermalOffset (double X, double Y, double RobotTemp,
         double ObsTemp, double CTE, double* DeltaX, double* DeltaY)
{
   static const double MicronsPerMetre = 1.0e6;
   double DeltaT = RobotTemp - ObsTemp;
   *DeltaX = (X * CTE * DeltaT) / MicronsPerMetre;
   *DeltaY = (Y * CTE * DeltaT) / MicronsPerMetre;
   if (I_Debug.Active("Temp")) {
      char Text[1024];
      snprintf (Text,sizeof(Text),
         "X,Y [%f,%f], Robot %.2f (K) Obs %.2f (K), DeltaX,Y [%f,%f] mu",
                                X,Y,RobotTemp,ObsTemp,*DeltaX,*DeltaY);
      I_Debug.Log("Temp",std::string(Text));
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
      equivalent to a sky fibre position, but I now suspect that isn't the
      case, and the sky fibres don't need the telecentricity or mechanical
      offsets applied. Do they need the thermal expansion correction? (I
      suspect they do.)
 
   o  I have not checked that the positions calculated by the program are actually
      correct, and frankly, I'm not sure how to do that!

*/
