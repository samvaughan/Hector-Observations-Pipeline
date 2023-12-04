//
//                    H e c t o r  S t r u c t u r e s . h
//
//  Function:
//     Defines some common data structures for the Hector configuration utility.
//
//  Description:
//     The Hector configuration utility is a relatively small program that works
//     with the Hector configuration program, performing the basic Ra,Dec <-> X,Y
//     coordinate conversions needed, and also checking on the sky fibres to see
//     which need capping because they are contaiminated by catalogue objects.
//     It works by reading the input target list in the format output by the
//     tiling program (with positions in Ra,Dec), together with details of the
//     observation conditions (time, metrology, etc) and a list of sky fibre
//     positions, and outputting the same target list, with the converted X,Y
//     positions of the targets added, and also with the sky fibre details,
//     in particular including flags to show which need to be capped. To do
//     this, it makes use of a number of routines all of which need to hold
//     information about the various entities involved, and this file defines
//     the structures they use for this purpose.
//
//  Author(s): Keith Shortridge, K&V  (Keith@KnaveAndVarlet.com.au)
//
//  History:
//     11th Jul 2019.  Original version. KS.
//      6th Oct 2020.  Modified contents of HectorTargetType structure and
//                     HectorUtilProgDetails structure to reflect changes to
//                     support changes following discussion with Julia and Sam
//                     about how the program should be run. KS.
//     12th Nov 2020.  Fairly major revision, since the two files - galaxy and guide
//                     have different fields, and this needs to be handled. Now
//                     treats the galaxy fields as the required superset, and works
//                     out the guide star fields that correspond to these where
//                     possible and uses nulls otherwise. Structures modified to
//                     support this. KS.
//      5th Dec 2020.  HectorSkyCheck.h isn't used, so was dropped. KS.
//     12th Dec 2020.  Added options to suppress telecentricity and offset
//                     corrections to ProgDetails structure. KS.
//     20th Dec 2020.  Expanded HectorSkyFibre structure to include details
//                     included in the specimen sky fibre file sent by Julia
//                     Bryant. KS.
//      5th Jan 2021.  Various modifications to support the full sky fibre
//                     processing, including writing to the output file. KS.
//     16th Feb 2021.  Added CheckSky flag to the program details structure. KS.
//      9th Jun 2021.  Added LinFileName and LinCorrection to the program
//                     details structure. Added LinFilePath to the observation
//                     details structure. KS.
//     15th Jun 2021.  Added ModelPars and NumberPars to the program details. KS.
//     24th Nov 2021.  Added XYRotMatrix to HectorUtilProgDetails structure. KS.
//
// ----------------------------------------------------------------------------------

#ifndef __HectorStructures__
#define __HectorStructures__

#include "HectorRaDecXY.h"
//#include "HectorSkyCheck.h"
#include "slalib.h"

#include <string>
#include <vector>

//  Possible target types - galaxy, guide star, unknown

enum HectorTargetType {GALAXY,GUIDE,UNKNOWN};

//  Number of model parameters used by the coordinate conversion code.

const static int C_MODEL_PARMS = 20;

//  A HectorTarget structure describes one of the targets - galaxy or guide star.
//  We read most of this from the input file, but have to calculate the X,Y
//  position. Note that the mean Ra, Dec values are extracted from the input file
//  data, but these are the only values read from the file that this program
//  understands. The original line read from the input file is included, and
//  for the galaxy data this is written directly to the output file, followed
//  by the calculated values - the X and Y magnet positions. PMRa and PMDec
//  are here because they may be needed in the future, but aren't used at present.
//  XF and YF are not yet calculated, but will have to be at some point once
//  the way to do this is properly understood by me (KS). For guide star data,
//  the fields in OriginalLine need to be shuffled to match the order used by
//  the galaxy files, using the GuideFieldIndices array in the prog details
//  structure.

struct HectorTarget {
   double MeanRa = 0.0;       // Target Ra in radians
   double MeanDec = 0.0;      // Target Dec in radians
   double PMRa = 0.0;         // Proper motion in RA
   double PMDec = 0.0;        // Proper motion in Dec.
   HectorTargetType Type
                   = UNKNOWN; // From the galaxy or guide files?.
   double X = 0.0;            // Calculated X position on field plate in microns
   double Y = 0.0;            // Calculated Y position on field plate in microns
   double XF = 0.0;           // Variant on X for sky fibres - see ADS143.
   double YF = 0.0;           // Variant on X for sky fibres - see ADS143.
   std::string OriginalLine;  // Line as read from input file.
   HectorTargetType TargetType = UNKNOWN;
};

//  A HectorSkyFibre structure describes a sky fibre. Each fibre is on a
//  subplate labelled A1 through A5 and H1 through H7, and each fibre on
//  a subplate has a number going from 1 to 7 for the A subplate fibres
//  and 1 to 8 for the H subplate fibres. Each fibre then has four positions
//  labelled 0,1,2 and 3. I *believe* position 0 is a home position out of
//  the field, and 1,2 and 3 are the potential "in use" positions that
//  need to be checked for contamination, with position 1 being the most and
//  position 3 the least preferred. Position 0 has a much higher radius than
//  the others, and position 3 has the lowest radius. The 'A' fibres go to
//  the AAOmega spectrograph, the 'H' fibres to the Hector spectrograph.

struct HectorSkyFibre {
   char SubplateType = ' ';              // The subplate designation 'A' or 'H'
   int SubplateNo = 0;                   // Subplate number - (1..8)
   int FibreNumber;                      // Fibre number on subplate (0..8)
   double X[4] = {0.0,0.0,0.0,0.0};      // X position on plate in microns
   double Y[4] = {0.0,0.0,0.0,0.0};      // Y position on plate in microns
   double Radius[4] = {0.0,0.0,0.0,0.0}; // Position radius in microns
   double MeanRa[4] = {0.0,0.0,0.0,0.0}; // Corresponding mean Ra in radians
   double MeanDec[4] = {0.0,0.0,0.0,0.0};// Corresponding mean Dec in radians
   int ChosenPosn = 0;                   // Preferred position 1,2,3 or 0
};

//  A HectorObsDetails structure describes the details of the observation - the
//  proposed date, and the assumed metrological conditions. These are the values
//  passed to initialise the standard 2dF conversion routines.

struct HectorObsDetails {
   bool Initialised = false;
   double CenRa = 0.0;      // Central Ra in radians
   double CenDec = 0.0;     // Central Dec in radians
   double Mjd = 0.0;
   double Dut = 0.0;
   double Temp = 0.0;       // Expected observing temperature, in deg K.
   double Press = 0.0;
   double Humid = 0.0;
   double CenWave = 0.0;
   double ObsWave = 0.0;
   std::string DistFilePath = "";
   std::string LinFilePath = "";
};

//  A HectorFileHeader structure contains any details read from the input file that
//  have to be included in the output file.

struct HectorFileHeader {
   std::string FileName;
   std::vector<std::string> HeaderLines;
};

//  A HectorUtilProgDetails structure contains all the information needed by
//  the program itself, including the Ok flag, which provides an 'inherited
//  status' mechanism - this isn't as modern as using exceptions, but it keeps
//  things simple, and this program doesn't merit more complexity. Note that
//  FieldNames is set when the galaxy data is read, and GuideFieldIndices is
//  calculated when the guide data is read, by comparing the field names in
//  the guide star file label with the values stored in FieldNames.

struct HectorUtilProgDetails {
   bool Ok = true;                       // 'Inherited' status variable
   int Argc = 0.0;                       // Number of command line aarguments
   char** Argv = NULL;                   // Command line arguments
   std::string MainTargetFileName = "";  // Name of main (gaalaxy) target file
   std::string GuideTargetFileName = ""; // Name of guide star target file
   std::string OutputFileName = "";      // Output file to be written
   std::string Label = "";               // Value of output file LABEL field
   std::string PlateID = "";             // Value of output file PLATEID field
   std::string DateAndTime = "";         // Obs date/time, eg 2020 01 28 15 30 00.00"
   bool MechCorrection = true;           // Apply mechanical offset corrections
   bool TeleCorrection = true;           // Apply telecentricity corrections
   bool LinCorrection = true;            // Apply linearity corrections
   bool CheckSky = true;                 // Check sky fibre contamination
   int ExpectedAFibres = 0;              // # of expected AAOmega sky fibres
   int ExpectedHFibres = 0;              // # of expected Hector sky fibres
   std::string SkyFibreFileName = "";    // Name of file with sky fibre details
   std::string ListOfFields = "";        // List of data fields as in input file.
   std::vector<std::string> FieldNames;  // The data fields list as separate names.
   std::vector<int> GuideFieldIndices;   // Guide field indices for each data field.
   int RaItem = 0;                       // Field item for Ra (used for sky fibres)
   int DecItem = 0;                      // Field item for Dec (used for sky fibres)
   double Mjd = 0.0;                     // Obs date and time as Mjd.
   float RobotTemp = 0.0;                // Robot configuration temp, deg K.
   float ObsTemp = 0.0;                  // Estimated observation temp, deg K.
   std::string RotMatString = "";        // Rotation matrix as specified as string
   double XYRotMatrix[4] = {1.,0.,0.,1.};// Rotation matrix applied to X,Y values
   double ModelPars[C_MODEL_PARMS];      // Coordinate model parameters
   int NumberPars = 0;                   // Number of items used in Pars.
   HectorRaDecXY CoordConverter;         // Instance of coordinate converter
   bool ConverterInitialised = false;    // True once converter initialised
   double CentreRa = 0.0;                // Central Ra in radians
   double CentreDec = 0.0;               // Central Dec in radians
   double FieldRadius = 0.0;             // Field radius in radians
   double SkyRadiusAsec = 0.0;           // Sky fibre clear radius in arcsec
   std::string DistFileName = "";        // Name of 2dF distortion file
   std::string LinFileName = "";         // Name of 2dF linearity file
   std::string ProfitDirectory = "";     // Directory holding Profit maskfiles
   std::string DebugLevels = "";         // Used to control debugging
   std::string Error = "";               // Describes last error
   std::vector<std::string> Warnings;    // Any warning messages.
};

#endif

// ----------------------------------------------------------------------------------

/*                        P r o g r a m m i n g  N o t e s

   o  The only methods I ever like to have in structs are constructors that ensure
      the fields are initialised. With C++11 I can use in-class initialisers for
      this, as hers, but it means this code does need -std=c++11 when compiled.
 
   o  It seems an unnecessary duplication to have the 2dF distortion and linearity
      file names in both the program details and observation details structures.
 
*/
