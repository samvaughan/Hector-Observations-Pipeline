/*+          C O D _ P L A T E S E G M E N T E R _ T E S T . H
 *
 * Module name:
 *     cod_PlateSegmenter_test.h
 *
 * Function:
 *     Unit test counterpart to cod_PlateSegmenter.h
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
 *     "@(#) $Id: ACMM:coneOfDarkness/cod_PlateSegmenter_test.h,v 1.3 13-Mar-2018 13:57:32+11 nlorente $"
 *
 */

#ifndef COD_PLATESEGMENTER_TEST_H_
#define COD_PLATESEGMENTER_TEST_H_

#include <vector>
#include <sstream>

#include "cpptest.h"
#include "cod_PlateSegmenter.h"

namespace COD {

class PlateSegmenterTest : public Test::Suite{
private:
    void testConstructors();
    void testNoExtantHoles();
    void testWithExtantHoles();
    void testCandidatesAvailable();
    void testGetMostIsolatedCandidate();
    void testLessIsolatedThan();

    PositionList generatePositions();
    PositionList generatePositions2();
    PositionList generatePositions1();
    PositionList generatePositions4();
public:
    PlateSegmenterTest(){
        TEST_ADD(PlateSegmenterTest::testConstructors);
        TEST_ADD(PlateSegmenterTest::testNoExtantHoles);
        TEST_ADD(PlateSegmenterTest::testWithExtantHoles);
        TEST_ADD(PlateSegmenterTest::testCandidatesAvailable);
        TEST_ADD(PlateSegmenterTest::testGetMostIsolatedCandidate);
        TEST_ADD(PlateSegmenterTest::testLessIsolatedThan);
    }
};

} // Namespace
#endif /* COD_PLATESEGMENTER_TEST_H_ */
