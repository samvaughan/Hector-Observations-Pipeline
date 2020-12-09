#ifndef SAMI_POS_INC
#define SAMI_POS_INC
/*                                  s a m i _ p o s i t i o n s

 * Module name:
      sami_positions.h

 * Function:
	Include file that defines the SAMI file reader.  Thie file
        defines the classes used to hold sky, probe and object positions.
      

 * Description:
        This C++ incude file declares the SAMI file reader and
        releated classes.

        SAMI is an AAT Instrument which implements a number of IFU
        probes on the AAT Prime Focus.
      

 * Language:
      C++

 * Support: Tony Farrell, AAO

 * History:
      19-May-2011 - TJF - Start histroy.
      14-Jun-2011 - TJF - Add stringVector iterators.  
      04-May-2012 - TJF - Add ProbeRotation type and add it to the Probe class.
      25-Sep-2012 - TJF - SkyPosition RAtoString() and DectoString() now public.
      28-Sep-2012 - TJF - Add WriteSimCSV() method.
      23-Feb-2016 - TJF - Undo the change by Nuria made on 21-Mar-2014, which
                           had changed the value of MaxObjectProbes from
                           13 to 16, it is now back at 13. The comment
                           suggested MaxSkyProbes had been increased but
                           as of this version, it was already set back.

 * "@(#) $Id: ACMM:sami/sami_positions.h,v 1.29 23-Feb-2016 12:26:12+11 tjf $"

 */

#include <string>
#include <vector>
#include <iostream>
// Define the SAMI namespace
namespace SAMI {


    class SkyPosition;  // Needed by MeanToAppConverter;
    //
    // Class which maintains information used by the
    //  slalib conversion from Mean to apparent place.
    //    Used by slaMappa(), slaMapqk etc.
    class MeanToAppConverter {


    private:
        double _mjd;       // If non-zero, gmap initialised for this.
        double _equinox;   // gmap initialised for this equnoix.
        double _gmap[21];  // Se slaMappa() for details, that is where 21 comes from.
    public:
        MeanToAppConverter() : _mjd(0), _equinox(0) { }
        void Convert(SkyPosition *pos, double mjd, double equnoix=2000);
    };


    // Class which maintains a Sky position.  Sky positions are read
    //  as mean positions in J2000 coordinates and may have a proper
    //  motion.   They can be converted to an apparent position at a
    //  a specified MJD.  
    class SkyPosition {
        friend class MeanToAppConverter;  // So Convert() can update values.
    private:
        double _MeanRA;   // Mean RA.  Radians, J2000.
        double _MeanDec;  // Mean DEC. Radians, J2000.
        double _PM_RA;    // Proper Motion - RA. arcsec/year since J2000
        double _PM_Dec;   // Proper Motion - Dec. arcsec/year since J2000
        double _AppRA;    // Apparent RA (if _AppMJD is non-zero)
        double _AppDec;   // Apparent Dec (if _AppMJD is non-zero)
        double _AppMJD;   // MJD of apparent place. If zero, has not
                            // been converted
       /*
         * _converter is used to convert mean to apparent.  It is 
         * static since it will retain information between conversions
         * is slow to calculate.  This info is updated only if needed.
         */
        static MeanToAppConverter _converter; 

    public:
            // Basic constructor - default sto all zero but can specify items.
            SkyPosition(double RA=0, double Dec=0, double pRA=0, double pDec=0) :
                _MeanRA(RA), _MeanDec(Dec), _PM_RA(pRA), _PM_Dec(pDec), 
                _AppRA(0), _AppDec(0), _AppMJD(0) {
            }

            // Convert the mean position to apparent at specified MJD.
            void CalcApparent(double mjd);

            // Constructor that has string arguments.
            SkyPosition(int lineNum,
                        const std::string &RA,   const std::string &Dec,
                        const std::string &pmRA="",const std::string &pmDec="");

            
            // Assignment operator, copy constructor allowed for general use.
            // SkyPosition & operator=(const SkyPosition &a);
            // SkyPosition (const SkyPosition& a);


            // Destructor allowed for use by all. Object not virtual.
            ~SkyPosition() {};

            std::ostream & Print(std::ostream &s) const;
            std::ostream & PrintCSV(std::ostream &s, bool print_pms) const;
            std::ostream & PrintFreeFormat(std::ostream &s) const;
            // Return various values
            double GetMeanRA()  const { return _MeanRA; }
            double GetMeanDec() const { return _MeanDec; }
            double GetPMRA()    const { return _PM_RA; }
            double GetPMDec()   const { return _PM_Dec; }
            double GetAppRA()   const { return _AppRA; }
            double GetAppDec()  const { return _AppDec; }
            

            std::string GetMeanRAAsString() const {
                return RAtoString(_MeanRA);
            }
            std::string GetMeanDecAsString() const {
                return DectoString(_MeanDec);
            }
            std::string GetAppRAAsString() const {
                return RAtoString(_AppRA);
            }
            std::string GetAppDecAsString() const {
                return DectoString(_AppDec);
            }

            /*
             * Static members which convert positions to strings.  
             *   separator is the character used to separate each part
             *   of the string, typically a space or colon.
             */
            static std::string RAtoString(double RA, char separator=':');
            static std::string DectoString(double Dec, char separator=':');
 

    };

    // A SAMI probe is of one of three types.  Here we define the types.
    typedef enum { Fiducial='F', ProgramObject='P', 
                   Sky='S', Hole='H', ImagingGuide='G', Undefined='U' } ObjectType;

    // A type which records the rotation of a bundle.
    struct ProbeRotation {
        bool valid;     //  Is this rotation value valid (i.e. has been set)
        double angle;   //  The actual value, only if valid = true.
        ProbeRotation() :  valid(false), angle(0) {};
        void Set(const double a) {
            valid = true;
            angle = a;
        }
    };

    // TODO: The following constants are also defined in samigenfile.C. 
    //   Look into this. (npfl 11-Jan-2013).
    //   TJF Comment - this is simply an include file structure issue.  IF
    //        these various were extracted from here into a separate
    //        include file, it could probably be made to work. (TJF, 23-Feb-16).
    //
    // Define the number of probes of each times  Fiducial = Guide.
#if 0
    const unsigned MaxFiducialProbes = 1;
    const unsigned MaxObjectProbes = 13;
    const unsigned MaxSkyProbes = 26;
    const unsigned MaxImgGuideProbes = 5; // To accommodate the new imaging guide bundles.
#endif

    // These need to be defined correctly yet - but represent the maximum values
    // of X and Y in microns.
    const unsigned MaxXY = 150000;   // 150mm - Field is actually about 125mm

    //  A "Probe" class is maintains deails of a probe allocation.  A probe
    //  has a type and number and an x/y position on the focal plane.
    //
    class Probe {
    private:
        ObjectType _type;          // Probe Type
        unsigned short _num;       // Probe number. Max dependent on type.
        int _x;                    // Probe position in X, microns.
        int _y;                    // Probe position in Y, microns
        ProbeRotation _rotation;   // Probe rotation (may not be valid).
    public:
        // Basic constructor.
        Probe(ObjectType type=Undefined, short num=0, int x=0, int y=0) : 
            _type(type), _num(num), _x(x), _y(y) {}
        // Constructor that interprets string values.    
        Probe(int lineNum,
              ObjectType type, const std::string& num, 
              const std::string &x, const std::string &y);
        // Constructor that interprets string values, but no X/Y provided.  
        Probe(int lineNum,
              ObjectType type, const std::string& num);

        // Output details to the specified stream.
        std::ostream &  Print(std::ostream &s) const;

        // Return type or number.
        ObjectType GetType() const { return _type; }
        unsigned short GetNum() const { return _num; }
        int GetXPos() const { return _x; }
        int GetYPos() const { return _y; }

        void SetXY(int x, int y) {
            _x = x;
            _y = y;
        }
        // Write a CSV file line for this probe, but the
        //  rotation is not written here.
        void WriteCSV(std::ostream &ofile) const;

        void WriteRotationCSV(std::ostream &ofile) const;

        void SetRotation(const double r) {
            _rotation.Set(r);
        }
        ProbeRotation GetRotation() const {
            return _rotation;
        }


        ~Probe() {}
    };


    // We have need for stringVectors in some places.
    typedef std::vector<std::string> stringVector;
    typedef stringVector::iterator stringVectorIt;
    typedef stringVector::const_iterator stringVectorConstIt;

    // An object has a priority - floating point, but no greater then this.
    const int MaxPriority = 9;

    // An object structure is used to represent details of objects
    // read from a file. 
    //
    class Object {
    private:
        SkyPosition _SkyPosition; //Object positon on sky;
        std::string _Name;        // Object Name.
        std::string _Comment;     // Object command.
        ObjectType  _Type;        // Object Type.
        double      _Priority;    // Object priority.  0 to 9.
        double      _Magnitude;   // Magnitude of object.
        double      _wlen;        // Object wavelength.
        Probe       _Probe;       // Probe details for object.
        unsigned    _LineNum;     // Line in file object was defined on.
      
        void ParseProbe(const stringVector & lineVec,  int lineNum, 
                        bool allowProbesWithoutPos);

    public:
        // Default constructor.
        Object(ObjectType t = Undefined) :
            _Type(t), _Priority(0), _Magnitude(0), _wlen(0), _LineNum(0)
        {
        }
        // Constructor which processes a line from a data file.
        //   lineVec must have at least 7 entries.
        //   lineNum is just used for reports.
        //   If allowProbesWithoutPos is set true, then when parsing probe details,
        //     we allow the probe to be specified without a position, which is appropiate
        //     the samigencfg program, but not ftf_sami.
        Object(const stringVector & lineVec, int lineNum, bool allowProbesWithoutPos);
            
        // Return the probe assoicated with an object.
        const Probe & GetProbe() const {
            return _Probe;
        }
        // Return the line number of the fil ethe object was defined on.
        int GetLineNum() const {
            return _LineNum;
        }
        // Return the name of the object
        const std::string &GetName() const {
            return _Name;
        }
        const SkyPosition &GetSkyPos() const {
            return _SkyPosition;
        }

        // Return type or number.
        ObjectType GetType() const { return _Type; }

        double GetMagnitude() const {
            return _Magnitude;
        } 
        double GetWlen() const {
            return _wlen;
        }

        const std::string &GetComment() const {
            return _Comment;
        }
        // Convert the mean position to apparent at specified MJD.
        void CalcApparent(double mjd) {
            _SkyPosition.CalcApparent(mjd);
        }

        // Set the position of an object's probe (should be X/Y for RA/Dec).
        void SetXY(int X, int Y) {
            _Probe.SetXY(X, Y);
        }

        // Write a CSV file line for this object.
        void WriteCSV(std::ostream &ofile) const;
        // Write a simple CSV file line for this object (Aladin style format).
        void WriteSimCSV(std::ostream &ofile) const;
        // Write a dummy sky row to a CSV file
        static void WriteDummySkyCSV(std::ostream &ofile, int index, int probeNum);
        // Write a line for this object in free format (e.g. 02 23 45.1 -50 43 22.2)
        void WriteFreeFormat(std::ostream &ofile) const;

        // Assignment operator, copy constructor allowed for general use.
        // Object & operator=(const Object &a);
        // Object (const Object& a);
        // Destructor allowed for use by all. Object not virtual.
        ~Object() {};

        // Print object details;
        void Print() const;
        void PrintSim() const;
        
    };
    // Define types to represetent a vector of objects.
    typedef std::vector<Object> ObjectVector;
    typedef ObjectVector::iterator ObjectVectorIt;
    typedef ObjectVector::const_iterator ObjectVectorConstIt;



} // namespace SAMI

// Declare operators to output SkyPositions and Probe details to an ostream.
std::ostream & operator << (std::ostream &s, const SAMI::SkyPosition &p);
std::ostream & operator << (std::ostream &s, const SAMI::Probe &p);


#endif
