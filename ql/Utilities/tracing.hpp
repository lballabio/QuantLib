
/*
 Copyright (C) 2005 StatPro Italia srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email quantlib-dev@lists.sf.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file ql/Utilities/tracing.hpp
    \brief tracing facilities
*/

#ifndef quantlib_tracing_hpp
#define quantlib_tracing_hpp

#include <ql/Patterns/singleton.hpp>
#include <boost/current_function.hpp>
#include <iosfwd>

namespace QuantLib {

    /*! \example tracing_example.cpp
        This code exemplifies how to insert trace statements to follow
        the flow of program execution. When compiler under gcc 3.3 and
        run, the following program will output the following trace:
        \code
        Entering int main()
        Entering int Foo::bar(int)
        i = 21
        Currently at line 16 in tracing_example.cpp
        Wrong answer
        i = 42
        Exiting int Foo::bar(int)
        Entering int Foo::bar(int)
        i = 42
        Currently at line 13 in tracing_example.cpp
        Right answer, but no question
        i = 42
        Exiting int Foo::bar(int)
        Exiting int main()
        \endcode
    */

    //! tracing class
    /*! <b>Example: </b>
        \link tracing_example.cpp
        tracing code execution
        \endlink

        \test the facility is tested by comparing actual output
              against expected traces.
    */
    class Tracing : public Singleton<Tracing> {
        friend class Singleton<Tracing>;
      private:
        Tracing();
      public:
        //! trace level
        enum Level { Severe,   //!< Trace a possible error
                     Warning,  //!< Trace something fishy
                     Info,     //!< Generic tracing information
                     Fine,     //!< Detailed tracing information
                     Finer,    //!< Detailed tracing information
                     Finest,   //!< Detailed tracing information
                     All       //!< Trace everything
        };
        //! \name Modifiers
        //@{
        void enable() { enabled_ = true; }
        void disable() { enabled_ = false; }
        void setLevel(Level level) { level_ = level; }
        void setStream(std::ostream& stream) { out_ = &stream; }
        //@}
        /*! \name Inspectors
            \warning Do not use these methods directly; use the
                     provided QL_TRACE* macros instead
        */
        //@{
        bool enabled() const { return enabled_; }
        Level level() const { return level_; }
        std::ostream& stream() { return *out_; }
        //@}
      private:
        std::ostream* out_;
        Level level_;
        bool enabled_;
    };

}

/*! \addtogroup macros
    @{
*/

/*! \defgroup debugMacros Debugging macros

    For debugging purposes, macros can be used to output information
    about the code being executed.

    @{
*/

/*! \def QL_TRACE
    \brief output tracing information

    The statement
    \code
    QL_TRACE(level, message);
    \endcode
    can be used to output a trace of the code being executed. If
    tracing was disabled during configuration, such statements are
    removed by the preprocessor for maximum performance; if it was
    enabled, whether and where the message is output depends on the
    current settings. See \link tracing_example.cpp here \endlink
    for an example of usage.
*/

/*! \def QL_TRACE_ENTER_FUNCTION
    \brief output tracing information

    The statement
    \code
    QL_TRACE_ENTER_FUNCTION(level);
    \endcode
    can be used at the beginning of a function to trace the fact that
    the program execution is entering such function. Such statement
    might be ignored; refer to QL_TRACE for details. Also, function
    information might not be available depending on the compiler. See
    \link tracing_example.cpp here \endlink for an example of usage.
*/

/*! \def QL_TRACE_EXIT_FUNCTION
    \brief output tracing information

    The statement
    \code
    QL_TRACE_EXIT_FUNCTION(level);
    \endcode
    can be used before returning from a function to trace the fact
    that the program execution is exiting such function. Such
    statement might be ignored; refer to QL_TRACE for details. Also,
    function information might not be available depending on the
    compiler. See \link tracing_example.cpp here \endlink for an
    example of usage.
*/

/*! \def QL_TRACE_LOCATION
    \brief output tracing information

    The statement
    \code
    QL_TRACE_LOCATION(level);
    \endcode
    can be used to trace the current file and line. Such statement
    might be ignored; refer to QL_TRACE for details. See
    \link tracing_example.cpp here \endlink for an example of usage.
*/

/*! \def QL_TRACE_VARIABLE
    \brief output tracing information

    The statement
    \code
    QL_TRACE_VARIABLE(level, variable);
    \endcode
    can be used to trace the current value of a variable. Such
    statement might be ignored; refer to QL_TRACE for details. Also,
    the variable type must allow sending it to an output stream. See
    \link tracing_example.cpp here \endlink for an example of usage.
*/

/*! @} */

/*! @} */

#if defined(QL_ENABLE_TRACING)

#define QL_DEFAULT_TRACER   QuantLib::Tracing::instance()

#define QL_TRACE(traceLevel,message) \
if (QL_DEFAULT_TRACER.enabled() && traceLevel <= QL_DEFAULT_TRACER.level()) \
    try { \
        QL_DEFAULT_TRACER.stream() << message << std::endl; \
    } catch (...) {} \
else

#define QL_TRACE_ENTER_FUNCTION(traceLevel) \
QL_TRACE(traceLevel, "Entering " << BOOST_CURRENT_FUNCTION)

#define QL_TRACE_EXIT_FUNCTION(traceLevel) \
QL_TRACE(traceLevel, "Exiting " << BOOST_CURRENT_FUNCTION)

#define QL_TRACE_LOCATION(traceLevel) \
QL_TRACE(traceLevel, "Currently at line " << __LINE__ << " in " << __FILE__)

#define QL_TRACE_VARIABLE(traceLevel, variable) \
QL_TRACE(traceLevel, #variable << " = " << variable)

#else

#define QL_DEFAULT_TRACER
#define QL_TRACE(traceLevel,message)
#define QL_TRACE_ENTER_FUNCTION(traceLevel)
#define QL_TRACE_EXIT_FUNCTION(traceLevel)
#define QL_TRACE_LOCATION(traceLevel)
#define QL_TRACE_VARIABLE(traceLevel,variable)

#endif

#endif
