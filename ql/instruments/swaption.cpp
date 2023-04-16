/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2001, 2002, 2003 Sadruddin Rejeb
 Copyright (C) 2006 Cristina Duminuco
 Copyright (C) 2006 Marco Bianchetti
 Copyright (C) 2007 StatPro Italia srl
 Copyright (C) 2014 Ferdinando Ametrano
 Copyright (C) 2016, 2018 Peter Caspers

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
#include <ql/instruments/swaption.hpp>
#include <ql/math/solvers1d/newtonsafe.hpp>
#include <ql/pricingengines/swaption/blackswaptionengine.hpp>
#include <ql/quotes/simplequote.hpp>
#include <memory>
#include <utility>

namespace QuantLib {

    namespace {

        class ImpliedSwaptionVolHelper {
          public:
            ImpliedSwaptionVolHelper(const Swaption&,
                                     Handle<YieldTermStructure> discountCurve,
                                     Real targetValue,
                                     Real displacement,
                                     VolatilityType type);
            Real operator()(Volatility x) const;
            Real derivative(Volatility x) const;
          private:
            std::shared_ptr<PricingEngine> engine_;
            Handle<YieldTermStructure> discountCurve_;
            Real targetValue_;
            std::shared_ptr<SimpleQuote> vol_;
            const Instrument::results* results_;
        };

        ImpliedSwaptionVolHelper::ImpliedSwaptionVolHelper(const Swaption& swaption,
                                                           Handle<YieldTermStructure> discountCurve,
                                                           Real targetValue,
                                                           Real displacement,
                                                           VolatilityType type)
        : discountCurve_(std::move(discountCurve)), targetValue_(targetValue),
          vol_(std::make_shared<SimpleQuote>(-1.0)) {

            // vol_ is set an implausible value, so that calculation is forced
            // at first ImpliedSwaptionVolHelper::operator()(Volatility x) call

            Handle<Quote> h(vol_);

            switch (type) {
            case ShiftedLognormal:
                engine_ = std::make_shared<BlackSwaptionEngine>(
                    discountCurve_, h, Actual365Fixed(), displacement);
                break;
            case Normal:
                engine_ = std::make_shared<BachelierSwaptionEngine>(
                    discountCurve_, h, Actual365Fixed());
                break;
            default:
                QL_FAIL("unknown VolatilityType (" << type << ")");
                break;
            }
            swaption.setupArguments(engine_->getArguments());
            results_ = dynamic_cast<const Instrument::results *>(
                engine_->getResults());
        }

        Real ImpliedSwaptionVolHelper::operator()(Volatility x) const {
            if (x!=vol_->value()) {
                vol_->setValue(x);
                engine_->calculate();
            }
            return results_->value-targetValue_;
        }

        Real ImpliedSwaptionVolHelper::derivative(Volatility x) const {
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

    std::ostream& operator<<(std::ostream& out, Settlement::Method m) {
        switch (m) {
        case Settlement::PhysicalOTC:
            return out << "PhysicalOTC";
        case Settlement::PhysicalCleared:
            return out << "PhysicalCleared";
        case Settlement::CollateralizedCashPrice:
            return out << "CollateralizedCashPrice";
        case Settlement::ParYieldCurve:
            return out << "ParYieldCurve";
        default:
            QL_FAIL("unknown Settlement::Method(" << Integer(m) << ")");
        }
    }

    Swaption::Swaption(std::shared_ptr<VanillaSwap> swap,
                       const std::shared_ptr<Exercise>& exercise,
                       Settlement::Type delivery,
                       Settlement::Method settlementMethod)
    : Option(std::shared_ptr<Payoff>(), exercise), swap_(std::move(swap)),
      settlementType_(delivery), settlementMethod_(settlementMethod) {
        registerWith(swap_);
        registerWithObservables(swap_);
    }

    bool Swaption::isExpired() const {
        return detail::simple_event(exercise_->dates().back()).hasOccurred();
    }

    void Swaption::setupArguments(PricingEngine::arguments* args) const {

        swap_->setupArguments(args);

        auto* arguments = dynamic_cast<Swaption::arguments*>(args);

        QL_REQUIRE(arguments != nullptr, "wrong argument type");

        arguments->swap = swap_;
        arguments->settlementType = settlementType_;
        arguments->settlementMethod = settlementMethod_;
        arguments->exercise = exercise_;
    }

    void Swaption::arguments::validate() const {
        VanillaSwap::arguments::validate();
        QL_REQUIRE(swap, "vanilla swap not set");
        QL_REQUIRE(exercise, "exercise not set");
        Settlement::checkTypeAndMethodConsistency(settlementType,
                                                  settlementMethod);
    }

    Volatility Swaption::impliedVolatility(Real targetValue,
                                           const Handle<YieldTermStructure>& d,
                                           Volatility guess,
                                           Real accuracy,
                                           Natural maxEvaluations,
                                           Volatility minVol,
                                           Volatility maxVol,
                                           VolatilityType type,
                                           Real displacement) const {
        //calculate();
        QL_REQUIRE(!isExpired(), "instrument expired");

        ImpliedSwaptionVolHelper f(*this, d, targetValue, displacement, type);
        //Brent solver;
        NewtonSafe solver;
        solver.setMaxEvaluations(maxEvaluations);
        return solver.solve(f, accuracy, guess, minVol, maxVol);
    }

    void Settlement::checkTypeAndMethodConsistency(
                                        Settlement::Type settlementType,
                                        Settlement::Method settlementMethod) {
        if (settlementType == Physical) {
            QL_REQUIRE(settlementMethod == PhysicalOTC ||
                       settlementMethod == PhysicalCleared,
                       "invalid settlement method for physical settlement");
        }
        if (settlementType == Cash) {
            QL_REQUIRE(settlementMethod == CollateralizedCashPrice ||
                       settlementMethod == ParYieldCurve,
                       "invalid settlement method for cash settlement");
        }
    }

}
