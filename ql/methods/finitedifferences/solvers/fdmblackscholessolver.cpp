/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Andreas Gaida
 Copyright (C) 2008, 2009 Ralph Schreyer
 Copyright (C) 2008, 2009 Klaus Spanderen

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

#include <ql/methods/finitedifferences/operators/fdmblackscholesop.hpp>
#include <ql/methods/finitedifferences/solvers/fdm1dimsolver.hpp>
#include <ql/methods/finitedifferences/solvers/fdmblackscholessolver.hpp>
#include <ql/processes/blackscholesprocess.hpp>
#include <utility>

namespace QuantLib {

    FdmBlackScholesSolver::FdmBlackScholesSolver(Handle<GeneralizedBlackScholesProcess> process,
                                                 Real strike,
                                                 FdmSolverDesc solverDesc,
                                                 const FdmSchemeDesc& schemeDesc,
                                                 bool localVol,
                                                 Real illegalLocalVolOverwrite,
                                                 Handle<FdmQuantoHelper> quantoHelper)
    : process_(std::move(process)), strike_(strike), solverDesc_(std::move(solverDesc)),
      schemeDesc_(schemeDesc), localVol_(localVol),
      illegalLocalVolOverwrite_(illegalLocalVolOverwrite), quantoHelper_(std::move(quantoHelper)) {

        registerWith(process_);
        registerWith(quantoHelper_);
    }

    void FdmBlackScholesSolver::performCalculations() const {
            const ext::shared_ptr<FdmBlackScholesOp> op(
            ext::make_shared<FdmBlackScholesOp>(
                solverDesc_.mesher, process_.currentLink(), strike_,
                localVol_, illegalLocalVolOverwrite_, 0,
                (quantoHelper_.empty())
                    ? ext::shared_ptr<FdmQuantoHelper>()
                    : quantoHelper_.currentLink()));

        solver_ = ext::make_shared<Fdm1DimSolver>(solverDesc_, schemeDesc_, op);
    }

    Real FdmBlackScholesSolver::valueAt(Real s) const {
        calculate();
        return solver_->interpolateAt(std::log(s));
    }

    Real FdmBlackScholesSolver::deltaAt(Real s) const {
        calculate();
        return solver_->derivativeX(std::log(s))/s;
    }

    Real FdmBlackScholesSolver::gammaAt(Real s) const {
        calculate();
        return (solver_->derivativeXX(std::log(s))
                -solver_->derivativeX(std::log(s)))/(s*s);
    }

    Real FdmBlackScholesSolver::thetaAt(Real s) const {
        return solver_->thetaAt(std::log(s));
    }
}
