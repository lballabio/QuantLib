/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2001, 2002, 2003 Sadruddin Rejeb
 Copyright (C) 2006 Cristina Duminuco
 Copyright (C) 2006 Marco Bianchetti
 Copyright (C) 2007 StatPro Italia srl
 Copyright (C) 2014 Ferdinando Ametrano
 Copyright (C) 2016 Peter Caspers

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
#include <ql/math/solvers1d/newtonsafe.hpp>
#include <ql/quotes/simplequote.hpp>
#include <ql/exercise.hpp>
#include <ql/shared_ptr.hpp>

namespace QuantLib {

    namespace {

        class ImpliedSwaptionVolHelper {
          public:
            ImpliedSwaptionVolHelper(const Swaption&,
                                     const Handle<YieldTermStructure>& discountCurve,
                                     Real targetValue,
                                     Real displacement,
                                     VolatilityType type);
            Real operator()(Volatility x) const;
            Real derivative(Volatility x) const;
          private:
            ext::shared_ptr<PricingEngine> engine_;
            Handle<YieldTermStructure> discountCurve_;
            Real targetValue_;
            ext::shared_ptr<SimpleQuote> vol_;
            const Instrument::results* results_;
        };

        ImpliedSwaptionVolHelper::ImpliedSwaptionVolHelper(
                              const Swaption& swaption,
                              const Handle<YieldTermStructure>& discountCurve,
                              Real targetValue,
                              Real displacement,
                              VolatilityType type)
        : discountCurve_(discountCurve), targetValue_(targetValue) {

            // set an implausible value, so that calculation is forced
            // at first ImpliedSwaptionVolHelper::operator()(Volatility x) call
            vol_ = ext::make_shared<SimpleQuote>(-1.0);
            Handle<Quote> h(vol_);

            switch (type) {
            case ShiftedLognormal:
                engine_ = ext::make_shared<BlackSwaptionEngine>(
                    discountCurve_, h, Actual365Fixed(), displacement);
                break;
            case Normal:
                engine_ = ext::make_shared<BachelierSwaptionEngine>(
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

    Swaption::Swaption(const ext::shared_ptr<VanillaSwap>& swap,
                       const ext::shared_ptr<Exercise>& exercise,
                       Settlement::Type delivery)
    : Option(ext::shared_ptr<Payoff>(), exercise), swap_(swap),
      settlementType_(delivery) {
        registerWith(swap_);
        registerWithObservables(swap_);
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

}
