/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Andreas Gaida
 Copyright (C) 2008 Ralph Schreyer
 Copyright (C) 2008 Klaus Spanderen

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

#include <ql/experimental/finitedifferences/fdblackscholesrebateengine.hpp>
#include <ql/exercise.hpp>
#include <ql/math/distributions/normaldistribution.hpp>
#include <ql/experimental/finitedifferences/fdmamericanstepcondition.hpp>
#include <ql/experimental/finitedifferences/fdmdividendhandler.hpp>
#include <ql/experimental/finitedifferences/fdmblackscholessolver.hpp>
#include <ql/experimental/finitedifferences/fdminnervaluecalculator.hpp>
#include <ql/experimental/finitedifferences/fdmlinearoplayout.hpp>
#include <ql/experimental/finitedifferences/fdmmeshercomposite.hpp>
#include <ql/experimental/finitedifferences/uniform1dmesher.hpp>
#include <ql/time/daycounters/actualactual.hpp>

namespace QuantLib {

    FdBlackScholesRebateEngine::FdBlackScholesRebateEngine(
            const boost::shared_ptr<GeneralizedBlackScholesProcess>& process,
            Size tGrid, Size xGrid, Real theta)
    : GenericEngine<DividendBarrierOption::arguments,
                    DividendBarrierOption::results>(),
      process_(process), tGrid_(tGrid), xGrid_(xGrid), theta_(theta) {
    }

    void FdBlackScholesRebateEngine::calculate() const {

        // 1. Layout
        std::vector<Size> dim;
        dim.push_back(xGrid_);
        const boost::shared_ptr<FdmLinearOpLayout> layout(
                                              new FdmLinearOpLayout(dim));

        // 2. Mesher
        // Calculate the forward
        const boost::shared_ptr<StrikedTypePayoff> payoff =
            boost::dynamic_pointer_cast<StrikedTypePayoff>(arguments_.payoff);
        const Time maturity = process_->time(arguments_.exercise->lastDate());
        const Real spot = process_->x0();
        QL_REQUIRE(spot > 0.0, "negative or null underlying given");
        Real F = spot*process_->dividendYield()->discount(maturity)
                     /process_->riskFreeRate()->discount(maturity);
        std::vector<Date> dividendDates;
        std::vector<Time> dividendTimes;
        std::vector<Real> dividends;
        if(!arguments_.cashFlow.empty()) {
            Date todaysDate = Date::todaysDate();
            DayCounter dayCounter = ActualActual();
            for (DividendSchedule::const_iterator iter
                                = arguments_.cashFlow.begin();
                                iter != arguments_.cashFlow.end(); ++iter) {
                dividendDates.push_back((*iter)->date());
                dividendTimes.push_back(dayCounter.yearFraction(
                        Settings::instance().evaluationDate(),(*iter)->date()));
                dividends.push_back((*iter)->amount());
                F -= dividends.back()*process_->riskFreeRate()->discount(
                                                        dividendTimes.back());
            }
        }
        QL_REQUIRE(F > 0.0, "negative forward given");

        // Set the grid boundaries
        const Real normInvEps = fabs(InverseCumulativeNormal()(0.0001));
        const Real sigmaSqrtT =  process_->blackVolatility()->blackVol(
                            maturity, payoff->strike())*std::sqrt(maturity);
        Real xMin = std::log(F) - sigmaSqrtT*normInvEps*2.0
                                - sigmaSqrtT*sigmaSqrtT/2.0;
        Real xMax = std::log(F) + sigmaSqrtT*normInvEps*2.0
                                - sigmaSqrtT*sigmaSqrtT/2.0;
        if (   arguments_.barrierType == Barrier::DownIn
            || arguments_.barrierType == Barrier::DownOut) {
            xMin = std::log(arguments_.barrier);
        }
        if (   arguments_.barrierType == Barrier::UpIn
            || arguments_.barrierType == Barrier::UpOut) {
            xMax = std::log(arguments_.barrier);
        }

        const boost::shared_ptr<Fdm1dMesher> equityMesher(
                            new Uniform1dMesher(xMin, xMax, layout->dim()[0]));

        std::vector<boost::shared_ptr<Fdm1dMesher> > meshers;
        meshers.push_back(equityMesher);
        boost::shared_ptr<FdmMesher> mesher (new FdmMesherComposite(
                                                            layout, meshers));

        // 3. Step conditions
        std::list<boost::shared_ptr<StepCondition<Array> > > stepConditions;
        std::list<std::vector<Time> > stoppingTimes;

        // 3.1 Step condition if discrete dividends
        if(!arguments_.cashFlow.empty()) {
            boost::shared_ptr<StepCondition<Array> > dividendCondition(
                new FdmDividendHandler(dividendTimes, dividends, mesher, 0));
            stepConditions.push_back(dividendCondition);
            stoppingTimes.push_back(dividendTimes);
        }

        // 3.2 Step condition if american exercise
        boost::shared_ptr<StrikedTypePayoff> rebatePayoff(
                new CashOrNothingPayoff(Option::Call, 0.0, arguments_.rebate));
        if (arguments_.exercise->type() == Exercise::American) {
            boost::shared_ptr<FdmInnerValueCalculator> calculator(
                                    new FdmLogInnerValue(rebatePayoff, 0));
            stepConditions.push_back(boost::shared_ptr<StepCondition<Array> >(
                            new FdmAmericanStepCondition(mesher,calculator)));
        }

        boost::shared_ptr<FdmStepConditionComposite> conditions(
                new FdmStepConditionComposite(stoppingTimes, stepConditions));

        // 4. Boundary conditions
        std::vector<boost::shared_ptr<FdmDirichletBoundary> > boundaries;
        if (   arguments_.barrierType == Barrier::DownIn
            || arguments_.barrierType == Barrier::DownOut) {
            boundaries.push_back(boost::shared_ptr<FdmDirichletBoundary>(
                new FdmDirichletBoundary(layout, arguments_.rebate, 0,
                                         FdmDirichletBoundary::Lower)));

        }
        if (   arguments_.barrierType == Barrier::UpIn
            || arguments_.barrierType == Barrier::UpOut) {
            boundaries.push_back(boost::shared_ptr<FdmDirichletBoundary>(
                new FdmDirichletBoundary(layout, arguments_.rebate, 0,
                                         FdmDirichletBoundary::Upper)));
        }

        // 5. Solver
        boost::shared_ptr<FdmBlackScholesSolver> solver(
                new FdmBlackScholesSolver(
                                Handle<GeneralizedBlackScholesProcess>(process_),
                                mesher, boundaries, conditions,
                                rebatePayoff, maturity, tGrid_, theta_));

        results_.value = solver->valueAt(spot);
        results_.delta = solver->deltaAt(spot);
        results_.gamma = solver->gammaAt(spot);
        results_.theta = solver->thetaAt(spot);
    }
}
