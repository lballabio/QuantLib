/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2009 Ralph Schreyer

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
#include <ql/math/distributions/normaldistribution.hpp>
#include <ql/methods/finitedifferences/meshers/fdmblackscholesmesher.hpp>
#include <ql/methods/finitedifferences/meshers/fdmmeshercomposite.hpp>
#include <ql/methods/finitedifferences/operators/fdmlinearoplayout.hpp>
#include <ql/methods/finitedifferences/solvers/fdmsimple2dbssolver.hpp>
#include <ql/methods/finitedifferences/stepconditions/fdmarithmeticaveragecondition.hpp>
#include <ql/methods/finitedifferences/stepconditions/fdmstepconditioncomposite.hpp>
#include <ql/methods/finitedifferences/utilities/fdminnervaluecalculator.hpp>
#include <ql/pricingengines/asian/fdblackscholesasianengine.hpp>
#include <ql/processes/blackscholesprocess.hpp>
#include <utility>

namespace QuantLib {


    FdBlackScholesAsianEngine::FdBlackScholesAsianEngine(
        std::shared_ptr<GeneralizedBlackScholesProcess> process,
        Size tGrid,
        Size xGrid,
        Size aGrid,
        const FdmSchemeDesc& schemeDesc)
    : process_(std::move(process)), tGrid_(tGrid), xGrid_(xGrid), aGrid_(aGrid),
      schemeDesc_(schemeDesc) {}


    void FdBlackScholesAsianEngine::calculate() const {

        QL_REQUIRE(arguments_.exercise->type() == Exercise::European,
                   "European exercise supported only");
        QL_REQUIRE(arguments_.averageType == Average::Arithmetic,
                   "Arithmetic averaging supported only");
        QL_REQUIRE(   arguments_.runningAccumulator == 0
                   || arguments_.pastFixings > 0,
                   "Running average requires at least one past fixing");

        // 1. Mesher
        const std::shared_ptr<StrikedTypePayoff> payoff =
            std::dynamic_pointer_cast<StrikedTypePayoff>(arguments_.payoff);
        const Time maturity = process_->time(arguments_.exercise->lastDate());
        const std::shared_ptr<Fdm1dMesher> equityMesher(
            new FdmBlackScholesMesher(xGrid_, process_, maturity,
                                      payoff->strike()));

        const Real spot = process_->x0();
        QL_REQUIRE(spot > 0.0, "negative or null underlying given");

        const Real avg = (arguments_.runningAccumulator == 0)
                 ? spot : arguments_.runningAccumulator/arguments_.pastFixings;

        const Real normInvEps = InverseCumulativeNormal()(1-0.0001);
        const Real sigmaSqrtT 
            = process_->blackVolatility()->blackVol(maturity, payoff->strike())
                                                        *std::sqrt(maturity);
        const Real r = sigmaSqrtT*normInvEps;

        Real xMin = std::min(std::log(avg)  - 0.25*r, std::log(spot) - 1.5*r);
        Real xMax = std::max(std::log(avg)  + 0.25*r, std::log(spot) + 1.5*r);

        const std::shared_ptr<Fdm1dMesher> averageMesher(
            new FdmBlackScholesMesher(aGrid_, process_, maturity,
                                      payoff->strike(), xMin, xMax));

        const std::shared_ptr<FdmMesher> mesher (
            new FdmMesherComposite(equityMesher, averageMesher));

        // 2. Calculator
        std::shared_ptr<FdmInnerValueCalculator> calculator(
                                new FdmLogInnerValue(payoff, mesher, 1));

        // 3. Step conditions
        std::list<std::shared_ptr<StepCondition<Array> > > stepConditions;
        std::list<std::vector<Time> > stoppingTimes;

        // 3.1 Arithmetic average step conditions
        std::vector<Time> averageTimes;
        for (auto& fixingDate : arguments_.fixingDates) {
            Time t = process_->time(fixingDate);
            QL_REQUIRE(t >= 0, "Fixing dates must not contain past date");
            averageTimes.push_back(t);
        }
        stoppingTimes.emplace_back(averageTimes);
        stepConditions.push_back(std::shared_ptr<StepCondition<Array> >(
                new FdmArithmeticAverageCondition(
                        averageTimes, arguments_.runningAccumulator,
                        arguments_.pastFixings, mesher, 0)));

        std::shared_ptr<FdmStepConditionComposite> conditions(
                new FdmStepConditionComposite(stoppingTimes, stepConditions));

        // 4. Boundary conditions
        const FdmBoundaryConditionSet boundaries;

        // 5. Solver
        FdmSolverDesc solverDesc = { mesher, boundaries, conditions,
                                     calculator, maturity, tGrid_, 0 };
        std::shared_ptr<FdmSimple2dBSSolver> solver(
              new FdmSimple2dBSSolver(
                              Handle<GeneralizedBlackScholesProcess>(process_),
                              payoff->strike(), solverDesc, schemeDesc_));

        results_.value = solver->valueAt(spot, avg);
        results_.delta = solver->deltaAt(spot, avg, spot*0.01);
        results_.gamma = solver->gammaAt(spot, avg, spot*0.01);
    }
}
