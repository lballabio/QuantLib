/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Andreas Gaida
 Copyright (C) 2008, 2009 Ralph Schreyer
 Copyright (C) 2008, 2009 Klaus Spanderen

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
#include <ql/processes/blackscholesprocess.hpp>
#include <ql/experimental/finitedifferences/fdmamericanstepcondition.hpp>
#include <ql/experimental/finitedifferences/fdmdividendhandler.hpp>
#include <ql/experimental/finitedifferences/fdmblackscholessolver.hpp>
#include <ql/experimental/finitedifferences/fdminnervaluecalculator.hpp>
#include <ql/experimental/finitedifferences/fdmlinearoplayout.hpp>
#include <ql/experimental/finitedifferences/fdmmeshercomposite.hpp>
#include <ql/experimental/finitedifferences/fdmblackscholesmesher.hpp>
#include <ql/experimental/finitedifferences/fdblackscholesvanillaengine.hpp>
#include <ql/experimental/finitedifferences/fdmstepconditioncomposite.hpp>
#include <ql/experimental/finitedifferences/fdmbermudanstepcondition.hpp>

namespace QuantLib {

    FdBlackScholesVanillaEngine::FdBlackScholesVanillaEngine(
            const boost::shared_ptr<GeneralizedBlackScholesProcess>& process,
            Size tGrid, Size xGrid, Size dampingSteps, 
            const FdmSchemeDesc& schemeDesc,
            bool localVol, Real illegalLocalVolOverwrite)
    : GenericEngine<DividendVanillaOption::arguments,
                    DividendVanillaOption::results>(),
      process_(process),
      tGrid_(tGrid), xGrid_(xGrid), dampingSteps_(dampingSteps),
      schemeDesc_(schemeDesc), 
      localVol_(localVol),
      illegalLocalVolOverwrite_(illegalLocalVolOverwrite) {
    }

    void FdBlackScholesVanillaEngine::calculate() const {

        // 1. Layout
        std::vector<Size> dim;
        dim.push_back(xGrid_);
        const boost::shared_ptr<FdmLinearOpLayout> layout(
                                              new FdmLinearOpLayout(dim));

        const boost::shared_ptr<StrikedTypePayoff> payoff =
            boost::dynamic_pointer_cast<StrikedTypePayoff>(arguments_.payoff);

        // 2. Mesher
        const Time maturity = process_->time(arguments_.exercise->lastDate());
        const boost::shared_ptr<Fdm1dMesher> equityMesher(
            new FdmBlackScholesMesher(
                    xGrid_, process_, maturity, payoff->strike(), 
                    Null<Real>(), Null<Real>(), 0.0001, 1.5, 
                    std::pair<Real, Real>(payoff->strike(), 0.1)));
        
        std::vector<boost::shared_ptr<Fdm1dMesher> > meshers;
        meshers.push_back(equityMesher);
        boost::shared_ptr<FdmMesher> mesher (
                                     new FdmMesherComposite(layout, meshers));
        

        // 3. Calculator
        boost::shared_ptr<FdmInnerValueCalculator> calculator(
                                      new FdmLogInnerValue(payoff, mesher, 0));

        // 4. Step conditions
        std::list<boost::shared_ptr<StepCondition<Array> > > stepConditions;
        std::list<std::vector<Time> > stoppingTimes;

        // 4.1 Step condition if discrete dividends
        if(!arguments_.cashFlow.empty()) {
            boost::shared_ptr<FdmDividendHandler> dividendCondition(
                new FdmDividendHandler(arguments_.cashFlow, mesher,
                                       process_->riskFreeRate()->referenceDate(),
                                       process_->riskFreeRate()->dayCounter(),
                                       0));
            stepConditions.push_back(dividendCondition);
            stoppingTimes.push_back(dividendCondition->dividendTimes());
        }

        // 4.2 Step condition if american or bermudan exercise
        QL_REQUIRE(   arguments_.exercise->type() == Exercise::American
                   || arguments_.exercise->type() == Exercise::European
                   || arguments_.exercise->type() == Exercise::Bermudan,
                   "exercise type is not supported");
        if (arguments_.exercise->type() == Exercise::American) {
            stepConditions.push_back(boost::shared_ptr<StepCondition<Array> >(
                            new FdmAmericanStepCondition(mesher,calculator)));
        }
        else if (arguments_.exercise->type() == Exercise::Bermudan) {
            boost::shared_ptr<FdmBermudanStepCondition> bermudanCondition(
                new FdmBermudanStepCondition(
                                    arguments_.exercise->dates(),
                                    process_->riskFreeRate()->referenceDate(),
                                    process_->riskFreeRate()->dayCounter(),
                                    mesher, calculator));
            stepConditions.push_back(bermudanCondition);
            stoppingTimes.push_back(bermudanCondition->exerciseTimes());
        }

        boost::shared_ptr<FdmStepConditionComposite> conditions(
                new FdmStepConditionComposite(stoppingTimes, stepConditions));

        // 5. Boundary conditions
        std::vector<boost::shared_ptr<FdmDirichletBoundary> > boundaries;

        // 6. Solver
        boost::shared_ptr<FdmBlackScholesSolver> solver(
                new FdmBlackScholesSolver(
                             Handle<GeneralizedBlackScholesProcess>(process_),
                             mesher, boundaries, conditions, calculator,
                             payoff->strike(), maturity, tGrid_, 
                             dampingSteps_, schemeDesc_, 
                             localVol_, illegalLocalVolOverwrite_));

        const Real spot = process_->x0();
        results_.value = solver->valueAt(spot);
        results_.delta = solver->deltaAt(spot);
        results_.gamma = solver->gammaAt(spot);
        results_.theta = solver->thetaAt(spot);
    }
}
