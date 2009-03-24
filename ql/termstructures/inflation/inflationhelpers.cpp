/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Chris Kenyon
 Copyright (C) 2007 StatPro Italia srl

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

#include <ql/termstructures/inflation/inflationhelpers.hpp>
#include <ql/pricingengines/inflation/discountinginflationswapengines.hpp>

namespace QuantLib {

    namespace {
        void no_deletion(void*) {}
    }


    ZciisInflationHelper::ZciisInflationHelper(const Handle<Quote>& quote,
                                               const Period& lag,
                                               const Date& maturity,
                                               Natural settlementDays,
                                               const Calendar& calendar,
                                               BusinessDayConvention bdc,
                                               const DayCounter& dayCounter,
                                               Frequency frequency)
    : BootstrapHelper<ZeroInflationTermStructure>(quote),
      lag_(lag), maturity_(maturity), settlementDays_(settlementDays),
      calendar_(calendar), paymentConvention_(bdc), dayCounter_(dayCounter),
      frequency_(frequency) {

        earliestDate_ = maturity_ - lag_;
        latestDate_ = maturity_ - lag_;

        registerWith(Settings::instance().evaluationDate());
    }

    Real ZciisInflationHelper::impliedQuote() const {
        // what does the term structure imply?
        // in this case just the same value ... trivial case
        // (would not be so for an inflation-linked bond)
        zciis_->recalculate();
        return zciis_->fairRate();
    }

    void ZciisInflationHelper::setTermStructure(ZeroInflationTermStructure* z) {

        BootstrapHelper<ZeroInflationTermStructure>::setTermStructure(z);

        // set up a new ZCIIS
        // but this one does NOT own its inflation term structure
        const bool own = false;
        Rate K = quote()->value();

        Handle<ZeroInflationTermStructure> inflationTS(
                 boost::shared_ptr<ZeroInflationTermStructure>(z,no_deletion),
                 own);

        Date start = z->nominalTermStructure()->referenceDate();
        zciis_.reset(new ZeroCouponInflationSwap(start, maturity_, lag_, K,
                                                 calendar_, paymentConvention_,
                                                 dayCounter_));
        zciis_->setPricingEngine(boost::shared_ptr<PricingEngine>(
             new DiscountingZeroInflationSwapEngine(z->nominalTermStructure(),
                                                    inflationTS)));
    }


    YyiisInflationHelper::YyiisInflationHelper(const Handle<Quote>& quote,
                                               const Period& lag,
                                               const Date& maturity,
                                               Natural settlementDays,
                                               const Calendar& calendar,
                                               BusinessDayConvention bdc,
                                               const DayCounter& dayCounter,
                                               Frequency frequency)
    : BootstrapHelper<YoYInflationTermStructure>(quote),
      lag_(lag), maturity_(maturity), settlementDays_(settlementDays),
      calendar_(calendar), paymentConvention_(bdc), dayCounter_(dayCounter),
      frequency_(frequency) {

        // the earliest date is only known after setting up the YYIIS.
        latestDate_ = maturity_ - lag_;
    }


    Real YyiisInflationHelper::impliedQuote() const {
        // what does the term structure imply?
        // in this case just the same value ... trivial case
        // (would not be so for an inflation-linked bond)
        yyiis_->recalculate();
        return yyiis_->fairRate();
    }

    void YyiisInflationHelper::setTermStructure(YoYInflationTermStructure* y) {

        BootstrapHelper<YoYInflationTermStructure>::setTermStructure(y);

        // set up a new YYIIS
        // but this one does NOT own its inflation term structure
        const bool own = false;
        Rate K = quote()->value();

        Handle<YoYInflationTermStructure> inflationTS(
                 boost::shared_ptr<YoYInflationTermStructure>(y,no_deletion),
                 own);

        //! N.B. if the maturity is just over an integer number of
        //! this may be caused by ambiguous data.  The YYIIS
        //! has specific settings for this that are enabled by default.
        Date start = y->nominalTermStructure()->referenceDate();
        yyiis_.reset(new YearOnYearInflationSwap(start, maturity_, lag_, K,
                                                 calendar_, paymentConvention_,
                                                 dayCounter_));
        yyiis_->setPricingEngine(boost::shared_ptr<PricingEngine>(
              new DiscountingYoYInflationSwapEngine(y->nominalTermStructure(),
                                                    inflationTS)));
        // now known
        earliestDate_ = yyiis_->paymentDates().front() - lag_;
        // Note that this can imply inflation _before_ the reference
        // date. This is correct, given that inflation is only
        // available with a lag.  However, maturity is always after
        // the reference date so the discount factor and NPV will
        // always be sensible.
        Date lastPayment = yyiis_->paymentDates().back(),
             lastFixing = calendar_.adjust(lastPayment - lag_,
                                           paymentConvention_);

        latestDate_ = std::max(maturity_ - lag_, lastFixing);
    }

}

