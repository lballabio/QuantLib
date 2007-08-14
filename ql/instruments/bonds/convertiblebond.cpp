/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2005, 2006 Theo Boafo
 Copyright (C) 2006, 2007 StatPro Italia srl

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

#include <ql/instruments/bonds/convertiblebond.hpp>
#include <ql/cashflows/cashflowvectors.hpp>
#include <ql/cashflows/couponpricer.hpp>
#include <ql/cashflows/simplecashflow.hpp>
#include <ql/processes/blackscholesprocess.hpp>
#include <ql/exercise.hpp>

namespace QuantLib {

    ConvertibleBond::ConvertibleBond(
            const boost::shared_ptr<StochasticProcess>& process,
            const boost::shared_ptr<Exercise>&,
            const boost::shared_ptr<PricingEngine>& engine,
            Real conversionRatio,
            const DividendSchedule& dividends,
            const CallabilitySchedule& callability,
            const Handle<Quote>& creditSpread,
            const Date& issueDate,
            Natural settlementDays,
            const DayCounter& dayCounter,
            const Schedule& schedule,
            Real)
    : Bond(settlementDays, schedule.calendar(), 100.0, issueDate),
      conversionRatio_(conversionRatio), callability_(callability),
      dividends_(dividends), creditSpread_(creditSpread) {

        //firstAccrualDate_ = schedule.startDate();
        maturityDate_     = schedule.endDate();

        setPricingEngine(engine);

        registerWith(process);
        registerWith(creditSpread);
    }

    void ConvertibleBond::performCalculations() const {
        option_->setPricingEngine(engine_);
        NPV_ = option_->NPV();
        errorEstimate_ = Null<Real>();
    }


    ConvertibleZeroCouponBond::ConvertibleZeroCouponBond(
                          const boost::shared_ptr<StochasticProcess>& process,
                          const boost::shared_ptr<Exercise>& exercise,
                          const boost::shared_ptr<PricingEngine>& engine,
                          Real conversionRatio,
                          const DividendSchedule& dividends,
                          const CallabilitySchedule& callability,
                          const Handle<Quote>& creditSpread,
                          const Date& issueDate,
                          Natural settlementDays,
                          const DayCounter& dayCounter,
                          const Schedule& schedule,
                          Real redemption)
    : ConvertibleBond(process, exercise, engine, conversionRatio,
                      dividends, callability, creditSpread, issueDate,
                      settlementDays, dayCounter, schedule, redemption) {

        cashflows_ = Leg();

        // redemption
        // !!!
        redemption *= faceAmount_/100.0;
        cashflows_.push_back(boost::shared_ptr<CashFlow>(new
            SimpleCashFlow(redemption, maturityDate_)));

        option_ = boost::shared_ptr<option>(
                           new option(this, process, exercise, engine,
                                      conversionRatio, dividends, callability,
                                      creditSpread, cashflows_, dayCounter,
                                      schedule, issueDate, settlementDays,
                                      redemption));
    }


    ConvertibleFixedCouponBond::ConvertibleFixedCouponBond(
                          const boost::shared_ptr<StochasticProcess>& process,
                          const boost::shared_ptr<Exercise>& exercise,
                          const boost::shared_ptr<PricingEngine>& engine,
                          Real conversionRatio,
                          const DividendSchedule& dividends,
                          const CallabilitySchedule& callability,
                          const Handle<Quote>& creditSpread,
                          const Date& issueDate,
                          Natural settlementDays,
                          const std::vector<Rate>& coupons,
                          const DayCounter& dayCounter,
                          const Schedule& schedule,
                          Real redemption)
    : ConvertibleBond(process, exercise, engine, conversionRatio,
                      dividends, callability, creditSpread, issueDate,
                      settlementDays, dayCounter, schedule, redemption) {

        // !!!
        cashflows_ = FixedRateLeg(std::vector<Real>(1, faceAmount_),
                                  schedule,
                                  coupons,
                                  dayCounter,
                                  schedule.businessDayConvention());

        // redemption
        // !!!
        redemption *= faceAmount_/100.0;
        cashflows_.push_back(boost::shared_ptr<CashFlow>(new
            SimpleCashFlow(redemption, maturityDate_)));

        option_ = boost::shared_ptr<option>(
                           new option(this, process, exercise, engine,
                                      conversionRatio, dividends, callability,
                                      creditSpread, cashflows_, dayCounter,
                                      schedule, issueDate, settlementDays,
                                      redemption));
    }


    ConvertibleFloatingRateBond::ConvertibleFloatingRateBond(
                          const boost::shared_ptr<StochasticProcess>& process,
                          const boost::shared_ptr<Exercise>& exercise,
                          const boost::shared_ptr<PricingEngine>& engine,
                          Real conversionRatio,
                          const DividendSchedule& dividends,
                          const CallabilitySchedule& callability,
                          const Handle<Quote>& creditSpread,
                          const Date& issueDate,
                          Natural settlementDays,
                          const boost::shared_ptr<IborIndex>& index,
                          Natural fixingDays,
                          const std::vector<Spread>& spreads,
                          const DayCounter& dayCounter,
                          const Schedule& schedule,
                          Real redemption)
    : ConvertibleBond(process, exercise, engine, conversionRatio,
                      dividends, callability, creditSpread, issueDate,
                      settlementDays, dayCounter, schedule, redemption) {

        cashflows_ = IborLeg(std::vector<Real>(1, faceAmount_),
                             schedule,
                             index,dayCounter,
                             schedule.businessDayConvention(),
                             std::vector<Natural>(1,fixingDays),
                             std::vector<Real>(1, 1.0), spreads);
        boost::shared_ptr<IborCouponPricer> fictitiousPricer(new
            BlackIborCouponPricer(Handle<CapletVolatilityStructure>()));
        setCouponPricer(cashflows_,fictitiousPricer);

        redemption *= faceAmount_/100.0;
        cashflows_.push_back(boost::shared_ptr<CashFlow>(
                              new SimpleCashFlow(redemption, maturityDate_)));

        option_ = boost::shared_ptr<option>(
                           new option(this, process, exercise, engine,
                                      conversionRatio, dividends, callability,
                                      creditSpread, cashflows_, dayCounter,
                                      schedule, issueDate, settlementDays,
                                      redemption));
    }

    ConvertibleBond::option::option(
            const ConvertibleBond* bond,
            const boost::shared_ptr<StochasticProcess>& process,
            const boost::shared_ptr<Exercise>& exercise,
            const boost::shared_ptr<PricingEngine>& engine,
            Real  conversionRatio,
            const DividendSchedule& dividends,
            const CallabilitySchedule& callability,
            const Handle<Quote>& creditSpread,
            const Leg& cashflows,
            const DayCounter& dayCounter,
            const Schedule& schedule,
            const Date& issueDate,
            Natural settlementDays,
            Real redemption)
    : OneAssetStrikedOption(process, boost::shared_ptr<StrikedTypePayoff>(new
          PlainVanillaPayoff(Option::Call,
                             bond->faceAmount()/100.0
                             *redemption/conversionRatio)),
                            exercise, engine),
      bond_(bond), conversionRatio_(conversionRatio),
      callability_(callability), dividends_(dividends),
      creditSpread_(creditSpread), cashflows_(cashflows),
      dayCounter_(dayCounter), issueDate_(issueDate), schedule_(schedule),
      settlementDays_(settlementDays), redemption_(redemption) {}



    void ConvertibleBond::option::setupArguments(
                                       PricingEngine::arguments* args) const {

        OneAssetStrikedOption::setupArguments(args);

        ConvertibleBond::option::arguments* moreArgs =
            dynamic_cast<ConvertibleBond::option::arguments*>(args);
        QL_REQUIRE(moreArgs != 0, "wrong argument type");

        moreArgs->conversionRatio = conversionRatio_;

        boost::shared_ptr<GeneralizedBlackScholesProcess> process =
            boost::dynamic_pointer_cast<GeneralizedBlackScholesProcess>(
                                                          stochasticProcess_);
        QL_REQUIRE(process, "Black-Scholes process required");

        Size i;
        Date settlement = bond_->settlementDate();
        DayCounter dayCounter = process->riskFreeRate()->dayCounter();

        moreArgs->stoppingTimes = std::vector<Time>(exercise_->dates().size());
        for (i=0; i<exercise_->dates().size(); i++) {
            moreArgs->stoppingTimes[i] =
                dayCounter.yearFraction(settlement, exercise_->date(i));
        }

        Size n = callability_.size();
        moreArgs->callabilityTimes.clear();
        moreArgs->callabilityTypes.clear();
        moreArgs->callabilityPrices.clear();
        moreArgs->callabilityTriggers.clear();
        moreArgs->callabilityTimes.reserve(n);
        moreArgs->callabilityTypes.reserve(n);
        moreArgs->callabilityPrices.reserve(n);
        moreArgs->callabilityTriggers.reserve(n);
        for (i=0; i<n; i++) {
            if (!callability_[i]->hasOccurred(settlement)) {
                moreArgs->callabilityTypes.push_back(callability_[i]->type());
                moreArgs->callabilityTimes.push_back(
                             dayCounter.yearFraction(settlement,
                                                     callability_[i]->date()));
                moreArgs->callabilityPrices.push_back(
                                            callability_[i]->price().amount());
                if (callability_[i]->price().type() ==
                                                    Callability::Price::Clean)
                    moreArgs->callabilityPrices.back() +=
                        bond_->accruedAmount(callability_[i]->date());
                boost::shared_ptr<SoftCallability> softCall =
                    boost::dynamic_pointer_cast<SoftCallability>(
                                                             callability_[i]);
                if (softCall)
                    moreArgs->callabilityTriggers.push_back(
                                                         softCall->trigger());
                else
                    moreArgs->callabilityTriggers.push_back(Null<Real>());
            }
        }

        const Leg& cashflows =
                                                           bond_->cashflows();
        moreArgs->couponTimes.clear();
        moreArgs->couponAmounts.clear();
        for (i=0; i<cashflows.size()-1; i++) {
            if (!cashflows[i]->hasOccurred(settlement)) {
                moreArgs->couponTimes.push_back(
                    dayCounter.yearFraction(settlement,cashflows[i]->date()));
                moreArgs->couponAmounts.push_back(cashflows[i]->amount());
            }
        }

        moreArgs->dividends.clear();
        moreArgs->dividendTimes.clear();
        for (i=0; i<dividends_.size(); i++) {
            if (!dividends_[i]->hasOccurred(settlement)) {
                moreArgs->dividends.push_back(dividends_[i]);
                moreArgs->dividendTimes.push_back(
                              dayCounter.yearFraction(settlement,
                                                      dividends_[i]->date()));
            }
        }

        moreArgs->creditSpread = creditSpread_;
        moreArgs->dayCounter = dayCounter_;
        moreArgs->issueDate = issueDate_;
        moreArgs->settlementDate = settlement;
        moreArgs->settlementDays = settlementDays_;
        moreArgs->redemption = redemption_;
    }


    void ConvertibleBond::option::arguments::validate() const {

        OneAssetStrikedOption::arguments::validate();

        QL_REQUIRE(conversionRatio != Null<Real>(), "null conversion ratio");
        QL_REQUIRE(conversionRatio > 0.0,
                   "positive conversion ratio required: "
                   << conversionRatio << " not allowed");

        QL_REQUIRE(redemption != Null<Real>(), "null redemption");
        QL_REQUIRE(redemption >= 0.0,
                   "positive redemption required: "
                   << redemption << " not allowed");

        QL_REQUIRE(settlementDate != Date(), "null settlement date");

        QL_REQUIRE(settlementDays != Null<Natural>(), "null settlement days");

        QL_REQUIRE(callabilityTimes.size() == callabilityTypes.size(),
                   "different number of callability times and types");
        QL_REQUIRE(callabilityTimes.size() == callabilityPrices.size(),
                   "different number of callability times and prices");
        QL_REQUIRE(callabilityTimes.size() == callabilityTriggers.size(),
                   "different number of callability times and triggers");

        QL_REQUIRE(couponTimes.size() == couponAmounts.size(),
                   "different number of coupon times and amounts");
    }

}
