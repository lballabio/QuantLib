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

    namespace {       
        Real calculateFixedPayment(const Date& startDate,
                                   const Date& maturityDate,
                                   Real baseNominal,
                                   Rate rate,
                                   const DayCounter& dayCounter,
                                   const Calendar& calendar,
                                   BusinessDayConvention convention) {
            Real T = dayCounter.yearFraction(calendar.adjust(startDate, convention), 
                                             calendar.adjust(maturityDate, convention));
            return baseNominal * (std::pow(1.0 + rate, T) - 1.0);
        }

        ext::shared_ptr<CashFlow> createSubPeriodicCoupon(const Date& paymentDate,
                                                          const Date& startDate,
                                                          const Date& maturityDate,
                                                          Real nominal,
                                                          const ext::shared_ptr<IborIndex>& index,
                                                          RateAveraging::Type averagingMethod) {
            auto floatCpn = ext::make_shared<SubPeriodsCoupon>(
                paymentDate, nominal, startDate, maturityDate, index->fixingDays(), index);
            switch (averagingMethod) {
                case RateAveraging::Simple:
                    floatCpn->setPricer(
                        ext::shared_ptr<FloatingRateCouponPricer>(new AveragingRatePricer));
                    break;
                case RateAveraging::Compound:
                    floatCpn->setPricer(
                        ext::shared_ptr<FloatingRateCouponPricer>(new CompoundingRatePricer));
                    break;
                default:
                    QL_FAIL("unknown compounding convention (" << Integer(averagingMethod) << ")");
            }
            return floatCpn;
        }
    }

    ZeroCouponSwap::ZeroCouponSwap(Type type,
                                   Real baseNominal,
                                   const Date& startDate,
                                   const Date& maturityDate,
                                   Real fixedPayment,
                                   ext::shared_ptr<IborIndex> iborIndex,
                                   const Calendar& calendar,
                                   BusinessDayConvention convention,
                                   Natural paymentDelay,
                                   RateAveraging::Type averagingMethod)
    : Swap(2), type_(type), baseNominal_(baseNominal), 
      fixedPayment_(fixedPayment), iborIndex_(std::move(iborIndex)) {
        Date adjustedStart = calendar.adjust(startDate, convention);
        Date adjustedMaturity = calendar.adjust(maturityDate, convention);
        Date paymentDate = calendar.advance(adjustedMaturity, paymentDelay, Days, convention);

        legs_[0].push_back(
            ext::shared_ptr<CashFlow>(new SimpleCashFlow(fixedPayment_, paymentDate)));
        legs_[1].push_back(createSubPeriodicCoupon(paymentDate, adjustedStart, adjustedMaturity,
                                                   baseNominal_, iborIndex_, averagingMethod));
        for (Leg::const_iterator i = legs_[1].begin(); i < legs_[1].end(); ++i)
            registerWith(*i);

        switch (type_) {
            case Payer:
                payer_[0] = -1.0;
                payer_[1] = +1.0;
                break;
            case Receiver:
                payer_[0] = +1.0;
                payer_[1] = -1.0;
                break;
            default:
                QL_FAIL("unknown zero coupon swap type");
        }
    }

    ZeroCouponSwap::ZeroCouponSwap(Type type,
                                   Real baseNominal,
                                   const Date& startDate,
                                   const Date& maturityDate,
                                   Real fixedRate,
                                   const DayCounter& fixedDayCounter,
                                   ext::shared_ptr<IborIndex> iborIndex,
                                   const Calendar& calendar,
                                   BusinessDayConvention convention,
                                   Natural paymentDelay,
                                   RateAveraging::Type averagingMethod)
    : ZeroCouponSwap(type, baseNominal, startDate, maturityDate,
      calculateFixedPayment(startDate, maturityDate, baseNominal, fixedRate, fixedDayCounter, calendar, convention),
      iborIndex, calendar, convention, paymentDelay, averagingMethod) {
    }

    void ZeroCouponSwap::setupArguments(PricingEngine::arguments* args) const {
        Swap::setupArguments(args);
    }

    void ZeroCouponSwap::arguments::validate() const {
        Swap::arguments::validate();
    }

    void ZeroCouponSwap::fetchResults(const PricingEngine::results* r) const {
        Swap::fetchResults(r);
    }

    Real ZeroCouponSwap::fixedLegNPV() const {
        calculate();
        QL_REQUIRE(legNPV_[0] != Null<Real>(), "result not available");
        return legNPV_[0];
    }

    Real ZeroCouponSwap::floatingLegNPV() const {
        calculate();
        QL_REQUIRE(legNPV_[1] != Null<Real>(), "result not available");
        return legNPV_[1];
    }

    const Leg& ZeroCouponSwap::fixedLeg() const { return legs_[0]; }

    const Leg& ZeroCouponSwap::floatingLeg() const { return legs_[1]; }
}
