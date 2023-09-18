/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl
 Copyright (C) 2003, 2004, 2005, 2006, 2007 StatPro Italia srl
 Copyright (C) 2007 Ferdinando Ametrano

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

#include <ql/cashflows/iborcoupon.hpp>
#include <ql/indexes/iborindex.hpp>
#include <ql/instruments/vanillaswap.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>
#include <utility>

namespace QuantLib {

    VanillaSwap::VanillaSwap(Type type,
                             Real nominal,
                             Schedule fixedSchedule,
                             Rate fixedRate,
                             DayCounter fixedDayCount,
                             Schedule floatSchedule,
                             ext::shared_ptr<IborIndex> index,
                             Spread spread,
                             DayCounter floatingDayCount,
                             ext::optional<BusinessDayConvention> paymentConvention,
                             ext::optional<bool> useIndexedCoupons)
    : FixedVsFloatingSwap(type, {nominal}, std::move(fixedSchedule), fixedRate, std::move(fixedDayCount),
                          {nominal}, std::move(floatSchedule), std::move(index), spread, std::move(floatingDayCount),
                          paymentConvention) {

        legs_[1] = IborLeg(floatingSchedule(), iborIndex())
            .withNotionals(this->floatingNominals())
            .withPaymentDayCounter(this->floatingDayCount())
            .withPaymentAdjustment(this->paymentConvention())
            .withSpreads(this->spread())
            .withIndexedCoupons(useIndexedCoupons);
        for (Leg::const_iterator i = legs_[1].begin(); i < legs_[1].end(); ++i)
            registerWith(*i);
    }

    void VanillaSwap::setupFloatingArguments(arguments* args) const {
        const Leg& floatingCoupons = floatingLeg();
        Size n = floatingCoupons.size();

        args->floatingResetDates = args->floatingPayDates = args->floatingFixingDates = std::vector<Date>(n);
        args->floatingAccrualTimes = std::vector<Time>(n);
        args->floatingSpreads = std::vector<Spread>(n);
        args->floatingCoupons = args->floatingNominals = std::vector<Real>(n);

        for (Size i=0; i<n; ++i) {
            auto coupon = ext::dynamic_pointer_cast<IborCoupon>(floatingCoupons[i]);

            args->floatingResetDates[i] = coupon->accrualStartDate();
            args->floatingPayDates[i] = coupon->date();
            args->floatingNominals[i] = coupon->nominal();

            args->floatingFixingDates[i] = coupon->fixingDate();
            args->floatingAccrualTimes[i] = coupon->accrualPeriod();
            args->floatingSpreads[i] = coupon->spread();
            try {
                args->floatingCoupons[i] = coupon->amount();
            } catch (Error&) {
                args->floatingCoupons[i] = Null<Real>();
            }
        }
    }

}
