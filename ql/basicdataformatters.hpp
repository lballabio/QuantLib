
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

#include <ql/currency.hpp>
#include <ql/option.hpp>
#include <iostream>

namespace QuantLib {

    //! Formats integers for output
    class IntegerFormatter {
      public:
        static std::string toString(BigInteger l, Integer digits = 0);
        static std::string toPowerOfTwo(BigInteger l, Integer digits = 0);
    };

    //! Formats unsigned integers for output
    class SizeFormatter {
      public:
        static std::string toString(Size l, Integer digits = 0);
        static std::string toOrdinal(Size l);
        static std::string toPowerOfTwo(Size l, Integer digits = 0);
    };

    //! Formats real numbers for output
    class DecimalFormatter {
      public:
        static std::string toString(Decimal x,
                                    Integer precision = 6,
                                    Integer digits = 0);
        static std::string toExponential(Decimal x,
                                         Integer precision = 6,
                                         Integer digits = 0);
        static std::string toPercentage(Decimal x,
                                        Integer precision = 6,
                                        Integer digits = 0);
    };


#ifndef QL_DISABLE_DEPRECATED
    //! \deprecated Renamed to DecimalFormatter
    typedef DecimalFormatter DoubleFormatter;
#endif


    //! Formats amounts in Euro for output
    /*! Formatting follows Euro convention (x,xxx,xxx.xx) */
    class EuroFormatter {
      public:
        static std::string toString(Decimal amount);
    };

    //! Formats rates for output
    /*! Formatting is in percentage form (xx.xxxxx%) */
    class RateFormatter {
      public:
        static std::string toString(Rate rate,
                                    Integer precision = 5);
    };

    //! Formats volatilities for output
    /*! Formatting is in percentage form (xx.xxxxx%) */
    class VolatilityFormatter {
      public:
        static std::string toString(Volatility vol,
                                    Integer precision = 5);
    };


    //! Formats dates for output
    /*! Formatting can be in short (mm/dd/yyyy)
        or long (Month ddth, yyyy) form.
    */
    class DateFormatter {
      public:
        enum Format { Long, Short, ISO };
        static std::string toString(const Date& d,
                                    Format f = Long);
    };

    /*! \relates Date */
    std::ostream& operator<<(std::ostream&, const Date&);

    //! Formats currencies for output
    class CurrencyFormatter {
      public:
        static std::string toString(Currency c);
    };

    //! Formats strings as lower- or uppercase
    class StringFormatter {
      public:
        static std::string toLowercase(const std::string& s);
        static std::string toUppercase(const std::string& s);
    };

    //! Formats option type for output
    class OptionTypeFormatter {
      public:
        static std::string toString(Option::Type type);
    };

}


#endif
