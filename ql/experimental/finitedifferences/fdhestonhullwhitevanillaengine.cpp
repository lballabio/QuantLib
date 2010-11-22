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

#include <ql/pricingengines/vanilla/analytichestonengine.hpp>
#include <ql/experimental/finitedifferences/fdhestonhullwhitevanillaengine.hpp>
#include <ql/experimental/finitedifferences/fdmstepconditioncomposite.hpp>
#include <ql/experimental/finitedifferences/fdmamericanstepcondition.hpp>
#include <ql/experimental/finitedifferences/fdmbermudanstepcondition.hpp>
#include <ql/experimental/finitedifferences/fdmdividendhandler.hpp>
#include <ql/experimental/finitedifferences/uniform1dmesher.hpp>
#include <ql/experimental/finitedifferences/fdmblackscholesmesher.hpp>
#include <ql/experimental/finitedifferences/fdmblackscholesmultistrikemesher.hpp>
#include <ql/experimental/finitedifferences/fdmhullwhitemesher.hpp>
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
            Size dampingSteps,
            bool controlVariate,
            const FdmSchemeDesc& schemeDesc)
    : GenericModelEngine<HestonModel,
                         DividendVanillaOption::arguments,
                         DividendVanillaOption::results>(hestonModel),
      hwProcess_(hwProcess),
      corrEquityShortRate_(corrEquityShortRate),
      tGrid_(tGrid), xGrid_(xGrid), 
      vGrid_(vGrid), rGrid_(rGrid),
      dampingSteps_(dampingSteps),
      schemeDesc_(schemeDesc),
      controlVariate_(controlVariate) {
    }

    void FdHestonHullWhiteVanillaEngine::calculate() const {
  
        // cache lookup for precalculated results
         for (Size i=0; i < cachedArgs2results_.size(); ++i) {
             if (   cachedArgs2results_[i].first.exercise->type() 
                         == arguments_.exercise->type()
                 && cachedArgs2results_[i].first.exercise->dates()
                         == arguments_.exercise->dates()) {
                 boost::shared_ptr<PlainVanillaPayoff> p1 =
                     boost::dynamic_pointer_cast<PlainVanillaPayoff>(
                                                             arguments_.payoff);
                 boost::shared_ptr<PlainVanillaPayoff> p2 =
                     boost::dynamic_pointer_cast<PlainVanillaPayoff>(
                                           cachedArgs2results_[i].first.payoff);
                 
                 if (p1 && p1->strike()     == p2->strike() 
                        && p1->optionType() == p2->optionType()) {
                     QL_REQUIRE(arguments_.cashFlow.empty(),
                                "multiple strikes engine does "
                                "not work with discrete dividends");
                     results_ = cachedArgs2results_[i].second;
                     return;
                 }
             }
         }
       
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
        const Size tGridMin = 5;
        const boost::shared_ptr<FdmHestonVarianceMesher> varianceMesher(
            new FdmHestonVarianceMesher(layout->dim()[1], hestonProcess, 
                                        maturity,std::max(tGridMin,tGrid_/50)));

        // 2.2 The equity mesher
        const boost::shared_ptr<StrikedTypePayoff> payoff =
            boost::dynamic_pointer_cast<StrikedTypePayoff>(arguments_.payoff);
        QL_REQUIRE(payoff, "worng payoff type given");

        boost::shared_ptr<Fdm1dMesher> equityMesher;
        if (strikes_.empty()) {
            equityMesher = boost::shared_ptr<Fdm1dMesher>(
                new FdmBlackScholesMesher(
                    xGrid_, 
                    FdmBlackScholesMesher::processHelper(
                      hestonProcess->s0(), hestonProcess->dividendYield(), 
                      hestonProcess->riskFreeRate(), 
                      varianceMesher->volaEstimate()),
                      maturity, payoff->strike(),
                      Null<Real>(), Null<Real>(), 0.0001, 1.5, 
                      std::pair<Real, Real>(payoff->strike(), 0.1)));
        }
        else {
            QL_REQUIRE(arguments_.cashFlow.empty(),"multiple strikes engine "
                       "does not work with discrete dividends");
            equityMesher = boost::shared_ptr<Fdm1dMesher>(
                new FdmBlackScholesMultiStrikeMesher(
                    xGrid_,
                    FdmBlackScholesMesher::processHelper(
                      hestonProcess->s0(), hestonProcess->dividendYield(), 
                      hestonProcess->riskFreeRate(), 
                      varianceMesher->volaEstimate()),
                    maturity, strikes_, 0.0001, 1.5,
                    std::pair<Real, Real>(payoff->strike(), 0.075)));            
        }
       
        //2.3 The short rate mesher        
        const Rate r0 = hwProcess_->x0();
        const boost::shared_ptr<Fdm1dMesher> shortRateMesher(
                        new FdmHullWhiteMesher(rGrid_, hwProcess_, maturity));
        
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

        // 3.2 Step condition if american or bermudan exercise 
        QL_REQUIRE(   arguments_.exercise->type() == Exercise::American
                   || arguments_.exercise->type() == Exercise::European
                   || arguments_.exercise->type() == Exercise::Bermudan,
                   "exercise type is not supported");

        boost::shared_ptr<FdmInnerValueCalculator> calculator(
                            new FdmLogInnerValue(arguments_.payoff, mesher, 0));
        if (arguments_.exercise->type() == Exercise::American) {
            stepConditions.push_back(boost::shared_ptr<StepCondition<Array> >(
                            new FdmAmericanStepCondition(mesher, calculator)));
        }
        else if (arguments_.exercise->type() == Exercise::Bermudan) {
            boost::shared_ptr<FdmBermudanStepCondition> bermudanCondition(
                new FdmBermudanStepCondition(
                                arguments_.exercise->dates(),
                                hestonProcess->riskFreeRate()->referenceDate(),
                                hestonProcess->riskFreeRate()->dayCounter(),
                                mesher, calculator));
            stepConditions.push_back(bermudanCondition);
            stoppingTimes.push_back(bermudanCondition->exerciseTimes());
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
                                         calculator, 
                                         maturity, tGrid_, dampingSteps_,
                                         schemeDesc_));

        const Real spot = hestonProcess->s0()->value();
        const Real v0   = hestonProcess->v0();
        results_.value = solver->valueAt(spot, v0, r0);
        results_.delta = solver->deltaAt(spot, v0, r0, spot*0.01);
        results_.gamma = solver->gammaAt(spot, v0, r0, spot*0.01);
        results_.theta = solver->thetaAt(spot, v0, r0);

        cachedArgs2results_.resize(strikes_.size());        
        for (Size i=0; i < strikes_.size(); ++i) {
            cachedArgs2results_[i].first.exercise = arguments_.exercise;
            cachedArgs2results_[i].first.payoff = 
                boost::shared_ptr<PlainVanillaPayoff>(
                    new PlainVanillaPayoff(payoff->optionType(), strikes_[i]));
            const Real d = payoff->strike()/strikes_[i];
            
            DividendVanillaOption::results& 
                                results = cachedArgs2results_[i].second;
            results.value = solver->valueAt(spot*d, v0, r0)/d;
            results.delta = solver->deltaAt(spot*d, v0, r0, spot*d*0.01);
            results.gamma = solver->gammaAt(spot*d, v0, r0, spot*d*0.01)*d;
            results.theta = solver->thetaAt(spot*d, v0, r0)/d;
        }
     
        if (controlVariate_) {
            boost::shared_ptr<PricingEngine> analyticEngine(
                                       new AnalyticHestonEngine(*model_, 164));
            boost::shared_ptr<Exercise> exercise(
                        new EuropeanExercise(arguments_.exercise->lastDate()));
            
            VanillaOption option(payoff, exercise);
            option.setPricingEngine(analyticEngine);
            Real analyticNPV = option.NPV();

            boost::shared_ptr<FdHestonVanillaEngine> fdEngine(
                    new FdHestonVanillaEngine(*model_, tGrid_, xGrid_, 
                                              vGrid_, dampingSteps_, 
                                              schemeDesc_));
            fdEngine->enableMultipleStrikesCaching(strikes_);
            option.setPricingEngine(fdEngine);
            
            Real fdNPV = option.NPV();
            results_.value += analyticNPV - fdNPV;
            for (Size i=0; i < strikes_.size(); ++i) {
                VanillaOption controlVariateOption(
                    boost::shared_ptr<StrikedTypePayoff>(
                        new PlainVanillaPayoff(payoff->optionType(), 
                                               strikes_[i])), exercise);
                controlVariateOption.setPricingEngine(analyticEngine);
                analyticNPV = controlVariateOption.NPV();
                
                controlVariateOption.setPricingEngine(fdEngine);
                fdNPV = controlVariateOption.NPV();
                cachedArgs2results_[i].second.value += analyticNPV - fdNPV;
            }
        }
    }
    
    void FdHestonHullWhiteVanillaEngine::update() {
        cachedArgs2results_.clear();
        GenericModelEngine<HestonModel, DividendVanillaOption::arguments,
                           DividendVanillaOption::results>::update();
    }
    void FdHestonHullWhiteVanillaEngine::enableMultipleStrikesCaching(
                                        const std::vector<Real>& strikes) {
        strikes_ = strikes;
        update();
    }
}
