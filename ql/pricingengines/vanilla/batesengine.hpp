/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2005 Klaus Spanderen
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

/*! \file batesengine.hpp
    \brief analytic Bates model engine
*/

#ifndef quantlib_bates_engine_hpp
#define quantlib_bates_engine_hpp

#include <ql/qldefines.hpp>
#include <ql/models/equity/batesmodel.hpp>
#include <ql/pricingengines/vanilla/analytichestonengine.hpp>

namespace QuantLib {

    //! Bates model engines based on Fourier transform
    /*! this classes price european options under the following processes

        1. Jump-Diffusion with Stochastic Volatility

        \f[
        \begin{array}{rcl}
        dS(t, S)  &=& (r-d-\lambda m) S dt +\sqrt{v} S dW_1 + (e^J - 1) S dN \\
        dv(t, S)  &=& \kappa (\theta - v) dt + \sigma \sqrt{v} dW_2 \\
        dW_1 dW_2 &=& \rho dt
        \end{array}
        \f]

        N is a Poisson process with the intensity \f$ \lambda
        \f$. When a jump occurs the magnitude J has the probability
        density function \f$ \omega(J) \f$.

        1.1 Log-Normal Jump Diffusion: BatesEngine

        Logarithm of the jump size J is normally distributed
        \f[
        \omega(J) = \frac{1}{\sqrt{2\pi \delta^2}}
                    \exp\left[-\frac{(J-\nu)^2}{2\delta^2}\right]
        \f]

        1.2  Double-Exponential Jump Diffusion: BatesDoubleExpEngine

        The jump size has an asymmetric double exponential distribution
        \f[
        \begin{array}{rcl}
        \omega(J)&=&  p\frac{1}{\eta_u}e^{-\frac{1}{\eta_u}J} 1_{J>0}
                    + q\frac{1}{\eta_d}e^{\frac{1}{\eta_d}J} 1_{J<0} \\
        p + q &=& 1
        \end{array}
        \f]

        2. Stochastic Volatility with Jump Diffusion
           and Deterministic Jump Intensity

        \f[
        \begin{array}{rcl}
        dS(t, S)  &=& (r-d-\lambda m) S dt +\sqrt{v} S dW_1 + (e^J - 1) S dN \\
        dv(t, S)  &=& \kappa (\theta - v) dt + \sigma \sqrt{v} dW_2 \\
        d\lambda(t) &=& \kappa_\lambda(\theta_\lambda-\lambda) dt \\
        dW_1 dW_2 &=& \rho dt
        \end{array}
        \f]

        2.1 Log-Normal Jump Diffusion with Deterministic Jump Intensity
              BatesDetJumpEngine

        2.2 Double-Exponential Jump Diffusion with Deterministic Jump Intensity
              BatesDoubleExpDetJumpEngine


        References:

        D. Bates, Jumps and stochastic volatility: exchange rate processes
        implicit in Deutsche mark options,
        Review of Financial Sudies 9, 69-107.

        A. Sepp, Pricing European-Style Options under Jump Diffusion
        Processes with Stochastic Volatility: Applications of Fourier
        Transform (<http://math.ut.ee/~spartak/papers/stochjumpvols.pdf>)

        \ingroup vanillaengines

        \test the correctness of the returned value is tested by
              reproducing results available in web/literature, testing
              against QuantLib's jump diffusion engine
              and comparison with Black pricing.
    */
    class BatesEngine : public AnalyticHestonEngine {
      public:
        explicit BatesEngine(const ext::shared_ptr<BatesModel>& model,
                             Size integrationOrder = 144);
        BatesEngine(const ext::shared_ptr<BatesModel>& model,
                    Real relTolerance, Size maxEvaluations);

      protected:
        std::complex<Real> addOnTerm(Real phi, Time t, Size j) const override;
    };


    class BatesDetJumpEngine : public BatesEngine {
      public:
        explicit BatesDetJumpEngine(const ext::shared_ptr<BatesDetJumpModel>& model,
                                    Size integrationOrder = 144);
        BatesDetJumpEngine(const ext::shared_ptr<BatesDetJumpModel>& model,
                           Real relTolerance, Size maxEvaluations);

      protected:
        std::complex<Real> addOnTerm(Real phi, Time t, Size j) const override;
    };


    class BatesDoubleExpEngine : public AnalyticHestonEngine {
      public:
        explicit BatesDoubleExpEngine(
            const ext::shared_ptr<BatesDoubleExpModel>& model,
            Size integrationOrder = 144);
        BatesDoubleExpEngine(
            const ext::shared_ptr<BatesDoubleExpModel>& model,
            Real relTolerance, Size maxEvaluations);

      protected:
        std::complex<Real> addOnTerm(Real phi, Time t, Size j) const override;
    };


    class BatesDoubleExpDetJumpEngine : public BatesDoubleExpEngine {
      public:
        explicit BatesDoubleExpDetJumpEngine(
            const ext::shared_ptr<BatesDoubleExpDetJumpModel>& model,
            Size integrationOrder = 144);
        BatesDoubleExpDetJumpEngine(
            const ext::shared_ptr<BatesDoubleExpDetJumpModel>& model,
            Real relTolerance, Size maxEvaluations);

      protected:
        std::complex<Real> addOnTerm(Real phi, Time t, Size j) const override;
    };

}

#endif


#ifndef id_76f3fcef77e5619cc8b12294ed132aaf
#define id_76f3fcef77e5619cc8b12294ed132aaf
inline bool test_76f3fcef77e5619cc8b12294ed132aaf(const int* i) {
    return i != nullptr;
}
#endif
