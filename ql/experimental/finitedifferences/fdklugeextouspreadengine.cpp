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


#include <ql/exercise.hpp>
#include <ql/experimental/finitedifferences/fdklugeextouspreadengine.hpp>
#include <ql/experimental/finitedifferences/fdmklugeextousolver.hpp>
#include <ql/experimental/finitedifferences/fdmspreadpayoffinnervalue.hpp>
#include <ql/experimental/processes/extendedornsteinuhlenbeckprocess.hpp>
#include <ql/experimental/processes/extouwithjumpsprocess.hpp>
#include <ql/experimental/processes/klugeextouprocess.hpp>
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

    FdKlugeExtOUSpreadEngine::FdKlugeExtOUSpreadEngine(
        std::shared_ptr<KlugeExtOUProcess> klugeOUProcess,
        std::shared_ptr<YieldTermStructure> rTS,
        Size tGrid,
        Size xGrid,
        Size yGrid,
        Size uGrid,
        std::shared_ptr<GasShape> gasShape,
        std::shared_ptr<PowerShape> powerShape,
        const FdmSchemeDesc& schemeDesc)
    : klugeOUProcess_(std::move(klugeOUProcess)), rTS_(std::move(rTS)), tGrid_(tGrid),
      xGrid_(xGrid), yGrid_(yGrid), uGrid_(uGrid), gasShape_(std::move(gasShape)),
      powerShape_(std::move(powerShape)), schemeDesc_(schemeDesc) {}

    void FdKlugeExtOUSpreadEngine::calculate() const {
        // 1. Mesher
        const Time maturity
            = rTS_->dayCounter().yearFraction(rTS_->referenceDate(),
                                              arguments_.exercise->lastDate());
        const std::shared_ptr<ExtOUWithJumpsProcess> klugeProcess
                                          = klugeOUProcess_->getKlugeProcess();
        const std::shared_ptr<StochasticProcess1D> ouProcess
                        = klugeProcess->getExtendedOrnsteinUhlenbeckProcess();
        const std::shared_ptr<Fdm1dMesher> xMesher(
            new FdmSimpleProcess1dMesher(xGrid_, ouProcess,maturity));

        const std::shared_ptr<Fdm1dMesher> yMesher(
            new ExponentialJump1dMesher(yGrid_,
                                        klugeProcess->beta(),
                                        klugeProcess->jumpIntensity(),
                                        klugeProcess->eta()));

        const std::shared_ptr<Fdm1dMesher> uMesher(
            new FdmSimpleProcess1dMesher(uGrid_,
                                         klugeOUProcess_->getExtOUProcess(),
                                         maturity));

        const std::shared_ptr<FdmMesher> mesher(
            new FdmMesherComposite(xMesher, yMesher, uMesher));

        // 2. Calculator
        std::shared_ptr<BasketPayoff> basketPayoff =
            std::dynamic_pointer_cast<BasketPayoff>(arguments_.payoff);
        QL_REQUIRE(basketPayoff," basket payoff expected");

        const std::shared_ptr<Payoff> zeroStrikeCall(
            new PlainVanillaPayoff(Option::Call, 0.0));

        const std::shared_ptr<FdmInnerValueCalculator> gasPrice(
            new FdmExpExtOUInnerValueCalculator(zeroStrikeCall,
                                                mesher, gasShape_, 2));

        const std::shared_ptr<FdmInnerValueCalculator> powerPrice(
            new FdmExtOUJumpModelInnerValue(zeroStrikeCall,mesher,powerShape_));

        const std::shared_ptr<FdmInnerValueCalculator> calculator(
            new FdmSpreadPayoffInnerValue(basketPayoff, powerPrice, gasPrice));

        // 3. Step conditions
        const std::shared_ptr<FdmStepConditionComposite> conditions =
            FdmStepConditionComposite::vanillaComposite(
                                DividendSchedule(), arguments_.exercise,
                                mesher, calculator,
                                rTS_->referenceDate(), rTS_->dayCounter());

        // 4. Boundary conditions
        const FdmBoundaryConditionSet boundaries;

        // 5. set-up solver
        FdmSolverDesc solverDesc = { mesher, boundaries, conditions,
                                     calculator, maturity, tGrid_, 0 };

        const std::shared_ptr<FdmKlugeExtOUSolver<3> > solver(
            new FdmKlugeExtOUSolver<3>(
                Handle<KlugeExtOUProcess>(klugeOUProcess_),
                rTS_, solverDesc, schemeDesc_));

        std::vector<Real> x(3);
        x[0] = klugeOUProcess_->initialValues()[0];
        x[1] = klugeOUProcess_->initialValues()[1];
        x[2] = klugeOUProcess_->initialValues()[2];

        results_.value = solver->valueAt(x);
    }
}
