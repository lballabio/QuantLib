/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Giorgio Facchinetti
 Copyright (C) 2010 Ferdinando Ametrano
 Copyright (C) 2026 Kyrylo Protsenko

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

#include <ql/cashflows/cashflows.hpp>
#include <ql/cashflows/floatingratecoupon.hpp>
#include <ql/indexes/iborindex.hpp>
#include <ql/instruments/capfloor.hpp>
#include <ql/math/solvers1d/brent.hpp>
#include <ql/quotes/simplequote.hpp>
#include <ql/termstructures/volatility/capfloor/capfloortermvolcurve.hpp>
#include <ql/termstructures/volatility/optionlet/optionletstripper1.hpp>
#include <ql/termstructures/volatility/optionlet/optionletstripper2.hpp>
#include <ql/termstructures/volatility/optionlet/spreadedoptionletvol.hpp>
#include <ql/termstructures/volatility/optionlet/strippedoptionletadapter.hpp>
#include <utility>


namespace QuantLib {

    OptionletStripper2::OptionletStripper2(
        const ext::shared_ptr<OptionletStripper1>& optionletStripper1,
        const Handle<CapFloorTermVolCurve>& atmCapFloorTermVolCurve)
    : OptionletStripper(optionletStripper1->termVolSurface(),
                        optionletStripper1->iborIndex(),
                        Handle<YieldTermStructure>(),
                        optionletStripper1->volatilityType(),
                        optionletStripper1->displacement(),
                        optionletStripper1->optionletFrequency(),
                        optionletStripper1->paymentLag()),
      stripper1_(optionletStripper1), atmCapFloorTermVolCurve_(atmCapFloorTermVolCurve),
      dc_(stripper1_->termVolSurface()->dayCounter()),
      nOptionExpiries_(atmCapFloorTermVolCurve->optionTenors().size()),
      atmCapFloorStrikes_(nOptionExpiries_), atmCapFloorPrices_(nOptionExpiries_),
      spreadsVolImplied_(nOptionExpiries_), caps_(nOptionExpiries_) {
        registerWith(stripper1_);
        registerWith(atmCapFloorTermVolCurve_);

        QL_REQUIRE(dc_ == atmCapFloorTermVolCurve->dayCounter(),
                   "different day counters provided");
    }

    void OptionletStripper2::performCalculations() const {

        //// optionletStripper data
        optionletDates_ = stripper1_->optionletFixingDates();
        optionletPaymentDates_ = stripper1_->optionletPaymentDates();
        optionletAccrualPeriods_ = stripper1_->optionletAccrualPeriods();
        optionletTimes_ = stripper1_->optionletFixingTimes();
        atmOptionletRate_ = stripper1_->atmOptionletRates();
        for (Size i=0; i<optionletTimes_.size(); ++i) {
            optionletStrikes_[i] = stripper1_->optionletStrikes(i);
            optionletVolatilities_[i] = stripper1_->optionletVolatilities(i);
        }

        // atmCapFloorTermVolCurve data
        const std::vector<Period>& optionExpiriesTenors =
                                    atmCapFloorTermVolCurve_->optionTenors();
        const std::vector<Time>& optionExpiriesTimes =
                                    atmCapFloorTermVolCurve_->optionTimes();
        std::vector<std::vector<Size>> capOptionletIndices(nOptionExpiries_);

        for (Size j=0; j<nOptionExpiries_; ++j) {
            Volatility atmOptionVol = atmCapFloorTermVolCurve_->volatility(
                optionExpiriesTimes[j], 33.3333); // dummy strike
            Handle<Quote> atmOptionVolHandle(
                ext::make_shared<SimpleQuote>(atmOptionVol));
            auto engine = makeCapFloorPricingEngine(
                iborIndex_->forwardingTermStructure(), atmOptionVolHandle);

            Leg leg = makeCapFloorLeg(optionExpiriesTenors[j]);
            if (isOvernightIndex()) {
                for (const auto& cashflow : leg) {
                    auto coupon = ext::dynamic_pointer_cast<FloatingRateCoupon>(cashflow);
                    QL_REQUIRE(coupon, "non-floating-rate coupon in cap/floor leg");
                    auto optionlet = std::find(
                        optionletDates_.begin(), optionletDates_.end(), coupon->fixingDate());
                    QL_REQUIRE(optionlet != optionletDates_.end(),
                               "ATM cap/floor fixing date " << coupon->fixingDate()
                                                             << " is not represented in the source "
                                                                "optionlet surface");
                    capOptionletIndices[j].push_back(optionlet - optionletDates_.begin());
                }
            } else {
                // Historical rule: the adjustment covers the optionlets up to
                // the cap's leg size, inclusive.
                for (Size i = 0; i < optionletVolatilities_.size(); ++i)
                    if (i <= leg.size())
                        capOptionletIndices[j].push_back(i);
            }
            atmCapFloorStrikes_[j] = CashFlows::atmRate(
                leg, **iborIndex_->forwardingTermStructure(), false,
                iborIndex_->forwardingTermStructure()->referenceDate());
            caps_[j] = ext::make_shared<CapFloor>(
                CapFloor::Cap, leg, std::vector<Rate>(1, atmCapFloorStrikes_[j]));
            caps_[j]->setPricingEngine(engine);
            atmCapFloorPrices_[j] = caps_[j]->NPV();
        }

        Handle<OptionletVolatilityStructure> adapter(
            ext::make_shared<StrippedOptionletAdapter>(stripper1_));
        adapter->enableExtrapolation();
        spreadsVolImplied_ = spreadsVolImplied(adapter);

        Volatility unadjustedVol, adjustedVol;
        for (Size j=0; j<nOptionExpiries_; ++j) {
            for (Size i : capOptionletIndices[j]) {
                unadjustedVol = adapter->volatility(optionletTimes_[i],
                                                    atmCapFloorStrikes_[j]);
                adjustedVol = unadjustedVol + spreadsVolImplied_[j];

                // insert adjusted volatility
                auto previous =
                    std::lower_bound(optionletStrikes_[i].begin(),
                                     optionletStrikes_[i].end(),
                                     atmCapFloorStrikes_[j]);
                Size insertIndex = previous - optionletStrikes_[i].begin();

                optionletStrikes_[i].insert(
                            optionletStrikes_[i].begin() + insertIndex,
                            atmCapFloorStrikes_[j]);
                optionletVolatilities_[i].insert(
                            optionletVolatilities_[i].begin() + insertIndex,
                            adjustedVol);
            }
        }
    }

    std::vector<Volatility> OptionletStripper2::spreadsVolImplied(
        const Handle<OptionletVolatilityStructure>& baseVolatility) const {

        Brent solver;
        std::vector<Volatility> result(nOptionExpiries_);
        const Volatility maxSpread = 0.1;
        for (Size j=0; j<nOptionExpiries_; ++j) {
            Volatility minBaseVol = QL_MAX_REAL;
            for (const auto& cashflow : caps_[j]->floatingLeg()) {
                auto coupon = ext::dynamic_pointer_cast<FloatingRateCoupon>(cashflow);
                QL_REQUIRE(coupon, "non-floating-rate coupon in cap/floor leg");
                minBaseVol = std::min(
                    minBaseVol,
                    baseVolatility->volatility(
                        coupon->fixingDate(), atmCapFloorStrikes_[j], true));
            }
            QL_REQUIRE(minBaseVol != QL_MAX_REAL,
                       "empty cap/floor leg for option expiry "
                           << atmCapFloorTermVolCurve_->optionTenors()[j]);
            QL_REQUIRE(minBaseVol >= 0.0,
                       "negative optionlet volatility at ATM strike "
                           << atmCapFloorStrikes_[j]);

            auto spreadQuote = ext::make_shared<SimpleQuote>(-1.0);
            Handle<OptionletVolatilityStructure> spreadedVolatility(
                ext::make_shared<SpreadedOptionletVolatility>(
                    baseVolatility, Handle<Quote>(spreadQuote)));
            caps_[j]->setPricingEngine(makeCapFloorPricingEngine(
                iborIndex_->forwardingTermStructure(), spreadedVolatility));

            ObjectiveFunction f(spreadQuote, caps_[j], atmCapFloorPrices_[j]);
            solver.setMaxEvaluations(maxEvaluations_);
            Real valueAtZero = f(0.0);
            if (close(valueAtZero, 0.0)) {
                result[j] = 0.0;
            } else if (valueAtZero > 0.0) {
                QL_REQUIRE(minBaseVol > 0.0,
                           "ATM cap/floor price for option expiry "
                               << atmCapFloorTermVolCurve_->optionTenors()[j]
                               << " is below the minimum attainable price");
                Volatility minSpread = -minBaseVol;
                Real valueAtMinSpread = f(minSpread);
                QL_REQUIRE(valueAtMinSpread <= 0.0,
                           "ATM cap/floor price for option expiry "
                               << atmCapFloorTermVolCurve_->optionTenors()[j]
                               << " is below the minimum attainable price");
                result[j] = solver.solve(
                    f, accuracy_, 0.5 * minSpread, minSpread, 0.0);
            } else {
                Real valueAtMaxSpread = f(maxSpread);
                QL_REQUIRE(valueAtMaxSpread >= 0.0,
                           "ATM cap/floor price for option expiry "
                               << atmCapFloorTermVolCurve_->optionTenors()[j]
                               << " requires a volatility spread greater than "
                               << maxSpread);
                result[j] = solver.solve(
                    f, accuracy_, 0.5 * maxSpread, 0.0, maxSpread);
            }
        }
        return result;
    }

    std::vector<Volatility> OptionletStripper2::spreadsVol() const {
        calculate();
        return spreadsVolImplied_;
    }

    std::vector<Rate> OptionletStripper2::atmCapFloorStrikes() const{
        calculate();
        return atmCapFloorStrikes_;
    }

    std::vector<Real> OptionletStripper2::atmCapFloorPrices() const {
        calculate();
        return atmCapFloorPrices_;
    }

//==========================================================================//
//                 OptionletStripper2::ObjectiveFunction                    //
//==========================================================================//

    OptionletStripper2::ObjectiveFunction::ObjectiveFunction(
        ext::shared_ptr<SimpleQuote> spreadQuote,
        ext::shared_ptr<CapFloor> cap,
        Real targetValue)
    : spreadQuote_(std::move(spreadQuote)), cap_(std::move(cap)),
      targetValue_(targetValue) {}

    Real OptionletStripper2::ObjectiveFunction::operator()(Volatility s) const
    {
        if (s!=spreadQuote_->value())
            spreadQuote_->setValue(s);
        return cap_->NPV()-targetValue_;
    }
}
