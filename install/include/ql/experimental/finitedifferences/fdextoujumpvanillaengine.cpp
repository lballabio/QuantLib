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

/*! \file fdoujumpvanillaengine.cpp
    \brief Finite Differences Ornstein Uhlenbeck plus exponential jumps engine 
           for simple swing options
*/

#include <ql/exercise.hpp>
#include <ql/experimental/finitedifferences/fdextoujumpvanillaengine.hpp>
#include <ql/experimental/finitedifferences/fdmextoujumpmodelinnervalue.hpp>
#include <ql/experimental/finitedifferences/fdmextoujumpsolver.hpp>
#include <ql/experimental/processes/extendedornsteinuhlenbeckprocess.hpp>
#include <ql/experimental/processes/extouwithjumpsprocess.hpp>
#include <ql/methods/finitedifferences/meshers/exponentialjump1dmesher.hpp>
#include <ql/methods/finitedifferences/meshers/fdmmeshercomposite.hpp>
#include <ql/methods/finitedifferences/meshers/fdmsimpleprocess1dmesher.hpp>
#include <ql/methods/finitedifferences/operators/fdmlinearoplayout.hpp>
#include <ql/methods/finitedifferences/stepconditions/fdmamericanstepcondition.hpp>
#include <ql/methods/finitedifferences/stepconditions/fdmbermudanstepcondition.hpp>
#include <ql/methods/finitedifferences/stepconditions/fdmstepconditioncomposite.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>
#include <utility>

namespace QuantLib {

    FdExtOUJumpVanillaEngine::FdExtOUJumpVanillaEngine(
        ext::shared_ptr<ExtOUWithJumpsProcess> process,
        ext::shared_ptr<YieldTermStructure> rTS,
        Size tGrid,
        Size xGrid,
        Size yGrid,
        ext::shared_ptr<Shape> shape,
        const FdmSchemeDesc& schemeDesc)
    : process_(std::move(process)), rTS_(std::move(rTS)), shape_(std::move(shape)), tGrid_(tGrid),
      xGrid_(xGrid), yGrid_(yGrid), schemeDesc_(schemeDesc) {}

    void FdExtOUJumpVanillaEngine::calculate() const {
        // 1. Mesher
        const Time maturity 
            = rTS_->dayCounter().yearFraction(rTS_->referenceDate(),
                                              arguments_.exercise->lastDate());
        const ext::shared_ptr<StochasticProcess1D> ouProcess(
            process_->getExtendedOrnsteinUhlenbeckProcess());
        const ext::shared_ptr<Fdm1dMesher> xMesher(
            new FdmSimpleProcess1dMesher(xGrid_, ouProcess,maturity));

        const ext::shared_ptr<Fdm1dMesher> yMesher(
            new ExponentialJump1dMesher(yGrid_, 
                                        process_->beta(), 
                                        process_->jumpIntensity(),
                                        process_->eta()));

        const ext::shared_ptr<FdmMesher> mesher(
            new FdmMesherComposite(xMesher, yMesher));

        // 2. Calculator
        const ext::shared_ptr<FdmInnerValueCalculator> calculator(
            new FdmExtOUJumpModelInnerValue(arguments_.payoff, mesher, shape_));

        // 3. Step conditions
        const ext::shared_ptr<FdmStepConditionComposite> conditions =
            FdmStepConditionComposite::vanillaComposite(
                                DividendSchedule(), arguments_.exercise, 
                                mesher, calculator, 
                                rTS_->referenceDate(), rTS_->dayCounter());

        // 4. Boundary conditions
        const FdmBoundaryConditionSet boundaries;
        
        // 5. set-up solver
        FdmSolverDesc solverDesc = { mesher, boundaries, conditions,
                                    calculator, maturity, tGrid_, 0 };

        const ext::shared_ptr<FdmExtOUJumpSolver> solver(
            new FdmExtOUJumpSolver(Handle<ExtOUWithJumpsProcess>(process_), 
                                   rTS_, solverDesc, schemeDesc_));
      
        const Real x = process_->initialValues()[0];
        const Real y = process_->initialValues()[1];
        results_.value = solver->valueAt(x, y);      
    }
}
