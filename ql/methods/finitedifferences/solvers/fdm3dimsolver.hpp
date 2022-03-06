/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2011 Klaus Spanderen

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

/*! \file fdm3dimsolver.hpp
*/

#ifndef quantlib_fdm_3_dim_solver_hpp
#define quantlib_fdm_3_dim_solver_hpp

#include <ql/math/matrix.hpp>
#include <ql/patterns/lazyobject.hpp>
#include <ql/methods/finitedifferences/solvers/fdmsolverdesc.hpp>
#include <ql/methods/finitedifferences/solvers/fdmbackwardsolver.hpp>


namespace QuantLib {

    class BicubicSpline;
    class FdmSnapshotCondition;

    class Fdm3DimSolver : public LazyObject {
      public:
        Fdm3DimSolver(const FdmSolverDesc& solverDesc,
                      const FdmSchemeDesc& schemeDesc,
                      ext::shared_ptr<FdmLinearOpComposite> op);

        void performCalculations() const override;

        Real interpolateAt(Real x, Real y, Rate z) const;
        Real thetaAt(Real x, Real y, Rate z) const;

      private:
        const FdmSolverDesc solverDesc_;
        const FdmSchemeDesc schemeDesc_;
        const ext::shared_ptr<FdmLinearOpComposite> op_;

        const ext::shared_ptr<FdmSnapshotCondition> thetaCondition_;
        const ext::shared_ptr<FdmStepConditionComposite> conditions_;

        std::vector<Real> x_, y_, z_, initialValues_;
        mutable std::vector<Matrix> resultValues_;
        mutable std::vector<ext::shared_ptr<BicubicSpline> > interpolation_;
    };
}

#endif


#ifndef id_20f48ba04bdcd5fe7cd25a59fd52247b
#define id_20f48ba04bdcd5fe7cd25a59fd52247b
inline bool test_20f48ba04bdcd5fe7cd25a59fd52247b(int* i) { return i != 0; }
#endif
