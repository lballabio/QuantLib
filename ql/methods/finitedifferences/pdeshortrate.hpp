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

/*! \file pdeshortrate.hpp
    \brief adapter to short rate
*/

#ifndef quantlib_pdeshortrate_hpp
#define quantlib_pdeshortrate_hpp

#include <ql/methods/finitedifferences/pde.hpp>
#include <ql/models/shortrate/onefactormodel.hpp>
#include <ql/stochasticprocess.hpp>
#include <utility>

namespace QuantLib {

    class PdeShortRate : public PdeSecondOrderParabolic {
      public:
        typedef ext::shared_ptr<OneFactorModel::ShortRateDynamics>
                                                                argument_type;
        typedef TransformedGrid grid_type;
        PdeShortRate(argument_type d) : dynamics_(std::move(d)) {}
        Real diffusion(Time t, Real x) const override {
            return dynamics_->process()->diffusion(t, x);
        }
        Real drift(Time t, Real x) const override { return dynamics_->process()->drift(t, x); }
        Real discount(Time t, Real x) const override { return dynamics_->shortRate(t, x); }

      private:
        const argument_type dynamics_;
    };

}


#endif



#ifndef id_3385dac4c2249d60904a06a91df7ec70
#define id_3385dac4c2249d60904a06a91df7ec70
inline bool test_3385dac4c2249d60904a06a91df7ec70(int* i) { return i != 0; }
#endif
