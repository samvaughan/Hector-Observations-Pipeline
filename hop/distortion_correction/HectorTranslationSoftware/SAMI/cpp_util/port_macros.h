#ifndef PORT_MACROS_INC
#define PORT_MACROS_INC
/*
 * "@(#) $Id: ACMM:cpp_util/port_macros.h,v 1.6 03-Nov-2016 08:24:43+11 tjf $"
 *
 * Portability macros required by the Imperfect C++ book stuff.  This started
 * from stuff publiced in Imperfect C++ and by Dr. Dobbs.  I then modified
 * it for GCC 3.4 + and Solaris C++.
 *
 * Author: Tony Farrell, AAO (from Imperfect C++)
 *
 * History:
 *    18-May-2006 - TJF - Original version and mods for GCC 3.4+ and 
 *                         Solaris C++.
 *    11-Sep-2006 - TJF - Added PORT_MACROS_INC def.
 *    03-Mar-2012 - TJF - Support CLANG compiler, now being used on 
 *                        MAC OS X, through the feature used gives
 *                        warning about being a C++11 feature.
 *    07-Jul-2015 - TJF - Add PMDIAG series of macros.  In particular,
 *                         PMDIAG_CPP11_EXT_OFF and PMDIAG_CPP11_EXT_ON
 *                         used to stop warnings under CLang about using
 *                         a C++11 extension.
 *
 */ 


/*
 * Define the macros  PMDIAG_ON, PMDIAG_OFF.  These can be
 * use to turn GCC warnings on/off around a bit of code.  E.g.
 *   PMDIAG_OFF(sign-compare)
 *   if(a < b){
 *   PMDIAG_ON(sign-compare)
 *
 * These make use of the "#pragma GCC diagnostic" feature.  Part of
 *  this was  (enabling/disabling) was introduced from GCC version 4.2,
 *  whilst the push/pop feature was introduced from GCC version 4.6
 *
 *Based on code on
 *     http://dbp-consulting.com/tutorials/SuppressingGCCWarnings.html
 *  as at 4-Jan-2013, by  Patrick Horgan. Macro attributed
 *  to "Jonathan Wakely"
 *
 * Modified (1-June-2015) to explicly Apple's version of Clang, which
 * has the __apple_build_version__ macro defined and has
 * the value of it as 5030040 (for MacOsX Mavericks).  If this is not
 * defined, then Apple CLang indicates it is GCC 4.2 and the macros
 * for that does work.
 *
 * Based on DramaConfig/drama.h, version 5.7.
 */


#if defined(__apple_build_version__) && (__apple_build_version__ >= 5030040)
#define PMDIAG_STR(s) #s
#define PMDIAG_JOINSTR(x,y) PMDIAG_STR(x ## y)
#define PMDIAG_DO_PRAGMA(x) _Pragma (#x)
#define PMDIAG_PRAGMA(x) PMDIAG_DO_PRAGMA(GCC diagnostic x)
#define PMDIAG_OFF(x) PMDIAG_PRAGMA(push) \
        PMDIAG_PRAGMA(ignored PMDIAG_JOINSTR(-W,x))
#define PMDIAG_ON(x) PMDIAG_PRAGMA(pop)

#elif ((__GNUC__ * 100) + __GNUC_MINOR__) >= 402
#define PMDIAG_STR(s) #s
#define PMDIAG_JOINSTR(x,y) PMDIAG_STR(x ## y)
# define PMDIAG_DO_PRAGMA(x) _Pragma (#x)
# define PMDIAG_PRAGMA(x) PMDIAG_DO_PRAGMA(GCC diagnostic x)
# if ((__GNUC__ * 100) + __GNUC_MINOR__) >= 406
#  define PMDIAG_OFF(x) PMDIAG_PRAGMA(push) \
        PMDIAG_PRAGMA(ignored PMDIAG_JOINSTR(-W,x))
#  define PMDIAG_ON(x) PMDIAG_PRAGMA(pop)
# else
#  define PMDIAG_OFF(x) PMDIAG_PRAGMA(ignored PMDIAG_JOINSTR(-W,x))
#  define PMDIAG_ON(x)  PMDIAG_PRAGMA(warning PMDIAG_JOINSTR(-W,x))
# endif /* GCC version not greater/equal to 4.6 */
#else
# define PMDIAG_OFF(x)
# define PMDIAG_ON(x)
#endif /* GCC version not greater/equal to 4.2 */

/*
 * The c++11-extensions warning only apples to CLang, not GCC.  
 */
#if defined(__clang__)
# define PMDIAG_CPP11_EXT_OFF PMDIAG_OFF(c++11-extensions) 
# define PMDIAG_CPP11_EXT_ON  PMDIAG_ON(c++11-extensions) 
#else
# define PMDIAG_CPP11_EXT_OFF /* */
# define PMDIAG_CPP11_EXT_ON  /* */
#endif





/**  DECLARE_TEMPLATE_PARAM_AS_FRIEND
 *
 *  The specified class is a template parameter, and is made a friend 
 *  of the class in which this declaration is made.  Note that this is not 
 *  really supported by the C++ standard, but there are work-arounds.
 */
#if defined(__BORLANDC__) || \
    defined(__COMO__) || \
    defined(__DMC__) || \
    (   defined(__GNUC__) && \
        __GNUC__ < 3) || \
    defined(__INTEL_COMPILER) || \
    defined(__WATCOMC__) || \
    defined(_MSC_VER)
/*
 * This often works.
 */
# define    DECLARE_TEMPLATE_PARAM_AS_FRIEND(T)     friend T
#elif defined(__MWERKS__)
/*
 * Metorworks apparently accepts this one.
 */
# define    DECLARE_TEMPLATE_PARAM_AS_FRIEND(T)     friend class T

#elif defined(__clang__)
/*
 * CLANG compiler - now used by Apple on OsX.  Beware that it also
 * defines macros for GNU C
 */
# define    DECLARE_TEMPLATE_PARAM_AS_FRIEND(T)     friend T

#elif defined(__GNUC__) && \
      __GNUC__ == 3 && \
      __GNUC_MINOR__ < 4
/*
 * GCC 3, prior to 3.4 (I think).
 */
# define    DECLARE_TEMPLATE_PARAM_AS_FRIEND(T)     \
                                                    \
    struct friend_maker                             \
    {                                               \
        typedef T T2;                               \
    };                                              \
                                                    \
    typedef typename friend_maker::T2 friend_type;  \
                                                    \
    friend friend_type

#elif defined(__GNUC__) && __GNUC__ == 3 && __GNUC_MINOR__ >= 4
/*
 *  GCC 3 series, 3.4 and later.
 */
//#error "GCC 3 versions, .4 onwards - no defintion of DECLARE_TEMPLATE_PARAM_AS_FRIEND as yet"
# define    DECLARE_TEMPLATE_PARAM_AS_FRIEND(T)     \
                                                    \
    struct friend_maker                             \
    {                                               \
        typedef T T2;                               \
    };                                              \
                                                    \
    friend class friend_maker::T2
#elif defined(__GNUC__) && __GNUC__ >= 4
/*
 *  GCC 4 series.  This may actually work for 3.4, but I 
 *  Don't yet know.  The basic issue is that we can no 
 *  longer have a typedef in a friend declaration.  And
 *  we must have "friend class" rather then just "friend"
 */
# define    DECLARE_TEMPLATE_PARAM_AS_FRIEND(T)     \
                                                    \
    struct friend_maker                             \
    {                                               \
        typedef T T2;                               \
    };                                              \
                                                    \
    friend class friend_maker::T2

#elif defined(sun)
/*
 * This seems ok on Solaris.
 */
# define    DECLARE_TEMPLATE_PARAM_AS_FRIEND(T)     friend T
#else
#error "Unexpected C++ compiler - needs a defintion for DECLARE_TEMPLATE_PARAM_AS_FRIEND macro"

#endif /* compiler */

#endif /* !defined PORT_MACROS */
