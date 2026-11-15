/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2026 Raghav Subbarao

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <https://www.quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/experimental/fxslv/fxforwardcurve.hpp>
#include <ql/math/interpolations/loginterpolation.hpp>
#include <ql/quotes/simplequote.hpp>
#include <ql/settings.hpp>
#include <ql/termstructures/yield/piecewiseyieldcurve.hpp>
#include <ql/termstructures/yield/ratehelpers.hpp>
#include <ql/time/daycounters/actual365fixed.hpp>

namespace QuantLib {

    ext::shared_ptr<YieldTermStructure>
    bootstrapFxCurve(const Date& asOf,
                     const FxPairConventions& conventions,
                     Real spot,
                     const std::vector<Real>& forwardPoints,
                     const std::string& collateralCurrency,
                     const Handle<YieldTermStructure>& collateralCurve) {
        const std::vector<Period>& tenors = conventions.forwardTenors();
        QL_REQUIRE(!tenors.empty(), conventions.name() << ": no forward tenors to bootstrap from");
        QL_REQUIRE(forwardPoints.size() == tenors.size(),
                   conventions.name() << ": " << forwardPoints.size()
                                      << " forward points given for " << tenors.size()
                                      << " forward tenors");
        QL_REQUIRE(collateralCurrency == conventions.foreignCurrency() ||
                       collateralCurrency == conventions.domesticCurrency(),
                   "collateral currency " << collateralCurrency << " is not a currency of "
                                          << conventions.name());
        QL_REQUIRE(!collateralCurve.empty(), "no " << collateralCurrency << " curve given");
        // the helpers date their forwards from the evaluation date
        QL_REQUIRE(Settings::instance().evaluationDate() == asOf,
                   "the evaluation date (" << Settings::instance().evaluationDate()
                                           << ") must be the as-of date (" << asOf
                                           << ") to bootstrap the curve");

        const Handle<Quote> spotQuote = makeQuoteHandle(spot);
        const bool collateralIsForeign = collateralCurrency == conventions.foreignCurrency();
        const FxSettleConvention& settlement = conventions.settlement();
        std::vector<ext::shared_ptr<RateHelper>> helpers;
        helpers.reserve(tenors.size());
        for (Size i = 0; i < tenors.size(); ++i)
            helpers.push_back(ext::make_shared<FxSwapRateHelper>(
                makeQuoteHandle(forwardPoints[i]), spotQuote, tenors[i], settlement.spotLag(),
                settlement.calendar(), Following, false, collateralIsForeign, collateralCurve));

        auto curve =
            ext::make_shared<PiecewiseYieldCurve<Discount, LogLinear>>(asOf, helpers,
                                                                      Actual365Fixed());
        curve->enableExtrapolation();
        // bootstrap now, so that bad points fail here rather than at first use
        curve->discount(curve->maxDate());
        return curve;
    }

}
