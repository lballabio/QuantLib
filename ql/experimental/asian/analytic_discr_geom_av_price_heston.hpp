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

#ifndef quantlib_analytic_discrete_geometric_average_price_asian_heston_engine_hpp
#define quantlib_analytic_discrete_geometric_average_price_asian_heston_engine_hpp

#include <ql/instruments/asianoption.hpp>
#include <ql/processes/hestonprocess.hpp>
#include <ql/math/integrals/gaussianquadratures.hpp>
#include <ql/exercise.hpp>

namespace QuantLib {

    //! Pricing engine for European discrete geometric average price Asian
    /*! This class implements a discrete geometric average price
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
        options developed in "A Recursive Method for Discretely Monitored
        Geometric Asian Option Prices", B. Kim, J. Kim, J. Kim & I. S. Wee,
        Bull. Korean Math. Soc. 53, 733-749 (2016)

        \ingroup asianengines

        \test
        - the correctness of the returned value is tested by reproducing
              results in Tables 1, 2 and 3 of the paper

        \todo handle seasoned options
    */
    class AnalyticDiscreteGeometricAveragePriceAsianHestonEngine
        : public DiscreteAveragingAsianOption::engine {
      public:
        explicit AnalyticDiscreteGeometricAveragePriceAsianHestonEngine(
            ext::shared_ptr<HestonProcess> process, Real xiRightLimit = 100.0);
        void calculate() const override;

        // Equation (21) - must be public so the integrand can access it.
        std::complex<Real> Phi(std::complex<Real> s,
                               std::complex<Real> w,
                               Time t,
                               Time T,
                               Size kStar,
                               const std::vector<Time>& t_n,
                               const std::vector<Time>& tauK) const;

      private:
        // Initial process params
        Real v0_, rho_, kappa_, theta_, sigma_, logS0_;
        Handle<YieldTermStructure> dividendYield_;
        Handle<YieldTermStructure> riskFreeRate_;
        Handle<Quote> s0_;

        ext::shared_ptr<HestonProcess> process_;

        // A lookup table for the reuslts of omega_tilde() to avoid repeated calls for given Phi call
        mutable std::map<Size, std::complex<Real> > omegaTildeLookupTable_;

        // Cutoff parameter for integral in Eqs (23) and (24)
        Real xiRightLimit_;

        // Integrator for equation (23) and (24)
        GaussLegendreIntegration integrator_;

        // Integrand
        class Integrand;

        // We need to set up several variables inside calculate as they depend on fixing times. Rather
        // than pass them between a, omega, F etc. which makes for very messy method signatures, we
        // make them mutable class properties instead.
        mutable Real tr_t_;
        mutable Real Tr_T_;
        mutable std::vector<Real> tkr_tk_;

        // Equation (11)
        std::complex<Real> F(const std::complex<Real>& z1,
                             const std::complex<Real>& z2,
                             Time tau) const;

        std::complex<Real> F_tilde(const std::complex<Real>& z1,
                                   const std::complex<Real>& z2,
                                   Time tau) const;

        // Equation (14)
        std::complex<Real> z(const std::complex<Real>& s,
                             const std::complex<Real>& w,
                             Size k, Size n) const;

        // Equation (15)
        std::complex<Real> omega(const std::complex<Real>& s,
                                 const std::complex<Real>& w,
                                 Size k, Size kStar, Size n) const;

        // Equation (16)
        std::complex<Real> a(const std::complex<Real>& s,
                             const std::complex<Real>& w,
                             Time t, Time T, Size kStar,
                             const std::vector<Time>& t_n) const;

        // Equation (19)
        std::complex<Real> omega_tilde(const std::complex<Real>& s,
                                       const std::complex<Real>& w,
                                       Size k, Size kStar, Size n,
                                       const std::vector<Time>& tauK) const;
    };
}


#endif


#ifndef id_8fbf8c1598af693ab189735c36a0ba6b
#define id_8fbf8c1598af693ab189735c36a0ba6b
inline bool test_8fbf8c1598af693ab189735c36a0ba6b(int* i) { return i != 0; }
#endif
