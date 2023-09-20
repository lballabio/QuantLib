/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2004, 2005, 2008 Klaus Spanderen
 Copyright (C) 2007 StatPro Italia srl

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

/*! \file analytichestonengine.hpp
    \brief analytic Heston-model engine
*/

#ifndef quantlib_analytic_heston_engine_hpp
#define quantlib_analytic_heston_engine_hpp

#include <ql/utilities/null.hpp>
#include <ql/math/integrals/integral.hpp>
#include <ql/math/integrals/gaussianquadratures.hpp>
#include <ql/pricingengines/genericmodelengine.hpp>
#include <ql/models/equity/hestonmodel.hpp>
#include <ql/instruments/vanillaoption.hpp>
#include <ql/functional.hpp>
#include <complex>

namespace QuantLib {

    //! analytic Heston-model engine based on Fourier transform

    /*! Integration detail:
        Two algebraically equivalent formulations of the complex
        logarithm of the Heston model exist. Gatherals [2005]
        (also Duffie, Pan and Singleton [2000], and Schoutens,
        Simons and Tistaert[2004]) version does not cause
        discoutinuities whereas the original version (e.g. Heston [1993])
        needs some sort of "branch correction" to work properly.
        Gatheral's version does also work with adaptive integration
        routines and should be preferred over the original Heston version.
    */

    /*! References:

        Heston, Steven L., 1993. A Closed-Form Solution for Options
        with Stochastic Volatility with Applications to Bond and
        Currency Options.  The review of Financial Studies, Volume 6,
        Issue 2, 327-343.

        A. Sepp, Pricing European-Style Options under Jump Diffusion
        Processes with Stochastic Volatility: Applications of Fourier
        Transform (<http://math.ut.ee/~spartak/papers/stochjumpvols.pdf>)

        R. Lord and C. Kahl, Why the rotation count algorithm works,
        http://papers.ssrn.com/sol3/papers.cfm?abstract_id=921335

        H. Albrecher, P. Mayer, W.Schoutens and J. Tistaert,
        The Little Heston Trap, http://www.schoutens.be/HestonTrap.pdf

        J. Gatheral, The Volatility Surface: A Practitioner's Guide,
        Wiley Finance

        F. Le Floc'h, Fourier Integration and Stochastic Volatility
        Calibration,
        https://papers.ssrn.com/sol3/papers.cfm?abstract_id=2362968

        L. Andersen, and V. Piterbarg, 2010,
        Interest Rate Modeling, Volume I: Foundations and Vanilla Models,
        Atlantic Financial Press London.

        L. Andersen and M. Lake, 2018
        Robust High-Precision Option Pricing by Fourier Transforms:
        Contour Deformations and Double-Exponential Quadrature,
        https://papers.ssrn.com/sol3/papers.cfm?abstract_id=3231626

        \ingroup vanillaengines

        \test the correctness of the returned value is tested by
              reproducing results available in web/literature
              and comparison with Black pricing.
    */
    class AnalyticHestonEngine
        : public GenericModelEngine<HestonModel,
                                    VanillaOption::arguments,
                                    VanillaOption::results> {
      public:
        class Integration;
        class OptimalAlpha;
        class AP_Helper;

        enum ComplexLogFormula {
            // Gatheral form of characteristic function w/o control variate
            Gatheral,
            // old branch correction form of the characteristic function w/o control variate
            BranchCorrection,
            // Gatheral form with Andersen-Piterbarg control variate
            AndersenPiterbarg,
            // same as AndersenPiterbarg, but a slightly better control variate
            AndersenPiterbargOptCV,
            // Gatheral form with asymptotic expansion of the characteristic function as control variate
            // https://hpcquantlib.wordpress.com/2020/08/30/a-novel-control-variate-for-the-heston-model
            AsymptoticChF,
            // angled contour shift integral with control variate
            AngledContour,
            // angled contour shift integral w/o control variate
            AngledContourNoCV,
            // auto selection of best control variate algorithm from above
            OptimalCV
        };

        // Simple to use constructor: Using adaptive
        // Gauss-Lobatto integration and Gatheral's version of complex log.
        // Be aware: using a too large number for maxEvaluations might result
        // in a stack overflow as the Lobatto integration is a recursive
        // algorithm.
        AnalyticHestonEngine(const ext::shared_ptr<HestonModel>& model,
                             Real relTolerance, Size maxEvaluations);

        // Constructor using Laguerre integration
        // and Gatheral's version of complex log.
        AnalyticHestonEngine(const ext::shared_ptr<HestonModel>& model,
                             Size integrationOrder = 144);

        // Constructor giving full control
        // over the Fourier integration algorithm
        AnalyticHestonEngine(const ext::shared_ptr<HestonModel>& model,
                             ComplexLogFormula cpxLog, const Integration& itg,
                             Real andersenPiterbargEpsilon = 1e-25,
                             Real alpha = -0.5);

        void calculate() const override;

        // normalized characteristic function
        std::complex<Real> chF(const std::complex<Real>& z, Time t) const;
        std::complex<Real> lnChF(const std::complex<Real>& z, Time t) const;

        Size numberOfEvaluations() const;

        Real priceVanillaPayoff(
           const ext::shared_ptr<PlainVanillaPayoff>& payoff,
           const Date& maturity) const;

        Real priceVanillaPayoff(
           const ext::shared_ptr<PlainVanillaPayoff>& payoff, Time maturity) const;

        static ComplexLogFormula optimalControlVariate(
             Time t, Real v0, Real kappa, Real theta, Real sigma, Real rho);

      protected:
        // call back for extended stochastic volatility
        // plus jump diffusion engines like bates model
        virtual std::complex<Real> addOnTerm(Real phi,
                                             Time t,
                                             Size j) const;

      private:
        class Fj_Helper;

        Real priceVanillaPayoff(
           const ext::shared_ptr<PlainVanillaPayoff>& payoff,
           const Time maturity, Real fwd) const;


        mutable Size evaluations_;
        const ComplexLogFormula cpxLog_;
        const ext::shared_ptr<Integration> integration_;
        const Real andersenPiterbargEpsilon_, alpha_;
    };


    class AnalyticHestonEngine::Integration {
      public:
        // non adaptive integration algorithms based on Gaussian quadrature
        static Integration gaussLaguerre    (Size integrationOrder = 128);
        static Integration gaussLegendre    (Size integrationOrder = 128);
        static Integration gaussChebyshev   (Size integrationOrder = 128);
        static Integration gaussChebyshev2nd(Size integrationOrder = 128);

        // Gatheral's version has to be used for an adaptive integration
        // algorithm .Be aware: using a too large number for maxEvaluations might
        // result in a stack overflow as the these integrations are based on
        // recursive algorithms.
        static Integration gaussLobatto(Real relTolerance, Real absTolerance,
                                        Size maxEvaluations = 1000,
                                        bool useConvergenceEstimate = false);

        // usually these routines have a poor convergence behavior.
        static Integration gaussKronrod(Real absTolerance,
                                        Size maxEvaluations = 1000);
        static Integration simpson(Real absTolerance,
                                   Size maxEvaluations = 1000);
        static Integration trapezoid(Real absTolerance,
                                     Size maxEvaluations = 1000);
        static Integration discreteSimpson(Size evaluation = 1000);
        static Integration discreteTrapezoid(Size evaluation = 1000);

        static Real andersenPiterbargIntegrationLimit(
            Real c_inf, Real epsilon, Real v0, Real t);

        Real calculate(Real c_inf,
                       const ext::function<Real(Real)>& f,
                       const ext::function<Real()>& maxBound = {}) const;

        Real calculate(Real c_inf,
                       const ext::function<Real(Real)>& f,
                       Real maxBound) const;

        Size numberOfEvaluations() const;
        bool isAdaptiveIntegration() const;

      private:
        enum Algorithm
            { GaussLobatto, GaussKronrod, Simpson, Trapezoid,
              DiscreteTrapezoid, DiscreteSimpson,
              GaussLaguerre, GaussLegendre,
              GaussChebyshev, GaussChebyshev2nd };

        Integration(Algorithm intAlgo, ext::shared_ptr<GaussianQuadrature> quadrature);

        Integration(Algorithm intAlgo, ext::shared_ptr<Integrator> integrator);

        const Algorithm intAlgo_;
        const ext::shared_ptr<Integrator> integrator_;
        const ext::shared_ptr<GaussianQuadrature> gaussianQuadrature_;
    };

    class AnalyticHestonEngine::AP_Helper {
      public:
        AP_Helper(Time term, Real fwd, Real strike,
                  ComplexLogFormula cpxLog,
                  const AnalyticHestonEngine* enginePtr,
                  const Real alpha = -0.5);

        Real operator()(Real u) const;
        Real controlVariateValue() const;

      private:
        const Time term_;
        const Real fwd_, strike_, freq_;
        const ComplexLogFormula cpxLog_;
        const AnalyticHestonEngine* const enginePtr_;
        const Real alpha_, s_alpha_;
        Real vAvg_, tanPhi_;
        std::complex<Real> phi_, psi_;
    };


    class AnalyticHestonEngine::OptimalAlpha {
      public:
        OptimalAlpha(
            const Time t,
            const AnalyticHestonEngine* const enginePtr);

        Real operator()(Real strike) const;
        std::pair<Real, Real> alphaGreaterZero(Real strike) const;
        std::pair<Real, Real> alphaSmallerMinusOne(Real strike) const;



        Size numberOfEvaluations() const;
        Real M(Real k) const;
        Real k(Real x, Integer sgn) const;
        Real alphaMin(Real strike) const;
        Real alphaMax(Real strike) const;

      private:
        std::pair<Real, Real> findMinima(Real lower, Real upper, Real strike) const;

        const Real t_, fwd_, kappa_, theta_, sigma_, rho_;

        const int bits_;
        const Real eps_;

        const AnalyticHestonEngine* const enginePtr_;
        Real km_, kp_;
        mutable Size evaluations_;
    };


    inline std::complex<Real> AnalyticHestonEngine::addOnTerm(
        Real, Time, Size) const {
        return std::complex<Real>(0,0);
    }
}

#endif
