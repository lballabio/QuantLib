/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2009 Ralph Schreyer

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

/*!
 * \file fdmsimple2dbssolver.cpp
*/

#include <ql/methods/finitedifferences/operators/fdmblackscholesop.hpp>
#include <ql/methods/finitedifferences/solvers/fdm2dimsolver.hpp>
#include <ql/methods/finitedifferences/solvers/fdmsimple2dbssolver.hpp>
#include <utility>

namespace QuantLib {

    FdmSimple2dBSSolver::FdmSimple2dBSSolver(Handle<GeneralizedBlackScholesProcess> process,
                                             Real strike,
                                             FdmSolverDesc solverDesc,
                                             const FdmSchemeDesc& schemeDesc)
    : process_(std::move(process)), strike_(strike), solverDesc_(std::move(solverDesc)),
      schemeDesc_(schemeDesc) {

        registerWith(process_);
    }

    void FdmSimple2dBSSolver::performCalculations() const {
        ext::shared_ptr<FdmBlackScholesOp> op(ext::make_shared<FdmBlackScholesOp>(
                solverDesc_.mesher, process_.currentLink(), strike_));

        solver_ = ext::make_shared<Fdm2DimSolver>(solverDesc_, schemeDesc_, op);
    }

    Real FdmSimple2dBSSolver::valueAt(Real s, Real a) const {
        calculate();
        return solver_->interpolateAt(std::log(s), std::log(a));
    }

    Real FdmSimple2dBSSolver::deltaAt(Real s, Real a, Real eps) const {
        return (valueAt(s+eps, a) - valueAt(s-eps, a))/(2*eps);
    }

    Real FdmSimple2dBSSolver::gammaAt(Real s, Real a, Real eps) const {
        return (valueAt(s+eps, a)+valueAt(s-eps, a)-2*valueAt(s,a))/(eps*eps);
    }

    Real FdmSimple2dBSSolver::thetaAt(Real s, Real a) const {
        calculate();
        return solver_->thetaAt(std::log(s), std::log(a));
    }
}
