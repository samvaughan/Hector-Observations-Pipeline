#ifndef SAMI_EXCEPT_INC
#define SAMI_EXCEPT_INC
/*                                  s a m i _ e x c e p t i o n s

 * Module name:
      sami_exceptions.h

 * Function:
	Include file that defines the SAMI file reader exceptions.
      

 * Description:
        This C++ incude file declares the SAMI file reader exceptions and
        releated classes.

        SAMI is an AAT Instrument which implements a number of IFU
        probes on the AAT Prime Focus.
      

 * Language:
      C++

 * Support: Tony Farrell, AAO

 * History:
      19-May-2011 - TJF - Start history.


 * "@(#) $Id: ACMM:sami/sami_exceptions.h,v 1.29 23-Feb-2016 12:26:12+11 tjf $"

 */

#include <string>
#include <sstream>
#include "status.h"
#include "sami_err.h"

// This is found in the SAMI namespace
namespace SAMI {


    // A SAMI exception - we allow the SAMI code too raise exceptions,
    // but must allow these exceptions to be converted to DRAMA Status
    // and ERS calls.  
    class S_Exception  {
        std::string _name;
        std::string _text;
        StatusType _status;
        std::string _extraText;
    protected:
        // Allows the inheritor to change the text, which is usefull
        // in cases where the sub-class needs to build up the string.
        void SetText(const std::string &newText) {
            _text = newText;
        }
        // Each sub-class should have one of these which returns its name.
        virtual std::string  GetName() const { return "SAMI::S_Exception"; }
    public:
        // Create the exception.
        S_Exception(std::string text , StatusType status) :
            _text(text), _status(status) { }

        // Report the exception using ERS and set status to the
        // bad value.
        void Report(StatusType *status) const;
        // Report to cerr.
        void Report() const;

        // Return the DRAMA status associated with this exception.
        StatusType GetStatus() const {
            return _status;
        }
        /*
         * Return the three strings associated with an exception, the
         * name of the exception, the main exception text and the
         * extra text;
         */
        void GetStrings(std::string * name,
                        std::string * text,
                        std::string * extraText) const;

        // Add extra text to be output with a message.  One line of extra
        //  information can be added to an exception using this operators,
        //  as long as sstreams supports the type being used.
        template<class T> S_Exception & operator << (const T &t) {
            std::ostringstream buf;
            buf << _extraText;
            buf << t;
            _extraText = buf.str();
            return *this; 
        }
            
        virtual ~S_Exception(){}
    private:
        
    };

    // Various parser generate exceptions.

    // ParserException is the base of the rest.
    class ParserException : public S_Exception{
    private:
        int _line;
        std::string _value;
    protected:
        virtual std::string GetName() const { return "SAMI::ParserException"; }
    public:
        ParserException(int line, const std::string & value);
    };

    // We sub-class ParserException for various other cases.
    class HeaderParseException : public ParserException { 
    protected:
        std::string GetName() const { return "SAMI::HeaderParserException"; }
    public:
        HeaderParseException(int line, const std::string &value):
            ParserException(line, value) { };
    };

    class DateParseException : public ParserException  { 
    public:
        DateParseException(const std::string &value):
            ParserException(-1, value) { };
    };
    class YearException : public ParserException  { 
    private:
        int _year;
    protected:
        std::string GetName() const { return "SAMI::YearException"; }
    public:
        YearException(int year, const std::string &value):
            ParserException(-1, value),_year(year) { };
    };

    class RealNumParseException : public ParserException {
    protected:
        std::string GetName() const { return "SAMI::RealNumException"; }
    public:
        RealNumParseException(int line, const std::string &value):
            ParserException(line, value) { };
    };
                
    class IntegerNumParseException : public ParserException {
    protected:
        std::string GetName() const { return "SAMI::IntegerNumException"; }
    public:
        IntegerNumParseException(int line, const std::string &value):
            ParserException(line, value) { };
    };
            
    class TimeParseException : public ParserException  { 
    protected:
        std::string GetName() const { return "SAMI::TimeParseException"; }
    public:
        TimeParseException(const std::string &value):
            ParserException(-1, value) { };
    };
    class RAParseException : public ParserException  { 
    protected:
        virtual std::string GetName() const { return "SAMI::RAParseException"; }
    public:
        RAParseException(int line, const std::string &value):
            ParserException(line, value) { };
    };

    class DecParseException : public ParserException  { 
    protected:
        std::string GetName() const { return "SAMI::DecParseException"; }
    public:
        DecParseException(int line, const std::string &value):
            ParserException(line, value) { };
    };

    class BodyParseException : public ParserException {
    protected:
        std::string GetName() const { return "SAMI::BodyParserException"; }
    public:
        BodyParseException(int line, const std::string &value):
            ParserException(line, value) { } ;
    };
    class ProbeNumParseException : public ParserException {
    protected:
        std::string GetName() const { return "SAMI::ProbeNumParseException"; }
    public:
        ProbeNumParseException(int line, const std::string &value):
            ParserException(line, value) { } ;
    };

    class ProbePosParseException : public ParserException {
    protected:
        std::string GetName() const { return "SAMI::ProbePosParseException"; }
    public:
        ProbePosParseException(int line, const std::string &value):
            ParserException(line, value) { } ;
    };


    class BundleConfigException : public S_Exception {
	public:
	    BundleConfigException(const std::string &s) :
		S_Exception(s, SAMI__BUNDLE) { }
    };

} // namespace SAMI


#endif
