/*+          DarkNight.h
 *
 * Module name:
 *     DarkNight.h
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
 *     Created on: 11/07/2014 (nlorente)
 *     See ChangeLog
 *
 * ACMM Id:
 *     "@(#) $Id: ACMM:coneOfDarkness/DarkNight.h,v 1.3 13-Mar-2018 13:57:32+11 nlorente $"
 *
 */
#ifndef DARKNIGHT_H_
#define DARKNIGHT_H_

#include "coneOfDarkness.h"

namespace COD {

class DarkNight: public ConeOfDarkness {
public:
    DarkNight(std::string infileName);
    virtual ~DarkNight();

    /* There's probably a better way of doing this, but
     * for now we'll just override the method from the parent.
     */
    void getInputParameters(std::string infileName);
};

} /* namespace COD */

#endif /* DARKNIGHT_H_ */
