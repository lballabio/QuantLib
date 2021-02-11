/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Andreas Gaida
 Copyright (C) 2008, 2009 Ralph Schreyer
 Copyright (C) 2008, 2009, 2011, 2014, 2015 Klaus Spanderen
 Copyright (C) 2015 Johannes Göttker-Schnetmann

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

#include <ql/methods/finitedifferences/operators/fdmhestonop.hpp>
#include <ql/methods/finitedifferences/solvers/fdm2dimsolver.hpp>
#include <ql/methods/finitedifferences/solvers/fdmhestonsolver.hpp>
#include <ql/processes/hestonprocess.hpp>
#include <utility>

namespace QuantLib {

    FdmHestonSolver::FdmHestonSolver(Handle<HestonProcess> process,
                                     FdmSolverDesc solverDesc,
                                     const FdmSchemeDesc& schemeDesc,
                                     Handle<FdmQuantoHelper> quantoHelper,
                                     ext::shared_ptr<LocalVolTermStructure> leverageFct,
                                     const Real mixingFactor)
    : process_(std::move(process)), solverDesc_(std::move(solverDesc)), schemeDesc_(schemeDesc),
      quantoHelper_(std::move(quantoHelper)), leverageFct_(std::move(leverageFct)),
      mixingFactor_(mixingFactor) {

        registerWith(process_);
        registerWith(quantoHelper_);
    }

    void FdmHestonSolver::performCalculations() const {
        ext::shared_ptr<FdmLinearOpComposite> op(
			ext::make_shared<FdmHestonOp>(
                solverDesc_.mesher, process_.currentLink(),
                (!quantoHelper_.empty()) ? quantoHelper_.currentLink()
                             : ext::shared_ptr<FdmQuantoHelper>(),
                leverageFct_, mixingFactor_));

        solver_ = ext::make_shared<Fdm2DimSolver>(solverDesc_, schemeDesc_, op);
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
