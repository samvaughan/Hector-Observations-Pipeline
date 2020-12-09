#ifndef SAMI_BD_INC
#define SAMI_BD_INC
/*                  s a m i _ b u n d l e d a t . h

 * Module name:
      sami_bundledata.h

 * Function:
	Include file that defines the SAMI file reader BundleData class.
      

 * Description:
        This C++ incude file declares the SAMI file reader BundleData class
        and those classes/types/constants needed to implement BundleData

        This class is used to read and access the deatails of a SAMI bundle
        layout file and provide access to it..

        SAMI is an AAT Instrument which implements a number of IFU
        probes on the AAT Prime Focus.
      

 * Language:
      C++

 * Support: Tony Farrell, AAO

 * History:
      24-May-2011 - TJF - Start history.
      29-Jun-2011 - TJF - Files now have measurements in microns rather then mm.


 * "@(#) $Id: ACMM:sami/sami_bundledat.h,v 1.29 23-Feb-2016 12:26:12+11 tjf $"

 */



#include <string.h>
#include <string>
#include <map>
#include <math.h>
#include "sami.h"

namespace SAMI {

    const unsigned MaxFibreRingNumber = 4;  // Max rings in bundle bundle.
    const double   MaxFibreOffset = 1000.0;    // Max offset in microns.
    /*
     * Details on a single fibre.
     */
    struct FibreInfo {
        unsigned _fibreNum;// 1 to FibresPerIFU. 0 indicates not filled in.
        unsigned _ring;    // ring number, 0 to MaxFibreRingNumber.
        double   _x;       // microns from fibre 1.  0 to MaxFibreOffset.
        double   _y;       // microns from fibre 1.  0 to MaxFibreOffset.

        // Returns offset in microns.
        void GetOffset(int *x, int *y) const {
            *x =  int(round(_x));
            *y =  int(round(_y));
        }
    };

    /*
     * Details on an IFU bundle.
     */
    class BundleDataItem {
        
    private:
        FibreInfo _fibres[FibresPerIFU];
        std::string _ID;
    public:
        // Default contructor.  Empty item.
        BundleDataItem()  {
            memset(&_fibres, 0, sizeof(_fibres));
        }
        // Constructor which reads item from a file.
        BundleDataItem(const std::string & ID, const std::string& filename); 

        // Returns offset of specified fibre in microns.
        void GetOffset(unsigned fibre_num, int *x, int *y) const;

    };
    
    // We use a map to releate bundle ID's to the data for the bundle.
    typedef std::map<std::string, BundleDataItem> BundleMapType;
    typedef BundleMapType::iterator BundleMapIter;
    typedef BundleMapType::const_iterator BundlMapIterConst;

    /*
     * Details on all bundles.
     *
     *  This is the intended exteranl interface.  The user
     *  constructs one of these items, specifying the directory
     *  the files exist in and then can access the data.
     */
    class BundleData {
    private:
        // The lookup to the has is the name of the bundle.
        // The value is the data of the bundle.
        BundleMapType _dataMap;
        
    public:
        //
        // "directory" will contain a set of files of the
        // name format "<id>_coord_data.dat" where <id> is
        // to be determined and will be the bundle ID. 
        //
        //  The constructor will read each such file found
        //  in "directory", treating each as a bundle 
        //  configuration file.  
        //
        BundleData(const std::string & directory);

        // Given a given fibre within the specified bundle, return
        //  the fibre offset in microns from the bundle centre.
        void GetOffset(const std::string &bundle,
                    unsigned fibre_num,
                       int *x, int *y) const;
        
    };
}
#endif
