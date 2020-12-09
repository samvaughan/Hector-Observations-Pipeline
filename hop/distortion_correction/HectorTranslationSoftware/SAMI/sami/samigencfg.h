/*+          samigencfg.h
 *
 * Module name:
 *     samigencfg.h
 *
 * Function:
 *     
 *
 * Language:
 *     C++
 *
 * Support:
 *     Nuria P.F. Lorente, AAO
 *
 * History:
 *     Created on: 31/05/2014 (nlorente)
 *     See ChangeLog
 *
 * ACMM Id:
 *     "@(#) $Id: ACMM:sami/samigencfg.h,v 1.29 23-Feb-2016 12:26:11+11 tjf $"
 *
 */
#ifndef SAMIGENCFG_H_
#define SAMIGENCFG_H_

// I don't think we need the C include. If there are problems because
// this has been removed replace it with #include <cstring>
//#include <string.h>
#include <stdlib.h>
#include <vector>
#include <string>
#include <stdio.h>
#include "sami.h"
#include "sami_util.h"
#include "aatTelMdl.h"
#include <fstream>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <errno.h>
//#include <math.h>
#include <cmath>
#include <time.h>
#include <assert.h>
#include <map>
#include <stdexcept> // For std::out_of_range
#include "tinyxml2.h"


/*
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *  Various constants used in this program.
 */

const double TEMP_DEFAULT=15;     /* Default Temperature, Celsius */
const double PRES_DEFAULT=850;    /* Default Pressure, millibars */
const double HUMID_DEFAULT=0.5;   /* Default Humidity,  0 to 1 */

const double TEMP_DRILLING=20.0;  /* Temperature of plate fabrication */
                                  /* Used to calculate thermal expansion of the plate */

/* Email from  Michael Goodwin on 27/June/2011. IFU and SKU use the same
   connector, hence probe separation should be 20mm.  Clearance should
   hole buffer is 1.5mm (hole to hole).  Guide probe hole is
   10mm, but allow allow 15 diameter to other holes

   So, IFU/SKU probe sep of 20mm, allowing for buffer of 1.5, means
    the radius of the probes is (20-0.75)/2 = 9.25

   Email from Michael Goodwin on 14/June/2012.  Drop buffer to 19mm.
      I implemented this by dropping BUFFER to 500, but am not
      sure if that is the right approach.


   Email from Michael Goodwin on 28/Aug/2012.

   - central plate exclusion zone for bundles, dia=60mm, due to camera lens,
        sky fibres are OK in this zone.
   - min separation between connectors within a field, sep=20mm
   - min separation between holes of separate fields, sep=13mm
        (the hole diameters are about 7.3mm).

        This triggered a significant revamp with correct implementation
        of collision detection required.

   Email from Michael Goodwin on 29/Aug/2012.
       I have re-checked the plates. I was using 12mm not 13mm for the hole
       separation between separate fields!   Note small discrepancies are acceptable,
       e.g. 500 microns, or 11.5mm, due to different distortion models and
       atmospheric corrections that you are using. Note the holes are 7.29 mm
       (I think) so 12mm should be OK.

       With the sky objects, I ensured a clear window within a radius of 10", so
       this about 650 microns, hence 470 microns should be OK.

*/

/*
 * The following are the connector footprint radii - double-check!
 */
const double IFU_PROBE_RADIUS=6250.0; /* IFU Probe size, Microns */
                                      /* Was 9250 until Run 8 */
const double SKY_PROBE_RADIUS=5000.0; /* Sky Probe size, Microns */
                                      /* Was 9250 until Run 8 */
const double GUI_PROBE_RADIUS=6250;  /* Guider Probe size, Microns */
const double SIM_HOLE_RADIUS =4035;   /* Simple hole for calibration */

/*
 * This is the size of the holes required in the plate.
 */
const double IFU_HOLE_DIAMETER=4.230;    /* IFU drill hole size, mm */
                                         /* Changed from 4.22 for AAT plate fabrication in Run 20 (npfl 15-Dec-2014) */
                                         /* Changed from 7.29 for the redesigned connectors in Run 8 */
const double SKY_HOLE_DIAMETER=4.230;    /* Sky drill hole size, mm */
                                        /* Changed from 4.22 for AAT plate fabrication in Run 20 (npfl 15-Dec-2014) */
                                        /* Changed from 7.29 for the redesigned connectors in Run 8 */
const double GUI_HOLE_DIAMETER=10.0;    /* Guider hole size, mm    */
const double SIM_HOLE_DIAMETER=5.00;    /* Simple drill hole size, mm */
const double KEY_HOLE_DIAMETER=3.00;    /* IFU keyhole size, mm */

//const std::string IFU_HOLE_DIAM_DRILL_TOLERANCE="+0.02/-0.00 mm"; /* IFU drill tolerance, mm. Runs 8 and 9*/
//const std::string IFU_HOLE_DIAM_DRILL_TOLERANCE="4.220 - 4.224 mm"; /* IFU drill tolerance, mm. Run 10 */
const std::string IFU_HOLE_DIAM_DRILL_TOLERANCE="4.225 - 4.235 mm"; /* IFU drill tolerance, mm. Run 20 */
const std::string KEY_HOLE_DIAM_DRILL_TOLERANCE="+0.05/-0.00 mm"; /* Key drill tolerance, mm */

const double IFU_CONNECTOR_SEPARATION=15.0;  /* Min separation between connectors on a field, mm */
const double IFU_HOLE_SEPARATION=15.0;       /* Min separation between on separate fields, mm */
const double SKY_CONNECTOR_SEPARATION=15.0;  /* Min separation between connectors in a field, mm */
const double SKY_HOLE_SEPARATION=12.5;       /* Min separation between holes in separate fields, mm */
const double CENTRAL_EXCLUSION=20.0;         /* connectors can't be this close to the centre, mm */
const double KEYHOLE_SEPARATION=5.875; /* Separation between the centres of the IFU hole and its keyhole, mm */
                                       /* Introduced for Run 8 */

const double SKYS_ARE_SAME_DIST = 600;  /* Skys on different plates less then this far apart are considered the same */

/* TODO: What is the reason for the 65 hole limit? */
const unsigned MAX_DRILL_HOLES=65; /* The initial plates can have this many drill holes*/

const unsigned SKY_MERGE_DIST=500; /* If two sky holes are this close, merge them for
                                      the drilling - this is a problem,
                                      .allocated.csv will not have the correct
                                      position */

const double AIRMASS_LIMIT_DEFAULT=2.0; /* Default air-mass limit */

const double distortionCentreX = 0;  /* Offset of plate centre from distortion centre */
const double distortionCentreY = 0;

/* Coefficient of thermal expansion for the SAMI plates
 * (Steel: 11-13e-6 /degC)
 */
const double THERMALEXPANSIONCOEFF = 12.0e-6; /* (using the same value as on Stan's spreadsheet) */

/* Difference between drilling and observing (nominal) temperature -
 * e.g. deltaDegrees = 5.0 assumes that the plate is drilled at
 * 20decC (given a nominal temperature of 15degC).
 */
const double DELTADEGREES = TEMP_DRILLING - TEMP_DEFAULT;

const double LOCATOR_HOLE_DIAMETER = 3.0;  /* mm */
const double LOCATOR_HOLE_1_X = 129.5000;  /* mm */
const double LOCATOR_HOLE_1_Y = 0.0;
const double LOCATOR_HOLE_2_X = -125.0874; /* mm */
const double LOCATOR_HOLE_2_Y = -33.5171;  /* mm */

/*
 * Minimum separation between a locator hole and any other object (hole or label)
 */
const double LOCATOR_HOLE_SEPARATION = 10.0; /* mm */

/*
 * Width and height of box which defines the space
 * for the engraved labels, in mm
 *
 * From 9-Jul-2014 email from Carl Holmesby:
 *   The lettering height is set at 3.5mm by variable #503
 *   Letter width is at about 56% set by #500
 *   The actual measured height engraved 0.2mm deep measures around 3.8mm.
 *   4 characters is less than 11mm wide, measured.
 */
const double HOLE_LABEL_HEIGHT_MM = 5.0; /* mm */
// Originally used 13.0
// Changed to 4*(w+s)=15.8 ~16.0
const double HOLE_LABEL_WIDTH_MM = 16.0; /* mm */
// Originally used 3.5mm
const double LABEL_CHAR_HEIGHT_MM = 4.0; /* mm */
// Originally used LABEL_CHAR_WIDTH_MM = LABEL_CHAR_HEIGHT_MM / 1.782 = 1.964mm (for a 3.5mm height)
// Changed to 2.95 (14-Jul-2014)
const double LABEL_CHAR_WIDTH_MM = 2.95; /* mm */
// Inter-character space in the engraved label
// Was 1.0mm for first attempt (see 14-Jul-2014 email from Carl, with photos of the engraving)
const double LABEL_CHAR_SPACE_MM = 1.0; /* mm */

/*
 * Space between the end of the exclusion zone and the box
 * which defines the label space.
 *
 * Originally 1.5mm - labels too far from holes
 *
 * 17-Jul-2015: Was 1.0mm. This produced a label that was 2.5mm from the edge of the connector.
 *              After discussion with Carl Holmesby we agreed that this could be reduced by 1.0mm
 *              Setting it to 0.1mm rather than 0 for paranoia.
 */
const double HOLE_LABEL_SEPARATION_MM = 0.1; /* mm */

const double HOLE_LABEL_HEIGHT_LGE_MM = 6.0; /* mm */
const double HOLE_LABEL_WIDTH_LGE_MM = 25.0; /* mm */

/*
 * Parameters required by the CNC machine to drill the plates
 */
// The engraving depth for the plate labels.
// This must always be a negative number (because it's removing material)
// Originally -0.2mm.
// Changed to -0.08mm for SAMI Run 23 (30-Mar-2015)
const double ENGRAVING_DEPTH = -0.08; /* mm */

/*
 *  Use the SAMI version number and date.
 */
extern const char *SamiVersion;
extern const char *SamiDate;

// Global variable holding the full date/time string which is
// engraved on the plate
std::string plate_date_str;

/*
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Internally used classes and type declarations.
 */
enum class CentralHole{
    NO_CENTRAL_HOLE,
    CENTRAL_HOLE,
    KEYED_HOLE
};

// String to CentralHole utility function

class UserParameters {
private:
    UserParameters(){};
public:
//    static int RUN_ID;
    // Default observing run:
    // 8 - February 2013
    // 9 - March 2013
    // 10 - April 2013
    static int RUN_ID;

    // Default plate number within a run
    static int PLATE_ID;

    // Plate type - S=star G=galaxy
    static std::string PLATE_TYPE;

    // Assumed dome temperature for the observations
    static double DOME_TEMPERATURE_DEGC;

    // Assumed drilling temperature for plate manufacture
    // This is used to calculate the thermal expansion of the plate.
    static double DRILLING_TEMPERATURE_DEGC;

    // Pressure (millibars)
    static double ATMOSPRESSURE_MBAR;

    // Humidity (0-1)
    static double ATMOSHUMIDITY;

    // Airmass limit
    static double AIRMASS_LIMIT;

    // Input data file (.dat from Michael Goodwin)
    static std::string INPUT_DATA_FILE;

    // The plate's archive name (e.g. Y13SAR1_P015_12T014_15T023)
    static std::string PLATE_ARCHIVE_NAME;

    // Do each field have their own sky positions, or is there one set for all fields on a plate?
//    static bool FIELDSPECIFIC_SKY_POSITIONS;
//    static final std::string FIELDSPECIFIC_SKY_POSITIONS_KEY = "FIELDSPECIFIC_SKY_POSITIONS";

    // Automatically find sky positions, or are they provided in the data file?
    static bool FIND_SKY_POSITIONS;

    // Number of sky positions required
    static int SKY_POSITIONS_REQUIRED;

    // Are the sky positions global or specific to each field?
    // This option is only available when the sky positions are provided in the
    // input data file, and so should only be set with FIND_SKY_POSITIONS=false
    static bool FIELD_SKY_POSITIONS;

    // Display the results using Aladin?
    static bool RUN_VISUALISER;

    // Default field number within a plate, for use by the visualiser
    static int FIELD_ID;

    // Coefficient of thermal expansion for the SAMI plates
    // (Steel: 11-13e-6 /degC)
    // Default is value used on Stan's spreadsheet
    static double THERMAL_EXPANSION_COEFF;

    // Plate field of view
    static double PLATE_FOV_ARCSEC;

    // Plate scale
    static double PLATE_SCALE_ARCSECPERMM;

    // The margin required between the edge of the plate and any label
    static double PLATE_EDGE_MARGIN_MM;

    // Plate size - calculated from the FoV and the plate scale
//    static double PLATE_RADIUS_MICRONS = MMtoMicrons(UserParameters::PLATE_FOV_ARCSEC / UserParameters::PLATE_SCALE_ARCSECPERMM / 2.0);
    // Plate size - measured (Run7G3)
    static double PLATE_RADIUS_MM;


    // Central guide hole diameter
    static double GUIDE_HOLE_DIAMETER_MM;

    // Central guide hole exclusion zone
    static double GUIDE_EXCLUSION_RADIUS_MM;

    // Hexabundle total active area: 980 microns
    static double BUNDLE_DIAMETER_EFF_MM;

    // Hexabundle hole diameter.
    // Pre-run 8 value = 5.0mm
    // Pre-run 20 value = 4.220mm
    // Connector outer diameter = 12.5mm (pre-run 8 value = 12mm)
    //
    // Exclusions:
    // There shall not be any holes within the connectors' footprint.
    // There shall not be any engraving within the connector footprint.
    static double TARGET_HOLE_DIAMETER_MM;

    // IFU connector size, microns
    static double TARGET_PROBE_RADIUS_MICRONS;

    // Hexabundle key hole diameter
    // Did not exist before run 8
    static double KEY_HOLE_DIAMETER_MM;

    // Distance of the key hole from the centre of the hexabundle hole.
    // The position of the key hole is compass South with respect to the hexabundle hole.
    // This puts it above the hexabundle hole when looking down on the plate, and on the diagnostic plots.
    static double KEY_HOLE_POSITION_MM;

    // TODO: Check that the following is still valid
    static double TARGET_HOLE_EXCLUSION_DIAMETER_SAMEFIELD_MM;

    static double TARGET_HOLE_EXCLUSION_DIAMETER_DIFFFIELD_MM;

    static double SKY_FIBRE_DIAMETER_MM;

    static double SKY_PROBE_RADIUS_MICRONS;

    // Sky fibre hole diameter
    // Connector outer diameter = 10mm
    //
    // Exclusions:
    // There shall not be any holes within the connectors' footprint.
    // There shall not be any engraving within the connector footprint.
    static double SKY_HOLE_DIAMETER_MM;

    static double SKY_HOLE_EXCLUSION_DIAMETER_SAMEFIELD_MM;
    static double SKY_HOLE_EXCLUSION_DIAMETER_DIFFFIELD_MM;

    static double DELTADEGREES;
    // Sort this out!
//    static double CALIBRATION_HOLE_DIAMETER_mm = TARGET_HOLE_DIAMETER_MM;

    // Is the guide hole in the centre of the plate a single hole (original)
    // or a keyed bundle hole (new)?
//    static bool KEYED_CENTRAL_GUIDE_HOLE;
    // The central guide hole can either be a single hole (this was the original
    // and only option when the guide camera was centrally mounted), a keyed hole,
    // or no central hole at all.
    static CentralHole CENTRAL_GUIDE_HOLE;

    // Download thumbnail images for each target, sky and guide position
    // from SuperCOSMOS.
    static bool GET_THUMBNAILS;
    static float THUMBNAIL_SIZE;
    static int THUMBNAIL_HORI;
    static int THUMBNAIL_VERT;

    static void initialise(std::string filename);
};

/*
 * A KeyHole gives the position of a drill hole's key hole.
 * A keyhole cannot exist independently of its drill hole.
 */
class KeyHole {
private:
    int _x;
    int _y;

public:
    KeyHole() : _x(0), _y(0){}
    KeyHole(int x, int y) : _x(x), _y(y) {};
    double GetHoleSize() const;
    int GetPosX() const;
    int GetPosY() const;
};


// Forward declaration so that we can use the DrillHoleVector as
// a parameter to static method getNamedHole()
class DrillHole;
typedef std::vector<DrillHole> DrillHoleVector; // Vector of config files




class LabelBoundingBox {
    // The height of the bounding box is set in HOLE_LABEL_HEIGHT
public:
    // Lower left coordinates
    double _xll; // Microns
    double _yll; // Microns
    // Lower right coordinates
    double _xlr; // Microns
    double _ylr; // Microns
    // Upper left coordinates
    double _xul; // Microns
    double _yul; // Microns
    // Upper right coordinates
    double _xur; // Microns
    double _yur; // Microns

private:
    // Returns the area of the triangle formed by vertices (x1, y1), (x2, y2), (x3, y3)
    double triangleArea(double x1, double y1, double x2, double y2, double x3, double y3);
    bool pointIsInsideBB(double px, double py, LabelBoundingBox bb, double bbArea);

public:
    LabelBoundingBox();
    LabelBoundingBox(double xll, double yll, double xlr, double ylr, double xul, double yul, double xur, double yur);
    LabelBoundingBox(const LabelBoundingBox& bb);
    void set(double xll, double yll, double xlr, double ylr, double xul, double yul, double xur, double yur);
    // Set the bounding box south of the hole
    void setSouth(double holeX, double holeY, double minSeparationRadiusMM, double labelHeight, double labelWidth);
    // Set the bounding box west of the hole (left, looking at the engraved side of the plate)
    void setWest(double holeX, double holeY, double minSeparationRadiusMM, double labelHeight, double labelWidth);
    // Sets the bounding box in portrait mode - i.e. doesn't rotate it.
    void setWestPortrait(double holeX, double holeY, double minSeparationRadiusMM, double labelHeight, double labelWidth);
    // Set the bounding box north of the hole
    void setNorth(double holeX, double holeY, double minSeparationRadiusMM, double labelHeight, double labelWidth);
    // Set the bounding box west of the hole (left, looking at the engraved side of the plate)
    void setEast(double holeX, double holeY, double minSeparationRadiusMM, double labelHeight, double labelWidth);
    // Sets the bounding box in portrait mode - i.e. doesn't rotate it.
    void setEastPortrait(double holeX, double holeY, double minSeparationRadiusMM, double labelHeight, double labelWidth);
    // Sets the bounding box off the plate
    void setDummy();
    // Determines whether the bounding box overlaps the given bounding box
    bool overlaps(LabelBoundingBox bb);
    // Utility function which returns the bounding box vertices as a string.
    std::string toString();

};

enum class LabelPosition{
    SOUTH = 0,
    WEST = 90,
    NORTH = 180,
    EAST = 270,
    WEST_PORTRAIT = 91,
    EAST_PORTRAIT = 271,
    PLATE_EDGE = 888,
    SPECIAL = 999,
    DEFAULT = -1
};
std::ostream& operator<<(std::ostream& os, LabelPosition pos)
{
    return os << static_cast<int>(pos);
}

/*
 * Describes a character in a HoleLabel.
 * It has a position, rotation and the character itself.
 */
class LabelToken {
public:
    double _xll; // Microns
    double _yll; // Microns
    double _theta; // Rotation angle, in degrees
    char _character;

public:
    LabelToken(double xll, double yll, double theta, double character);
    // Utility function which returns the label token as a string.
    std::string toString();

};

class HoleLabel {
private:
    DrillHole* _parentHole;
    std::string _text;
    LabelBoundingBox _boundingBox;
    // The characters, and their positions, which make up the label text
    std::vector<LabelToken> _labelTokens;
    // Position angle of the label with respect to its centre, measured anticlockwise from North.
    // e.g. for LabelPosition::SOUTH, NORTH positionAngleDeg=0.0;
    double _positionAngleDeg;

public:
    // Holes which are within a label height radius of the hole
    std::vector<DrillHole*> _neighbours;
    int _clumpID;
    LabelPosition _labelPosition;

    //    HoleLabel();
    HoleLabel(DrillHole* parentHole);

    // Determines and sets the label text.
    void setText();
    void setShortNameText();
    // First row of descriptive text to be engraved on the plate.
    // This is usually the plate archive name.
    void setLongName1Text();
    // First row of descriptive text to be engraved on the plate.
    // This is usually the dates for which the plate is configured.
    void setLongName2Text();

//    void setText(const std::string& text) {
//        _text = text;
//    }

    const std::string& getText() const {
        return _text;
    }

    void setLabelTokens(double thetaDeg, double radius); // TODO: radius should be an optional parameter
    const std::vector<LabelToken>& getLabelTokens() const {
        return _labelTokens;
    }

    void setBoundingBox(double xll, double yll, double xlr, double ylr, double xul, double yul, double xur, double yur);
    void setBoundingBox(LabelPosition pos);
    void setBoundingBox(LabelPosition pos, bool defaultOrientation);
    void setBoundingBoxOld(int thetaDeg); // Junk version - delete this later
    void setBoundingBox(int thetaDeg);
    // Sets the bounding box a given radius from the plate's centre, with the given rotation.
    // This is used for placing the plate label at the edge of the plate.
    void setBoundingBox(double thetaRad, double radius);
    const LabelBoundingBox& getBoundingBox() const {
        return _boundingBox;
    }

    void setPositionAngle(double angleDeg);
    double getPositionAngle(){
        return _positionAngleDeg;
    }


    /*
     * Returns true if the label's bounding box is outside all of its neighbours'
     * exclusion zones and if it does not overlap the bounding box of any of its
     * neighbours.
     */
    bool proximityToNeighboursOK();

    /*
     * Returns true if the label's bounding box
     * 1) within the plate diameter, and
     * 2) does not impinge on the plate's locator holes
     */
    bool proximityToPlateFixturesOK();
    bool overlaps(HoleLabel label);
    bool withinExclusionZone(DrillHole* holePtr);
    double distanceBetween2Points(
            double xPoint1, double yPoint1,
            double xPoint2, double yPoint2);
    double distanceBetweenPointAndLineSegment2(
            double xPoint, double yPoint,
            double xLineSegBeg, double yLineSegBeg,
            double xLineSegEnd, double yLineSegEnd);
    /*
     * Starting with the given position rotates the plate label until a suitable position is found;
     * i.e. one which does not impinge on its neighbours or the plate fixtures (edge or locator holes).
     * Returns true if the process has found a suitable label position, false otherwise.
     */
    bool rotateLabel(LabelPosition pos);

    /*
     * Starting with the given angle rotates the plate label until a suitable position is found;
     * i.e. one which does not impinge on its neighbours or the plate fixtures (edge or locator holes).
     * Returns true if a suitable position has been found. False otherwise.
     */
    bool rotatePlateLabel(double startAngleDeg);

    LabelToken determineLabelTokens();

    // Utility function which returns the list of neighbouring holes as a string.
    std::string neighboursToString();
    std::string boundingBoxToString();

};


/*
 * A "DrillHole" object is used to maintain details of a hole to
 * be drilled in the plate.   Each hole has a type and a position, and
 * the name of the file in which it was created.  It is also used to
 * write information to a file used for plotting, which is why it stores
 * things like the RA and Dec.
 */
class DrillHole {
private:
    std::string _file;
    int _x;
    int _y;
    unsigned _fileNum;
    unsigned _probeNum;
    unsigned _holeNum;
    SAMI::ObjectType _type;
    std::string _objectName;
    std::string _RA;
    std::string _Dec;
    bool _drill;
    KeyHole _keyhole;
    bool _hasKey;
    // Not all holes should be labelled. Specify that here.
    // By default holes are not labelled. (false)
    // TODO: _doLabel doesn't seem to be used and is always set to false.
    // What's going on here? (npfl 11-Dec-2014)
    bool _doLabel;
    HoleLabel _label;

    static unsigned _currentFileNum;
    const double _thermalExpansionFactor;

//    bool inDrillHoleList(std::vector<DrillHole*> v);

public:
    DrillHole();
    DrillHole(const std::string &file, int x, int y, unsigned probeNum,
              SAMI::ObjectType type,
              const std::string &objectName,
              const std::string &RA, const std::string &Dec);
    DrillHole(const DrillHole& dh);
    DrillHole& operator=(const DrillHole& dh);
    bool operator==(const DrillHole &dh) const;
    bool operator!=(const DrillHole &dh) const;

    // Getters
    double GetRadius() const;
    double GetHoleSize() const;
    double GetMinSeparationSameField() const;
    double GetMinSeparationDiffFieldMM() const;
    int GetX() const { return (_x); }
    int GetY() const { return (_y); }
    KeyHole GetKey() const {return (_keyhole);}
    std::string GetFileName() const { return _file; }
    unsigned GetFileNum() const { return _fileNum; }
    unsigned GetProbeNum() const {return _probeNum; }
    unsigned GetHoleNum() const {return _holeNum; }
    std::string GetObjectName() const { return _objectName; }
    SAMI::ObjectType GetType() const { return _type; }
    char GetTypeAsChar();
    std::string GetRA() const { return _RA; }
    std::string GetDec() const { return _Dec; }
    double GetThermalExpansionFactor() const { return _thermalExpansionFactor; }
    const HoleLabel& getLabel() const { return _label; }
    bool GetDoLabel() const { return _doLabel; }

    bool DoDrill() const { return _drill; }
    bool HasKey() const { return _hasKey; }
    void SetNoDrill() { _drill = false; }
    void SetHoleNum(unsigned num) {
        _holeNum = num;
    }

    void WriteCSV(std::ofstream &ofile, std::ostringstream &oStream) const;
    void WriteCSV_NoDrill(std::ofstream &ofile) const;
    void WriteCSV_Key(std::ostringstream &oStream) const;
    void WriteCNC(std::ofstream &ofile, bool writeKeyhole, bool frontOfPlate) const;
    void WritePlot(std::ofstream &ofile) const;

    bool SameFile(const DrillHole &other) const {
        return _fileNum == other._fileNum;
    }
    bool SameFile(const unsigned otherFileNum) const {
        return _fileNum == otherFileNum;
    }
    bool UsesConnector() const {
        return (_type == SAMI::ProgramObject || _type == SAMI::Sky || _type == SAMI::ImagingGuide);
    }
    double calculateSeparationMicrons(DrillHole h);

    // Utility methods
    std::string xyToString();

    // Static methods
    static void WriteCSV_LocatorHoles(std::ofstream &ofile);
    static void NewFile() { _currentFileNum++; }
    static DrillHole* getNamedHole(unsigned reqFileNum, SAMI::ObjectType reqType,
            unsigned reqProbeNum, DrillHoleVector *drillHoles);

    // Methods dealing with the engraved label
    static void calculateLabelPositions(DrillHoleVector *drillHoles);
    static void assignToClump(int clumpID, DrillHole* holePtr, std::map<int, std::vector<DrillHole*>> &clumpMap);
    bool findNeighbours(DrillHoleVector *drillHoles, double minSeparation);

};

unsigned DrillHole::_currentFileNum = 0;


typedef DrillHoleVector::iterator DrillHoleVectorIt;
typedef DrillHoleVector::const_iterator DrillHoleVectorConstIt;
typedef DrillHoleVector::const_reverse_iterator DrillHoleVectorConstItRev;



/*
 * A "ConfigFile" class is used to maintain details of each input configuration
 * file we process.
 *
 *  Note that we maintain pointers to some used types, so that we can
 *  be copied as part of vector operations without those types being
 *  re-constructed (as they are expensive and may not have copy constructors).
 *
 *  Currently - the pointers leak.  We need a counting point scheme to manage
 *  them correctly.
 */
class ConfigFile {

private:
    SAMI::FileContents *_InputFile;    // Pointer to output file
    AAT::telMdlMean    *_TelModel;     // Pointer to telescope model.
    std::string        _InputFileName; // Input configuration name
    std::string        _OutputFileName;// Output configuration file name.
    std::string        _AladinFileName;// Name for Aladin style output file.
    std::string        _TopcatFileName;// Name for TOPCAT style output file
    bool _writeAladin;                 // Are we writing the Aladin file.
    std::string        _GuideThumbnailFileName; // Name for guide star thumbnail positions file
    std::string        _SkyThumbnailFileName;   // Name for sky thumbnail positions file
    std::string        _TargetThumbnailFileName; // Name for target thumbnail positions file
public:
    ConfigFile(const std::string &outDirectory,
               const std::string &inFileName,
               const bool writeAladinFile,
               double temp,  double pressure,  double humidty);
    ~ConfigFile();
    void Process(DrillHoleVector *drillHoles, double &observingTime);
    void fetchThumbnailImages(std::string guideFilename, std::string targetFilename, std::string skyFilename);
};
typedef std::vector<ConfigFile> ConfigFileVector; // Vector of config files.
typedef ConfigFileVector::iterator ConfigFileVectorIt;
typedef ConfigFileVector::const_iterator ConfigFileVectorConstIt;



/*
 * This is a function object used to calculate a field position for
 * an object.  It is used when invoking ForEachObject() method of the
 * SAMI::FileContents class.
 */
class CalculatePos {
    AAT::telMdlMean    *_TelModel;  // Points to telescope model to use
    DrillHoleVector    *_DrillHoles; // New drill holes added here
    std::string        _InputFileName; // Input config file name
public:
    CalculatePos(AAT::telMdlMean *model, DrillHoleVector *drillHoles,
                 const std::string & fileName) :
        _TelModel(model), _DrillHoles(drillHoles), _InputFileName(fileName) {}
    void operator()(SAMI::Object *object);
};


class CNCFileBuilder {
private:
    std::string _op3Filename;
    std::string _op4Filename;
    std::string _op5Filename;
    std::string _labelFilename;
    DrillHoleVector* _drillHoles;
    std::string _plateName;
    std::string _plateLongname;
    std::string _plateDate;
    std::map<char, float> charToEngravingCodeMap;

    void initCharToEngravingCodeMap();
public:
//    CNCFileBuilder();
    CNCFileBuilder(const std::string & cncFilename,
            DrillHoleVector *drillHoles,
            const std::string & plateName,
            const std::string & plateLongname,
            const std::string & plateDate);



    void build();
    void buildHeader(std::ofstream &ofile);
    void buildFooter(std::ofstream &ofile);

    void build_Op3File();
    void buildOp3Header(std::ofstream &ofile);
    void buildOp3MainHoles_toolSetup(std::ofstream &ofile);
    void buildOp3MainHoles_hexabundle(std::ofstream &ofile);
    void buildOp3MainHoles_sky(std::ofstream &ofile);
    // The following is no longer used but is kept for reference.
    void buildOp3MainHoles(std::ofstream &ofile);
    void buildOp3KeyHoles(std::ofstream &ofile);

    void buildOp3MainHoles2(std::ofstream &ofile);
    // We may or may not need the following methods
    void buildOp3MainHoles2_toolSetup(std::ofstream &ofile);
    void buildOp3MainHoles2_sky(std::ofstream &ofile);
    void buildOp3MainHoles2_hexabundle(std::ofstream &ofile);

    void buildOp3KeyHoles2(std::ofstream &ofile);

    void build_Op4File();
    void buildOp4Header(std::ofstream &ofile);
    void buildOp4MainHoles(std::ofstream &ofile);
    void buildOp4KeyHoles(std::ofstream &ofile);

    void build_EngraveFile();
    void buildEngraveHeader(std::ofstream &ofile);
    void buildEngraveLabels(std::ofstream &ofile);
    void buildEngraveHoleLabel(std::ofstream & ofile, HoleLabel holeLabel, std::ofstream& debugOutFile);
    void buildEngraveHoleLabelOld(std::ofstream & ofile, HoleLabel holeLabel, std::ofstream& debugOutFile);
    void buildEngraveDefinitions(std::ofstream & ofile);

    void build_Op5File();
    void buildOp5Header(std::ofstream &ofile);
    // A header to list which tools will be used by the program
    void buildOp5ToolDefinitionHeader(std::ofstream &ofile);
    void buildOp5MainHoles(std::ofstream &ofile);
    void buildOp5KeyHoles(std::ofstream &ofile);
    void buildOp5MainHoles2(std::ofstream &ofile);
    void buildOp5KeyHoles2(std::ofstream &ofile);

    float convertCharacterToEngraveCode(char character);

    const std::string& getLabelFilename() const {
        return _labelFilename;
    }

    const std::string& getOp4Filename() const {
        return _op4Filename;
    }

    const std::string& getOp5Filename() const {
        return _op5Filename;
    }
};


/*
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Prototypes for static functions.
 *
 */
static void Process(ConfigFileVector *input_files,
                    const std::string & drill_file,
                    const std::string & cnc_file,
                    const std::string & plot_file,
                    const std::string & plate_name,
                    const std::string & plate_longname,
                    const double temp);

typedef enum { Primary='P', Aladin='A', TargetThumbnail='T', GuideThumbnail='G', SkyThumbnail='S', Topcat='C' } OutputFileType;
static std::string GetOutputFileName(const std::string &directory,
                                     const std::string &input_file,
                                     const OutputFileType fileType);


static unsigned  CheckForHoleCollisions(DrillHoleVector *drillHoles);
static bool DoHolesCollide(DrillHole & h1, DrillHole & h2);

static void OutputDrillHoleList(const std::string & drill_file,
                                const DrillHoleVector *drillHoles,
                                const double temp,
                                unsigned HoleCount,
                                const std::string & plate_name,
                                const std::string & plate_longname,
                                const std::string & plate_date);
static void OutputPlotList(const std::string & drill_file,
                           const DrillHoleVector *drillHoles,
                           const std::string & plate_name,
                           const std::string & plate_longname,
                           const std::string & plate_date);

//static void OutputDrillHoleListToCNC(
//        const std::string & cnc_file,
//        DrillHoleVector *drillHoles,
//        const std::string & plate_name,
//        const std::string & plate_longname,
//        const std::string & plate_date);
//
//static void OutputToCNC_OP3File(
//        const std::string &cnc_filename,
//        DrillHoleVector *drillHoles,
//        const std::string & plate_name,
//        const std::string & plate_longname,
//        const std::string & plate_date);
//
//static void OutputToCNC_header(std::ofstream &ofile,
//        const std::string & plate_name,
//        const std::string & plate_longname,
//        const std::string & plate_date);
//static void OutputToCNC_footer(std::ofstream &ofile);
//
//static void OutputToCNCop3_header(std::ofstream &ofile);
//static void OutputToCNCop3_mainHoles(std::ofstream &ofile, DrillHoleVector *drillHoles);
//static void OutputToCNCop3_keyHoles(std::ofstream &ofile, DrillHoleVector *drillHoles);
//static void OutputToCNCop3_mainHoles2(std::ofstream &ofile, DrillHoleVector *drillHoles);
//static void OutputToCNCop3_keyHoles2(std::ofstream &ofile, DrillHoleVector *drillHoles);
//
//static void OutputToCNC_OP4File(
//        const std::string &cnc_filename,
//        DrillHoleVector *drillHoles,
//        const std::string & plate_name,
//        const std::string & plate_longname,
//        const std::string & plate_date);
//
//static void OutputToCNCop4_header(std::ofstream &ofile);
//static void OutputToCNCop4_mainHoles(std::ofstream &ofile, DrillHoleVector *drillHoles);
//static void OutputToCNCop4_keyHoles(std::ofstream &ofile, DrillHoleVector *drillHoles);
//
//static void OutputToCNC_labelFile(
//        const std::string &labelFilename,
//        DrillHoleVector *drillHoles,
//        const std::string & plate_name,
//        const std::string & plate_longname,
//        const std::string & plate_date);
//
//static void OutputToCNClabel_header(std::ofstream &ofile);
//static void OutputToCNClabel_labels(std::ofstream &ofile, DrillHoleVector *drillHoles);
//static void OutputToCNClabels_holeLabel(std::ofstream & ofile, std::string holeLabel);
//static void OutputToCNClabel_definitions(std::ofstream &ofile);

static void usage(const char *);

#define CELSIUS_TO_KELVIN(_c) ((_c)+273.150)
#define ANGSTROMS_TO_MICRONS (1.0/10000.0)

inline double SQRD(double x) { return x*x; }

inline double MMtoMicrons(double m) { return m*1000.0; }
inline double MicronsToMM(int m) { return double(m)/1000.0; }



#endif /* SAMIGENCFG_H_ */
