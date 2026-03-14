/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2021 Marcin Rybacki

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <https://www.quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
 */

#include <ql/cashflows/fixedratecoupon.hpp>
#include <ql/cashflows/simplecashflow.hpp>
#include <ql/cashflows/multipleresetscoupon.hpp>
#include <ql/indexes/iborindex.hpp>
#include <ql/instruments/zerocouponswap.hpp>
#include <utility>

namespace QuantLib {

    namespace {       

        ext::shared_ptr<CashFlow>
        compoundedSubPeriodicCoupon(const Date& paymentDate,
                                    const Date& startDate,
                                    const Date& maturityDate,
                                    Real nominal,
                                    const ext::shared_ptr<IborIndex>& index) {
            Schedule schedule = MakeSchedule()
                           .from(startDate)
                           .to(maturityDate)
                           .withTenor(index->tenor())
                           .withCalendar(index->fixingCalendar())
                           .withConvention(index->businessDayConvention())
                           .backwards()
                           .endOfMonth(index->endOfMonth());
            auto floatCpn = ext::make_shared<MultipleResetsCoupon>(
                paymentDate, nominal, schedule, index->fixingDays(), index);
            floatCpn->setPricer(ext::make_shared<CompoundingMultipleResetsPricer>());
            return floatCpn;
        }

    }

    ZeroCouponSwap::ZeroCouponSwap(Type type,
                                   Real baseNominal,
                                   const Date& startDate,
                                   const Date& maturityDate,
                                   ext::shared_ptr<IborIndex> iborIndex,
                                   const Calendar& paymentCalendar,
                                   BusinessDayConvention paymentConvention,
                                   Natural paymentDelay)
    : Swap(2), type_(type), baseNominal_(baseNominal), iborIndex_(std::move(iborIndex)), 
      startDate_(startDate), maturityDate_(maturityDate) {

        QL_REQUIRE(!(baseNominal < 0.0), "base nominal cannot be negative");
        QL_REQUIRE(startDate < maturityDate,
                   "start date (" << startDate 
                   << ") later than or equal to maturity date ("
                   << maturityDate << ")");

        paymentDate_ = paymentCalendar.advance(maturityDate, paymentDelay, Days, paymentConvention);

        legs_[1].push_back(compoundedSubPeriodicCoupon(paymentDate_, startDate, maturityDate,
                                                       baseNominal_, iborIndex_));
        for (auto i = legs_[1].begin(); i < legs_[1].end(); ++i)
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
                                   Real fixedPayment,
                                   ext::shared_ptr<IborIndex> iborIndex,
                                   const Calendar& paymentCalendar,
                                   BusinessDayConvention paymentConvention,
                                   Natural paymentDelay)
    : ZeroCouponSwap(type,
                     baseNominal,
                     startDate,
                     maturityDate,
                     std::move(iborIndex),
                     paymentCalendar,
                     paymentConvention,
                     paymentDelay) {

        legs_[0].push_back(
            ext::make_shared<SimpleCashFlow>(fixedPayment, paymentDate_));
    }

    ZeroCouponSwap::ZeroCouponSwap(Type type,
                                   Real baseNominal,
                                   const Date& startDate,
                                   const Date& maturityDate,
                                   Rate fixedRate,
                                   const DayCounter& fixedDayCounter,
                                   ext::shared_ptr<IborIndex> iborIndex,
                                   const Calendar& paymentCalendar,
                                   BusinessDayConvention paymentConvention,
                                   Natural paymentDelay)
    : ZeroCouponSwap(type,
                     baseNominal,
                     startDate,
                     maturityDate,
                     std::move(iborIndex),
                     paymentCalendar,
                     paymentConvention,
                     paymentDelay) {

        InterestRate interest(fixedRate, fixedDayCounter, Compounded, Annual);
        legs_[0].push_back(ext::make_shared<FixedRateCoupon>(
            paymentDate_, baseNominal_, std::move(interest), startDate, maturityDate));
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

    Rate ZeroCouponSwap::fairFixedRate(const DayCounter& dayCounter) const {
        // Given the relation between the fixed payment (N^FIX) and the fixed rate (R),
        // N^FIX = N * [(1 + R)^T - 1],
        // the compound factor C = (1 + R)^T
        // can be equivalently expressed as:
        // C = N^FIX / N + 1
        Real compound = fairFixedPayment() / baseNominal_ + 1.0;
        return InterestRate::impliedRate(compound, dayCounter, Compounded, Annual, startDate_,
                                         maturityDate_);
    }

    const Leg& ZeroCouponSwap::fixedLeg() const { return leg(0); }

    const Leg& ZeroCouponSwap::floatingLeg() const { return leg(1); }

    Real ZeroCouponSwap::fixedPayment() const { return fixedLeg()[0]->amount(); }
}
