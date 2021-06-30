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

/*! \file crosscurrencyratehelpers.hpp
    \brief FX and cross currency basis swaps rate helpers
*/

#ifndef quantlib_crosscurrencyratehelpers_hpp
#define quantlib_crosscurrencyratehelpers_hpp

#include <ql/termstructures/yield/ratehelpers.hpp>

namespace QuantLib {
    //! Rate helper for bootstrapping over XCCY basis swap rates
    /*! 
    Constant notional cross currency swap helper.
    Unlike marked-to-market cross currency swaps, both notionals
    expressed in base and quote currency remain constant throughout
    the lifetime of the swap.

    For more details see:
    N. Moreni, A. Pallavicini (2015)
    FX Modelling in Collateralized Markets: foreign measures, basis curves
    and pricing formulae.
    */
    class CrossCurrencyBasisSwapRateHelper : public RelativeDateRateHelper {
      public:
        CrossCurrencyBasisSwapRateHelper(const Handle<Quote>& basis,
                                         const Period& tenor,
                                         Natural fixingDays,
                                         Calendar calendar,
                                         BusinessDayConvention convention,
                                         bool endOfMonth,
                                         ext::shared_ptr<IborIndex> baseCurrencyIndex,
                                         ext::shared_ptr<IborIndex> quoteCurrencyIndex,
                                         Handle<YieldTermStructure> collateralCurve,
                                         bool isFxBaseCurrencyCollateralCurrency,
                                         bool isBasisOnFxBaseCurrencyLeg);
        //! \name RateHelper interface
        //@{
        Real impliedQuote() const override;
        void setTermStructure(YieldTermStructure*) override;
        //@}
        //! \name CrossCurrencyBasisSwapRateHelper inspectors
        //@{
        const Leg& baseCurrencyLeg() const;
        const Leg& quoteCurrencyLeg() const;
        //@}
        //! \name Visitability
        //@{
        void accept(AcyclicVisitor&) override;
        //@}
        //! \name CrossCurrencyBasisSwapRateHelper utility functions.
        //@{
        static ext::shared_ptr<Swap> buildCrossCurrencyLeg(const Date& evaluationDate,
                                                           const Period& tenor,
                                                           Natural fixingDays,
                                                           const Calendar& calendar,
                                                           BusinessDayConvention convention,
                                                           bool endOfMonth,
                                                           const ext::shared_ptr<IborIndex>& idx,
                                                           Swap::Type type,
                                                           Real notional = 1.0,
                                                           Spread basis = 0.0);
        //@}
      protected:
        void initializeDates() override;

        Period tenor_;
        Natural fixingDays_;
        Calendar calendar_;
        BusinessDayConvention convention_;
        bool endOfMonth_;
        ext::shared_ptr<IborIndex> baseCcyIdx_;
        ext::shared_ptr<IborIndex> quoteCcyIdx_;
        Handle<YieldTermStructure> collateralHandle_;
        bool isFxBaseCurrencyCollateralCurrency_;
        bool isBasisOnFxBaseCurrencyLeg_;

        ext::shared_ptr<Swap> baseCcyLeg_;
        ext::shared_ptr<Swap> quoteCcyLeg_;

        RelinkableHandle<YieldTermStructure> termStructureHandle_;
    };

    inline const Leg& CrossCurrencyBasisSwapRateHelper::baseCurrencyLeg() const {
        return baseCcyLeg_->leg(0);
    }

    inline const Leg& CrossCurrencyBasisSwapRateHelper::quoteCurrencyLeg() const {
        return quoteCcyLeg_->leg(0);
    }
}

#endif
