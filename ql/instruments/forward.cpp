/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Allen Kuo

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

#include <ql/event.hpp>
#include <ql/instruments/forward.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>
#include <utility>

namespace QuantLib {

    Forward::Forward(DayCounter dayCounter,
                     Calendar calendar,
                     BusinessDayConvention businessDayConvention,
                     Natural settlementDays,
                     ext::shared_ptr<Payoff> payoff,
                     const Date& valueDate,
                     const Date& maturityDate,
                     Handle<YieldTermStructure> discountCurve)
    : dayCounter_(std::move(dayCounter)), calendar_(std::move(calendar)),
      businessDayConvention_(businessDayConvention), settlementDays_(settlementDays),
      payoff_(std::move(payoff)), valueDate_(valueDate), maturityDate_(maturityDate),
      discountCurve_(std::move(discountCurve)) {

        maturityDate_ = calendar_.adjust(maturityDate_,
                                         businessDayConvention_);

        registerWith(Settings::instance().evaluationDate());
        registerWith(discountCurve_);
    }


    Date Forward::settlementDate() const {
        Date d = calendar_.advance(Settings::instance().evaluationDate(),
                                   settlementDays_, Days);
        return std::max(d,valueDate_);
    }


    bool Forward::isExpired() const {
        return detail::simple_event(maturityDate_)
               .hasOccurred(settlementDate());
    }


    Real Forward::forwardValue() const {
        calculate();
        return (underlyingSpotValue_ - underlyingIncome_ )/
               discountCurve_->discount(maturityDate_);
    }


    InterestRate Forward::impliedYield(Real underlyingSpotValue,
                                       Real forwardValue,
                                       Date settlementDate,
                                       Compounding comp,
                                       const DayCounter& dayCounter) {

        Time t = dayCounter.yearFraction(settlementDate,maturityDate_) ;
        Real compoundingFactor = forwardValue/
            (underlyingSpotValue-spotIncome(incomeDiscountCurve_)) ;
        return InterestRate::impliedRate(compoundingFactor,
                                         dayCounter, comp, Annual,
                                         t);
    }


    void Forward::performCalculations() const {

        QL_REQUIRE(!discountCurve_.empty(),
                   "null term structure set to Forward");

        ext::shared_ptr<ForwardTypePayoff> ftpayoff =
            ext::dynamic_pointer_cast<ForwardTypePayoff>(payoff_);
        Real fwdValue = forwardValue();
        NPV_ = (*ftpayoff)(fwdValue) * discountCurve_->discount(maturityDate_);
    }

}
