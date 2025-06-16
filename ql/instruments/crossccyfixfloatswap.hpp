/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2018 Quaternion Risk Management Ltd
 All rights reserved.

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

/*! \file qle/instruments/crossccyfixfloatswap.hpp
    \brief Cross currency fixed vs float swap instrument
    \ingroup instruments
*/

#ifndef quantlib_cross_ccy_fix_float_swap_hpp
#define quantlib_cross_ccy_fix_float_swap_hpp

#include <ql/indexes/iborindex.hpp>
#include <ql/time/schedule.hpp>
#include <ql/instruments/crossccyswap.hpp>

namespace QuantLib {

/*! Cross currency fixed vs float swap
    \ingroup instruments
*/
class CrossCcyFixFloatSwap : public CrossCcySwap {
public:
    enum Type { Receiver = -1, Payer = 1 };
    class arguments;
    class results;

    //! \name Constructors
    //@{
    //! Detailed constructor
    CrossCcyFixFloatSwap(Type type, Real fixedNominal, const Currency& fixedCurrency,
                         const Schedule& fixedSchedule, Rate fixedRate,
                         const DayCounter& fixedDayCount, BusinessDayConvention fixedPaymentBdc,
                         Natural fixedPaymentLag, const Calendar& fixedPaymentCalendar,
                         Real floatNominal, const Currency& floatCurrency,
                         const Schedule& floatSchedule,
                         const ext::shared_ptr<IborIndex>& floatIndex, Spread floatSpread,
                         BusinessDayConvention floatPaymentBdc, Natural floatPaymentLag,
                         const Calendar& floatPaymentCalendar);
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
    //@}

    //! \name Additional interface
    //@{
    Rate fairFixedRate() const {
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

    mutable Rate fairFixedRate_;
    mutable Spread fairSpread_;
};

//! \ingroup instruments
class CrossCcyFixFloatSwap::arguments : public CrossCcySwap::arguments {
public:
    Rate fixedRate;
    Spread spread;
    void validate() const override;
};

//! \ingroup instruments
class CrossCcyFixFloatSwap::results : public CrossCcySwap::results {
public:
    Rate fairFixedRate;
    Spread fairSpread;
    void reset() override;
};

} // namespace QuantLib

#endif
