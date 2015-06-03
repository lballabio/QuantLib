/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2013, 2015 Peter Caspers

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

/*! \file gsrprocess.hpp
    \brief GSR model process with piecewise volatilities and mean reversions,
           the dynamic is expressed in some T-forward measure.
           You may provide a single value for the mean reversion, then
           it is assumed to be constant. For many grid points (like 20 and above)
           evaluation may get slow. A caching is therefore provided.
           By that the results become inconsistent as soon as the parameters
           change. In that case flushCache() must be called. To ensure correct
           calibration this is done in the generateArguments() of the GSR model
*/

#ifndef quantlib_gsr_process_hpp
#define quantlib_gsr_process_hpp

#include <ql/processes/forwardmeasureprocess.hpp>
#include <ql/math/comparison.hpp>
#include <map>

namespace QuantLib {

    //! GSR stochastic process
    /*! \ingroup processes */
    class GsrProcess : public ForwardMeasureProcess1D {
      public:
        GsrProcess(const Array &times, const Array &vols,
                   const Array &reversions, const Real T = 60.0);
        //! \name StochasticProcess1D interface
        //@{
        Real x0() const;
        Real drift(Time t, Real x) const;
        Real diffusion(Time t, Real) const;
        Real expectation(Time t0, Real x0, Time dt) const;
        Real stdDeviation(Time t0, Real x0, Time dt) const;
        Real variance(Time t0, Real, Time dt) const;

        Real sigma(Time t) const;
        Real reversion(Time t) const;
        Real y(Time t) const;
        Real G(Time t, Time T, Real x) const;
        //@}
        void setForwardMeasureTime(Time t) {
            flushCache();
            ForwardMeasureProcess1D::setForwardMeasureTime(t);
        }
        void flushCache() const;

      protected:
        const Array &times_;
        const Array &vols_;
        const Array &reversions_;

      private:
        Real expectationp1(Time t0, Real x0,
                           Time dt) const; // expectation can be split into a x0
                                           // dependent term (p1) and an
                                           // independent term (p2)
        Real expectationp2(Time t0, Time dt) const;
        const int lowerIndex(Time t) const;
        const int upperIndex(Time t) const;
        const Real time2(Size index) const;
        const Real cappedTime(Size index, Real cap = Null<Real>()) const;
        const Real flooredTime(Size index, Real floor = Null<Real>()) const;
        const Real vol(Size index) const;
        const Real rev(Size index) const;
        const bool revZero(Size index) const;
        mutable std::map<std::pair<Real, Real>, Real> cache1_, cache2_, cache3_,
            cache5_;
        mutable std::map<Real, Real> cache4_;
        mutable std::vector<bool> revZero_;
    };
}

#endif
