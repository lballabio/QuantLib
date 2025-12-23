/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2011 Klaus Spanderen

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

/*! \file fdmndimsolver.hpp
*/

#ifndef quantlib_fdm_n_dim_solver_hpp
#define quantlib_fdm_n_dim_solver_hpp

#include <ql/patterns/lazyobject.hpp>
#include <ql/math/interpolations/multicubicspline.hpp>
#include <ql/methods/finitedifferences/finitedifferencemodel.hpp>
#include <ql/methods/finitedifferences/meshers/fdmmesher.hpp>
#include <ql/methods/finitedifferences/solvers/fdmsolverdesc.hpp>
#include <ql/methods/finitedifferences/operators/fdmlinearoplayout.hpp>
#include <ql/methods/finitedifferences/solvers/fdmbackwardsolver.hpp>
#include <ql/methods/finitedifferences/stepconditions/fdmsnapshotcondition.hpp>
#include <ql/methods/finitedifferences/utilities/fdminnervaluecalculator.hpp>
#include <ql/methods/finitedifferences/stepconditions/fdmstepconditioncomposite.hpp>

#include <numeric>

namespace QuantLib {

    template <Size N>
    class FdmNdimSolver : public LazyObject {
      public:
        FdmNdimSolver(const FdmSolverDesc& solverDesc,
                      const FdmSchemeDesc& schemeDesc,
                      ext::shared_ptr<FdmLinearOpComposite> op);

        void performCalculations() const override;

        Real interpolateAt(const std::vector<Real>& x) const;
        Real thetaAt(const std::vector<Real>& x) const;

        // template meta programming
        typedef typename MultiCubicSpline<N>::data_table data_table;
        void static setValue(data_table& f,
                             const std::vector<Size>& x, Real value);

      private:
        const FdmSolverDesc solverDesc_;
        const FdmSchemeDesc schemeDesc_;
        const ext::shared_ptr<FdmLinearOpComposite> op_;

        const ext::shared_ptr<FdmSnapshotCondition> thetaCondition_;
        const ext::shared_ptr<FdmStepConditionComposite> conditions_;

        std::vector<std::vector<Real> > x_;
        std::vector<Real> initialValues_;
        const std::vector<bool> extrapolation_;

        mutable ext::shared_ptr<data_table> f_;
        mutable ext::shared_ptr<MultiCubicSpline<N> > interp_;
    };


    template <Size N>
    inline FdmNdimSolver<N>::FdmNdimSolver(const FdmSolverDesc& solverDesc,
                                           const FdmSchemeDesc& schemeDesc,
                                           ext::shared_ptr<FdmLinearOpComposite> op)
    : solverDesc_(solverDesc), schemeDesc_(schemeDesc), op_(std::move(op)),
      thetaCondition_(new FdmSnapshotCondition(
          0.99 * std::min(1.0 / 365.0,
                          solverDesc.condition->stoppingTimes().empty() ?
                              solverDesc.maturity :
                              solverDesc.condition->stoppingTimes().front()))),
      conditions_(FdmStepConditionComposite::joinConditions(thetaCondition_, solverDesc.condition)),
      x_(solverDesc.mesher->layout()->dim().size()),
      initialValues_(solverDesc.mesher->layout()->size()),
      extrapolation_(std::vector<bool>(N, false)) {

        QL_REQUIRE(solverDesc.mesher->layout()->dim().size() == N, "solver dim " << N
                    << "does not fit to layout dim " << solverDesc.mesher->layout()->size());

        for (Size i=0; i < N; ++i) {
            x_[i].reserve(solverDesc.mesher->layout()->dim()[i]);
        }

        for (const auto& iter : *solverDesc.mesher->layout()) {
            initialValues_[iter.index()] = solverDesc_.calculator
                                ->avgInnerValue(iter, solverDesc.maturity);

            const std::vector<Size>& c = iter.coordinates();
            for (Size i=0; i < N; ++i) {
                if ((std::accumulate(c.begin(), c.end(), 0UL) - c[i]) == 0U) {
                    x_[i].push_back(solverDesc.mesher->location(iter, i));
                }
            }
        }

        f_ = ext::shared_ptr<data_table>(new data_table(x_));
    }


    template <Size N> inline
    void FdmNdimSolver<N>::performCalculations() const {
        Array rhs(initialValues_.size());
        std::copy(initialValues_.begin(), initialValues_.end(), rhs.begin());

        FdmBackwardSolver(op_, solverDesc_.bcSet, conditions_, schemeDesc_)
                 .rollback(rhs, solverDesc_.maturity, 0.0,
                           solverDesc_.timeSteps, solverDesc_.dampingSteps);

        for (const auto& iter : *solverDesc_.mesher->layout()) {
            setValue(*f_, iter.coordinates(), rhs[iter.index()]);
        }

        interp_ = ext::shared_ptr<MultiCubicSpline<N> >(
            new MultiCubicSpline<N>(x_, *f_, extrapolation_));
    }


    template <Size N> inline
    Real FdmNdimSolver<N>::thetaAt(const std::vector<Real>& x) const {
        if (conditions_->stoppingTimes().front() == 0.0)
            return Null<Real>();

        calculate();
        const Array& rhs = thetaCondition_->getValues();

        data_table f(x_);

        for (const auto& iter : *solverDesc_.mesher->layout()) {
            setValue(f, iter.coordinates(), rhs[iter.index()]);
        }

        return (MultiCubicSpline<N>(x_, f)(x)
                        - interpolateAt(x)) / thetaCondition_->getTime();
    }

    template <Size N> inline
    Real FdmNdimSolver<N>::interpolateAt(const std::vector<Real>& x) const {
        calculate();

        return (*interp_)(x);
    }

    template <Size N> inline
    void FdmNdimSolver<N>::setValue(data_table& f,
                                    const std::vector<Size>& x, Real value) {
        FdmNdimSolver<N-1>::setValue(f[x[x.size()-N]], x, value);
    }

    template <> inline
    void FdmNdimSolver<1>::setValue(data_table& f,
                                    const std::vector<Size>& x, Real value) {
        f[x.back()] = value;
    }
}

#endif
