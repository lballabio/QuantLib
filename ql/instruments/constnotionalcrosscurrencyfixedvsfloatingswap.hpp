/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2018 Quaternion Risk Management Ltd
 Copyright (C) 2025 Paolo D'Elia

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

/*! \file constnotionalcrosscurrencyfixedvsfloatingswap.hpp
    \brief Cross-currency fixed-vs-floating swap
*/

#ifndef quantlib_cross_currency_fix_vs_floating_swap_hpp
#define quantlib_cross_currency_fix_vs_floating_swap_hpp

#include <ql/indexes/iborindex.hpp>
#include <ql/cashflows/rateaveraging.hpp>
#include <ql/time/schedule.hpp>
#include <ql/instruments/constnotionalcrosscurrencyswap.hpp>

namespace QuantLib {

/*! Cross-currency fixed-vs-floating swap
    \ingroup instruments
*/
class ConstNotionalCrossCurrencyFixedVsFloatingSwap : public ConstNotionalCrossCurrencySwap {
  public:
    class arguments;
    class results;

    //! \name Constructors
    //@{
    /*!
        \brief Constructs a cross-currency fixed vs floating rate swap.

        This instrument represents a cross-currency swap where one leg pays fixed-rate cashflows in one currency, 
        and the other leg pays floating-rate cashflows in another currency.

        \param type                  The type of the swap (Receiver or Payer).
        \param fixedNominal          Notional amount for the fixed leg.
        \param fixedCurrency         Currency of the fixed leg.
        \param fixedSchedule         Payment schedule for the fixed leg.
        \param fixedRate             Fixed interest rate for the fixed leg.
        \param fixedDayCount         Day count convention for the fixed leg.
        \param fixedPaymentBdc       Business day convention for fixed leg payments.
        \param fixedPaymentLag       Payment lag for the fixed leg (default: 0).
        \param fixedPaymentCalendar  Calendar for fixed leg payments.
        \param floatNominal          Notional amount for the floating leg.
        \param floatCurrency         Currency of the floating leg.
        \param floatSchedule         Payment schedule for the floating leg.
        \param floatIndex            Floating rate index for the floating leg.
        \param floatSpread           Spread over the floating rate for the floating leg.
        \param floatPaymentBdc       Business day convention for floating leg payments.
        \param floatPaymentLag       Payment lag for the floating leg (default: 0).
        \param floatPaymentCalendar  Calendar for floating leg payments.
        \param telescopicValueDates  For overnight legs, whether to use telescopic value dates (default: false).
        \param floatCompoundSpread   For overnight legs, whether to compound the spread daily (default: false).
        \param floatLookbackDays     For overnight legs, optional lookback days (default: null).
        \param floatObservationShift For overnight legs, whether the observation shift is applied (default: false).
        \param floatLockoutDays      For overnight legs, optional lockout period in business days (default: 0).
        \param floatAveragingMethod  For overnight legs, averaging method (default: compounding).
    */
    ConstNotionalCrossCurrencyFixedVsFloatingSwap(
                         Type type, Real fixedNominal, const Currency& fixedCurrency,
                         const Schedule& fixedSchedule, Rate fixedRate,
                         const DayCounter& fixedDayCount, BusinessDayConvention fixedPaymentBdc,
                         Natural fixedPaymentLag, const Calendar& fixedPaymentCalendar,
                         Real floatNominal, const Currency& floatCurrency,
                         const Schedule& floatSchedule,
                         const ext::shared_ptr<IborIndex>& floatIndex, Spread floatSpread,
                         BusinessDayConvention floatPaymentBdc, Natural floatPaymentLag,
                         const Calendar& floatPaymentCalendar, 
                         const bool telescopicValueDates = false,
                         bool floatCompoundSpread = false,
                         Natural floatLookbackDays = Null<Natural>(),
                         bool floatObservationShift = false,
                         Natural floatLockoutDays = 0,
                         RateAveraging::Type floatAveragingMethod = RateAveraging::Compound);
    //@}

    //! \name Instrument interface
    //@{
    void setupArguments(PricingEngine::arguments* a) const override;
    void fetchResults(const PricingEngine::results* r) const override;
    //@}

    //! \name Inspectors
    //@{
    Type type() const { return type_; }

    Real fixedNominal() const { return fixedNominal_; }
    const Currency& fixedCurrency() const { return fixedCurrency_; }
    const Schedule& fixedSchedule() const { return fixedSchedule_; }
    Rate fixedRate() const { return fixedRate_; }
    const DayCounter& fixedDayCount() const { return fixedDayCount_; }
    BusinessDayConvention fixedPaymentBdc() const { return fixedPaymentBdc_; }
    Natural fixedPaymentLag() const { return fixedPaymentLag_; }
    const Calendar& fixedPaymentCalendar() const { return fixedPaymentCalendar_; }

    Real floatNominal() const { return floatNominal_; }
    const Currency& floatCurrency() const { return floatCurrency_; }
    const Schedule& floatSchedule() const { return floatSchedule_; }
    const ext::shared_ptr<IborIndex>& floatIndex() const { return floatIndex_; }
    Rate floatSpread() const { return floatSpread_; }
    BusinessDayConvention floatPaymentBdc() const { return floatPaymentBdc_; }
    Natural floatPaymentLag() const { return floatPaymentLag_; }
    const Calendar& floatPaymentCalendar() const { return floatPaymentCalendar_; }
    bool floatCompoundSpread() const { return floatCompoundSpread_; }
    Natural floatLookbackDays() const { return floatLookbackDays_; }
    Natural floatLockoutDays() const { return floatLockoutDays_; }
    RateAveraging::Type floatAveragingMethod() const { return floatAveragingMethod_; }
    //@}

    //! \name Additional interface
    //@{
    Rate fairRate() const {
        calculate();
        QL_REQUIRE(fairFixedRate_ != Null<Real>(), "Fair fixed rate is not available");
        return fairFixedRate_;
    }

    Spread fairSpread() const {
        calculate();
        QL_REQUIRE(fairSpread_ != Null<Real>(), "Fair spread is not available");
        return fairSpread_;
    }
    //@}

  protected:
    //! \name Instrument interface
    //@{
    void setupExpired() const override;
    //@}

  private:
    Type type_;

    Real fixedNominal_;
    Currency fixedCurrency_;
    Schedule fixedSchedule_;
    Rate fixedRate_;
    DayCounter fixedDayCount_;
    BusinessDayConvention fixedPaymentBdc_;
    Natural fixedPaymentLag_;
    Calendar fixedPaymentCalendar_;

    Real floatNominal_;
    Currency floatCurrency_;
    Schedule floatSchedule_;
    ext::shared_ptr<IborIndex> floatIndex_;
    Spread floatSpread_;
    BusinessDayConvention floatPaymentBdc_;
    Natural floatPaymentLag_;
    Calendar floatPaymentCalendar_;
    bool telescopicValueDates_;
    bool floatCompoundSpread_;
    Natural floatLookbackDays_;
    bool floatObservationShift_;
    Natural floatLockoutDays_;
    RateAveraging::Type floatAveragingMethod_;

    mutable Rate fairFixedRate_;
    mutable Spread fairSpread_;
};


class ConstNotionalCrossCurrencyFixedVsFloatingSwap::arguments : public ConstNotionalCrossCurrencySwap::arguments {
  public:
    Rate fixedRate;
    Spread spread;
    void validate() const override;
};


class ConstNotionalCrossCurrencyFixedVsFloatingSwap::results : public ConstNotionalCrossCurrencySwap::results {
  public:
    Rate fairFixedRate;
    Spread fairSpread;
    void reset() override;
};

}

#endif
