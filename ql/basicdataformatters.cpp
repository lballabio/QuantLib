
/*
 Copyright (C) 2003, 2004 Ferdinando Ametrano
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

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

/*! \file basicdataformatters.cpp
    \brief classes used to format basic types for output
*/

#include <ql/basicdataformatters.hpp>
#include <ql/null.hpp>
#include <iomanip>

namespace QuantLib {

    #ifndef QL_DISABLE_DEPRECATED
    std::string IntegerFormatter::toString(BigInteger l, Integer digits) {
        static BigInteger null = Null<BigInteger>();
        static std::ostringstream out;
        if (l == null) {
            return std::string("null");
        } else {
            out.str("");
            out << std::setw(digits) << l;
            return out.str();
        }
    }

    std::string IntegerFormatter::toPowerOfTwo(BigInteger l, Integer digits) {
        if (l < 0L)
            return "-" + SizeFormatter::toPowerOfTwo(Size(-l),digits);
        else
            return SizeFormatter::toPowerOfTwo(Size(l),digits);
    }
    #endif

    #ifndef QL_DISABLE_DEPRECATED
    std::string SizeFormatter::toString(Size l, Integer digits) {
        static Size null = Null<Size>();
        static std::ostringstream out;
        if (l == null) {
            return std::string("null");
        } else {
            out.str("");
            out << std::setw(digits) << l;
            return out.str();
        }
    }

    std::string SizeFormatter::toOrdinal(Size l) {
        std::ostringstream s;
        s << l;
        if (l == Size(11) || l == Size(12) || l == Size(13)) {
            s << "th";
        } else {
            switch (l % 10) {
              case 1:  s << "st";  break;
              case 2:  s << "nd";  break;
              case 3:  s << "rd";  break;
              default: s << "th";
            }
        }
        return s.str();
    }

    std::string SizeFormatter::toPowerOfTwo(Size l, Integer digits) {
        static Size null = Null<Size>();
        if (l == null)
            return std::string("null");
        Integer power = 0;
        while (!(l & 1UL)) {
            power++;
            l >>= 1;
        }
        std::ostringstream s;
        s << std::setw(digits) << l << "*2^" << std::setw(2) << power;
        return s.str();
    }
    #endif

    #ifndef QL_DISABLE_DEPRECATED
    std::string DecimalFormatter::toString(Decimal x, Integer precision,
                                           Integer digits) {
        static Decimal null = Null<Decimal>();
        static std::ostringstream out;
        if (x == null) {
            return std::string("null");
        } else {
            out.str("");
            out << std::fixed;
            out << std::setw(digits) << std::setprecision(precision) << x;
            return out.str();
        }
    }

    std::string DecimalFormatter::toExponential(Decimal x, Integer precision,
                                                Integer digits) {
        static Decimal null = Null<Decimal>();
        static std::ostringstream out;
        if (x == null) {
            return std::string("null");
        } else {
            out.str("");
            out << std::scientific;
            out << std::setw(digits) << std::setprecision(precision) << x;
            return out.str();
        }
    }

    std::string DecimalFormatter::toPercentage(Decimal x, Integer precision,
                                               Integer digits) {
        static std::ostringstream out;
        out.str("");
        out << std::fixed;
        out << std::setw(digits) << std::setprecision(precision);
        out << x*100 << "%";
        return out.str();
    }
    #endif

    #ifndef QL_DISABLE_DEPRECATED
    std::string StringFormatter::toLowercase(const std::string& s) {
        std::string output = s;
        for (std::string::iterator i=output.begin(); i!=output.end(); i++)
            *i = std::tolower(*i);
        return output;
    }

    std::string StringFormatter::toUppercase(const std::string& s) {
        std::string output = s;
        for (std::string::iterator i=output.begin(); i!=output.end(); i++)
            *i = std::toupper(*i);
        return output;
    }
    #endif

    #ifndef QL_DISABLE_DEPRECATED
    std::string RateFormatter::toString(Rate rate, Integer precision) {
        return DecimalFormatter::toPercentage(rate,precision);
    }

    std::string VolatilityFormatter::toString(Volatility vol,
                                              Integer precision) {
        return DecimalFormatter::toPercentage(vol,precision);
    }
    #endif

}
