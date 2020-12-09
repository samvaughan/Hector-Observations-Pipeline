#ifndef SAMI_FC_INC
#define SAMI_FC_INC
/*                                  s a m i _ f i l e c o n t e n t s

 * Module name:
      sami.h

 * Function:
	Include file that defines the SAMI file reader FileContents class.
      

 * Description:
        This C++ incude file declares the SAMI file reader FileContents class.
        This class is used to read and access the deatails of a SAMI configuration
        file.

        SAMI is an AAT Instrument which implements a number of IFU
        probes on the AAT Prime Focus.
      

 * Language:
      C++

 * Support: Tony Farrell, AAO

 * History:
      03-May-2011 - TJF - Start history.
      06-Jul-2011 - TJF - Add non-const version of ForEachObject.
      28-Sep-2012 - TJF - Add WriteBodySim() method.


 * "@(#) $Id: ACMM:sami/sami_filecontents.h,v 1.29 23-Feb-2016 12:26:12+11 tjf $"

 */

#include <string>
#include <iostream>
#include <fstream>

#include "sami_positions.h"   // Declares SkyPosition, Object and Probe classes
// Define the SAMI namespace
namespace SAMI {

    // Maximum number of objects we will read from the file.
    const int MaxObjects=1000;

    // Needed for the following typedef to work.
    class FileContents;
    /*
     * This typedef is used by the ParseSection method.
     *   Implementors return "true" if they are expecting more data, false
     *   otherwise.  They will throw an exception on error, which should
     *   be a subclass of SAMI::_S_Exception;
     */
    typedef bool (FileContents::*lineParser)(
        const stringVector & lineVec,
        int lineNum);

    /*
     * This structure is used in parsing the header of the file - enables us
     * to be order insenstive (but the optional equinox must be before something
     * else in the header.
     */
    struct HeaderParseState {
        bool HaveLabel;     // Bunch of logical flags indicating if we have seen
        bool HavePlate;    
        bool HaveDate;      //   the item in question.
        bool HaveTime;
        bool HavePosition;
        bool HaveEquinox;
        bool HaveWlen;
        bool error;
        std::string utdate; // Must hold date and time until we have both.
        std::string uttime;
        

        HeaderParseState() :
            HaveLabel(false), HavePlate(false), HaveDate(false), HaveTime(false),
            HavePosition(false), HaveEquinox(false), HaveWlen(false),
            error(false){}

        bool IsComplete() const {
            // It is not compuslory to have equinox, in fact, we ignore the
            // equinox for the moment.
            return (HaveLabel&&HaveDate&&HaveTime&&HavePosition&&
                    HaveWlen&&HavePlate);
        }

           
    };


    // Class which describes the contents of a SAMI file and allows us to access
    // that data.
    class FileContents  {
    private:
        SkyPosition _centrePos;    // Centre position of the configuration.
        std::string _label;        // Field label.
        std::string _fileName;     // Name of the file which was read.
        std::string _plateID;      // Plate ID (E.g. when physical plate was built - TBD).
        double      _wlen;         // Observation wavelength in angstroms
        double      _obsTime;      // Observation time as an MJD.
        bool        _allowProbesWithoutPos;// When parsing object lines, allow probes
                                   // to be defined without a position.
        HeaderParseState _headerState; // State of header parsing.

        ObjectVector _objects; // Vector of objects read from file, which make up the SAMI
                               // configuration.


        // An object may (or may not) be allocated to a SAMI probe.  There are three
        // probe type  - the (original) Guider, the Imaging Guide bundles,
        // the Object (Hexabundle IFU) probes and the Sky probes (Single fibres).

        // These arrays maintain references to loaded objects for each probe type.
        //  If the reference is the null pointer, probe is not allocated
        //  These references are set up by GenerateProbePointers(), which must
        //  be invoked at the end of any constructor or assignment operator.
        const Object *_guideProbes[MaxFiducialProbes];
        const Object *_objectProbes[MaxObjectProbes];
        const Object *_skyProbes[MaxSkyProbes];
        const Object *_imgGuideProbes[MaxImgGuideProbes];


        // Assignment operator, copy constructor made private to avoid misuse.
        //   Note - if these are every to be used - then 
        //     they must use "GenerateProbePointers()" after coping all the
        //     all contents. 
        FileContents & operator=(void *a);
        FileContents (const FileContents& a);


        // This method is used to parse a section of the file, where a 
        // section is either the Header or the Body.  The lineParser is
        // specific to the section.
        void ParseSection(std::istream& in, int *lineNum, lineParser lp);
                          

        // Methods used in parser header or body lines. Are specified when
        //   ParseSection (above) is inovked.
        bool ParseHeaderLine(const stringVector & lineVec, int lineNum);
        
        bool ParseBodyLine(const stringVector &lineVec, int lineNum);
            

        // This method fills out the arrays of references to probe objects
        void GenerateProbePointers();

        // These are used to help implement GenerateProbePointers()
        void SetGuideProbePointer(const Object *p, unsigned short n);
        void SetImgGuideProbePointer(const Object *p, unsigned short n);
        void SetSkyProbePointer(const Object *p, unsigned short n);
        void SetObjectProbePointer(const Object *p, unsigned short n);
        
    public:

        // Main constructor - create a SAMI::FileContents object and read
        //  details from a file.
        FileContents(const char *fileName, bool allowProbesWithoutPos);


        // Secondary constructor.  Just does the header from an already open
        //  stream.
        FileContents(std::istream &in, int *lineNum);

        // Destructor allowed for use by all. Object not virtual.
        ~FileContents() {};

        // Return the field center position
        const SkyPosition & GetCentre() const { return _centrePos; }
        const SkyPosition & GetCenter() const { return _centrePos; }

        // Return MJD of observation time.
        double GetObsTime() const { return _obsTime; }

        // Return the label, plateID and filename.
        const std::string & GetFilename() const { return _fileName; }
        const std::string & GetLabel() const { return _label; }
        const std::string & GetPlateID() const { return _plateID; }
        double GetWavelength() const { return _wlen; }

        // Return details of a specified object given a probe reference.
        //   Returns dummy object on failure.
        const Object & GetObject(ObjectType type, unsigned probe) const;

        // Return details of a specified object given a probe reference.
        //   Throws exception on failure.
        const Object & GetObjectE(ObjectType type, unsigned probe) const;



        // Return the offset of object assoicated with a probe in x/y microns.
        void GetObjectOffsetXY(ObjectType type, unsigned probe, 
                               int *x, int *y) const;

        /*
         * Apply a particular operation to each element in the object vector.
	 *  const and non-const versions.
         */
        template <class Operation> void ForEachObject(Operation &Op) 
        {
            for (register unsigned i = 0 ; i < _objects.size() ; ++i)
            {
                Op(&_objects[i]);
            }
  
        }
        template <class Operation> void ForEachObject(Operation &Op)  const
        {
            for (register unsigned i = 0 ; i < _objects.size() ; ++i)
            {
                Op(&_objects[i]);
            }
  
        }

        // Is a probe in use?  (given a probe reference)

        // Is a probe in use?  (given a probe reference)
        bool IsProbeInUse(ObjectType type, unsigned probe) const;

        // Output details to std::cout.
        void Print() const;


        void WriteHeader(std::ostream &ofile) const;
        void WriteBody(std::ostream &ofile, bool addDummies) const;
        void WriteBodySim(std::ostream &ofile) const;
        void WriteBodyFree(std::ostream &ofile, std::ostream &guideStream, std::ostream &skyStream) const;
    };


} // namespace SAMI



#endif
