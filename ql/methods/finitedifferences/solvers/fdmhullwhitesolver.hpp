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

/*! \file fdmhullwhitesolver.hpp
*/

#ifndef quantlib_fdm_hull_white_solver_hpp
#define quantlib_fdm_hull_white_solver_hpp

#include <ql/handle.hpp>
#include <ql/patterns/lazyobject.hpp>
#include <ql/methods/finitedifferences/solvers/fdmsolverdesc.hpp>
#include <ql/methods/finitedifferences/solvers/fdmbackwardsolver.hpp>

namespace QuantLib {

    class HullWhite;
    class Fdm1DimSolver;

    class FdmHullWhiteSolver : public LazyObject {
      public:
        FdmHullWhiteSolver(Handle<HullWhite> model,
                           FdmSolverDesc solverDesc,
                           const FdmSchemeDesc& schemeDesc = FdmSchemeDesc::Hundsdorfer());

        Real valueAt(Real r) const;

      protected:
        void performCalculations() const override;

      private:
        const Handle<HullWhite> model_;
        const FdmSolverDesc solverDesc_;
        const FdmSchemeDesc schemeDesc_;

        mutable ext::shared_ptr<Fdm1DimSolver> solver_;
    };
}

#endif


#ifndef id_b9aad607eee86ab37e34361e80b40098
#define id_b9aad607eee86ab37e34361e80b40098
inline bool test_b9aad607eee86ab37e34361e80b40098(const int* i) {
    return i != nullptr;
}
#endif
