/*+          C O D _ P O S I T I O N . H
 *
 * Module name:
 *     cod_Position.h
 *
 * Function:
 *     Position-related data structures.
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
 *     "@(#) $Id: ACMM:coneOfDarkness/cod_Position.h,v 1.3 13-Mar-2018 13:57:34+11 nlorente $"
 *
 */

#ifndef COD_POSITION_H_
#define COD_POSITION_H_

#include <iostream>
#include <sstream>
#include <iomanip>
#include <vector>
#include <string>
#include <fstream>
#include <cmath>
#include <limits>
#include <map>
#include <errno.h>
#include <stdexcept>
#include <algorithm>
#include <cstring>

#include "aatTelMdl.h"
#include "sami_positions.h"

#include "cod_util.h"

namespace COD {

std::string decimalToSexagesimal(double degrees);

class RightAscension{
private:
    double _hours;
    double _uncertainty;

public:
    RightAscension();
    /*
     * Input is a colon-delimited sexagesimal string - "hh:mm:ss.s"
     * Uncertainty is in hours.
     */
    RightAscension(const std::string& sexagesimal, double uncertaintyHours=0.0);
    RightAscension(double degrees, double uncertaintyDegrees=0.0);
    virtual ~RightAscension();
    std::string toString_SpaceDelimited();
    friend std::ostream& operator<<(std::ostream& stream, const RightAscension& ra);
    friend bool operator==(const RightAscension& raLhs, const RightAscension& raRhs);
    double getDegrees();
    /*
     * Inputs are in degrees.
     */
    void setDegrees(double degrees);
    void setUncertaintyDegrees(double degrees);
    double getRadians();
    void setRadians(double radians);

    double getHours() const {
        return _hours;
    }
    void setHours(double hours) {
        this->_hours = hours;
    }
    double getUncertainty() const {
        return _uncertainty;
    }
};

class Declination{
private:
    double _degrees;
    double _uncertainty;

public:
    Declination();
    Declination(const std::string& sexagesimal, double uncertainty=0.0);
    Declination(double degrees, double uncertainty=0.0);
    virtual ~Declination();
    std::string toString_SpaceDelimited();
    friend std::ostream& operator<<(std::ostream& stream, const Declination& dec);
    friend bool operator==(const Declination& decLhs, const Declination& decRhs);

    double getDegrees() const {
        return _degrees;
    }

    void setDegrees(double degrees) {
        this->_degrees = degrees;
    }

    void setUncertainty(double degrees){
        this->_uncertainty = degrees;
    }

    double getUncertainty(){
        return _uncertainty;
    }

    double getRadians(){
        return (_degrees * PION180);
    }

    void setRadians(double radians){
        this->_degrees = radians / PION180;
    }

};

class EquatorialPosition{
public:
    RightAscension _ra;
    Declination _dec;
    EquatorialPosition();
    EquatorialPosition(RightAscension RA, Declination Dec);
    virtual ~EquatorialPosition();
    /* Returns the object as a string */
    std::string str() const;
    friend std::ostream& operator<<(std::ostream& stream, const EquatorialPosition& position);
};

class SkyPosition{
public:
    EquatorialPosition _meanPosition;
    EquatorialPosition _apparentPosition;
    EquatorialPosition _calcMeanPosition;
    EquatorialPosition _calcApparentPosition;
    SkyPosition();
    SkyPosition(EquatorialPosition meanPos);
    SkyPosition(std::string meanRA, std::string meanDec);
    virtual ~SkyPosition();

    /* Returns the object as a string */
    std::string toString(bool debug) const;
    friend std::ostream& operator<<(std::ostream& stream, const SkyPosition& position);

    const EquatorialPosition& getApparentPosition() const {
        return _apparentPosition;
    }

    void setApparentPosition(const EquatorialPosition& apparentPosition) {
        _apparentPosition = apparentPosition;
    }

    const EquatorialPosition& getCalcApparentPosition() const {
        return _calcApparentPosition;
    }

    void setCalcApparentPosition(const EquatorialPosition& calApparentPosition) {
        _calcApparentPosition = calApparentPosition;
    }
    void setCalcApparentPosition(const RightAscension& RA, const Declination& Dec);

    const EquatorialPosition& getCalcMeanPosition() const {
        return _calcMeanPosition;
    }

    void setCalcMeanPosition(const EquatorialPosition& calcMeanPosition) {
        _calcMeanPosition = calcMeanPosition;
    }
    void setCalcMeanPosition(const RightAscension& RA, const Declination& Dec);

    const EquatorialPosition& getMeanPosition() const {
        return _meanPosition;
    }

    void setMeanPosition(const EquatorialPosition& meanPosition) {
        _meanPosition = meanPosition;
    }
};

class TelModelParams{
public:
    int _fieldID;      // The field to which the model belongs
    double _mjd;        // Observing time
    double _tempK;      // Dome temperature (default=15 degC)
    double _pressure;   // default=850 millibars
    double _humidity;   // (default=0.5)
    double _wavelength; // Angstroms
    double _distortionCentreX;
    double _distortionCentreY;

    TelModelParams(int fieldID, double mjd, double tempK, double pressure, double humidity, double wavelength, double distortionCtrX, double distortionCtrY);
    TelModelParams(int fieldID=0, double mjd=0);
    friend std::ostream& operator<<(std::ostream& stream, const TelModelParams& telModelParams);
};

class FocalPlanePosition {
private:
    std::string _id;
    double _x;       // Position on the plate (mm)
    double _y;       // Position on the plate (mm)
    // For efficiency, the distance to the nearest neighbour is only
    // calculated/updated just before it is used.
    double _distToNearestHole; // (mm)
    // The sky positions corresponding to the focal plane position, keyed by fieldID
    std::map<int, SkyPosition> _skyPositionMap;

public:
    FocalPlanePosition(); // From CP
    FocalPlanePosition(const std::string& id, double x, double y, const std::string& RA, const std::string& Dec, int fieldID);
    FocalPlanePosition(double x, double y); // From CP
    virtual ~FocalPlanePosition();

    SkyPosition* addSkyPos(int fieldID, const std::string& meanRA, const std::string& meanDec);
//    std::map<int, SkyPosition>::iterator addSkyPos(int fieldID, const std::string& meanRA, const std::string& meanDec);
    // Get a reference to the sky position for a given fieldID
    SkyPosition* getSkyPos(int fieldID) throw (std::invalid_argument);
//    std::map<int, SkyPosition>::iterator getSkyPos(int fieldID) throw (std::invalid_argument);
    bool skyPosExists(int fieldID);
    double distanceTo(double x, double y); // From CP
    double distanceTo(FocalPlanePosition position);
    std::string toString(bool printHeader, bool debug);

    friend int findMostIsolated(std::vector<FocalPlanePosition> focalPlanePosVector); // From CP
    friend std::ostream& operator<<(std::ostream& stream, const FocalPlanePosition& position); // CP part needs to be merged
    friend bool operator==(const FocalPlanePosition& posLhs, const FocalPlanePosition& posRhs);

    /*** Getters and setters ***/
    double getDistToNearestHole() const {return _distToNearestHole;}
    void setDistToNearestHole(double distToNearestHole) {_distToNearestHole = distToNearestHole;}
    const std::string& getId() const {return _id;}
    void setId(const std::string& id) {_id = id;}
    double getX() const {return _x;}
    void setX(double x) {_x = x;}
    double getY() const {return _y;}
    void setY(double y) {_y = y;}
    std::map<int, SkyPosition>& getSkyPositionMap() {return _skyPositionMap;}
    int getNumFields();
};

class PositionList {
private:
    std::vector<FocalPlanePosition> _positionVector;
public:
    PositionList();
    virtual ~PositionList();
    std::vector<FocalPlanePosition>::iterator begin();
    std::vector<FocalPlanePosition>::iterator end();
    double size();
    /* Adds a position to the list */
    void add(FocalPlanePosition position);
    /* Removes a position from the list */
    void remove(FocalPlanePosition position);
    std::vector<FocalPlanePosition>::iterator remove(std::vector<FocalPlanePosition>::iterator posIter);
    /* Returns the first item in the list, removing it in the process */
//    FocalPlanePosition get();
    FocalPlanePosition& at(int index);
    /* Returns the distance from the input position to the closest PositionList object */
    double nearestNeighbourDistance(FocalPlanePosition position); // distance from nearest list obj to pos
    /* Based on the input position list, sets the nearest neighbour distance of each position in THIS position list. */
    /* The input list are the (fixed) neighbours - their distance members are not modified.
     * The distance members of THIS position list are modified.
     */
    void setNearestNeighbourDistance(const PositionList neighboursList); // set nearest neighbour distance of each list position, based on the input set of neighbours.
    void toFile(std::string filename);
    void toCSVFile(std::vector<std::string> cfgFilenameV);
    std::string toString(bool printHeader);
    void toString(std::vector<std::stringstream>& streamV, std::vector<int>& fieldV);
//    friend std::ostream& operator<<(std::ostream& stream, const PositionList& list);
};

class CandidatePositionList : PositionList {

};

//class samiSkyTest {
//private:
//    AAT::telMdlMean *_telModel;  // The telescope model
//public:
//    samiSkyTest();
//    virtual ~samiSkyTest();
//    //void setupTelModel();
//    //void readTestDataFile(const char* infileName, std::vector<FocalPlanePosition>& testDataVector);
//    void translatePlate2AppSky(std::vector<FocalPlanePosition>& testDataVector);
//    void translatePlate2AppSky(FocalPlanePosition& position);
//};



} /* namespace COD */
#endif /* COD_POSITION_H_ */

