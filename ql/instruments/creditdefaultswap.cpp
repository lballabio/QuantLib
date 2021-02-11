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

#include <ql/cashflows/fixedratecoupon.hpp>
#include <ql/cashflows/simplecashflow.hpp>
#include <ql/instruments/claim.hpp>
#include <ql/instruments/creditdefaultswap.hpp>
#include <ql/math/solvers1d/brent.hpp>
#include <ql/pricingengines/credit/isdacdsengine.hpp>
#include <ql/pricingengines/credit/midpointcdsengine.hpp>
#include <ql/quotes/simplequote.hpp>
#include <ql/termstructures/credit/flathazardrate.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>
#include <ql/time/calendars/weekendsonly.hpp>
#include <ql/time/schedule.hpp>
#include <utility>

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
                                         ext::shared_ptr<Claim> claim,
                                         const DayCounter& lastPeriodDayCounter,
                                         const bool rebatesAccrual,
                                         const Date& tradeDate,
                                         Natural cashSettlementDays)
    : side_(side), notional_(notional), upfront_(boost::none), runningSpread_(spread),
      settlesAccrual_(settlesAccrual), paysAtDefaultTime_(paysAtDefaultTime),
      claim_(std::move(claim)),
      protectionStart_(protectionStart == Null<Date>() ? schedule[0] : protectionStart),
      tradeDate_(tradeDate), cashSettlementDays_(cashSettlementDays) {

        init(schedule, convention, dayCounter, lastPeriodDayCounter, rebatesAccrual);
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
                                         ext::shared_ptr<Claim> claim,
                                         const DayCounter& lastPeriodDayCounter,
                                         const bool rebatesAccrual,
                                         const Date& tradeDate,
                                         Natural cashSettlementDays)
    : side_(side), notional_(notional), upfront_(upfront), runningSpread_(runningSpread),
      settlesAccrual_(settlesAccrual), paysAtDefaultTime_(paysAtDefaultTime),
      claim_(std::move(claim)),
      protectionStart_(protectionStart == Null<Date>() ? schedule[0] : protectionStart),
      tradeDate_(tradeDate), cashSettlementDays_(cashSettlementDays) {

        init(schedule, convention, dayCounter, lastPeriodDayCounter, rebatesAccrual, upfrontDate);
    }

    void CreditDefaultSwap::init(const Schedule& schedule, BusinessDayConvention paymentConvention,
        const DayCounter& dayCounter, const DayCounter& lastPeriodDayCounter,
        bool rebatesAccrual, const Date& upfrontDate) {

        QL_REQUIRE(!schedule.empty(), "CreditDefaultSwap needs a non-empty schedule.");

        bool postBigBang = false;
        if (schedule.hasRule()) {
            DateGeneration::Rule rule = schedule.rule();
            postBigBang = rule == DateGeneration::CDS || rule == DateGeneration::CDS2015;
        }

        if (!postBigBang) {
            QL_REQUIRE(protectionStart_ <= schedule[0], "protection can not start after accrual");
        }

        leg_ = FixedRateLeg(schedule)
            .withNotionals(notional_)
            .withCouponRates(runningSpread_, dayCounter)
            .withPaymentAdjustment(paymentConvention)
            .withLastPeriodDayCounter(lastPeriodDayCounter);

        // Deduce the trade date if not given.
        if (tradeDate_ == Date()) {
            if (postBigBang) {
                tradeDate_ = protectionStart_;
            } else {
                tradeDate_ = protectionStart_ - 1;
            }
        }

        // Deduce the cash settlement date if not given.
        Date effectiveUpfrontDate = upfrontDate;
        if (effectiveUpfrontDate == Date()) {
            effectiveUpfrontDate = schedule.calendar().advance(tradeDate_,
                cashSettlementDays_, Days, paymentConvention);
        }
        QL_REQUIRE(effectiveUpfrontDate >= protectionStart_, "The cash settlement date must not " <<
            "be before the protection start date.");

        // Create the upfront payment, if one is provided.
        Real upfrontAmount = 0.0;
        if (upfront_) // NOLINT(readability-implicit-bool-conversion)
            upfrontAmount = *upfront_ * notional_;
        upfrontPayment_ = ext::make_shared<SimpleCashFlow>(upfrontAmount, effectiveUpfrontDate);

        // Set the maturity date.
        maturity_ = schedule.dates().back();

        // Deal with the accrual rebate. We use the standard conventions for accrual calculation introduced with the 
        // CDS Big Bang in 2009.
        if (rebatesAccrual) {

            Real rebateAmount = 0.0;
            Date refDate = tradeDate_ + 1;

            if (tradeDate_ >= schedule.dates().front()) {
                for (Size i = 0; i < leg_.size(); ++i) {
                    const ext::shared_ptr<CashFlow>& cf = leg_[i];
                    if (refDate < cf->date()) {
                        // Calculate the accrual. The most likely scenario.
                        ext::shared_ptr<FixedRateCoupon> frc = ext::dynamic_pointer_cast<FixedRateCoupon>(cf);
                        rebateAmount = frc->accruedAmount(refDate);
                        break;
                    } else if (refDate == cf->date() && i < leg_.size() - 1) {
                        // If not the last coupon and trade date + 1 is the next coupon payment date, 
                        // the accrual is 0 so do nothing.
                        break;
                    } else {
                        // Must have trade date + 1 >= last coupon's payment date. '>' here probably does not make
                        // sense - should possibly have an exception above if trade date >= last coupon's date.
                        ext::shared_ptr<FixedRateCoupon> frc = ext::dynamic_pointer_cast<FixedRateCoupon>(cf);
                        rebateAmount = frc->amount();
                        break;
                    }
                }
            }

            accrualRebate_ = ext::make_shared<SimpleCashFlow>(rebateAmount, effectiveUpfrontDate);
        }

        if (!claim_)
            claim_ = ext::make_shared<FaceValueClaim>();
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
        for (auto i = leg_.rbegin(); i != leg_.rend(); ++i) {
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
        auto* arguments = dynamic_cast<CreditDefaultSwap::arguments*>(args);
        QL_REQUIRE(arguments != nullptr, "wrong argument type");

        arguments->side = side_;
        arguments->notional = notional_;
        arguments->leg = leg_;
        arguments->upfrontPayment = upfrontPayment_;
        arguments->accrualRebate = accrualRebate_;
        arguments->settlesAccrual = settlesAccrual_;
        arguments->paysAtDefaultTime = paysAtDefaultTime_;
        arguments->claim = claim_;
        arguments->upfront = upfront_;
        arguments->spread = runningSpread_;
        arguments->protectionStart = protectionStart_;
        arguments->maturity = maturity_;
    }


    void CreditDefaultSwap::fetchResults(
                                      const PricingEngine::results* r) const {
        Instrument::fetchResults(r);

        const auto* results = dynamic_cast<const CreditDefaultSwap::results*>(r);
        QL_REQUIRE(results != nullptr, "wrong result type");

        fairSpread_ = results->fairSpread;
        fairUpfront_ = results->fairUpfront;
        couponLegBPS_ = results->couponLegBPS;
        couponLegNPV_ = results->couponLegNPV;
        defaultLegNPV_ = results->defaultLegNPV;
        upfrontNPV_ = results->upfrontNPV;
        upfrontBPS_ = results->upfrontBPS;
        accrualRebateNPV_ = results->accrualRebateNPV;
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

    Real CreditDefaultSwap::accrualRebateNPV() const {
        calculate();
        QL_REQUIRE(accrualRebateNPV_ != Null<Real>(),
                   "accrual Rebate NPV not available");
        return accrualRebateNPV_;
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
                               Real accuracy,
                               PricingModel model) const {

        ext::shared_ptr<SimpleQuote> flatRate = ext::make_shared<SimpleQuote>(0.0);

        Handle<DefaultProbabilityTermStructure> probability =
            Handle<DefaultProbabilityTermStructure>(
                ext::make_shared<FlatHazardRate>(0, WeekendsOnly(),
                                                   Handle<Quote>(flatRate), dayCounter));

        ext::shared_ptr<PricingEngine> engine;
        switch (model) {
          case Midpoint:
            engine = ext::make_shared<MidPointCdsEngine>(
                probability, recoveryRate, discountCurve);
            break;
          case ISDA:
            engine = ext::make_shared<IsdaCdsEngine>(
                probability, recoveryRate, discountCurve,
                boost::none,
                IsdaCdsEngine::Taylor,
                IsdaCdsEngine::HalfDayBias,
                IsdaCdsEngine::Piecewise);
            break;
          default:
            QL_FAIL("unknown CDS pricing model: " << model);
        }

        setupArguments(engine->getArguments());
        const auto* results = dynamic_cast<const CreditDefaultSwap::results*>(engine->getResults());

        ObjectiveFunction f(targetNPV, *flatRate, *engine, results);
        //very close guess if targetNPV = 0.
        Rate guess = runningSpread_ / (1 - recoveryRate) * 365./360.;
        Real step = 0.1 * guess;
        return Brent().solve(f, accuracy, guess, step);
    }

    Rate CreditDefaultSwap::conventionalSpread(
                              Real conventionalRecovery,
                              const Handle<YieldTermStructure>& discountCurve,
                              const DayCounter& dayCounter,
                              PricingModel model) const {

        ext::shared_ptr<SimpleQuote> flatRate = ext::make_shared<SimpleQuote>(0.0);

        Handle<DefaultProbabilityTermStructure> probability =
            Handle<DefaultProbabilityTermStructure>(
                ext::make_shared<FlatHazardRate>(0, WeekendsOnly(),
                                                   Handle<Quote>(flatRate), dayCounter));

        ext::shared_ptr<PricingEngine> engine;
        switch (model) {
          case Midpoint:
            engine = ext::make_shared<MidPointCdsEngine>(
                probability, conventionalRecovery, discountCurve);
            break;
          case ISDA:
            engine = ext::make_shared<IsdaCdsEngine>(
                probability, conventionalRecovery, discountCurve,
                boost::none,
                IsdaCdsEngine::Taylor,
                IsdaCdsEngine::HalfDayBias,
                IsdaCdsEngine::Piecewise);
            break;
          default:
            QL_FAIL("unknown CDS pricing model: " << model);
        }

        setupArguments(engine->getArguments());
        const auto* results = dynamic_cast<const CreditDefaultSwap::results*>(engine->getResults());

        ObjectiveFunction f(0., *flatRate, *engine, results);
        Rate guess = runningSpread_ / (1 - conventionalRecovery) * 365./360.;
        Real step = guess * 0.1;

        Brent().solve(f, 1e-9, guess, step);
        return results->fairSpread;
    }


    const Date& CreditDefaultSwap::protectionStartDate() const {
        return protectionStart_;
    }

    const Date& CreditDefaultSwap::protectionEndDate() const {
        return ext::dynamic_pointer_cast<Coupon>(leg_.back())
            ->accrualEndDate();
    }

    const ext::shared_ptr<SimpleCashFlow>& CreditDefaultSwap::upfrontPayment() const {
        return upfrontPayment_;
    }

    const ext::shared_ptr<SimpleCashFlow>& CreditDefaultSwap::accrualRebate() const {
        return accrualRebate_;
    }

    const Date& CreditDefaultSwap::tradeDate() const {
        return tradeDate_;
    }

    Natural CreditDefaultSwap::cashSettlementDays() const {
        return cashSettlementDays_;
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
        QL_REQUIRE(maturity != Null<Date>(),
                   "maturity date not set");
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
        accrualRebateNPV = Null<Real>();
    }

    Date cdsMaturity(const Date& tradeDate, const Period& tenor, DateGeneration::Rule rule) {

        QL_REQUIRE(rule == DateGeneration::CDS2015 || rule == DateGeneration::CDS || rule == DateGeneration::OldCDS,
            "cdsMaturity should only be used with date generation rule CDS2015, CDS or OldCDS");

        QL_REQUIRE(tenor.units() == Years || (tenor.units() == Months && tenor.length() % 3 == 0),
            "cdsMaturity expects a tenor that is a multiple of 3 months.");

        if (rule == DateGeneration::OldCDS) {
            QL_REQUIRE(tenor != 0 * Months, "A tenor of 0M is not supported for OldCDS.");
        }

        Date anchorDate = previousTwentieth(tradeDate, rule);
        if (rule == DateGeneration::CDS2015 && (anchorDate == Date(20, Dec, anchorDate.year()) ||
            anchorDate == Date(20, Jun, anchorDate.year()))) {
            if (tenor.length() == 0) {
                return Null<Date>();
            } else {
                anchorDate -= 3 * Months;
            }
        }

        Date maturity = anchorDate + tenor + 3 * Months;
        QL_REQUIRE(maturity > tradeDate, "error calculating CDS maturity. Tenor is " << tenor << ", trade date is " <<
            io::iso_date(tradeDate) << " generating a maturity of " << io::iso_date(maturity) << " <= trade date.");

        return maturity;
    }

}
