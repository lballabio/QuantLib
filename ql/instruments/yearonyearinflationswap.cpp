/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Chris Kenyon

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

#include <ql/instruments/yearonyearinflationswap.hpp>
#include <ql/time/schedule.hpp>

namespace QuantLib {

    YearOnYearInflationSwap::YearOnYearInflationSwap(
                   const Date& start,
                   const Date& maturity,
                   const Period& lag,
                   Rate fixedRate,
                   const Calendar& calendar,
                   BusinessDayConvention convention,
                   const DayCounter& dayCounter,
                   const Handle<YieldTermStructure>& yieldTS,
                   const Handle<YoYInflationTermStructure>& inflationTS,
                   bool allowAmbiguousPayments,
                   const Period& ambiguousPaymentPeriod)
    : InflationSwap(start, maturity, lag, calendar, convention,
                    dayCounter, yieldTS),
      fixedRate_(fixedRate), inflationTS_(inflationTS),
      allowAmbiguousPayments_(allowAmbiguousPayments),
      ambiguousPaymentPeriod_(ambiguousPaymentPeriod) {

        Schedule temp = MakeSchedule(start_, maturity_,
                                     Period(1,Years),
                                     calendar_, bdc_);
        paymentDates_.clear();
        paymentDates_.reserve(temp.size()-1);

        // the first payment date is the _second_ date in the schedule,
        // so we start from index 1
        for (Size i=1; i<temp.size(); ++i) {
            if (!allowAmbiguousPayments_) {
                if (temp[i] > start_ + ambiguousPaymentPeriod_) {
                    paymentDates_.push_back(temp[i]);
                }
            } else {
                paymentDates_.push_back(temp[i]);
            }
        }

        QL_REQUIRE(!paymentDates_.empty(),
                   " no payments dates, start " << start_
                    << ", maturity: " <<  maturity_);
    }


    bool YearOnYearInflationSwap::isExpired() const {
        return yieldTS_->referenceDate() > maturity_;
    }


    Rate YearOnYearInflationSwap::fairRate() const {
        calculate();
        return fairRate_;
    }


    Rate YearOnYearInflationSwap::fixedRate() const {
        return fixedRate_;
    }


    std::vector<Date> YearOnYearInflationSwap::paymentDates() const {
        return paymentDates_;
    }


    void YearOnYearInflationSwap::setupExpired() const {
        Instrument::setupExpired();
        fairRate_ = Null<Rate>();
    }


    void YearOnYearInflationSwap::performCalculations() const {
        // Rates for instruments always look at earlier values paid later.
        Real nom = 0.0;
        Real infl = 0.0;
        Real frac;

        Date referenceDate = yieldTS_->referenceDate();
        for (Size i=0; i<paymentDates_.size(); i++) {
            Date couponPayDate = paymentDates_[i];
            if (couponPayDate >= referenceDate) {
                if (i==0) {
                    frac = dayCounter_.yearFraction(referenceDate,
                                                    couponPayDate);
                } else {
                    if (referenceDate > paymentDates_[i-1])
                        frac = dayCounter_.yearFraction(referenceDate,
                                                        couponPayDate);
                    else
                        frac = dayCounter_.yearFraction(paymentDates_[i-1],
                                                        couponPayDate);
                }

                nom += frac * yieldTS_->discount(couponPayDate);
                infl += frac * inflationTS_->yoyRate(
                             calendar().adjust(couponPayDate - lag(), bdc_)) *
                    yieldTS_->discount(couponPayDate);
            }
        }

        NPV_ = nom*fixedRate_ - infl;
        errorEstimate_ = 0.0;
        fairRate_ = infl/nom;
    }

}

