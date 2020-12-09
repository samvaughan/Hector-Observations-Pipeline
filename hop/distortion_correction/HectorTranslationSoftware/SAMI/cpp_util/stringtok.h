#ifndef STRINGTOK_INC
#define STRINGTOK_INC

/*
 * "@(#) $Id: ACMM:cpp_util/stringtok.h,v 1.6 03-Nov-2016 08:24:43+11 tjf $"
 *
 * stringtok.h
 *
 * stringtok is a replacment for C's strtok() function.  It uses a
 * STL container (any with push_back() should do).
 *
 * This was sourced from
 *     http://gcc.gnu.org/onlinedocs/libstdc++/21_strings/stringtok_h.txt
 *
 * on 25/Jun/03.
 *
 * After following a referrence on 
 *     http://gcc.gnu.org/onlinedocs/libstdc++/21_strings/howto.html#3
 *
 *   indicating it was suggested by Chris King and tweaked by Petr Prikryl.
 *
 *  There is apparently no copyright notice associated with it.
 *
 * History:
 *     12-Aug-2006 - TJF - Extracted from 2dFctrlTask/tdfct.h to this file.
 */
#include <string>
#include <vector>
namespace  aao_cpp_util {

    template <typename Container>
    void
    stringtok (Container &container, std::string const &in,
               const char * const delimiters = " \t\n\r")
    {
        const std::string::size_type len = in.length();
        std::string::size_type i = 0;
        
        while ( i < len )
        {
            // eat leading whitespace
            i = in.find_first_not_of (delimiters, i);
            if (i == std::string::npos)
                return;   // nothing left but white space
            
            // find the end of the token
            std::string::size_type j = in.find_first_of (delimiters, i);
            
            // push token
            if (j == std::string::npos) {
                container.push_back (in.substr(i));
                return;
            } else
                container.push_back (in.substr(i, j-i));
            
            // set up for next loop
            i = j + 1;
        }

    }

} // namespace aao_cpp_util
#endif /* !def STRINGTOK_INC */
