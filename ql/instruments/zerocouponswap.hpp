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

#ifndef quantlib_zerocouponswap_hpp
#define quantlib_zerocouponswap_hpp

#include <ql/instruments/swap.hpp>
#include <ql/time/calendar.hpp>
#include <ql/time/daycounter.hpp>

namespace QuantLib {
    class IborIndex;

    //! Zero-coupon interest rate swap
    /*! Quoted in terms of a known fixed cash flow \f$ N^{FIX} \f$ or
        a fixed rate \f$ R \f$, where:
        \f[
        N^{FIX} = N \left[ (1+R)^{\alpha(T_{0}, T_{K})}-1 \right] ,
        \f]
        with \f$ \alpha(T_{0}, T_{K}) \f$ being the time fraction
        between the start date of the contract \f$ T_{0} \f$ and
        the end date \f$ T_{K} \f$ - according to a given day count
        convention. \f$ N \f$ is the base notional amount prior to
        compounding.
        The floating leg also pays a single cash flow \f$ N^{FLT} \f$,
        which value is determined by periodically averaging (e.g. every
        6 months) interest rate index fixings.
        Assuming the use of compounded averaging the projected value of
        the floating leg becomes:
        \f[
        N^{FLT} = N \left[ \prod_{k=0}^{K-1} (1+\alpha(T_{k},T_{k+1})
                           L(T_{k},T_{k+1})) -1 \right],
        \f]
        where \f$ L(T_{i}, T_{j})) \f$ are interest rate index fixings
        for accrual period \f$ [T_{i}, T_{j}] \f$.
        For a par contract, it holds that:
        \f[
        P_n(0,T) N^{FIX} = P_n(0,T) N^{FLT}
        \f]
        where \f$ T \f$ is the final payment time, \f$ P_n(0,t) \f$
        is the nominal discount factor at time \f$ t \f$.

        At maturity the two single cashflows are swapped.

        \note we do not need Schedules on the legs because they use
              one or two dates only per leg. Those dates are not
              adjusted for potential non-business days. Only the
              payment date is subject to adjustment.
    */

    class ZeroCouponSwap : public Swap {
      public:
        ZeroCouponSwap(Type type,
                       Real baseNominal,
                       const Date& startDate,
                       const Date& maturityDate,
                       Real fixedPayment,
                       std::shared_ptr<IborIndex> iborIndex,
                       const Calendar& paymentCalendar,
                       BusinessDayConvention paymentConvention = Following,
                       Natural paymentDelay = 0);

        ZeroCouponSwap(Type type,
                       Real baseNominal,
                       const Date& startDate,
                       const Date& maturityDate,
                       Rate fixedRate,
                       const DayCounter& fixedDayCounter,
                       std::shared_ptr<IborIndex> iborIndex,
                       const Calendar& paymentCalendar,
                       BusinessDayConvention paymentConvention = Following,
                       Natural paymentDelay = 0);

        //! \name Inspectors
        //@{
        //! "payer" or "receiver" refer to the fixed leg.
        Type type() const { return type_; }
        Real baseNominal() const { return baseNominal_; }
        Date startDate() const override { return startDate_; }
        Date maturityDate() const override { return maturityDate_; }
        const std::shared_ptr<IborIndex>& iborIndex() const { return iborIndex_; }

        //! just one cashflow in each leg
        const Leg& fixedLeg() const;
        //! just one cashflow in each leg
        const Leg& floatingLeg() const;

        Real fixedPayment() const;
        //@}

        //! \name Results
        //@{
        Real fixedLegNPV() const;
        Real floatingLegNPV() const;
        Real fairFixedPayment() const;
        Rate fairFixedRate(const DayCounter& dayCounter) const;
        //@}

      private:
        ZeroCouponSwap(Type type,
                       Real baseNominal,
                       const Date& startDate,
                       const Date& maturityDate,
                       std::shared_ptr<IborIndex> iborIndex,
                       const Calendar& paymentCalendar,
                       BusinessDayConvention paymentConvention,
                       Natural paymentDelay);

        Type type_;
        Real baseNominal_;
        std::shared_ptr<IborIndex> iborIndex_;
        Date startDate_;
        Date maturityDate_;
        Date paymentDate_;
    };
}

#endif
