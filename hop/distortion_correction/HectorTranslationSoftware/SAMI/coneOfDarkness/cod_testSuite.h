/*+          C O D _ T E S T S U I T E . H
 *
 * Module name:
 *     cod_testSuite.h
 *
 * Function:
 *     Unit test suite for the coneOfDarkness sub-system.
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
 *     "@(#) $Id: ACMM:coneOfDarkness/cod_testSuite.h,v 1.3 13-Mar-2018 13:57:33+11 nlorente $"
 *
 */

#ifndef CONEOFDARKNESS_TESTSUITE_H_
#define CONEOFDARKNESS_TESTSUITE_H_

#include <memory>

#include "cpptest.h"

#include "coneOfDarkness_test.h"
#include "cod_PlateSegmenter_test.h"
#include "cod_Position_test.h"
#include "cod_PlateToSkyConverter_test.h"

namespace COD {

class TestSuite : public Test::Suite{
public:
    TestSuite();
};

} // Namespace
#endif /* CONEOFDARKNESS_TESTSUITE_H_ */
