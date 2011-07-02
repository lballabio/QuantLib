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

/*! \file fdmklugeextousolver.cpp
*/

#include <ql/experimental/processes/extouwithjumpsprocess.hpp>
#include <ql/experimental/processes/extendedornsteinuhlenbeckprocess.hpp>

#include <ql/experimental/finitedifferences/fdmndimsolver.hpp>
#include <ql/experimental/finitedifferences/fdmklugeextouop.hpp>
#include <ql/experimental/finitedifferences/fdmklugeextousolver.hpp>

namespace QuantLib {

    FdmKlugeExtOUSolver::FdmKlugeExtOUSolver(
        Real correlation,
        const Handle<ExtOUWithJumpsProcess>& klugeProcess,
        const Handle<ExtendedOrnsteinUhlenbeckProcess>& ouProcess,
        const boost::shared_ptr<YieldTermStructure>& rTS,
        const FdmSolverDesc& solverDesc,
        const FdmSchemeDesc& schemeDesc)
    : klugeProcess_(klugeProcess),
      ouProcess_   (ouProcess),
      correlation_ (correlation),
      rTS_         (rTS),
      solverDesc_  (solverDesc),
      schemeDesc_  (schemeDesc) {
        registerWith(klugeProcess_);
        registerWith(ouProcess_);
    }

    void FdmKlugeExtOUSolver::performCalculations() const {
        boost::shared_ptr<FdmLinearOpComposite>op(
            new FdmKlugeExtOUOp(solverDesc_.mesher,
                                correlation_,
                                klugeProcess_.currentLink(),
                                ouProcess_.currentLink(),
                                rTS_, solverDesc_.bcSet, 32));

        solver_ = boost::shared_ptr<FdmNdimSolver<3> >(
                         new FdmNdimSolver<3>(solverDesc_, schemeDesc_, op));
    }

    Real FdmKlugeExtOUSolver::valueAt(Real x, Real y, Real z) const {
        calculate();

        std::vector<Real> u(3);
        u[0] = x; u[1] = y; u[2] = z;

        return solver_->interpolateAt(u);
    }
}
