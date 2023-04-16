/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2020 Lew Wei Hao

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

#include <ql/methods/finitedifferences/operators/fdmcirop.hpp>
#include <ql/methods/finitedifferences/solvers/fdm2dimsolver.hpp>
#include <ql/methods/finitedifferences/solvers/fdmcirsolver.hpp>
#include <ql/processes/hestonprocess.hpp>
#include <utility>

namespace QuantLib {

    FdmCIRSolver::FdmCIRSolver(Handle<CoxIngersollRossProcess> cirProcess,
                               Handle<GeneralizedBlackScholesProcess> bsProcess,
                               FdmSolverDesc solverDesc,
                               const FdmSchemeDesc& schemeDesc,
                               const Real rho,
                               const Real strike)
    : bsProcess_(std::move(bsProcess)), cirProcess_(std::move(cirProcess)),
      solverDesc_(std::move(solverDesc)), schemeDesc_(schemeDesc), rho_(rho), strike_(strike) {
        registerWith(bsProcess_);
        registerWith(cirProcess_);
    }

    void FdmCIRSolver::performCalculations() const {
        std::shared_ptr<FdmLinearOpComposite> op(
			std::make_shared<FdmCIROp>(
                solverDesc_.mesher,
                cirProcess_.currentLink(),
                bsProcess_.currentLink(),
                rho_,
                strike_));

        solver_ = std::make_shared<Fdm2DimSolver>(solverDesc_, schemeDesc_, op);
    }

    Real FdmCIRSolver::valueAt(Real s, Real r) const {
        calculate();
        return solver_->interpolateAt(std::log(s), r);
    }

    Real FdmCIRSolver::deltaAt(Real s, Real r) const {
        calculate();
        return solver_->derivativeX(std::log(s), r)/s;
    }

    Real FdmCIRSolver::gammaAt(Real s, Real r) const {
        calculate();
        const Real x = std::log(s);
        return (solver_->derivativeXX(x, r)-solver_->derivativeX(x, r))/(s*s);
    }

    Real FdmCIRSolver::thetaAt(Real s, Real r) const {
        calculate();
        return solver_->thetaAt(std::log(s), r);
    }
}
