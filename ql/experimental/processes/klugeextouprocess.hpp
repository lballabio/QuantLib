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

/*! \file klugeextouprocess.hpp
    \brief joint Kluge process an d Ornstein Uhlenbeck process
*/

#ifndef quantlib_kluge_ext_ou_process_hpp
#define quantlib_kluge_ext_ou_process_hpp

#include <ql/stochasticprocess.hpp>

namespace QuantLib {

    class ExtOUWithJumpsProcess;
    class ExtendedOrnsteinUhlenbeckProcess;

    /*! This class describes a correlated Kluge - extended Ornstein-Uhlenbeck
        process governed by
        \f[
        \begin{array}{rcl}
            P_t &=& \exp(p_t + X_t + Y_t) \\
            dX_t &=& -\alpha X_tdt + \sigma_x dW_t^x \\
            dY_t &=& -\beta Y_{t-}dt + J_tdN_t \\
            \omega(J) &=& \eta e^{-\eta J} \\
            G_t &=& \exp(g_t + U_t) \\
            dU_t &=& -\kappa U_tdt + \sigma_udW_t^u \\
            \rho &=& \mathrm{corr} (dW_t^x, dW_t^u)
         \end{array}
         \f]
    */

    /*! References:
        B. Hambly, S. Howison, T. Kluge, Modelling spikes and pricing
        swing options in electricity markets,
        http://people.maths.ox.ac.uk/hambly/PDF/Papers/elec.pdf
    */


    class KlugeExtOUProcess : public StochasticProcess {
      public:
        KlugeExtOUProcess(
            Real rho,
            const boost::shared_ptr<ExtOUWithJumpsProcess>& kluge,
            const boost::shared_ptr<ExtendedOrnsteinUhlenbeckProcess>& extOU);

        Size size() const;
        Size factors() const;

        Disposable<Array> initialValues() const;
        Disposable<Array> drift(Time t, const Array& x) const;
        Disposable<Matrix> diffusion(Time t, const Array& x) const;
        Disposable<Array> evolve(Time t0, const Array& x0,
                                 Time dt, const Array& dw) const;

        boost::shared_ptr<ExtOUWithJumpsProcess> getKlugeProcess() const;
        boost::shared_ptr<ExtendedOrnsteinUhlenbeckProcess>
                                                 getExtOUProcess() const;

        Real rho() const;

      private:
        const Real rho_, sqrtMRho_;
        const boost::shared_ptr<ExtOUWithJumpsProcess> klugeProcess_;
        const boost::shared_ptr<ExtendedOrnsteinUhlenbeckProcess> ouProcess_;
    };
}
#endif
