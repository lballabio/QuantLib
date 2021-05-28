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
                                   const DayCounter& dayCounter) {
            Time T = dayCounter.yearFraction(startDate, maturityDate);
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
                                   const Calendar& paymentCalendar,
                                   BusinessDayConvention paymentConvention,
                                   Natural paymentDelay,
                                   RateAveraging::Type averagingMethod)
    : Swap(2), type_(type), baseNominal_(baseNominal), 
      fixedPayment_(fixedPayment), iborIndex_(std::move(iborIndex)),
      averagingMethod_(averagingMethod) {

        QL_REQUIRE(!(baseNominal < 0.0), "base nominal cannot be negative");
        QL_REQUIRE(startDate < maturityDate,
                   "start date (" << startDate 
                   << ") later than or equal to maturity date ("
                   << maturityDate << ")");

        Date paymentDate = paymentCalendar.advance(maturityDate, paymentDelay, 
                                                   Days, paymentConvention);

        legs_[0].push_back(
            ext::shared_ptr<CashFlow>(new SimpleCashFlow(fixedPayment_, paymentDate)));
        legs_[1].push_back(createSubPeriodicCoupon(paymentDate, startDate, maturityDate,
                                                   baseNominal_, iborIndex_, averagingMethod_));
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
                                   const Calendar& paymentCalendar,
                                   BusinessDayConvention paymentConvention,
                                   Natural paymentDelay,
                                   RateAveraging::Type averagingMethod)
    : ZeroCouponSwap(type, baseNominal, startDate, maturityDate,
      calculateFixedPayment(startDate, maturityDate, baseNominal, fixedRate, fixedDayCounter),
      iborIndex, paymentCalendar, paymentConvention, paymentDelay, averagingMethod) {
    }

    Real ZeroCouponSwap::fixedLegNPV() const {
        return legNPV(0);
    }

    Real ZeroCouponSwap::floatingLegNPV() const {
        return legNPV(1);
    }

    Real ZeroCouponSwap::fairFixedPayment() const {
        // Knowing that for the fair payment NPV = 0.0, where:
        // NPV = (discount at fixed amount pay date) * (payer\receiver * fixed amount)
        //     + (discount at float amount pay date) * (-payer\receiver * float amount)
        // we have:
        // fair amount = NPV float / discount at fixed amount pay date
        // with NPV float corrected for the payer sign.
        Real scaling = payer(1) ? -1.0 : 1.0;
        return floatingLegNPV() / (endDiscounts(0) * scaling);
    }

    const Leg& ZeroCouponSwap::fixedLeg() const { return leg(0); }

    const Leg& ZeroCouponSwap::floatingLeg() const { return leg(1); }
}
