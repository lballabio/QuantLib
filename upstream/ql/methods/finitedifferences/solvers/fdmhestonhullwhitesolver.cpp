/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2009 Klaus Spanderen

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

#include <ql/methods/finitedifferences/operators/fdmhestonhullwhiteop.hpp>
#include <ql/methods/finitedifferences/solvers/fdm3dimsolver.hpp>
#include <ql/methods/finitedifferences/solvers/fdmhestonhullwhitesolver.hpp>
#include <ql/methods/finitedifferences/utilities/fdmquantohelper.hpp>
#include <utility>

namespace QuantLib {

    FdmHestonHullWhiteSolver::FdmHestonHullWhiteSolver(const Handle<HestonProcess>& hestonProcess,
                                                       const Handle<HullWhiteProcess>& hwProcess,
                                                       Rate corrEquityShortRate,
                                                       FdmSolverDesc solverDesc,
                                                       const FdmSchemeDesc& schemeDesc)
    : hestonProcess_(hestonProcess), hwProcess_(hwProcess),
      corrEquityShortRate_(corrEquityShortRate), solverDesc_(std::move(solverDesc)),
      schemeDesc_(schemeDesc) {

        registerWith(hestonProcess);
        registerWith(hwProcess);
    }

    void FdmHestonHullWhiteSolver::performCalculations() const {
        const ext::shared_ptr<FdmLinearOpComposite> op(
			ext::make_shared<FdmHestonHullWhiteOp>(solverDesc_.mesher,
                                     hestonProcess_.currentLink(),
                                     hwProcess_.currentLink(), 
                                     corrEquityShortRate_));

        solver_ = ext::make_shared<Fdm3DimSolver>(solverDesc_, schemeDesc_, op);
    }

    Real FdmHestonHullWhiteSolver::valueAt(Real s, Real v, Rate r) const {
        calculate();

        const Real x = std::log(s);
        return solver_->interpolateAt(x, v, r);
    }

    Real FdmHestonHullWhiteSolver::deltaAt(Real s, Real v, Rate r, Real eps) 
    const {
        return (valueAt(s+eps, v, r) - valueAt(s-eps, v, r))/(2*eps);
    }

    Real FdmHestonHullWhiteSolver::gammaAt(Real s, Real v, Rate r, Real eps) 
    const {
        return (valueAt(s+eps, v, r)+valueAt(s-eps, v,r )
                -2*valueAt(s, v, r))/(eps*eps);
    }

    Real FdmHestonHullWhiteSolver::thetaAt(Real s, Real v, Rate r) const {
        calculate();

        const Real x = std::log(s);
        return solver_->thetaAt(x, v, r);
    }
}
