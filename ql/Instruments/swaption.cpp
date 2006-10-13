/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2001, 2002, 2003 Sadruddin Rejeb
 Copyright (C) 2006 Cristina Duminuco
 Copyright (C) 2006 Marco Bianchetti

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

#include <ql/Instruments/swaption.hpp>
#include <ql/CashFlows/fixedratecoupon.hpp>
#include <ql/CashFlows/parcoupon.hpp>
#include <ql/Solvers1D/brent.hpp>
#include <ql/DayCounters/all.hpp>
#include <ql/CashFlows/analysis.hpp>
#include <ql/PricingEngines/Swaption/blackswaptionengine.hpp>

namespace QuantLib {

    Swaption::Swaption(const boost::shared_ptr<VanillaSwap>& swap,
                       const boost::shared_ptr<Exercise>& exercise,
                       const Handle<YieldTermStructure>& termStructure,
                       const boost::shared_ptr<PricingEngine>& engine,
                       Settlement::Type delivery)
    : Option(boost::shared_ptr<Payoff>(), exercise, engine), swap_(swap),
      termStructure_(termStructure), settlementType_(delivery) {
        registerWith(swap_);
        registerWith(termStructure_);
    }

    bool Swaption::isExpired() const {
        return exercise_->dates().back() < termStructure_->referenceDate();
    }

    void Swaption::setupArguments(Arguments* args) const {

        swap_->setupArguments(args);

        Swaption::arguments* arguments =
            dynamic_cast<Swaption::arguments*>(args);

        QL_REQUIRE(arguments != 0, "wrong argument type");

        DayCounter counter = termStructure_->dayCounter();

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
        Date settlement = termStructure_->referenceDate();
        // only if cash settled
        if (arguments->settlementType==Settlement::Cash) {
            const std::vector<boost::shared_ptr<CashFlow> >& swapFixedLeg =
                swap_->fixedLeg();
            DayCounter dc = (boost::dynamic_pointer_cast<FixedRateCoupon>(
                swapFixedLeg[0]))->dayCounter();
            arguments->fixedCashBPS = Cashflows::bps(swapFixedLeg,
                InterestRate(arguments->fairRate, dc, Compounded),
                settlement) ;
        }
        arguments->exercise = exercise_;
        arguments->stoppingTimes.clear();
        for (Size i=0; i<exercise_->dates().size(); i++) {
            Time time = counter.yearFraction(settlement,
                                             exercise_->dates()[i]);
            arguments->stoppingTimes.push_back(time);
        }
    }

    void Swaption::arguments::validate() const {
        #if defined(QL_PATCH_MSVC6)
        VanillaSwap::arguments copy = *this;
        copy.validate();
        #else
        VanillaSwap::arguments::validate();
        #endif
        QL_REQUIRE(fixedRate != Null<Real>(),
                   "fixed swap rate null or not set");
        QL_REQUIRE(fairRate != Null<Real>(),
                   "fair swap rate null or not set");
        QL_REQUIRE(fixedBPS != Null<Real>(),
                   "fixed swap BPS null or not set");
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

        ImpliedVolHelper f(*this,termStructure_,targetValue);
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
        engine_ = boost::shared_ptr<PricingEngine>(new BlackSwaptionEngine(h));
        swaption.setupArguments(engine_->arguments());

        results_ = dynamic_cast<const Value*>(engine_->results());
    }

    Real Swaption::ImpliedVolHelper::operator()(Volatility x) const {
        vol_->setValue(x);
        engine_->calculate();
        return results_->value-targetValue_;
    }

    Real Swaption::vega() const {
        calculate();
        return vega_;
    }

    void Swaption::fetchResults (const Results* r) const{
        Instrument::fetchResults(r);
        const Swaption::results* results =
            dynamic_cast<const Swaption::results*>(r);
        vega_ = results->vega_;
        
    }

}
