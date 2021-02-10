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
           If a single value for the mean reversion is provided, it is assumed
           constant. Results are cached for performance reasons, so if parameters
           change you need to call flushCache() to avoid inconsistent results.
           For a derivation of the formulas, see http://ssrn.com/abstract=2246013
*/

#ifndef quantlib_gsr_process_hpp
#define quantlib_gsr_process_hpp

#include <ql/processes/forwardmeasureprocess.hpp>
#include <ql/processes/gsrprocesscore.hpp>
#include <ql/time/daycounter.hpp>

namespace QuantLib {

    //! GSR stochastic process
    /*! \ingroup processes */
    class GsrProcess : public ForwardMeasureProcess1D {
      public:
        GsrProcess(const Array& times,
                   const Array& vols,
                   const Array& reversions,
                   Real T = 60.0,
                   const Date& referenceDate = Null<Date>(),
                   DayCounter dc = DayCounter());
        //! \name StochasticProcess1D interface
        //@{
        Real x0() const override;
        Real drift(Time t, Real x) const override;
        Real diffusion(Time t, Real) const override;
        Real expectation(Time t0, Real x0, Time dt) const override;
        Real stdDeviation(Time t0, Real x0, Time dt) const override;
        Real variance(Time t0, Real, Time dt) const override;
        Real time(const Date& d) const override;
        //@}
        //! \name ForwardMeasureProcess1D interface
        void setForwardMeasureTime(Time t) override;
        //@}
        //! additional inspectors
        Real sigma(Time t) const;
        Real reversion(Time t) const;
        Real y(Time t) const;
        Real G(Time t, Time T, Real x) const;
        //! reset cache
        void flushCache() const;

      private:
        void checkT(Time t) const;
        const detail::GsrProcessCore core_;
        Date referenceDate_;
        DayCounter dc_;
    };

    // inline definitions

    inline void GsrProcess::setForwardMeasureTime(Time t) {
        flushCache();
        ForwardMeasureProcess1D::setForwardMeasureTime(t);
    }

    inline void GsrProcess::flushCache() const {
        core_.flushCache();
    }

} // namesapce QuantLib

#endif
