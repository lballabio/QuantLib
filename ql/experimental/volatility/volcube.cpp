/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Ferdinando Ametrano

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/experimental/volatility/volcube.hpp>
#include <ql/experimental/volatility/abcdatmvolcurve.hpp>
#include <ql/experimental/volatility/interestratevolsurface.hpp>

namespace QuantLib {

    VolatilityCube::VolatilityCube(
                const std::vector<Handle<InterestRateVolSurface> >& surfaces,
                const std::vector<Handle<AbcdAtmVolCurve> >& curves)
    : surfaces_(surfaces), curves_(curves)
    {
        QL_REQUIRE(surfaces_.size()>1, "at least 2 surfaces are needed");

        Date refDate = surfaces_[0]->referenceDate();
        for (Size i=0; i<surfaces_.size(); ++i) {
            QL_REQUIRE(surfaces_[i]->referenceDate() == refDate,
                       "different reference dates");
            //curves_.push_back(surfaces_[i]);
        }

        for (Size i=0; i<curves_.size(); ++i) {
            QL_REQUIRE(curves_[i]->referenceDate() == refDate,
                       "different reference dates");
        }

        // sort increasing index tenor
    }

}
