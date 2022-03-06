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

/*! \file analytichestonforwardeuropeanengine.hpp
    \brief analytic heston engine for forward-starting european options
*/

#ifndef quantlib_analytic_heston_forward_european_engine_hpp
#define quantlib_analytic_heston_forward_european_engine_hpp

#include <ql/instruments/forwardvanillaoption.hpp>
#include <ql/pricingengines/vanilla/analytichestonengine.hpp>
#include <ql/processes/hestonprocess.hpp>
#include <ql/models/equity/hestonmodel.hpp>
#include <ql/math/modifiedbessel.hpp>
#include <ql/math/integrals/gaussianquadratures.hpp>
#include <ql/exercise.hpp>
#include <ql/quotes/simplequote.hpp>

namespace QuantLib {

    //! Analytic Heston engine incl. stochastic interest rates
    /*! This class is pricing a european option under the following process

        \f[
        \begin{array}{rcl}
        dS(t, S)  &=& (r-d) S dt +\sqrt{v} S dW_1 \\
        dv(t, S)  &=& \kappa (\theta - v) dt + \sigma \sqrt{v} dW_2 \\
        dW_1 dW_2 &=& \rho dt \\
        \end{array}
        \f]

        References:

        Implements the analytical solution for forward-starting 
        strike-reset options descriped in "On the Pricing of Forward 
        Starting Options under Stochastic Volatility", S. Kruse (2003)

        \ingroup forwardengines

        \test
        - For tReset > 0, price from the analytic pricer is compared
          to the MC priver for calls/puts at various moneynesses
        - For tReset ~ 0, price from the analytic pricer is compared
          to the Heston analytic vanilla pricer for various options
    */

    class AnalyticHestonForwardEuropeanEngine : public GenericEngine<ForwardOptionArguments<VanillaOption::arguments>,
                                                       VanillaOption::results> {
      public:
        explicit AnalyticHestonForwardEuropeanEngine(ext::shared_ptr<HestonProcess> process,
                                                     Size integrationOrder = 144);

        void calculate() const override;

        // The evolution probability function from t0 to tReset
        Real propagator(Time resetTime,
                        Real varReset) const;

        // Forward characteristic function for given (t, vt, St)
        ext::shared_ptr<AnalyticHestonEngine> forwardChF(
                                            Handle<Quote>& spotReset,
                                            Real varReset) const;

      private:
        // Parameters for the internal chF generators
        ext::shared_ptr<HestonProcess> process_;
        Size integrationOrder_;

        // Initial process params
        Real v0_, rho_, kappa_, theta_, sigma_;
        Handle<YieldTermStructure> dividendYield_;
        Handle<YieldTermStructure> riskFreeRate_;
        Handle<Quote> s0_;

        // Some intermediate calculation constant parameters
        Real kappaHat_, thetaHat_, R_;

        // The integrator for nu. This pricer needs to perform nested integrations, the outer
        // integrator should be non-adaptive as this can cause very long runtimes. Gaussian
        // Quadrature has been found to work well.
        GaussLegendreIntegration outerIntegrator_;

        // Integrate over characteristic function to generate P1, P2 (fall back on this
        // when reset time is very close, to avoid numerical issues with tReset=0s)
        std::pair<Real, Real> calculateP1P2(Time t,
                                            Handle<Quote>& St,
                                            Real K,
                                            Real ratio,
                                            Real phiRightLimit = 100) const;

        // Integrate P1, P2 over te propagator function to calculate forward-start price
        std::pair<Real, Real> calculateP1P2Hat(Time tenor,
                                               Time resetTime,
                                               Real K,
                                               Real ratio,
                                               Real phiRightLimit = 100,
                                               Real nuRightLimit = 2.0) const;

    };
}

#endif


#ifndef id_598aff30cdab2abf7bae4f8f3dc90524
#define id_598aff30cdab2abf7bae4f8f3dc90524
inline bool test_598aff30cdab2abf7bae4f8f3dc90524(const int* i) {
    return i != nullptr;
}
#endif
