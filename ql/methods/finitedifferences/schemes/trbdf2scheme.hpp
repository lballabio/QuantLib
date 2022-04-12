/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2018 Klaus Spanderen

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

/*! \file trbdf2scheme.hpp
    \brief trapezoidal BDF2 scheme
*/

#ifndef quantlib_tr_bdf2_scheme_hpp
#define quantlib_tr_bdf2_scheme_hpp

#include <ql/functional.hpp>
#include <ql/math/functional.hpp>
#include <ql/math/matrixutilities/bicgstab.hpp>
#include <ql/math/matrixutilities/gmres.hpp>
#include <ql/methods/finitedifferences/operators/fdmlinearopcomposite.hpp>
#include <ql/methods/finitedifferences/operatortraits.hpp>
#include <ql/methods/finitedifferences/schemes/boundaryconditionschemehelper.hpp>
#include <utility>

namespace QuantLib {

    template <class TrapezoidalScheme>
    class TrBDF2Scheme {
      public:
        enum SolverType { BiCGstab, GMRES };

        // typedefs
        typedef OperatorTraits<FdmLinearOp> traits;
        typedef traits::operator_type operator_type;
        typedef traits::array_type array_type;
        typedef traits::bc_set bc_set;
        typedef traits::condition_type condition_type;

        // constructors
        TrBDF2Scheme(Real alpha,
                     ext::shared_ptr<FdmLinearOpComposite> map,
                     const ext::shared_ptr<TrapezoidalScheme>& trapezoidalScheme,
                     const bc_set& bcSet = bc_set(),
                     Real relTol = 1e-8,
                     SolverType solverType = BiCGstab);

        void step(array_type& a, Time t);
        void setStep(Time dt);

        Size numberOfIterations() const;
      protected:
        Array apply(const Array& r) const;

        Time dt_;
        Real beta_;
        ext::shared_ptr<Size> iterations_;

        const Real alpha_;
        const ext::shared_ptr<FdmLinearOpComposite> map_;
        const ext::shared_ptr<TrapezoidalScheme>& trapezoidalScheme_;
        const BoundaryConditionSchemeHelper bcSet_;
        const Real relTol_;
        const SolverType solverType_;
    };

    template <class TrapezoidalScheme>
    inline TrBDF2Scheme<TrapezoidalScheme>::TrBDF2Scheme(
        Real alpha,
        ext::shared_ptr<FdmLinearOpComposite> map,
        const ext::shared_ptr<TrapezoidalScheme>& trapezoidalScheme,
        const bc_set& bcSet,
        Real relTol,
        SolverType solverType)
    : dt_(Null<Real>()), beta_(Null<Real>()), iterations_(ext::make_shared<Size>(0U)),
      alpha_(alpha), map_(std::move(map)), trapezoidalScheme_(trapezoidalScheme), bcSet_(bcSet),
      relTol_(relTol), solverType_(solverType) {}

    template <class TrapezoidalScheme>
    inline void TrBDF2Scheme<TrapezoidalScheme>::setStep(Time dt) {
        dt_=dt;
        beta_= (1.0-alpha_)/(2.0-alpha_)*dt_;
    }

    template <class TrapezoidalScheme>
    inline Size TrBDF2Scheme<TrapezoidalScheme>::numberOfIterations() const {
        return *iterations_;
    }

    template <class TrapezoidalScheme>
    inline Array TrBDF2Scheme<TrapezoidalScheme>::apply(const Array& r) const {
        return r - beta_*map_->apply(r);
    }

    template <class TrapezoidalScheme>
    inline void TrBDF2Scheme<TrapezoidalScheme>::step(array_type& fn, Time t) {
        QL_REQUIRE(t-dt_ > -1e-8, "a step towards negative time given");

        const Time intermediateTimeStep = dt_*alpha_;

        array_type fStar = fn;
        trapezoidalScheme_->setStep(intermediateTimeStep);
        trapezoidalScheme_->step(fStar, t);

        bcSet_.setTime(std::max(0.0, t-dt_));
        bcSet_.applyBeforeSolving(*map_, fn);

        const array_type f =
            (1/alpha_*fStar - square<Real>()(1-alpha_)/alpha_*fn)/(2-alpha_);

        if (map_->size() == 1) {
            fn = map_->solve_splitting(0, f, -beta_);
        }
        else {
            auto preconditioner = [&](const Array& _a){ return map_->preconditioner(_a, -beta_); };
            auto applyF = [&](const Array& _a){ return apply(_a); };

            if (solverType_ == BiCGstab) {
                const BiCGStabResult result =
                    QuantLib::BiCGstab(applyF, std::max(Size(10), fn.size()),
                        relTol_, preconditioner).solve(f, f);

                (*iterations_) += result.iterations;
                fn = result.x;
            } else if (solverType_ == GMRES) {
                const GMRESResult result =
                    QuantLib::GMRES(applyF, std::max(Size(10), fn.size() / 10U), relTol_,
                                    preconditioner)
                        .solve(f, f);

                (*iterations_) += result.errors.size();
                fn = result.x;
            }
            else
                QL_FAIL("unknown/illegal solver type");
        }

        bcSet_.applyAfterSolving(fn);
    }
}

#endif
