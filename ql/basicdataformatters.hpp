
/*
 Copyright (C) 2003, 2004 Ferdinando Ametrano
 Copyright (C) 2000-2004 StatPro Italia srl

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

/*! \file basicdataformatters.hpp
    \brief Classes used to format basic types for output
*/

#ifndef quantlib_basic_data_formatters_hpp
#define quantlib_basic_data_formatters_hpp

#include <ql/types.hpp>
#include <sstream>
#include <iomanip>

namespace QuantLib {

    #ifndef QL_DISABLE_DEPRECATED
    //! Formats integers for output
    /*! \deprecated use streams and manipulators for proper formatting */
    class IntegerFormatter {
      public:
        static std::string toString(BigInteger l, Integer digits = 0);
        static std::string toPowerOfTwo(BigInteger l, Integer digits = 0);
    };
    #endif

    #ifndef QL_DISABLE_DEPRECATED
    //! Formats unsigned integers for output
    /*! \deprecated use streams and manipulators for proper formatting */
    class SizeFormatter {
      public:
        static std::string toString(Size l, Integer digits = 0);
        static std::string toOrdinal(Size l);
        static std::string toPowerOfTwo(Size l, Integer digits = 0);
    };
    #endif

    #ifndef QL_DISABLE_DEPRECATED
    //! Formats real numbers for output
    /*! \deprecated use streams and manipulators for proper formatting */
    class DecimalFormatter {
      public:
        static std::string toString(Decimal x,
                                    Integer precision = 6,
                                    Integer digits = 0);
        /*! \deprecated use streams and manipulators for proper formatting */
        static std::string toExponential(Decimal x,
                                         Integer precision = 6,
                                         Integer digits = 0);
        static std::string toPercentage(Decimal x,
                                        Integer precision = 6,
                                        Integer digits = 0);
    };
    #endif

    //! Formats strings as lower- or uppercase
    class StringFormatter {
      public:
        static std::string toLowercase(const std::string& s);
        static std::string toUppercase(const std::string& s);
    };

    #ifndef QL_PATCH_MSVC6
    //! Formats numeric sequences for output
    class SequenceFormatter {
      public:
        template<class Iterator>
        static std::string toString(Iterator begin,
                                    Iterator end,
                                    Integer precision = 6,
                                    Integer digits = 0,
                                    Size elementsPerRow = QL_MAX_INTEGER) {
            std::ostringstream s;
            s.precision(precision);
            s << "[ ";
            for (Size n=0; begin!=end; ++begin, ++n) {
                if (n == elementsPerRow) {
                    s << ";\n  ";
                    n = 0;
                }
                if (n!=0)
                    s << " ; ";
                s << std::setw(digits) << *begin;
            }
            s << " ]";
            return s.str();
        }
    };
    #endif

    #ifndef QL_DISABLE_DEPRECATED
    //! Formats rates for output
    /*! \deprecated use streams and manipulators for proper formatting */
    class RateFormatter {
      public:
        static std::string toString(Rate rate,
                                    Integer precision = 5);
    };
    #endif

    #ifndef QL_DISABLE_DEPRECATED
    //! Formats volatilities for output
    /*! \deprecated use streams and manipulators for proper formatting */
    class VolatilityFormatter {
      public:
        static std::string toString(Volatility vol,
                                    Integer precision = 5);
    };
    #endif

}


#endif
