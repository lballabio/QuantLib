/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2009 Chris Kenyon

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

#include <ql/experimental/inflation/inflationcoupon.hpp>
#include <ql/experimental/inflation/inflationcouponpricer.hpp>

namespace QuantLib {

    InflationCoupon::InflationCoupon(
                  const Date& paymentDate,
                  const Real nominal,
                  const Date& startDate,
                  const Date& endDate,
                  const Period &lag,
                  const Natural fixingDays,
                  const Real gearing , const Spread spread,
                  const DayCounter& dayCounter,
                  const BusinessDayConvention &bdc,
                  const Calendar &cal,
                  const Date& refPeriodStart, const Date& refPeriodEnd)
    : Coupon(nominal, paymentDate,
             startDate, endDate, refPeriodStart, refPeriodEnd),
      dayCounter_(dayCounter), businessDayConvention_(bdc), cal_(cal),
      lag_(lag), fixingDays_(fixingDays),
      gearing_(gearing), spread_(spread) {
        QL_REQUIRE(gearing_!=0, "Null gearing not allowed");

        refPeriodStart_ = calendar().adjust(cal_.advance(startDate, -lag), bdc);
        refPeriodEnd_ = calendar().adjust(cal_.advance(endDate, -lag), bdc);

        registerWith(Settings::instance().evaluationDate());
    }


    Real InflationCoupon::amount() const {
        return rate() * accrualPeriod() * nominal();
    }


    Real InflationCoupon::accruedAmount(const Date& d) const {
        if (d <= accrualStartDate_ || d > paymentDate_) {
            return 0.0;
        } else {
            return nominal() * rate() *
                dayCounter().yearFraction(accrualStartDate_,
                                          std::min(d,accrualEndDate_),
                                          refPeriodStart_,
                                          refPeriodEnd_);
        }
    }


    Real InflationCoupon::cleanPrice(
                                const Handle<YieldTermStructure>& yts) const {
        return amount() * yts->discount(date());
    }


    Date InflationCoupon::fixingDate() const {
        // always fix lag + fixings days before the end of period
        return calendar().advance(refPeriodEnd_, -fixingDays_, Days,
                                  businessDayConvention_);
    }


    Rate InflationCoupon::indexFixing() const {
        return myindex()->fixing(fixingDate());
    }

    void InflationCoupon::accept(AcyclicVisitor& v) {
        Visitor<InflationCoupon>* v1 =
            dynamic_cast<Visitor<InflationCoupon>*>(&v);
        if (v1 != 0)
            v1->visit(*this);
        else
            Coupon::accept(v);
    }



    YoYInflationCoupon::YoYInflationCoupon(
                  const Date& paymentDate,
                  const Real nominal,
                  const Date& startDate,
                  const Date& endDate,
                  const Period &lag,
                  const Natural fixingDays,
                  const boost::shared_ptr<YoYInflationIndex>& index,
                  const Real gearing , const Spread spread,
                  const DayCounter& dayCounter,
                  const BusinessDayConvention &bdc,
                  const Calendar &cal,
                  const Date& refPeriodStart, const Date& refPeriodEnd)
    : InflationCoupon(paymentDate, nominal, startDate, endDate,lag, fixingDays,
                      gearing, spread, dayCounter, bdc, cal,
                      refPeriodStart, refPeriodEnd),
      index_(index) {
        registerWith(index_);
    }


    Rate YoYInflationCoupon::rate() const {
        QL_REQUIRE(pricer_, "pricer not set");
        pricer_->initialize(*this);     // at this point the pricer
                                        // can see that it has the
                                        // right sort of inflation
                                        // coupon
        return pricer_->swapletRate();  // N.B. this is a swap rate
                                        // (NOT a swaption rate)
                                        // N.B. note that this HAD
                                        // BETTER agree with the YYIIS
                                        // rate (in the instruments)
    }


    void YoYInflationCoupon::setPricer(
                  const boost::shared_ptr<YoYInflationCouponPricer>& pricer) {

        if (pricer_)
            unregisterWith(pricer_);
        pricer_ = pricer;
        QL_REQUIRE(pricer_, "no adequate pricer given");
        registerWith(pricer_);
        update();
    }

}

