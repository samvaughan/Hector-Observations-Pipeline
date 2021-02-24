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

HectorRaDecXY::HectorRaDecXY (void)
{
   I_Initialised = false;
   I_Mjd = 0.0;
   I_Dut = 0.0;
   I_Temp = 0.0;
   I_Press = 0.0;
   I_Humid = 0.0;
   I_CenWave = 0.0;
   I_ObsWave = 0.0;
   I_DistFilePath = "";
   I_ErrorText = "";
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

//                            I n i t i a l i s e
//
//  Initialises the conversion routines, passing all the information needed
//  about the observation and the 2dF distortion.
//
//  CenRa       RA of the centre of the field plate in radians.
//  CenDec      Dec of the centre of the field plate in radians.
//  Mjd         UTC observing date and time as a modified julian date.
//  Dut         Delta UT (UT1 - UTC) in seconds.
//  Temp        Observing temperature in degrees Kelvin.
//  Press       Observing atmospheric pressure in mB.
//  Humid       Atmospheric relative humidity (0 to 1.0).
//  CenWave     Telescope pointing wavelength in microns.
//  ObsWave     Observing wavelength in microns.
//  DisFilePath The path for the 2dF distortion file to use.

bool HectorRaDecXY::Initialise (
   double CenRa, double CenDec, double Mjd, double Dut, double Temp, double Press,
   double Humid, double CenWave, double ObsWave, const std::string& DistFilePath)
{
   
   bool ReturnOK = false;
   
   StatusType Status = STATUS__OK;
   
   I_Initialised = false;
   
   I_CenRa = CenRa;
   I_CenDec = CenDec;
   I_Mjd = Mjd;
   I_Dut = Dut;
   I_Temp = Temp;
   I_Press = Press;
   I_Humid = Humid;
   I_CenWave = CenWave;
   I_ObsWave = ObsWave;
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
      
      TdfXyInit (Mjd,Dut,Temp,Press,Humid,CenWave,ObsWave,
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
         TdfXyInit (I_Mjd,I_Dut,I_Temp,I_Press,I_Humid,I_CenWave,ObsWave,
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
//  on the sky.
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
      StatusType Status = STATUS__OK;
      TdfXy2rd (&I_XYPars,I_CenRa,I_CenDec,X,Y,I_Mjd,Ra,Dec,&Status);
      if (Status != STATUS__OK) {
         std::string StatusText = StatusToText(Status);
         if (StatusText == "") StatusText = "Unexpected conversion error";
         I_ErrorText = "Cannot convert X,Y to Ra,Dec - " + StatusText;
      } else {
         TeleCorrToRaDec (X,Y,Ra,Dec);
         ReturnOK = true;
      }
   }
   return ReturnOK;
}

// ----------------------------------------------------------------------------------

//                            R a  D e c  2  X Y
//
//  Converts an apparent RA,Dec position on the sky to X,Y coordinates on the
//  field plate.
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
      
         //  See comments in TeleCorrToXY(). It may be that I should really
         //  work out the telecentricity offset at the start of this routine
         //  based on the X,Y values, using them and the plate scale to work out
         //  the angle and hence the offset band in question. Doing it this way
         //  involves two calls to TdfRd2xy(), one in TeleCorrToXY(), which
         //  really doesn't look quite right now I think of it.
         
         TeleCorrToXY (Ra,Dec,X,Y);
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

   //  This sequence is a bit messier than that used by TeleCorrToXY(), because
   //  we need the angle between the field centre and the position in question,
   //  and it's easiest to get that from the calculated Ra,Dec. In principle,
   //  the telecentricity correction could be applied in this case in XY2RaDec()
   //  as soon as it's called, just from X,Y that it's passed. See Programming
   //  Notes.
   
   //  Given the Ra and Dec and the central Ra Dec of the field plate, work out
   //  the angle between the two positions. This angle determines the prism that
   //  will be used for this position.
   
   double AngleRad = slaDsep (I_CenRa, I_CenDec, *Ra, *Dec);
   
   //  And that in turn determines the offset.
   
   double Offset = TelecentricityOffset (AngleRad);
   
   //  Given that, we can now work out the X and Y positions in the field plate
   //  without the offset.
   
   double R = sqrt (X * X + Y * Y);
   double NewY = Y * (R - Offset) / R;
   double NewX = X * (R - Offset) / R;
   
   //  And then we can work out the Ra, Dec corresponding to the new X,Y position.

   StatusType Status = STATUS__OK;
   TdfXy2rd (&I_XYPars,I_CenRa,I_CenDec,NewX,NewY,I_Mjd,Ra,Dec,&Status);
   
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
//  which is why it was written to return an outward offset.)
//
//  AngleRad    The angle to the position in question, in radians.

double HectorRaDecXY::TelecentricityOffset (double AngleRad) {

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
   
   double Offset = 31.0;
   double MechOffset = 1173.0;
   double AngleDeg = AngleRad * DR2D;
   if (AngleDeg < 0.396) {
      //  In the angle range 0 to 0.396 degrees, the prism angle offset is a
      //  constant 31 microns, and the mechanical offset is 0.1173mm.
      Offset = 31.0;
      MechOffset = 117.3;
   } else if (AngleDeg < 0.627) {
      //  In the angle range 0.396 to 0.627 degrees, the prism angle offset is a
      //  constant 64 microns, and the mechanical offset is 0.2382mm.
      Offset = 64.0;
      MechOffset = 238.2;
   } else if (AngleDeg < 0.823) {
      //  In the angle range 0.627 to 0.823 degrees, the prism angle offset
      //  increases linearly between 94 and 95 microns, and the mechanical offset
      //  is 0.3468mm.
      Offset = 94.0 + (95.0 - 94.0) * (AngleDeg - 0.627) / (0.823 - 0.627);
      MechOffset = 346.8;
   } else if (AngleDeg < 1.0) {
      //  In the angle range 0.823 to 1.0 degrees, the prism angle offset decreases
      //  linearly between 118 and 116 microns and the mechanical offset is 0.4529mm.
      Offset = 118.0 + (116.0 - 118.0) * (AngleDeg - 0.823) / (1.0 - 0.823);
      MechOffset = 452.9;
   } else {
      //  This shouldn't happen, but we trap it anyway.
      Offset = 116.0;
      MechOffset = 452.9;
   }
   
   //  This may be a useful disgnostic to enable
   
   bool ListOffsets = false;
   if (ListOffsets) {
      printf ("Angle %f deg, Offset %f micron, MechOffset %f micron\n",
                                              AngleDeg,Offset,MechOffset);
   }

   return Offset - MechOffset;
}

// ----------------------------------------------------------------------------------

//                            G e t  E r r o r
//
//  Returns a description of the latest error.

std::string HectorRaDecXY::GetError (void) {
   return I_ErrorText;
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
      X.Y position, what I'm doing here is working out Ra,Dec from the X,Y without
      allowing for the offset, then getting the offset on the basis of that Ra,Dec,
      applying the offset to the X,Y values and then recalcuating the Ra,Dec. I
      don't think this is really the best way, but it probably doesn't matter
      given this is only used to see if the Ra,Dec position is contaminated and
      there's some leeway there. But it might be better to work out the angle
      directly from the X,Y position knowing the plate scale, and apply that
      before doing just a single conversion to Ra,Dec. I hope that's clear. I
      didn't do it that way because a) I wasn't quite sure of the plate scale, and
      b) wasn't sure if the temperature correction would affect this, once that
      gets added to the code. Maybe the time to revisit this is when the temp
      correction code gets added. Added later: PG seems to think the way to get the
      angle is from the 2dF code, and that is what I'm doing here, although I'm not
      sure how clear I was about why I had reservations when starting from X,Y
      rather than Ra,Dec. Still, for the moment, let it stand.
 
   o  I have not checked that the positions calculated by the program are actually
      correct, and frankly, I'm not sure how to do that!

*/
