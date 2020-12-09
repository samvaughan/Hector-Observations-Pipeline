/*+          C O D _ P O S I T I O N _ T E S T . H
 *
 * Module name:
 *     cod_Position_test.h
 *
 * Function:
 *     Unit test counterpart to cod_Position.h
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
 *     "@(#) $Id: ACMM:coneOfDarkness/cod_Position_test.h,v 1.3 13-Mar-2018 13:57:33+11 nlorente $"
 *
 */

#ifndef COD_POSITION_TEST_H_
#define COD_POSITION_TEST_H_

#include <string>
#include <sstream>

#include "cpptest.h"
#include "cod_Position.h"

namespace COD {

class RightAscensionTest : public Test::Suite{
private:
    void testConstructors();
    void testOperators(); // Test << and ==

public:
    RightAscensionTest(){
        TEST_ADD(RightAscensionTest::testConstructors);
    }
};

class FocalPlanePositionTest : public Test::Suite{
private:
    void testConstructors();
    void testAddSkyPos();
    void testDistanceTo();
    void checkDistanceTo(double x, double y, double toX, double toY, double expectedDistance, std::string msg);
public:
    FocalPlanePositionTest(){
        TEST_ADD(FocalPlanePositionTest::testConstructors);
        TEST_ADD(FocalPlanePositionTest::testAddSkyPos);
        TEST_ADD(FocalPlanePositionTest::testDistanceTo);
    }
};

class PositionListTest : public Test::Suite{
private:
    void testConstructors();
    void testAdd();
    void testNearestNeighbourDistance();
    void testSetNearestNeighbourDistance();
    void testRemove();
public:
    PositionListTest(){
        TEST_ADD(PositionListTest::testConstructors);
        TEST_ADD(PositionListTest::testAdd);
        TEST_ADD(PositionListTest::testNearestNeighbourDistance);
        TEST_ADD(PositionListTest::testSetNearestNeighbourDistance);
        TEST_ADD(PositionListTest::testRemove);
    }
};

} // Namespace
#endif /* COD_POSITION_TEST_H_ */
