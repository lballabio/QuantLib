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

/*! \file fdsimpleklugeouvppengine.cpp
    \brief Finite Differences engine for simple vpp options
*/


#include <ql/instruments/basketoption.hpp>
#include <ql/instruments/vanillaswingoption.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>
#include <ql/experimental/processes/klugeextouprocess.hpp>
#include <ql/experimental/processes/extouwithjumpsprocess.hpp>
#include <ql/experimental/processes/extendedornsteinuhlenbeckprocess.hpp>
#include <ql/methods/finitedifferences/meshers/fdm1dmesher.hpp>
#include <ql/methods/finitedifferences/meshers/fdmmeshercomposite.hpp>
#include <ql/methods/finitedifferences/meshers/uniform1dmesher.hpp>
#include <ql/methods/finitedifferences/solvers/fdmsolverdesc.hpp>
#include <ql/methods/finitedifferences/operators/fdmlinearoplayout.hpp>
#include <ql/experimental/finitedifferences/fdmklugeextousolver.hpp>
#include <ql/methods/finitedifferences/meshers/exponentialjump1dmesher.hpp>
#include <ql/experimental/finitedifferences/fdmvppstepconditionfactory.hpp>
#include <ql/experimental/finitedifferences/fdsimpleklugeextouvppengine.hpp>
#include <ql/methods/finitedifferences/stepconditions/fdmstepconditioncomposite.hpp>
#include <ql/methods/finitedifferences/meshers/fdmsimpleprocess1dmesher.hpp>
#include <ql/experimental/finitedifferences/fdmexpextouinnervaluecalculator.hpp>

#include <list>

namespace QuantLib {

    namespace {
        class FdmSparkSpreadInnerValue : public FdmInnerValueCalculator {

          public:
            FdmSparkSpreadInnerValue(
                const ext::shared_ptr<BasketPayoff>& basketPayoff,
                const ext::shared_ptr<FdmInnerValueCalculator>& fuelPrice,
                const ext::shared_ptr<FdmInnerValueCalculator>& powerPrice)
            : basketPayoff_(basketPayoff),
              fuelPrice_(fuelPrice),
              powerPrice_(powerPrice) { }

            Real innerValue(const FdmLinearOpIterator& iter, Time t) {
                Array s(2);
                s[0] = powerPrice_->innerValue(iter, t);
                s[1] = fuelPrice_->innerValue(iter, t);

                return (*basketPayoff_)(s);
            }
            Real avgInnerValue(const FdmLinearOpIterator& iter, Time t) {
                return innerValue(iter, t);
            }

          private:
            const ext::shared_ptr<BasketPayoff> basketPayoff_;
            const ext::shared_ptr<FdmInnerValueCalculator> fuelPrice_;
            const ext::shared_ptr<FdmInnerValueCalculator> powerPrice_;
        };
    }


    FdSimpleKlugeExtOUVPPEngine::FdSimpleKlugeExtOUVPPEngine(
        const ext::shared_ptr<KlugeExtOUProcess>& process,
        const ext::shared_ptr<YieldTermStructure>& rTS,
        const ext::shared_ptr<Shape>& fuelShape,
        const ext::shared_ptr<Shape>& powerShape,
        Real fuelCostAddon,
        Size tGrid, Size xGrid, Size yGrid, Size gGrid,
        const FdmSchemeDesc& schemeDesc)
    : process_      (process),
      rTS_          (rTS),
      fuelCostAddon_(fuelCostAddon),
      fuelShape_     (fuelShape),
      powerShape_   (powerShape),
      tGrid_        (tGrid),
      xGrid_        (xGrid),
      yGrid_        (yGrid),
      gGrid_        (gGrid),
      schemeDesc_   (schemeDesc) {
    }

    void FdSimpleKlugeExtOUVPPEngine::calculate() const {

        ext::shared_ptr<SwingExercise> swingExercise(
            ext::dynamic_pointer_cast<SwingExercise>(arguments_.exercise));

        QL_REQUIRE(swingExercise, "Swing exercise supported only");

        const FdmVPPStepConditionFactory stepConditionFactory(arguments_);

        // 1. Exercise definition
        const std::vector<Time> exerciseTimes
            = swingExercise->exerciseTimes(rTS_->dayCounter(),
                                           rTS_->referenceDate());

        // 2. mesher set-up
        const Time maturity = exerciseTimes.back();
        const ext::shared_ptr<ExtOUWithJumpsProcess> klugeProcess
            = process_->getKlugeProcess();

        const ext::shared_ptr<StochasticProcess1D> klugeOUProcess
            = klugeProcess->getExtendedOrnsteinUhlenbeckProcess();

        const ext::shared_ptr<Fdm1dMesher> xMesher(
            new FdmSimpleProcess1dMesher(xGrid_, klugeOUProcess, maturity));

        const ext::shared_ptr<Fdm1dMesher> yMesher(
            new ExponentialJump1dMesher(yGrid_,
                                        klugeProcess->beta(),
                                        klugeProcess->jumpIntensity(),
                                        klugeProcess->eta(), 1e-3));

        const ext::shared_ptr<Fdm1dMesher> gMesher(
            new FdmSimpleProcess1dMesher(gGrid_,
                                         process_->getExtOUProcess(),maturity));

        const ext::shared_ptr<Fdm1dMesher> exerciseMesher(
            stepConditionFactory.stateMesher());

        const ext::shared_ptr<FdmMesher> mesher (
            new FdmMesherComposite(xMesher, yMesher, gMesher, exerciseMesher));

        // 3. Calculator
        const ext::shared_ptr<FdmInnerValueCalculator> zeroInnerValue(
            new FdmZeroInnerValue());

        const ext::shared_ptr<Payoff> zeroStrikeCall(
            new PlainVanillaPayoff(Option::Call, 0.0));

        const ext::shared_ptr<FdmInnerValueCalculator> fuelPrice(
            new FdmExpExtOUInnerValueCalculator(zeroStrikeCall,
                                                mesher, fuelShape_, 2));

        const ext::shared_ptr<FdmInnerValueCalculator> powerPrice(
            new FdmExtOUJumpModelInnerValue(zeroStrikeCall,mesher,powerShape_));

        const ext::shared_ptr<FdmInnerValueCalculator> sparkSpread(
            new FdmSparkSpreadInnerValue(
                ext::dynamic_pointer_cast<BasketPayoff>(arguments_.payoff),
                fuelPrice, powerPrice));

        // 4. Step conditions
        std::list<std::vector<Time> > stoppingTimes;
        std::list<ext::shared_ptr<StepCondition<Array> > > stepConditions;

        // 4.1 Bermudan step conditions
        stoppingTimes.push_back(exerciseTimes);
        const FdmVPPStepConditionMesher mesh = {3U, mesher};

        const ext::shared_ptr<FdmVPPStepCondition> stepCondition(
            stepConditionFactory.build(mesh, fuelCostAddon_,
                                       fuelPrice, sparkSpread));

        stepConditions.push_back(stepCondition);

        const ext::shared_ptr<FdmStepConditionComposite> conditions(
            new FdmStepConditionComposite(stoppingTimes, stepConditions));

        // 5. Boundary conditions
        const FdmBoundaryConditionSet boundaries;

        // 6. set-up solver
        FdmSolverDesc solverDesc = { mesher, boundaries, conditions,
                                     zeroInnerValue, maturity, tGrid_, 0 };

        const ext::shared_ptr<FdmKlugeExtOUSolver<4> > solver(
            new FdmKlugeExtOUSolver<4>(Handle<KlugeExtOUProcess>(process_),
                                       rTS_, solverDesc, schemeDesc_));

        std::vector<Real> x(4);
        x[0] = process_->initialValues()[0];
        x[1] = process_->initialValues()[1];
        x[2] = process_->initialValues()[2];
        
        const Real tol = 1e-8;
        const Real maxExerciseValue = exerciseMesher->locations().back();
        const Real minExerciseValue = exerciseMesher->locations().front();

        Array results(exerciseMesher->size());
        for (Size i=0; i < results.size(); ++i) {

            x[3] = std::max(minExerciseValue + tol,
                            std::min(exerciseMesher->location(i),
                                     maxExerciseValue - tol));
            results[i] = solver->valueAt(x);
        }
        results_.value = stepCondition->maxValue(results);
    }
}
