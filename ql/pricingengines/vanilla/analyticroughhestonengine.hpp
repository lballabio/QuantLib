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
#include <ql/math/array.hpp>
#include <ql/math/integrals/fourierintegration.hpp>
#include <ql/models/equity/roughhestonmodel.hpp>
#include <ql/pricingengines/genericmodelengine.hpp>
#include <complex>
#include <map>
#include <tuple>
#include <vector>

namespace QuantLib {

    //! Analytic rough Heston-model engine based on Fourier transform
    /*! The log-price characteristic function of the rough Heston model
        retains the affine structure of the classical Heston model, with
        the Riccati ODE replaced by a fractional Riccati equation.  This
        engine solves the latter by one of three routes: the reference
        Diethelm-Ford-Freed Adams predictor-corrector scheme, the Padè
        approximation, or the lifted (multi-factor Markovian)
        approximation.  European options are priced with the
        Andersen-Piterbarg Fourier integral and a Black-Scholes control
        variate.

        \note The fractional Adams scheme is the reference-quality but
              \f$ O(N^2) \f$ route to the characteristic function.  The
              Padè approximation is a closed form matching the short
              and long-time asymptotics of the Riccati solution; it is
              most precise for the strongly negative correlation typical of
              equity indices and loses a few percent of accuracy as the
              correlation moves towards zero or positive.  The lifted
              approximation replaces the power-law kernel by a sum of
              \f$ n \f$ exponentials, which turns the fractional Riccati
              equation into an \f$ n \f$-dimensional classical one solved
              in \f$ O(n N) \f$. Future pull requests will add forward-variance 
              term structures.

        References:

        O. El Euch and M. Rosenbaum, The characteristic function of rough
        Heston models, Mathematical Finance 29(1), 3-38 (2019).

        K. Diethelm, N.J. Ford, A.D. Freed, A Predictor-Corrector Approach
        for the Numerical Solution of Fractional Differential Equations,
        Nonlinear Dynamics 29, 3-22 (2002).

        J. Gatheral and R. Radoicic, Rational approximation of the rough
        Heston solution, International Journal of Theoretical and Applied
        Finance 22(3), 1950010 (2019).

        E. Abi Jaber, Lifting the Heston model, Quantitative Finance
        19(12), 1995-2013 (2019).

        \ingroup vanillaengines

        \test the correctness of the returned value is tested by comparison
              against the classical Heston model for Hurst exponent
              \f$ H = \frac{1}{2} \f$, against values from an independent
              implementation, and by checking known qualitative properties
              of rough volatility.  The Padè and lifted paths are validated
              against the Adams reference across the pricing contour.
    */
    class AnalyticRoughHestonEngine
        : public GenericModelEngine<RoughHestonModel,
                                    VanillaOption::arguments,
                                    VanillaOption::results> {
      public:
        typedef FourierIntegration Integration;

        //! Route used to solve the fractional Riccati equation
        enum class Approximation { AdamsPredictorCorrector, Pade, Lifted };

        /*! Constructor using Gauss-Laguerre integration. nFactors is the
            number of exponentials of the lifted kernel and is ignored by
            the other routes.
        */
        explicit AnalyticRoughHestonEngine(
            const ext::shared_ptr<RoughHestonModel>& model,
            Size integrationOrder = 128,
            Size timeSteps = 256,
            Approximation approximation
                = Approximation::AdamsPredictorCorrector,
            Size nFactors = 20);

        /*! Constructor giving full control over the Fourier integration
            algorithm. alpha is the payoff dampening exponent, which must
            lie in (-1, 0) to keep the required moments finite.
        */
        AnalyticRoughHestonEngine(
            const ext::shared_ptr<RoughHestonModel>& model,
            const Integration& integration,
            Size timeSteps = 256,
            Real andersenPiterbargEpsilon = 1e-25,
            Real alpha = -0.5,
            Approximation approximation
                = Approximation::AdamsPredictorCorrector,
            Size nFactors = 20);

        void update() override;
        void calculate() const override;

        Real priceVanillaPayoff(
            const ext::shared_ptr<PlainVanillaPayoff>& payoff,
            const Date& maturity) const;
        Real priceVanillaPayoff(
            const ext::shared_ptr<PlainVanillaPayoff>& payoff,
            Time maturity) const;

        /*! Normalized characteristic function of the log forward moneyness,
            \f$ E\left[\exp(i z \ln(S_t/F_t))\right] \f$
        */
        std::complex<Real> chF(const std::complex<Real>& z, Time t) const;
        std::complex<Real> lnChF(const std::complex<Real>& z, Time t) const;

        /*! Solution \f$ h(z, t) \f$ of the fractional Riccati equation,
            computed by the route configured at construction
        */
        std::complex<Real> riccatiSolution(const std::complex<Real>& z, Time t) const;

        Size numberOfEvaluations() const;

      private:
        class AP_Helper;

        //! Coefficients c0, c1, c2 of the Riccati equation h' = c0 + c1 h + c2 h^2
        struct RiccatiCoefficients {
            std::complex<Real> c0, c1, c2;
        };
        RiccatiCoefficients riccatiCoefficients(const std::complex<Real>& z) const;

        // Fractional Adams solution grid h(z, j dt), j = 0, ..., timeSteps_
        std::vector<std::complex<Real>> solveAdamsRiccati(const std::complex<Real>& z, Time t) const;

        /*! Coefficients of the (3, 3) Gatheral-Radoicic rational
            approximation \f$ h(z, t) = N(y) / D(y) \f$ with
            \f$ y = \sigma t^a \f$, matching the short-time series and
            the long-time root of the Riccati solution
        */
        struct PadeCoefficients {
            std::complex<Real> p1, p2, p3, q1, q2, q3;
        };
        PadeCoefficients padeCoefficients(const std::complex<Real>& z) const;
        std::complex<Real> padeRiccati(const std::complex<Real>& z, Time t) const;
        static std::complex<Real> evaluatePade(const PadeCoefficients& c, Real y);

        /*! Per-maturity data of the lifted approximation: the exponential
            kernel nodes \f$ (c_i, x_i) \f$ and the step weights of the
            integrating-factor scheme.  All of it is real and independent
            of the frequency \f$ z \f$, so it is built once per maturity
            and shared by all quadrature nodes.
        */
        struct LiftedGrid {
            Array c;
            
            // e^{-x_i dt}, its c_i-weighted form, and the weights of
            // F_k, F_{k + 1} in the step
            Array expDt, cExpDt, w0, w1;
            
            // c_i (1 - e^{-x_i dt}) / x_i and its sum, the psi_i and
            // constant-F weights of the psi-integral quadrature
            Array cA;
            Real cASum;

            // The F_k and (F_{k + 1} - F_k) weights of the same quadrature
            Real cP, cQ;
        };
        const LiftedGrid& liftedGrid(Time t) const;

        //! psi^n(z, t) together with the two integrals the chF is built from
        struct LiftedSolution {
            std::complex<Real> psi, fIntegral, psiIntegral;
        };
        LiftedSolution solveLiftedRiccati(
            const std::complex<Real>& z, Time t) const;

        std::complex<Real> lnChFAdams(const std::complex<Real>& z, Time t) const;
        std::complex<Real> lnChFPade(const std::complex<Real>& z, Time t) const;
        std::complex<Real> lnChFLifted(const std::complex<Real>& z, Time t) const;

        Real priceVanillaPayoff(
            const ext::shared_ptr<PlainVanillaPayoff>& payoff,
            Time maturity, Real fwd) const;

        const Size timeSteps_;
        const Integration integration_;
        const Real andersenPiterbargEpsilon_, alpha_;
        const Approximation approximation_;
        const Size nFactors_;

        mutable Size evaluations_{0};
        mutable std::map<std::tuple<Real, Real, Time>, std::complex<Real>>
            chFCache_;
        mutable std::map<Time, LiftedGrid> liftedGridCache_;
    };
}

#endif
