
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


    std::ostream& operator<< (std::ostream& stream, const Array& a) {
        return stream << ArrayFormatter::toString(a.begin(), a.end());
    }

    std::ostream& operator<< (std::ostream& stream, const Matrix& matrix) {
        for (Size i=0; i<matrix.rows(); i++) {
            for (Size j=0; j<matrix.columns(); j++) {
                stream << DoubleFormatter::toString(matrix[i][j]) << " ";
            }
            stream << "\n";
        }
        return stream;
    }


}
