/*+          C O D _ U T I L . H
 *
 * Module name:
 *     cod_util.h
 *
 * Function:
 *     Various utilities.
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
 *     "@(#) $Id: ACMM:coneOfDarkness/cod_util.h,v 1.3 13-Mar-2018 13:57:33+11 nlorente $"
 *
 */

#ifndef COD_UTIL_H_
#define COD_UTIL_H_

#include <string>
#include <vector>
#include <sstream>
#include <iostream>
#include <limits>

namespace COD {

#define CELSIUS_TO_KELVIN(_c) ((_c)+273.150)
#define ANGSTROMS_TO_MICRONS (0.0001)
#define PI (3.14159265358979323846264338327950288)
#define PION180 (PI/180.0)

const double UNDEFINED_D=std::numeric_limits<double>::max();

inline double MMtoMicrons(double m) { return m*1000.0; }

std::string trim(const std::string &s);
std::vector<std::string> &split(const std::string &input, char delimiter, std::vector<std::string> &tokenVector);
std::vector<std::string> &split(const std::string &input, std::string startDelimiter, std::string endDelimiter, std::vector<std::string> &tokenVector);
std::vector<std::string> &splitXmlTableRow(const std::string &input, std::vector<std::string> &tokenVector);
/**
 * Takes a string in the format "hh:mm" and returns
 * a string in the format "hh mm ss", suitable as a parameter
 * for the SAMI::Ut2Mjd method
 */
std::string formatTime(const std::string &s);
std::string formatDate(const std::string &s);
std::string formatMonthNumeric(const std::string &s);

} // Namespace


#endif /* COD_UTIL_H_ */
