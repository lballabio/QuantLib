/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2026 Raghav Subbarao

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

/*! \file fxsmilesectionbydelta.hpp
    \brief FX smile sections parameterised in delta
*/

#ifndef quantlib_fx_smile_section_delta_hpp
#define quantlib_fx_smile_section_delta_hpp

#include <ql/math/array.hpp>
#include <ql/experimental/fxslv/fxsmilesection.hpp>

namespace QuantLib {

    class FxSmileSectionByDelta : public FxSmileSection {
      public:
        //! Date mode (see FxSmileSection).
        FxSmileSectionByDelta(const Date& exerciseDate,
                              const Handle<Quote>& spot,
                              const ext::shared_ptr<FxSmileQuotes>& quotes,
                              const Handle<YieldTermStructure>& foreignDiscount,
                              const Handle<YieldTermStructure>& domesticDiscount,
                              DeltaVolQuote::DeltaType deltaType,
                              DeltaVolQuote::AtmType atmType,
                              const DayCounter& dayCounter,
                              const FxSettleConvention& settlement,
                              const Date& referenceDate = Date());

        //! Time mode (see FxSmileSection).
        FxSmileSectionByDelta(Time exerciseTime,
                              const Handle<Quote>& spot,
                              const ext::shared_ptr<FxSmileQuotes>& quotes,
                              const Handle<YieldTermStructure>& foreignDiscount,
                              const Handle<YieldTermStructure>& domesticDiscount,
                              DeltaVolQuote::DeltaType deltaType,
                              DeltaVolQuote::AtmType atmType,
                             const DayCounter& dayCounter = DayCounter());

        //! \name FxSmileSection interface
        //@{
        Volatility volByStrike(Rate strike) const override;
        Volatility volByDelta(Real delta, Option::Type parity) const override;
        Real deltaByStrike(Rate strike, Option::Type parity) const override;
        Rate strikeByDelta(Real delta, Option::Type parity) const override;
        //! Residual at the point's put delta, the natural coordinate here.
        Real volResidual(Rate strike, Volatility vol) const override;
        //@}

      private:
        virtual Volatility volByDeltaImpl(Real delta,
                                          Real fwd,
                                          const std::vector<Real>& params) const = 0;


      protected:
        //! \name FxSmileSection interface
        //@{
        void setParams(const Array& params) const override {
            params_.assign(params.begin(), params.end());
        }
        //@}

        mutable std::vector<Real> params_;
    };



    //! Quadratic smile section parameterized by put delta.
    /*! Implied volatility is a quadratic function of put delta:
        \f$ \sigma(\Delta) = a \Delta^2 + b \Delta + c \f$
    */
    class FxQuadraticSmileSection : public FxSmileSectionByDelta {
      public:
        //! Date mode (see FxSmileSection).
        FxQuadraticSmileSection(const Date& exerciseDate,
                                const Handle<Quote>& spot,
                                const ext::shared_ptr<FxSmileQuotes>& quotes,
                                const Handle<YieldTermStructure>& foreignDiscount,
                                const Handle<YieldTermStructure>& domesticDiscount,
                                DeltaVolQuote::DeltaType deltaType,
                                DeltaVolQuote::AtmType atmType,
                                const DayCounter& dayCounter,
                                const FxSettleConvention& settlement,
                                const Date& referenceDate = Date());

        //! Time mode (see FxSmileSection).
        FxQuadraticSmileSection(Time exerciseTime,
                                const Handle<Quote>& spot,
                                const ext::shared_ptr<FxSmileQuotes>& quotes,
                                const Handle<YieldTermStructure>& foreignDiscount,
                                const Handle<YieldTermStructure>& domesticDiscount,
                                DeltaVolQuote::DeltaType deltaType,
                                DeltaVolQuote::AtmType atmType,
                                const DayCounter& dayCounter = DayCounter());

        // Introspection
        Real a() const { calculate(); return params_[0]; }
        Real b() const { calculate(); return params_[1]; }
        Real c() const { calculate(); return params_[2]; }

      private:
        //! \name FxSmileSectionByDelta interface
        //@{
        Volatility volByDeltaImpl(Real delta,
                                  Real fwd,
                                  const std::vector<Real>& params) const override;
        //@}

      protected:
        Array initialParams() const override;
    };

} // namespace QuantLib

#endif
