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

/*! \file fdm1dimsolver.hpp
*/

#ifndef quantlib_fdm_1_dim_solver_hpp
#define quantlib_fdm_1_dim_solver_hpp

#include <ql/handle.hpp>
#include <ql/math/matrix.hpp>
#include <ql/patterns/lazyobject.hpp>
#include <ql/methods/finitedifferences/solvers/fdmsolverdesc.hpp>
#include <ql/methods/finitedifferences/solvers/fdmbackwardsolver.hpp>


namespace QuantLib {

    class CubicInterpolation;
    class FdmSnapshotCondition;

    class Fdm1DimSolver : public LazyObject {
      public:
        Fdm1DimSolver(const FdmSolverDesc& solverDesc,
                      const FdmSchemeDesc& schemeDesc,
                      ext::shared_ptr<FdmLinearOpComposite> op);

        Real interpolateAt(Real x) const;
        Real thetaAt(Real x) const;

        Real derivativeX(Real x) const;
        Real derivativeXX(Real x) const;

      protected:
        void performCalculations() const override;

      private:
        const FdmSolverDesc solverDesc_;
        const FdmSchemeDesc schemeDesc_;
        const ext::shared_ptr<FdmLinearOpComposite> op_;

        const ext::shared_ptr<FdmSnapshotCondition> thetaCondition_;
        const ext::shared_ptr<FdmStepConditionComposite> conditions_;

        std::vector<Real> x_, initialValues_;
        mutable Array resultValues_;
        mutable ext::shared_ptr<CubicInterpolation> interpolation_;
    };
}

#endif


#ifndef id_0a97f29fe39af6b774cdd3bb2c4cc996
#define id_0a97f29fe39af6b774cdd3bb2c4cc996
inline bool test_0a97f29fe39af6b774cdd3bb2c4cc996(int* i) { return i != 0; }
#endif
