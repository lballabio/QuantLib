/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2003 Ferdinando Ametrano
 Copyright (C) 2001, 2002, 2003 Sadruddin Rejeb
 Copyright (C) 2004, 2005 StatPro Italia srl

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

/*! \file squarerootprocess.hpp
    \brief square-root process
*/

#ifndef quantlib_square_root_process_hpp
#define quantlib_square_root_process_hpp

#include <ql/stochasticprocess.hpp>
#include <ql/processes/eulerdiscretization.hpp>

namespace QuantLib {

    //! Square-root process class
    /*! This class describes a square-root process governed by
        \f[
            dx = a (b - x_t) dt + \sigma \sqrt{x_t} dW_t.
        \f]

        \ingroup processes
    */
    class SquareRootProcess : public StochasticProcess1D {
      public:
        SquareRootProcess(
            Real b, Real a, Volatility sigma, Real x0 = 0.0,
            const ext::shared_ptr<discretization>& d =
                  ext::shared_ptr<discretization>(new EulerDiscretization));
        //! \name StochasticProcess interface
        //@{
        Real x0() const override;
        Real drift(Time t, Real x) const override;
        Real diffusion(Time t, Real x) const override;
        //@}

        Real a() const { return speed_;  }
        Real b() const { return mean_; }
        Real sigma() const { return volatility_; }
      private:
        Real x0_, mean_, speed_;
        Volatility volatility_;
    };

}


#endif


#ifndef id_0c0b450ff1265079906b39669d0fdc19
#define id_0c0b450ff1265079906b39669d0fdc19
inline bool test_0c0b450ff1265079906b39669d0fdc19(int* i) { return i != 0; }
#endif
