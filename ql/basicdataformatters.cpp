
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
#include <ql/Utilities/dataformatters.hpp>
#include <ql/Utilities/strings.hpp>

namespace QuantLib {

    #ifndef QL_DISABLE_DEPRECATED
    std::string IntegerFormatter::toString(BigInteger l, Integer digits) {
        std::ostringstream out;
        out << std::setw(digits) << io::checknull(l);
        return out.str();
    }

    std::string IntegerFormatter::toPowerOfTwo(BigInteger l, Integer digits) {
        std::ostringstream out;
        out << std::setw(digits) << io::power_of_two(l);
        return out.str();
    }
    #endif

    #ifndef QL_DISABLE_DEPRECATED
    std::string SizeFormatter::toString(Size l, Integer digits) {
        std::ostringstream out;
        out << std::setw(digits) << io::checknull(l);
        return out.str();
    }

    std::string SizeFormatter::toOrdinal(Size l) {
        std::ostringstream out;
        out << io::ordinal(l);
        return out.str();
    }

    std::string SizeFormatter::toPowerOfTwo(Size l, Integer digits) {
        std::ostringstream out;
        out << std::setw(digits) << io::power_of_two(l);
        return out.str();
    }
    #endif

    #ifndef QL_DISABLE_DEPRECATED
    std::string DecimalFormatter::toString(Decimal x, Integer precision,
                                           Integer digits) {
        std::ostringstream out;
        out << std::fixed << std::setprecision(precision);
        out << std::setw(digits) << io::checknull(x);
        return out.str();
    }

    std::string DecimalFormatter::toExponential(Decimal x, Integer precision,
                                                Integer digits) {
        std::ostringstream out;
        out << std::scientific << std::setprecision(precision);
        out << std::setw(digits) << io::checknull(x);
        return out.str();
    }

    std::string DecimalFormatter::toPercentage(Decimal x, Integer precision,
                                               Integer digits) {
        std::ostringstream out;
        out << std::setw(digits) << std::setprecision(precision);
        out << io::checknull(x);
        return out.str();
    }
    #endif

    #ifndef QL_DISABLE_DEPRECATED
    std::string StringFormatter::toLowercase(const std::string& s) {
        return lowercase(s);
    }

    std::string StringFormatter::toUppercase(const std::string& s) {
        return uppercase(s);
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
