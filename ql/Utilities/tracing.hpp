
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

#include <ql/qldefines.hpp>
#include <boost/noncopyable.hpp>
#include <iosfwd>

namespace QuantLib {

    //! placeholder for tracing level enumeration
    struct Tracing {
        //! trace level
        enum Level { Severe,   //!< Trace a possible error
                     Warning,  //!< Trace something fishy
                     Info,     //!< Generic tracing information
                     Fine,     //!< Detailed tracing information
                     Finer,    //!< Detailed tracing information
                     Finest,   //!< Detailed tracing information
                     All       //!< Trace everything
        };
    };

    //! tracing class
    /*! \test the facility is tested by comparing actual output
              against expected traces.
    */
    class Tracer : public boost::noncopyable {
        friend class Settings;
      private:
        Tracer() {}
      public:
        //! \name Modifiers
        //@{
        void enable() { enabled_ = true; }
        void disable() { enabled_ = false; }
        void setLevel(Tracing::Level level) { level_ = level; }
        void setStream(std::ostream& stream) { out_ = &stream; }
        //@}
        /*! \name Inspectors
            \warning Do not use these methods directly; use the
                     QL_TRACE macro instead
        */
        //@{
        bool enabled() const { return enabled_; }
        Tracing::Level level() const { return level_; }
        std::ostream& stream() { return *out_; }
        //@}
      private:
        std::ostream* out_;
        Tracing::Level level_;
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
    current settings (see the Settings class.)
*/

/*! @} */

/*! @} */

#if defined(QL_ENABLE_TRACING)

#define QL_DEFAULT_TRACER   QuantLib::Settings::instance().tracing()

#define QL_TRACE(traceLevel,message) \
if (QL_DEFAULT_TRACER.enabled() && traceLevel <= QL_DEFAULT_TRACER.level()) \
    try { \
        QL_DEFAULT_TRACER.stream() << message << std::endl; \
    } catch (...) {} \
else

#else

#define QL_TRACE(traceLevel,message)

#endif

#endif
