/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2019 Klaus Spanderen

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
#include <ql/methods/finitedifferences/schemes/cranknicolsonscheme.hpp>

namespace QuantLib {
    CrankNicolsonScheme::CrankNicolsonScheme(
        Real theta,
        const ext::shared_ptr<FdmLinearOpComposite> & map,
        const bc_set& bcSet,
        Real relTol,
        ImplicitEulerScheme::SolverType solverType)
    : dt_(Null<Real>()),
      theta_(theta),
      explicit_(ext::make_shared<ExplicitEulerScheme>(map, bcSet)),
      implicit_(ext::make_shared<ImplicitEulerScheme>(
          map, bcSet, relTol, solverType)) {
    }

    void CrankNicolsonScheme::step(array_type& a, Time t) {
        QL_REQUIRE(t-dt_ > -1e-8, "a step towards negative time given");

        if (theta_ != 1.0)
            explicit_->step(a, t, 1.0-theta_);

        if (theta_ != 0.0)
            implicit_->step(a, t, theta_);
    }

    void CrankNicolsonScheme::setStep(Time dt) {
        dt_ = dt;
        explicit_->setStep(dt_);
        implicit_->setStep(dt_);
    }

    Size CrankNicolsonScheme::numberOfIterations() const {
        return implicit_->numberOfIterations();
    }
}
