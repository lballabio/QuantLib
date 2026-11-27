/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2023 Marcin Rybacki

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

/*! \file equitytotalreturnswap.hpp
 \brief Equity total return swap
 */

#ifndef quantlib_equitytotalreturnswap_hpp
#define quantlib_equitytotalreturnswap_hpp

#include <ql/instruments/dividendschedule.hpp>
#include <ql/instruments/swap.hpp>
#include <ql/time/schedule.hpp>
#include <ql/time/calendar.hpp>
#include <ql/time/daycounter.hpp>
#include <ql/time/frequency.hpp>

namespace QuantLib {
    class InterestRateIndex;
    class IborIndex;
    class OvernightIndex;
    class EquityIndex;

    //! Equity total return swap
    /*! It exchanges a total return of an equity index for a set of
        floating cash flows linked to either Ibor or overnight index.
        With equity payment frequency Once, the equity leg future value (FV) is:
        \f[
        FV^{equity} = N \left[ \frac{I(t, T_{M})}{I(T_{0})} -1 \right],
        \f]
        where \f$ N \f$ is the swap notional, \f$ I(T_{0}) \f$ is the
        value of the equity index on the start date and \f$ I(t, T_{M}) \f$
        is the value of the equity index at maturity.
        For periodic equity payments, the cash flow for each equity period is:
        \f[
        C_i = N \left[ \frac{I(T_i)}{I(T_{i-1})} - 1 \right],
        \f]
        where \f$ T_{i-1} \f$ and \f$ T_i \f$ are consecutive equity
        period boundaries and \f$ I(T_i) \f$ is the equity index level at
        that boundary. Each period uses the same fixed notional \f$ N \f$;
        returns are not accumulated into the notional. Each cash flow is paid
        on its period end date, adjusted for the payment convention and delay.
        The floating leg payments are linked to either an Ibor or
        an overnight index. In case of an overnight index the interest
        rate fixings are compounded over the accrual period.

        The supplied schedule controls the funding leg. Equity payments default
        to Once, settling the full return at maturity. Other frequencies generate
        independent equity periods, each paying the return on the fixed notional
        between its start and end dates. The equity schedule uses the funding
        schedule's endpoints, calendar, adjustment conventions and end-of-month
        setting (false if unavailable). It uses the funding schedule's Forward
        or Backward rule, defaulting to Backward for other or unavailable rules.
        Stub dates are generated from these endpoints rather than copied from
        the funding schedule. Payment adjustment and delay apply to both legs.

        With notional reset, the notional of each period tracks a fixed number
        of index units instead of staying constant:
        \f[
        N_i = N \frac{I(T_{i-1})}{I(T_{0})},
        \f]
        so that the equity cash flow for the period is
        \f[
        C_i = N_i \left[ \frac{I(T_i)}{I(T_{i-1})} - 1 \right]
            = \frac{N}{I(T_{0})} \left[ I(T_i) - I(T_{i-1}) \right]
        \f]
        and the funding coupon for the same period accrues on \f$ N_i \f$.
        The equity cash flows add up to the return of a buy-and-hold position
        between the start date and maturity. Notional reset requires periodic
        equity payments whose periods match those of the funding schedule.

        \note With notional reset, the funding coupons whose notional is not
              yet fixed are forecast as the product of the forecast notional
              and the forecast rate. This is exact for deterministic rates;
              no convexity adjustment is made for the correlation between
              the equity index and the rate.

        Optionally, the swap can have a third leg paying the dividends of
        the index on the index units held over each equity period. Each
        dividend whose ex-dividend date falls in the equity period
        \f$ (T_{i-1}, T_i] \f$ pays
        \f[
        D_i = f \, D \, \frac{N_i}{I(T_{i-1})},
        \f]
        where \f$ D \f$ is the dividend in index points, \f$ f \f$ is the
        dividend factor (e.g., the share passed on after withholding taxes)
        and \f$ N_i \f$ is the notional of the period, i.e., \f$ N \f$
        without notional reset. The dividend is paid together with the
        equity cash flow of the period; with equity payment frequency Once,
        that is at maturity. The dividend leg has the same sign as the
        equity leg and is only available for price return indexes, since
        total return and excess return indexes already reinvest the
        dividends. Ex-dividend dates must fall between the start date
        (excluded) and the maturity (included).

        To price an equity index denominated in a different currency than
        the swap (quanto swap), set an equity cash-flow pricer such as
        EquityQuantoCashFlowPricer on the equity leg using setCouponPricer().
        Its adjustment applies to the return of each period as well as to
        the index levels driving the reset notionals and the number of index
        units on which dividends are paid; therefore, the funding coupons
        with notional reset and the dividend cash flows are adjusted
        consistently. The quanto pricer only supports price return indexes.

        Swap type (payer or receiver) refers to the equity leg.
    */
    class EquityTotalReturnSwap : public Swap {
      public:
        /*! Construct a swap with Ibor funding.
            \param equityPaymentFrequency Frequency of equity settlements,
            independent of the funding schedule. Defaults to Once, which pays
            the full equity return at maturity. NoFrequency, OtherFrequency
            and invalid frequency values are not supported.
            \param notionalReset If true, the notional of each period is
            reset to the index level at the start of the period. Requires
            an equity payment frequency giving the same periods as the
            funding schedule.
            \param dividends Dividends of the index, in index points, paid
            on a separate dividend leg. Requires a price return index. If
            empty, the swap has no dividend leg.
            \param dividendFactor Share of each dividend passed on to the
            receiver of the equity leg.
        */
        EquityTotalReturnSwap(Type type,
                              Real nominal,
                              Schedule schedule,
                              ext::shared_ptr<EquityIndex> equityIndex,
                              const ext::shared_ptr<IborIndex>& interestRateIndex,
                              DayCounter dayCounter,
                              Rate margin,
                              Real gearing = 1.0,
                              Calendar paymentCalendar = Calendar(),
                              BusinessDayConvention paymentConvention = Unadjusted,
                              Natural paymentDelay = 0,
                              Frequency equityPaymentFrequency = Once,
                              bool notionalReset = false,
                              DividendSchedule dividends = {},
                              Real dividendFactor = 1.0);

        /*! Construct a swap with overnight funding.
            \param equityPaymentFrequency Frequency of equity settlements,
            independent of the funding schedule. Defaults to Once, which pays
            the full equity return at maturity. NoFrequency, OtherFrequency
            and invalid frequency values are not supported.
            \param notionalReset If true, the notional of each period is
            reset to the index level at the start of the period. Requires
            an equity payment frequency giving the same periods as the
            funding schedule.
            \param dividends Dividends of the index, in index points, paid
            on a separate dividend leg. Requires a price return index. If
            empty, the swap has no dividend leg.
            \param dividendFactor Share of each dividend passed on to the
            receiver of the equity leg.
        */
        EquityTotalReturnSwap(Type type,
                              Real nominal,
                              Schedule schedule,
                              ext::shared_ptr<EquityIndex> equityIndex,
                              const ext::shared_ptr<OvernightIndex>& interestRateIndex,
                              DayCounter dayCounter,
                              Rate margin,
                              Real gearing = 1.0,
                              Calendar paymentCalendar = Calendar(),
                              BusinessDayConvention paymentConvention = Unadjusted,
                              Natural paymentDelay = 0,
                              Frequency equityPaymentFrequency = Once,
                              bool notionalReset = false,
                              DividendSchedule dividends = {},
                              Real dividendFactor = 1.0);

        //! \name Inspectors
        //@{
        Type type() const { return type_; }
        //! Frequency of equity settlements; Once denotes payment at maturity.
        Frequency equityPaymentFrequency() const { return equityPaymentFrequency_; }
        //! Whether the notional is reset to the index level at the start of each period.
        bool notionalReset() const { return notionalReset_; }
        //! The notional at the start date; with notional reset, the one of the first period.
        Real nominal() const { return nominal_; }
        const ext::shared_ptr<EquityIndex>& equityIndex() const { return equityIndex_; } 
        const ext::shared_ptr<InterestRateIndex>& interestRateIndex() const { return interestRateIndex_; }
        const Schedule& schedule() const { return schedule_; }
        const DayCounter& dayCounter() const { return dayCounter_; }
        Rate margin() const { return margin_; }
        Real gearing() const { return gearing_; }
        const Calendar& paymentCalendar() const { return paymentCalendar_; }
        BusinessDayConvention paymentConvention() const { return paymentConvention_; }
        Natural paymentDelay() const { return paymentDelay_; }
        //! Dividends paid on the dividend leg, if any.
        const DividendSchedule& dividends() const { return dividends_; }
        //! Share of each dividend passed on to the receiver of the equity leg.
        Real dividendFactor() const { return dividendFactor_; }

        const Leg& equityLeg() const;
        const Leg& interestRateLeg() const;
        //! Whether the swap has a dividend leg, i.e., whether dividends were given.
        bool hasDividendLeg() const { return legs_.size() > 2; }
        //! The dividend leg; empty if no dividends were given.
        const Leg& dividendLeg() const;
        //@}

        //! \name Results
        //@{
        Real equityLegNPV() const;
        Real interestRateLegNPV() const;
        //! The NPV of the dividend leg; zero if the swap has no dividend leg.
        Real dividendLegNPV() const;
        Real fairMargin() const;
        //@}

      private:
        EquityTotalReturnSwap(ext::shared_ptr<EquityIndex> equityIndex,
                              ext::shared_ptr<InterestRateIndex> interestRateIndex,
                              Type type,
                              Real nominal,
                              Schedule schedule,
                              DayCounter dayCounter,
                              Rate margin,
                              Real gearing = 1.0,
                              Calendar paymentCalendar = Calendar(),
                              BusinessDayConvention paymentConvention = Unadjusted,
                              Natural paymentDelay = 0,
                              Frequency equityPaymentFrequency = Once,
                              bool notionalReset = false,
                              DividendSchedule dividends = {},
                              Real dividendFactor = 1.0);

        ext::shared_ptr<EquityIndex> equityIndex_;
        ext::shared_ptr<InterestRateIndex> interestRateIndex_;
        Type type_;
        Real nominal_;
        Schedule schedule_;
        DayCounter dayCounter_;
        Rate margin_;
        Real gearing_;
        Calendar paymentCalendar_;
        BusinessDayConvention paymentConvention_;
        Natural paymentDelay_;
        Frequency equityPaymentFrequency_;
        bool notionalReset_;
        DividendSchedule dividends_;
        Real dividendFactor_;
    };
}

#endif