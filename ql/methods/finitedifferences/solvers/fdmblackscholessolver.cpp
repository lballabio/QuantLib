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
 <http://quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/processes/blackscholesprocess.hpp>
#include <ql/methods/finitedifferences/solvers/fdm1dimsolver.hpp>
#include <ql/methods/finitedifferences/operators/fdmblackscholesop.hpp>
#include <ql/methods/finitedifferences/solvers/fdmblackscholessolver.hpp>

namespace QuantLib {

    FdmBlackScholesSolver::FdmBlackScholesSolver(
        const Handle<GeneralizedBlackScholesProcess>& process,
        Real strike,
        const FdmSolverDesc& solverDesc,
        const FdmSchemeDesc& schemeDesc,
        bool localVol,
        Real illegalLocalVolOverwrite)
    : process_(process),
      strike_(strike),
      solverDesc_(solverDesc),
      schemeDesc_(schemeDesc),
      localVol_(localVol),
      illegalLocalVolOverwrite_(illegalLocalVolOverwrite) {

        registerWith(process_);
    }

    void FdmBlackScholesSolver::performCalculations() const {
        const boost::shared_ptr<FdmBlackScholesOp> op(new FdmBlackScholesOp(
                solverDesc_.mesher, process_.currentLink(), strike_,
                localVol_, illegalLocalVolOverwrite_));

        solver_ = boost::shared_ptr<Fdm1DimSolver>(
            new Fdm1DimSolver(solverDesc_, schemeDesc_, op));
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
