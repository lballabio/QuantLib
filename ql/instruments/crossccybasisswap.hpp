/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2016 Quaternion Risk Management Ltd
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

/*! \file crossccybasisswap.hpp
    \brief Cross currency basis swap instrument

        \ingroup instruments
*/

#ifndef quantlib_cross_ccy_basis_swap_hpp
#define quantlib_cross_ccy_basis_swap_hpp

#include <ql/indexes/iborindex.hpp>
#include <ql/time/schedule.hpp>
#include <ql/instruments/crossccyswap.hpp>

namespace QuantLib {

//! Cross currency basis swap
/*! The first leg holds the pay currency cashflows and second leg holds
    the receive currency cashflows.

            \ingroup instruments
*/
class CrossCcyBasisSwap : public CrossCcySwap {
public:
    class arguments;
    class results;
    //! \name Constructors
    //@{
    /*! First leg holds the pay currency cashflows and the second leg
        holds the receive currency cashflows.
    */
    CrossCcyBasisSwap(
        Real payNominal, const Currency& payCurrency, const Schedule& paySchedule,
        const ext::shared_ptr<IborIndex>& payIndex, Spread paySpread, Real payGearing, Real recNominal,
        const Currency& recCurrency, const Schedule& recSchedule, const ext::shared_ptr<IborIndex>& recIndex,
        Spread recSpread, Real recGearing, Size payPaymentLag = 0, Size recPaymentLag = 0,
        ext::optional<bool> payIncludeSpread = ext::nullopt, ext::optional<Natural> payLookbackDays = ext::nullopt,
        ext::optional<bool> recIncludeSpread = ext::nullopt, ext::optional<Natural> recLookbackDays = ext::nullopt,
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

    Size payPaymentLag_;
    Size recPaymentLag_;
    // OIS only
    ext::optional<bool> payIncludeSpread_;
    ext::optional<Natural> payLookbackDays_;
    ext::optional<bool> recIncludeSpread_;
    ext::optional<Natural> recLookbackDays_;
    bool telescopicValueDates_;

    mutable Spread fairPaySpread_;
    mutable Spread fairRecSpread_;
};

//! \ingroup instruments
class CrossCcyBasisSwap::arguments : public CrossCcySwap::arguments {
public:
    Spread paySpread;
    Spread recSpread;
    void validate() const override;
};

//! \ingroup instruments
class CrossCcyBasisSwap::results : public CrossCcySwap::results {
public:
    Spread fairPaySpread;
    Spread fairRecSpread;
    void reset() override;
};
} // namespace QuantLib

#endif
