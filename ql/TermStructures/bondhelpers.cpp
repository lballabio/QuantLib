/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2005 Toyin Akin

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

#include <ql/TermStructures/bondhelpers.hpp>

namespace QuantLib {

    namespace {
        void no_deletion(YieldTermStructure*) {}
    }

    #ifndef QL_DISABLE_DEPRECATED
    FixedCouponBondHelper::FixedCouponBondHelper(
                                             const Handle<Quote>& cleanPrice,
                                             const Date& issueDate,
                                             const Date& datedDate,
                                             const Date& maturityDate,
                                             Integer settlementDays,
                                             const std::vector<Rate>& coupons,
                                             Frequency frequency,
                                             const DayCounter& dayCounter,
                                             const Calendar& calendar,
                                             BusinessDayConvention convention,
                                             Real redemption,
                                             const Date& stub, bool fromEnd)
	: RateHelper(cleanPrice),
      issueDate_(issueDate), datedDate_(datedDate),
      maturityDate_(maturityDate), settlementDays_(settlementDays),
      coupons_(coupons), frequency_(frequency), dayCounter_(dayCounter),
      calendar_(calendar), accrualConvention_(convention),
      paymentConvention_(convention), redemption_(redemption),
      stub_(stub), fromEnd_(fromEnd) {

		registerWith(Settings::instance().evaluationDate());
	}
    #endif

    FixedCouponBondHelper::FixedCouponBondHelper(
                                      const Handle<Quote>& cleanPrice,
                                      const Date& issueDate,
                                      const Date& datedDate,
                                      const Date& maturityDate,
                                      Integer settlementDays,
                                      const std::vector<Rate>& coupons,
                                      Frequency frequency,
                                      const Calendar& calendar,
                                      const DayCounter& dayCounter,
                                      BusinessDayConvention accrualConvention,
                                      BusinessDayConvention paymentConvention,
                                      Real redemption,
                                      const Date& stub, bool fromEnd)
	: RateHelper(cleanPrice),
      issueDate_(issueDate), datedDate_(datedDate),
      maturityDate_(maturityDate), settlementDays_(settlementDays),
      coupons_(coupons), frequency_(frequency), dayCounter_(dayCounter),
      calendar_(calendar), accrualConvention_(accrualConvention),
      paymentConvention_(paymentConvention), redemption_(redemption),
      stub_(stub), fromEnd_(fromEnd) {

		registerWith(Settings::instance().evaluationDate());
	}

    void FixedCouponBondHelper::setTermStructure(YieldTermStructure* t) {
        // do not set the relinkable handle as an observer -
        // force recalculation when needed
        termStructureHandle_.linkTo(
                 boost::shared_ptr<YieldTermStructure>(t,no_deletion), false);

        RateHelper::setTermStructure(t);
        Date today = Settings::instance().evaluationDate();
        settlement_ = calendar_.advance(today,settlementDays_,Days);

        bond_ = boost::shared_ptr<FixedCouponBond>(
                   new FixedCouponBond(issueDate_, datedDate_, maturityDate_,
                                       settlementDays_, coupons_,
                                       frequency_, calendar_, dayCounter_,
                                       accrualConvention_, paymentConvention_,
                                       redemption_, termStructureHandle_,
                                       stub_, fromEnd_));
        latestDate_ = maturityDate_;
    }

    Date FixedCouponBondHelper::latestDate() const {
        QL_REQUIRE(termStructure_ != 0, "null term structure set");
        return latestDate_;
    }

    Real FixedCouponBondHelper::impliedQuote() const {
        QL_REQUIRE(termStructure_ != 0, "term structure not set");
        // we didn't register as observers - force calculation
        bond_->recalculate();
        return bond_->cleanPrice();
    }

}

