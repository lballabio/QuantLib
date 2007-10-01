/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2001, 2002, 2003 Sadruddin Rejeb
 Copyright (C) 2006 Cristina Duminuco
 Copyright (C) 2006 Marco Bianchetti
 Copyright (C) 2007 StatPro Italia srl

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

#include <ql/instruments/swaption.hpp>
#include <ql/cashflows/fixedratecoupon.hpp>
#include <ql/math/solvers1d/brent.hpp>
#include <ql/cashflows/cashflows.hpp>
#include <ql/pricingengines/swaption/blackswaptionengine.hpp>
#include <ql/yieldtermstructure.hpp>
#include <ql/exercise.hpp>

namespace QuantLib {

    std::ostream& operator<<(std::ostream& out,
                             Settlement::Type type) {
        switch (type) {
          case Settlement::Physical:
            return out << "delivery";
          case Settlement::Cash:
            return out << "cash";
          default:
            QL_FAIL("unknown settlement type");
        }
    }

    Swaption::Swaption(const boost::shared_ptr<VanillaSwap>& swap,
                       const boost::shared_ptr<Exercise>& exercise,
                       Settlement::Type delivery)
    : Option(boost::shared_ptr<Payoff>(), exercise), swap_(swap),
      settlementType_(delivery) {
        registerWith(swap_);
    }

    bool Swaption::isExpired() const {
        return exercise_->dates().back() < swap_->discountCurve()->referenceDate();
    }

    void Swaption::setupArguments(PricingEngine::arguments* args) const {

        swap_->setupArguments(args);
        
        Swaption::arguments* arguments =
            dynamic_cast<Swaption::arguments*>(args);

        QL_REQUIRE(arguments != 0, "wrong argument type");

        DayCounter counter = swap_->discountCurve()->dayCounter();

        // volatilities are calculated for zero-spreaded swaps.
        // Therefore, the spread on the floating leg is removed
        // and a corresponding correction is made on the fixed leg.
        Spread correction = swap_->spread() *
            swap_->floatingLegBPS() / swap_->fixedLegBPS();
        // the above is the opposite of the needed value since the
        // two BPSs have opposite sign; hence the + sign below
        arguments->fixedRate = swap_->fixedRate() + correction;
        arguments->fairRate = swap_->fairRate() + correction;
        // this is passed explicitly for precision
        arguments->fixedBPS = std::fabs(swap_->fixedLegBPS());
        arguments->settlementType = settlementType_;
        Date settlement = swap_->discountCurve()->referenceDate();
        // only if cash settled
        if (arguments->settlementType==Settlement::Cash) {
            const Leg& swapFixedLeg =
                swap_->fixedLeg();
            DayCounter dc = (boost::dynamic_pointer_cast<FixedRateCoupon>(
                swapFixedLeg[0]))->dayCounter();
            arguments->fixedCashBPS = CashFlows::bps(swapFixedLeg,
                InterestRate(arguments->fairRate, dc, Compounded),
                settlement) ;
        }
        arguments->exercise = exercise_;
        Size n = exercise_->dates().size();
        arguments->stoppingTimes.clear();
        arguments->stoppingTimes.reserve(n);
        for (Size i=0; i<n; ++i) {
            Time time = counter.yearFraction(settlement,
                                             exercise_->dates()[i]);
            arguments->stoppingTimes.push_back(time);
        }
        arguments->forecastingDiscount = 
            swap_->discountCurve()->discount(arguments->floatingPayTimes.back());
    }

    void Swaption::arguments::validate() const {
        VanillaSwap::arguments::validate();
        QL_REQUIRE(fixedRate != Null<Real>(),
                   "fixed swap rate null or not set");
        QL_REQUIRE(fairRate != Null<Real>(),
                   "fair swap rate null or not set");
        QL_REQUIRE(fixedBPS != Null<Real>(),
                   "fixed swap BPS null or not set");
        QL_REQUIRE(forecastingDiscount != Null<Real>(),
                   "forecasting discount null or not set");
        if(settlementType == Settlement::Cash) {
            QL_REQUIRE(fixedCashBPS != Null<Real>(),
                       "fixed swap cash BPS null or not set "
                       "for cash-settled swaption");
        }
    }

    Volatility Swaption::impliedVolatility(Real targetValue,
                                           Real accuracy,
                                           Size maxEvaluations,
                                           Volatility minVol,
                                           Volatility maxVol) const {
        calculate();
        QL_REQUIRE(!isExpired(), "instrument expired");

        Volatility guess = 0.10; // improve

        ImpliedVolHelper f(*this, swap_->discountCurve(), targetValue);
        Brent solver;
        solver.setMaxEvaluations(maxEvaluations);
        return solver.solve(f, accuracy, guess, minVol, maxVol);
    }

    Swaption::ImpliedVolHelper::ImpliedVolHelper(
                              const Swaption& swaption,
                              const Handle<YieldTermStructure>& termStructure,
                              Real targetValue)
    : termStructure_(termStructure), targetValue_(targetValue) {

        vol_ = boost::shared_ptr<SimpleQuote>(new SimpleQuote(0.0));
        Handle<Quote> h(vol_);
        engine_ = boost::shared_ptr<PricingEngine>(new BlackSwaptionEngine(h, termStructure));
        swaption.setupArguments(engine_->getArguments());

        results_ =
            dynamic_cast<const Instrument::results*>(engine_->getResults());
    }

    Real Swaption::ImpliedVolHelper::operator()(Volatility x) const {
        vol_->setValue(x);
        engine_->calculate();
        return results_->value-targetValue_;
    }

    Rate Swaption::atmRate() const{
        return swap_->fairRate();
    }
}
