/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2009 Andreas Gaida
 Copyright (C) 2009 Ralph Schreyer
 Copyright (C) 2009, 2017 Klaus Spanderen

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


#include <ql/math/matrixutilities/bicgstab.hpp>
#include <ql/math/matrixutilities/gmres.hpp>
#include <ql/methods/finitedifferences/schemes/impliciteulerscheme.hpp>
#include <utility>

namespace QuantLib {

    ImplicitEulerScheme::ImplicitEulerScheme(std::shared_ptr<FdmLinearOpComposite> map,
                                             const bc_set& bcSet,
                                             Real relTol,
                                             SolverType solverType)
    : dt_(Null<Real>()), iterations_(std::make_shared<Size>(0U)), relTol_(relTol),
      map_(std::move(map)), bcSet_(bcSet), solverType_(solverType) {}

    Array ImplicitEulerScheme::apply(const Array& r, Real theta) const {
        return r - (theta*dt_)*map_->apply(r);
    }

    void ImplicitEulerScheme::step(array_type& a, Time t) {
        step(a, t, 1.0);
    }

    void ImplicitEulerScheme::step(array_type& a, Time t, Real theta) {
        QL_REQUIRE(t-dt_ > -1e-8, "a step towards negative time given");
        map_->setTime(std::max(0.0, t-dt_), t);
        bcSet_.setTime(std::max(0.0, t-dt_));

        bcSet_.applyBeforeSolving(*map_, a);

        if (map_->size() == 1) {
            a = map_->solve_splitting(0, a, -theta*dt_);
        }
        else {
            auto preconditioner = [&](const Array& _a){ return map_->preconditioner(_a, -theta*dt_); };
            auto applyF = [&](const Array& _a){ return apply(_a, theta); };

            if (solverType_ == BiCGstab) {
                const BiCGStabResult result =
                    QuantLib::BiCGstab(applyF, std::max(Size(10), a.size()),
                        relTol_, preconditioner).solve(a, a);

                (*iterations_) += result.iterations;
                a = result.x;
            }
            else if (solverType_ == GMRES) {
                const GMRESResult result =
                    QuantLib::GMRES(applyF, std::max(Size(10), a.size() / 10U), relTol_,
                                    preconditioner)
                        .solve(a, a);

                (*iterations_) += result.errors.size();
                a = result.x;
            }
            else
                QL_FAIL("unknown/illegal solver type");
        }
        bcSet_.applyAfterSolving(a);
    }

    void ImplicitEulerScheme::setStep(Time dt) {
        dt_=dt;
    }

    Size ImplicitEulerScheme::numberOfIterations() const {
        return *iterations_;
    }
}
