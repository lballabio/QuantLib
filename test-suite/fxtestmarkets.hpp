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

/*! \file fxtestmarkets.hpp
    \brief FX markets from real quotes, and the curve bootstrap they need, for the tests
*/

#ifndef quantlib_test_fx_test_markets_hpp
#define quantlib_test_fx_test_markets_hpp

#include <ql/experimental/fxslv/fxpairmarket.hpp>
#include <ql/indexes/iborindex.hpp>
#include <vector>

namespace QuantLib {

    //! Quotes of a collateral curve: an overnight fixing and OIS par rates
    struct OisQuotes {
        ext::shared_ptr<OvernightIndex> index;  //!< without a curve
        Rate fixing;                            //!< today's overnight rate
        std::vector<Period> tenors;             //!< from the spot date, T+2
        std::vector<Rate> rates;                //!< par rates of annual-paying OIS
    };

    //! Spot and forward points of a pair, in units of the spot rate (not pips)
    /*! ON runs from today to the next business day, TN from there to
        spot and SN from spot to the business day after; the outright
        tenors start at spot.  Each quote is the outright at the later
        date minus the outright at the earlier one.
    */
    struct FxForwardQuotes {
        Real spot;
        Real on, tn, sn;
        std::vector<Period> tenors;
        std::vector<Real> points;
    };

    //! The curve of an OIS index: an overnight deposit and OIS from spot
    /*! The OIS settle T+2 on the index's calendar, pay annually with a
        lag of 2 days, and roll ModifiedFollowing.  The curve is
        log-linear in discount factors, Actual/365 (Fixed) from \p asOf,
        which must be the evaluation date.
    */
    ext::shared_ptr<YieldTermStructure> bootstrapOisCurve(const Date& asOf,
                                                          const OisQuotes& quotes);

    //! The curve of a pair's other currency implied by its forward points
    /*! Collateralised in the currency of \p collateralCurve, which is the
        pair's base currency if \p collateralIsBase.  ON and TN give the
        curve from \p asOf, which must be the evaluation date; outright
        tenors roll ModifiedFollowing, end of month, on the settlement
        calendar.  Log-linear in discount factors, Actual/365 (Fixed).
    */
    ext::shared_ptr<YieldTermStructure>
    bootstrapFxImpliedCurve(const Date& asOf,
                            const FxSettleConvention& settlement,
                            const FxForwardQuotes& quotes,
                            bool collateralIsBase,
                            const Handle<YieldTermStructure>& collateralCurve);

    //! A pair market built from real quotes, with the quotes it was built from
    struct FxTestMarket {
        FxPairMarket market;
        FxForwardQuotes forwards;
        OisQuotes ois;
    };

    //! EUR/USD on 2 January 2024, USD curve from SOFR OIS
    /*! Spot deltas, not premium-adjusted, ATM delta-neutral straddle,
        smile strangles, NY 10:00 cut, TARGET and US calendars.  Sets
        the evaluation date to the as-of date.
    */
    FxTestMarket eurusdJan2024(const FxVolSurfaceConfig& config = FxVolSurfaceConfig());

    //! USD/ZAR on 25 March 2020, USD curve from Fed Funds OIS
    /*! Premium-adjusted spot deltas, ATM delta-neutral straddle, smile
        strangles, NY 10:00 cut, South African and US calendars.  Sets
        the evaluation date to the as-of date.
    */
    FxTestMarket usdzarMar2020(const FxVolSurfaceConfig& config = FxVolSurfaceConfig());

}

#endif
