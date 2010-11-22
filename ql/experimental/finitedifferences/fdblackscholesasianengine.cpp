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
#include <ql/time/daycounters/actualactual.hpp>
#include <ql/math/distributions/normaldistribution.hpp>
#include <ql/processes/blackscholesprocess.hpp>
#include <ql/experimental/finitedifferences/fdmarithmeticaveragecondition.hpp>
#include <ql/experimental/finitedifferences/fdblackscholesasianengine.hpp>
#include <ql/experimental/finitedifferences/fdmsimple2dbssolver.hpp>
#include <ql/experimental/finitedifferences/fdminnervaluecalculator.hpp>
#include <ql/experimental/finitedifferences/fdmlinearoplayout.hpp>
#include <ql/experimental/finitedifferences/fdmblackscholesmesher.hpp>
#include <ql/experimental/finitedifferences/fdmmeshercomposite.hpp>
#include <ql/experimental/finitedifferences/fdmstepconditioncomposite.hpp>

namespace QuantLib {


    FdBlackScholesAsianEngine::FdBlackScholesAsianEngine(
            const boost::shared_ptr<GeneralizedBlackScholesProcess>& process,
            Size tGrid, Size xGrid, Size aGrid, 
            const FdmSchemeDesc& schemeDesc)
    : GenericEngine<DiscreteAveragingAsianOption::arguments,
                    DiscreteAveragingAsianOption::results>(),
      process_(process), tGrid_(tGrid), xGrid_(xGrid), aGrid_(aGrid),
      schemeDesc_(schemeDesc) {}


    void FdBlackScholesAsianEngine::calculate() const {

        QL_REQUIRE(arguments_.exercise->type() == Exercise::European,
                   "European exercise supported only");
        QL_REQUIRE(arguments_.averageType == Average::Arithmetic,
                   "Arithmetic averaging supported only");
        QL_REQUIRE(   arguments_.runningAccumulator == 0
                   || arguments_.pastFixings > 0,
                   "Running average requires at least one past fixing");

        // 1. Layout
        std::vector<Size> dim;
        dim.push_back(xGrid_);
        dim.push_back(aGrid_);
        const boost::shared_ptr<FdmLinearOpLayout> layout(
                                              new FdmLinearOpLayout(dim));

        // 2. Mesher
        const boost::shared_ptr<StrikedTypePayoff> payoff =
            boost::dynamic_pointer_cast<StrikedTypePayoff>(arguments_.payoff);
        const Time maturity = process_->time(arguments_.exercise->lastDate());
        const boost::shared_ptr<Fdm1dMesher> equityMesher(
            new FdmBlackScholesMesher(xGrid_, process_, maturity,
                                      payoff->strike()));
        const boost::shared_ptr<Fdm1dMesher> averageMesher(
            new FdmBlackScholesMesher(aGrid_, process_, maturity,
                                      payoff->strike()));

        std::vector<boost::shared_ptr<Fdm1dMesher> > meshers;
        meshers.push_back(equityMesher);
        meshers.push_back(averageMesher);
        boost::shared_ptr<FdmMesher> mesher (
                                     new FdmMesherComposite(layout, meshers));

        // 3. Calculator
        boost::shared_ptr<FdmInnerValueCalculator> calculator(
                                new FdmLogInnerValue(payoff, mesher, 1));

        // 4. Step conditions
        std::list<boost::shared_ptr<StepCondition<Array> > > stepConditions;
        std::list<std::vector<Time> > stoppingTimes;

        // 4.1 Arithmetic average step conditions
        std::vector<Time> averageTimes;
        for (Size i=0; i<arguments_.fixingDates.size(); ++i) {
            Time t = process_->time(arguments_.fixingDates[i]);
            QL_REQUIRE(t >= 0, "Fixing dates must not contain past date");
            averageTimes.push_back(t);
        }
        stoppingTimes.push_back(std::vector<Time>(averageTimes));
        stepConditions.push_back(boost::shared_ptr<StepCondition<Array> >(
                new FdmArithmeticAverageCondition(
                        averageTimes, arguments_.runningAccumulator,
                        arguments_.pastFixings, mesher, 0)));

        boost::shared_ptr<FdmStepConditionComposite> conditions(
                new FdmStepConditionComposite(stoppingTimes, stepConditions));

        // 5. Boundary conditions
        std::vector<boost::shared_ptr<FdmDirichletBoundary> > boundaries;

        // 6. Solver
        boost::shared_ptr<FdmSimple2dBSSolver> solver(
                new FdmSimple2dBSSolver(
                                Handle<GeneralizedBlackScholesProcess>(process_),
                                mesher, boundaries, conditions, calculator,
                                payoff->strike(),maturity, tGrid_,
                                schemeDesc_));

        const Real spot = process_->x0();
        const Real avg = arguments_.runningAccumulator == 0
                 ? spot : arguments_.runningAccumulator/arguments_.pastFixings;
        results_.value = solver->valueAt(spot, avg);
        results_.delta = solver->deltaAt(spot, avg, spot*0.01);
        results_.gamma = solver->gammaAt(spot, avg, spot*0.01);
    }
}
