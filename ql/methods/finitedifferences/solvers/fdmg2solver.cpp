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

/*! \file fdmg2solver.cpp
*/

#include <ql/methods/finitedifferences/operators/fdmg2op.hpp>
#include <ql/methods/finitedifferences/solvers/fdm2dimsolver.hpp>
#include <ql/methods/finitedifferences/solvers/fdmg2solver.hpp>
#include <ql/methods/finitedifferences/stepconditions/fdmsnapshotcondition.hpp>
#include <ql/models/shortrate/twofactormodels/g2.hpp>
#include <utility>

namespace QuantLib {

    FdmG2Solver::FdmG2Solver(Handle<G2> model,
                             FdmSolverDesc solverDesc,
                             const FdmSchemeDesc& schemeDesc)
    : model_(std::move(model)), solverDesc_(std::move(solverDesc)), schemeDesc_(schemeDesc) {
        registerWith(model_);
    }


    void FdmG2Solver::performCalculations() const {
        const ext::shared_ptr<FdmG2Op> op(
            new FdmG2Op(solverDesc_.mesher, model_.currentLink(), 0, 1));

        solver_ = ext::make_shared<Fdm2DimSolver>(
            solverDesc_, schemeDesc_, op);
    }

    Real FdmG2Solver::valueAt(Real x, Real y) const {
        calculate();
        return solver_->interpolateAt(x, y);
    }
}
