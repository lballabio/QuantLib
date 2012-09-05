/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2010 Klaus Spanderen

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
#include <ql/methods/finitedifferences/solvers/fdm2dimsolver.hpp>
#include <ql/methods/finitedifferences/operators/fdm2dblackscholesop.hpp>
#include <ql/methods/finitedifferences/solvers/fdm2dblackscholessolver.hpp>


namespace QuantLib {

    Fdm2dBlackScholesSolver::Fdm2dBlackScholesSolver(
        const Handle<GeneralizedBlackScholesProcess>& p1,
        const Handle<GeneralizedBlackScholesProcess>& p2,
        const Real correlation,
        const FdmSolverDesc& solverDesc,
        const FdmSchemeDesc& schemeDesc)
    : p1_(p1),
      p2_(p2),
      correlation_(correlation),
      solverDesc_(solverDesc),
      schemeDesc_(schemeDesc) {
        registerWith(p1_);
        registerWith(p2_);
    }

        
    void Fdm2dBlackScholesSolver::performCalculations() const {
        
        boost::shared_ptr<Fdm2dBlackScholesOp> op(
                new Fdm2dBlackScholesOp(solverDesc_.mesher,
                                        p1_.currentLink(), 
                                        p2_.currentLink(), 
                                        correlation_,
                                        solverDesc_.maturity));

        solver_ = boost::shared_ptr<Fdm2DimSolver>(
                            new Fdm2DimSolver(solverDesc_, schemeDesc_, op));
    }

    Real Fdm2dBlackScholesSolver::valueAt(Real u, Real v) const {
        calculate();
        const Real x = std::log(u);
        const Real y = std::log(v);

        return solver_->interpolateAt(x, y);
    }
    
    Real Fdm2dBlackScholesSolver::thetaAt(Real u, Real v) const {
        calculate();
        const Real x = std::log(u);
        const Real y = std::log(v);
        return solver_->thetaAt(x, y);
    }


    Real Fdm2dBlackScholesSolver::deltaXat(Real u, Real v) const {
        calculate();

        const Real x = std::log(u);
        const Real y = std::log(v);

        return solver_->derivativeX(x, y)/u;
    }

    Real Fdm2dBlackScholesSolver::deltaYat(Real u, Real v) const {
        calculate();

        const Real x = std::log(u);
        const Real y = std::log(v);

        return solver_->derivativeY(x, y)/v;
    }

    Real Fdm2dBlackScholesSolver::gammaXat(Real u, Real v) const {
        calculate();
        
        const Real x = std::log(u);
        const Real y = std::log(v);
        
        return (solver_->derivativeXX(x, y)
                -solver_->derivativeX(x, y))/(u*u);
    }

    Real Fdm2dBlackScholesSolver::gammaYat(Real u, Real v) const {
        calculate();
        
        const Real x = std::log(u);
        const Real y = std::log(v);
        
        return (solver_->derivativeYY(x, y)
                -solver_->derivativeY(x, y))/(v*v);
    }
}
