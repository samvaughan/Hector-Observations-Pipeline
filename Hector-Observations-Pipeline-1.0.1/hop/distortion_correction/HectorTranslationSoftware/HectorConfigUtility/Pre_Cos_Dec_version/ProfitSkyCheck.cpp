//
//                       P r o f i t  S k y  C h e c k . c p p
//
//  Function:
//     A utility class that checks sky positions for Hector using Profit masks.
//
//  Description:
//     This defines a C++ class, ProfitSkyCheck, that exists to answer the simple
//     question, "can this given Ra,Dec position be used for a sky fibre?" that is,
//     is the specified position clear of any objects - stars, galaxies, etc?.
//     This uses Profit masks, which are FITS files that cover regions of the sky
//     that will be used for HECTOR, and are essentially binary masks - each pixel
//     of the FITS image covers an area of sky (which can be calculated from the WCS
//     coordinate information in the header) and has a zero value of that area
//     of sky is clear and a non-zero value if there this something there.
//
//     For more details, see the comments in ProfitSkyCheck.h
//
//  Remaining issues: None of this has been tested in anger yet.
//
//  Author(s): Keith Shortridge, K&V  (Keith@KnaveAndVarlet.com.au)
//
//  History:
//     27th Nov 2020.  Original version.
//      4th Dec 2020.  Now fleshed out and believed to be working, baring some
//                     tidying up and some more testing. KS.
//      5th Dec 2020.  Made the main data array int instead of float. KS.
//     12th Jan 2021.  Added <string.h> to build under Linux. Now uses size_t
//                     instead of int for string positions. KS.
//     18th Jan 2021.  Added use of a DebugHandler to control debugging. KS.
//     16th Feb 2021.  Fixed a signed/unsigned warning under Linux. KS.
//     17th Feb 2021.  Trivial commenting fixes and a minor correction to one
//                     message. KS.
//     25th Feb 2021.  When more than one Profit pask covers a position, any
//                     mask showing clear is now assumed to be enough to declare
//                     the position clear - this assumes false positives are
//                     most likely to be due to transient objects. KS.

// ----------------------------------------------------------------------------------

#include <dirent.h>
#include <errno.h>
#include <algorithm>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "fitsio.h"
#include "wcslib.h"

#include "ProfitSkyCheck.h"

#include "TcsUtil.h"

using std::string;
using std::list;

//  For the moment, assume the size of a mask is square, the same for each mask,
//  and is fixed. See programming notes.

static const double C_MASK_SIZE_IN_DEG = 1.0;

//  One quite common conversion value.

static const double DegToAsec = 60.0 * 60.0;

// ----------------------------------------------------------------------------------
//
//                            C o n s t r u c t o r
//
//  This is the only constructor. I don't like initialising things in constructors
//  as it makes it harder to report errors. So all this does is initialise the
//  various instance variables.

ProfitSkyCheck::ProfitSkyCheck (void) : I_Debug("Profit")
{
   I_Initialised = false;
   I_ErrorText = "";
   
   //  The directory path and the field centre and size will be set when
   //  Initialise() is called.
   
   I_DirectoryPath = "";
   I_CentralRaDeg = 0.0;
   I_CentralDecDeg = 0.0;
   I_FieldRadiusDeg = 0.0;
   
   //  The size of each mask file is a bit of a moot point. It could be read
   //  from each file, but then we're back having to read each file at least
   //  once and having to remember it so we don't have to do this again. For
   //  the moment we assume each mask is the same size, and we hard-code the
   //  value. See programming notes.
   
   I_MaskRaSizeDeg = C_MASK_SIZE_IN_DEG;
   I_MaskDecSizeDeg = C_MASK_SIZE_IN_DEG;
   
   //  Initial impossible values for the Ra,Dec range tested.
   
   I_RaDecRange[0] = 720.0;  // Min Ra
   I_RaDecRange[1] = 720.0;  // Min Dec
   I_RaDecRange[2] = -720.0; // Max Ra
   I_RaDecRange[3] = -720.0; // Max Dec
   
   //  Set the list of debug levels currently supported by the Debug handler.
   //  If new calls to I_Debug.Log() or I_Debug.Logf() are added, the levels
   //  they use need to be included in this list.
   
   I_Debug.LevelsList("Files,SkyCheck,SkyCheckFiles,SkyCheckCoords");

}

// ----------------------------------------------------------------------------------
//
//                             D e s t r u c t o r
//
//  The destructor releases any resources used by the code.

ProfitSkyCheck::~ProfitSkyCheck ()
{
   //  At present, nothing needs to be released. The arrays allocated to hold
   //  the mask data are released automatically when the array manager closes
   //  down.
   
   //  But we will output any warnings that we'd built up.
   
   for (const string& Warning : I_Warnings) {
      printf ("Warning: %s\n",Warning.c_str());
   }
}

// ----------------------------------------------------------------------------------
//
//                             I n i t i a l i s e
//
//  This routine does a lot of the work. It works through all the FITS files in
//  the directory whose path is passed, opening them up and looking to see if they
//  contain masks that cover any part of the area of sky currently of interest.
//  If they do, it reads the mask data and adds the details about the file to
//  the list of relevant files held in I_FileDetails. Note that the files are
//  not left open - this routine reads what it needs into memory (mainly the
//  mask data itself and the necessary coordinate data) and then closes the file.

bool ProfitSkyCheck::Initialise (
   const string& DirectoryPath, double CentralRaDeg,
                               double CentralDecDeg, double FieldRadiusDeg)
{
   bool ReturnOK = false;
   
   //  This 'do' is just a sequence of operations any one of which could go wrong.
   //  If anything goes wrong, I set OkSoFar false and break out of the sequence.
   //  Anything that does go wrong is supposed to put a sufficient explanation
   //  into I_ErrorText.
   
   bool OkSoFar = true;
   
   do {

      //  Check we're not already initialised. I suppose we could forget about all
      //  the files we've already read, but I suspect this really indicates
      //  an error.
      
      if (I_Initialised) {
         I_ErrorText = "The ProfitSkyCheck object has already been initialised";
         OkSoFar = false;
         break;
      }
      
      //  Save the values we've been passed in the class instance variables.
      
      I_DirectoryPath = DirectoryPath;
      I_CentralRaDeg = CentralRaDeg;
      I_CentralDecDeg = CentralDecDeg;
      I_FieldRadiusDeg = FieldRadiusDeg;
      
      //  Get a list of all the FITS files (compressed or uncompressed) in the
      //  mask directory. This list goes into the list at I_MaskFileList.
      
      OkSoFar = GetListOfMaskFiles();
      if (!OkSoFar) break;
      
      //  Work through all the files in the directory.
         
      for (const string& MaskFile : I_MaskFileList) {
         
         I_Debug.Log ("Files","Considering " + MaskFile);
         
         //  Extract the central Ra,Dec position from the file name. If we can't
         //  decode it, we don't treat this as an error, but we ignore the file
         //  and log a warning about it. See programming notes.
         
         double FileRa,FileDec;
         if (!GetCoordsFromFileName (MaskFile,&FileRa,&FileDec)) {
            I_Warnings.push_back(I_ErrorText);
            continue;   // Handle next file.
         }
         
         //  We have an Ra,Dec from the file name. See if that overlaps the field
         //  in question. If not, we ignore the file. See programming notes - we
         //  could check the WCS data in each file instead of using the name,
         //  but we don't.
         
         if (!FileOverlapsField(FileRa,FileDec,I_MaskRaSizeDeg,I_MaskDecSizeDeg,
                              I_CentralRaDeg,I_CentralDecDeg,I_FieldRadiusDeg)) {
            continue;   // Handle next file.
         }

         //  This is a file that overlaps the field. Open it up, check the
         //  WCS data in the header against the file's claimed central Ra,Dec,
         //  read the mask data, and add the relevant details to the file list
         //  in I_FileDetails.
         
         OkSoFar = ReadAndCheckFile (MaskFile,FileRa,FileDec);
         if (!OkSoFar) break;  // Break from file loop, showing a problem.
         
         //break;  //  DEBUG - quit after one file.
      }
      if (!OkSoFar) break;
      
      //  Make sure we have some mask file data to work with - at least one
      //  file overlapped the field.
   
      if (I_FileDetails.size() == 0) {
         I_ErrorText = "Faied to find any mask files in " + I_DirectoryPath +
                " overlapping field with centre at " +
                   FormatRaDecDeg(I_CentralRaDeg,I_CentralDecDeg) + ".";
         OkSoFar = false;
         break;
      }
   
      //  Finally, if we got to here, we're initialised.
      
      I_Initialised = true;
      ReturnOK = true;

      //  We only go through the 'do' construct once - remember, it's just
      //  something to break out from if something goes wrong.
      
   } while (false);
   
   return ReturnOK;
}

// ----------------------------------------------------------------------------------

//                        S e t  D e b u g  L e v e l s
//
//  Control the level of debug output. See the DebugHandler documentation
//  for details of the general format of a Levels string - a series of comma-
//  separated specifications, all of the form [subsystem.]level with
//  'subsystem." being optional. If subsystem is omitted or set to "Profit"
//  or a wildcard that matches "Profit" (matching the constructor for I_Debug),
//  then the specified level of debugging will be active and I_Debug.Log()
//  calls specifying that level will be active. Search the code for such
//  calls to see which levels are supported.

void ProfitSkyCheck::SetDebugLevels (const std::string& Levels)
{
   I_Debug.SetLevels(Levels);
}

// ----------------------------------------------------------------------------------
//
//                G e t  C o o r d s  F r o m  F i l e  N a m e
//
//  I'm assuming that all the Profit mask files will follow a naming convention
//  that includes the coordinates of the centre of the image. This is certainly
//  the case for the example files I was sent by Luca, which all have names like
//  "segmap_gama_350.6_-32.1.fits", ie "segmap_gama_<Ra>_<Dec>.fits". I also want
//  to allow for the possibility that compressed versions of these files are
//  being used, so there might be .gz extensions following this usual name string.
//  If any of that changes, this will have to be reworked.

bool ProfitSkyCheck::GetCoordsFromFileName (const string& FileName,
                                      double* CenRaDeg, double* CenDecDeg)

{
   bool ReturnOK = false;
   
   //  Convert the file name to lower case, and try to split off the extension(s).
   //  I'm assuming it's safe to look for a ".f" from the end of the string to
   //  find the start of the extensions, since the file presumably has a .fits
   //  .fts etc extension (possibly followed by a .gz). I'm assuming that this is
   //  a file that's been selected on the basis of having a FITS extension, and
   //  they all start with ".f".
   
   string LowerName = FileName;
   std::transform(LowerName.begin(),LowerName.end(),LowerName.begin(),
                        [](unsigned char c){ return std::tolower(c); });
   size_t ExtPosn = LowerName.rfind(".f");
   if (ExtPosn != string::npos) {
   
      //  We've found a ".f". Clip off anything to the right of that, and
      //  now look for the underscore that starts the Dec part of the file name.
      
      LowerName.erase(ExtPosn);
      size_t UscorePosn = LowerName.rfind('_');
      if (UscorePosn != string::npos) {
      
         //  Extract the Dec string, try to decode it, trim it from the name
         //  string, and look for the underscore delimiting the RA part of the
         //  name and repeat the performance.
         
         string Dec = LowerName.substr(UscorePosn + 1);
         double CenDec = 0.0;
         int Items = 0;
         Items = sscanf(Dec.c_str(),"%lf",&CenDec);
         if (Items == 1) {
            LowerName.erase(UscorePosn);
            UscorePosn = LowerName.rfind('_');
            if (UscorePosn != string::npos) {
               string Ra = LowerName.substr(UscorePosn + 1);
               double CenRa = 0.0;
               Items = sscanf(Ra.c_str(),"%lf",&CenRa);
               if (Items == 1) {
               
                  //  Finally, we have a properly decoded file name.
                  
                  *CenRaDeg = CenRa;
                  *CenDecDeg = CenDec;
                  ReturnOK = true;
               }
            }
         }
      }
      if (!ReturnOK) {
         I_ErrorText = "Filename '" + FileName +
                           "' does not have Ra,Dec encoded as expected";
      }
   }
   return ReturnOK;
}


// ----------------------------------------------------------------------------------
//
//                     L o c a t e  P i x  F r o m  C o o r d s
//
//  Given the Ra,Dec coordinates of a point, and the details structure describing
//  a mask file (in particular, the coordinate information) this routine returns
//  the index values for a pixel in the mask data that contains this point. This
//  assumes that the Nx,Ny,MidRa,MidDec,DeltaRa,DeltaDec and Wcs fields in the
//  FileDetails structure have been set. Because of the slightly non-linear
//  coordinate system used for the mask files, this uses an iterative process
//  of successive linear extrapolations to locate the pixel in question, and
//  it's always possible this may fail (although in tests it usually only needs
//  about two iterations). If it fails, it returns false as the function value
//  and puts an error description in I_ErrorText. Most cases of failure should
//  be because the coordinate is outside the range of the mask. In this case,
//  Outside will be set true. If the function returns false with Outside set
//  false, there has been some internal failure to converge.

bool ProfitSkyCheck::LocatePixFromCoords (ProfitFileDetails& FileDetails,
                        double RaDeg,double DecDeg,int* Ix,int* Iy,bool* Outside)
{
   bool ReturnOK = false;
   
   //  It's convenient to have the dimensions just as Nx,Ny.
   
   int Nx = FileDetails.Nx;
   int Ny = FileDetails.Ny;
   
   //  Set up the starting point for the first iteration. We start at the centre
   //  of the image, and assume the scales in Ra and Dec are the average scales
   //  across the whole image. It's not perfect, because the Ra,Dec scales vary
   //  slightly across the image - that's why we have to iterate - but it's a good
   //  start.
   
   double FIx = (double(Nx) + 1.0) / 2.0;
   double FIy = (double(Ny) + 1.0) / 2.0;
   double MidRa = FileDetails.MidRa;
   double MidDec = FileDetails.MidDec;
   double DeltaRa = FileDetails.DeltaRa;
   double DeltaDec = FileDetails.DeltaDec;

   int Tries = 0;
   const int MaxTries = 10;
   int LastIx = 0;
   int LastIy = 0;
   
   *Outside = false;
   
   while (Tries++ < MaxTries) {
   
      //  For each iteration, we have a starting pixel with pixel coordinates
      //  [FIx,Fiy] and Ra Dec coordinates [MidRa,MidDec], and we have a value
      //  for the image scale. On the basis of these values, we work out a new
      //  pair of possible pixel coordinates [IxTry,IyTry] that we hope will
      //  point us at the pixel we want.
   
      int IxTry = int(FIx + 0.5 + ((RaDeg - MidRa) / DeltaRa));
      int IyTry = int(FIy + 0.5 + ((DecDeg - MidDec) / DeltaDec));
      if (IxTry < 1) IxTry = 1;
      if (IxTry > Nx) IxTry = Nx;
      if (IyTry < 1) IyTry = 1;
      if (IyTry > Ny) IyTry = Ny;
      
      //  Check to make sure we're going to be looking at a new pixel. If we
      //  aren't, it's probably because we've hit the edge of the image because
      //  the point we want is outside the range of the image. Anything else,
      //  and we have some unexpected algorithmic error.
      
      if (IxTry == LastIx && IyTry == LastIy) {
         if ((IxTry == LastIx && (IxTry == 1 || IxTry == Nx)) ||
                 (IyTry == LastIy && (IyTry == 1 || IyTry == Nx))) {
            I_ErrorText = "Coordinates " + FormatRaDecDeg(RaDeg,DecDeg) +
                                              " are outside the mask range";
            *Outside = true;
         } else {
            I_ErrorText = "Iteration stuck trying to locate " +
                              FormatRaDecDeg(RaDeg,DecDeg) + " in mask";
         }
         break;
      }
      *Ix = IxTry;
      *Iy = IyTry;
      LastIx = IxTry;
      LastIy = IyTry;

      //  We use GetPixelDims() to return the actual central Ra,Dec for this
      //  pixel on the basis of the WCS information for the image. It also
      //  gives us the local values for the Ra,Dec scales, based on the way the
      //  sky coordinates vary across this particular pixel.
      
      double CenRa,CenDec,LocalDRa,LocalDDec;
      if (!GetPixelDims(&FileDetails.Wcs, IxTry, IyTry, &CenRa, &CenDec,
                                                       &LocalDRa, &LocalDDec)) {
         break;   //  Error converting coords.
      }
   
      //  How well did we do?
      
      double RaErr = fabs(RaDeg - CenRa);
      double DecErr = fabs(DecDeg - CenDec);

      if (RaErr * DegToAsec < fabs(LocalDRa * 0.5) &&
                DecErr * DegToAsec < fabs(LocalDDec * 0.5)) {

         //  If we're within half a pixel of the coordinate we want, that will
         //  do nicely. We have the pixel we want.
      
         ReturnOK = true;
         break;
         
      } else {
      
         //  Otherwise, we use the centre of this pixel as our new jumping off
         //  for the next iteration, along with the new values for the image scales.
         
         FIx = double(IxTry);
         FIy = double(IyTry);
         MidRa = CenRa;
         MidDec = CenDec;
         DeltaRa = LocalDRa / DegToAsec;
         DeltaDec = LocalDDec / DegToAsec;
      }
   }
   
   //  And, there's always the possibility we didn't manage to converge - being
   //  given a point outside the image would do this, for example.
   
   if (Tries >= MaxTries) {
      I_ErrorText = "Failed to find pixel for coordinates: " +
         FormatRaDecDeg(RaDeg,DecDeg) + " after " +
         TcsUtil::FormatInt(MaxTries) + " iterations";
      ReturnOK = false;
   }

   return ReturnOK;
   
}
// ----------------------------------------------------------------------------------
//
//                          G e t  P i x e l  D i m s
//
//  A little utility that returns the centre and RA,Dec range of a specified
//  pixel. Calling this for each individual pixel is not going to be efficient -
//  much better to set up one big call to wcsp2s() - but it can provide useful
//  diagnostics and can be a useful utility to use when considering only a few
//  pixles. The Ra and Dec range are, respectively, from the centre of the
//  left edge to the centre of the right edge, and from the centre of the bottom
//  edge to the centre of the top edge. Note pixel coords (FIx,FIy) are floating
//  point, to allow positions within a pixel to be specified, and the centre of
//  bottom left pixel is (1.0,1.0). If integer pixel coordinates are selected,
//  then the values returned really do refer to the edges of the actual pixel.
//  If non-integer pixel coordinates are used, then this calculates for a 'pseudo
//  pixel' from -0.5 to + 0.5 in each pixel coordinate. If the call to wcsp2s()
//  that this uses fails, this returns false and sets an error description in
//  I_ErrorText.

bool ProfitSkyCheck::GetPixelDims (
      wcsprm* WcsPtr,double FIx,double FIy,double* CentreRaDeg,double* CentreDecDeg,
      double* DeltaRaAsec,double* DeltaDecAsec)
{
   bool ReturnOK = true;
   *DeltaRaAsec = 0.0;
   *DeltaDecAsec = 0.0;
   *CentreRaDeg = 0.0;
   *CentreDecDeg = 0.0;
   
   //  We set up a single call to wcsp2s() and pass it five coordinate pairs
   //  to convert - the centre of each edge, and the very centre of the pixel.
   
   double Pixcrd[5 * 2];
   double Imgcrd[5 * 2];
   double Skycrd[5 * 2];
   double Phi[5 * 2];
   double Theta[5 * 2];
   int Stat[5];
   int Status = 0;
   Pixcrd[0] = FIx - 0.5; Pixcrd[1] = FIy;  // Centre of left edge
   Pixcrd[2] = FIx + 0.5; Pixcrd[3] = FIy;  // Centre of right edge
   Pixcrd[4] = FIx; Pixcrd[5] = FIy + 0.5;  // Centre of top edge
   Pixcrd[6] = FIx; Pixcrd[7] = FIy - 0.5;  // Centre of bottom edge
   Pixcrd[8] = FIx; Pixcrd[9] = FIy;        // Centre of pixel
   wcsp2s(WcsPtr,5,2,Pixcrd,Imgcrd,Phi,Theta,Skycrd,Stat);
   for (int I = 0; I < 5; I++) { if (Stat[I]) Status = Stat[I]; }
   if (Status != 0) {
      char FitsError[80];
      int Status = 0;
      fits_get_errstatus (Status,FitsError);
      I_ErrorText =
         "Unable to convert at least one FITS coordinate using wcsp2s(): " +
                                                           string(FitsError);
      ReturnOK = false;
   } else {
   
      //  The converted sky coordinates give the pixel centre, and the change
      //  in Ra and Dec across the pixel can be estimated from the difference
      //  between the centre edge positions. (Note that the delta values actually
      //  change slightly as you move across the pixel, because the coordinate
      //  system isn't completely linear, but at this scale that probably doesn't
      //  matter too much. Note that the delta values can be -ve if the axis
      //  runs backwards - this is generally what the caller wants.
      
      *DeltaRaAsec = (Skycrd[2] - Skycrd[0]) * DegToAsec;
      *DeltaDecAsec = (Skycrd[5] - Skycrd[7]) * DegToAsec;
      *CentreRaDeg = Skycrd[8];
      *CentreDecDeg = Skycrd[9];
   }
   return ReturnOK;
}

// ----------------------------------------------------------------------------------
//
//                   F i l e  O v e r l a p s  F i e l d
//
//  See if a mask file with a given centre, and covering a given range in Ra
//  and Dec, overlaps with the field of interest, with a given centre and a
//  given radius. Note that this routine is not passed the name of the mask
//  file, and does not open it. It merely checks for overlap between the specified
//  mask rectangle (probably really a square) and the field circle. All arguments
//  are in degrees.
//
//  Note that this is treated as a simple exercise in coordinate geometry,
//  looking for the possible overlap of a given circle with a given rectangle
//  that is parallel to the coordinate axes. Clearly, that isn't strictly the
//  case for Profit masks, with their slightly non-linear coordinate systems,
//  but I'm assuming it's good enough.

bool ProfitSkyCheck::FileOverlapsField (
   double MaskCentreRa, double MaskCentreDec, double MaskRaRange,
      double MaskDecRange, double FieldCentreRa, double FieldCentreDec,
      double FieldRadius)
{
   //  The code here is a reworking (mostly just changing the variable names
   //  to match those passed) of the code in the second answer in
   //  https://stackoverflow.com/questions/401847/
   //                 circle-rectangle-collision-detection-intersection
   //  by e.James, to whom I'm grateful. The answer there includes a diagram
   //  in colour that I can't reproduce here, but I've extracted bits of the
   //  comments here, although without the references to the diagram, which
   //  would make it all much clearer.
   //
   //  "The first pair of lines calculate the absolute values of the x and y
   //  difference between the center of the circle and the center of the rectangle.
   //  The second pair of lines eliminate the easy cases where the circle is far
   //  enough away from the rectangle (in either direction) that no intersection
   //  is possible.
   //  The third pair of lines handle the easy cases where the circle is close
   //  enough to the rectangle (in either direction) that an intersection is
   //  guaranteed. Note that this step must be done after step 2 for the logic
   //  to make sense.
   //  The remaining lines calculate the difficult case where the circle may
   //  intersect the corner of the rectangle. To solve, compute the distance from
   //  the center of the circle and the corner, and then verify that the distance
   //  is not more than the radius of the circle."
   
   double CentreDiffRa = fabs(FieldCentreRa - MaskCentreRa);
   double CentreDiffDec = fabs(FieldCentreDec - MaskCentreDec);

   if (CentreDiffRa > (MaskRaRange * 0.5 + FieldRadius)) { return false; }
   if (CentreDiffDec > (MaskDecRange * 0.5 + FieldRadius)) { return false; }

   if (CentreDiffRa <= (MaskRaRange * 0.5)) { return true; }
   if (CentreDiffDec <= (MaskDecRange * 0.5)) { return true; }

   double CornerDistSq =
      (CentreDiffRa - MaskRaRange * 0.5) * (CentreDiffRa - MaskRaRange * 0.5) +
      (CentreDiffDec - MaskDecRange * 0.5) * (CentreDiffDec - MaskDecRange * 0.5);
   return (CornerDistSq <= (FieldRadius * FieldRadius));
}

// ----------------------------------------------------------------------------------
//
//                      R e a d  A n d  C h e c k  F i l e
//
//  Read one of the mask files, checking coordinates and possibly reading data.
//  The file name passed should be the path name of a file that's already been
//  identified as a FITS format file that is expected to have Profit mask data,
//  and which is believed to cover an area of sky that overlaps the field in
//  question, on the basis of the central Ra,Dec embedded in its filename. That
//  Ra,Dec centre value is passed so it can be checked against the actual WCS
//  data in the file. This routine accesses the header for the file, checks
//  that it holds 2D data centered as expected, read in the data and adds a
//  file details structure for the file to the list held in I_FileDetails.

bool ProfitSkyCheck::ReadAndCheckFile (
   const string& MaskFile, double FileRa, double FileDec)
{
   static const int C_MaxDims = 7;
   
   bool ReturnOK = true;
   
   //  If we want to use this file, we'll put the relevant details in here
   //  and add it to the I_FileDetails list.
   
   ProfitFileDetails FileDetails;
   
   fitsfile* Fptr = NULL;
   char* HeaderPtr = NULL;
   wcsprm* WcsPtr = NULL;

   //  There are a few steps here, and I'm using the same overall scheme as in
   //  Initialise(), with a do structure that can be broken out of if anything
   //  goes wrong.
   
   bool OKSoFar = true;
   
   do {

      //  It looks as if this is a file we're interested in. We open it up and
      //  start looking at the header values. We are mainly interested in the
      //  WCS coordinates and the size of the main data (mask) array. First,
      //  open the file.
      
      char FitsError[80];
      int Status = 0;
      I_Debug.Log ("Files","Opening file " + MaskFile);
      fits_open_file (&Fptr,MaskFile.c_str(),READONLY,&Status);
      if (Status != 0) {
         fits_get_errstatus (Status,FitsError);
         I_ErrorText = "Failed to open '" + MaskFile + "' : " + string(FitsError);
         OKSoFar = false;
         break;
      }
      
      //  Now get the dimensions of the main array from the NAXIS and NAXIS1
      //  and NAXIS2 keywords.
      
      long Dims[C_MaxDims];
      int NDims;
      fits_read_keys_lng(Fptr, const_cast<char*>("NAXIS"), 1, C_MaxDims, Dims,
                                                              &NDims, &Status);
      if (Status != 0) {
         fits_get_errstatus (Status,FitsError);
         I_ErrorText = "Failed to get dimensions of '" + MaskFile + "' : " +
                                                             string(FitsError);
         OKSoFar = false;
         break;
      }
      
      //  We expect the mask array to be 2-dimensional.
      
      if (NDims != 2) {
         I_ErrorText = "Data array in '" + MaskFile + "' is not a 2D array";
         OKSoFar = false;
         break;
      }
      
      //  Now onto the trickier part, handling the WCS coordinate system. This
      //  is a standard sequence, using hdr2str() to get the keyword values,
      //  then using wcspih() and wcsfix() to recognise and set up the wcs
      //  structure in WcsPtr that we need to do the coordinate transforms.
      //  (Some of this code was based on that used by AAOGlimpse, which would
      //  fall back on the traditional CRPIX and CRDELT etc keywords if it
      //  couldn't find useful WCS information. We assume we don't need to so
      //  that here.)
      
      int NKeys;
      int Nx = Dims[0];
      int Ny = Dims[1];
      fits_hdr2str (Fptr,1,NULL,0,&HeaderPtr,&NKeys,&Status);
      if (Status != 0) {
         fits_get_errstatus (Status,FitsError);
         I_ErrorText = "Unable to read FITS header keywords in '" + MaskFile
                                                       + "' : " + string(FitsError);
         OKSoFar = false;
         break;
      }
      
      //  Some early mask files needed some headers fixing or wcspih() rejected
      //  them. There shouldn't be any like that now, but check anyway.
      
      int NFixed,NRejected,NWcs;
      TidyHeaderFloats(HeaderPtr,NKeys,&NFixed);
      if (NFixed > 0) {
         I_Warnings.push_back("Fixed " + TcsUtil::FormatInt(NFixed) +
                     " rogue quoted floating point keywords in " + MaskFile);
      }
      Status = wcspih (HeaderPtr,NKeys,WCSHDR_all,-2,&NRejected,&NWcs,&WcsPtr);
      if (Status != 0 || NWcs <= 0) {
         I_ErrorText = "Unable to use WCS entries in '" + MaskFile +
                                                       "' : " + string(FitsError);
         OKSoFar = false;
         break;
      }
      int Statuses[NWCSFIX];
      int IntDims[C_MaxDims];
      for (int Axis = 0; Axis < C_MaxDims; Axis++) {
         IntDims[Axis] = int(Dims[Axis]);
      }
      (void) wcsfix (1,IntDims,WcsPtr,Statuses);
      
      //  At this point, WcsPtr is set up and can be used for coordinate
      //  transformations. This next block of code is probably unnecessary, but
      //  it's worth keeping for diagnostic purposes, and it provides the useful
      //  check of calculating the central Ra,Dec position for the mask and
      //  the range it covers, to compare with the coordinates from the file name
      //  and the asumed mask dimensions.
      
      //  Set up wcs2ps() to convert 5 coordinate pairs - NDims is 2. The corners
      //  of the mask, and the very centre - note that the pixel coordinates
      //  of the very centre of the bottom left pixel are 1.0,1.0, and those
      //  of the top right pixel are Nx,Ny. The pixel range of the image goes
      //  from -0.5,-0.5 to Nx+0.5,Ny+0.5. That's the FITS convention - these are
      //  floating point pixel numbers, starting from 1, not the offsets from zero
      //  that C/C++ uses when handling arrays.
      
      double Pixcrd[5 * 2];
      double Imgcrd[5 * 2];
      double Skycrd[5 * 2];
      double Phi[5 * 2],Theta[5 * 2];
      int Stat[5];
      Pixcrd[0] = 1.0; Pixcrd[1] = 1.0;
      Pixcrd[2] = float(Nx); Pixcrd[3] = 1.0;
      Pixcrd[4] = 1.0; Pixcrd[5] = float(Ny);
      Pixcrd[6] = float(Nx); Pixcrd[7] = float(Ny);
      Pixcrd[8] = (float(Nx) + 1.0) / 2.0;
      Pixcrd[9] = (float(Ny) + 1.0) / 2.0;
      Status = 0;
      wcsp2s(WcsPtr,5,NDims,Pixcrd,Imgcrd,Phi,Theta,Skycrd,Stat);
      for (int I = 0; I < 5; I++) { if (Stat[I]) Status = Stat[I]; }
      if (Status != 0) {
         fits_get_errstatus (Status,FitsError);
         I_ErrorText = "Unable to convert at least one FITS coordinate in '"
                                    + MaskFile + "' : " + string(FitsError);
         OKSoFar = false;
         break;
      }
      
      //  We now have the central position for the mask from the WCS coordinates
      //  and we can work out the approximate image range and the average delta
      //  RA/pixel and Dec/pixel values that we need to locate pixels corresponding
      //  to given Ra,Dec coordinates (using LocatePixFromCoords).
      
      double MidRa = Skycrd[8];
      double MidDec = Skycrd[9];
      double RaRange1toNx = (Skycrd[2] + Skycrd[6]) * 0.5 -
                               (Skycrd[0] + Skycrd[4]) * 0.5;
      double DecRange1toNy = (Skycrd[5] + Skycrd[7]) * 0.5 -
                               (Skycrd[1] + Skycrd[3]) * 0.5;
      double DeltaRa = RaRange1toNx / double(Nx - 1);
      double DeltaDec = DecRange1toNy / double(Ny - 1);
      
      //  We check for consistency. Is the central position what we expected
      //  from the file name? Is the mask coverage roughly what we have been
      //  assuming? If not, warn about this. A mask coverage less than expected
      //  would be more of a problem than having more than expected.
      
      if ((fabs(MidRa - FileRa) * DegToAsec > 1.0) ||
               (fabs(MidDec - FileDec) * DegToAsec > 1.0)) {
         I_Warnings.push_back("File: " + MaskFile + " image centre from WCS " +
                     FormatRaDecDeg(MidRa,MidDec) + " does not match file name");
      }
      I_Debug.Logf ("Files","Ra range = %f, Dec range = %f",
                                  fabs(RaRange1toNx),fabs(RaRange1toNx));
      if ((fabs(fabs(RaRange1toNx) - I_MaskRaSizeDeg) > I_MaskRaSizeDeg * 0.2) ||
         (fabs(fabs(RaRange1toNx) - I_MaskDecSizeDeg) > I_MaskDecSizeDeg * 0.2)) {
         I_Warnings.push_back("File: " + MaskFile + " image range from WCS " +
            FormatRaDecDeg(fabs(RaRange1toNx),fabs(DecRange1toNy)) +
                     " differs significantly from assumed range " +
                             FormatRaDecDeg(I_MaskRaSizeDeg,I_MaskDecSizeDeg));
      }
      
      //  At this point, we can set up the fields for the structure
      //  used to describe this file. Note that we copy the WCS information
      //  into this structure, so we can happily free the original version
      //  read from the file.
      
      FileDetails.Path = MaskFile;
      FileDetails.Nx = Nx;
      FileDetails.Ny = Ny;
      FileDetails.DataArray = NULL;
      FileDetails.MidRa = MidRa;
      FileDetails.MidDec = MidDec;
      FileDetails.DeltaRa = DeltaRa;
      FileDetails.DeltaDec = DeltaDec;
      memcpy (&FileDetails.Wcs,WcsPtr,sizeof(wcsprm));
      
      //  Finally, we read in the mask data. Perhaps we should check BITPIX
      //  to see what native format is being used for the data. This assumes
      //  32 bit signed int data.
      
      int** DataArray = (int**) I_ArrayManager.Malloc2D(sizeof(float),Ny,Nx);
      long long StartPixel = 1;
      long long PixelsThisTime = Nx * Ny;
      float Nullval = 0.0;
      int Anynull = 0;
      int* BaseData = (int*) I_ArrayManager.BaseArray (DataArray);
      fits_read_img(Fptr, TINT, StartPixel, PixelsThisTime, &Nullval,
                                                BaseData, &Anynull, &Status);
      if (Status != 0) {
         fits_get_errstatus (Status,FitsError);
         I_ErrorText = "Failed to read mask data from '" + MaskFile +
                                              "' : " + string(FitsError);
         I_ArrayManager.Free(DataArray);
         DataArray = NULL;
         OKSoFar = false;
         break;
      }
      
      //  All OK. Set the address of the Mask data in the file details structure,
      //  and add that structure to the list of such structures that we use to
      //  access the data from the various masks that overlap the field.
      
      FileDetails.DataArray = DataArray;
      I_FileDetails.push_back(FileDetails);
      break;
      
   } while (false);
   
   //  We can now release any resources we allocated during the process.
                  
   int IgnoreStatus = 0;
   if (HeaderPtr) fits_free_memory(HeaderPtr,&IgnoreStatus);
   HeaderPtr = NULL;
   if (WcsPtr) free(WcsPtr);
   WcsPtr = NULL;
   int Status = 0;
   if (Fptr) fits_close_file (Fptr,&Status);
   Fptr = NULL;

   ReturnOK = OKSoFar;
   
   return ReturnOK;
}


// ----------------------------------------------------------------------------------
//
//                   G e t  L i s t  O f  M a s k  F i l e s
//
//  Looks at the contents of the directory given by I_DirectoryPath and fills
//  up the list at I_MaskFileList with the full path names of all the FITS
//  files (compressed or uncompressed) in the directory. If there is an error,
//  (including there being no such files in the directory) this returns false
//  with an error description in I_ErrorText.

bool ProfitSkyCheck::GetListOfMaskFiles (void)
{
   DIR *DirPtr;
   struct dirent *EntryPtr;

   bool ReturnOK = true;
   
   //  Open the directory
   
   errno = 0;
   if ((DirPtr = opendir(I_DirectoryPath.c_str())) == NULL) {
      string ErrorString(string(strerror(errno)));
      I_ErrorText = "Unable to open mask file directory '" + I_DirectoryPath +
                                                            "': " + ErrorString;
      ReturnOK = false;
   } else {

      //  Loop through all the files in the directory, looking for FITS files.
      
      int Count = 0;
      int CompressedCount = 0;
      do {
         errno = 0;
         if ((EntryPtr = readdir(DirPtr)) != NULL) {
            string FileName = string(EntryPtr->d_name);
            
            //  Getting the file name is one thing, manipulating the file name in
            //  C++ to see if it a) is compressed, b) has a recognised FITS file
            //  extension, is more work. The transform gets a lower case version,
            //  then we look for the last extension - ie anything after the last
            //  '.'. If that is ".gz" we strip that off, and then get the extension.
            //  If that extension is ".fits" or any of the usual other options,
            //  we assume this is a FITS file.
            
            bool Compressed = false;
            bool FitsFile = false;
            string LowerName = FileName;
            std::transform(LowerName.begin(),LowerName.end(),LowerName.begin(),
                                 [](unsigned char c){ return std::tolower(c); });
            size_t ExtPosn = LowerName.rfind('.');
            if (ExtPosn != string::npos) {
               string Ext = LowerName.substr(ExtPosn);
               if (!Ext.compare(".gz")) {
                  Compressed = true;
                  LowerName.erase(ExtPosn);
                  ExtPosn = LowerName.rfind('.');
                  if (ExtPosn != string::npos) {
                     Ext = LowerName.substr(ExtPosn);
                  }
               }
               if (!Ext.compare(".fits")) FitsFile = true;
               else if (!Ext.compare(".fit")) FitsFile = true;
               else if (!Ext.compare(".fts")) FitsFile = true;
            }
            
            //  Add anything we think is a FITS file to I_MakeFileList. We add
            //  the full path name, including the directory. That should make it
            //  easier later.
            
            if (FitsFile) {
               I_MaskFileList.push_back(I_DirectoryPath + '/' + FileName);
               Count++;
               if (Compressed) CompressedCount++;
            }
            if (!FitsFile) I_Debug.Logf ("Files","'%s' is not a FITS file",
                                                            FileName.c_str());
         }
      } while (EntryPtr != NULL);
      closedir(DirPtr);
      
      //  Not finding any FITS files at all is an error.
      
      if (Count == 0) {
         I_ErrorText = "No FITS files found in mask file directory '" +
                                                       I_DirectoryPath + "'";
         ReturnOK = false;
      }
      
      I_Debug.Logf ("Files", "Found %d FITS files, %d were compressed",
                                                      Count,CompressedCount);
   }
   return ReturnOK;
}

// ----------------------------------------------------------------------------------
//
//                        C h e c k  U s e  F o r  S k y
//
//  Checks whether the specified area (centered on RaDeg,DecDeg, with a radius of
//  RadiusDeg) is clear of any catalogue objects. If it is clear, this
//  sets Clear to true. Otherwise, it sets Clear to false. Note that the bool
//  returned as the function value is used to indicate success or failure (this
//  will fail if the Catalogue was not initialised, for example), and not whether
//  or not the area is clear.

bool ProfitSkyCheck::CheckUseForSky (
   double RaDeg, double DecDeg, double RadiusDeg, bool* Clear)
{
   bool ReturnOK = false;
   
   *Clear = false;
   
   bool Checked = false;
   
   //  Check we're initialised.
   
   if (!I_Initialised) {
      I_ErrorText = "The ProfitSkyCheck object has not been initialised properly.";
   } else {
   
      //  Keep an eye on the coordinate range we're being asked to handle, for
      //  diagnostic purposes.
      
      if (RaDeg < I_RaDecRange[0]) I_RaDecRange[0] = RaDeg;
      if (DecDeg < I_RaDecRange[1]) I_RaDecRange[1] = DecDeg;
      if (RaDeg > I_RaDecRange[2]) I_RaDecRange[2] = RaDeg;
      if (DecDeg > I_RaDecRange[3]) I_RaDecRange[3] = DecDeg;

      I_Debug.Log ("SkyCheckCoords",
                       "Checking coordinates " + FormatRaDecDeg(RaDeg,DecDeg));
      
      int FileCount = 0;
      int ClearCount = 0;
      
      //  Now, this is what all the hard work has been for. We should have a list
      //  of structures, each of which describes one mask file that overlaps at
      //  least part of the field in question. At least one of these should overlap
      //  this particular position. We try to find it in each file and see if
      //  the position is clear. If any file indicates that it is obscured, we
      //  assume it is not clear. (We'd expect files to agree, if they overlap,
      //  but the images are not identical in their overlap regions. See programming
      //  notes.)
      
      ReturnOK = true;
      for (struct ProfitFileDetails& Details : I_FileDetails) {
         bool Contaminated = false;

         //  This code does rather assume that the first axis (the X-axis) of the
         //  data is Ra and the second (the Y-axis) is Dec. If anyone switches
         //  this around in the Profit files, some recoding will be needed.
         
         int Nx = Details.Nx;
         int Ny = Details.Ny;
         
         //  Find the pixel containing the point of interest. If we have an error,
         //  we bail out. LocatePixFromCoords() returns false for both an error
         //  from WCS and for a point outside the mask. Just being outside isn't
         //  an error from our point of view here, so we have to check just what
         //  it's complaining about.
         
         int PIx = 0, PIy = 0;
         bool Outside= false;
         if (!LocatePixFromCoords (Details,RaDeg,DecDeg,&PIx,&PIy,&Outside)) {
            if (Outside) {
               I_Debug.Log ("SkyCheckFiles","Not covered by " + Details.Path);
               continue;  // Not covered by this mask. Try the next.
            }
            I_Debug.Log ("SkyCheckFiles","WCS error from " + Details.Path);
            ReturnOK = false;
            break;                  // Error from WCS - treat as a real error.
         }
         I_Debug.Log ("SkyCheckFiles",
                             "Checking against data in file " + Details.Path);
         
         I_Debug.Logf ("SkyCheck","Start pixel [%d,%d], coords %s",PIx,PIy,
                                        FormatRaDecDeg(RaDeg,DecDeg).c_str());
         
         //  We want to know the local pixel to degree scale in both Ra,Dec,
         //  which we get from looking at the range covered by this central
         //  pixel. We want to look at all the pixels in a circle of the
         //  specified radius around the central one. First we work out the
         //  rectangle that encloses that circle. All the pixels we need to
         //  look at will be within that rectangle, whose pixel coordinates
         //  go from Ixst,Iyst to Ixen,Iyen. Note that here, the bottom left
         //  pixel in the image is [1,1], with a centre at [1.0,1.0].
         
         double FIx = double(PIx);
         double FIy = double(PIy);
         double CentreRaDeg,CentreDecDeg,DeltaRaAsec,DeltaDecAsec;
         if (!GetPixelDims (&Details.Wcs,FIx,FIy,&CentreRaDeg,
                                 &CentreDecDeg,&DeltaRaAsec,&DeltaDecAsec)) {
            break;  // Error from WCS.
         }
         double DeltaRaDeg = DeltaRaAsec / DegToAsec;
         double DeltaDecDeg = DeltaDecAsec / DegToAsec;
         
         I_Debug.Logf ("SkyCheck","Radius = %f asec",RadiusDeg * DegToAsec);
         I_Debug.Log  ("SkyCheck","Deltas = " +
                                    FormatRaDecDeg(DeltaRaDeg,DeltaDecDeg));
         I_Debug.Logf ("SkyCheck","Deltas = [%f,%f] pix",
               fabs(RadiusDeg / DeltaRaDeg),fabs(RadiusDeg / DeltaDecDeg));
         
         int Ixst = int(FIx - 0.5 - fabs(RadiusDeg / DeltaRaDeg));
         int Ixen = int(FIx + 0.5 + fabs(RadiusDeg / DeltaRaDeg));
         int Iyst = int(FIy - 0.5 - fabs(RadiusDeg / DeltaDecDeg));
         int Iyen = int(FIy + 0.5 + fabs(RadiusDeg / DeltaDecDeg));
         
         I_Debug.Logf ("SkyCheck",
                          "Range %d to %d, %d to %d",Ixst,Ixen,Iyst,Iyen);
         
         //  Keep the Ra,Dec ranges within the bounds of the image.
         
         if (Ixen > Nx) Ixen = Nx;
         if (Iyen > Ny) Iyen = Ny;
         if (Ixst < 1) Ixst = 1;
         if (Iyst < 1) Iyst = 1;
         
         I_Debug.Logf ("SkyCheck","Mod Range %d to %d, %d to %d",
                                                       Ixst,Ixen,Iyst,Iyen);
         
         //  Now run through all the pixels in the rectangle. For each, calculate
         //  the minimum distance from the centre of the test circle (RaDeg,DecDeg)
         //  to the rectangle formed by the pixel. We assume that in the are of
         //  interest the Ra/Dec to pixel scale is constant and given by DeltaRa,
         //  DeltaDec. We compare this minimum distance with the radius of the test
         //  circle (RadiusDeg).
         
         Checked = true;
         FileCount++;
         for (int Ix = Ixst; Ix <= Ixen; Ix++) {
            for (int Iy = Iyst; Iy <= Iyen; Iy++) {
               
               //  This gives the centre of the pixel in question and the local
               //  delta Ra,Dec values on the basis of the WCS coordinates in
               //  the file. This will be more accurate (but hardly significantly
               //  so) and much slower than the simple extrapolation actually used.
               //  But it may be useful for diagnostic purposes, or to provide
               //  a double check on things, so it stays in for now, commented out.
               
               //double WRa,WDec,DRa,DDec;
               //GetPixelDims (&Details.Wcs,Ix,Iy,&WRa,&WDec,&DRa,&DDec);

               //  Ra,Dec of the centre of the pixel in question, on the basis
               //  of the distance of the point from the centre of the test
               //  circle (whose pixel coordinates are [FIx,FIy] and whose
               //  Ra,Dec coords are [CentreRaDeg,CentreDecDeg].
               
               double Ra = (double(Ix) - FIx) * DeltaRaDeg + CentreRaDeg;
               double Dec = (double(Iy) - FIy) * DeltaDecDeg + CentreDecDeg;
               
               //  We assume the size of the pixel we're testing is DeltaRaDeg
               //  by DeltaDecDeg. The question is now one of calculating the
               //  minimum distance from a point to an axis aligned rectangle
               //  of given width. (The code here is based on that given in
               //  https://gamedev.stackexchange.com/questions/44483/how-do-i-
               //  calculate-distance-between-a-point-and-an-axis-aligned-rectangle
               //  from Sam Hocevar and isn't quite the fastest possible, but it's
               //  neat. If both of the DistRa or DistDec values are < 0.0, then
               //  the point is within the rectangle.)
               
               double DistSq = 0.0;
               double DistRa = fabs(RaDeg - Ra) - DeltaRaDeg * 0.5;
               double DistDec = fabs(DecDeg - Dec) - DeltaDecDeg * 0.5;
               if ((DistRa > 0.0) || (DistDec > 0.0)) {
                  DistSq = DistRa * DistRa + DistDec * DistDec;
               }
               if (DistSq <= (RadiusDeg * RadiusDeg)) {
                  if (Details.DataArray[Iy - 1][Ix - 1] > 0.0) {
                  
                     //  We've found one contaminated pixel, and that's all
                     //  we need. We have to break out of both the X & Y loops.
                     
                     I_Debug.Logf ("SkyCheck",
                         "Test pixel [%d,%d], is non-zero (%d)",
                                  Ix,Iy,Details.DataArray[Iy - 1][Ix - 1]);
                     Contaminated = true;
                     break;
                  }
               }
            }
            if (Contaminated) break;
         }
         
         //  At this point, we've gone through the pixels in this Profit
         //  file. If Contaminated is set, then this file shows contamination
         //  in at least one pixel within the test radius. However, if
         //  Contaminated is clear, then at least this one file shows no
         //  contamination, and we take that as an OK to use this position.
         
         if (!Contaminated) {
            I_Debug.Logf ("SkyCheck","Checked all pixels in range. All clear.");
            *Clear = true;
            ClearCount++;
            
            //  We don't need to check the rest of the Profit files now, since
            //  we've found one clear, and it may be a useful disgnostic if
            //  we actually look at all the rest of the files.
            
            if (!I_Debug.Active("SkyCheckFiles")) break;
         }
      }
      
      if (I_Debug.Active("SkyCheckFiles")) {
         I_Debug.Logf ("SkyCheckFiles",
            "Profit files checked %d, files clear %d",FileCount,ClearCount);
         if (ClearCount != 0 && ClearCount != FileCount) {
            I_Debug.Log("SkyCheckFiles","Note: Profit files disagree.");
         }
      }

      //  We've now gone through all the mask files with any overlap with the
      //  field. At least one of them should have had data for this point. If
      //  none did, the point is probably outside the specified field - maybe
      //  we should have checked that, but this is a more direct test.
      
      if (!Checked) {
         I_ErrorText = "No mask found that covers the coordinates " +
                                 FormatRaDecDeg(RaDeg,DecDeg) + ".";
         ReturnOK = false;
      }
   }
   
   return ReturnOK;
}

// ----------------------------------------------------------------------------------

//                      T i d y  H e a d e r  F l o a t s
//
//  This was introduced to handle what seemed to be a rogue file where most of the
//  WCS floating point FITS keywords were enclosed in single quote characters,
//  causing them to be rejected when wcslib parsed the header. This takes a fairly
//  simplistic approach to the problem, simply looking at all the keywords, and
//  replacing the quotes around any valid floating point numbers with blanks. There
//  may be cases where this causes problems, but it seems unlikely. It returns the
//  number of fixed keywords.
//
//  This code was lifted from AAOGlimpse, which had it added to be able to handle
//  the original Profit mask file, which did have this problem. It may not be needed
//  any more, but it does no harm. (Actually, it also provides a convenient place
//  to bodge the header values, and I've done that in testing on occasion.)

void ProfitSkyCheck::TidyHeaderFloats(char* HeaderPtr,int NKeys,int* NFixed)
{
   int Count = 0;
   
   //  Loop through each headr 'card'. See if there are two single quote characters
   //  in the 'card', and copy the characters between them into the Number string,
   //  terminating it with a nul. Record the quote locations in First and Second;
   
   for (int Key = 0; Key < NKeys; Key++) {
      char* First = NULL;
      char* Second = NULL;
      char Number[80];
      int Index = 0;
      for (int Ich = 0; Ich < 80; Ich++) {
         if (HeaderPtr[Ich] == '\'') {
            if (First == NULL) {
               First = &(HeaderPtr[Ich]);
            } else {
               Second = &(HeaderPtr[Ich]);
               Number[Index] = '\0';
               break;
            }
         } else {
            if (First) {
               Number[Index++] = HeaderPtr[Ich];
            }
         }
      }
      
      //  If Second is set, we found two quote characters. See if strtod() can
      //  parse the characters (now in Number) between them. If strtod() terminated
      //  on the final nul, the string was a valid number, and we clear out the
      //  quotes from the header.
      
      if (Second) {
         char* EndPtr;
         (void) strtod(Number,&EndPtr);
         if (*EndPtr == '\0') {
            *First = ' ';
            *Second = ' ';
            Count++;
         }
      }
      HeaderPtr += 80;
   }
   *NFixed = Count;
}

// ----------------------------------------------------------------------------------
//
//                      F o r m a t  R a  D e c  D e g
//
//  A handly little utility that formats a pair of coordinates into a string
//  of the form "[ra,dec]".

string ProfitSkyCheck::FormatRaDecDeg (double RaDeg, double DecDeg)
{
   return ("[" + TcsUtil::FormatArcsec(RaDeg * DegToAsec) +
                        "," + TcsUtil::FormatArcsec(DecDeg * DegToAsec) + "]");
}

// ----------------------------------------------------------------------------------
//
//                      R e p o r t  R a  D e c  R a n g e
//
//  A diagnostic routine that reports the range of coordinates checked
//  so far. Calling this resets the array used to keep track of the range,
//  so the effect if to report the range of coordinates converted since the
//  start fo the program or the last time this was called.

void ProfitSkyCheck::ReportRaDecRange (void)
{
   double RaMin = I_RaDecRange[0];
   double DecMin = I_RaDecRange[1];
   double RaMax = I_RaDecRange[2];
   double DecMax = I_RaDecRange[3];

   if (RaMin > 360.0) {
      printf ("No coordinates checked.\n");
   } else {
      printf ("Ra range = %f [%s] to %f [%s]\n",
                RaMin,TcsUtil::FormatArcsec(RaMin * DegToAsec).c_str(),
                RaMax,TcsUtil::FormatArcsec(RaMax * DegToAsec).c_str());
      printf ("Dec range = %f [%s] to %f [%s]\n",
                DecMin,TcsUtil::FormatArcsec(DecMin * DegToAsec).c_str(),
                DecMax,TcsUtil::FormatArcsec(DecMax * DegToAsec).c_str());
   }

   //  Reset to implausible values
   
   I_RaDecRange[0] = 720.0;  // Min Ra
   I_RaDecRange[1] = 720.0;  // Min Dec
   I_RaDecRange[2] = -720.0; // Max Ra
   I_RaDecRange[3] = -720.0; // Max Dec
}

// ----------------------------------------------------------------------------------

/*                        P r o g r a m m i n g  N o t e s

   o  I've realised that throughout this code there is an assumption that the
      first axis of the Profit files (the X-axis) is RA and the second axis
      (the Y-axis) is Dec. If that ever gets changes, someone is going to have
      to look quite carefully at this code in order to sort that out. This is
      left as an exercise to the reader. (Or as a warning not to mess with the
      Profit file format!)
 
   o  If we can't get a central Ra,Dec out of a file's name, we could always
      open it up anyway and get them from the WCS in the header. The code doesn't
      do this, but we could just call ReadAndCheckFile() with 'unknown' Ra,Dec
      values from the file name.
 
   o  Similarly, we could ignore the overlap calculated on the basis of the file
      name and get it for each file on the basis of the WCS data. In fact, we
      could ignore the file name entirely! The problem here isn't that opening a
      .fits file and just checking the WCS coordinates takes time, because it
      doesn't. I hardly noticed the time to open all 200+ files just to check
      the overlap on the basis of their WCS header values. The problem is that
      I think it makes much more sense to hold the huge Profit files in .gz
      form, and if you do that, then it makes no difference to the code - cfitsio
      handles .gz files on the fly - but it adds nearly three seconds (on my
      laptop) to the time to open each file. That's OK if you know it is one you
      need to open - there won't be many of those - but it's not OK if you're
      opening every file just to check the WCS.
 
   o  If a test coordinate is in the region where two (or more) Profit masks
      overlap, the code at present says the point is uncontaminated if any of the
      masks show clear sky aaround that position. I think this is the correct
      thing to do - looking at the images, some show transients that
      others don't - possibly satellites, asteroids, who knows? - and in this case
      perhaps you'd want to only consider a point as contaminated if all masks
      show it as contaminated. (On the other hand, in some cases one image simply
      shows a genuine object as slightly larger than another does, and in this
      case you'd probably want to take the worst case.) I doubt if this makes a lot
      of difference in practice.
 
   o  This assumes the main data array in the ProFit files is integer. It may
      be that the code should check BITPIX and use whatever format is being used
      to avoid unnecessary conversion.
*/

/*
int main(int argc, char* argv[])
{
   if (argc < 7) {
      printf ("Usage: directory name, ra, dec, radius, pointra, pointdec\n");
   } else {
      ProfitSkyCheck TheChecker;
      bool Status = TheChecker.Initialise(argv[1],atof(argv[2]),atof(argv[3]),
              atof(argv[4]));
      if (!Status) {
         printf ("Error: %s\n",TheChecker.GetError().c_str());
      }
      bool Clear = false;
      Status = TheChecker.CheckUseForSky(atof(argv[5]),atof(argv[6]),
                                                   3.0 / 3600.0,&Clear);
      if (!Status) {
         printf ("Error: %s\n",TheChecker.GetError().c_str());
      } else {
         if (Clear) printf ("Clear\n");
         else printf("Obscured\n");
      }
   }
   return 0;
}
*/

/*  A snippet of code that used to be in Initialise(), used for testing the
    iterative pixel location.
 
                  double MaxRaErr = 0.0,MaxDecErr = 0.0;
                  int IxRaAt = 0, IyRaAt = 0;
                  int IxDecAt = 0, IyDecAt = 0;
                  for (int Ix = 1; Ix < Nx; Ix += 6000) {
                     for (int Iy = 1; Iy < Ny; Iy += 6000) {
                  
                        double Ra = MidRa + (double(Ix) - MidPixX) * DeltaRa;
                        double Dec = MidDec + (double(Iy) - MidPixY) * DeltaDec;
                        double CenRa,CenDec,LocalDRa,LocalDDec;
                        GetPixelDims(WcsPtr,Ix,Iy,&CenRa,&CenDec,
                                                          &LocalDRa,&LocalDDec);
                        double RaErr = fabs(Ra - CenRa) * DegToAsec;
                        double DecErr = fabs(Dec - CenDec) * DegToAsec;
                        
                        printf ("Pixel at %d,%d\n",Ix,Iy);
                        printf ("[%f,%f] vs [%f,%f]\n",Ra,Dec,CenRa,CenDec);
                        printf ("Discrepancy %f %f\n",RaErr,DecErr);
                        printf ("Local deltas %f %f\n",LocalDRa,LocalDDec);
                        printf ("Discrepancy in pix %f %f\n",
                                   RaErr / LocalDRa, DecErr / LocalDDec);
                        printf ("Midpix %f %f\n",MidPixX,MidPixY);
 
                        double Pix = MidPixX + (CenRa - MidRa) / DeltaRa;
                        double Piy = MidPixY + (CenDec - MidDec) / DeltaDec;
                        printf ("Predicted Ix,Iy %f %f\n",Pix,Piy);
 
                        if (RaErr > MaxRaErr) {
                           MaxRaErr = RaErr; IxRaAt = Ix; IyRaAt = Iy;
                        }
                        if (DecErr > MaxDecErr) {
                           MaxDecErr = DecErr; IxDecAt = Ix; IyDecAt = Iy;
                        }
                        
                        //  See if we can locate the correct pixel for this
                        //  central coordinate.
                        
                        printf ("Pixel at [%d,%d] has coords [%f,%f]\n",Ix,Iy,
                                                                 CenRa,CenDec);
 
                        int IxFound,IyFound;
                        bool Outside;
                        if (LocatePixFromCoords (FileDetails,CenRa,CenDec,
                                                 &IxFound,&IyFound,&Outside)) {
                           printf ("Pixel for [%d,%d] found at [%d,%d]\n",
                                                      Ix,Iy,IxFound,IyFound);
                        } else {
                           printf ("Failed to locate pixel for [%d,%d]: %s\n",
                                    Ix,Iy,I_ErrorText.c_str());
                        }
                     }
                  }
                  printf ("MaxErr in Ra = %f at [%d,%d]\n",MaxRaErr,IxRaAt,IyRaAt);
                  printf ("MaxErr in Dec = %f at [%d,%d]\n",
                                                        MaxDecErr,IxDecAt,IyDecAt);
               }
            }
*/
