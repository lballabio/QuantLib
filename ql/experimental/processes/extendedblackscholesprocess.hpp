/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Frank Hövermann

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

/*! \file extendedblackscholesprocess.hpp
    \brief experimental Black-Scholes-Merton process
*/

#ifndef quantlib_extended_black_scholes_process_hpp
#define quantlib_extended_black_scholes_process_hpp

#include <ql/processes/blackscholesprocess.hpp>

namespace QuantLib {

    //! experimental Black-Scholes-Merton stochastic process
    /*! This class allows to choose a built-in discretization scheme

        \ingroup processes
    */
    class ExtendedBlackScholesMertonProcess
        : public GeneralizedBlackScholesProcess {
      public:
        enum Discretization { Euler, Milstein, PredictorCorrector };
        ExtendedBlackScholesMertonProcess(
            const Handle<Quote>& x0,
            const Handle<YieldTermStructure>& dividendTS,
            const Handle<YieldTermStructure>& riskFreeTS,
            const Handle<BlackVolTermStructure>& blackVolTS,
            const std::shared_ptr<discretization>& d =
                  std::shared_ptr<discretization>(new EulerDiscretization),
            Discretization evolDisc = Milstein);
        Real drift(Time t, Real x) const override;
        Real diffusion(Time t, Real x) const override;
        Real evolve(Time t0, Real x0, Time dt, Real dw) const override;

      private:
        const Discretization discretization_;
    };

}


#endif
