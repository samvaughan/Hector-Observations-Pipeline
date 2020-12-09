/*+          C O D _ P L A T E T O S K Y C O N V E R T E R . H
 *
 * Module name:
 *     cod_PlateToSkyConverter.h
 *
 * Function:
 *     Converts positions on the plate to sky positions.
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
 *     "@(#) $Id: ACMM:coneOfDarkness/cod_PlateToSkyConverter.h,v 1.3 13-Mar-2018 13:57:32+11 nlorente $"
 *
 */

#ifndef COD_PLATETOSKYCONVERTER_H_
#define COD_PLATETOSKYCONVERTER_H_

#include <vector>
#include <errno.h>
#include <map>

#include "aatTelMdl.h"
#include "sami_util.h"
#include "cod_util.h"
#include "cod_Position.h"

namespace COD {

class PlateToSkyConverter {
private:
    std::map<int, AAT::telMdlMean*> _telModelMap;  // The telescope model

public:
    PlateToSkyConverter();
    PlateToSkyConverter(FocalPlanePosition fieldCentre, std::map<int, TelModelParams> telModelParamsMap);
    virtual ~PlateToSkyConverter();
    void setupTelescopeModel(int fieldID, SkyPosition fieldCentrePos, TelModelParams modelParams);
    void translatePlate2AppSky(std::vector<FocalPlanePosition>& testDataVector);
    void translatePlate2AppSky(FocalPlanePosition& position);

//    const AAT::telMdlMean*& getTelModelMap(int fieldID) const;

    const std::map<int, AAT::telMdlMean*>& getTelModelMap() const {
        return _telModelMap;
    }
};

} /* namespace COD */
#endif /* COD_PLATETOSKYCONVERTER_H_ */
