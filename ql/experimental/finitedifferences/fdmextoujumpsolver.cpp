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

/*! \file fdmextoujumpsolver.cpp
*/

#include <ql/experimental/processes/extouwithjumpsprocess.hpp>
#include <ql/methods/finitedifferences/solvers/fdm2dimsolver.hpp>
#include <ql/experimental/finitedifferences/fdmextoujumpop.hpp>
#include <ql/experimental/finitedifferences/fdmextoujumpsolver.hpp>

namespace QuantLib {

    FdmExtOUJumpSolver::FdmExtOUJumpSolver(
            const Handle<ExtOUWithJumpsProcess>& process,
            const boost::shared_ptr<YieldTermStructure>& rTS,
            const FdmSolverDesc& solverDesc,
            const FdmSchemeDesc& schemeDesc) 
    : process_     (process),
      rTS_         (rTS),
      solverDesc_  (solverDesc),
      schemeDesc_  (schemeDesc) {
                
        registerWith(process_);
    }
            
    void FdmExtOUJumpSolver::performCalculations() const {
        boost::shared_ptr<FdmLinearOpComposite>op(
            new FdmExtOUJumpOp(solverDesc_.mesher, process_.currentLink(),
                               rTS_, solverDesc_.bcSet, 32));
        
        solver_ = boost::shared_ptr<Fdm2DimSolver>(
                              new Fdm2DimSolver(solverDesc_, schemeDesc_, op));
    }
    
    Real FdmExtOUJumpSolver::valueAt(Real x, Real y) const {
        calculate();
        return solver_->interpolateAt(x, y);
    }
}

