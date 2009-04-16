/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Jose Aparicio
 Copyright (C) 2008 Roland Lichters
 Copyright (C) 2008 StatPro Italia srl

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

#include <ql/instruments/creditdefaultswap.hpp>
#include <ql/instruments/claim.hpp>
#include <ql/cashflows/fixedratecoupon.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>
#include <ql/termstructures/credit/flathazardrate.hpp>
#include <ql/pricingengines/credit/midpointcdsengine.hpp>
#include <ql/quotes/simplequote.hpp>
#include <ql/math/solvers1d/brent.hpp>
#include <ql/event.hpp>

namespace QuantLib {

    CreditDefaultSwap::CreditDefaultSwap(Protection::Side side,
                                         Real notional,
                                         Rate spread,
                                         const Schedule& schedule,
                                         BusinessDayConvention convention,
                                         const DayCounter& dayCounter,
                                         bool settlesAccrual,
                                         bool paysAtDefaultTime,
                                         const boost::shared_ptr<Claim>& claim)
    : side_(side), notional_(notional), spread_(spread),
      settlesAccrual_(settlesAccrual), paysAtDefaultTime_(paysAtDefaultTime),
      claim_(claim) {
        leg_ = FixedRateLeg(schedule, dayCounter)
            .withNotionals(notional)
            .withCouponRates(spread)
            .withPaymentAdjustment(convention);

        if (!claim_)
            claim_ = boost::shared_ptr<Claim>(new FaceValueClaim);
        registerWith(claim_);
    }

    Protection::Side CreditDefaultSwap::side() const {
        return side_;
    }

    Real CreditDefaultSwap::notional() const {
        return notional_;
    }

    Rate CreditDefaultSwap::spread() const {
        return spread_;
    }

    bool CreditDefaultSwap::settlesAccrual() const {
        return settlesAccrual_;
    }

    bool CreditDefaultSwap::paysAtDefaultTime() const {
        return paysAtDefaultTime_;
    }

    const Leg& CreditDefaultSwap::coupons() const {
        return leg_;
    }


    bool CreditDefaultSwap::isExpired() const {
        for (Size i=leg_.size(); i>0; --i)
            if (!leg_[i-1]->hasOccurred())
                return false;
        return true;
    }

    void CreditDefaultSwap::setupExpired() const {
        Instrument::setupExpired();
        fairSpread_ = 0.0;
        couponLegBPS_ = 0.0;
        couponLegNPV_ = defaultLegNPV_ = 0.0;
    }

    void CreditDefaultSwap::setupArguments(
                                       PricingEngine::arguments* args) const {
        CreditDefaultSwap::arguments* arguments =
            dynamic_cast<CreditDefaultSwap::arguments*>(args);
        QL_REQUIRE(arguments != 0, "wrong argument type");

        arguments->side = side_;
        arguments->notional = notional_;
        arguments->spread = spread_;
        arguments->leg = leg_;
        arguments->settlesAccrual = settlesAccrual_;
        arguments->paysAtDefaultTime = paysAtDefaultTime_;
        arguments->claim = claim_;
    }


    void CreditDefaultSwap::fetchResults(
                                      const PricingEngine::results* r) const {
        Instrument::fetchResults(r);

        const CreditDefaultSwap::results* results =
            dynamic_cast<const CreditDefaultSwap::results*>(r);
        QL_REQUIRE(results != 0, "wrong result type");

        fairSpread_ = results->fairSpread;
        couponLegBPS_ = results->couponLegBPS;
        couponLegNPV_ = results->couponLegNPV;
        defaultLegNPV_ = results->defaultLegNPV;
    }


    Rate CreditDefaultSwap::fairSpread() const {
        calculate();
        QL_REQUIRE(fairSpread_ != Null<Rate>(),
                   "fair spread not available");
        return fairSpread_;
    }

    Real CreditDefaultSwap::couponLegBPS() const {
        calculate();
        QL_REQUIRE(couponLegBPS_ != Null<Rate>(),
                   "coupon-leg BPS not available");
        return couponLegBPS_;
    }

    Real CreditDefaultSwap::couponLegNPV() const {
        calculate();
        QL_REQUIRE(couponLegNPV_ != Null<Rate>(),
                   "coupon-leg NPV not available");
        return couponLegNPV_;
    }

    Real CreditDefaultSwap::defaultLegNPV() const {
        calculate();
        QL_REQUIRE(defaultLegNPV_ != Null<Rate>(),
                   "default-leg NPV not available");
        return defaultLegNPV_;
    }


    namespace {

        class ObjectiveFunction {
          public:
            ObjectiveFunction(Real target,
                              SimpleQuote& quote,
                              PricingEngine& engine,
                              const CreditDefaultSwap::results* results)
            : target_(target), quote_(quote),
              engine_(engine), results_(results) {}

            Real operator()(Real guess) const {
                quote_.setValue(guess);
                engine_.calculate();
                return results_->value - target_;
            }
          private:
            Real target_;
            SimpleQuote& quote_;
            PricingEngine& engine_;
            const CreditDefaultSwap::results* results_;
        };

    }

    Rate CreditDefaultSwap::impliedHazardRate(
                               Real targetNPV,
                               const Handle<YieldTermStructure>& discountCurve,
                               const DayCounter& dayCounter,
                               Real recoveryRate,
                               Real accuracy) const {

        boost::shared_ptr<SimpleQuote> flatRate(new SimpleQuote(0.0));

        Handle<DefaultProbabilityTermStructure> probability(
            boost::shared_ptr<DefaultProbabilityTermStructure>(new
                FlatHazardRate(0, NullCalendar(),
                               Handle<Quote>(flatRate), dayCounter)));

        MidPointCdsEngine engine(probability, recoveryRate, discountCurve);
        setupArguments(engine.getArguments());
        const CreditDefaultSwap::results* results =
            dynamic_cast<const CreditDefaultSwap::results*>(
                                                       engine.getResults());

        ObjectiveFunction f(targetNPV, *flatRate, engine, results);
        Rate guess = 0.001;
        Rate step = guess*0.1;

        return Brent().solve(f, accuracy, guess, step);
    }



    CreditDefaultSwap::arguments::arguments()
    : side(Protection::Side(-1)), notional(Null<Real>()),
      spread(Null<Rate>()) {}

    void CreditDefaultSwap::arguments::validate() const {
        QL_REQUIRE(side != Protection::Side(-1), "side not set");
        QL_REQUIRE(notional != Null<Real>(), "notional not set");
        QL_REQUIRE(notional != 0.0, "null notional set");
        QL_REQUIRE(spread != Null<Rate>(), "spread not set");
        QL_REQUIRE(!leg.empty(), "coupons not set");
        QL_REQUIRE(claim, "claim not set");
    }

    void CreditDefaultSwap::results::reset() {
        Instrument::results::reset();
        fairSpread = Null<Rate>();
        couponLegBPS = Null<Real>();
        couponLegNPV = Null<Real>();
        defaultLegNPV = Null<Real>();
    }

}
