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
//  Remaining issues: The original version of this code was used for the first
//     Hector observing run in Dec 2021. Since then, this has been restructured
//     to allow for mask files with different Ra,Dec ranges to those for the
//     original ProFit files, and that needed  changes to the file name format.
//     These aspects are still to be tested in anger.
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
//     16th Dec 2021.  In CheckUseForSky() added SkyCheckDist debug option, and
//                     fixed problem where having a reversed Ra or Dec scale
//                     (leading to a -ve delta Ra or Dec for the pixels) would
//                     cause pixels not to be tested. In ReadAndCheckFile(), the
//                     "Files" diagnostic was giving the Ra range value for both
//                     Ra and Dec ranges. Corrected. KS.
//      6th Jan 2022.  Added GetFileDetsils(), as the start of the re-organisation
//                     to handle mask files covering different Ra,Dec ranges. KS.
//     11th Jan 2022.  Re-organisation now mainly complete. File names can now
//                     contain Ra,Dec range information as well as Ra,Dec centre
//                     positions, and those that don't can be processed and
//                     renamed so that they do. A number of new routines have
//                     been added to support this, and any variables used to
//                     assume a common range for all mask files have gone. Also
//                     the warnings about file names not matching the actual WCS
//                     coordinates now use more realistic tests. KS.

// ----------------------------------------------------------------------------------

#include <dirent.h>
#include <errno.h>
#include <algorithm>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "ProfitSkyCheck.h"

#include "TcsUtil.h"

#include "slalib.h"
#include "slamac.h"

using std::string;
using std::list;

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
   
   //  Initial impossible values for the Ra,Dec range tested.
   
   I_RaDecRange[0] = 720.0;  // Min Ra
   I_RaDecRange[1] = 720.0;  // Min Dec
   I_RaDecRange[2] = -720.0; // Max Ra
   I_RaDecRange[3] = -720.0; // Max Dec
   
   //  Set the list of debug levels currently supported by the Debug handler.
   //  If new calls to I_Debug.Log() or I_Debug.Logf() are added, the levels
   //  they use need to be included in this list.
   
   I_Debug.LevelsList(
        "Files,SkyCheck,SkyCheckFiles,SkyCheckCoords,SkyCheckDist");

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
         
         double FileRaDeg,FileDecDeg,FileRaRangeDeg,FileDecRangeDeg;
         bool HasRanges;
         string Prefix;
         string Extension;
         if (!GetCoordsFromFileName (MaskFile,Prefix,Extension,
             &FileRaDeg,&FileDecDeg,&HasRanges,&FileRaRangeDeg,&FileDecRangeDeg)) {
            I_Warnings.push_back(I_ErrorText);
            continue;   // Handle next file.
         }
         
         //  We now have two possibilities, depending on whether or not the file
         //  name included Ra,Dec range information or not.
         
         if (HasRanges) {
         
            //  If we have the Ra,Dec range from the file name, we can immediately
            //  see if the mask file overlaps the field in question.  If not, we
            //  ignore the file.

            if (!FileOverlapsField(
                        FileRaDeg,FileDecDeg,FileRaRangeDeg,FileDecRangeDeg,
                              I_CentralRaDeg,I_CentralDecDeg,I_FieldRadiusDeg)) {
               continue;   // Handle next file.
            }
            
            //  This is a file that overlaps the field. Open it up, check the
            //  WCS data in the header against the file's claimed central Ra,Dec,
            //  and range, read the mask data, and add the relevant details to
            //  the file list in I_FileDetails.
            
            OkSoFar = ReadAndCheckFile (MaskFile,FileRaDeg,FileDecDeg,
                                           FileRaRangeDeg,FileDecRangeDeg);
            if (!OkSoFar) break;  // Break from file loop, showing a problem.

         } else {
         
            //  This is the trickier case where the file name does not include
            //  range information. In this case, we have to get that range
            //  information by opening the file and examining the WCS data it
            //  contains. Then we can decide if the mask file overlaps the field
            //  in question and, if so, read its mask data and add the relevant
            //  details to I_FileDetails. This sequence is best hived off to a
            //  separate routine, CheckWCSandReadFile().
            
            OkSoFar = CheckWCSandReadFile (MaskFile,FileRaDeg,FileDecDeg,
                                          &FileRaRangeDeg,&FileDecRangeDeg);
            
            //  And we rename the file. Failure here shouldn't stop us - it
            //  may be that the files are write-protected, and that's fine.
            //  But we issue a warning.
            
            if (OkSoFar) {
               if (!RenameMaskFile (MaskFile,FileRaRangeDeg,FileDecRangeDeg)) {
                  I_Warnings.push_back(I_ErrorText);
               }
            }
         }
         
         //break;  //  DEBUG - quit after one file (useful for testing sometimes).
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
//  There is some history associated with the interface to this routine.
//  I originally assumed that all the Profit mask files would follow a naming
//  convention that included the coordinates of the centre of the image. This was
//  certainly the case for the example files I was sent by Luca, which all had names
//  like"segmap_gama_350.6_-32.1.fits", ie "segmap_gama_<Ra>_<Dec>.fits". I also
//  wanted to allow for the possibility that compressed versions of these files were
//  being used, so there might be .gz extensions following this usual name string.
//  If any of that changes, this will have to be reworked.
//
//  However, what this scheme missed was any consideration of the range in Ra,Dec
//  covered by each file. The first production version of the code simply had two
//  hard-coded values for the Ra,Dec ranges of the ProFit files, which happened to
//  all cover exactly the same square range. it eventually became clear that the code
//  would have to deal with other mask files, some of which would cover other ranges,
//  and the best scheme would be to have these ranges coded into the file names as
//  well, even if this meant re-naming files to match. (Note that the range can be
//  determined by opening the files and processing the WCS data, but this has a
//  lot of overheads, particularly when a lot of compressed files are involved.
//  Doing this once and renaming the files once this is done is acceptable, but doing
//  this each run of the program is not.)
//
//  So the code now expects to find files called either "<prefix>_<Ra>_<Dec>.fits"
//  or "prefix_<Ra>_<Dec>_<RaRange>_<DecRange>.fits" (or the .fits.gz versions).
//  If the range information is present, it returns HasRanges set to true and the
//  range values in RaRangeDeg and DecRangeDeg. Otherwise it sets HasRanges false,
//  and returns range values of zero. It also returns the prefix string used, and
//  the file extension, to simplify renaming of the file if this is needed.

bool ProfitSkyCheck::GetCoordsFromFileName (const string& FileName,
         string& Prefix, string& Extension, double* CenRaDeg, double* CenDecDeg,
         bool* HasRanges, double* RaRangeDeg, double*DecRangeDeg)
{
   bool ReturnOK = false;
   
   *HasRanges = false;
   *RaRangeDeg = 0.0;
   *DecRangeDeg = 0.0;

   //  Convert the file name to lower case, and try to split off the extension(s).
   //  I'm assuming it's safe to look for a ".f" from the end of the string to
   //  find the start of the extensions, since the file presumably has a .fits
   //  .fts etc extension (possibly followed by a .gz). I'm assuming that this is
   //  a file that's been selected on the basis of having a FITS extension, and
   //  they all start with ".f".
   
   int NValues = 0;
   double Values[4];
   string LowerName = FileName;
   std::transform(LowerName.begin(),LowerName.end(),LowerName.begin(),
                        [](unsigned char c){ return std::tolower(c); });
   size_t EndPosn = LowerName.rfind(".f");
   if (EndPosn != string::npos) {
   
      //  We've found a ".f". Clip off anything to the right of that, and
      //  now look for the underscore that starts the last numeric value in the
      //  file name. Then keep working back until we have either 4 numbers
      //  or we hit something that doesn't fit the pattern we're looking for.
      //  The first time through the loop, EndPosn points to the . beginning
      //  the file extension. Subsequent times through, it points to the underscore
      //  delimiting the start of the number we parsed in the previous loop.
      
      Extension = FileName.substr(EndPosn,string::npos);
      do {
         LowerName.erase(EndPosn);
         size_t UscorePosn = LowerName.rfind('_');
         if (UscorePosn == string::npos) break;
         string ValueStr = LowerName.substr(UscorePosn + 1);
         double Value = 0.0;
         int Items = sscanf(ValueStr.c_str(),"%lf",&Value);
         if (Items != 1) break;
         Values[NValues] = Value;
         NValues++;
         EndPosn = UscorePosn;
      } while (NValues < 4);
   }
   
   //  After all that, we should have either 2 or 4 numbers depending on whether
   //  the file name incldded the range information or not. Anythig else isn't
   //  a file name we were expecting to find, and we'll return an error.
   //  Remember the values are parsed from the end of the string moving to the
   //  beginning.
   
   if (NValues == 2) {
      *CenRaDeg = Values[1];
      *CenDecDeg = Values[0];
      *HasRanges = false;
      *RaRangeDeg = 0.0;
      *DecRangeDeg = 0.0;
      ReturnOK = true;
   } else if (NValues == 4) {
      *CenRaDeg = Values[3];
      *CenDecDeg = Values[2];
      *HasRanges = true;
      *RaRangeDeg = Values[1];
      *DecRangeDeg = Values[0];
      ReturnOK = true;
   }
   if (ReturnOK) {
      Prefix = FileName.substr(0,EndPosn);
   } else {
      I_ErrorText = "Filename '" + FileName +
                           "' does not have Ra,Dec values encoded as expected";
   }
   return ReturnOK;
}

// ----------------------------------------------------------------------------------
//
//                         R e n a m e  M a s k  F i l e
//
//  Passed the original name of a mask file, renames it to include the full Ra,Dec
//  centre position and Ra,Dec range as determined. All this does is insert the
//  range information between ar the end of the file name, in front of the extension.
//  This therefore assumes that the file already has the Centre Ra,Dec position
//  encoded in the file name. The routine does try to check this.

bool ProfitSkyCheck::RenameMaskFile (
        const string& MaskFile,double FileRaRangeDeg, double FileDecRangeDeg)
{
   bool ReturnOK = true;
   
   //  The code that follows is very similar to that in GetCoordsFromFileName(),
   //  except that ere we're using this to a) locate the position of the extension
   //  - which is the point where we will insert the range information - and b)
   //  simply check that there are only two numeric values already encoded in the
   //  filename. It's possible that this code and that in GetCoordsFromFileName()
   //  could be reqorked to get rid of this duplication, but it looks more work
   //  than it's worth. (The reason GetCoordsFromFileName() returns Prefix and
   //  Extension is that originally this was the intention, but it proved that
   //  that interface didn't quite do what was wanted here.
   
   string LowerName = MaskFile;
   std::transform(LowerName.begin(),LowerName.end(),LowerName.begin(),
                        [](unsigned char c){ return std::tolower(c); });
   bool ParsedOK = false;
   int NValues = 0;
   string Extension;
   string Prefix;
   size_t EndPosn = LowerName.rfind(".f");
   if (EndPosn != string::npos) {
   
      //  We've got the start of the extension. Note the part of the filename
      //  that goes before, and the extension itself.
      
      Extension = MaskFile.substr(EndPosn,string::npos);
      Prefix = MaskFile.substr(0,EndPosn);
      
      //  We could skip right now to formatting the new name for the file, but
      //  let's work back through the file name and see how many numeric values
      //  it has. All we're doing is counting them - the values don't matter.
      //  If we get anything other than two, we have a potential problem.
      
      do {
         LowerName.erase(EndPosn);
         size_t UscorePosn = LowerName.rfind('_');
         if (UscorePosn == string::npos) break;
         string ValueStr = LowerName.substr(UscorePosn + 1);
         double Value = 0.0;
         int Items = sscanf(ValueStr.c_str(),"%lf",&Value);
         if (Items != 1) break;
         NValues++;
         EndPosn = UscorePosn;
      } while (NValues < 4);
      if (NValues == 2) ParsedOK = true;
      
   } if (ParsedOK) {
   
      //  All looks fine. Try to rename the file.
      
      char Numbers[64];
      sprintf (Numbers,"_%.2f_%.2f",fabs(FileRaRangeDeg),fabs(FileDecRangeDeg));
      string NewName = Prefix + string(Numbers) + Extension;
      I_Debug.Logf ("Files","Renaming %s' as '%s'",MaskFile.c_str(),NewName.c_str());
      int Result = rename(MaskFile.c_str(),NewName.c_str());
      if (Result != 0) {
         string ErrorString(string(strerror(errno)));
         I_ErrorText = "Error renaming '" + MaskFile + "': " + ErrorString;
         ReturnOK = false;
      }
   } else {
   
      //  We don't like the look of the file name. Say so.
      
      I_ErrorText = "Unable to rename file '" + MaskFile +
                     "'. Existing file name does not parse as expected.";
      ReturnOK = false;
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
   //  slightly across the image - that's why we have to iterate - but it's a
   //  good start.
   
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
//  Originally, this routine treated the problem as a simple exercise in
//  coordinate geometry, looking for the intersection of a rctangle with a circle,
//  but in fact the cos(Dec) effect of declination on Ra means that the circle
//  becomes stretched out as declinations increase, and that sort of cleverness
//  is just over-complicating a problem that can be handled much more simply
//  by treating this as the overlap of two rectangles giving the extremes
//  of the Ra,Dec vlaues for a) the mask and b) the field. The important thing
//  is not to miss out any mask files; picking up a few unnecessarily adds a
//  bit of overhead, but doesn't really matter.
//  (For what it's still worth, the original rectangle/circle intersection code
//  was based on this StackOverflow answer by e.James:
//       https://stackoverflow.com/questions/401847/
//                 circle-rectangle-collision-detection-intersection

bool ProfitSkyCheck::FileOverlapsField (
   double MaskCentreRa, double MaskCentreDec, double MaskRaRange,
      double MaskDecRange, double FieldCentreRa, double FieldCentreDec,
      double FieldRadius)
{
   //  Work out the differences between the mask and field centres
   
   double CentreDiffRa = fabs(FieldCentreRa - MaskCentreRa);
   double CentreDiffDec = fabs(FieldCentreDec - MaskCentreDec);
   
   //  If either of these are greater than the combined half-widths of the rectangles
   //  (allowing for the cos(dec) effect for Ra, and not necessarily assuming the
   //  range values we were passed were absolute values) then there's no interection.
   
   double CosDec = cos(FieldCentreDec * DD2R);
   if (CentreDiffRa > (fabs(MaskRaRange) * 0.5 + FieldRadius / CosDec)) { return false; }
   if (CentreDiffDec > (fabs(MaskDecRange) * 0.5 + FieldRadius)) { return false; }
   
   //  Otherwise, they overlap in both Ra and Dec, so they intersect.
   
   return true;

}

// ----------------------------------------------------------------------------------
//
//                      C h e c k  W C S  a n d  R e a d  F i l e
//
//   This routine handles tha case of a mask file whose name does not include
//   range information. In this case the file needs to be opened and the range
//   information determined from the WCS data in the header. If the central
//   Ra,Dec and the determined range shows that this file potentially overlaps
//   the current field of interest, then the file's mask data is read in and
//   its details added to the list held in I_FileDetails. The routine is passed
//   the central Ra,Dec for the mask, which it is assumed was encoded in the
//   file name, and it checks that this matches the WCS data held in the file.
//   This routine returns the Ra,Dec range determined from the WCS data, and
//   this can be used to rename the file if required.

bool ProfitSkyCheck::CheckWCSandReadFile (
   const std::string& MaskFile, double FileRaDeg, double FileDecDeg,
                              double* FileRaRangeDeg, double* FileDecRangeDeg)
{
   bool ReturnOK = true;
   
   fitsfile* Fptr = NULL;
   ProfitFileDetails FileDetails;
   
   bool OKSoFar = true;
   
   do {
   
      //  Open the file. An error here indicates this isn't a FITS format file.
      
      OKSoFar = OpenMaskFile (MaskFile,&Fptr);
      if (!OKSoFar) break;
      
      //  Get the file details including the WCS values. An error here may just
      //  mean this isn't a mask file, althogh it may be a perfectly good FITS file.
      //  We treat this as a severe error, but maybe it should just be a warning?
      
      OKSoFar = GetFileDetails (MaskFile,Fptr,&FileDetails);
      if (!OKSoFar) break;
      
      //  Warn if the central Ra,Dec from the file name doesn't match that shown
      //  from the WCS data. Really, you'd think it should be good to a pixel, but
      //  actually the files usually only give a centre to one place of decimals
      //  in degrees, so the best we can hope for (even with proper rounding) is
      //  probably accuracy to .05 deg, and maybe we should just test for right
      //  to .1 deg - which will pick up glaring errors, but no more.
      
      double MidRa = FileDetails.MidRa;
      double MidDec = FileDetails.MidDec;
      if ((fabs(MidRa - FileRaDeg) > 0.1) || (fabs(MidDec - FileDecDeg) > 0.1)) {
         I_Warnings.push_back("File: " + MaskFile + " image centre from WCS " +
                     FormatRaDecDeg(MidRa,MidDec) + " does not match file name");
      }

      //  Get the Ra,Dec ranges from the file details, and see if this overlaps
      //  the field. If it doesn't. we don't bother to read in the file data,
      //  but this isn't an error.
      
      *FileRaRangeDeg = FileDetails.DeltaRa * (FileDetails.Nx - 1);
      *FileDecRangeDeg = FileDetails.DeltaDec * (FileDetails.Ny - 1);
      if (!FileOverlapsField(FileRaDeg,FileDecDeg,*FileRaRangeDeg,*FileDecRangeDeg,
                          I_CentralRaDeg,I_CentralDecDeg,I_FieldRadiusDeg)) break;
      
      //  If we get here, this overlaps the field. Read in its data and add to
      //  the I_FileDetails list.
      
      OKSoFar = ReadFileData (MaskFile,Fptr,&FileDetails);
   } while (false);
   
   //  Close the file - this is safe, even if the file wasn't opened properly.
   
   CloseMaskFile (Fptr);
   
   if (!OKSoFar) ReturnOK = false;
   
   return ReturnOK;
}

// ----------------------------------------------------------------------------------
//
//                         O p e n  M a s k  F i l e
//
//  Opens a namesd FITS file, returning a pointer to the fitsfile structure
//  used by the cfitsio routines. This routine returns true if the file was
//  opened OK, false (with an error description set into I_ErrorText) if there
//  was an error.

bool ProfitSkyCheck::OpenMaskFile (
   const std::string& MaskFile, fitsfile** Fptr)
{
   bool ReturnOK = true;
   
   char FitsError[80];
   int Status = 0;
   I_Debug.Log ("Files","Opening file " + MaskFile);
   fits_open_file (Fptr,MaskFile.c_str(),READONLY,&Status);
   if (Status != 0) {
      fits_get_errstatus (Status,FitsError);
      I_ErrorText = "Failed to open '" + MaskFile + "' : " + string(FitsError);
      ReturnOK = false;
   }
   return ReturnOK;

}

// ----------------------------------------------------------------------------------
//
//                         C l o s e  M a s k  F i l e
//
//  Given a pointer to the fitsfile structure as returned when the file was
//  opened by a call to fits_open_file(), this routine closes the file. Note
//  that this routine does not report errors, and can be called at the end
//  of processing the file even if errors have occurred in that processing,
//  simply to ensure that the file is not left open. If Fptr is passed as null.
//  which it will be if the original fits_open_file() call failed, this routine
//  does nothing.

void ProfitSkyCheck::CloseMaskFile (fitsfile* Fptr)
{
   int Status = 0;
   if (Fptr) fits_close_file (Fptr,&Status);
}


// ----------------------------------------------------------------------------------
//
//                      G e t  F i l e  D e t a i l s
//
//  Passed the addresss of a fitsfile structure for an already opened mask
//  file, this routine fills in a ProfitFileDetails structure with the array
//  dimensions, the Ra,Dec centre and range, and the WCS information. It
//  does not read in the data from the file. This also does some basic checks
//  on the mask file - does it contain a 2D array, for example? The routine
//  returns true if it processes the file without problems. Otherwise it
//  returns false - which may indicate that it isn't a valid FITS format file,
//  or that it doesn't have valid WCS data, or simply that it isn't a 2D file.

bool ProfitSkyCheck::GetFileDetails (
   const std::string& MaskFile, fitsfile* Fptr, ProfitFileDetails* FileDetails)
{
   static const int C_MaxDims = 7;
   
   bool ReturnOK = true;
   
   char* HeaderPtr = NULL;
   wcsprm* WcsPtr = NULL;

   //  There are a few steps here, and I'm using the same overall scheme as in
   //  Initialise(), with a do structure that can be broken out of if anything
   //  goes wrong.
   
   bool OKSoFar = true;
   
   do {

      char FitsError[80];
      int Status = 0;
      
      //  The file is already open. Get the dimensions of the main array from
      //  the NAXIS and NAXIS1 and NAXIS2 keywords.
      
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
      
      //  At this point, we can set up the fields for the structure
      //  used to describe this file. Note that we copy the WCS information
      //  into this structure, so we can happily free the original version
      //  read from the file.
      
      FileDetails->Path = MaskFile;
      FileDetails->Nx = Nx;
      FileDetails->Ny = Ny;
      FileDetails->DataArray = NULL;
      FileDetails->MidRa = MidRa;
      FileDetails->MidDec = MidDec;
      FileDetails->DeltaRa = DeltaRa;
      FileDetails->DeltaDec = DeltaDec;
      memcpy (&(FileDetails->Wcs),WcsPtr,sizeof(wcsprm));
      
      I_Debug.Logf ("Files","Mask %d by %d pixels, centre at %f, %f",Nx,Ny,MidRa,MidDec);
      I_Debug.Logf ("Files","Ra range %f deg, Dec range %f deg",RaRange1toNx,DecRange1toNy);
      
   } while (false);
   
   //  We can now release any resources we allocated during the process.
   
   int IgnoreStatus = 0;
   if (HeaderPtr) fits_free_memory(HeaderPtr,&IgnoreStatus);
   HeaderPtr = NULL;
   if (WcsPtr) free(WcsPtr);
   WcsPtr = NULL;

   ReturnOK = OKSoFar;
   
   return ReturnOK;

}

// ----------------------------------------------------------------------------------
//
//                      R e a d  F i l e  D a t a
//
//  Passed the addresss of a fitsfile structure for an already opened 2D mask
//  file, this routine reads in the mask data from that file and records the
//  address of that data in a ProfitFileDetails structure. The routine
//  returns true if it processes the file without problems. Otherwise it
//  returns false - which may indicate that it isn't a valid FITS format file.
//  If the data is read OK, its details are added to the list of mask data
//  in use held in the list I_FileDetails.

bool ProfitSkyCheck::ReadFileData (
   const std::string& MaskFile, fitsfile* Fptr, ProfitFileDetails* FileDetails)
{
   bool ReturnOK = true;
   
   char FitsError[80];
   int Status = 0;
   
   //  Note the assumption that the file is already open and contains 2D
   //  data.
   
   //  We read in the mask data. Perhaps we should check BITPIX
   //  to see what native format is being used for the data. This assumes
   //  32 bit signed int data.

   int Nx = FileDetails->Nx;
   int Ny = FileDetails->Ny;
   int** DataArray = (int**) I_ArrayManager.Malloc2D(sizeof(float),Ny,Nx);
   long long StartPixel = 1;
   long long PixelsThisTime = Nx * Ny;
   float Nullval = 0.0;
   int Anynull = 0;
   int* BaseData = (int*) I_ArrayManager.BaseArray (DataArray);
   fits_read_img(Fptr, TINT, StartPixel, PixelsThisTime, &Nullval,
                                             BaseData, &Anynull, &Status);
   if (Status == 0) {
   
      //  All OK. Set the address of the Mask data in the file details structure,
      //  and add that structure to the list of such structures that we use to
      //  access the data from the various masks that overlap the field.

      FileDetails->DataArray = DataArray;
      I_FileDetails.push_back(*FileDetails);

   } else {
   
      fits_get_errstatus (Status,FitsError);
      I_ErrorText = "Failed to read mask data from '" + MaskFile +
                                           "' : " + string(FitsError);
      I_ArrayManager.Free(DataArray);
      DataArray = NULL;
      ReturnOK = false;
   }

   return ReturnOK;
}


// ----------------------------------------------------------------------------------
//
//                      R e a d  A n d  C h e c k  F i l e
//
//  Read one of the mask files, checking coordinates and reading its data.
//  The file name passed should be the path name of a file that's already been
//  identified as a FITS format file that is expected to have Profit mask data,
//  and which is believed to cover an area of sky that overlaps the field in
//  question, on the basis of the central Ra,Dec embedded in its filename. That
//  Ra,Dec centre value is passed so it can be checked against the actual WCS
//  data in the file. This routine accesses the header for the file, checks
//  that it holds 2D data centered as expected, read in the data and adds a
//  file details structure for the file to the list held in I_FileDetails.

bool ProfitSkyCheck::ReadAndCheckFile (
   const string& MaskFile, double FileRaDeg, double FileDecDeg,
   double FileRaRangeDeg, double FileDecRangeDeg)
{
   bool ReturnOK = true;
   
   //  If we want to use this file, we'll put the relevant details in here
   //  and add it to the I_FileDetails list.
   
   ProfitFileDetails FileDetails;
   
   fitsfile* Fptr = NULL;

   //  There are a few steps here, and I'm using the same overall scheme as in
   //  Initialise(), with a do structure that can be broken out of if anything
   //  goes wrong. (There aren't as many as steps there used to be, because
   //  quite a bit of the code was moved into GetFileDetails() in Jan 2022.)
   
   bool OKSoFar = true;
   
   do {

      //  It looks as if this is a file we're interested in. We open it up and
      //  start looking at the header values. We are mainly interested in the
      //  WCS coordinates and the size of the main data (mask) array. First,
      //  open the file.
      
      fitsfile* Fptr;
      OKSoFar = OpenMaskFile (MaskFile,&Fptr);
      if (!OKSoFar) break;

      //  Now get the details of the file data array and its Ra,Dec coordinates
      //  into FileDetails. It's not quite clear to me what the best thing to
      //  do if there's an error from GetFileDetails(). It may just be that
      //  we have a FITS file in the mask file directory that isn't actually
      //  a mask. We could ignore it, but I suspect it's better to bail at
      //  this point and do something about this rogue file. GetFileDetails()
      //  will have already set I_ErrorText.
      
      OKSoFar = GetFileDetails (MaskFile,Fptr,&FileDetails);
      if (!OKSoFar) break;
      
      //  We check for consistency. Is the central position what we expected
      //  from the file name? Is the mask coverage roughly what we have been
      //  assuming? If not, warn about this. A mask coverage less than expected
      //  would be more of a problem than having more than expected. See comments
      //  to CheckWCSandReadFile() for discussion of mask centre accuracy. (A
      //  similar argument says the ranges should be good to 0.01 deg.)
      
      double MidRa = FileDetails.MidRa;
      double MidDec = FileDetails.MidDec;
      if ((fabs(MidRa - FileRaDeg) > 0.1) || (fabs(MidDec - FileDecDeg) > 0.1)) {
         I_Warnings.push_back("File: " + MaskFile + " image centre from WCS " +
                     FormatRaDecDeg(MidRa,MidDec) + " does not match file name");
      }
      
      double RaRange1toNx = FileDetails.DeltaRa * (FileDetails.Nx - 1);
      double DecRange1toNy = FileDetails.DeltaDec * (FileDetails.Ny - 1);
      I_Debug.Logf ("Files","Ra range = %f, Dec range = %f",
                                  fabs(RaRange1toNx),fabs(DecRange1toNy));
      if ((fabs(fabs(RaRange1toNx) - fabs(FileRaRangeDeg)) > 0.01) ||
         (fabs(fabs(DecRange1toNy) - fabs(FileDecRangeDeg)) > 0.01)) {
         I_Warnings.push_back("File: " + MaskFile + " image range from WCS " +
            FormatRaDecDeg(fabs(RaRange1toNx),fabs(DecRange1toNy)) +
                     " differs significantly from assumed range " +
                             FormatRaDecDeg(FileRaRangeDeg,FileDecRangeDeg));
      }
      
      OKSoFar = ReadFileData (MaskFile,Fptr,&FileDetails);
      if (!OKSoFar) break;
      
   } while (false);
   
   //  We can now release any resources we allocated during the process.
                  
   CloseMaskFile(Fptr);

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
               double DistRa = fabs(RaDeg - Ra) - fabs(DeltaRaDeg) * 0.5;
               double DistDec = fabs(DecDeg - Dec) - fabs(DeltaDecDeg) * 0.5;
               if ((DistRa > 0.0) || (DistDec > 0.0)) {
                  DistSq = DistRa * DistRa + DistDec * DistDec;
               }
               bool CheckPixel = (DistSq <= (RadiusDeg * RadiusDeg));
               
               I_Debug.Logf("SkyCheckDist",
                  "Pixel[%d,%d] = %d, Ra,Dec dist = %f,%f will %s",
                  Ix,Iy,Details.DataArray[Iy - 1][Ix - 1],
                  DistRa * DegToAsec, DistDec * DegToAsec,
                  CheckPixel ? "check" : "ignore");
               
               if (CheckPixel) {
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
//  start of the program or the last time this was called.

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
//
//                      M a i n   ( t e s t  p r o g r a m )
//
//  This is a small main test program that can be used to test this code in a
//  standalone way. It is only compiled if the compiler symbol ProfitSkyTest is
//  defined. On my test laptop, this can be used to link it:
//
//      g++ -o ProfitCheck -D ProfitSkyTest -I ../Packages/wcslib-5.16
//        -I ../Packages/wcslib-5.16/C -I ../Packages/Misc/ -std=c++11
//        ProfitSkyCheck.cpp ../Packages/wcslib-5.16/C /libwcs-5.16.a
//        ../Packages/Misc/ArrayManager.o ../Packages/Misc/TcsUtil.o
//        ../Packages/Misc/Wildcard.o ../Packages/cfitsio/libcfitsio.a

#ifdef ProfitSkyTest

int main(int argc, char* argv[])
{
   if (argc < 7) {
      printf ("Usage: directory name, ra, dec, radius, pointra, pointdec\n");
   } else {
      ProfitSkyCheck TheChecker;
      TheChecker.SetDebugLevels("*.files");
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

#endif

// ----------------------------------------------------------------------------------

/*                        P r o g r a m m i n g  N o t e s

   o  The code as originally written ignored the cos(dec) effect that causes
      RA values to get closer together as the declination increases. This is
      mainly an issue with the operation of FileOverlapsField(), which should
      now be fixed.
      
   o  As of Jan 2022, this code has been significantly reworked to allow for the
      use of mask files with Ra,Dec ranges that differ fro the square formats
      of the original Profit mask files.
 
   o  I've realised that throughout this code there is an assumption that the
      first axis of the Profit files (the X-axis) is RA and the second axis
      (the Y-axis) is Dec. If that ever gets changed, someone is going to have
      to look quite carefully at this code in order to sort that out. This is
      left as an exercise to the reader. (Or as a warning not to mess with the
      Profit file format!)
 
   o  (This note was put here early on in the development of this code, and I
      think it explains the thinking behind some of the design.)
      We could ignore the overlap calculated on the basis of the file
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
 
   o  In testing, I hit an embarassing bug in the TCS routine that formats a
      value in degrees into degrees,arcminutes,arcsec and fractions of arcsec.
      In one case a value that was almost exactly 2.0 arcsec was displayed as
      000:00:01.100. I believe the code had formatted a fractional value os
      something like .9998 but had rounded this to 10000 and truncated this to
      three digits, leaving it as 100 for the fractional arcsec part. This
      ought to be fixed! But it may hang around for a while, so just beware of
      this if there seem to be inexplicable arcsec values in the diagnostics.
*/


/*  A snippet of code that used to be in Initialise(), used for testing the
    iterative pixel location.  It may be worth re-instating, but note that
    there needs to be a check to make sure RaDec->XY isn't tested at the
    same time as XY->RaDec, as that will just lead to an infinite loop.
 
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
