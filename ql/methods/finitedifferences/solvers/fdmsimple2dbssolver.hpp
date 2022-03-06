/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2009 Ralph Schreyer

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

/*!
 * \file fdmsimple2dbssolver.hpp
*/

#ifndef quantlib_fdm_simple_2d_bs_solver_hpp
#define quantlib_fdm_simple_2d_bs_solver_hpp

#include <ql/handle.hpp>
#include <ql/patterns/lazyobject.hpp>
#include <ql/methods/finitedifferences/solvers/fdmsolverdesc.hpp>
#include <ql/methods/finitedifferences/solvers/fdmbackwardsolver.hpp>
#include <ql/methods/finitedifferences/solvers/fdmbackwardsolver.hpp>

namespace QuantLib {

    class Fdm2DimSolver;
    class GeneralizedBlackScholesProcess;

    class FdmSimple2dBSSolver : public LazyObject {
      public:
        FdmSimple2dBSSolver(Handle<GeneralizedBlackScholesProcess> process,
                            Real strike,
                            FdmSolverDesc desc,
                            const FdmSchemeDesc& schemeDesc = FdmSchemeDesc::Douglas());

        Real valueAt(Real s, Real a) const;
        Real deltaAt(Real s, Real a, Real eps) const;
        Real gammaAt(Real s, Real a, Real eps) const;
        Real thetaAt(Real s, Real a) const;

      protected:
        void performCalculations() const override;

      private:
        Handle<GeneralizedBlackScholesProcess> process_;
        const Real strike_;
        const FdmSolverDesc solverDesc_;
        const FdmSchemeDesc schemeDesc_;

        mutable ext::shared_ptr<Fdm2DimSolver> solver_;
    };
}

#endif /* quantlib_fdm_simple_2d_bs_solver_hpp */


#ifndef id_08cc9d55784faec86b23a84993460cc3
#define id_08cc9d55784faec86b23a84993460cc3
inline bool test_08cc9d55784faec86b23a84993460cc3(const int* i) {
    return i != nullptr;
}
#endif
