/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2026 Colin Alberts

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

/*! \file analyticroughhestonengine.hpp
    \brief analytic rough Heston-model engine
*/

#ifndef quantlib_analytic_rough_heston_engine_hpp
#define quantlib_analytic_rough_heston_engine_hpp

#include <ql/instruments/vanillaoption.hpp>
#include <ql/models/equity/roughhestonmodel.hpp>
#include <ql/pricingengines/vanilla/analytichestonengine.hpp>
#include <complex>
#include <map>
#include <tuple>

namespace QuantLib {

    //! analytic rough Heston-model engine based on Fourier transform
    /*! The log-price characteristic function of the rough Heston model
        retains the affine structure of the classical Heston model, with
        the Riccati ODE replaced by the fractional Riccati equation
        \f[
            D^{\alpha} h(z, t) = -\frac{1}{2} z (z+i)
                + (i z \rho \sigma - \kappa)\, h(z, t)
                + \frac{1}{2} \sigma^2 h^2(z, t),
            \qquad I^{1-\alpha} h(z, 0) = 0,
        \f]
        with \f$ \alpha = H + \frac{1}{2} \f$, giving
        \f[
            E\!\left[e^{i z \ln (S_t/F_t)}\right]
              = \exp\!\left( \kappa\theta\, I^{1} h(z, t)
                             + V_0\, I^{1-\alpha} h(z, t) \right).
        \f]
        This engine solves the fractional Riccati equation numerically with
        the Diethelm-Ford-Freed Adams predictor-corrector scheme and prices
        European options with the Andersen-Piterbarg Fourier integral,
        using a Black-Scholes control variate whose variance is matched to
        the model characteristic function.  Characteristic function values
        are cached per frequency and maturity, so that with a non-adaptive
        integration rule (the Gauss-Laguerre default) all strikes sharing a
        maturity reuse the same fractional Riccati solutions -- the typical
        calibration access pattern.

        \note The fractional Adams scheme is the reference-quality but
              \f$ O(N^2) \f$ route to the characteristic function.  Future
              pull requests are planned to add faster approximations,
              among them the rational (Pade) approximation of Gatheral and
              Radoicic, the multi-factor "lifted Heston" Markovian
              approximation of Abi Jaber, and support for forward-variance
              term structures.

        References:

        O. El Euch and M. Rosenbaum, The characteristic function of rough
        Heston models, Mathematical Finance 29(1), 3-38 (2019).

        K. Diethelm, N.J. Ford, A.D. Freed, A Predictor-Corrector Approach
        for the Numerical Solution of Fractional Differential Equations,
        Nonlinear Dynamics 29, 3-22 (2002).

        L. Andersen and V. Piterbarg, Interest Rate Modeling, Volume I:
        Foundations and Vanilla Models, Atlantic Financial Press (2010).

        \ingroup vanillaengines

        \test the correctness of the returned value is tested by comparison
              against the classical Heston model for Hurst exponent
              \f$ H = \frac{1}{2} \f$, against values from an independent
              implementation, and by checking known qualitative properties
              of rough volatility (short-maturity skew explosion).
    */
    class AnalyticRoughHestonEngine
        : public GenericModelEngine<RoughHestonModel,
                                    VanillaOption::arguments,
                                    VanillaOption::results> {
      public:
        typedef AnalyticHestonEngine::Integration Integration;

        // Constructor using Gauss-Laguerre integration; timeSteps is the
        // number of steps of the fractional Adams scheme per maturity.
        explicit AnalyticRoughHestonEngine(
            const ext::shared_ptr<RoughHestonModel>& model,
            Size integrationOrder = 128,
            Size timeSteps = 256);

        // Constructor giving full control over the Fourier integration
        // algorithm; alpha is the payoff dampening exponent, which must
        // lie in (-1, 0) to keep the required moments finite.
        AnalyticRoughHestonEngine(
            const ext::shared_ptr<RoughHestonModel>& model,
            const Integration& integration,
            Size timeSteps = 256,
            Real andersenPiterbargEpsilon = 1e-25,
            Real alpha = -0.5);

        void update() override;
        void calculate() const override;

        Real priceVanillaPayoff(
            const ext::shared_ptr<PlainVanillaPayoff>& payoff,
            const Date& maturity) const;
        Real priceVanillaPayoff(
            const ext::shared_ptr<PlainVanillaPayoff>& payoff,
            Time maturity) const;

        // normalized characteristic function of the log forward moneyness,
        // E[exp(i z ln(S_t/F_t))]
        std::complex<Real> chF(const std::complex<Real>& z, Time t) const;
        std::complex<Real> lnChF(const std::complex<Real>& z, Time t) const;

        Size numberOfEvaluations() const;

      private:
        class AP_Helper;

        Real priceVanillaPayoff(
            const ext::shared_ptr<PlainVanillaPayoff>& payoff,
            Time maturity, Real fwd) const;

        const Size timeSteps_;
        const ext::shared_ptr<Integration> integration_;
        const Real andersenPiterbargEpsilon_, alpha_;

        mutable Size evaluations_ = 0;
        mutable std::map<std::tuple<Real, Real, Time>, std::complex<Real> >
            chFCache_;
    };
}

#endif
