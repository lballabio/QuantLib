/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 J. Erik Radmall

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

#include <ql/experimental/commodities/commoditypricinghelpers.hpp>

namespace QuantLib {

    void CommodityPricingHelper::createPricingPeriods(
                          Date startDate, Date endDate,
                          const Quantity& quantity,
                          EnergyCommodity::DeliverySchedule deliverySchedule,
                          EnergyCommodity::QuantityPeriodicity qtyPeriodicity,
                          const PaymentTerm& paymentTerm,
                          PricingPeriods& pricingPeriods) {
        if (deliverySchedule == EnergyCommodity::Monthly) {
            Quantity periodQuantity;
            if (qtyPeriodicity == EnergyCommodity::PerMonth) {
                periodQuantity = quantity;
            } else {
                QL_FAIL("Invalid period quantity/pricing period combination.");
            }

            for (Date periodStartDate=startDate; periodStartDate<endDate; ) {
                Date periodEndDate = (periodStartDate + (1 * Months)) - 1;
                Date paymentDate = paymentTerm.getPaymentDate(periodEndDate);
                pricingPeriods.push_back(boost::shared_ptr<PricingPeriod>(
                             new PricingPeriod(periodStartDate, periodEndDate,
                                               paymentDate, periodQuantity)));
                periodStartDate = periodEndDate + 1;
            }
        } else if (deliverySchedule == EnergyCommodity::Daily) {
            QL_REQUIRE(qtyPeriodicity == EnergyCommodity::PerDay,
                       "Invalid period quantity/pricing period combination.");

            for (Date periodStartDate=startDate; periodStartDate<endDate; ) {
                Date periodEndDate = (periodStartDate + (1 * Months)) - 1;

                Quantity periodQuantity =
                    quantity * (periodEndDate - periodStartDate);
                Date paymentDate = paymentTerm.getPaymentDate(periodEndDate);
                pricingPeriods.push_back(boost::shared_ptr<PricingPeriod>(
                             new PricingPeriod(periodStartDate, periodEndDate,
                                               paymentDate, periodQuantity)));
                periodStartDate = periodEndDate + 1;
            }
        }
    }

}
