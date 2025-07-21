/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2009 Klaus Spanderen

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

#include <ql/methods/finitedifferences/schemes/modifiedcraigsneydscheme.hpp>
#include <utility>

namespace QuantLib {

    ModifiedCraigSneydScheme::ModifiedCraigSneydScheme(Real theta,
                                                       Real mu,
                                                       ext::shared_ptr<FdmLinearOpComposite> map,
                                                       const bc_set& bcSet)
    : dt_(Null<Real>()), theta_(theta), mu_(mu), map_(std::move(map)), bcSet_(bcSet) {}

    void ModifiedCraigSneydScheme::step(array_type& a, Time t) {
        QL_REQUIRE(t-dt_ > -1e-8, "a step towards negative time given");
        map_->setTime(std::max(0.0, t-dt_), t);
        bcSet_.setTime(std::max(0.0, t-dt_));

        bcSet_.applyBeforeApplying(*map_);
        Array y = a + dt_*map_->apply(a);
        bcSet_.applyAfterApplying(y);

        Array y0 = y;

        for (Size i=0; i < map_->size(); ++i) {
            Array rhs = y - theta_*dt_*map_->apply_direction(i, a);
            y = map_->solve_splitting(i, rhs, -theta_*dt_);
        }

        bcSet_.applyBeforeApplying(*map_);
        Array yt =  y0 + mu_*dt_*map_->apply_mixed(y-a)
                  +(0.5-mu_)*dt_*map_->apply(y-a);;
        bcSet_.applyAfterApplying(yt);

        for (Size i=0; i < map_->size(); ++i) {
            Array rhs = yt - theta_*dt_*map_->apply_direction(i, a);
            yt = map_->solve_splitting(i, rhs, -theta_*dt_);
        }
        bcSet_.applyAfterSolving(yt);

        a = yt;
    }

    void ModifiedCraigSneydScheme::setStep(Time dt) {
        dt_=dt;
    }
}
