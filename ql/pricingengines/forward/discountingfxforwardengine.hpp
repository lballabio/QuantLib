/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2024

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

#include <ql/currency.hpp>
#include <ql/handle.hpp>
#include <ql/instruments/fxforward.hpp>
#include <ql/optional.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>

namespace QuantLib {

    //! Discounting engine for FX Forward
    /*! This engine discounts the two legs of an FX forward using their
        respective currency discount curves.

        The NPV is computed as:
        \f[
        \text{NPV} = \pm N_{\text{dom}} \times D_{\text{dom}}(T)
                     \mp N_{\text{for}} \times D_{\text{for}}(T) \times X
        \f]
        where:
        - \f$ N_{\text{dom}} \f$ is the domestic currency nominal
        - \f$ N_{\text{for}} \f$ is the foreign currency nominal
        - \f$ D_{\text{dom}}(T) \f$ is the domestic discount factor to maturity
        - \f$ D_{\text{for}}(T) \f$ is the foreign discount factor to maturity
        - \f$ X \f$ is the spot FX rate (domestic/foreign)
        - \f$ T \f$ is the maturity date

        The fair forward rate is computed as:
        \f[
        F = X \times \frac{D_{\text{for}}(T)}{D_{\text{dom}}(T)}
        \f]

        \ingroup forwardengines
    */
    class DiscountingFxForwardEngine : public FxForward::engine {
      public:
        /*! \param currency1DiscountCurve   Discount curve for currency1
            \param currency2DiscountCurve   Discount curve for currency2
            \param spotFx                   Spot FX rate (currency2/currency1), i.e.,
                                            1 unit of currency1 = spotFx units of currency2
            \param includeSettlementDateFlows Whether to include flows on settlement date
            \param settlementDate           The settlement date for discounting
            \param npvDate                  The date to which NPV is discounted
        */
        DiscountingFxForwardEngine(
            const Currency& currency1,
            Handle<YieldTermStructure> currency1DiscountCurve,
            const Currency& currency2,
            Handle<YieldTermStructure> currency2DiscountCurve,
            Handle<Quote> spotFx,
            const ext::optional<bool>& includeSettlementDateFlows = ext::nullopt,
            const Date& settlementDate = Date(),
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
        Currency currency1_;
        Handle<YieldTermStructure> currency1DiscountCurve_;
        Currency currency2_;
        Handle<YieldTermStructure> currency2DiscountCurve_;
        Handle<Quote> spotFx_;
        ext::optional<bool> includeSettlementDateFlows_;
        Date settlementDate_;
        Date npvDate_;
    };

}

#endif
