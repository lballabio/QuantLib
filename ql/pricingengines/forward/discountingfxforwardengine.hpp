/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2026 Chirag Desai

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

/*! \file discountingfxforwardengine.hpp
    \brief Discounting FX Forward engine
*/

#ifndef quantlib_discounting_fx_forward_engine_hpp
#define quantlib_discounting_fx_forward_engine_hpp

#include <ql/handle.hpp>
#include <ql/instruments/fxforward.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>

namespace QuantLib {

    //! Discounting engine for FX Forward
    /*! This engine discounts the two legs of an FX forward using their
        respective currency discount curves.

        The NPV is computed as:
        \f[
        \text{NPV} = \pm N_{source} \times D_{source}(T) \mp N_{target} \times D_{target}(T) / S
        \f]
        where:
        - \f$ N_{source} \f$ is the source currency nominal
        - \f$ N_{target} \f$ is the target currency nominal
        - \f$ D_{source}(T) \f$ is the source currency discount factor to maturity
        - \f$ D_{target}(T) \f$ is the target currency discount factor to maturity
        - \f$ S \f$ is the spot FX rate (target/source)
        - \f$ T \f$ is the maturity date

        The fair forward rate is computed as:
        \f[
        F = S \times \frac{D_{target}(T)}{D_{source}(T)}
        \f]

        \ingroup forwardengines
    */
    class DiscountingFxForwardEngine : public FxForward::engine {
      public:
        /*! \param sourceCurrencyDiscountCurve  Discount curve for source currency
            \param targetCurrencyDiscountCurve  Discount curve for target currency
            \param spotFx                       Spot FX rate (target/source), i.e.,
                                                1 unit of source currency = spotFx units of target currency
        */
        DiscountingFxForwardEngine(Handle<YieldTermStructure> sourceCurrencyDiscountCurve,
                                   Handle<YieldTermStructure> targetCurrencyDiscountCurve,
                                   Handle<Quote> spotFx);

        void calculate() const override;

        //! \name Inspectors
        //@{
        const Handle<YieldTermStructure>& sourceCurrencyDiscountCurve() const {
            return sourceCurrencyDiscountCurve_;
        }
        const Handle<YieldTermStructure>& targetCurrencyDiscountCurve() const {
            return targetCurrencyDiscountCurve_;
        }
        const Handle<Quote>& spotFx() const { return spotFx_; }
        //@}

      private:
        Handle<YieldTermStructure> sourceCurrencyDiscountCurve_;
        Handle<YieldTermStructure> targetCurrencyDiscountCurve_;
        Handle<Quote> spotFx_;
    };

}

#endif
