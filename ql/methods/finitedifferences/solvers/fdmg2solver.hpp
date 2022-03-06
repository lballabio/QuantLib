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

/*! \file fdmg2solver.hpp
*/

#ifndef quantlib_fdm_g2_solver_hpp
#define quantlib_fdm_g2_solver_hpp

#include <ql/handle.hpp>
#include <ql/patterns/lazyobject.hpp>
#include <ql/methods/finitedifferences/solvers/fdmsolverdesc.hpp>
#include <ql/methods/finitedifferences/solvers/fdmbackwardsolver.hpp>

namespace QuantLib {

    class G2;
    class Fdm2DimSolver;

    class FdmG2Solver : public LazyObject {
      public:
        FdmG2Solver(Handle<G2> model,
                    FdmSolverDesc solverDesc,
                    const FdmSchemeDesc& schemeDesc = FdmSchemeDesc::Hundsdorfer());

        Real valueAt(Real x, Real y) const;

      protected:
        void performCalculations() const override;

      private:
        const Handle<G2> model_;
        const FdmSolverDesc solverDesc_;
        const FdmSchemeDesc schemeDesc_;

        mutable ext::shared_ptr<Fdm2DimSolver> solver_;
    };
}

#endif


#ifndef id_695dd077f374aa98cddc8d688bf3c4e6
#define id_695dd077f374aa98cddc8d688bf3c4e6
inline bool test_695dd077f374aa98cddc8d688bf3c4e6(int* i) { return i != 0; }
#endif
