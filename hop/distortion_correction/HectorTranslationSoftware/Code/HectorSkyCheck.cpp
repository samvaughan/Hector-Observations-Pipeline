//
//                       H e c t o r  S k y  C h e c k . c p p
//
//  Function:
//     A small utility class that checks sky positions for contamination for Hector.
//
//  Description:
//     This defines a C++ class, HectorSkyCheck, that exists to answer the simple
//     question, "can this given Ra,Dec position be used for a sky fibre?" that is,
//     is the specified position clear of any objects - stars, galaxies, etc?. This
//     is actually just a very simple interface to the AAO ConeOfDarkness code,
//     originally written for SAMI.
//
//     For more details, see the comments in HectorSkyCheck.h
//
//  Remaining issues: None of this has been tested.
//
//  Author(s): Keith Shortridge, K&V  (Keith@KnaveAndVarlet.com.au)
//
//  History:
//      5th Jun 2019.  Original version.
// ----------------------------------------------------------------------------------

#include "HectorSkyCheck.h"

// ----------------------------------------------------------------------------------
//
//                            C o n s t r u c t o r
//
//  This is the only constructor. I don't like initialising things in constructors
//  as it makes it harder to report errors. So all this does is initialise the
//  various instance variables, and initialise the curl sub-system.
//
//  Note that this has to be called from the main thread, with no other thread
//  running. See under 'threads' in the comments in HectorSkyCheck.h.

HectorSkyCheck::HectorSkyCheck (void)
{
   I_ErrorText = "";
   I_Catalogue = NULL;
   
   //  The Catalogue method initialise() is a static routine that initialises
   //  curl.
   
   COD::Catalogue.initialise();

}

// ----------------------------------------------------------------------------------
//
//                             D e s t r u c t o r
//
//  The destructor releases any resources used by the code.

HectorSkyCheck::~HectorSkyCheck ()
{
   //  Release the Catalogue.
   
   if (I_Catalogue) delete I_Catalogue;
   I_Catalogue = NULL;
   
   //  The Catalogue method cleanup() is a static routine that closes down curl.
   
   COD::Catalogue.initialise();

}

// ----------------------------------------------------------------------------------
//
//                             I n i t i a l i s e
//
//  Initialises the Catalogue object that does all the owrk, creating it and
//  passing it the details of the area of sky we're interested in. The Catalogue
//  will see if the required data is already cached locally, and if not will
//  get the required data from the on-line catalogue it uses. This may take
//  a little while.

bool HectorSkyCheck::Initialise (
   double CentralRa, double CentralDec, double RadiusInDegrees)
{
   bool ReturnOK = false;
   
   //  Check we're not already initialised. I suppose we could delete the
   //  Catalogue and create a new one, but I suspect this really indicates
   //  an error.
   
   if (I_Catalogue) {
      I_ErrorText = "The HectorSkyCheck object has already been initialised";
   } else {
   
      //  Create the new Catalogue. This code checks for an error, but at the
      //  moment it seems that the constructor simply exits the program if it
      //  hits a problem. (This is why I don't like doing things in constructors.)
      
      I_Catalogue = new COD::Catalogue(CentralRa,CentralDec,RadiusInDegrees);
      if (I_Catalogue == NULL) {
         I_ErrorText = "Unable to create new Catalogue object."
      } else {
      
         //  If it created OK, that's all we have to do. Now we can start to
         //  use it.
         
         ReturnOK = true;
      }
   }
   return OK;
}

// ----------------------------------------------------------------------------------
//
//                        C h e c k  U s e  F o r  S k y
//
//  Checks whether the specified area (centered on Ra,Dec, with a radius of
//  RadiusInDegrees) is clear of any catalogue objects. If it is clear, this
//  sets Clear to true. Otherwise, it sets Clear to false. Note that the bool
//  returned as the function value is used to indicate success or failure (this
//  will fail if the Catalogue was not initialised, for example), and not whether
//  or not the area is clear.

bool HectorSkyCheck::CheckUseForSky (
   double Ra, double Dec, double RadiusInDegrees)
{
   bool ReturnOK = false;
   
   //  Check we're initialised.
   
   if (I_Catalogue == NULL) {
      I_ErrorText = "The HectorSkyCheck object has not been initialised properly.";
   } else {
   
      //  Get the list of objects in the catalogue in the specified area. If we
      //  find any, then it isn't clear. Simple as that.
      
      std::vector<CatObject> ObjectList = I_Catalogue->objectsInField(
                                                  Ra,Dec,RadiusInDegrees);
      *Clear = (ObjectList.size() > 0);
      ReturnOK = true;
   }
}

// ----------------------------------------------------------------------------------

/*                        P r o g r a m m i n g  N o t e s

   o  At the moment, the CoD Catalogue code seems to report errors via messages,
      rather than returning error indications to the caller or taking exceptions.
      Which is probably OK for the way this is expected to be used for Hector.
      It does mean this code doesn't have to do much error handling.
*/


