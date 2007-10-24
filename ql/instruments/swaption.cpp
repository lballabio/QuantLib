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
#include <ql/math/solvers1d/brent.hpp>
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
        Date today = Settings::instance().evaluationDate();
        return exercise_->dates().back() < today;
    }

    void Swaption::setupArguments(PricingEngine::arguments* args) const {

        swap_->setupArguments(args);

        Swaption::arguments* arguments =
            dynamic_cast<Swaption::arguments*>(args);

        QL_REQUIRE(arguments != 0, "wrong argument type");

        arguments->swap = swap_;
        arguments->settlementType = settlementType_;
        arguments->exercise = exercise_;
    }

    void Swaption::arguments::validate() const {
        VanillaSwap::arguments::validate();
        QL_REQUIRE(swap, "vanilla swap not set");
        QL_REQUIRE(exercise, "exercise not set");
        //QL_REQUIRE(fixedRate != Null<Real>(),
        //           "fixed swap rate null or not set");
        //QL_REQUIRE(fairRate != Null<Real>(),
        //           "fair swap rate null or not set");
        //QL_REQUIRE(fixedBPS != Null<Real>(),
        //           "fixed swap BPS null or not set");
        //QL_REQUIRE(forecastingDiscount != Null<Real>(),
        //           "forecasting discount null or not set");
        //if(settlementType == Settlement::Cash) {
        //    QL_REQUIRE(fixedCashBPS != Null<Real>(),
        //               "fixed swap cash BPS null or not set "
        //               "for cash-settled swaption");
        //}
    }

    Volatility Swaption::impliedVolatility(
                              Real targetValue,
                              const Handle<YieldTermStructure>& termStructure,
                              Real accuracy,
                              Size maxEvaluations,
                              Volatility minVol,
                              Volatility maxVol) const {
        calculate();
        QL_REQUIRE(!isExpired(), "instrument expired");

        Volatility guess = 0.10; // improve

        ImpliedVolHelper f(*this, termStructure, targetValue);
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
        engine_ = boost::shared_ptr<PricingEngine>(new
                                        BlackSwaptionEngine(termStructure, h));
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
