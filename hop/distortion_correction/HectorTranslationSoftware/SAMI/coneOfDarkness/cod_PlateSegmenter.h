/*+          C O D _ P L A T E S E G M E N T E R . H
 *
 * Module name:
 *     cod_PlateSegmenter.h
 *
 * Function:
 *    Partitions the field into a grid, with the constraint that a cell
 *    may not overlap with the central exclusion zone or any of the extant
 *    holes already allocated to the plate.
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
 *     "@(#) $Id: ACMM:coneOfDarkness/cod_PlateSegmenter.h,v 1.3 13-Mar-2018 13:57:33+11 nlorente $"
 *
 */

#ifndef COD_PLATESEGMENTER_H_
#define COD_PLATESEGMENTER_H_

#include <algorithm>
#include <limits>
#include <cmath>
#include <vector>

#include "cod_Position.h"

namespace COD {

/* The following quantities are defined in samigencfg.C.
 * These are not in a namespace, so they are copied here so they can be used.
 * At some point, all of these constants should be moved to a configuration file.
 */
const double SKY_PROBE_RADIUS_MM=5.0;   /* Sky Probe footprint, mm */
const double SKY_HOLE_DIAMETER=4.20;    /* Sky drill hole size, mm */
const double IFU_CONNECTOR_SEPARATION=15.0;  /* Min separation between connectors on a field, mm */
const double SKY_CONNECTOR_SEPARATION=15.0;  /* Min separation between connectors in a field, mm */
const double CENTRAL_EXCLUSION=30.0;         /* Min separation between a connector and the centre of the plate, mm */
const double PLATE_RADIUS_DEGREES=0.5;       /* Size of the plate */
const double PLATE_SCALE=15.2;               /* Arcsec/mm */
const double PLATE_RADIUS_MM=PLATE_RADIUS_DEGREES*3600.0/PLATE_SCALE; /* Radius of plate, mm */


/*
 * Builds a list of candidate positions, based on existing holes and exclusion zones.
 */
bool lessIsolatedThan(FocalPlanePosition pos1, FocalPlanePosition pos2);

class PlateSegmenter {
private:
    PositionList _candidatePositionList;
    // Extant plate holes, which define the forbidden areas for sky candidates.
    PositionList _extantHoleList;
    bool inForbiddenArea(FocalPlanePosition candidate);
    bool inSkyHoleExclusionZone(FocalPlanePosition candidate, FocalPlanePosition skyHole);
public:
    /*
     * Builds a list of candidate positions, in the simple case where there are no extant holes.
     */
    PlateSegmenter();
    PlateSegmenter(PositionList& extantHoleList);
    virtual ~PlateSegmenter();
    void segment_grid();
    FocalPlanePosition& getMostIsolatedCandidate();
    bool candidatesAvailable();
    /* Removes a given position from the list of candidate positions */
    void deleteCandidatePosition(FocalPlanePosition position);
    /* Removes the position given by the input iterator from the list of candidate positions */
    std::vector<FocalPlanePosition>::iterator deleteCandidatePosition(std::vector<FocalPlanePosition>::iterator posIter);
    /* Adds a given position to the list of extant holes */
    void addExtantHole(FocalPlanePosition position);
    /* Removes a given position from the candidate list and adds
     * it to the extant list.
     * i.e. Confirms a candidate position, so it becomes an extant
     * hole on the plate.
     */
    void candidateToExtant(FocalPlanePosition position);


    const PositionList& getCandidatePositionList() const {
        return _candidatePositionList;
    }
    /* This method should ONLY be used for testing the PlateSegmenter class
     * The candidate position list should normally only be set by the
     * segment_grid() method.
     */
    void setCandidatePositionList(const PositionList& candidatePositionList) {
        std::cout << "WARNING: Internal list has been set by a getter. This is an error unless it is part of a test." << std::endl;
        _candidatePositionList = candidatePositionList;
    }

    const PositionList& getExtantHoleList() const {
        return _extantHoleList;
    }

};

} /* namespace SAMI */
#endif /* COD_PLATESEGMENTER_H_ */
