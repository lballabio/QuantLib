/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2020 Jack Gillett

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

/*! \file analytic_cont_geom_av_price.hpp
    \brief Analytic engine for continuous geometric average price Asian
           in the Heston model
*/

#ifndef quantlib_analytic_continuous_geometric_average_price_asian_heston_engine_hpp
#define quantlib_analytic_continuous_geometric_average_price_asian_heston_engine_hpp

#include <ql/instruments/asianoption.hpp>
#include <ql/processes/hestonprocess.hpp>
#include <ql/math/integrals/gaussianquadratures.hpp>
#include <ql/math/integrals/simpsonintegral.hpp>
#include <ql/exercise.hpp>
#include <complex>

namespace QuantLib {

    //! Pricing engine for European continuous geometric average price Asian
    /*! This class implements a continuous geometric average price
        Asian option with European exercise under the Heston stochastic
        vol model where spot and variance follow the processes

        \f[
        \begin{array}{rcl}
        dS(t, S)  &=& (r-d) S dt +\sqrt{v} S dW_1 \\
        dv(t, S)  &=& \kappa (\theta - v) dt + \sigma \sqrt{v} dW_2 \\
        dW_1 dW_2 &=& \rho dt \\
        \end{array}
        \f]

        References:

        Implements the analytical solution for continuous geometric Asian
        options developed in "Pricing of geometric Asian options under
        Heston's stochastic volatility model", B. Kim & I. S. Wee, Quantative
        Finance 14:10, 1795-1809 (2014)

        \ingroup asianengines

        \test
        - the correctness of the returned value is tested by reproducing
              results in Table 1 and Table 4 of the paper

        \todo handle seasoned options
    */
    class AnalyticContinuousGeometricAveragePriceAsianHestonEngine
        : public ContinuousAveragingAsianOption::engine {
      public:
        explicit AnalyticContinuousGeometricAveragePriceAsianHestonEngine(
            ext::shared_ptr<HestonProcess> process,
            Size summationCutoff = 50,
            Real xiRightLimit = 100.0);
        void calculate() const override;

        // Phi, defined in eq (25). Must be public so the integrand can access it (Could
        // use friend functions I think, but perhaps overkill?)
        std::complex<Real> Phi(const std::complex<Real>& s,
                               const std::complex<Real>& w,
                               Real T, Real t = 0.0, Size cutoff = 50) const;

      private:
        // Initial process params
        Real v0_, rho_, kappa_, theta_, sigma_;
        Handle<YieldTermStructure> dividendYield_;
        Handle<YieldTermStructure> riskFreeRate_;
        Handle<Quote> s0_;

        ext::shared_ptr<HestonProcess> process_;

        // Some intermediate calculation constant parameters
        Real a1_, a2_;
        mutable Real a3_ = 0.0, a4_ = 0.0, a5_ = 0.0;

        // A lookup table for the reuslts of f() to avoid repeated calls
        mutable std::map<int, std::complex<Real> > fLookupTable_;

        // Cutoff parameters for summation (19), (20) and for integral (29)
        Size summationCutoff_;
        Real xiRightLimit_;

        // Integrator for equation (29)
        GaussLegendreIntegration integrator_;

        // Integrands
        class Integrand;
        class DcfIntegrand;

        // Equations (13)
        std::complex<Real> z1_f(const std::complex<Real>& s, const std::complex<Real>& w, Real T) const;
        std::complex<Real> z2_f(const std::complex<Real>& s, const std::complex<Real>& w, Real T) const;
        std::complex<Real> z3_f(const std::complex<Real>& s, const std::complex<Real>& w, Real T) const;
        std::complex<Real> z4_f(const std::complex<Real>& s, const std::complex<Real>& w) const;

        // Equations (19), (20)
        std::pair<std::complex<Real>, std::complex<Real> > F_F_tilde(
                                        const std::complex<Real>& z1,
                                        const std::complex<Real>& z2,
                                        const std::complex<Real>& z3,
                                        const std::complex<Real>& z4,
                                        Real tau,
                                        Size cutoff = 50) const;

        // Equation (21)
        std::complex<Real> f(const std::complex<Real>& z1,
                             const std::complex<Real>& z2,
                             const std::complex<Real>& z3,
                             const std::complex<Real>& z4,
                             int n,
                             Real tau) const;
    };
}


#endif
