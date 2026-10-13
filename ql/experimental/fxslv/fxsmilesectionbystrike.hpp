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

/*! \file fxsmilesectionbystrike.hpp
    \brief FX smile sections parameterised in strike
*/

#ifndef quantlib_fx_smile_section_strike_hpp
#define quantlib_fx_smile_section_strike_hpp

#include <ql/math/array.hpp>
#include <ql/experimental/fxslv/fxsmilesection.hpp>

namespace QuantLib {

    class FxSmileSectionByStrike : public FxSmileSection {
      public:
        //! Date mode (see FxSmileSection).
        FxSmileSectionByStrike(const Date& exerciseDate,
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
        FxSmileSectionByStrike(Time exerciseTime,
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
        //@}

      private:
        virtual Volatility volByStrikeImpl(Real strike,
                                           Real fwd,
                                           Time tau,
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



    class FxPolynomialSmileSection : public FxSmileSectionByStrike {
      public:
        //! Date mode (see FxSmileSection).
        FxPolynomialSmileSection(const Date& exerciseDate,
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
        FxPolynomialSmileSection(Time exerciseTime,
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
        //! \name FxSmileSectionByStrike interface
        //@{
        Volatility volByStrikeImpl(Real strike,
                                   Real fwd,
                                   Time tau,
                                   const std::vector<Real>& params) const override;
        //@}

      protected:
        Array initialParams() const override;
    };


    class FxSabrSmileSection : public FxSmileSectionByStrike {
      public:
        //! Date mode (see FxSmileSection).
        FxSabrSmileSection(const Date& exerciseDate,
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
        FxSabrSmileSection(Time exerciseTime,
                           const Handle<Quote>& spot,
                           const ext::shared_ptr<FxSmileQuotes>& quotes,
                           const Handle<YieldTermStructure>& foreignDiscount,
                           const Handle<YieldTermStructure>& domesticDiscount,
                           DeltaVolQuote::DeltaType deltaType,
                           DeltaVolQuote::AtmType atmType,
                           const DayCounter& dayCounter = DayCounter());

        // Introspection
        Real alpha() const { calculate(); return params_[0]; }
        Real beta() const { return 1.0; }
        Real nu() const { calculate(); return params_[1]; }
        Real rho() const { calculate(); return params_[2]; }

      private:
        //! \name FxSmileSectionByStrike interface
        //@{
        Volatility volByStrikeImpl(Real strike,
                                   Real fwd,
                                   Time tau,
                                   const std::vector<Real>& params) const override;
        //@}

      protected:
        //! \name FxSmileSection interface
        /*! The fit searches \f$ x \f$ with \f$ \alpha = e^{x_0} \f$,
            \f$ \nu = e^{x_1} \f$ and \f$ \rho = \tanh x_2 \f$, so that
            every point it tries is a valid SABR smile: \f$ \alpha > 0 \f$,
            \f$ \nu > 0 \f$, \f$ |\rho| < 1 \f$.
        */
        //@{
        Array initialParams() const override;
        void setParams(const Array& x) const override;
        //@}
    };


    //! SVI (Stochastic Volatility Inspired) smile section.
    /*! Total implied variance is given by the SVI raw parameterization:
        \f$ w(k) = a + b \left( \rho (k - m) + \sqrt{(k - m)^2 + \sigma^2} \right) \f$
        where \f$ k = \log(K / F) \f$ is the log-moneyness.
        Implied volatility is \f$ \sigma_{impl} = \sqrt{w(k) / \tau} \f$.

        Parameters: a, b, rho, m, sigma (5 parameters), with
        \f$ b > 0 \f$, \f$ |\rho| < 1 \f$, \f$ \sigma > 0 \f$ and a
        positive minimum total variance
        \f$ a + b \sigma \sqrt{1 - \rho^2} > 0 \f$, which the fit
        guarantees.  The wing slopes \f$ b(1 \pm \rho) \f$ must not
        exceed 2 (Roger Lee's moment bound), which is checked after the fit.
    */
    class FxSviSmileSection : public FxSmileSectionByStrike {
      public:
        //! Date mode (see FxSmileSection).
        FxSviSmileSection(const Date& exerciseDate,
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
        FxSviSmileSection(Time exerciseTime,
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
        Real rho() const { calculate(); return params_[2]; }
        Real m() const { calculate(); return params_[3]; }
        Real sigma() const { calculate(); return params_[4]; }
        //! Minimum total variance, \f$ a + b \sigma \sqrt{1 - \rho^2} \f$.
        Real minimumVariance() const;

      private:
        //! \name FxSmileSectionByStrike interface
        //@{
        Volatility volByStrikeImpl(Real strike,
                                   Real fwd,
                                   Time tau,
                                   const std::vector<Real>& params) const override;
        //@}

      protected:
        //! \name FxSmileSection interface
        /*! The fit searches \f$ x \f$ with \f$ b = e^{x_1} \f$,
            \f$ \rho = \tanh x_2 \f$, \f$ m = x_3 \f$,
            \f$ \sigma = e^{x_4} \f$ and minimum total variance
            \f$ e^{x_0} \f$, i.e.
            \f$ a = e^{x_0} - b \sigma \sqrt{1 - \rho^2} \f$, so that
            every point it tries is a valid SVI smile.
        */
        //@{
        Array initialParams() const override;
        void setParams(const Array& x) const override;
        void calibrate() const override;
        //@}
    };

} // namespace QuantLib

#endif
