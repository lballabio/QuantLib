/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2009 Klaus Spanderen

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

#include <ql/math/distributions/normaldistribution.hpp>
#include <ql/pricingengines/vanilla/analytichestonengine.hpp>
#include <ql/experimental/finitedifferences/fdhestonhullwhitevanillaengine.hpp>
#include <ql/experimental/finitedifferences/fdmstepconditioncomposite.hpp>
#include <ql/experimental/finitedifferences/fdmamericanstepcondition.hpp>
#include <ql/experimental/finitedifferences/fdmdividendhandler.hpp>
#include <ql/experimental/finitedifferences/uniform1dmesher.hpp>
#include <ql/experimental/finitedifferences/fdmblackscholesmesher.hpp>
#include <ql/experimental/finitedifferences/fdmhestonvariancemesher.hpp>
#include <ql/experimental/finitedifferences/fdminnervaluecalculator.hpp>
#include <ql/experimental/finitedifferences/fdmlinearoplayout.hpp>
#include <ql/experimental/finitedifferences/fdmmeshercomposite.hpp>
#include <ql/experimental/finitedifferences/fdhestonvanillaengine.hpp>

namespace QuantLib {

    FdHestonHullWhiteVanillaEngine::FdHestonHullWhiteVanillaEngine(
            const boost::shared_ptr<HestonModel>& hestonModel,
            const boost::shared_ptr<HullWhiteProcess>& hwProcess,
            Real corrEquityShortRate,
            Size tGrid, Size xGrid, 
            Size vGrid, Size rGrid,
            bool controlVariate,
            FdmHestonHullWhiteSolver::FdmSchemeType type, 
            Real theta, Real mu)
    : GenericModelEngine<HestonModel,
                         DividendVanillaOption::arguments,
                         DividendVanillaOption::results>(hestonModel),
      hwProcess_(hwProcess),
      corrEquityShortRate_(corrEquityShortRate),
      tGrid_(tGrid), xGrid_(xGrid), 
      vGrid_(vGrid), rGrid_(rGrid),
      controlVariate_(controlVariate),
      type_(type), theta_(theta), mu_(mu) {
    }

    void FdHestonHullWhiteVanillaEngine::calculate() const {

        // 1. Layout
        std::vector<Size> dim;
        dim.push_back(xGrid_);
        dim.push_back(vGrid_);
        dim.push_back(rGrid_);
        const boost::shared_ptr<FdmLinearOpLayout> layout(
                                              new FdmLinearOpLayout(dim));

        // 2. Mesher
        const boost::shared_ptr<HestonProcess> hestonProcess=model_->process();
        const Time maturity=hestonProcess->time(arguments_.exercise->lastDate());

        // 2.1 The variance mesher
        const Size tGridMin = 10;
        const boost::shared_ptr<FdmHestonVarianceMesher> varianceMesher(
            new FdmHestonVarianceMesher(layout->dim()[1], hestonProcess, 
                                        maturity,std::max(tGridMin, tGrid_/5)));

        // 2.2 The equity mesher
        const boost::shared_ptr<StrikedTypePayoff> payoff =
            boost::dynamic_pointer_cast<StrikedTypePayoff>(arguments_.payoff);

        const boost::shared_ptr<Fdm1dMesher> equityMesher(
            new FdmBlackScholesMesher(
                FdmBlackScholesMesher::processHelper(
                    hestonProcess->s0(), hestonProcess->dividendYield(), 
                    hestonProcess->riskFreeRate(), 
                    varianceMesher->volaEstimate()),
                 layout, 0, maturity,
                 payoff->strike(), arguments_.cashFlow));
       
        //2.3 The short rate mesher
        const Rate r0    = hwProcess_->x0();
        const Rate r_exp = hwProcess_->expectation (0, r0, maturity);
        const Rate dev =  InverseCumulativeNormal()(1-0.0025)
                          *hwProcess_->stdDeviation(0, r0, maturity);
        
        const Rate rMin = ( r_exp-dev < r0-0.2*dev) ? r_exp-dev : r0-0.2*dev;
        const Rate rMax = ( r_exp+dev > r0+0.2*dev) ? r_exp+dev : r0+0.2*dev;
        
        const boost::shared_ptr<Fdm1dMesher> shortRateMesher(
                                      new Uniform1dMesher(rMin, rMax, rGrid_));
        
        std::vector<boost::shared_ptr<Fdm1dMesher> > meshers;
        meshers.push_back(equityMesher);
        meshers.push_back(varianceMesher);
        meshers.push_back(shortRateMesher);
        boost::shared_ptr<FdmMesher> mesher(
                                     new FdmMesherComposite(layout, meshers));
        
        // 3. Step conditions
        std::list<boost::shared_ptr<StepCondition<Array> > > stepConditions;
        std::list<std::vector<Time> > stoppingTimes;

        // 3.1 Step condition if discrete dividends
        if(!arguments_.cashFlow.empty()) {
            boost::shared_ptr<FdmDividendHandler> dividendCondition(
                new FdmDividendHandler(
                    arguments_.cashFlow, mesher,
                    hestonProcess->riskFreeRate()->referenceDate(),
                    hestonProcess->riskFreeRate()->dayCounter(), 0));
            stepConditions.push_back(dividendCondition);
            stoppingTimes.push_back(dividendCondition->dividendTimes());
        }

        // 3.2 Step condition if american exercise
        if (arguments_.exercise->type() == Exercise::American) {
            boost::shared_ptr<FdmInnerValueCalculator> calculator(
                                    new FdmLogInnerValue(arguments_.payoff, 0));
            stepConditions.push_back(boost::shared_ptr<StepCondition<Array> >(
                            new FdmAmericanStepCondition(mesher, calculator)));
        }

        boost::shared_ptr<FdmStepConditionComposite> conditions(
                new FdmStepConditionComposite(stoppingTimes, stepConditions));

        // 4. Boundary conditions
        std::vector<boost::shared_ptr<FdmDirichletBoundary> > boundaries;

        // 5. Solver
        boost::shared_ptr<FdmHestonHullWhiteSolver> solver(
            new FdmHestonHullWhiteSolver(Handle<HestonProcess>(hestonProcess),
                                         Handle<HullWhiteProcess>(hwProcess_),
                                         corrEquityShortRate_,
                                         mesher, boundaries, conditions,
                                         arguments_.payoff, 
                                         maturity, tGrid_,
                                         type_, theta_, mu_));

        const Real spot = hestonProcess->s0()->value();
        const Real v0   = hestonProcess->v0();
        results_.value = solver->valueAt(spot, v0, r0);
        results_.delta = solver->deltaAt(spot, v0, r0, spot*0.01);
        results_.gamma = solver->gammaAt(spot, v0, r0, spot*0.01);
        results_.theta = solver->thetaAt(spot, v0, r0);
        
        if (controlVariate_) {
            VanillaOption option(payoff, 
                boost::shared_ptr<Exercise>(new EuropeanExercise(
                                            arguments_.exercise->lastDate())));
            
            option.setPricingEngine(boost::shared_ptr<PricingEngine>(
                                       new AnalyticHestonEngine(model_, 164)));
            const Real analyticNPV = option.NPV();
            
            FdmHestonSolver::FdmSchemeType hestonType;
            switch (type_) {
              case FdmHestonHullWhiteSolver::CraigSneydScheme:
                hestonType = FdmHestonSolver::CraigSneydScheme;
                break;
              case FdmHestonHullWhiteSolver::DouglasScheme:
                hestonType = FdmHestonSolver::DouglasScheme;
                break;
              case FdmHestonHullWhiteSolver::HundsdorferScheme:
                hestonType = FdmHestonSolver::HundsdorferScheme;
                break;
              default:
                QL_FAIL("Unknown scheme type");
            }
            option.setPricingEngine(boost::shared_ptr<PricingEngine>(
                 new FdHestonVanillaEngine(model_, tGrid_, xGrid_, vGrid_, 
                                           hestonType, theta_, mu_)));
            const Real fdNPV = option.NPV();
            results_.value += analyticNPV - fdNPV;
        }
    }
}
