/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2024 Chirag Desai

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
        \text{NPV} = \pm N_1 \times D_1(T) \mp N_2 \times D_2(T) / S
        \f]
        where:
        - \f$ N_1 \f$ is the currency1 nominal
        - \f$ N_2 \f$ is the currency2 nominal
        - \f$ D_1(T) \f$ is the currency1 discount factor to maturity
        - \f$ D_2(T) \f$ is the currency2 discount factor to maturity
        - \f$ S \f$ is the spot FX rate (currency2/currency1)
        - \f$ T \f$ is the maturity date

        The fair forward rate is computed as:
        \f[
        F = S \times \frac{D_2(T)}{D_1(T)}
        \f]

        \ingroup forwardengines
    */
    class DiscountingFxForwardEngine : public FxForward::engine {
      public:
        /*! \param currency1DiscountCurve   Discount curve for currency1
            \param currency2DiscountCurve   Discount curve for currency2
            \param spotFx                   Spot FX rate (currency2/currency1), i.e.,
                                            1 unit of currency1 = spotFx units of currency2
            \param npvDate                  The date to which NPV is discounted
                                            (defaults to the discount curve reference date)
        */
        DiscountingFxForwardEngine(
            Handle<YieldTermStructure> currency1DiscountCurve,
            Handle<YieldTermStructure> currency2DiscountCurve,
            Handle<Quote> spotFx,
            const Date& npvDate = Date());

        void calculate() const override;

        //! \name Inspectors
        //@{
        const Handle<YieldTermStructure>& currency1DiscountCurve() const {
            return currency1DiscountCurve_;
        }
        const Handle<YieldTermStructure>& currency2DiscountCurve() const {
            return currency2DiscountCurve_;
        }
        const Handle<Quote>& spotFx() const { return spotFx_; }
        //@}

      private:
        Handle<YieldTermStructure> currency1DiscountCurve_;
        Handle<YieldTermStructure> currency2DiscountCurve_;
        Handle<Quote> spotFx_;
        Date npvDate_;
    };

}

#endif
