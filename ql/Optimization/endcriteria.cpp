/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006, 2007 Ferdinando Ametrano
 Copyright (C) 2001, 2002, 2003 Nicolas Di Césaré

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/reference/license.html>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/Optimization/endcriteria.hpp>
#include <ql/errors.hpp>

namespace QuantLib {

    EndCriteria::EndCriteria(Size maxIterations,
                             Real functionEpsilon,
                             Real gradientEpsilon,
                             Size maxStationaryStateIterations)
    : maxIterations_(maxIterations),
      functionEpsilon_(functionEpsilon),
      gradientEpsilon_(gradientEpsilon),
      maxStationaryStateIterations_(maxStationaryStateIterations) {

        if (maxStationaryStateIterations_ == Null<Size>())
            maxStationaryStateIterations_ = std::min(Size(maxIterations/10.0),
                                                     Size(1000));

        if (gradientEpsilon_ == Null<Real>())
            gradientEpsilon_ = functionEpsilon_;
    }

    std::ostream& operator<<(std::ostream& out,  EndCriteria::Type ec) {
        switch (ec) {
        case QuantLib::EndCriteria::None:
            return out << "None";
        case QuantLib::EndCriteria::MaxIterations:
            return out << "MaxIterations";
        case QuantLib::EndCriteria::StationaryPoint:
            return out << "StationaryPoint";
        case QuantLib::EndCriteria::StationaryGradient:
            return out << "StationaryGradient";
        case QuantLib::EndCriteria::Unknown:
            return out << "Unknown";
        default:
            QL_FAIL("unknown EndCriteria::Type (" << Integer(ec) << ")");
        }
    }

}
