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

/*! \file extouwithjumpsprocess.hpp
    \brief Ornstein Uhlenbeck process plus exp jumps (Kluge Model)
*/

#ifndef quantlib_ext_ou_with_jumps_process_hpp
#define quantlib_ext_ou_with_jumps_process_hpp

#include <ql/stochasticprocess.hpp>
#include <ql/math/distributions/normaldistribution.hpp>

namespace QuantLib {

    class ExtendedOrnsteinUhlenbeckProcess;

    /*! This class describes a Ornstein Uhlenbeck model plus exp jump, 
        an extension of the Lucia and Schwartz model
        \f[
        \begin{array}{rcl}
        S &=& exp(X_t + Y_t) \\
        dX_t  &=& \alpha(\mu(t)-X_t)dt + \sigma dW_t \\
        dY_t  &=& -\beta Y_{t-}dt + J_tdN_t \\
        \omega(J)&=& \eta_u e^{-\eta_u J}
        \end{array}
        \f]
        
        \ingroup processes
    */


    /*! References:
        T. Kluge, 2008. Pricing Swing Options and other 
        Electricity Derivatives, http://eprints.maths.ox.ac.uk/246/1/kluge.pdf
        
        B. Hambly, S. Howison, T. Kluge, Modelling spikes and pricing 
        swing options in electricity markets,
        http://people.maths.ox.ac.uk/hambly/PDF/Papers/elec.pdf
    */
        

    class ExtOUWithJumpsProcess : public StochasticProcess {
      public:
        ExtOUWithJumpsProcess(std::shared_ptr<ExtendedOrnsteinUhlenbeckProcess> process,
                              Real Y0,
                              Real beta,
                              Real jumpIntensity,
                              Real eta);

        Size size() const override;
        Size factors() const override;

        Array initialValues() const override;
        Array drift(Time t, const Array& x) const override;
        Matrix diffusion(Time t, const Array& x) const override;
        Array evolve(Time t0, const Array& x0, Time dt, const Array& dw) const override;

        std::shared_ptr<ExtendedOrnsteinUhlenbeckProcess> getExtendedOrnsteinUhlenbeckProcess() const;

        Real beta()          const;
        Real eta()           const;
        Real jumpIntensity() const;

      private:
        const Real Y0_, beta_, jumpIntensity_, eta_;
        const std::shared_ptr<ExtendedOrnsteinUhlenbeckProcess> ouProcess_;
        
        const CumulativeNormalDistribution cumNormalDist_;
    };
}
#endif
