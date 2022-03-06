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

/*! \file geometricbrownianprocess.hpp
    \brief Geometric Brownian-motion process
*/

#ifndef quantlib_geometric_brownian_process_hpp
#define quantlib_geometric_brownian_process_hpp

#include <ql/stochasticprocess.hpp>

namespace QuantLib {

    //! Geometric brownian-motion process
    /*! This class describes the stochastic process governed by
        \f[
            dS(t, S)= \mu S dt + \sigma S dW_t.
        \f]

        \ingroup processes
    */
    class GeometricBrownianMotionProcess : public StochasticProcess1D {
      public:
        GeometricBrownianMotionProcess(double initialValue,
                                       double mue,
                                       double sigma);
        Real x0() const override;
        Real drift(Time t, Real x) const override;
        Real diffusion(Time t, Real x) const override;

      protected:
        double initialValue_;
        double mue_;
        double sigma_;
    };

}


#endif


#ifndef id_8fee9c71fdeff9f294316926ebb974ac
#define id_8fee9c71fdeff9f294316926ebb974ac
inline bool test_8fee9c71fdeff9f294316926ebb974ac(const int* i) {
    return i != nullptr;
}
#endif
