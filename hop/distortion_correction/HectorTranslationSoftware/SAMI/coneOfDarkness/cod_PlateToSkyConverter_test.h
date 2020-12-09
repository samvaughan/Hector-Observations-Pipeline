/*+          C O D _ P L A T E T O S K Y C O N V E R T E R _ T E S T . H
 *
 * Module name:
 *     cod_PlateToSkyConverter_test.h
 *
 * Function:
 *     Unit test counterpart to cod_PlateToSkyConverter.h
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
 *     "@(#) $Id: ACMM:coneOfDarkness/cod_PlateToSkyConverter_test.h,v 1.3 13-Mar-2018 13:57:33+11 nlorente $"
 *
 */

#ifndef COD_PLATETOSKYCONVERTER_TEST_H_
#define COD_PLATETOSKYCONVERTER_TEST_H_

#include <string>
#include <iostream>

#include "cpptest.h"
#include "aatTelMdl.h"
#include "cod_PlateToSkyConverter.h"
#include "sami_util.h"

#include "cod_util.h"
#include "cod_Position.h"

namespace COD {

class PlateToSkyConverterTest : public Test::Suite {
private:
    void testTelMdlMean();
    void testConstructor();

    TelModelParams generateTelescopeModelParams1(int fieldID);
    TelModelParams generateTelescopeModelParams2(int fieldID);
    TelModelParams generateTelescopeModelParams3(int fieldID);
    std::map<int, TelModelParams> generateTelescopeModelParamsMap(int modelsRequired);
public:
    PlateToSkyConverterTest(){
        TEST_ADD(PlateToSkyConverterTest::testTelMdlMean);
        TEST_ADD(PlateToSkyConverterTest::testConstructor);
    }
};

} // Namespace
#endif /* COD_PLATETOSKYCONVERTER_TEST_H_ */
