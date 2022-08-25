/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2013 Peter Caspers

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

/*! \file mfstateprocess.hpp
    \brief State process for markov functional model
*/

#ifndef quantlib_mfstateprocess_hpp
#define quantlib_mfstateprocess_hpp

#include <ql/stochasticprocess.hpp>

namespace QuantLib {

    //! Markov functional state process class
    /*! This class describes the process governed by
        \f[ dx = \sigma(t) e^{at} dW(t) \f]
        \ingroup processes
    */
    class MfStateProcess : public StochasticProcess1D {
      public:
        MfStateProcess(Real reversion, const Array &times, const Array &vols);

        //! \name StochasticProcess interface
        //@{
        Real x0() const override;
        Real drift(Time t, Real x) const override;
        Real diffusion(Time t, Real x) const override;
        Real expectation(Time t0, Real x0, Time dt) const override;
        Real stdDeviation(Time t0, Real x0, Time dt) const override;
        Real variance(Time t0, Real x0, Time dt) const override;
        //@}
      private:
        Real reversion_;
        bool reversionZero_ = false;
        const Array &times_;
        const Array &vols_;
    };
}

#endif
