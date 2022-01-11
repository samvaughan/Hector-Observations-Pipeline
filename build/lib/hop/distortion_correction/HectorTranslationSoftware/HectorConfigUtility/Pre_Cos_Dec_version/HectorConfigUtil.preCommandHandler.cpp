//
//                    H e c t o r  C o n f i g  U t i l . c p p
//
//  Function:
//     Provides initial utility functions for the Hector configuration program.
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
//     in particular including flags to show which need to be capped.
//
//  Invocation:
//     HectorConfigUtil <galaxy_file_path> <guide_file_path> <output_file_path>
//                      <label> <plateId> <date_and_time> <robot_temp> <obs_temp>
//
//     Where:
//
//     HectorConfigUtil   is the name of the Translation program. (This could be
//                        changed, of course.)
//     <galaxy_file_path> is a string giving the path name of the .fld file
//                        containing the galaxy target details.
//     <guide_file_path>  is a string giving the path name of the .fld file
//                        containing the guide star target details.
//     <output_file_path> is a string giving the path name of the .fld file
//                        that will be output by this program.
//     <label>            is a string (probably enclosed in quotes, if it
//                        contains spaces) used for the LABEL in the output file.
//     <plateId>          is a string to be used for the PLATEID in the output file.
//     <date_and_time>    is a string giving the UT date and time for the
//                        observation, eg “2020 01 28 15 30 00.00”
//     <robot_temp>       is the temperature in degrees C at which the robot will
//                        configure the plate
//     <obs_temp>         is the temperature in degrees C at which the observation
//                        will be performed.
//
//     The path name of the 2dF distortion file will be set in the environment
//     variable “TDF_DISTORTION”
//
//     The file names could simply be the full path names for the files. If not,
//     they must be such that a call such as fopen() with the file name as supplied
//     in the context in which the program will run will open the correct file.
//
//     At the moment, this program does nothing with sky fibre positions,
//     the field centre is not specified in the input file but is calculated
//     as a stopgap measure as the median of the target positions, and the
//     observing time (MJD) is not supplied, but is calculated as another
//     stopgap to put the field on the meridian at the current date.
//
//  Return codes:
//     If the program completes successfully, it will return a completion code
//     of zero. If it hits a problem and fails to complete properly, it returns
//     a completion code of 1, and will have written details to standard error.
//     It may encounter some conditions that cause it to issue warnings but allow
//     it to complete. In this case, the warnings will be written to standard
//     error, but it will return a completion code of zero. If invoked from a
//     pipeline, for example, the pipeline code should both read the standard error
//     output and should check the completion code.
//
//  Author(s): Keith Shortridge, K&V  (Keith@KnaveAndVarlet.com.au)
//
//  History:
//     11th Jul 2019.  Original version. KS.
//     19th Jul 2019.  Fleshed out to the point that it can read an example
//                     target file, and output a file in SAMI format that has
//                     the converted X,Y field plate positions. Notable omissions
//                     are anything to do with sky fibres, and the telecentricity
//                     correction. KS.
//      6th Oct 2020.  Significant reworking of this and HectorStructures.h to
//                     support reading two files - a galaxy and a guide star file -
//                     instead of just one. Now matches the specification I sent
//                     to Sam and Julia. KS.
//      9th Oct 2020.  Minor changes, version sent to Sam for testing. KS.
//     12th Nov 2020.  Fairly major revision, since the two files - galaxy and guide
//                     have different fields, and this needs to be handled. Now
//                     treats the galaxy fields as the required superset, and works
//                     out the guide star fields that correspond to these where
//                     possible and uses nulls otherwise. KS.
//      5th Dec 2020.  Added include of slalib.h, now that HectorStructures.h
//                     is no longer pulling it in as it no longer included
//                     HectorSkyCheck.h. KS.
//      6th Dec 2020.  Fixed a couple of signed/unsigned integer comparisons the
//                     gcc compiler used by Sam Vaughan was complaining about. KS.
//     12th Dec 2020.  Now supports the -nomech and -notele options on the command
//                     line. KS.
//      5th Jan 2021.  Finally includes the code for reading in the sky fibre
//                     positions, doing the coordinate conversions, using the
//                     Profit mask files to check for contamination, and
//                     writing the reults out to the output file. This needed
//                     a number of new command line parameters to be added,
//                     and those still need to be sorted out properly and
//                     documented. KS.
//
//  Note:
//     The structure of this code has a main program that simply calls a set of
//     processing subroutines in a fixed order. The code layout has the subroutines
//     first, to avoid the need for forward definitions of them to allow the main
//     routine to compile, followed by the main routine. When looking at this code
//     for the first time, it would be as well to skip straight to the main routine
//     to see the big picture and then look back to the subroutines for more detail.
//
// ----------------------------------------------------------------------------------

#include "HectorStructures.h"

#include <ctime>
#include <iostream>
#include <stdio.h>
#include <string.h>

#include "slalib.h"
#include "slamac.h"

#include "ProfitSkyCheck.h"

//  This code uses a few utility routines (mostly string-handling) originally
//  developed for the AAT TCS system, and included in a small TcsUtil library.

#include "TcsUtil.h"

using std::vector;
using std::string;

extern "C" {
   float gen_qfmed_(float* Data,int* Nelm);
}

//  Constants:
//  Zero degrees C in degrees K.

const double ZeroDegCinDegK = 273.15;

// ----------------------------------------------------------------------------------

//                      L i s t  P r o g  D e t a i l s
//
//  This is a diagnostic routine that lists the contents of the overall program
//  details structure.

void ListProgDetails (const HectorUtilProgDetails& ProgDetails)
{
   printf ("\n");
   printf ("Contents of program details structure\n");
   printf ("\n");

   printf ("Ok flag: %s\n",ProgDetails.Ok ? "true" : "false");
   
   //  Listing the command line arguments may be useful, but note that most
   //  are just copied into the other items in the ProgDetails structure, so
   //  they'll end up dimply duplicated. At least this will show if any
   //  of these were mishandled. Note the order needs to be changed if the
   //  command arguments change - the awkward use of I and N allows these
   //  lines to be moved around easily, even if it looks odd.
   
   printf ("Number of command line arguments: %d\n",ProgDetails.Argc);
   int I = 0;
   int N = ProgDetails.Argc;
   if (I < N) printf ("   Program name: '%s'\n",ProgDetails.Argv[I++]);
   if (I < N) printf ("   Galaxy target file: '%s'\n",ProgDetails.Argv[I++]);
   if (I < N) printf ("   Guide target file: '%s'\n",ProgDetails.Argv[I++]);
   if (I < N) printf ("   Output file: '%s'\n",ProgDetails.Argv[I++]);
   if (I < N) printf ("   Label: '%s'\n",ProgDetails.Argv[I++]);
   if (I < N) printf ("   PlateID: '%s'\n",ProgDetails.Argv[I++]);
   if (I < N) printf ("   Date and time: '%s'\n",ProgDetails.Argv[I++]);
   if (I < N) printf ("   Robot temp: '%s' C\n",ProgDetails.Argv[I++]);
   if (I < N) printf ("   Observing temp: '%s' C\n",ProgDetails.Argv[I++]);
   
   //  Now the same values as they ended up in the individual fields of the
   //  structure.
   
   printf ("Main target file name: '%s'\n",ProgDetails.MainTargetFileName.c_str());
   printf ("Guide target file name: '%s'\n",ProgDetails.GuideTargetFileName.c_str());
   printf ("Output file name: '%s'\n",ProgDetails.OutputFileName.c_str());
   printf ("Label: '%s'\n",ProgDetails.Label.c_str());
   printf ("PlateID: '%s'\n",ProgDetails.PlateID.c_str());
   printf ("Date and time: '%s'\n",ProgDetails.DateAndTime.c_str());
   printf ("Mechanical corrections: %s\n",ProgDetails.NoMechCorrection ?
                                                  "disabled" : "enabled");
   printf ("Telecentricity corrections: %s\n",ProgDetails.NoTeleCorrection ?
                                                     "disabled" : "enabled");
   int Year,Month,Day,Ihmsf[4],Jstat;
   double Frac,Mjd;
   char Sign[1];
   Mjd = ProgDetails.Mjd;
   slaDd2tf(0, Mjd - floor(Mjd), Sign, Ihmsf);
   slaDjcl(Mjd, &Year, &Month, &Day, &Frac, &Jstat);
   printf("Mjd: %f (UT %.4d/%.2d/%.2d  %.2d:%.2d:%.2d)\n",
               Mjd, Year, Month, Day, Ihmsf[0], Ihmsf[1], Ihmsf[2]);
   printf ("Robot temp: '%.2f' K (%.2f C)\n",ProgDetails.RobotTemp,
                                       ProgDetails.RobotTemp - ZeroDegCinDegK);
   printf ("Observing temp: '%.2f' K (%.2f C)\n",ProgDetails.ObsTemp,
                                         ProgDetails.ObsTemp - ZeroDegCinDegK);

   //  And the rest.
   
   printf ("Converter initialised: %s\n",
                      ProgDetails.ConverterInitialised ? "true" : "false");
   printf ("CenterRa : %f deg\n",ProgDetails.CentreRa * DR2D);
   printf ("CenterDec : %f deg\n",ProgDetails.CentreDec * DR2D);
   printf ("Distortion file name: '%s'\n",ProgDetails.DistFileName.c_str());
   printf ("Error string: '%s'\n",ProgDetails.Error.c_str());
   int NWarnings = ProgDetails.Warnings.size();
   printf ("Warning strings: %d\n",NWarnings);
   for (int I = 0; I < NWarnings; I++) {
      printf ("   '%s'\n",ProgDetails.Warnings[I].c_str());
   }

   printf ("\n");
}

// ----------------------------------------------------------------------------------

//                      V a l i d  I n t e g e r
//
//  Simple utility to check if a string represents a valid integer, and
//  to return the value of that integer if so.

bool ValidInteger (const string& String, long* Value)
{
   bool Valid = true;
   size_t Idx;
   try {
      *Value = stol(String,&Idx);
   } catch (...) {
      *Value = 0;
      Valid = false;
   }
   if (Valid) {
      if ((Idx != string::npos) && (Idx != String.size())) Valid = false;
   }
   return Valid;
}

// ----------------------------------------------------------------------------------

//                        V a l i d  R e a l
//
//  Simple utility to check if a string represents a valid floating point
//  number, and to return the value of that number if so.

bool ValidReal (const string& String, double* Value)
{
   bool Valid = true;
   size_t Idx;
   try {
      *Value = stod(String,&Idx);
   } catch (...) {
      *Value = 0.0;
      Valid = false;
   }
   if (Valid) {
      if ((Idx != string::npos) && (Idx != String.size())) Valid = false;
   }
   return Valid;
}


// ----------------------------------------------------------------------------------

//                      S e t u p  P r o g  D e t a i l s
//
//  This routine sets up the program details structure used by most of the routines.
//  The details may depend on any command line arguments, so these are passed to
//  the routine as Argc and Argv.

void SetUpProgDetails (int Argc,char** Argv,HectorUtilProgDetails* ProgDetails)
{
   //  Most of the fields in the ProgDetails structure should have been initialised
   //  in the compiler-generated constructor to the default values specified in
   //  the HectorStructures.h file structure definition. But it doesn't hurt to
   //  make doubly certain.
   
   ProgDetails->Ok = true;
   ProgDetails->Argc = Argc;
   ProgDetails->Argv = Argv;
   ProgDetails->ConverterInitialised = false;
   
   vector<string> Arguments;
   vector<string> Options;
   
   //  Split up the command line arguments into options - flags beginning
   //  with '-' (for the moment, we assume these can't take values, and are
   //  just things like -notele or -nomech) - and positional arguments.
   //  Note that Arguments will include the initial argument, the name of
   //  the program. I'd really like to expand on this parsing - see programming
   //  notes.
   
   for (int I = 0; I < Argc; I++) {
      if (Argv[I][0] == '-') {
         Options.push_back(string(Argv[I]));
      } else {
         Arguments.push_back(string(Argv[I]));
      }
   }
   
   //  At this point we parse the positional arguments, and set up other
   //  things accordingly.
   
   //  For the moment, I have suggested the command line syntax described at the
   //  start of this file, that is:
   //
   //  HectorConfigUtil <galaxy_file_path> <guide_file_path>
   //        <output_file_path> <label> <plateId> <date_and_time>
   //          <robot_temp> <obs_temp>

   //  Note that we don't try to validate the values we get at this point. That
   //  happens later, when we come to use the values, as at that point we may be
   //  in a better position to calculate default values for unspecified items,
   //  particularly things like observing time. (Note that using ArgItem as a
   //  counter here makes it easier to re-order the argument code if needed.
   
   int ArgItem = 0;
   if (Arguments.size() > ++ArgItem) {
      if (Arguments[ArgItem] == "") {
         ProgDetails->Error = "Main (galaxy) input file name cannot be null";
         ProgDetails->Ok = false;
      } else {
         ProgDetails->MainTargetFileName = Arguments[ArgItem];
      }
   } else if (ProgDetails->Ok) {
      ProgDetails->Error = "No main (galaxy) input file name specified";
      ProgDetails->Ok = false;
   }
   if (Arguments.size() > ++ArgItem) {
      ProgDetails->GuideTargetFileName = Arguments[ArgItem];
   } else if (ProgDetails->Ok) {
      ProgDetails->Error = "No guide target file name specified";
      ProgDetails->Ok = false;
   }
   if (Arguments.size() > ++ArgItem) {
      ProgDetails->OutputFileName = Arguments[ArgItem];
   } else if (ProgDetails->Ok) {
      ProgDetails->Error = "No output file name specified";
      ProgDetails->Ok = false;
   }
   if (Arguments.size() > ++ArgItem) {
      ProgDetails->Label = Arguments[ArgItem];
   } else {
      ProgDetails->Label = "";
   }
   if (Arguments.size() > ++ArgItem) {
      ProgDetails->PlateID = Arguments[ArgItem];
   } else {
      ProgDetails->PlateID = "";
   }
   
   //  Parsing the observing time string, which is allowed to default in a
   //  couple of complicated ways, is best handled by a separate routine,
   //  but we can't run that until we've read in the input files, not so
   //  much for how it parses the time string, but for how it sets a sensible
   //  default in the case where one isn't specified.
   
   if (Arguments.size() > ++ArgItem) {
      ProgDetails->DateAndTime = Arguments[ArgItem];
   } else {
      ProgDetails->DateAndTime = "";
   }
   
   //  The temperatures are given in deg C, so need to be converted to deg K
   //  (one nice thing about using K is that 0.0 can be used to indicate no
   //  value was specified). We need to check that the value specified is
   //  a valid number.
   
   if (Arguments.size() > ++ArgItem) {
      float TempInC;
      int N = sscanf(Arguments[ArgItem].c_str(),"%f",&TempInC);
      if (N == 1) {
         ProgDetails->RobotTemp = TempInC + ZeroDegCinDegK;
      } else if (ProgDetails->Ok) {
         ProgDetails->Error = "Invalid robot temp value ('"
                                    + Arguments[ArgItem] + "') specified";
         ProgDetails->Ok = false;
      }
   } else {
      ProgDetails->RobotTemp = 0.0;
   }
   if (Arguments.size() > ++ArgItem) {
      float TempInC;
      int N = sscanf(Arguments[ArgItem].c_str(),"%f",&TempInC);
      if (N == 1) {
         ProgDetails->ObsTemp = TempInC + ZeroDegCinDegK;
      } else if (ProgDetails->Ok) {
         ProgDetails->Error = "Invalid observation temp value ('"
                                    + Arguments[ArgItem] + "') specified";
         ProgDetails->Ok = false;
      }
   } else {
      ProgDetails->ObsTemp = 0.0;
   }
   
   //  Check the options, if any, to make sure we expect them.
   
   for (string Option : Options) {
      if (TcsUtil::MatchCaseBlind(Option,"-notele")) {
         ProgDetails->NoTeleCorrection = true;
      } else if (TcsUtil::MatchCaseBlind(Option,"-nomech")) {
         ProgDetails->NoMechCorrection = true;
      } else {
         ProgDetails->Error = "Invalid option string ('"
                                    + Option + "') specified";
         ProgDetails->Ok = false;
      }
   }

   //  The name of the 2dF distortion file is taken from the environment variable
   //  TDF_DISTORTION, and defaults to tdFdistortion0.sds in the current directory.
   
   char* DistFile = getenv("TDF_DISTORTION");
   if (DistFile) {
      ProgDetails->DistFileName = DistFile;
   } else {
      ProgDetails->DistFileName = "tdFdistortion0.sds";
   }
   
   //  The name of the file holding the skyfibre position details is taken from
   //  the environment variable SKY_FIBRES, and defaults to SkyFibres.csv in the
   //  current directory.
   
   char* SkyFibreFile = getenv("SKY_FIBRES");
   if (SkyFibreFile) {
      ProgDetails->SkyFibreFileName = SkyFibreFile;
   } else {
      ProgDetails->SkyFibreFileName = "SkyFibres.csv";
   }
   
   //  The name of the directory containing the Profit mask files is taken from
   //  the environment variable PROFIT_DIR. There isn't a sensible default.
   
   char* ProfitDir = getenv("PROFIT_DIR");
   if (ProfitDir) {
      ProgDetails->ProfitDirectory = ProfitDir;
   } else {
      ProgDetails->ProfitDirectory = "";
   }
   
   //  For the moment, expect 35 AAOmega sky fibres and 56 Hector sky fibres
   
   ProgDetails->ExpectedHFibres = 56;
   ProgDetails->ExpectedAFibres = 35;
   
   //  Some sensible value for the field and sky fibre clear radius.
   
   ProgDetails->FieldRadius = 1.1 * DD2R;
   ProgDetails->SkyRadiusAsec = 3.0;
   
}

// ----------------------------------------------------------------------------------

//                      R e a d  I n p u t  F i l e
//
//  This routine reads the specified input file and fills the File header structure
//  with the details of the header lines adds the details of the targets specified
//  in the input fileto to the target list vector. The FileType should be either
//  GALAXY or GUIDE, and the name of the target file will be taken from the
//  relevant entry in the ProgDetails structure.

void ReadInputFile (
   HectorTargetType FileType,
   HectorFileHeader* FileHeader,
   vector<HectorTarget> *TargetList,
   HectorUtilProgDetails* ProgDetails)
{
   if (!ProgDetails->Ok) return;
   
   //  I do have a pretty good idea of the format of the input file, although a
   //  few details are still TBD. Actually, I think a better description is that
   //  not all the details are completely clear to me. Still, all the files I've
   //  seen so far seem to follow the pattern shown by the first few lines of
   //  one sample file:
   //
   //  # Target and Standard Star file from Sam's tiling code
   //  # 336.5224915 -31.106872600000003
   //  # Proximity Value: 217.616
   //  ID RA DEC mag type
   //  999388004253 337.1053162 -31.6497154 14.0115051 2
   //  999388006577 336.2006226 -31.7324677 14.036174800000001 2
   //  etc..
   //
   //  As far as I can see:
   //  The first line is pretty much the same in all cases.
   //  The second, although it looks like a comment, actually gives the Ra,Dec
   //  of the field centre, in decimal degrees.
   //  The third line gives a 'proximity value', and I don't know what that is.
   //  The fourth line, which doesn't have a '#' at the start, gives the fields
   //  to expect in the rest of the lines in the file. Some files just have the
   //  five fields shown here. Other files have a great many other items.
   //
   //  There is then one line for each object, which gives the values for the
   //  various fields for each object. These are blank-separated in some cases,
   //  comma-separated in others. However, all seem to have in common that the
   //  first field is a numeric ID or 'name' and the second and third fields are
   //  the Ra and Dec position for the object in decimal degrees.
   //
   //  That doesn't exactly constitute a full understanding of the format, but
   //  really this program only needs to be able to work with the Ra and Dec
   //  values and to pass on the rest of the field values unchanged to the
   //  output file.
   //
   //  The code that follows assumes the format described above.
   
   //  Open the input target file. I'm assuming we get this from the command line
   //  arguments.
   
   char Error[1024];
   string TargetFileName;
   if (FileType == GALAXY) {
      TargetFileName = ProgDetails->MainTargetFileName;
   } else {
      TargetFileName = ProgDetails->GuideTargetFileName;
   }
   
   //  We quietly ignore any file with a null name - the idea is that it might
   //  sometimes be convenient to have the guide file null so we can test with
   //  just one file.
   
   if (TargetFileName != "") {
      FileHeader->FileName = TargetFileName;
      FILE* TargetFile = fopen(TargetFileName.c_str(),"r");
      if (TargetFile == NULL) {
         ProgDetails->Error = "Error opening target file: " + TargetFileName;
         ProgDetails->Ok = false;
      } else {
         int TargetCount = 0;
         int GuideCount = 0;
         int LineNumber = 0;
         int ObjectItems = 0;
         int RaItem = -1;
         int DecItem = -1;
         for (;;) {
            char Line[1024];
            if (fgets (Line,sizeof(Line),TargetFile)) {
            
               //  Read a line. Parse it, after removing any newline or return
               //  characters and truncating after the last non-blank character.
               
               LineNumber++;
               Line[sizeof(Line) - 1] = '\0';
               int LastNonBlank = -1;
               for (unsigned int I = 0; I < sizeof(Line); I++) {
                  if (Line[I] == '\0') break;
                  if (Line[I] == '\n' || Line[I] == '\r') {
                     Line[I] = '\0';
                     break;
                  }
                  if (Line[I] != ' ') LastNonBlank = I;
               }
               Line[LastNonBlank + 1] = '\0';
               string LineString = Line;
               
               //  Tokenize the line. We'll need to do this for most lines, so
               //  do this here for all of them.
               
               vector<string> Tokens;
               TcsUtil::Tokenize(LineString,Tokens," ,");
               bool IsComment = false;
               int ItemCount = Tokens.size();
               if (ItemCount > 0 && Tokens[0] == "#") IsComment = true;
               if (LineNumber <= 3 && !IsComment) {
                  snprintf (Error,sizeof(Error),
                           "Line %d: Expected line to start with #: '%s'",
                                                                LineNumber,Line);
                  ProgDetails->Ok = false;
                  ProgDetails->Error = Error;
                  break;
               }

               if (LineNumber == 1) {
               
                  //  The first line should just be that first comment line,
                  //  something like "# Target and Standard Star file ..."
                  //  All we need do with that is remember it.
                  
                  FileHeader->HeaderLines.push_back(LineString);
                  
               } else if (LineNumber == 2) {
               
                  //  The second line should be a pseudo-comment that gives the
                  //  field centre Ra,Dec, eg
                  //  "# 336.5224915 -31.106872600000003"
                  //  This always seems to be space-separated. We extract the
                  //  Ra,Dec values, and remember the header line. This we can
                  //  check, because this should be the same for both files, so
                  //  if a field centre has already been set - ie if we're reading
                  //  the second (or any subsequent) file, we see if it matches.

                  FileHeader->HeaderLines.push_back(LineString);
                  if (ItemCount != 3) {
                     snprintf (Error,sizeof(Error),
                         "Line %d: Unexpected number of tokens in: '%s'",
                                                                LineNumber,Line);
                     ProgDetails->Ok = false;
                     ProgDetails->Error = Error;
                     break;
                  }
                  
                  //  The exact comparison of a floating point number should be
                  //  dodgy, but I'm assuming both have been generated from the
                  //  same program, so really should be identical. I doubt if
                  //  this is something to exit the program for, but we should
                  //  at least log it.
                  
                  double CentreRa = atof(Tokens[1].c_str()) * DD2R;
                  if (ProgDetails->CentreRa == 0.0) {
                     ProgDetails->CentreRa = CentreRa;
                  } else {
                     if (ProgDetails->CentreRa != CentreRa) {
                        snprintf (Error,sizeof(Error),
                           "Line %d: Mismatch in centre RA value: %f, %f",
                                       LineNumber,ProgDetails->CentreRa,CentreRa);
                        ProgDetails->Warnings.push_back(string(Error));
                     }
                  }
                  double CentreDec = atof(Tokens[2].c_str()) * DD2R;
                  if (ProgDetails->CentreDec == 0.0) {
                     ProgDetails->CentreDec = CentreDec;
                  } else {
                     if (ProgDetails->CentreDec != CentreDec) {
                        snprintf (Error,sizeof(Error),
                           "Line %d: Mismatch in centre Dec value: %f, %f",
                                     LineNumber,ProgDetails->CentreDec,CentreDec);
                        ProgDetails->Warnings.push_back(string(Error));
                     }
                  }

               } else if (LineNumber == 3) {
               
                  //  The third line should just be that third comment line,
                  //  something like "# Proximity Value: 217.616"
                  //  All we need do with that is remember it.
                  
                  FileHeader->HeaderLines.push_back(LineString);

               } else if (LineNumber == 4) {
               
                  //  The fourth line is the most informative of the header lines,
                  //  in terms of telling us what to expect in the lines describing
                  //  the objects. It gives the number of fields in each object line,
                  //  and should even tell us which give the Ra and Dec values -
                  //  I'm trying to play safe here and not assume they're always the
                  //  2nd and 3rd fields. The first field should be the ID field
                  //  that gives the name of the object, but we don't need that for
                  //  this program, so don't treat it in any sort of special way.
                  
                  ObjectItems = ItemCount;
                  for (int I = 0; I < ObjectItems; I++) {
                     if (TcsUtil::MatchCaseBlind(Tokens[I],"RA")) {
                        RaItem = I;
                     }  else if (TcsUtil::MatchCaseBlind(Tokens[I],"Dec")) {
                        DecItem = I;
                     }
                  }
                  if (RaItem < 0) {
                     snprintf (Error,sizeof(Error),
                        "Line %d: Could not find RA in: '%s'",LineNumber,Line);
                     ProgDetails->Warnings.push_back(string(Error));
                  }
                  if (DecItem < 0) {
                     snprintf (Error,sizeof(Error),
                        "Line %d: Could not find Dec in: '%s'",LineNumber,Line);
                     ProgDetails->Warnings.push_back(string(Error));
                  }

                  //  We can live without most of the other fields, but we do need
                  //  Ra,Dec positions for each object. And we need to remember
                  //  which fields are Ra and Dec for when we write out the
                  //  sky fibre details.
                  
                  if (RaItem < 0 || DecItem < 0) {
                     ProgDetails->Ok = false;
                     break;
                  }
                  ProgDetails->RaItem = RaItem;
                  ProgDetails->DecItem = DecItem;
                  
                  //  Now, things are different for the galaxy and the guide files,
                  //  because they may have different sets of items. We make no
                  //  assumptions about most of the fields, but we do assume that:
                  //  a) Both guide and galaxy files have Ra and Dec fields.
                  //  b) The galaxy files will list more items than the guide files,
                  //  but all the items in the guide files will also be in the
                  //  galaxy files. (This means we can treat the galaxy list as
                  //  the superset of both lists, which simplifies things a lot.
                  //  When we output the items, we have to mess with the guide lines
                  //  to make the fields match the galaxy lines, with lots of null
                  //  fields inserted.)
                  
                  if (FileType == GALAXY) {

                     //  For the galaxy file, we just record the list of fields,
                     //  which we assume is the superset list. We keep the simple
                     //  string, which makes things easy, and we also save the
                     //  parsed set of fields, which we'll need when this routine
                     //  is called again to read the guide file.
                  
                     ProgDetails->ListOfFields = LineString;
                     ProgDetails->FieldNames = Tokens;
                     
                  } else {
                  
                     //  For the guide file, we need to see how the list of fields
                     //  we have now compares to the list we got from the galaxy
                     //  file. We check that all the guide fields are present in the
                     //  galaxy list (ie that it really is a superset), and we note
                     //  which fields in that superset list are supplied in the guide
                     //  details, and which guide fields they correspond to.
                     
                     int NFields = ProgDetails->FieldNames.size();
                     ProgDetails->GuideFieldIndices.resize(NFields);
                     for (int IList = 0; IList < NFields; IList++) {
                        ProgDetails->GuideFieldIndices[IList] = -1;
                     }
                     
                     //  Go through each of the guide field items and see if it
                     //  is in the superset list of items.
                     
                     for (unsigned int IGuideItem = 0; IGuideItem < Tokens.size();
                                                                 IGuideItem++) {
                        int ListMatchItem = -1;
                        for (int IList = 0; IList < NFields; IList++) {
                           if (TcsUtil::MatchCaseBlind(Tokens[IGuideItem],
                                          ProgDetails->FieldNames[IList])) {
                              ListMatchItem = IList;
                              break;
                           }
                        }
                        
                        //  If we found it (at field entry ListMatchItem) then set
                        //  the guide field index for the list item to indicate
                        //  this entry in the guide field items. If not, issue a
                        //  warning - perhaps this should be an error?
                        
                        if (ListMatchItem < 0) {
                           snprintf (Error,sizeof(Error),
                              "Line %d: Could not find guide field '%s' in '%s'",
                                        LineNumber,Tokens[IGuideItem].c_str(),
                                          ProgDetails->ListOfFields.c_str());
                           ProgDetails->Warnings.push_back(string(Error));
                        } else {
                           ProgDetails->GuideFieldIndices[ListMatchItem] =
                                                                     IGuideItem;
                        }
                     }
                  }

               } else if (ItemCount == 0) {
               
                  //  This is a blank line. I don't expect to see them, but
                  //  it should be safe to ignore them.

                  FileHeader->HeaderLines.push_back(LineString);

               } else if (IsComment) {
               
                  //  This is a line starting with '#', but not part of the
                  //  'standard' header. I'm not sure if these might be expected,
                  //  or what to do with them if we find them. For the moment,
                  //  I'm adding them to the set of header lines.
                  
                  FileHeader->HeaderLines.push_back(LineString);

               } else {
               
                  //  This is one of the lines giving the details for an
                  //  object. We want to check that we have the expected number
                  //  of fields, and we want to extract the Ra and Dec values
                  //  because that's what this program is mostly about. We need to
                  //  keep all the details for when we write the output file,
                  //  however. We also need the object name, and - if they were
                  //  specified, the magnitude and Re values. (Actually, it seems
                  //  the real intent is that the output file contain all the values
                  //  from the input file, so all we really need to do is record
                  //  the whole original input line.
                  
                  if (ItemCount != ObjectItems) {
                     snprintf (Error,sizeof(Error),
                                 "Line %d: Expected %d values, read %d: '%s'",
                                        LineNumber,ObjectItems,ItemCount,Line);
                     ProgDetails->Ok = false;
                     ProgDetails->Error = Error;
                     break;
                  }
                  TargetCount++;
                  HectorTarget ThisTarget;
                  
                  //  We will have bailed out if RaItem and DecItem were
                  //  not set in parsing the header. The others can be allowed to
                  //  default to the values defined by the structure.
                  
                  ThisTarget.OriginalLine = LineString;
                  double Ra = atof(Tokens[RaItem].c_str());
                  double Dec = atof(Tokens[DecItem].c_str());
                  ThisTarget.MeanRa = Ra * DD2R;
                  ThisTarget.MeanDec = Dec * DD2R;
                  ThisTarget.Type = FileType;

                  (*TargetList).push_back(ThisTarget);
                  if (FileType == GUIDE) GuideCount++;
               }
               
            } else {
            
               //  Error or Eof. Check which. On EOF, we've read the whole file.
               //  Otherwise, it's an error. Either way, break out of the loop.
               
               if (!feof(TargetFile)) {
                  snprintf (Error,sizeof(Error),
                              "Error reading from, target file: '%s'",
                                                 TargetFileName.c_str());
                  ProgDetails->Ok = false;
                  ProgDetails->Error = Error;
               }
               break;
            }
         }
         
         //  Originally, I had no way of getting the field centre -
         //  it wasn't clear that it was to be found in the file
         //  headers - so instead I fell back on calculating the median
         //  position of the various targets, assuming this will be
         //  a reasonable value to work with until the proper mechansim
         //  for telling this program the field centre emerges.
   
         //  This code can be removed, of course, but for now I'm leaving
         //  it in, disabled, because it seems interesting to compare its
         //  values with the centre Ra,Dec read from the header (which are
         //  the values now used by the rest of the code).
         
         const bool CheckFieldCentre = false;         // Set true to re-enable
         if (CheckFieldCentre && ProgDetails->Ok) {
            if (TargetCount > 0) {

               float* RaValues = (float*) malloc(TargetCount * sizeof(float));
               float* DecValues = (float*) malloc(TargetCount * sizeof(float));
               if (RaValues && DecValues) {
                  for (int I = 0; I < TargetCount; I++) {
                     RaValues[I] = (*TargetList)[I].MeanRa;
                     DecValues[I] = (*TargetList)[I].MeanDec;
                  }
                  float MedianRa = gen_qfmed_(RaValues,&TargetCount);
                  float MedianDec = gen_qfmed_(DecValues,&TargetCount);
                  printf ("Centre RA, %f, median %f\n",
                                   ProgDetails->CentreRa * DR2D, MedianRa * DR2D);
                  printf ("Centre Dec, %f, median %f\n",
                                   ProgDetails->CentreDec * DR2D, MedianDec * DR2D);
                  free (RaValues);
                  free (DecValues);
               }
            }
         }
         fclose(TargetFile);
      }
   }
   
}

// ----------------------------------------------------------------------------------

//                        P a r s e  O b s  T i m e
//
//  The parsing of the observation time string is sufficiently complex that it
//  is best left to a separate routine, namely this. This routine is passed the
//  string supplied as part of the command line arguments that specifies the
//  observing time, and uses it to set the Mjd field in the ProgDetails structure.
//  If no string is specified, a null string can be passed to this routine, in
//  which case it will calculate a default Mjd that can be used for testing.

void ParseObsTime (const string& ObsTime,HectorUtilProgDetails* ProgDetails)
{
   //  The observing time is specified as a UT date and time, in the form
   //  "2020 01 28 15 30 0.00", ie UT date year, month, day followed by UT time
   //  hour, minute, second. If no observing time argument is specified, a default
   //  will be calulated in SetObsDetails(). If an argument is specified, the
   //  parsing is slightly complex. The date must be specified, but a time can
   //  be allowed to default, essentially to midnight. If a time is specified,
   //  it can be done in one of two ways, either using the hour minute sec
   //  fields, with minute and second fields defaulting to zero, or the UT
   //  can be specified with a fraactional day, eg "2020 01 28.6".

   if (ObsTime != "") {
   
      //  The general structure of this block of code comes from the 2dF
      //  configure code in tdFparse.c, although that does not support the
      //  use of explicit hours, minutes and seconds to specify a fractional
      //  day. Here, we have been given an observation time string, and need
      //  to split it up into its component parts.
      
      ProgDetails->DateAndTime = ObsTime;
      string Error = "";
      vector<string> Tokens;
      TcsUtil::Tokenize(ObsTime,Tokens);
      int Items = Tokens.size();
      if (Items < 3) {
         Error = "Need at least year, month, day for observing time";
      } else {
      
         //  Get the Year, month and day fields. Check to see if the day is
         //  fractional, because we need to know if a fraction was specified
         //  explicitly.
         
         //  Note, this code catches some errors, but error reporting could be
         //  better. In particular, there isn't a check that the various numeric
         //  strings are actually valid numbers, since the atoi() and atod()
         //  routines fail silently.
         
         int Uty = 0,Utm = 0,Utd = 0;
         double FracDay = 0.0;
         Uty = atoi(Tokens[0].c_str());
         Utm = atoi(Tokens[1].c_str());
         if (Tokens[2].find_first_of('.') == string::npos) {
            Utd = atoi(Tokens[2].c_str());
         } else {
            double Day = atof(Tokens[2].c_str());
            Utd = int(Day);
            FracDay = Day - double(Utd);
         }
         
         //  Similarly, get any time of day fields - we don't need them all,
         //  so any missing ones can default to zero.
         
         int Hour = 0;
         int Min = 0;
         double Sec = 0;
         if (Items > 3) Hour = atoi(Tokens[3].c_str());
         if (Items > 4) Min = atoi(Tokens[4].c_str());
         if (Items > 5) {
            if (Tokens[5].find_first_of('.') == string::npos) {
               Sec = double(atoi(Tokens[5].c_str()));
            } else {
               Sec = atof(Tokens[5].c_str());
            }
         }
         
         //  If a time of daya was specified, calculate the fractional day
         //  value on that basis.
         
         if (Items > 3) {
            if (FracDay != 0.0) {
               Error = "Cannot specify both a time of day and a fractional day";
            } else {
               FracDay = double(Hour)/24.0 + double(Min)/(24.0 * 60.) +
                                                 Sec/(24.0 * 60.0 * 60.0);
            }
         } else {
         
            //  If no fractional day was specified at all, assume .5, which
            //  allows for the half day difference (roughly) between Australia
            //  and Greenwich.
            
            if (FracDay == 0.0) FracDay = 0.5;
         }
      
         //  Allow for years specified using two digits. (SlaCldj() itself
         //  allows for 2 digit years, but the 2dF code chose to insert these
         //  tests, and I've left them in for the moment.
         
         if ((Uty > 0) && (Uty < 100)) {
            if (Uty <= 50) {
               Uty += 2000;
            } else if (Uty >= 70) {
               Uty += 1900;
            }
         }
         if (Uty < 1970) {
            Error = "Illegal UT year, 2dF does not support dates before 1970";
         }

         //  Finally, we can calculate the Mjd value.
      
         if (Error == "") {
            double Mjd = 0.0;
            int Jstat = 0;
            slaCldj(Uty,Utm,Utd,&Mjd,&Jstat);
            if (Jstat != 0) {
               Error = "Invalid UT date";
            }
            ProgDetails->Mjd = Mjd + FracDay;
         }
      }

      if (Error != "") {
         ProgDetails->Ok = false;
         ProgDetails->Error = Error + ": " + ObsTime;
      }

   } else {
   
      //  This is the case where no observation time was specified. In this
      //  case, let's just use halfway through the current UT day, which will be
      //  sometime in the middle of tonight in Oz. First, get today's date. Then
      //  we use the code from ConfSetUTMeridian() in configure.c to set the centre
      //  of the field on the meridian, given the centre of the field and the
      //  longitude of the AAT.
      
      std::time_t TimeNow = std::time(0);
      std::tm* UT = std::gmtime(&TimeNow);
      int Year = (UT->tm_year + 1900);
      int Month = (UT->tm_mon + 1);
      int Day = UT->tm_mday;
      double Mjd = 0.0;
      int Jstat = 0;
      slaCaldj (Year,Month,Day,&Mjd,&Jstat);
      Mjd += 0.5;
      
      double Longit = 149.0673 * DD2R;
      double Lst = slaGmst(Mjd) - Longit + slaEqeqx(Mjd);
      Mjd = Mjd + (slaDrange(ProgDetails->CentreRa - Lst)) / D2PI / 1.0027379;
      char Sign[1];
      int Ihmsf[4];
      double Frac;
      slaDd2tf(0, Mjd - floor(Mjd), Sign, Ihmsf);
      slaDjcl(Mjd, &Year, &Month, &Day, &Frac, &Jstat);
      printf("UT set to %.4d/%.2d/%.2d  %.2d:%.2d:%.2d to put field on meridian\n",
               Year, Month, Day, Ihmsf[0], Ihmsf[1], Ihmsf[2]);
      ProgDetails->Mjd = Mjd;
   }
}


// ----------------------------------------------------------------------------------

//                      G e t  O b s  D e t a i l s
//
//  This routine fills up the structure containing details of the observation -
//  date, time, met conditions etc.

void GetObsDetails (
   HectorObsDetails *ObsDetails,
   HectorUtilProgDetails* ProgDetails)
{
   if (!ProgDetails->Ok) return;
   
   //  We don't ask how, but we assume the name of the 2dF distortion file is
   //  in ObsDetails.
   
   ObsDetails->DistFilePath = ProgDetails->DistFileName;
   
   //  We need to add the actual initialisation of the observation details. Normally,
   //  we expect anything that isn't going to be allowed to default to have been
   //  supplied on the command line, so it will now be in ProgDetails.
   
   //  These met values are those used for 2dF in the testharness program, which
   //  is probably as good a set of defaults as any. They'll do for the moment
   //  as a set of default values.
   
   ObsDetails->Temp = 285;        // Temperature in degrees Kelvin.
   ObsDetails->Press = 900;       // Pressure in mm Hg.
   ObsDetails->Humid = 0.5;       // Humidity - as a fraction, ie 0 - 1.
   ObsDetails->ObsWave = 0.60;    // Wavelength in microns.
   ObsDetails->CenWave = 0.60;    // Wavelength in microns.

   //  Now, do we have any values in ProgDetails that will replace these defaults?
   //  At the moment, we only have specified command line parameters for the
   //  observation temperature. (And the robot temperature, but that's handled
   //  separately and not included in the observation details structure - maybe
   //  it should be? The distinction is that it's not needed to initialise the
   //  standard 2dF coordinate converter - the correction for robot temp is
   //  specific to Hector.)
   
   if (ProgDetails->ObsTemp > 0.0) ObsDetails->Temp = ProgDetails->ObsTemp;
   
   //  The field centre should be in ProgDetails, having been set in there when
   //  the input files were read.
   
   ObsDetails->CenRa = ProgDetails->CentreRa;
   ObsDetails->CenDec = ProgDetails->CentreDec;
   
   //  The observing time may have been specified - usually will be specified -
   //  in the command line parameters, as a string giving date and time. If it
   //  wasn't this string will be blank. ParseObsTime parses this string (the
   //  DateAndTime field in ProgDetails, and sets the Mjd field in the same
   //  structure. If the time string is blank, it picks a suitable default time.
   
   ParseObsTime(ProgDetails->DateAndTime,ProgDetails);
   ObsDetails->Mjd = ProgDetails->Mjd;

   //  Once we have the observation details, we can initialise the coordinate
   //  converter included in the ProgDetails structure.
   
   if (!ProgDetails->CoordConverter.Initialise(ObsDetails->CenRa,ObsDetails->CenDec,
      ObsDetails->Mjd,ObsDetails->Dut,ObsDetails->Temp,ObsDetails->Press,
         ObsDetails->Humid,ObsDetails->CenWave,ObsDetails->ObsWave,
            ObsDetails->DistFilePath)) {
      string Error = "Error initialising coordinate converter: " +
                                     ProgDetails->CoordConverter.GetError();
      ProgDetails->Ok = false;
      ProgDetails->Error = Error;
   } else {
      ProgDetails->ConverterInitialised = true;
   }
   
   //  If we are supposed to disable the telecentricity or mechanical offset
   //  calculations, do so. Should we log this in some way?
   
   if (ProgDetails->NoMechCorrection) {
      ProgDetails->CoordConverter.DisableMechOffset(true);
   }
   if (ProgDetails->NoTeleCorrection) {
      ProgDetails->CoordConverter.DisableTelecentricity(true);
   }

   
}

// ----------------------------------------------------------------------------------

//                  C o n v e r t  T a r g e t  C o o r d i n a t e s
//
//  This routine takes the list of targets and, using the observation details,
//  calculates the Ra,Dec positions for each target and sets those in the list of
//  targets.

void ConvertTargetCoordinates (
   const HectorObsDetails &ObsDetails,
   vector<HectorTarget>* TargetList,
   HectorUtilProgDetails* ProgDetails)
{
   if (!ProgDetails->Ok) return;
   
   //  It would be an internal error in this code if we got to this point with
   //  the coordinate converter not initialised properly, but we check anyway.
   
   if (!ProgDetails->ConverterInitialised) {
      ProgDetails->Error =
        "Coordinate converter not initialised: cannot convert target coordinates";
      ProgDetails->Ok = false;
   } else {
   
      //  Work through all the targets in the list we've been passed, converting
      //  the Ra,Dec positions to X,Y on the plate, setting the X,Y values in the
      //  structure describing each target.
      
      double MinX = 0.0;
      double MaxX = 0.0;
      double MaxY = 0.0;
      double MinY = 0.0;
      int NumberTargets = TargetList->size();
      for (int ITarget = 0; ITarget < NumberTargets; ITarget++) {
         double MeanRa = (*TargetList)[ITarget].MeanRa;
         double MeanDec = (*TargetList)[ITarget].MeanDec;
         double X,Y;
         if (!ProgDetails->CoordConverter.RaDec2XY(MeanRa,MeanDec,&X,&Y)) {
            char Error[1024];
            snprintf (Error,sizeof(Error),
               "Error converting Ra %f Dec %f to X,Y: %s\n",
                  MeanRa,MeanDec,ProgDetails->CoordConverter.GetError().c_str());
            ProgDetails->Ok = false;
            ProgDetails->Error = Error;
            break;
         }
         (*TargetList)[ITarget].X = X;
         (*TargetList)[ITarget].Y = Y;
         if (X > MaxX) MaxX = X;
         if (X < MinX) MinX = X;
         if (Y > MaxY) MaxY = Y;
         if (Y < MinY) MinY = Y;
      }
      
      //  It may be a useful check to list the range of calculated positions.
      
      bool ListRange = false;
      if (ListRange) {
         printf ("X range %f to %f, Y range %f to %f\n",MinX,MaxX,MinY,MaxY);
      }
   }
}

// ----------------------------------------------------------------------------------

//                  G e t  S k y  F i b r e  D e t a i l s
//
//  This routine fills the vector used for the list of sky fibres with details of
//  each of the sky fibres, particularly their X,Y positions on the plate.

void GetSkyFibreDetails (
   vector<HectorSkyFibre> *SkyFibreList,
   HectorUtilProgDetails* ProgDetails)
{
   static const double Mm2microns = 1000.0;
   
   printf ("In GetSkyFibreDetails, Ok = %d\n",ProgDetails->Ok);
   if (!ProgDetails->Ok) return;

   char Error[1024];
   string SkyFibreFileName = ProgDetails->SkyFibreFileName;
   
   FILE* SkyFibreFile = fopen(SkyFibreFileName.c_str(),"r");
   if (SkyFibreFile == NULL) {
      ProgDetails->Error = "Error opening sky fibre file: " + SkyFibreFileName;
      ProgDetails->Ok = false;
   } else {
      int AFibres = 0;
      int HFibres = 0;
      printf ("Opened file OK\n");
      int LineNumber = 0;
      for (;;) {
         char Line[1024];
         if (fgets (Line,sizeof(Line),SkyFibreFile)) {
         
            //  Read a line. Parse it, after removing any newline or return
            //  characters and truncating after the last non-blank character.
            
            LineNumber++;
            Line[sizeof(Line) - 1] = '\0';
            int LastNonBlank = -1;
            for (unsigned int I = 0; I < sizeof(Line); I++) {
               if (Line[I] == '\0') break;
               if (Line[I] == '\n' || Line[I] == '\r') {
                  Line[I] = '\0';
                  break;
               }
               if (Line[I] != ' ') LastNonBlank = I;
            }
            Line[LastNonBlank + 1] = '\0';
            string LineString = Line;
            
            //  Tokenize the line. This is a .csv file, so the only separator
            //  we care about is a comma.
            
            vector<string> Tokens;
            TcsUtil::Tokenize(LineString,Tokens,",");
            int ItemCount = Tokens.size();
            
            //  The only lines we care about are the fibre position lines,
            //  so we ignore any line that doesn't start with 'A' and a
            //  number or 'H' and a number. Look carefully at that first
            //  item, and if it looks OK, see if the second item is an
            //  integer fibre number.
            
            bool FibreLine = false;
            char Type = ' ';
            long TypeNo = 0;
            long FibreNo = 0;
            if (ItemCount > 2) {
               string Item = Tokens[0];
               if (Item.size() >= 2) {
                  Type = toupper(Item[0]);
                  if (Type == 'H' || Type == 'A') {
                     if (ValidInteger(Item.substr(1),&TypeNo)) {
                        Item = Tokens[1];
                        if (ValidInteger(Tokens[1],&FibreNo)) {
                           FibreLine = true;
                        }
                     }
                  }
               }
            }

            if (FibreLine) {
            
               if (Type == 'H') HFibres++;
               else AFibres++;
            
               HectorSkyFibre FibreDetails;
               FibreDetails.SubplateType = Type;
               FibreDetails.SubplateNo = TypeNo;
               FibreDetails.FibreNumber = FibreNo;

               //  This line starts properly, so we expect the rest of the
               //  fields to be three blanks, followed by 12 real numbers
               //  giving the fibre position data.
               
               if (ItemCount < 17) {
                  snprintf (Error,sizeof(Error),
                         "Line %d: Not enough position values in: '%s'",
                                                            LineNumber,Line);
                  ProgDetails->Error = Error;
                  ProgDetails->Ok = false;
                  break;
               }
               for (int Posn = 0; Posn < 4; Posn++) {
                  int Index = (Posn * 3) + 4;
                  double Values[3] = {0.0,0.0,0.0};
                  for (int I = 0; I < 3; I++) {
                     if (!ValidReal(Tokens[Index + I],&Values[I])) {
                        snprintf(Error,sizeof(Error),
                           "Line %d: '%s' is not a valid position value"
                               " in '%s'",LineNumber,Tokens[Index + I].c_str(),
                                                                         Line);
                        ProgDetails->Error = Error;
                        ProgDetails->Ok = false;
                        break;
                     }
                  }
                  if (!ProgDetails->Ok) break;
                  
                  //  The original documentation for the sky fibre file said
                  //  the positions were in microns, but they seem to be in mm.
                  
                  FibreDetails.X[Posn] = Values[0] * Mm2microns;
                  FibreDetails.Y[Posn] = Values[1] * Mm2microns;
                  FibreDetails.Radius[Posn] = Values[2] * Mm2microns;
                  //printf("Fibre %c%ld %ld posn %d X %f Y %f rad %f\n",
                  //   Type,TypeNo,FibreNo,Posn,Values[0],Values[1],Values[2]);
               }
               if (!ProgDetails->Ok) break;
               SkyFibreList->push_back(FibreDetails);
            }
         
         } else {
         
            //  Error or Eof. Check which. On EOF, we've read the whole file.
            //  Otherwise, it's an error. Either way, break out of the loop.
         
            if (!feof(SkyFibreFile)) {
               snprintf (Error,sizeof(Error),
                           "Error reading from sky fibre file: '%s'",
                                              SkyFibreFileName.c_str());
               ProgDetails->Ok = false;
               ProgDetails->Error = Error;
            }
            break;
         }
      }
      fclose (SkyFibreFile);
      
      //  Check on expected fibre numbers. Allow for the possibility that
      //  no sky fibres were expected for one of the spectrographs. If fibres
      //  were expected and none were defined, that's an error. If an
      //  unexpected number were found, treat that as a warning. If we
      //  didn't expect fibres of a given type but some were defined, that's
      //  OK - we expect the fibre file to have both.
      
      if (ProgDetails->ExpectedHFibres > 0) {
         if (HFibres == 0) {
            snprintf(Error,sizeof(Error),
               "No Hector fibre positions defined. Expected %d",
                                                 ProgDetails->ExpectedHFibres);
            ProgDetails->Error = Error;
            ProgDetails->Ok = false;
         } else {
            if (HFibres != ProgDetails->ExpectedHFibres) {
               snprintf (Error,sizeof(Error),
                  "Expected positions for %d Hector fibres, got %d",
                               ProgDetails->ExpectedHFibres,HFibres);
               ProgDetails->Warnings.push_back(string(Error));
            }
         }
      }
      if (ProgDetails->ExpectedAFibres > 0) {
         if (AFibres == 0) {
            snprintf(Error,sizeof(Error),
               "No AAOmega fibre positions defined. Expected %d",
                                                 ProgDetails->ExpectedAFibres);
            ProgDetails->Error = Error;
            ProgDetails->Ok = false;
         } else {
            if (AFibres != ProgDetails->ExpectedAFibres) {
               snprintf (Error,sizeof(Error),
                  "Expected positions for %d AAOmega fibres, got %d",
                               ProgDetails->ExpectedAFibres,AFibres);
               ProgDetails->Warnings.push_back(string(Error));
            }
         }
      }
      printf ("H fibres = %d, A fibres = %d, total = %d\n",HFibres,AFibres,
                                  HFibres + AFibres);
   }
}

// ----------------------------------------------------------------------------------

//             C o n v e r t  S k y  F i b r e  C o o r d i n a t e s
//
//  This routine takes the list of sky fibres and, using the observation details,
//  calculates the X,Y positions for each fibre and sets those in the list of
//  fibres.

void ConvertSkyFibreCoordinates (
   const HectorObsDetails &ObsDetails,
   vector<HectorSkyFibre> *SkyFibreList,
   HectorUtilProgDetails* ProgDetails)
{
   if (!ProgDetails->Ok) return;
   
   //  It would be an internal error in this code if we got to this point with
   //  the coordinate converter not initialised properly, but we check anyway.
   
   if (!ProgDetails->ConverterInitialised) {
      ProgDetails->Error =
        "Coordinate converter not initialised: cannot convert sky coordinates";
      ProgDetails->Ok = false;
   } else {
   
      //  Work through all the sky fibres in the list we've been passed, converting
      //  the X,Y positions on the plate to Ra,Dec coordinates, and setting the
      //  Ra,Dec values in the structure describing each sky fibre.
      
      int NumberSkies = SkyFibreList->size();
      for (int ISky = 0; ISky < NumberSkies; ISky++) {
         for (int IPosn = 0; IPosn < 4; IPosn++) {
            double X = (*SkyFibreList)[ISky].X[IPosn];
            double Y = (*SkyFibreList)[ISky].Y[IPosn];
            double MeanDec,MeanRa;
            if (!ProgDetails->CoordConverter.XY2RaDec(X,Y,&MeanRa,&MeanDec)) {
               char Error[1024];
               snprintf (Error,sizeof(Error),
                  "Error converting X %f Y %f to Ra,Dec: %s",
                  MeanRa,MeanDec,ProgDetails->CoordConverter.GetError().c_str());
               ProgDetails->Ok = false;
               ProgDetails->Error = Error;
               break;
            }
            (*SkyFibreList)[ISky].MeanRa[IPosn] = MeanRa;
            (*SkyFibreList)[ISky].MeanDec[IPosn] = MeanDec;
         }
         if (!ProgDetails->Ok) break;
      }
   }
}

// ----------------------------------------------------------------------------------

//             C h e c k  S k y  F i b r e s  A r e  C l e a r
//
//  This routine takes the list of sky fibres and checks that none of them are
//  contaminated by being too close to any known object. This tries the various
//  possible positions for each sky fibre, and picks the best position to use,
//  picking position 1 if this is clear, then falling back on position 2 and
//  finally on position 3. If all positions are contaminated, it picks position
//  0, which essentially means that fibre is unused. This version of the
//  routine uses Profit mask files to check for contamination. This assumes
//  that the Sky fibre details have already been read into the elements of
//  the SkyFibreList, and that any necessary parameters have been set in the
//  ProgDetails structure.

void CheckSkyFibresAreClear (
   vector<HectorSkyFibre> *SkyFibreList,
   HectorUtilProgDetails* ProgDetails)
{
   if (!ProgDetails->Ok) return;
   
   //  The contamination checks are performed using a ProfitSkyCheck object,
   //  which needs to be initialised with the name of the directory containing
   //  the Profit mask files (FITS files), and the centre and radius of the
   //  field to be checked. This lets it load the various mask files that
   //  cover the fiels in question.
   
   ProfitSkyCheck SkyChecker;
   
   if (!SkyChecker.Initialise (ProgDetails->ProfitDirectory,
      ProgDetails->CentreRa * DR2D,ProgDetails->CentreDec * DR2D,
                                          ProgDetails->FieldRadius * DR2D)) {
      ProgDetails->Error = SkyChecker.GetError();
      ProgDetails->Ok = false;
      
   } else {
   
      //  Go through each fibre in the list of sky fibres. Get the Ra and
      //  Dec of each of the four positions defined for it, and - in the
      //  preferred order, ie starting at 1, then 2, then 3, see if the
      //  checker thinks that position is clear. If so, use it. If none
      //  are clear, fall back on position 0 - which doesn't need to be
      //  checked for contamination.
      
      double RadiusDeg = ProgDetails->SkyRadiusAsec / 3600.0;
      int NFibres = (*SkyFibreList).size();
      for (int IFibre = 0; IFibre < NFibres; IFibre++) {
         HectorSkyFibre* FibreDetails = &(*SkyFibreList)[IFibre];
         FibreDetails->ChosenPosn = 0;
         for (int Posn = 1; Posn < 4; Posn++) {
            double RaDeg = FibreDetails->MeanRa[Posn] * DR2D;
            double DecDeg = FibreDetails->MeanDec[Posn] * DR2D;
            bool Clear = false;
            /*
            {  // DEBUG
               printf ("Checking sky fibre %c%d %d\n",
                    FibreDetails->SubplateType,
                    FibreDetails->SubplateNo,FibreDetails->FibreNumber);
               double DelRa = RaDeg - (ProgDetails->CentreRa * DR2D);
               double DelDec = DecDeg - (ProgDetails->CentreDec * DR2D);
               double Dist = sqrt(DelRa * DelRa + DelDec * DelDec);
               printf ("RaDeg %f DecDeg %f dist from centre = %f\n",
                   RaDeg,DecDeg,Dist);
            } // end DEBUG
            */
            
            if (!SkyChecker.CheckUseForSky(RaDeg,DecDeg,RadiusDeg,&Clear)) {
               ProgDetails->Error = SkyChecker.GetError();
               ProgDetails->Ok = false;
               break;
            }
            if (Clear) {
               FibreDetails->ChosenPosn = Posn;
               break;
            }
         }
         if (!ProgDetails->Ok) break;
         printf ("Sky fibre %c%d %d position %d\n",FibreDetails->SubplateType,
             FibreDetails->SubplateNo,FibreDetails->FibreNumber,
             FibreDetails->ChosenPosn);
      }
   }
   SkyChecker.ReportRaDecRange();
   
}

// ----------------------------------------------------------------------------------

//                 S h u f f l e  S k y  F i b r e s
//
//  This routine takes the list of sky fibres and applies the sky shuffling that
//  is required for the Hector spectrograph. This will probably result in some
//  previously uncapped fibres being flagged as capped in order to achieve the
//  spacing required.

void ShuffleSkyFibres (
   vector<HectorSkyFibre> *SkyFibreList,
   HectorUtilProgDetails* ProgDetails)
{
   if (!ProgDetails->Ok) return;
}

// ----------------------------------------------------------------------------------

//                 W r i t e  O u t p u t  F i l e
//
//  This routine takes all the details collected and calculated by the program
//  and writes the output file in the required format.

void WriteOutputFile (
   const HectorFileHeader &FileHeader,
   const HectorObsDetails &ObsDetails,
   const vector<HectorTarget> &TargetList,
   const vector<HectorSkyFibre> &SkyFibreList,
   HectorUtilProgDetails* ProgDetails)
{
   if (!ProgDetails->Ok) return;
   
   //  Open the output file specified in ProgDetails.
   
   FILE* OutputFile = fopen(ProgDetails->OutputFileName.c_str(),"w");
   if (OutputFile == NULL) {
      ProgDetails->Error = "Unable to create output file: '" +
                                             ProgDetails->OutputFileName;
      ProgDetails->Ok = false;
   } else {
   
      //  Now, write out what we have, in a format that should match that defined
      //  in the Hector software document, ASD 143. (As modified through subsequent
      //  discussions with Sam Vaughan, which emphasised that all the input fields
      //  for each object needed to be included.
      
      //  Basic header
      
      fprintf(OutputFile,"#LABEL,%s\n",ProgDetails->Label.c_str());
      fprintf(OutputFile,"#PLATEID,%s\n",ProgDetails->PlateID.c_str());
 
      //  Observing details
      
      char Sign[1];
      int Ihmsf[4],Idmsf[4];
      double Frac;
      int Year,Month,Day,Jstat;
      double Mjd = ObsDetails.Mjd;
      slaDd2tf(2, Mjd - floor(Mjd), Sign, Ihmsf);
      slaDjcl(Mjd, &Year, &Month, &Day, &Frac, &Jstat);
      fprintf(OutputFile,"#UTDATE,%04d %02d %02d #Target observing date\n",
                                                           Year,Month,Day);
      fprintf(OutputFile,"#UTTIME,%02d %02d %02d.%02d #Target observing time\n",
                                   Ihmsf[0],Ihmsf[1],Ihmsf[2],Ihmsf[3]);
      slaCr2tf(2,ObsDetails.CenRa,Sign,Ihmsf);
      slaDr2af(1,ObsDetails.CenDec,Sign,Idmsf);
      fprintf(OutputFile,
         "#CENTRE,%02d %02d %02d.%02d,%c%02d %02d %02d.%01d #Field centre\n",
         Ihmsf[0],Ihmsf[1],Ihmsf[2],Ihmsf[3],
         Sign[0],Idmsf[0],Idmsf[1],Idmsf[2],Idmsf[3]);
      fputs("#EQUINOX,J2000.0\n",OutputFile);
      
      //  Now the target details. The field labels come from the ListOfFields
      //  item, which is the list exactly as read from the galaxy input file,
      //  plus the two values calculated by the program, MagnetX and MagnetY.
      
      std::string FieldList = ProgDetails->ListOfFields +
           ",MagnetX,MagnetY,SkyFibrePosition\n";
      fputs(FieldList.c_str(),OutputFile);
      int TargetCount = TargetList.size();
      for (int ITarget = 0; ITarget < TargetCount; ITarget++) {
         int X = TargetList[ITarget].X + 0.5;
         int Y = TargetList[ITarget].Y + 0.5;
         
         if (TargetList[ITarget].Type == GALAXY) {

            //  For a galaxy, this is easy. We just put out the whole of the original
            //  input line, and append the values we've calculated - ie X and Y,
            //  which are the magnet X and Y positions.
         
            fprintf(OutputFile,"%s,%d,%d\n",
                       TargetList[ITarget].OriginalLine.c_str(),X,Y);
         } else {
         
            //  For a guide star, we have to extract the values of its fields and
            //  generate an output line that has all the fields in the superset
            //  used by the galaxy, with nulls where the star data has no value
            //  and the guide star's data where available. We make use of the array
            //  of GuideFieldIndices built up when the guide file header was read and
            //  compared with that of the galaxy file header.
            
            //  First, split up the line read from the guide file for this object
            //  into its tokens (this was done already, when it was read in, and
            //  maybe what should have been saved is the list of tokens - it
            //  doesn't really matter).
 
            vector<string> Tokens;
            TcsUtil::Tokenize(TargetList[ITarget].OriginalLine,Tokens);
            int OutputItems = ProgDetails->GuideFieldIndices.size();
            int GuideItems = Tokens.size();
            
            //  Now build up the output line. This has to have something for each
            //  field included in the galaxy input file. If the GuideFileIndices
            //  array entry for this field is -1, it wasn't one of the fields
            //  supplied in the guide file, and we put in a null string. Otherwise,
            //  the GuideFileIndices value is the index into the list of tokens
            //  read from the guilde file for this object, and we use that. Fields
            //  are comma separated.
            
            string OutputLine = "";
            for (int Item = 0; Item < OutputItems; Item++) {
               int GuideItem = ProgDetails->GuideFieldIndices[Item];
               if (GuideItem >= 0) {
                  if (GuideItem < GuideItems) {
                     OutputLine = OutputLine + Tokens[GuideItem];
                  }
               }
               OutputLine = OutputLine + ',';
            }
            
            //  Note we don't need a comma at the end of OutputLine as it already
            //  ends in one.
            
            fprintf(OutputFile,"%s%d,%d\n",OutputLine.c_str(),X,Y);
         }
      }
      
      //  Now the sky fibres. For each, we generate the name in the form
      //  Sky-<T>-<N> where <T> is A or H depending on the spectrograph for
      //  the fibre, and <N> is the fibre number, eg Sky-H-23.
      
      int SkyFibreCount = SkyFibreList.size();
      int HCount = 0;
      int ACount = 0;
      for (int ISky = 0; ISky < SkyFibreCount; ISky++) {
         string Spect{SkyFibreList[ISky].SubplateType};
         int FibreNo = 0;
         if (Spect == "H") {
            HCount++;
            FibreNo = HCount;
         } else {
            ACount++;
            FibreNo = ACount;
         }
         string Name = "Sky-" + Spect + "-" + TcsUtil::FormatInt(FibreNo);
         
         //  Name is the first field for a sky fibre. Most of the rest are
         //  null, except for the Ra,Dec fields and the final X,Y and
         //  Position fields.
         
         int Posn = SkyFibreList[ISky].ChosenPosn;
         double Ra = SkyFibreList[ISky].MeanRa[Posn];
         double Dec = SkyFibreList[ISky].MeanDec[Posn];
         int X = int(SkyFibreList[ISky].X[Posn] + 0.5);
         int Y = int(SkyFibreList[ISky].Y[Posn] + 0.5);
         string OutputLine = Name;
         int NFields = ProgDetails->FieldNames.size();
         char Number[64];
         for (int IField = 1; IField < NFields; IField++) {
            if (IField == ProgDetails->RaItem) {
               snprintf(Number,sizeof(Number),"%g",Ra);
               OutputLine = OutputLine + "," + string(Number);
            } else if (IField == ProgDetails->DecItem) {
               snprintf(Number,sizeof(Number),"%g",Dec);
               OutputLine = OutputLine + "," + string(Number);
            } else {
               OutputLine = OutputLine + ",";
            }
         }
         fprintf(OutputFile,"%s,%d,%d,%d\n",OutputLine.c_str(),X,Y,Posn);
      }

      fclose(OutputFile);
   }
}

// ----------------------------------------------------------------------------------

//                      R e p o r t  R e s u l t
//
//  This routine tidies up at the end of the program, reporting any errors and
//  anything else that might be needed.

void ReportResult(
   const HectorUtilProgDetails& ProgDetails)
{
   int NWarnings = ProgDetails.Warnings.size();
   if (ProgDetails.Ok && NWarnings == 0) {
      printf ("All OK\n");
   } else {
      if (ProgDetails.Error != "") {
         fprintf (stderr,"** Error ** %s\n",ProgDetails.Error.c_str());
      }
      for (int I = 0; I < NWarnings; I++) {
         fprintf (stderr,"* Warning * %s\n",ProgDetails.Warnings[I].c_str());
      }
   }
}

// ----------------------------------------------------------------------------------

//                          M a i n  P r o g r a m

int main (int Argc, char* Argv[]) {

   //  Create the structures we work with.
   
   //  a) Structures containing anything we need from the input files that will
   //     need to be included in the output file. We have two, one for the main
   //     target file and one for the guide star target file. The information in
   //     the two should be the same, except for the details about the fields
   //     describing each object.
   
   HectorFileHeader MainFileHeader;
   HectorFileHeader GuideFileHeader;

   //  b) A structure with the observation details - time, met conditions, etc.
   
   HectorObsDetails ObsDetails;
   
   //  c) A set of structures each containing the details of a target object.
   
   vector<HectorTarget> TargetList;
   
   //  d) A set of structures each containing the details of a sky fibre.
   
   vector<HectorSkyFibre> SkyFibreList;
   
   //  e) A structure containing internal details the program needs. In particular,
   //     including an 'OK' flag that each routine tests on entry (and exits
   //     immediately if something has already gone wrong). This keeps the
   //     program structure simple, without needing to use exceptions.
   
   HectorUtilProgDetails ProgDetails;
   
   //  This is where the program starts. Set up the program details - these
   //  may depend on the command line arguments. This main routine is as simple
   //  as possible, showing the processes as a simple linear progression, which
   //  is all that's needed - this is a simple utility program. If anything goes
   //  wrong, one stage will flag this in the program details, and subsequent
   //  stages will just fall through to the end.

   SetUpProgDetails (Argc,Argv,&ProgDetails);
   
   //  Read the input files, getting any header information that has to be
   //  included in the output file, and the set of target galaxies and guide
   //  stars. We do this for the main target file, and again for the guide star
   //  file, adding all the targets to the target list.
   
   ReadInputFile (GALAXY,&MainFileHeader,&TargetList,&ProgDetails);
   ReadInputFile (GUIDE,&GuideFileHeader,&TargetList,&ProgDetails);

   //  Get the observation details. Note - we do this after we read the input
   //  file, because having read the input files allows us to set things like
   //  sensible defaults for the observing time based on the field centre.
   
   GetObsDetails (&ObsDetails,&ProgDetails);
   
   //  Optionally, list the program details for diagnostic purposes.
   
   if (false) ListProgDetails(ProgDetails);

   //  Work through the targets, both galaxies and guide stars (we don't need to
   //  distinguish between them in this program, as all we are here to do is
   //  the coordinate conversion). Add the X,Y plate positions to the structures
   //  that describe them.
   
   ConvertTargetCoordinates (ObsDetails,&TargetList,&ProgDetails);
   
   //  Get the details of the sky fibre positions.
   
   GetSkyFibreDetails (&SkyFibreList,&ProgDetails);
   
   //  Work out the Ra,Dec coordinates for each of the sky fibres.
   
   ConvertSkyFibreCoordinates (ObsDetails,&SkyFibreList,&ProgDetails);
   
   //  See which of the sky fibres are contaminated by known objects in the
   //  catalogue we use. Flag those accordingly, both as contaiminated and capped.
   
   CheckSkyFibresAreClear (&SkyFibreList,&ProgDetails);
   
   //  Perform the sky shuffling process required for the Hector spectrograph.
   //  This may result in a number of additional sky fibres being flagged as capped.
   
   ShuffleSkyFibres (&SkyFibreList,&ProgDetails);
   
   //  Write out the new target file, including the converted coordinate positions
   //  for all the target objects, and the list of sky fibres and their capped flags.
   
   WriteOutputFile (MainFileHeader,ObsDetails,TargetList,SkyFibreList,&ProgDetails);
   
   //  And that's it. Report any final problems, as will be shown in the program
   //  details structure.
   
   ReportResult(ProgDetails);

   //  If we had an error, exit with an exit code of 1. Otherwise, exit with
   //  a zero (OK) code.
   
   exit (ProgDetails.Ok ? 0 : 1);

}

// ----------------------------------------------------------------------------------

/*                        P r o g r a m m i n g  N o t e s

   o As a convention, where I pass a structure to a subroutine, I pass a reference
     if the structure isn't changed by the subroutine (and declare it as const), and
     if the structure is changed by the subroutine I pass a pointer to it. This means
     that if you see an '&' in a call, this structure is potentially changed by
     the subroutine being called. Personally, I find this useful.

   o Right now, this code is ignoring the temperature values passed in the command
     line.
 
   o I'd really like to use a more sophisticated command line parsing that would
     allow various options to be specified using -option type flags. At the moment
     the parsing allows the two supported options -notele and -noffset to appear
     anywhere on the command line, but this was bolted on on top of an original
     design that simply specified a number of positional parameters. With more
     and more parameters potentially needed, this could be more straightforward
     and much neater.
 
   o At the moment, the parameters connected with the sky fibre processing are
     a bit of a lash-up, and aren't documented - even in the header comments
     for this file, which is pretty bad.
 
   o This code originally took a specific set of fields from the Hector software
     document (ASD143) and always wrote those - and only those - to the output file.
     This was awkward, since in many cases it had no idea how to get some of these
     fields from the input files and was just using defaults. Now all the fields
     in the input galaxy file are written to the output file, without knowing
     what any of them (except Ra and Dec at the moment) mean, followed bu the
     values calculated by this program - magnet X and Y, at present. The guide files
     have fewer fields, but we explicitly assume that the fields it does specify
     are a subset of those in the galaxy file, and the the code has to work out
     which guide file field goes where in the output file by comparing the names
     of the fields from the file header.
 
   o I have not checked that the positions calculated by the program are actually
     correct, and frankly, I'm not sure how to do that!

   o The error reporting has evolved from an original set of printf() calls to
     a scheme that sets the Error field in ProgDetails whenever an error occurs
     (whenever the Ok flag gets set false, the Error field should be set and the
     'inherited status' scheme should make the program speed to an exit). However,
     moving from using printf() to remembering an error string makes some of the
     formatting messy, and in places I've fallen back on changing the printf() calls
     to snprintf() into a char array and copying that into the Error string. It
     works, but it doesn't look very C++.
 
   o At the moment, a number of things are still unclear.
     * I believe I now have the input file format understood, but it's not been
       tested on many sample files as yet. This applies to the target and
       sky fibre input files.
     * The observation data is being taken from command line arguments in the
       format I suggested in the document I sent to Julia and Sam, but I've not
       yet had any confirmation that they're happy with this - also, I missed
       the output file name from what I sent them!
     * The thermal expansion correction is not implemented. AAO have supplied a
       single coefficient of expansion, which I guess is easy enough to apply, but
       how this program knows what temperatures to convert between is completely
       unclear.
     * The sky shuffling routine is null, since I don't yet really understand the
       requirements in enough detail. I believe this is no longer required.

   o The telecentricity correction is now implemented - in HectorRaDecXY.cpp, but
     I am slightly concerned it depends on the height of the magnets used for the
     hexabundles and/or guide fibres (ie for the target objects) and so may turn
     out to depend on choices made by the configuration program, particularly
     near the boundaries of the various angle bands. If that's the case, this
     would mean that the assumption made here that all the work done by this
     program can all be done in a single, simple, preliminary step before the main
     configuration program runs, would not be true, and this code may need to be
     split into two programs - or there may need to be two modes of running the
     one program.


*/
