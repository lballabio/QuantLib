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

/*! \file fxtenorexpiry.hpp
    \brief Expiry and delivery dates of FX option tenors.
*/

#ifndef quantlib_fx_tenor_expiry_hpp
#define quantlib_fx_tenor_expiry_hpp

#include <ql/experimental/fxslv/fxsettlementconvention.hpp>
#include <ql/time/period.hpp>
#include <vector>

namespace QuantLib {

    //! Delivery date of an FX option tenor traded on the given date
    /*! For tenors in days or weeks the expiry is the trade date plus the
        tenor (Following), and the delivery date is its value date.  For
        tenors in months or years the delivery date is the spot date plus
        the tenor, Modified Following with the end-of-month rule: if the
        spot date is the last business day of its month, so is the
        delivery date.  All dates are on the settlement calendar.
    */
    Date fxDeliveryDate(const Date& tradeDate,
                        const Period& tenor,
                        const FxSettleConvention& settlement);

    //! Expiry date of an FX option tenor traded on the given date
    /*! For tenors in months or years, the date whose value date is the
        delivery date (see fxDeliveryDate()), i.e. the delivery date moved
        back by the spot lag.
    */
    Date fxExpiryDate(const Date& tradeDate,
                      const Period& tenor,
                      const FxSettleConvention& settlement);

    //! Expiry dates of the given tenors, traded on the given date.
    std::vector<Date> fxExpiryDates(const Date& tradeDate,
                                    const std::vector<Period>& tenors,
                                    const FxSettleConvention& settlement);

}

#endif
