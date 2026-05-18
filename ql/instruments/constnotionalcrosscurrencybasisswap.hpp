/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2016 Quaternion Risk Management Ltd
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

/*! \file constnotionalcrosscurrencybasisswap.hpp
    \brief Cross-currency basis swap
*/

#ifndef quantlib_cross_currency_basis_swap_hpp
#define quantlib_cross_currency_basis_swap_hpp

#include <ql/indexes/iborindex.hpp>
#include <ql/cashflows/rateaveraging.hpp>
#include <ql/time/schedule.hpp>
#include <ql/instruments/constnotionalcrosscurrencyswap.hpp>

namespace QuantLib {

//! Cross-currency basis swap
/*! The first leg holds the pay currency cashflows and second leg holds
    the receive currency cashflows.

    \ingroup instruments
*/
class ConstNotionalCrossCurrencyBasisSwap : public ConstNotionalCrossCurrencySwap {
  public:
    class arguments;
    class results;

    //! \name Constructors
    //@{
    /*!
        \brief Constructs a cross-currency basis swap.

        First leg holds the pay currency cashflows and the second leg holds the receive currency cashflows.
        
        \param payNominal         Notional amount for the pay leg.
        \param payCurrency        Currency of the pay leg.
        \param paySchedule        Payment schedule for the pay leg.
        \param payIndex           Floating rate index for the pay leg.
        \param paySpread          Spread over the floating rate for the pay leg.
        \param payGearing         Gearing factor for the pay leg.
        \param recNominal         Notional amount for the receive leg.
        \param recCurrency        Currency of the receive leg.
        \param recSchedule        Payment schedule for the receive leg.
        \param recIndex           Floating rate index for the receive leg.
        \param recSpread          Spread over the floating rate for the receive leg.
        \param recGearing         Gearing factor for the receive leg.
        \param payPaymentLag      Payment lag in days for the pay leg if overnight (default: 0).
        \param recPaymentLag      Payment lag in days for the receive leg if overnight (default: 0).
        \param payCompoundSpread  Whether to compound the spread daily for the pay leg if overnight (default: false).
        \param payLookbackDays    Lookback days for the pay leg if overnight (default: null).
        \param payObservationShift  Whether the observation shift is applied for the pay leg if overnight (default: false).
        \param payLockoutDays     Lockout period (in business days) for the pay leg if overnight (default: 0).
        \param payAveragingMethod   Averaging method for the pay leg if overnight (default: compounding).
        \param recCompoundSpread  Whether to compound the spread daily for the receive leg if overnight (default: false).
        \param recLookbackDays    Lookback days for the receive leg if overnight (default: null).
        \param recObservationShift  Whether the observation shift is applied for the receive leg if overnight (default: false).
        \param recLockoutDays     Lockout period (in business days) for the receive leg if overnight (default: 0).
        \param recAveragingMethod   Averaging method for the receive leg if overnight (default: compounding).
        \param telescopicValueDates Flag indicating whether telescopic value dates are used if overnight (default: false).
    */
    ConstNotionalCrossCurrencyBasisSwap(
        Real payNominal, const Currency& payCurrency, const Schedule& paySchedule,
        const ext::shared_ptr<IborIndex>& payIndex, Spread paySpread, Real payGearing, Real recNominal,
        const Currency& recCurrency, const Schedule& recSchedule, const ext::shared_ptr<IborIndex>& recIndex,
        Spread recSpread, Real recGearing, Integer payPaymentLag = 0, Integer recPaymentLag = 0,
        bool payCompoundSpread = false, Natural payLookbackDays = Null<Natural>(), bool payObservationShift = false,
        Natural payLockoutDays = 0, RateAveraging::Type payAveragingMethod = RateAveraging::Compound,
        bool recCompoundSpread = false, Natural recLookbackDays = Null<Natural>(), bool recObservationShift = false,
        Natural recLockoutDays = 0, RateAveraging::Type recAveragingMethod = RateAveraging::Compound,
        const bool telescopicValueDates = false);
    //@}
    //! \name Instrument interface
    //@{
    void setupArguments(PricingEngine::arguments* args) const override;
    void fetchResults(const PricingEngine::results*) const override;
    //@}
    //! \name Inspectors
    //@{
    Real payNominal() const { return payNominal_; }
    const Currency& payCurrency() const { return payCurrency_; }
    const Schedule& paySchedule() const { return paySchedule_; }
    const ext::shared_ptr<IborIndex>& payIndex() const { return payIndex_; }
    Spread paySpread() const { return paySpread_; }
    Real payGearing() const { return payGearing_; }

    Real recNominal() const { return recNominal_; }
    const Currency& recCurrency() const { return recCurrency_; }
    const Schedule& recSchedule() const { return recSchedule_; }
    const ext::shared_ptr<IborIndex>& recIndex() const { return recIndex_; }
    Spread recSpread() const { return recSpread_; }
    Real recGearing() const { return recGearing_; }
    //@}

    //! \name Additional interface
    //@{
    Spread fairPaySpread() const {
        calculate();
        QL_REQUIRE(fairPaySpread_ != Null<Real>(), "Fair pay spread is not available");
        return fairPaySpread_;
    }
    Spread fairRecSpread() const {
        calculate();
        QL_REQUIRE(fairRecSpread_ != Null<Real>(), "Fair pay spread is not available");
        return fairRecSpread_;
    }
    //@}

  protected:
    //! \name Instrument interface
    //@{
    void setupExpired() const override;
    //@}

  private:
    void initialize();

    Real payNominal_;
    Currency payCurrency_;
    Schedule paySchedule_;
    ext::shared_ptr<IborIndex> payIndex_;
    Spread paySpread_;
    Real payGearing_;

    Real recNominal_;
    Currency recCurrency_;
    Schedule recSchedule_;
    ext::shared_ptr<IborIndex> recIndex_;
    Spread recSpread_;
    Real recGearing_;

    Integer payPaymentLag_;
    Integer recPaymentLag_;

    // OIS only
    bool payCompoundSpread_;
    Natural payLookbackDays_;
    bool payObservationShift_;
    Natural payLockoutDays_;
    RateAveraging::Type payAveragingMethod_;
    bool recCompoundSpread_;
    Natural recLookbackDays_;
    bool recObservationShift_;
    Natural recLockoutDays_;
    RateAveraging::Type recAveragingMethod_;
    bool telescopicValueDates_;

    mutable Spread fairPaySpread_;
    mutable Spread fairRecSpread_;
};


class ConstNotionalCrossCurrencyBasisSwap::arguments : public ConstNotionalCrossCurrencySwap::arguments {
  public:
    Spread paySpread;
    Spread recSpread;
    void validate() const override;
};


class ConstNotionalCrossCurrencyBasisSwap::results : public ConstNotionalCrossCurrencySwap::results {
  public:
    Spread fairPaySpread;
    Spread fairRecSpread;
    void reset() override;
};

}

#endif
