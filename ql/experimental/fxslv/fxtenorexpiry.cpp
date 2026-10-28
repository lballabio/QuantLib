/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2026 Raghav Subbarao

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

#include <ql/errors.hpp>
#include <ql/experimental/fxslv/fxtenorexpiry.hpp>

namespace QuantLib {

    namespace {

        bool isShortTenor(const Period& tenor) {
            return tenor.units() == Days || tenor.units() == Weeks;
        }

        Date shortTenorExpiry(const Date& tradeDate,
                              const Period& tenor,
                              const FxSettleConvention& settlement) {
            const Calendar& cal = settlement.calendar();
            // overnight is the next business day, whatever falls in between
            if (tenor.units() == Days)
                return cal.advance(tradeDate, tenor.length(), Days);
            return cal.adjust(tradeDate + tenor, Following);
        }

    }

    Date fxDeliveryDate(const Date& tradeDate,
                        const Period& tenor,
                        const FxSettleConvention& settlement) {
        QL_REQUIRE(tenor.length() > 0, "positive tenor required, " << tenor << " given");
        if (isShortTenor(tenor))
            return settlement.valueDate(shortTenorExpiry(tradeDate, tenor, settlement));
        const Date spotDate = settlement.valueDate(tradeDate);
        return settlement.calendar().advance(spotDate, tenor, ModifiedFollowing, true);
    }

    Date fxExpiryDate(const Date& tradeDate,
                      const Period& tenor,
                      const FxSettleConvention& settlement) {
        QL_REQUIRE(tenor.length() > 0, "positive tenor required, " << tenor << " given");
        if (isShortTenor(tenor))
            return shortTenorExpiry(tradeDate, tenor, settlement);
        const Date delivery = fxDeliveryDate(tradeDate, tenor, settlement);
        const Date expiry = settlement.calendar().advance(
            delivery, -static_cast<Integer>(settlement.spotLag()), Days);
        QL_ENSURE(settlement.valueDate(expiry) == delivery,
                  "expiry " << expiry << " of tenor " << tenor << " does not deliver on "
                            << delivery);
        return expiry;
    }

    std::vector<Date> fxExpiryDates(const Date& tradeDate,
                                    const std::vector<Period>& tenors,
                                    const FxSettleConvention& settlement) {
        std::vector<Date> expiries;
        expiries.reserve(tenors.size());
        for (const auto& tenor : tenors)
            expiries.push_back(fxExpiryDate(tradeDate, tenor, settlement));
        return expiries;
    }

}
