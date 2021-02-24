//
//                       H e c t o r  S k y  C h e c k . h
//
//  Function:
//     A small utility class that checks sky positions for contamination for Hector.
//
//  Description:
//     This defines a C++ class, HectorSkyCheck, that exists to answer the simple
//     question, "can this given Ra,Dec position be used for a sky fibre?" that is,
//     is the specified position clear of any objects - stars, galaxies, etc?. This
//     is actually just a very simple interface to the AAO ConeOfDarkness code,
//     originally written for SAMI. ConeOfDarkness has a Catalogue class that
//     provides all the necessary functionality, and this works by sending TAP
//     requests to an on-line catalogue (in this case superCOSMOS). The Catalogue
//     class needs to be initialised with a central postion Ra,Dec and a radius
//     in degrees - this allows it to make one single request for all objects in
//     that specified part of the sky, which is much more efficient than making
//     individual requests for each possible sky position. (It is also able to
//     cache the results of such an enquiry locally on disk, meansing re-running
//     for the same field becomes much faster.)
//
//     The HectorSkyCheck interface reflects the underlying workings of the
//     Catalogue class. It has an initialisation method, which needs to be
//     passed a central Ra,Dec position and radius, and a querry method that
//     takes the Ra,Dec of a possible sky fibre, together with a radius for the
//     search, and returns a simple boolean OK/not OK result. If there is anythin
//     at all in the catalogue within that radius of that position, this returns
//     false. If it appears clear, it returns true.
//
//  Threads:
//     The class is thread-safe, but with one slightly unusual requirement. Any
//     instance of the class must be created in the main thread, while that is
//     still the only thread running. That is because the constructor calls a
//     routine that initialises the Curl subsystem, and this needs to be called
//     from the only thread running, as what it does is not thread-safe. Similarly,
//     the destructor must also be called from the main thread when no other
//     is running. See the man page for libcurl(3) for details.
//
//  Remaining issues: None of this has been tested.
//
//  Author(s): Keith Shortridge, K&V  (Keith@KnaveAndVarlet.com.au)
//
//  History:
//      5th Jun 2019.  Original version.
//      9th Jul 2019.  Added the ifdef..endef block for __HectorSkyCheck__. KS.

// ----------------------------------------------------------------------------------

#ifndef __HectorSkyCheck__
#define __HectorSkyCheck__

#include <string>

#include "cod.h"

class HectorSkyCheck {
public:
   //  Constructor
   HectorSkyCheck (void);
   //  Destructor
   ~HectorSkyCheck ();
   //  Initialisation
   bool Initialise (double CentralRa, double CentralDec, double RadiusInDegrees);
   //  Querry a potential sky position - Clear returns result.
   bool CheckUseForSky (double Ra, double Dec, double RadiusInDegrees, bool* Clear);
   //  Get description of latest error
   std::string GetError (void) { return I_ErrorText; }
private:
   //  Description of last error, if any.
   std::string I_ErrorText;
   //  The Catalogue object that does most of the work.
   COD::Catalogue* I_Catalogue;
};

#endif

// ----------------------------------------------------------------------------------

/*                        P r o g r a m m i n g  N o t e s

   o  The constructor for the Catalogue does quite a lot of work, and needs the
      central RA,Dec and radius, so we can't declare it as an instance variable.
      Instead, we have a pointer to it as an instance variable and create it in
      Initialise(). The CoD code does declare a constructor with no arguments,
      but at the time of writing, it doesn't seem to do anything.
 
   o  I'd considered a diagnostic call that would supply the address of the
      Catalogue being used, but at the moment there don't seem to be many
      low-level Catalogue inquiry routines (something like 'print status') that
      this would provide access to. It may be useful later.
*/
