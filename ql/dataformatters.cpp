
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

/*! \file dataformatters.cpp
    \brief classes used to format data for output
*/

#include <ql/dataformatters.hpp>
#include <ql/null.hpp>

namespace QuantLib {

    #ifndef QL_DISABLE_DEPRECATED

    std::string EuroFormatter::toString(Decimal amount) {
        std::string output;
        if (amount < 0.0) {
            output = "-";
            amount = -amount;
        } else {
            output = " ";
        }
        Integer triples = 0;
        while (amount >= 1000.0) {
            amount /= 1000;
            triples++;
        }
        output += IntegerFormatter::toString(Integer(amount));
        amount -= Integer(amount);
        while (triples > 0) {
            amount *= 1000;
            output += ","+IntegerFormatter::toString(Integer(amount),3);
            amount -= Integer(amount);
            triples--;
        }
        amount *= 100;
        output += "."+IntegerFormatter::toString(Integer(amount+0.5),2);
        return output;
    }

    std::ostream& operator<<(std::ostream& stream, const Date& date) {
        return stream << DateFormatter::toString(date, DateFormatter::Short);
    }

    #ifndef QL_PATCH_MSVC6
    std::ostream& operator<<(std::ostream& stream, const Array& a) {
        return stream << ArrayFormatter::toString(a);
    }

    std::ostream& operator<<(std::ostream& stream, const Matrix& matrix) {
        return stream << MatrixFormatter::toString(matrix);
    }
    #endif

    #endif
}
