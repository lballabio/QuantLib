/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Klaus Spanderen

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

/*! \file batesprocess.hpp
    \brief Bates stochastic process, Heston process plus compound Poisson
    process plus log-normal jump diffusion size
*/

#ifndef quantlib_bates_process_hpp
#define quantlib_bates_process_hpp

#include <ql/processes/hestonprocess.hpp>
#include <ql/math/distributions/normaldistribution.hpp>


namespace QuantLib {

    //! Square-root stochastic-volatility Bates process
    /*! This class describes the square root stochastic volatility
        process incl jumps governed by
        \f[
        \begin{array}{rcl}
        dS(t, S)  &=& (r-d-\lambda m) S dt +\sqrt{v} S dW_1 + (e^J - 1) S dN \\
        dv(t, S)  &=& \kappa (\theta - v) dt + \sigma \sqrt{v} dW_2 \\
        dW_1 dW_2 &=& \rho dt \\
        \omega(J) &=& \frac{1}{\sqrt{2\pi \delta^2}}
                      \exp\left[-\frac{(J-\nu)^2}{2\delta^2}\right]
        \end{array}
        \f]

        \ingroup processes
    */
    class BatesProcess : public HestonProcess {
    public:
        BatesProcess(const Handle<YieldTermStructure>& riskFreeRate,
                     const Handle<YieldTermStructure>& dividendYield,
                     const Handle<Quote>& s0,
                     Real v0, Real kappa,
                     Real theta, Real sigma, Real rho,
                     Real lambda, Real nu, Real delta,
                     HestonProcess::Discretization d
                         = HestonProcess::FullTruncation);

        Size factors() const override;
        Disposable<Array> drift(Time t, const Array& x) const override;
        Disposable<Array> evolve(Time t0, const Array& x0, Time dt, const Array& dw) const override;

        Real lambda() const;
        Real nu()     const;
        Real delta()  const;
      private:
        const Real lambda_, delta_, nu_, m_;
        const CumulativeNormalDistribution cumNormalDist_;
    };
}


#endif


#ifndef id_9c15f47b4a459efa45822a7579ad0850
#define id_9c15f47b4a459efa45822a7579ad0850
inline bool test_9c15f47b4a459efa45822a7579ad0850(const int* i) {
    return i != nullptr;
}
#endif
