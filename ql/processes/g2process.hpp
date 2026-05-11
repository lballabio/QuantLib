/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Banca Profilo S.p.A.

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

/*! \file g2process.hpp
    \brief G2 stochastic processes
*/

#ifndef quantlib_g2_process_hpp
#define quantlib_g2_process_hpp

#include <ql/processes/forwardmeasureprocess.hpp>
#include <ql/processes/ornsteinuhlenbeckprocess.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>

namespace QuantLib {

    //! %G2 stochastic process
    /*! Simulates the two-factor G2++ process with state shifted so that
        the two simulated components sum to the short rate, i.e. the state
        is \f$ (z_1, z_2) = (x + \varphi(t),\, y) \f$, where \f$ x \f$ and
        \f$ y \f$ are the underlying zero-mean OU factors and
        \f$ \varphi(t) \f$ is the deterministic offset that fits the
        initial term structure. As a consequence, sample paths produced by
        a path generator built on this process satisfy
        \f$ r(t_i) = \mathrm{state}[0]_i + \mathrm{state}[1]_i \f$ and have
        curve-consistent expectation \f$ \varphi(t_i) \f$.

        If an empty term-structure handle is passed, the process degenerates
        to a pair of zero-mean OU processes (\f$ \varphi \equiv 0 \f$).

        \ingroup processes
    */
    class G2Process : public StochasticProcess {
      public:
        G2Process(Real a, Real sigma, Real b, Real eta, Real rho,
                  const Handle<YieldTermStructure>& termStructure = {});
        //! \name StochasticProcess interface
        //@{
        Size size() const override;
        Array initialValues() const override;
        Array drift(Time t, const Array& x) const override;
        Matrix diffusion(Time t, const Array& x) const override;
        Array expectation(Time t0, const Array& x0, Time dt) const override;
        Matrix stdDeviation(Time t0, const Array& x0, Time dt) const override;
        Matrix covariance(Time t0, const Array& x0, Time dt) const override;
        //@}
        Real x0() const;
        Real y0() const;
        Real a() const;
        Real sigma() const;
        Real b() const;
        Real eta() const;
        Real rho() const;
        const Handle<YieldTermStructure>& termStructure() const;
        Real phi(Time t) const;
        Rate shortRate(Time t, Real x, Real y) const;
      private:
        Real x0_ = 0.0, y0_ = 0.0, a_, sigma_, b_, eta_, rho_;
        ext::shared_ptr<QuantLib::OrnsteinUhlenbeckProcess> xProcess_;
        ext::shared_ptr<QuantLib::OrnsteinUhlenbeckProcess> yProcess_;
        Handle<YieldTermStructure> termStructure_;
    };

    //! %Forward %G2 stochastic process
    /*! Forward-measure counterpart of G2Process. The simulated state is
        again shifted so that \f$ \mathrm{state}[0] + \mathrm{state}[1] = r(t) \f$,
        on top of the usual T-forward convexity adjustments to the drift
        and the conditional expectation.

        \ingroup processes
    */
    class G2ForwardProcess : public ForwardMeasureProcess {
      public:
        G2ForwardProcess(Real a, Real sigma, Real b, Real eta, Real rho,
                         const Handle<YieldTermStructure>& termStructure = {});
        //! \name StochasticProcess interface
        //@{
        Size size() const override;
        Array initialValues() const override;
        Array drift(Time t, const Array& x) const override;
        Matrix diffusion(Time t, const Array& x) const override;
        Array expectation(Time t0, const Array& x0, Time dt) const override;
        Matrix stdDeviation(Time t0, const Array& x0, Time dt) const override;
        Matrix covariance(Time t0, const Array& x0, Time dt) const override;
        //@}
        const Handle<YieldTermStructure>& termStructure() const;
        Real phi(Time t) const;
        Rate shortRate(Time t, Real x, Real y) const;
      protected:
        Real x0_ = 0.0, y0_ = 0.0, a_, sigma_, b_, eta_, rho_;
        ext::shared_ptr<QuantLib::OrnsteinUhlenbeckProcess> xProcess_;
        ext::shared_ptr<QuantLib::OrnsteinUhlenbeckProcess> yProcess_;
        Handle<YieldTermStructure> termStructure_;
        Real xForwardDrift(Time t, Time T) const;
        Real yForwardDrift(Time t, Time T) const;
        Real Mx_T(Real s, Real t, Real T) const;
        Real My_T(Real s, Real t, Real T) const;
    };

}


#endif

