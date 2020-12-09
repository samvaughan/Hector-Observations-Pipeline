#ifndef SAMI_UTIL_INC
#define SAMI_UTIL_INC
/*                                  s a m i _ u t i l

 * Module name:
      sami.h

 * Function:
	Include file that defines the SAMI file reader - utility functions.
      

 * Description:
        This C++ incude file declares the SAMI file reader and
        releated classes utility functions.  

        This file is NOT expected to be included by the user and is
        not included by sami.h, it is used by the various SAMI classes
        to help implement them.

        SAMI is an AAT Instrument which implements a number of IFU
        probes on the AAT Prime Focus.
      

 * Language:
      C++

 * Support: Tony Farrell, AAO

 * History:
      19-May-2011 - TJF - Start history.
      07-Dec-2011 - TJF - As part of being extrated from 2dFctrlTask ACMM 
                          module into the SAMI module, create our own versions
                          of Mjd2Ut() and Mjd2Tm.
      08-May-2012 - TJF - Add RegressionTestMode variable to enable regression
                            test mode.


 * "@(#) $Id: ACMM:sami/sami_util.h,v 1.29 23-Feb-2016 12:26:12+11 tjf $"

 */
#include <drama.h>
#include <status.h>
#include <string>
#include <vector>
#include <time.h>
#include <sstream>
#include <algorithm>
#include <limits>
namespace SAMI {
    /* 
     * This is set to put us in regression test mode, the only thing
     * of interest here is to not output things like run date and code
     * version which change in otherwise identical output.  Also, reduce
     * precision of some output values.  This makes
     * it easy to compare the output of the program to a previous 
     * run.
     */
    extern bool RegressionTestMode;

    extern double myAtof(const std::string &str, int lineNum);
    extern long   myAtol(const std::string &str, int lineNum);
    extern void csvline_populate(std::vector<std::string> &record, 
                             const std::string& line, 
                             char delimiter=',');

    extern double Ut2Mjd(std::string utdate, std::string time);
    extern std::string Mjd2Ut(double mjd, StatusType *status);
    extern std::string Mjd2UtNoSeconds(double mjd, StatusType *status);

    extern void Mjd2Tm(double mjd, bool local, struct tm *tm_p,
                StatusType * status);

    /*
     * Splits a string into substrings, according to the given delimiter.
     * Returns a vector of strings, with each element being a token from the original string.
     */
    std::vector<std::string> &split(const std::string &input, char delimiter, std::vector<std::string> &tokenVector);

    std::string toUpperCase(const char* s);
    std::string toUpperCase(std::string str);


    // Define a constant to represent an undefined value.
//    const int UNDEFINED_I = std::numeric_limits<int>::min();
    const int UNDEFINED_F = std::numeric_limits<float>::min();

} // namespace SAMI



#endif
