/*+          C O D . H
 *
 * Module name:
 *     cod.h
 *
 * Function:
 *     Catalogue and position checker.
 *
 * Language:
 *     C++
 *
 * Support:
 *     Nuria P.F. Lorente, AAO
 *
 * History:
 *     See ChangeLog
 *
 * ACMM Id:
 *     "@(#) $Id: ACMM:coneOfDarkness/cod.h,v 1.3 13-Mar-2018 13:57:33+11 nlorente $"
 *
 */

#ifndef COD_H_
#define COD_H_

#include <sstream>
#include <iomanip>
#include <map>
#include <errno.h>
#include <cstring>

#include "curl/curl.h"

#include "cod_util.h"
#include "cod_Position.h"
#include "cod_PlateSegmenter.h"
#include "cod_PlateToSkyConverter.h"

namespace COD {

enum CatalogueObjectType {GALAXY=1, STAR=2, UNCLASSIFIABLE=3, NOISE=4};

class CatObject{
public:
    RightAscension _ra;
    Declination _dec;
    double _epoch;
    double _ellipseA;
    double _ellipseB;
    double _ellipsePA;
    CatalogueObjectType _objectType;
    std::string _id;            // Original catalogue ID of this object

    CatObject(RightAscension ra, Declination dec,
            float epoch, double ellipseA, double ellipseB, double ellipsePA, CatalogueObjectType type, std::string id);
    virtual ~CatObject();
    double distanceFrom(RightAscension ra, Declination dec);
    double distanceFrom(double raRad, double decRad);
    friend std::ostream& operator<<(std::ostream& stream, const CatObject& catObj);
};

class Catalogue {
private:
    std::vector<CatObject> _objectVector;
    std::string buildConeSearchURL(double centralPosRADeg, double centralPosDecDeg, double radiusDeg);
    std::string buildBoxSearchURL(double centralPosRADeg, double centralPosDecDeg, double radiusDeg);
public:
    Catalogue();
    Catalogue(RightAscension centralPosRA, Declination centralPosDec, double radiusDegrees);
    virtual ~Catalogue();
    /* Determines whether there is an object in the field defined by the
     * given central position and radius.
     */
    /* Loads a catalogue from a file */
    void loadCat(std::string filename);
    /* Gets a catalogue via a VO cone search */
    std::string getCat(double centralPosRADeg, double centralPosDecDeg, double radiusDeg);
    void writeCatToFile(std::string filename, std::string votableXml);
    void parseCat(std::ifstream& ifile);
    std::vector<CatObject> objectsInField(RightAscension ra, Declination dec, double radius);
    bool foundObjectsInField(RightAscension ra, Declination dec, double radius);
    static void initialise();
    static void cleanup();
};


class PositionChecker {
private:
    // TODO: The following should be a map, keyed on field ID
    std::map<int, Catalogue> _mapOfCatalogues;
    //Catalogue _catalogue;
    PlateToSkyConverter _plateToSkyConverter;
public:
    PositionChecker();
    //PositionChecker(std::vector<FocalPlanePosition> fieldCentreVector, double radiusDegrees, std::map<int, TelModelParams> telModelMparamsMap);
    PositionChecker(FocalPlanePosition fieldCentre, double radiusDegrees, std::map<int, TelModelParams> telModelMparamsMap);
    virtual ~PositionChecker();
    void initialiseCatalogue(int fieldID, RightAscension centralPosRA, Declination centralPosDec, double radiusDegrees);
    //void initialiseCatalogues(std::vector<FocalPlanePosition> fieldCentreVector, double radiusDegrees);
    void initialiseCatalogues(FocalPlanePosition fieldCentre, double radiusDegrees);
    bool isEmpty(int fieldID, RightAscension candidateRA, Declination candidateDec, double radiusDegrees);
    bool isEmpty(int fieldID, SkyPosition candidatePosition, double radiusDegrees);
    bool isEmptyForAllFields(FocalPlanePosition& candidatePosition, double radiusDegrees);
    /**
     * Takes in a list of candidate positions and returns a vector containing those which are empty.
     * It may be necessary to include more information than just the ra,dec in both input and output,
     * so that the position can be tranced back to an x,y hole on the plate.
     */
    //std::vector<EquatorialPosition> filterEmptyRegions(int fieldID, std::vector<EquatorialPosition>);
    std::vector<FocalPlanePosition> filterEmptyRegions(int fieldID, int resultSize, std::vector<FocalPlanePosition> candidatePosVector);
};

} /* namespace COD */
#endif
