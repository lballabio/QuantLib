/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2005, 2006 Theo Boafo
 Copyright (C) 2006 StatPro Italia srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/reference/license.html>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/Instruments/convertiblebond.hpp>
#include <ql/CashFlows/cashflowvectors.hpp>
#include <ql/CashFlows/indexedcashflowvectors.hpp>
#include <ql/CashFlows/upfrontindexedcoupon.hpp>
#include <ql/CashFlows/simplecashflow.hpp>
#include <ql/Processes/blackscholesprocess.hpp>

namespace QuantLib {

    ConvertibleBond::ConvertibleBond(
            const boost::shared_ptr<StochasticProcess>& process,
            const boost::shared_ptr<Exercise>&,
            const boost::shared_ptr<PricingEngine>& engine,
            Real conversionRatio,
            const DividendSchedule&  dividends,
            const CallabilitySchedule& callability,
            const Handle<Quote>& creditSpread,
            const Date& issueDate,
            Integer settlementDays,
            const DayCounter& dayCounter,
            const Schedule& schedule,
            Real redemption)
    : Bond(dayCounter, schedule.calendar(), schedule.businessDayConvention(),
           schedule.businessDayConvention(), settlementDays),
      conversionRatio_(conversionRatio), callability_(callability),
      dividends_(dividends), creditSpread_(creditSpread) {

        issueDate_ = issueDate;
        datedDate_ = schedule.startDate();
        maturityDate_ = schedule.endDate();
        frequency_ = schedule.frequency();

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
                          const DividendSchedule&  dividends,
                          const CallabilitySchedule& callability,
                          const Handle<Quote>& creditSpread,
                          const Date& issueDate,
                          Integer settlementDays,
                          const DayCounter& dayCounter,
                          const Schedule& schedule,
                          Real redemption)
    : ConvertibleBond(process, exercise, engine, conversionRatio,
                      dividends, callability, creditSpread, issueDate,
                      settlementDays, dayCounter, schedule, redemption) {

        cashflows_ = std::vector<boost::shared_ptr<CashFlow> >();
        // redemption
        cashflows_.push_back(boost::shared_ptr<CashFlow>(
                               new SimpleCashFlow(redemption,maturityDate_)));

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
                          const DividendSchedule&  dividends,
                          const CallabilitySchedule& callability,
                          const Handle<Quote>& creditSpread,
                          const Date& issueDate,
                          Integer settlementDays,
                          const std::vector<Rate>& coupons,
                          const DayCounter& dayCounter,
                          const Schedule& schedule,
                          Real redemption)
    : ConvertibleBond(process, exercise, engine, conversionRatio,
                      dividends, callability, creditSpread, issueDate,
                      settlementDays, dayCounter, schedule, redemption) {

        cashflows_ =
            FixedRateCouponVector(schedule, schedule.businessDayConvention(),
                                  std::vector<Real>(1, 100.0),
                                  coupons, dayCounter);
        // redemption
        cashflows_.push_back(boost::shared_ptr<CashFlow>(
                               new SimpleCashFlow(redemption,maturityDate_)));

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
                          Integer settlementDays,
                          const boost::shared_ptr<Xibor>& index,
                          Integer fixingDays,
                          const std::vector<Spread>& spreads,
                          const DayCounter& dayCounter,
                          const Schedule& schedule,
                          Real redemption)
    : ConvertibleBond(process, exercise, engine, conversionRatio,
                      dividends, callability, creditSpread, issueDate,
                      settlementDays, dayCounter, schedule, redemption) {

        cashflows_ = IndexedCouponVector<UpFrontIndexedCoupon>(
                                   schedule, schedule.businessDayConvention(),
                                   std::vector<Real>(1, 100.0),
                                   index, fixingDays,
                                   spreads, dayCounter
                                   #ifdef QL_PATCH_MSVC6
                                   , (const UpFrontIndexedCoupon*) 0
                                   #endif
                                   );
        // redemption
        cashflows_.push_back(boost::shared_ptr<CashFlow>(
                               new SimpleCashFlow(redemption,maturityDate_)));

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
            const DividendSchedule&  dividends,
            const CallabilitySchedule& callability,
            const Handle<Quote>& creditSpread,
            const std::vector<boost::shared_ptr<CashFlow> >& cashflows,
            const DayCounter& dayCounter,
            const Schedule& schedule,
            const Date& issueDate,
            Integer settlementDays,
            Real redemption)
    : OneAssetStrikedOption(
                    process, boost::shared_ptr<StrikedTypePayoff>(
                          new PlainVanillaPayoff(Option::Call,
                                                 redemption/conversionRatio)),
                    exercise, engine),
      bond_(bond), conversionRatio_(conversionRatio),
      callability_(callability), dividends_(dividends),
      creditSpread_(creditSpread), cashflows_(cashflows),
      dayCounter_(dayCounter), issueDate_(issueDate), schedule_(schedule),
      settlementDays_(settlementDays), redemption_(redemption) {}



    void ConvertibleBond::option::setupArguments(Arguments* args) const {

        OneAssetStrikedOption::setupArguments(args);

        ConvertibleBond::option::arguments* moreArgs =
            dynamic_cast<ConvertibleBond::option::arguments*>(args);
        QL_REQUIRE(moreArgs != 0, "wrong argument type");

        moreArgs->conversionRatio = conversionRatio_;
        moreArgs->dividends = dividends_;

        boost::shared_ptr<BlackScholesProcess> process =
            boost::dynamic_pointer_cast<BlackScholesProcess>(
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

        moreArgs->callabilityTimes.clear();
        moreArgs->callabilityTypes.clear();
        moreArgs->callabilityPrices.clear();
        for (i=0; i<callability_.size(); i++) {
            if (!callability_[i].hasOccurred(settlement)) {
                moreArgs->callabilityTypes.push_back(callability_[i].type());
                moreArgs->callabilityTimes.push_back(
                             dayCounter.yearFraction(settlement,
                                                     callability_[i].date()));
                moreArgs->callabilityPrices.push_back(
                                            callability_[i].price().amount());
                if (callability_[i].price().type() == Price::Clean)
                    moreArgs->callabilityPrices.back() +=
                        bond_->accruedAmount(callability_[i].date());
            }
        }

        const std::vector<boost::shared_ptr<CashFlow> >& cashflows =
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

        moreArgs->creditSpread = creditSpread_;
        moreArgs->dayCounter = dayCounter_;
        moreArgs->issueDate = issueDate_;
        moreArgs->settlementDate = settlement;
        moreArgs->settlementDays = settlementDays_;
        moreArgs->redemption = redemption_;
    }


    void ConvertibleBond::option::arguments::validate() const {

        #if defined(QL_PATCH_MSVC6)
        OneAssetStrikedOption::arguments copy = *this;
        copy.validate();
        #else
        OneAssetStrikedOption::arguments::validate();
        #endif

        QL_REQUIRE(conversionRatio != Null<Real>(), "null conversion ratio");
        QL_REQUIRE(conversionRatio > 0.0,
                   "positive conversion ratio required: "
                   << conversionRatio << " not allowed");

        QL_REQUIRE(redemption != Null<Real>(), "null redemption");
        QL_REQUIRE(redemption >= 0.0,
                   "positive redemption required: "
                   << redemption << " not allowed");

        QL_REQUIRE(settlementDate != Date(), "null settlement date");

        QL_REQUIRE(settlementDays != Null<Integer>(), "null settlement days");
        QL_REQUIRE(settlementDays >= 0,
                   "positive settlement days required: "
                   << settlementDays << " not allowed");

        QL_REQUIRE(callabilityTimes.size() == callabilityTypes.size(),
                   "different number of callability times and types");
        QL_REQUIRE(callabilityTimes.size() == callabilityPrices.size(),
                   "different number of callability times and prices");

        QL_REQUIRE(couponTimes.size() == couponAmounts.size(),
                   "different number of coupon times and amounts");
    }

}

