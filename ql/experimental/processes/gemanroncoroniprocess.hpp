/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2011 Klaus Spanderen
 
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

/*! \file gemanroncoroniprocess.hpp
    \brief Geman-Roncoroni process
*/

#ifndef quantlib_geman_roncoroni_process_hpp
#define quantlib_geman_roncoroni_process_hpp

#include <ql/stochasticprocess.hpp>
#include <ql/math/randomnumbers/rngtraits.hpp>

namespace QuantLib {

    //! Geman-Roncoroni process class
    /*! This class describes the Geman-Roncoroni process governed by
        \f[
        \begin{array}{rcl}
            dE(t) &=& \left[ \frac{\partial}{\partial t} \mu(t)
                +\theta_1 \left(\mu(t)-E(t^-)\right)\right]dt
                +\sigma dW(t) + h(E(t^-))dJ(t) \\
            \mu(t)&=& \alpha + \beta t +\gamma \cos(\epsilon+2\pi t)
                +\delta \cos(\zeta + 4\pi t)
        \end{array}
        \f]

        \ingroup processes
    */
    class GemanRoncoroniProcess : public StochasticProcess1D {
      public:
        GemanRoncoroniProcess(Real x0, 
                              Real alpha, Real beta, 
                              Real gamma, Real delta, 
                              Real eps, Real zeta, Real d, 
                              Real k, Real tau,
                              Real sig2, Real a, Real b,
                              Real theta1, Real theta2, Real theta3,
                              Real psi);

        Real x0() const override;
        Real drift(Time t, Real x) const override;
        Real diffusion(Time t, Real x) const override;
        Real stdDeviation(Time t0, Real x0, Time dt) const override;
        Real evolve(Time t0, Real x0, Time dt, Real dw) const override;

        Real evolve(Time t0, Real x0, Time dt, Real dw, const Array& du) const;
        
    private:
        // avoid clang++ warnings
        using StochasticProcess::evolve;

        const Real x0_;
        const Real alpha_, beta_, gamma_, delta_;
        const Real eps_, zeta_, d_; 
        const Real k_, tau_;
        const Real sig2_, a_, b_;
        const Real theta1_, theta2_, theta3_;
        const Real psi_;
        mutable ext::shared_ptr<PseudoRandom::urng_type> urng_;
    };
}


#endif


#ifndef id_649cdc669edf028970ee6d075702cf9f
#define id_649cdc669edf028970ee6d075702cf9f
inline bool test_649cdc669edf028970ee6d075702cf9f(const int* i) {
    return i != nullptr;
}
#endif
