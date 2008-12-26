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

#include <ql/experimental/finitedifferences/fdblackscholesvanillaengine.hpp>
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

    FdBlackScholesVanillaEngine::FdBlackScholesVanillaEngine(
            const boost::shared_ptr<GeneralizedBlackScholesProcess>& process,
            Size tGrid, Size xGrid, Real theta)
    : GenericEngine<DividendVanillaOption::arguments,
                    DividendVanillaOption::results>(),
      process_(process), tGrid_(tGrid), xGrid_(xGrid), theta_(theta) {
    }

    void FdBlackScholesVanillaEngine::calculate() const {

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
        const Real xMin = std::log(F) - sigmaSqrtT*normInvEps*2.0
                                      - sigmaSqrtT*sigmaSqrtT/2.0;
        const Real xMax = std::log(F) + sigmaSqrtT*normInvEps*2.0
                                      - sigmaSqrtT*sigmaSqrtT/2.0;

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
        QL_REQUIRE(   arguments_.exercise->type() == Exercise::American
                   || arguments_.exercise->type() == Exercise::European,
                   "exercise type is not supported");
        if (arguments_.exercise->type() == Exercise::American) {
            boost::shared_ptr<FdmInnerValueCalculator> calculator(
                                            new FdmLogInnerValue(payoff, 0));
            stepConditions.push_back(boost::shared_ptr<StepCondition<Array> >(
                            new FdmAmericanStepCondition(mesher,calculator)));
        }

        boost::shared_ptr<FdmStepConditionComposite> conditions(
                new FdmStepConditionComposite(stoppingTimes, stepConditions));

        // 4. Boundary conditions
        std::vector<boost::shared_ptr<FdmDirichletBoundary> > boundaries;

        // 5. Solver
        boost::shared_ptr<FdmBlackScholesSolver> solver(
                new FdmBlackScholesSolver(
                                Handle<GeneralizedBlackScholesProcess>(process_),
                                mesher, boundaries, conditions,
                                payoff, maturity, tGrid_, theta_));

        results_.value = solver->valueAt(spot);
        results_.delta = solver->deltaAt(spot);
        results_.gamma = solver->gammaAt(spot);
        results_.theta = solver->thetaAt(spot);
    }
}
