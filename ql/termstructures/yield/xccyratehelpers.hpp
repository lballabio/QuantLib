/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2021, Marcin Rybacki

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

/*! \file xccyratehelpers.hpp
    \brief FX and cross currency basis swaps rate helpers
*/

#ifndef quantlib_xccyratehelpers_hpp
#define quantlib_xccyratehelpers_hpp

#include <ql/termstructures/yield/ratehelpers.hpp>

namespace QuantLib {
    //! Rate helper for bootstrapping over XCCY basis swap rates
    /*! 
    
    */
    class XCCYBasisSwapRateHelper : public RelativeDateRateHelper {
      public:
        XCCYBasisSwapRateHelper(const Handle<Quote>& basis,
                                const Period& tenor,
                                Natural fixingDays,
                                const Calendar& calendar,
                                BusinessDayConvention convention,
                                const ext::shared_ptr<IborIndex>& baseCurrencyIndex,
                                const ext::shared_ptr<IborIndex>& quoteCurrencyIndex,
                                const Handle<YieldTermStructure>& collateralCurve,
                                bool isFxBaseCurrencyCollateralCurrency,
                                bool isBasisOnFxBaseCurrencyLeg);
        //! \name RateHelper interface
        //@{
        Real impliedQuote() const;
        void setTermStructure(YieldTermStructure*);
        //@}
        //! \name Visitability
        //@{
        void accept(AcyclicVisitor&);
        //@}
        //! \name Static helper function
        //@{
        static ext::shared_ptr<Swap> initialiseXCCYLeg(const Date& evaluationDate,
                                                       const Period& tenor,
                                                       Natural settlementDays,
                                                       const Calendar& calendar,
                                                       BusinessDayConvention convention,
                                                       const ext::shared_ptr<IborIndex>& idx,
                                                       VanillaSwap::Type type,
                                                       Real notional = 1.0,
                                                       Spread basis = 0.0);
        //@}
      protected:
        void initializeDates();

        Period tenor_;
        Natural fixingDays_;
        Calendar calendar_;
        BusinessDayConvention convention_;
        ext::shared_ptr<IborIndex> baseCcyIdx_;
        ext::shared_ptr<IborIndex> quoteCcyIdx_;

        ext::shared_ptr<Swap> baseCcyLeg_;
        ext::shared_ptr<Swap> quoteCcyLeg_;
        bool isFxBaseCurrencyCollateralCurrency_;
        bool isBasisOnFxBaseCurrencyLeg_;

        RelinkableHandle<YieldTermStructure> termStructureHandle_;
        Handle<YieldTermStructure> collateralHandle_;
    };
}

#endif
