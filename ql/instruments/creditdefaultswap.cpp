/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008, 2009 Jose Aparicio
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
#include <ql/cashflows/simplecashflow.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>
#include <ql/termstructures/credit/flathazardrate.hpp>
#include <ql/pricingengines/credit/midpointcdsengine.hpp>
#include <ql/quotes/simplequote.hpp>
#include <ql/math/solvers1d/brent.hpp>
#include <ql/time/calendars/weekendsonly.hpp>

namespace QuantLib {

    CreditDefaultSwap::CreditDefaultSwap(Protection::Side side,
                                         Real notional,
                                         Rate spread,
                                         const Schedule& schedule,
                                         BusinessDayConvention convention,
                                         const DayCounter& dayCounter,
                                         bool settlesAccrual,
                                         bool paysAtDefaultTime,
                                         const Date& protectionStart,
                                         const boost::shared_ptr<Claim>& claim)
    : side_(side), notional_(notional), upfront_(boost::none),
      runningSpread_(spread), settlesAccrual_(settlesAccrual),
      paysAtDefaultTime_(paysAtDefaultTime), claim_(claim),
      protectionStart_(protectionStart == Null<Date>() ? schedule[0] :
                                                         protectionStart) {
        QL_REQUIRE(protectionStart_ <= schedule[0],
                   "protection can not start after accrual");
        leg_ = FixedRateLeg(schedule)
            .withNotionals(notional)
            .withCouponRates(spread, dayCounter)
            .withPaymentAdjustment(convention);
        upfrontPayment_.reset(new SimpleCashFlow(0.0, schedule[0]));

        if (!claim_)
            claim_ = boost::shared_ptr<Claim>(new FaceValueClaim);
        registerWith(claim_);
    }

    CreditDefaultSwap::CreditDefaultSwap(Protection::Side side,
                                         Real notional,
                                         Rate upfront,
                                         Rate runningSpread,
                                         const Schedule& schedule,
                                         BusinessDayConvention convention,
                                         const DayCounter& dayCounter,
                                         bool settlesAccrual,
                                         bool paysAtDefaultTime,
                                         const Date& protectionStart,
                                         const Date& upfrontDate,
                                         const boost::shared_ptr<Claim>& claim)
    : side_(side), notional_(notional), upfront_(upfront),
      runningSpread_(runningSpread), settlesAccrual_(settlesAccrual),
      paysAtDefaultTime_(paysAtDefaultTime), claim_(claim),
      protectionStart_(protectionStart == Null<Date>() ? schedule[0] :
                                                         protectionStart) {
        QL_REQUIRE(protectionStart_ <= schedule[0],
                   "protection can not start after accrual");
        leg_ = FixedRateLeg(schedule)
            .withNotionals(notional)
            .withCouponRates(runningSpread, dayCounter)
            .withPaymentAdjustment(convention);
        Date d = upfrontDate == Null<Date>() ? schedule[0] : upfrontDate;
        upfrontPayment_.reset(new SimpleCashFlow(notional*upfront, d));
        QL_REQUIRE(upfrontPayment_->date() >= protectionStart_,
                   "upfront can not be due before contract start");

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

    Rate CreditDefaultSwap::runningSpread() const {
        return runningSpread_;
    }

    boost::optional<Rate> CreditDefaultSwap::upfront() const {
        return upfront_;
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
        for (Leg::const_reverse_iterator i = leg_.rbegin();
                                         i != leg_.rend(); ++i) {
            if (!(*i)->hasOccurred())
                return false;
        }
        return true;
    }

    void CreditDefaultSwap::setupExpired() const {
        Instrument::setupExpired();
        fairSpread_ = fairUpfront_ = 0.0;
        couponLegBPS_ = upfrontBPS_ = 0.0;
        couponLegNPV_ = defaultLegNPV_ = upfrontNPV_ = 0.0;
    }

    void CreditDefaultSwap::setupArguments(
                                       PricingEngine::arguments* args) const {
        CreditDefaultSwap::arguments* arguments =
            dynamic_cast<CreditDefaultSwap::arguments*>(args);
        QL_REQUIRE(arguments != 0, "wrong argument type");

        arguments->side = side_;
        arguments->notional = notional_;
        arguments->leg = leg_;
        arguments->upfrontPayment = upfrontPayment_;
        arguments->settlesAccrual = settlesAccrual_;
        arguments->paysAtDefaultTime = paysAtDefaultTime_;
        arguments->claim = claim_;
        arguments->upfront = upfront_;
        arguments->spread = runningSpread_;
        arguments->protectionStart = protectionStart_;
    }


    void CreditDefaultSwap::fetchResults(
                                      const PricingEngine::results* r) const {
        Instrument::fetchResults(r);

        const CreditDefaultSwap::results* results =
            dynamic_cast<const CreditDefaultSwap::results*>(r);
        QL_REQUIRE(results != 0, "wrong result type");

        fairSpread_ = results->fairSpread;
        fairUpfront_ = results->fairUpfront;
        couponLegBPS_ = results->couponLegBPS;
        couponLegNPV_ = results->couponLegNPV;
        defaultLegNPV_ = results->defaultLegNPV;
        upfrontNPV_ = results->upfrontNPV;
        upfrontBPS_ = results->upfrontBPS;
    }

    Rate CreditDefaultSwap::fairUpfront() const {
        calculate();
        QL_REQUIRE(fairUpfront_ != Null<Rate>(),
                   "fair upfront not available");
        return fairUpfront_;
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

    Real CreditDefaultSwap::upfrontNPV() const {
        calculate();
        QL_REQUIRE(upfrontNPV_ != Null<Real>(),
                   "upfront NPV not available");
        return upfrontNPV_;
    }

    Real CreditDefaultSwap::upfrontBPS() const {
        calculate();
        QL_REQUIRE(upfrontBPS_ != Null<Real>(),
                   "upfront BPS not available");
        return upfrontBPS_;
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
                FlatHazardRate(0, WeekendsOnly(),
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


    Rate CreditDefaultSwap::conventionalSpread(
                              Real conventionalRecovery,
                              const Handle<YieldTermStructure>& discountCurve,
                              const DayCounter& dayCounter) const {
        Rate flatHazardRate = impliedHazardRate(0.0,
                                                discountCurve,
                                                dayCounter,
                                                conventionalRecovery);

        Handle<DefaultProbabilityTermStructure> probability(
            boost::shared_ptr<DefaultProbabilityTermStructure>(
                             new FlatHazardRate(0, WeekendsOnly(),
                                                flatHazardRate, dayCounter)));

        MidPointCdsEngine engine(probability, conventionalRecovery,
                                 discountCurve, true);
        setupArguments(engine.getArguments());
        engine.calculate();
        const CreditDefaultSwap::results* results =
            dynamic_cast<const CreditDefaultSwap::results*>(
                                                       engine.getResults());
        return results->fairSpread;
    }


    const Date& CreditDefaultSwap::protectionStartDate() const {
        return protectionStart_;
    }

    const Date& CreditDefaultSwap::protectionEndDate() const {
        return boost::dynamic_pointer_cast<Coupon>(leg_.back())
            ->accrualEndDate();
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
        QL_REQUIRE(upfrontPayment, "upfront payment not set");
        QL_REQUIRE(claim, "claim not set");
        QL_REQUIRE(protectionStart != Null<Date>(),
                   "protection start date not set");
    }

    void CreditDefaultSwap::results::reset() {
        Instrument::results::reset();
        fairSpread = Null<Rate>();
        fairUpfront = Null<Rate>();
        couponLegBPS = Null<Real>();
        couponLegNPV = Null<Real>();
        defaultLegNPV = Null<Real>();
        upfrontBPS = Null<Real>();
        upfrontNPV = Null<Real>();
    }

}
