/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2005, 2006 Theo Boafo

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

namespace QuantLib {

    // constructor for fixed rate coupon convertible bond.
    ConvertibleBond::ConvertibleBond(
            const boost::shared_ptr<StochasticProcess>& process,
            const boost::shared_ptr<StrikedTypePayoff>& payoff,
            const boost::shared_ptr<Exercise>& exercise,
            const boost::shared_ptr<PricingEngine>& engine,
            Real  conversionRatio,
            const DividendSchedule&  dividends,
            const CallabilitySchedule& callability,
            const Handle<Quote>& creditSpread,
            const Date& issueDate,
            Integer settlementDays,
            const std::vector<Rate>& coupons,
            const DayCounter& dayCounter,
            const Schedule& schedule,
            Real redemption,
            const Handle<YieldTermStructure>& discountCurve)
    : Bond(dayCounter, schedule.calendar(), schedule.businessDayConvention(),
           schedule.businessDayConvention(), settlementDays,discountCurve),
      conversionRatio_(conversionRatio), callability_(callability),
      dividends_(dividends), creditSpread_(creditSpread) {

        issueDate_ = issueDate;
        datedDate_ = schedule.startDate();
        maturityDate_ =schedule.endDate();
        frequency_ = schedule.frequency();
        redemption_ = boost::shared_ptr<CashFlow>(
                                new SimpleCashFlow(redemption,maturityDate_));

        cashFlows_ =
            FixedRateCouponVector(schedule, schedule.businessDayConvention(),
                                  std::vector<Real>(1, redemption),
                                  coupons,dayCounter);

        option_ = boost::shared_ptr<option>(
                           new option(this, process, payoff, exercise, engine,
                                      conversionRatio, dividends, callability,
                                      creditSpread, cashFlows_, dayCounter,
                                      schedule, issueDate, settlementDays,
                                      redemption, discountCurve));

    }


    // constructor for floating rate coupon convertible bond.
    ConvertibleBond::ConvertibleBond(
            const boost::shared_ptr<StochasticProcess>& process,
            const boost::shared_ptr<StrikedTypePayoff>& payoff,
            const boost::shared_ptr<Exercise>& exercise,
            const boost::shared_ptr<PricingEngine>& engine,
            Real  conversionRatio,
            const DividendSchedule&  dividends,
            const CallabilitySchedule& callability,
            const Handle<Quote>& creditSpread,
            const Date& issueDate,
            Integer settlementDays,
            const boost::shared_ptr<Xibor>& index,
            Integer fixingDays,
            const std::vector<Spread>& spreads,
            const DayCounter& dayCounter,
            const Schedule& schedule,
            Real redemption,
            const Handle<YieldTermStructure>& discountCurve)
    : Bond(dayCounter, schedule.calendar(), schedule.businessDayConvention(),
           schedule.businessDayConvention(), settlementDays, discountCurve),
      conversionRatio_(conversionRatio), callability_(callability),
      dividends_(dividends), creditSpread_(creditSpread) {

        issueDate_ = issueDate;
        datedDate_ = schedule.startDate();
        maturityDate_ = schedule.endDate();
        frequency_ = schedule.frequency();
        redemption_ = boost::shared_ptr<CashFlow>(
                                new SimpleCashFlow(redemption,maturityDate_));

        cashFlows_ = IndexedCouponVector<UpFrontIndexedCoupon>(
                                   schedule, schedule.businessDayConvention(),
                                   std::vector<Real>(1, 100.0),
                                   index, fixingDays,
                                   spreads, dayCounter
                                   #ifdef QL_PATCH_MSVC6
                                   , (const UpFrontIndexedCoupon*) 0
                                   #endif
                                   );

        option_ = boost::shared_ptr<option>(
                           new option(this, process, payoff, exercise, engine,
                                      conversionRatio, dividends, callability,
                                      creditSpread, cashFlows_, dayCounter,
                                      schedule, issueDate, settlementDays,
                                      redemption, discountCurve));

    }

    // constructor for zero coupon convertible bond.
    ConvertibleBond::ConvertibleBond(
            const boost::shared_ptr<StochasticProcess>& process,
            const boost::shared_ptr<StrikedTypePayoff>& payoff,
            const boost::shared_ptr<Exercise>& exercise,
            const boost::shared_ptr<PricingEngine>& engine,
            Real  conversionRatio,
            const DividendSchedule&  dividends,
            const CallabilitySchedule& callability,
            const Handle<Quote>& creditSpread,
            const Date& issueDate,
            Integer settlementDays,
            const DayCounter& dayCounter,
            const Schedule& schedule,
            Real  redemption,
            const Handle<YieldTermStructure>& discountCurve)
    : Bond(dayCounter, schedule.calendar(), schedule.businessDayConvention(),
           schedule.businessDayConvention(), settlementDays,discountCurve),
      conversionRatio_(conversionRatio), callability_(callability),
      dividends_(dividends), creditSpread_(creditSpread) {

        issueDate_ = issueDate;
        datedDate_ = schedule.startDate();
        maturityDate_ = schedule.endDate();
        frequency_ = schedule.frequency();
        redemption_ = boost::shared_ptr<CashFlow>(
                                new SimpleCashFlow(redemption,maturityDate_));

        cashFlows_ = std::vector<boost::shared_ptr<CashFlow> >();

        option_ = boost::shared_ptr<option>(
                           new option(this, process, payoff, exercise, engine,
                                      conversionRatio, dividends, callability,
                                      creditSpread, cashFlows_, dayCounter,
                                      schedule, issueDate, settlementDays,
                                      redemption, discountCurve));

    }

    void ConvertibleBond::performCalculations() const {
        NPV_ = option_->NPV();
        errorEstimate_ = Null<Real>();
    }


    // Constructor for option class in ConvertibleBond class
    ConvertibleBond::option::option(
            const ConvertibleBond* bond,
            const boost::shared_ptr<StochasticProcess>& process,
            const boost::shared_ptr<StrikedTypePayoff>& payoff,
            const boost::shared_ptr<Exercise>& exercise,
            const boost::shared_ptr<PricingEngine>& engine,
            Real  conversionRatio,
            const DividendSchedule&  dividends,
            const CallabilitySchedule& callability,
            const Handle<Quote>& creditSpread,
            const std::vector<boost::shared_ptr<CashFlow> >& cashFlows,
            const DayCounter& dayCounter,
            const Schedule& schedule,
            const Date& issueDate,
            Integer settlementDays,
            Real redemption,
            const Handle<YieldTermStructure>& discountCurve)
    : OneAssetStrikedOption(process, payoff, exercise, engine),
      bond_(bond), conversionRatio_(conversionRatio),
      callability_(callability), dividends_(dividends),
      creditSpread_(creditSpread), cashFlows_(cashFlows),
      dayCounter_(dayCounter), issueDate_(issueDate), schedule_(schedule),
      discountCurve_(discountCurve), settlementDays_(settlementDays),
      redemption_(redemption) {}



    void ConvertibleBond::option::setupArguments(Arguments* args) const {

        OneAssetStrikedOption::setupArguments(args);

        ConvertibleBond::option::arguments* moreArgs =
            dynamic_cast<ConvertibleBond::option::arguments*>(args);
        QL_REQUIRE(moreArgs != 0, "wrong argument type");

        moreArgs->conversionRatio = conversionRatio_;

        moreArgs->dividends.clear();
        moreArgs->callability.clear();
        moreArgs->accruedAmounts.clear();
        moreArgs->cashFlows.clear();
        moreArgs->schedule.clear();

        moreArgs->dividends = dividends_;
        moreArgs->callability = callability_;
        for (Size i=0; i<callability_.size(); i++)
            moreArgs->accruedAmounts.push_back(
                                bond_->accruedAmount(callability_[i].date()));
        moreArgs->cashFlows = cashFlows_;
        moreArgs->creditSpread = creditSpread_;
        moreArgs->dayCounter = dayCounter_;
        std::copy(schedule_.begin(), schedule_.end(),
                  std::back_inserter(moreArgs->schedule));
        moreArgs->issueDate = issueDate_;
        moreArgs->settlementDays = settlementDays_;
        moreArgs->discountCurve = discountCurve_;
        moreArgs->redemption = redemption_;

    }


    void ConvertibleBond::option::arguments::validate() const {

        #if defined(QL_PATCH_MSVC6)
        OneAssetStrikedOption::arguments copy = *this;
        copy.validate();
        #else
        OneAssetStrikedOption::arguments::validate();
        #endif

        QL_REQUIRE(conversionRatio != Null<Real>(), "null conversionRatio");

        QL_REQUIRE(conversionRatio > 0.0,
                   "positive conversionRatio required: "
                   << conversionRatio << " not allowed");

    }

}

