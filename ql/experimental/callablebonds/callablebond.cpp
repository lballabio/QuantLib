/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Allen Kuo
 Copyright (C) 2017 BN Algorithms Ltd

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

#include <ql/cashflows/cashflowvectors.hpp>
#include <ql/experimental/callablebonds/blackcallablebondengine.hpp>
#include <ql/experimental/callablebonds/callablebond.hpp>
#include <ql/math/solvers1d/brent.hpp>
#include <ql/termstructures/yield/zerospreadedtermstructure.hpp>
#include <utility>

namespace QuantLib {

    CallableBond::CallableBond(Natural settlementDays,
                               const Schedule& schedule,
                               DayCounter paymentDayCounter,
                               const Date& issueDate,
                               CallabilitySchedule putCallSchedule)
    : Bond(settlementDays, schedule.calendar(), issueDate),
      paymentDayCounter_(std::move(paymentDayCounter)),
      putCallSchedule_(std::move(putCallSchedule)) {

        maturityDate_ = schedule.dates().back();

        if (!putCallSchedule_.empty()) {
            Date finalOptionDate = Date::minDate();
            for (auto& i : putCallSchedule_) {
                finalOptionDate = std::max(finalOptionDate, i->date());
            }
            QL_REQUIRE(finalOptionDate <= maturityDate_ ,
                       "Bond cannot mature before last call/put date");
        }

        // derived classes must set cashflows_ and frequency_
    }


    void CallableBond::arguments::validate() const {

        QL_REQUIRE(Bond::arguments::settlementDate != Date(),
                   "null settlement date");

        QL_REQUIRE(redemption != Null<Real>(), "null redemption");
        QL_REQUIRE(redemption >= 0.0,
                   "positive redemption required: "
                   << redemption << " not allowed");

        QL_REQUIRE(callabilityDates.size() == callabilityPrices.size(),
                   "different number of callability dates and prices");
        QL_REQUIRE(couponDates.size() == couponAmounts.size(),
                   "different number of coupon dates and amounts");
    }


    CallableBond::ImpliedVolHelper::ImpliedVolHelper(
                              const CallableBond& bond,
                              Real targetValue)
    : targetValue_(targetValue) {

        vol_ = ext::make_shared<SimpleQuote>(0.0);
        bond.blackVolQuote_.linkTo(vol_);

        QL_REQUIRE(bond.blackEngine_,
                   "Must set blackEngine_ to use impliedVolatility");

        engine_ = bond.blackEngine_;
        bond.setupArguments(engine_->getArguments());
        results_ =
            dynamic_cast<const Instrument::results*>(engine_->getResults());
    }


    Real CallableBond::ImpliedVolHelper::operator()(Volatility x) const {
        vol_->setValue(x);
        engine_->calculate(); // get the Black NPV based on vol x
        return results_->value-targetValue_;
    }

    Volatility CallableBond::impliedVolatility(
                              Real targetValue,
                              const Handle<YieldTermStructure>& discountCurve,
                              Real accuracy,
                              Size maxEvaluations,
                              Volatility minVol,
                              Volatility maxVol) const {
        calculate();
        QL_REQUIRE(!isExpired(), "instrument expired");
        Volatility guess = 0.5*(minVol + maxVol);
        blackDiscountCurve_.linkTo(*discountCurve, false);
        ImpliedVolHelper f(*this,targetValue);
        Brent solver;
        solver.setMaxEvaluations(maxEvaluations);
        return solver.solve(f, accuracy, guess, minVol, maxVol);
    }

    namespace {

    template<class T>
    class RestoreVal {
        T orig_;
        T &ref_;
    public:
        explicit RestoreVal(T &ref):
            orig_(ref),
            ref_(ref)  { }
        ~RestoreVal()
        {
            ref_=orig_;
        }
    };

    class OASHelper {
    public:
        OASHelper(const ext::function<Real(Real)>& npvhelper,
                  Real targetValue):
            npvhelper_(npvhelper),
            targetValue_(targetValue)
        {
        }

        Real operator()(Spread x) const
        {
            return targetValue_ - npvhelper_(x);
        }
    private:
        const ext::function<Real(Real)>& npvhelper_;
        Real targetValue_;
    };


    /* Convert a continuous spread to a conventional spread to a
       reference yield curve
    */
    Real continuousToConv(Real oas,
                          const Bond &b,
                          const Handle<YieldTermStructure>& yts,
                          const DayCounter& dayCounter,
                          Compounding compounding,
                          Frequency frequency)
    {
        double zz=yts->zeroRate(b.maturityDate(),
                                dayCounter,
                                Continuous,
                                NoFrequency);
        InterestRate baseRate(zz,
                              dayCounter,
                              Continuous,
                              NoFrequency);
        InterestRate spreadedRate(oas+zz,
                                  dayCounter,
                                  Continuous,
                                  NoFrequency);
        double br=baseRate.equivalentRate(dayCounter,
                                          compounding,
                                          frequency,
                                          yts->referenceDate(),
                                          b.maturityDate()).rate();
        double sr=spreadedRate.equivalentRate(dayCounter,
                                              compounding,
                                              frequency,
                                              yts->referenceDate(),
                                              b.maturityDate()).rate();
        // Return the spread
        return sr-br;
    }

    /* Convert a conventional spread to a reference yield curve to a
       continuous spread
    */
    Real convToContinuous(Real oas,
                          const Bond &b,
                          const Handle<YieldTermStructure>& yts,
                          const DayCounter& dayCounter,
                          Compounding compounding,
                          Frequency frequency)
    {
        double zz=yts->zeroRate(b.maturityDate(),
                                dayCounter,
                                compounding,
                                frequency);
        InterestRate baseRate(zz,
                              dayCounter,
                              compounding,
                              frequency);

        InterestRate spreadedRate(oas+zz,
                                  dayCounter,
                                  compounding,
                                  frequency);
        double br=baseRate.equivalentRate(dayCounter,
                                          Continuous,
                                          NoFrequency,
                                          yts->referenceDate(),
                                          b.maturityDate()).rate();
        double sr=spreadedRate.equivalentRate(dayCounter,
                                              Continuous,
                                              NoFrequency,
                                              yts->referenceDate(),
                                              b.maturityDate()).rate();
        // Return the spread
        return sr-br;
    }

    }


    CallableBond::NPVSpreadHelper::NPVSpreadHelper(CallableBond& bond):
        bond_(bond),
        results_(dynamic_cast<const Instrument::results*>(bond.engine_->getResults()))
    {
        bond.setupArguments(bond.engine_->getArguments());
    }

   Real CallableBond::NPVSpreadHelper::operator()(Real x) const
   {
       auto* args = dynamic_cast<CallableBond::arguments*>(bond_.engine_->getArguments());
       // Pops the original value when function finishes
       RestoreVal<Spread> restorer(args->spread);
       args->spread=x;
       bond_.engine_->calculate();
       return results_->value;
   }

    Spread CallableBond::OAS(Real cleanPrice,
                             const Handle<YieldTermStructure>& engineTS,
                             const DayCounter& dayCounter,
                             Compounding compounding,
                             Frequency frequency,
                             Date settlement,
                             Real accuracy,
                             Size maxIterations,
                             Spread guess)
    {
        if (settlement == Date())
            settlement = settlementDate();

        Real dirtyPrice = cleanPrice + accruedAmount(settlement);

        ext::function<Real(Real)> f = NPVSpreadHelper(*this);
        OASHelper obj(f, dirtyPrice);

        Brent solver;
        solver.setMaxEvaluations(maxIterations);

        Real step = 0.001;
        Spread oas=solver.solve(obj, accuracy, guess, step);

        return continuousToConv(oas,
                                *this,
                                engineTS,
                                dayCounter,
                                compounding,
                                frequency);
    }



    Real CallableBond::cleanPriceOAS(Real oas,
                                     const Handle<YieldTermStructure>& engineTS,
                                     const DayCounter& dayCounter,
                                     Compounding compounding,
                                     Frequency frequency,
                                     Date settlement)
    {
        if (settlement == Date())
            settlement = settlementDate();

        oas=convToContinuous(oas,
                             *this,
                             engineTS,
                             dayCounter,
                             compounding,
                             frequency);

        ext::function<Real(Real)> f = NPVSpreadHelper(*this);

        Real P = f(oas) - accruedAmount(settlement);

        return P;
    }

    Real CallableBond::effectiveDuration(Real oas,
                                         const Handle<YieldTermStructure>& engineTS,
                                         const DayCounter& dayCounter,
                                         Compounding compounding,
                                         Frequency frequency,
                                         Real bump)
    {
        Real P = cleanPriceOAS(oas,
                               engineTS,
                               dayCounter,
                               compounding,
                               frequency);

        Real Ppp = cleanPriceOAS(oas+bump,
                                 engineTS,
                                 dayCounter,
                                 compounding,
                                 frequency);
        Real Pmm = cleanPriceOAS(oas-bump,
                                 engineTS,
                                 dayCounter,
                                 compounding,
                                 frequency);
            
        if ( P == 0.0 )
            return 0;
        else
            {
                return (Pmm-Ppp)/(2*P*bump);
            }
    }

    Real CallableBond::effectiveConvexity(Real oas,
                                          const Handle<YieldTermStructure>& engineTS,
                                          const DayCounter& dayCounter,
                                          Compounding compounding,
                                          Frequency frequency,
                                          Real bump)
    {
        Real P = cleanPriceOAS(oas,
                               engineTS,
                               dayCounter,
                               compounding,
                               frequency);

        Real Ppp = cleanPriceOAS(oas+bump,
                                 engineTS,
                                 dayCounter,
                                 compounding,
                                 frequency);
        Real Pmm = cleanPriceOAS(oas-bump,
                                 engineTS,
                                 dayCounter,
                                 compounding,
                                 frequency);
            
        if ( P == 0.0 )
            return 0;
        else
            {
                return (Ppp + Pmm - 2*P) / ( std::pow(bump,2) * P);
            }        

    }


    CallableFixedRateBond::CallableFixedRateBond(
                              Natural settlementDays,
                              Real faceAmount,
                              const Schedule& schedule,
                              const std::vector<Rate>& coupons,
                              const DayCounter& accrualDayCounter,
                              BusinessDayConvention paymentConvention,
                              Real redemption,
                              const Date& issueDate,
                              const CallabilitySchedule& putCallSchedule,
                              const Period& exCouponPeriod,
                              const Calendar& exCouponCalendar,
                              BusinessDayConvention exCouponConvention,
                              bool exCouponEndOfMonth)
    : CallableBond(settlementDays, schedule, accrualDayCounter,
                   issueDate, putCallSchedule) {

        frequency_ = schedule.tenor().frequency();

        bool isZeroCouponBond = (coupons.size() == 1 && close(coupons[0], 0.0));

        if (!isZeroCouponBond) {
            cashflows_ =
                FixedRateLeg(schedule)
                .withNotionals(faceAmount)
                .withCouponRates(coupons, accrualDayCounter)
                .withPaymentAdjustment(paymentConvention)
                .withExCouponPeriod(exCouponPeriod,
                                    exCouponCalendar,
                                    exCouponConvention,
                                    exCouponEndOfMonth);

            addRedemptionsToCashflows(std::vector<Real>(1, redemption));
        } else {
            Date redemptionDate = calendar_.adjust(maturityDate_,
                                                   paymentConvention);
            setSingleRedemption(faceAmount, redemption, redemptionDate);
        }

        // used for impliedVolatility() calculation
        ext::shared_ptr<SimpleQuote> dummyVolQuote(new SimpleQuote(0.));
        blackVolQuote_.linkTo(dummyVolQuote);
        blackEngine_ = ext::shared_ptr<PricingEngine>(
                   new BlackCallableFixedRateBondEngine(blackVolQuote_,
                                                        blackDiscountCurve_));
    }


    Real CallableFixedRateBond::accrued(Date settlement) const {

        if (settlement == Date()) settlement = settlementDate();

        const bool IncludeToday = false;
        for (const auto& cashflow : cashflows_) {
            // the first coupon paying after d is the one we're after
            if (!cashflow->hasOccurred(settlement, IncludeToday)) {
                ext::shared_ptr<Coupon> coupon = ext::dynamic_pointer_cast<Coupon>(cashflow);
                if (coupon != nullptr)
                    // !!!
                    return coupon->accruedAmount(settlement) /
                           notional(settlement) * 100.0;
                else
                    return 0.0;
            }
        }
        return 0.0;
    }


    void CallableFixedRateBond::setupArguments(
                                       PricingEngine::arguments* args) const {

        CallableBond::setupArguments(args);

        auto* arguments = dynamic_cast<CallableBond::arguments*>(args);

        QL_REQUIRE(arguments != nullptr, "no arguments given");

        Date settlement = arguments->settlementDate;

        arguments->redemption = redemption()->amount();
        arguments->redemptionDate = redemption()->date();

        const Leg& cfs = cashflows();

        arguments->couponDates.clear();
        arguments->couponDates.reserve(cfs.size()-1);
        arguments->couponAmounts.clear();
        arguments->couponAmounts.reserve(cfs.size()-1);

        for (Size i=0; i<cfs.size()-1; i++) {
            if (!cfs[i]->hasOccurred(settlement, false)
                && !cfs[i]->tradingExCoupon(settlement)) {
                arguments->couponDates.push_back(cfs[i]->date());
                arguments->couponAmounts.push_back(cfs[i]->amount());
            }
        }

        arguments->callabilityPrices.clear();
        arguments->callabilityDates.clear();
        arguments->callabilityPrices.reserve(putCallSchedule_.size());
        arguments->callabilityDates.reserve(putCallSchedule_.size());

        arguments->paymentDayCounter = paymentDayCounter_;
        arguments->frequency = frequency_;

        arguments->putCallSchedule = putCallSchedule_;
        for (const auto& i : putCallSchedule_) {
            if (!i->hasOccurred(settlement, false)) {
                arguments->callabilityDates.push_back(i->date());
                arguments->callabilityPrices.push_back(i->price().amount());

                if (i->price().type() == Bond::Price::Clean) {
                    /* calling accrued() forces accrued interest to be zero
                       if future option date is also coupon date, so that dirty
                       price = clean price. Use here because callability is
                       always applied before coupon in the tree engine.
                    */
                    arguments->callabilityPrices.back() += this->accrued(i->date());
                }
            }
        }

        arguments->spread = 0.0;
    }


    CallableZeroCouponBond::CallableZeroCouponBond(
                              Natural settlementDays,
                              Real faceAmount,
                              const Calendar& calendar,
                              const Date& maturityDate,
                              const DayCounter& dayCounter,
                              BusinessDayConvention paymentConvention,
                              Real redemption,
                              const Date& issueDate,
                              const CallabilitySchedule& putCallSchedule)
    : CallableFixedRateBond(settlementDays,faceAmount,
                            Schedule(issueDate, maturityDate,
                                     Period(Once),
                                     calendar,
                                     paymentConvention,
                                     paymentConvention,
                                     DateGeneration::Backward,
                                     false),
                            std::vector<Rate>(1, 0.0), dayCounter,
                            paymentConvention, redemption,
                            issueDate, putCallSchedule) {}

}

