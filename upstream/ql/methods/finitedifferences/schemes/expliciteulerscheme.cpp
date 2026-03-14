/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2009 Andreas Gaida
 Copyright (C) 2009 Ralph Schreyer
 Copyright (C) 2009 Klaus Spanderen

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

#include <ql/methods/finitedifferences/schemes/expliciteulerscheme.hpp>
#include <utility>

namespace QuantLib {
    ExplicitEulerScheme::ExplicitEulerScheme(ext::shared_ptr<FdmLinearOpComposite> map,
                                             const bc_set& bcSet)
    : dt_(Null<Real>()), map_(std::move(map)), bcSet_(bcSet) {}

    void ExplicitEulerScheme::step(array_type& a, Time t) {
        step(a, t, 1.0);
    }

    void ExplicitEulerScheme::step(array_type& a, Time t, Real theta) {
        QL_REQUIRE(t-dt_ > -1e-8, "a step towards negative time given");
        map_->setTime(std::max(0.0, t - dt_), t);
        bcSet_.setTime(std::max(0.0, t-dt_));

        bcSet_.applyBeforeApplying(*map_);
        a += (theta*dt_) * map_->apply(a);
        bcSet_.applyAfterApplying(a);
    }

    void ExplicitEulerScheme::setStep(Time dt) {
        dt_ = dt;
    }
}
