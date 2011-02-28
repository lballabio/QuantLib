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
#include <ql/pricingengines/swaption/blackswaptionengine.hpp>
//#include <ql/math/solvers1d/brent.hpp>
#include <ql/math/solvers1d/newtonsafe.hpp>
#include <ql/quotes/simplequote.hpp>
#include <ql/exercise.hpp>

namespace QuantLib {

    namespace {

        class ImpliedVolHelper {
          public:
            ImpliedVolHelper(const Swaption&,
                             const Handle<YieldTermStructure>& discountCurve,
                             Real targetValue);
            Real operator()(Volatility x) const;
            Real derivative(Volatility x) const;
          private:
            boost::shared_ptr<PricingEngine> engine_;
            Handle<YieldTermStructure> discountCurve_;
            Real targetValue_;
            boost::shared_ptr<SimpleQuote> vol_;
            const Instrument::results* results_;
        };

        ImpliedVolHelper::ImpliedVolHelper(
                              const Swaption& swaption,
                              const Handle<YieldTermStructure>& discountCurve,
                              Real targetValue)
        : discountCurve_(discountCurve), targetValue_(targetValue) {

            // set an implausible value, so that calculation is forced
            // at first ImpliedVolHelper::operator()(Volatility x) call
            vol_ = boost::shared_ptr<SimpleQuote>(new SimpleQuote(-1.0));
            Handle<Quote> h(vol_);
            engine_ = boost::shared_ptr<PricingEngine>(new
                                    BlackSwaptionEngine(discountCurve_, h));
            swaption.setupArguments(engine_->getArguments());

            results_ =
                dynamic_cast<const Instrument::results*>(engine_->getResults());
        }

        Real ImpliedVolHelper::operator()(Volatility x) const {
            if (x!=vol_->value()) {
                vol_->setValue(x);
                engine_->calculate();
            }
            return results_->value-targetValue_;
        }

        Real ImpliedVolHelper::derivative(Volatility x) const {
            if (x!=vol_->value()) {
                vol_->setValue(x);
                engine_->calculate();
            }
            std::map<std::string,boost::any>::const_iterator vega_ =
                results_->additionalResults.find("vega");
            QL_REQUIRE(vega_ != results_->additionalResults.end(),
                       "vega not provided");
            return boost::any_cast<Real>(vega_->second);
        }
    }

    std::ostream& operator<<(std::ostream& out,
                             Settlement::Type t) {
        switch (t) {
          case Settlement::Physical:
            return out << "Delivery";
          case Settlement::Cash:
            return out << "Cash";
          default:
            QL_FAIL("unknown Settlement::Type(" << Integer(t) << ")");
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
        return detail::simple_event(exercise_->dates().back()).hasOccurred();
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
    }

    Volatility Swaption::impliedVolatility(
                              Real targetValue,
                              const Handle<YieldTermStructure>& discountCurve,
                              Volatility guess,
                              Real accuracy,
                              Natural maxEvaluations,
                              Volatility minVol,
                              Volatility maxVol) const {
        //calculate();
        QL_REQUIRE(!isExpired(), "instrument expired");

        ImpliedVolHelper f(*this, discountCurve, targetValue);
        //Brent solver;
        NewtonSafe solver;
        solver.setMaxEvaluations(maxEvaluations);
        return solver.solve(f, accuracy, guess, minVol, maxVol);
    }

}
