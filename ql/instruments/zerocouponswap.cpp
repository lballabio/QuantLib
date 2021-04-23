/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2021 Marcin Rybacki

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

/*! \file zerocouponswap.hpp
 \brief Zero-coupon interest rate swap
 */

#include <ql/cashflows/simplecashflow.hpp>
#include <ql/cashflows/subperiodcoupon.hpp>
#include <ql/indexes/iborindex.hpp>
#include <ql/instruments/zerocouponswap.hpp>

namespace QuantLib {
    ZeroCouponSwap::ZeroCouponSwap(Type type,
                                   Real baseNominal,
                                   const Date& startDate,
                                   const Date& endDate,
                                   Real fixedPayment,
                                   ext::shared_ptr<IborIndex> iborIndex,
                                   Calendar paymentCalendar,
                                   BusinessDayConvention paymentConvention,
                                   Natural paymentDelay)
    : Swap(2), type_(type), baseNominal_(baseNominal), startDate_(startDate),
      endDate_(endDate), fixedPayment_(fixedPayment),
      iborIndex_(std::move(iborIndex)), paymentCalendar_(std::move(paymentCalendar)), 
      paymentConvention_(paymentConvention), paymentDelay_(paymentDelay) {

        // basic validation procedure and construction of legs.
    }

    namespace {
        Real calculateFixedPayment(const Date& startDate,
                                   const Date& endDate,
                                   Real baseNominal,
                                   Rate rate,
                                   const DayCounter& dayCounter) {
            Real T = dayCounter.yearFraction(startDate, endDate);
            return baseNominal * (std::pow(1.0 + rate, T) - 1.0);
        }
    }

    ZeroCouponSwap::ZeroCouponSwap(Type type,
                                   Real baseNominal,
                                   const Date& startDate,
                                   const Date& endDate,
                                   Real fixedRate,
                                   const DayCounter& fixedDayCounter,
                                   ext::shared_ptr<IborIndex> iborIndex,
                                   Calendar paymentCalendar,
                                   BusinessDayConvention paymentConvention,
                                   Natural paymentDelay)
    : ZeroCouponSwap(type, baseNominal, startDate, endDate, 
        calculateFixedPayment(startDate, endDate, baseNominal, fixedRate, fixedDayCounter),
        iborIndex, paymentCalendar, paymentConvention, paymentDelay) {
    }
}