/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2001, 2002, 2003 Sadruddin Rejeb
 Copyright (C) 2006 Cristina Duminuco
 Copyright (C) 2006 Marco Bianchetti
 Copyright (C) 2007 StatPro Italia srl
 Copyright (C) 2010 Andre Miemiec

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

#include <ql/any.hpp>
#include <ql/exercise.hpp>
#include <ql/experimental/swaptions/irregularswaption.hpp>
#include <ql/math/solvers1d/newtonsafe.hpp>
#include <ql/pricingengines/swaption/blackswaptionengine.hpp>
#include <ql/quotes/simplequote.hpp>
#include <utility>

namespace QuantLib {

    namespace {

        class IrregularImpliedVolHelper {
          public:
            IrregularImpliedVolHelper(const IrregularSwaption&,
                                      Handle<YieldTermStructure> discountCurve,
                                      Real targetValue);
            Real operator()(Volatility x) const;
            Real derivative(Volatility x) const;
          private:
            std::shared_ptr<PricingEngine> engine_;
            Handle<YieldTermStructure> discountCurve_;
            Real targetValue_;
            std::shared_ptr<SimpleQuote> vol_;
            const Instrument::results* results_;
        };

        IrregularImpliedVolHelper::IrregularImpliedVolHelper(
            const IrregularSwaption& swaption,
            Handle<YieldTermStructure> discountCurve,
            Real targetValue)
        : discountCurve_(std::move(discountCurve)), targetValue_(targetValue),
          vol_(std::make_shared<SimpleQuote>(-1.0)) {

            Handle<Quote> h(vol_);
            engine_ = std::shared_ptr<PricingEngine>(new
                                    BlackSwaptionEngine(discountCurve_, h));
            swaption.setupArguments(engine_->getArguments());

            results_ =
                dynamic_cast<const Instrument::results*>(engine_->getResults());
        }

        Real IrregularImpliedVolHelper::operator()(Volatility x) const {
            if (x!=vol_->value()) {
                vol_->setValue(x);
                engine_->calculate();
            }
            return results_->value-targetValue_;
        }

        Real IrregularImpliedVolHelper::derivative(Volatility x) const {
            if (x!=vol_->value()) {
                vol_->setValue(x);
                engine_->calculate();
            }
            auto vega_ = results_->additionalResults.find("vega");
            QL_REQUIRE(vega_ != results_->additionalResults.end(),
                       "vega not provided");
            return std::any_cast<Real>(vega_->second);
        }
    }

    std::ostream& operator<<(std::ostream& out,
                             IrregularSettlement::Type t) {
        switch (t) {
          case IrregularSettlement::Physical:
            return out << "Delivery";
          case IrregularSettlement::Cash:
            return out << "Cash";
          default:
            QL_FAIL("unknown IrregularSettlement::Type(" << Integer(t) << ")");
        }
    }

    IrregularSwaption::IrregularSwaption(std::shared_ptr<IrregularSwap> swap,
                                         const std::shared_ptr<Exercise>& exercise,
                                         IrregularSettlement::Type delivery)
    : Option(std::shared_ptr<Payoff>(), exercise), swap_(std::move(swap)),
      settlementType_(delivery) {
        registerWith(swap_);
    }

    bool IrregularSwaption::isExpired() const {
        return detail::simple_event(exercise_->dates().back()).hasOccurred();
    }

    void IrregularSwaption::setupArguments(PricingEngine::arguments* args) const {

        swap_->setupArguments(args);

        auto* arguments = dynamic_cast<IrregularSwaption::arguments*>(args);

        QL_REQUIRE(arguments != nullptr, "wrong argument type");

        arguments->swap = swap_;
        arguments->settlementType = settlementType_;
        arguments->exercise = exercise_;
    }

    void IrregularSwaption::arguments::validate() const {
        IrregularSwap::arguments::validate();
        QL_REQUIRE(swap, "Irregular swap not set");
        QL_REQUIRE(exercise, "exercise not set");
    }

    Volatility IrregularSwaption::impliedVolatility(
                              Real targetValue,
                              const Handle<YieldTermStructure>& discountCurve,
                              Volatility guess,
                              Real accuracy,
                              Natural maxEvaluations,
                              Volatility minVol,
                              Volatility maxVol) const {
        calculate();
        QL_REQUIRE(!isExpired(), "instrument expired");

        IrregularImpliedVolHelper f(*this, discountCurve, targetValue);
        //Brent solver;
        NewtonSafe solver;
        solver.setMaxEvaluations(maxEvaluations);
        return solver.solve(f, accuracy, guess, minVol, maxVol);
    }

}
