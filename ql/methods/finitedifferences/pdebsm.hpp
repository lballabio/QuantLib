/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2005 Joseph Wang

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

/*! \file pdebsm.hpp
    \brief Black-Scholes-Merton PDE
*/

#ifndef quantlib_pdebsm_hpp
#define quantlib_pdebsm_hpp

#include <ql/methods/finitedifferences/pde.hpp>
#include <ql/processes/blackscholesprocess.hpp>
#include <utility>

namespace QuantLib {

    class PdeBSM : public PdeSecondOrderParabolic {
      public:
        typedef ext::shared_ptr<GeneralizedBlackScholesProcess>
                                                                argument_type;
        typedef LogGrid grid_type;
        PdeBSM(argument_type process) : process_(std::move(process)){};
        Real diffusion(Time t, Real x) const override { return process_->diffusion(t, x); }
        Real drift(Time t, Real x) const override { return process_->drift(t, x); }
        Real discount(Time t, Real) const override {
            if (std::fabs(t) < 1e-8) t = 0;
            return process_->riskFreeRate()->
                forwardRate(t,t,Continuous,NoFrequency,true);
        }

    private:
        const argument_type process_;
    };

}


#endif


#ifndef id_d1b580ca1c418a1985ae29a62ac065f6
#define id_d1b580ca1c418a1985ae29a62ac065f6
inline bool test_d1b580ca1c418a1985ae29a62ac065f6(const int* i) {
    return i != nullptr;
}
#endif
