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
#include <ql/experimental/finitedifferences/fdmvppstepcondition.hpp>
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
                const boost::shared_ptr<BasketPayoff>& basketPayoff,
                const boost::shared_ptr<FdmInnerValueCalculator>& gasPrice,
                const boost::shared_ptr<FdmInnerValueCalculator>& powerPrice)
            : basketPayoff_(basketPayoff),
              gasPrice_(gasPrice),
              powerPrice_(powerPrice) { }

            Real innerValue(const FdmLinearOpIterator& iter, Time t) {
                Array s(2);
                s[0] = powerPrice_->innerValue(iter, t);
                s[1] = gasPrice_->innerValue(iter, t);

                return (*basketPayoff_)(s);
            }
            Real avgInnerValue(const FdmLinearOpIterator& iter, Time t) {
                return innerValue(iter, t);
            }

          private:
            const boost::shared_ptr<BasketPayoff> basketPayoff_;
            const boost::shared_ptr<FdmInnerValueCalculator> gasPrice_;
            const boost::shared_ptr<FdmInnerValueCalculator> powerPrice_;
        };
    }


    FdSimpleKlugeExtOUVPPEngine::FdSimpleKlugeExtOUVPPEngine(
        const boost::shared_ptr<KlugeExtOUProcess>& process,
        const boost::shared_ptr<YieldTermStructure>& rTS,
        Real carbonPrice,
        Size tGrid, Size xGrid, Size yGrid, Size gGrid,
        const boost::shared_ptr<Shape>& gasShape,
        const boost::shared_ptr<Shape>& powerShape,
        const FdmSchemeDesc& schemeDesc)
    : process_    (process),
      rTS_        (rTS),
      carbonPrice_(carbonPrice),
      gasShape_   (gasShape),
      powerShape_ (powerShape),
      tGrid_      (tGrid),
      xGrid_      (xGrid),
      yGrid_      (yGrid),
      gGrid_      (gGrid),
      schemeDesc_ (schemeDesc) {
    }

    void FdSimpleKlugeExtOUVPPEngine::calculate() const {

        boost::shared_ptr<SwingExercise> swingExercise(
            boost::dynamic_pointer_cast<SwingExercise>(arguments_.exercise));

        QL_REQUIRE(swingExercise, "Swing exercise supported only");

        const Size nStates = 2*arguments_.tMinUp + arguments_.tMinDown;

        // 1. Layout
        std::vector<Size> dim;
        dim.push_back(xGrid_);
        dim.push_back(yGrid_);
        dim.push_back(gGrid_);
        dim.push_back(nStates);
        const boost::shared_ptr<FdmLinearOpLayout> layout(
            new FdmLinearOpLayout(dim));

        const std::vector<Time> exerciseTimes
            = swingExercise->exerciseTimes(rTS_->dayCounter(),
                                           rTS_->referenceDate());

        // 2. mesher set-up
        const Time maturity = exerciseTimes.back();
        const boost::shared_ptr<ExtOUWithJumpsProcess> klugeProcess
            = process_->getKlugeProcess();

        const boost::shared_ptr<StochasticProcess1D> klugeOUProcess
            = klugeProcess->getExtendedOrnsteinUhlenbeckProcess();

        const boost::shared_ptr<Fdm1dMesher> xMesher(
            new FdmSimpleProcess1dMesher(xGrid_, klugeOUProcess, maturity));

        const boost::shared_ptr<Fdm1dMesher> yMesher(
            new ExponentialJump1dMesher(yGrid_,
                                        klugeProcess->beta(),
                                        klugeProcess->jumpIntensity(),
                                        klugeProcess->eta(), 1e-3));

        const boost::shared_ptr<Fdm1dMesher> gMesher(
            new FdmSimpleProcess1dMesher(gGrid_,
                                         process_->getExtOUProcess(),maturity));

        const boost::shared_ptr<Fdm1dMesher> exerciseMesher(
            new Uniform1dMesher(1.0, nStates, nStates));

        std::vector<boost::shared_ptr<Fdm1dMesher> > meshers;
        meshers.push_back(xMesher);
        meshers.push_back(yMesher);
        meshers.push_back(gMesher);
        meshers.push_back(exerciseMesher);
        const boost::shared_ptr<FdmMesher> mesher (
            new FdmMesherComposite(layout, meshers));

        // 3. Calculator
        const boost::shared_ptr<FdmInnerValueCalculator> zeroInnerValue(
            new FdmZeroInnerValue());

        const boost::shared_ptr<Payoff> zeroStrikeCall(
            new PlainVanillaPayoff(Option::Call, 0.0));

        const boost::shared_ptr<FdmInnerValueCalculator> gasPrice(
            new FdmExpExtOUInnerValueCalculator(zeroStrikeCall,
                                                mesher, gasShape_, 2));

        const boost::shared_ptr<FdmInnerValueCalculator> powerPrice(
            new FdmExtOUJumpModelInnerValue(zeroStrikeCall,mesher,powerShape_));

        const boost::shared_ptr<FdmInnerValueCalculator> sparkSpread(
            new FdmSparkSpreadInnerValue(
                boost::dynamic_pointer_cast<BasketPayoff>(arguments_.payoff),
                gasPrice, powerPrice));

        // 4. Step conditions
        std::list<std::vector<Time> > stoppingTimes;
        std::list<boost::shared_ptr<StepCondition<Array> > > stepConditions;

        // 4.1 Bermudan step conditions
        stoppingTimes.push_back(exerciseTimes);
        stepConditions.push_back(boost::shared_ptr<StepCondition<Array> >(
            new FdmVPPStepCondition(arguments_.heatRate,
                                    arguments_.pMin, arguments_.pMax,
                                    arguments_.tMinUp, arguments_.tMinDown,
                                    arguments_.startUpFuel,
                                    arguments_.startUpFixCost,
                                    carbonPrice_, 3,
                                    mesher, gasPrice, sparkSpread)));

        boost::shared_ptr<FdmStepConditionComposite> conditions(
                new FdmStepConditionComposite(stoppingTimes, stepConditions));

        // 5. Boundary conditions
        const std::vector<boost::shared_ptr<FdmDirichletBoundary> > boundaries;

        // 6. set-up solver
        FdmSolverDesc solverDesc = { mesher, boundaries, conditions,
                                     zeroInnerValue, maturity, tGrid_, 0 };

        const boost::shared_ptr<FdmKlugeExtOUSolver<4> > solver(
            new FdmKlugeExtOUSolver<4>(Handle<KlugeExtOUProcess>(process_),
                                       rTS_, solverDesc, schemeDesc_));

        std::vector<Real> x(4);
        x[0] = process_->initialValues()[0];
        x[1] = process_->initialValues()[1];
        x[2] = process_->initialValues()[2];
        x[3] = (Real) arguments_.initialState;

        results_.value = solver->valueAt(x);
    }
}
