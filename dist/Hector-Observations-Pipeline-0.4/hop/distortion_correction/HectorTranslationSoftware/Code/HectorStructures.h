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

//  A HectorSkyFibre structure describes a sky fibre. We can get the X,Y position
//  from the file describing the sky fibre layout, but have to calculate the
//  Ra,Dec using the observation details. The Capped flag then depends a) on
//  whether there is a known object contaminating the position and b) whatever
//  sky shuffling calculation is required for the Hector spectrograph.

struct HectorSkyFibre {
   double X = 0.0;
   double Y = 0.0;
   double MeanRa = 0.0;  // Sky fibre corresponding Ra in radians
   double MeanDec = 0.0; // Sky fibre corresponding Dec in radians
   bool Contaiminated = false;
   bool Capped = false;
};

//  A HectorIbsDetails structure describes the details of the observation - the
//  proposed date, and the assumed metrological conditions.

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
   std::string ListOfFields = "";        // List of data fields as in input file.
   std::vector<std::string> FieldNames;  // The data fields list as separate names.
   std::vector<int> GuideFieldIndices;   // Guide field indices for each data field.
   double Mjd = 0.0;                     // Obs date and time as Mjd.
   float RobotTemp = 0.0;                // Robot configuration temp, deg K.
   float ObsTemp = 0.0;                  // Estimated observation temp, deg K.
   HectorRaDecXY CoordConverter;         // Instance of coordinate converter
   bool ConverterInitialised = false;    // True once converter initialised
   double CentreRa = 0.0;                // Central Ra in radians
   double CentreDec = 0.0;               // Central Dec in radians
   std::string DistFileName = "";        // Name of 2dF distortion file
   std::string Error = "";               // Describes last error
   std::vector<std::string> Warnings;    // Any warning messages.
};

#endif

// ----------------------------------------------------------------------------------

/*                        P r o g r a m m i n g  N o t e s

   o  The only methods I ever like to have in structs are constructors that ensure
      the fields are initialised. With C++11 I can use in-class initialisers for
      this, as hers, but it means this code does need -std=c++11 when compiled.
 
 
*/
