
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

namespace QuantLib {

    std::string IntegerFormatter::toString(BigInteger l, Integer digits) {
        static BigInteger null = Null<BigInteger>();
        if (l == null)
            return std::string("null");
        char s[64];
        QL_SPRINTF(s,"%*ld",(digits>64?64:digits),l);
        return std::string(s);
    }

    std::string IntegerFormatter::toPowerOfTwo(BigInteger l, Integer digits) {
        if (l < 0L)
            return "-" + SizeFormatter::toPowerOfTwo(Size(-l),digits);
        else
            return SizeFormatter::toPowerOfTwo(Size(l),digits);
    }

    std::string SizeFormatter::toString(Size l, Integer digits) {
        static Size null = Null<Size>();
        if (l == null)
            return std::string("null");
        char s[64];
        QL_SPRINTF(s,"%*lu",(digits>64?64:digits),l);
        return std::string(s);
    }

    std::string SizeFormatter::toOrdinal(Size l) {
        std::string suffix;
        if (l == Size(11) || l == Size(12) || l == Size(13)) {
            suffix = "th";
        } else {
            switch (l % 10) {
              case 1:  suffix = "st";  break;
              case 2:  suffix = "nd";  break;
              case 3:  suffix = "rd";  break;
              default: suffix = "th";
            }
        }
        return toString(l)+suffix;
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
        return toString(l,digits) + "*2^" + toString(power,2);
    }

    std::string DecimalFormatter::toString(Decimal x, Integer precision,
                                           Integer digits) {
        if (x == Null<Decimal>())
            return std::string("null");
        char s[64];
        QL_SPRINTF(s,"%*.*f",(digits>64?64:digits),
                             (precision>64?64:precision),x);
        return std::string(s);
    }

    std::string DecimalFormatter::toExponential(Decimal x, Integer precision,
                                                Integer digits) {
        if (x == Null<Decimal>())
            return std::string("null");
        char s[64];
        QL_SPRINTF(s,"%*.*e",(digits>64?64:digits),
                             (precision>64?64:precision),x);
        return std::string(s);
    }

    std::string DecimalFormatter::toPercentage(Decimal x, Integer precision,
                                               Integer digits) {
        return toString(x*100,precision,digits)+" %";
    }

    std::string StringFormatter::toLowercase(const std::string& s) {
        std::string output = s;
        for (std::string::iterator i=output.begin(); i!=output.end(); i++)
            *i = QL_TOLOWER(*i);
        return output;
    }

    std::string StringFormatter::toUppercase(const std::string& s) {
        std::string output = s;
        for (std::string::iterator i=output.begin(); i!=output.end(); i++)
            *i = QL_TOUPPER(*i);
        return output;
    }

}
