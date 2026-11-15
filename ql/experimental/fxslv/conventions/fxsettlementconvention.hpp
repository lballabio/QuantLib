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

/*! \file fxsettlementconvention.hpp
    \brief Settle conventions for FX
*/

#ifndef quantlib_fx_settlement_convention_hpp
#define quantlib_fx_settlement_convention_hpp

#include <ql/errors.hpp>
#include <ql/time/calendar.hpp>
#include <utility>

namespace QuantLib {

    //! Settle conventions for FX pairs
    /*! An FX option needs two settle dates for its trade date and expiry:

        - the value date \f$ s_0 \f$ of the trade date \f$ t_0 \f$, on
          which the spot is quoted for settlement, and spot premium is
          exchanged;
        - the delivery date \f$ T_d \f$ of an option expiring on
          \f$ T_e \f$, which is the value date of the expiry date.

        Both are obtained by advancing the given date by the spot lag
        in business days of the calendar.

        The calendar should be the joint calendar of the two currencies
        (and of US when the pair settles through USD).
    */
    class FxSettleConvention {
      public:
        explicit FxSettleConvention(Calendar calendar, Natural spotLag = 2)
        : calendar_(std::move(calendar)), spotLag_(spotLag) {
            QL_REQUIRE(!calendar_.empty(), "FX settlement convention requires a calendar");
        }

        const Calendar& calendar() const { return calendar_; }
        Natural spotLag() const { return spotLag_; }

        //! Value/Settle date of the given trade date.
        Date valueDate(const Date& tradeDate) const {
            // value dates carry no time of day, even when the trade date does
            const Date tradeDay(tradeDate.serialNumber());
            return calendar_.advance(tradeDay, static_cast<Integer>(spotLag_), Days);
        }

        //! Delivery date of an option expiring on the given date.
        //Date deliveryDate(const Date& expiryDate) const { return settleDate(expiryDate); }

      private:
        Calendar calendar_;
        Natural spotLag_;
    };

}

#endif
