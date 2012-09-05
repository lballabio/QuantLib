/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Andreas Gaida
 Copyright (C) 2008, 2009 Ralph Schreyer
 Copyright (C) 2008, 2009, 2011 Klaus Spanderen

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

#include <ql/processes/hestonprocess.hpp>
#include <ql/methods/finitedifferences/operators/fdmhestonop.hpp>
#include <ql/methods/finitedifferences/solvers/fdm2dimsolver.hpp>
#include <ql/methods/finitedifferences/solvers/fdmhestonsolver.hpp>

namespace QuantLib {

    FdmHestonSolver::FdmHestonSolver(
        const Handle<HestonProcess>& process,
        const FdmSolverDesc& solverDesc,
        const FdmSchemeDesc& schemeDesc,
        const Handle<FdmQuantoHelper>& quantoHelper)
    : process_(process),
      solverDesc_(solverDesc),
      schemeDesc_(schemeDesc),
      quantoHelper_(quantoHelper) {

        registerWith(process_);
        registerWith(quantoHelper_);
    }

    void FdmHestonSolver::performCalculations() const {
        boost::shared_ptr<FdmLinearOpComposite> op(
                new FdmHestonOp(
                        solverDesc_.mesher, process_.currentLink(),
                        (!quantoHelper_.empty()) ? quantoHelper_.currentLink()
                                     : boost::shared_ptr<FdmQuantoHelper>()));

        solver_ = boost::shared_ptr<Fdm2DimSolver>(
                               new Fdm2DimSolver(solverDesc_, schemeDesc_, op));
    }

    Real FdmHestonSolver::valueAt(Real s, Real v) const {
        calculate();
        return solver_->interpolateAt(std::log(s), v);
    }

    Real FdmHestonSolver::deltaAt(Real s, Real v) const {
        calculate();
        return solver_->derivativeX(std::log(s), v)/s;
    }

    Real FdmHestonSolver::gammaAt(Real s, Real v) const {
        calculate();
        const Real x = std::log(s);
        return (solver_->derivativeXX(x, v)-solver_->derivativeX(x, v))/(s*s);
    }

    Real FdmHestonSolver::meanVarianceDeltaAt(Real s, Real v) const {
        calculate();

        const Real alpha = process_->rho()*process_->sigma()/s;
        return deltaAt(s, v) + alpha*solver_->derivativeY(std::log(s), v);
    }

    Real FdmHestonSolver::meanVarianceGammaAt(Real s, Real v) const {
        calculate();

        const Real x = std::log(s);
        const Real alpha = process_->rho()*process_->sigma()/s;
        return gammaAt(s, v)
                +  solver_->derivativeYY(x, v)*alpha*alpha
                +2*solver_->derivativeXY(x, v)*alpha/s;
    }

    Real FdmHestonSolver::thetaAt(Real s, Real v) const {
        calculate();
        return solver_->thetaAt(std::log(s), v);
    }
}
