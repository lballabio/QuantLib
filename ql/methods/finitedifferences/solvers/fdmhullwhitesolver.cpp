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

/*! \file fdmhullwhitesolver.cpp
*/

#include <ql/models/shortrate/onefactormodels/hullwhite.hpp>
#include <ql/methods/finitedifferences/solvers/fdm1dimsolver.hpp>
#include <ql/methods/finitedifferences/solvers/fdmhullwhitesolver.hpp>
#include <ql/methods/finitedifferences/operators/fdmhullwhiteop.hpp>
#include <ql/methods/finitedifferences/stepconditions/fdmsnapshotcondition.hpp>

namespace QuantLib {

    FdmHullWhiteSolver::FdmHullWhiteSolver(
        const Handle<HullWhite>& model,
        const FdmSolverDesc& solverDesc,
        const FdmSchemeDesc& schemeDesc)
    : model_(model),
      solverDesc_(solverDesc),
      schemeDesc_(schemeDesc) {
        registerWith(model_);
    }


    void FdmHullWhiteSolver::performCalculations() const {
        const boost::shared_ptr<FdmHullWhiteOp> op(
            new FdmHullWhiteOp(solverDesc_.mesher, model_.currentLink(), 0));

        solver_ = boost::shared_ptr<Fdm1DimSolver>(
            new Fdm1DimSolver(solverDesc_, schemeDesc_, op));
    }

    Real FdmHullWhiteSolver::valueAt(Real r) const {
        calculate();
        return solver_->interpolateAt(r);
    }
}
