/*+          C O N E O F D A R K N E S S _ T E S T . H
 *
 * Module name:
 *     coneOfDarkness_test.h
 *
 * Function:
 *     Unit test counterpart of coneOfDarkness.h
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
 *     "@(#) $Id: ACMM:coneOfDarkness/coneOfDarkness_test.h,v 1.3 13-Mar-2018 13:57:32+11 nlorente $"
 *
 */

#ifndef CONEOFDARKNESS_TEST_H_
#define CONEOFDARKNESS_TEST_H_

#include "cpptest.h"
#include "coneOfDarkness.h"

namespace COD {

class coneOfDarknessTest : public Test::Suite {
private:
    void testGetInputParameters();
    std::vector<CatObject> findObjects(COD::Catalogue cat, RightAscension candidateRA, Declination candidateDec, double radiusDegrees);
    void testKnownBadPosition();

public:
    coneOfDarknessTest(){
        TEST_ADD(coneOfDarknessTest::testGetInputParameters);
        TEST_ADD(coneOfDarknessTest::testKnownBadPosition);
    }
};

} // Namespace
#endif /* CONEOFDARKNESS_TEST_H_ */
