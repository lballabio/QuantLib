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
    /*! \ingroup processes */
    class G2Process : public StochasticProcess {
      public:
        G2Process(const Handle<YieldTermStructure>& h, Real a, Real sigma, Real b, Real eta, Real rho);
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
      private:
        Real x0_ = 0.0, y0_ = 0.0, a_, sigma_, b_, eta_, rho_;
        ext::shared_ptr<QuantLib::OrnsteinUhlenbeckProcess> xProcess_;
        ext::shared_ptr<QuantLib::OrnsteinUhlenbeckProcess> yProcess_;
        Handle<YieldTermStructure> h_;
    };

    //! %Forward %G2 stochastic process
    /*! \ingroup processes */
    class G2ForwardProcess : public ForwardMeasureProcess {
      public:
        G2ForwardProcess(const Handle<YieldTermStructure>& h, Real a, Real sigma, Real b, Real eta, Real rho);
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
      protected:
        Real x0_ = 0.0, y0_ = 0.0, a_, sigma_, b_, eta_, rho_;
        ext::shared_ptr<QuantLib::OrnsteinUhlenbeckProcess> xProcess_;
        ext::shared_ptr<QuantLib::OrnsteinUhlenbeckProcess> yProcess_;
        Real xForwardDrift(Time t, Time T) const;
        Real yForwardDrift(Time t, Time T) const;
        Real Mx_T(Real s, Real t, Real T) const;
        Real My_T(Real s, Real t, Real T) const;
        Handle<YieldTermStructure> h_;
    };

}


#endif

