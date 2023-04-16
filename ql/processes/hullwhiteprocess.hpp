/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Banca Profilo S.p.A.

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

/*! \file hullwhiteprocess.hpp
    \brief Hull-White stochastic processes
*/

#ifndef quantlib_hull_white_processes_hpp
#define quantlib_hull_white_processes_hpp

#include <ql/processes/forwardmeasureprocess.hpp>
#include <ql/processes/ornsteinuhlenbeckprocess.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>

namespace QuantLib {

    //! Hull-White stochastic process
    /*! \ingroup processes */
    class HullWhiteProcess: public StochasticProcess1D {
      public:
        HullWhiteProcess(const Handle<YieldTermStructure>& h,
                         Real a,
                         Real sigma);
        //! \name StochasticProcess1D interface
        //@{
        Real x0() const override;
        Real drift(Time t, Real x) const override;
        Real diffusion(Time t, Real x) const override;
        Real expectation(Time t0, Real x0, Time dt) const override;
        Real stdDeviation(Time t0, Real x0, Time dt) const override;
        Real variance(Time t0, Real x0, Time dt) const override;

        Real a() const;
        Real sigma() const;
        Real alpha(Time t) const;
        //@}
    protected:
        std::shared_ptr<QuantLib::OrnsteinUhlenbeckProcess> process_;
        Handle<YieldTermStructure> h_;
        Real a_, sigma_;
    };

    //! %Forward Hull-White stochastic process
    /*! \ingroup processes */
    class HullWhiteForwardProcess: public ForwardMeasureProcess1D {
      public:
        HullWhiteForwardProcess(const Handle<YieldTermStructure>& h,
                                Real a,
                                Real sigma);
        //! \name StochasticProcess1D interface
        //@{
        Real x0() const override;
        Real drift(Time t, Real x) const override;
        Real diffusion(Time t, Real x) const override;
        Real expectation(Time t0, Real x0, Time dt) const override;
        Real stdDeviation(Time t0, Real x0, Time dt) const override;
        Real variance(Time t0, Real x0, Time dt) const override;
        //@}

        Real a() const;
        Real sigma() const;
        Real alpha(Time t) const;
        Real M_T(Real s, Real t, Real T) const;
        Real B(Time t, Time T) const;

    protected:
        std::shared_ptr<QuantLib::OrnsteinUhlenbeckProcess> process_;
        Handle<YieldTermStructure> h_;
        Real a_, sigma_;
    };

}


#endif
