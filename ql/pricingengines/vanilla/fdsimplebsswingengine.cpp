/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2010 Klaus Spanderen

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

/*! \file fdsimplebsswingengine.cpp
    \brief Finite Differences Black-Scholes engine for simple swing options
*/

#include <ql/processes/blackscholesprocess.hpp>
#include <ql/methods/finitedifferences/operators/fdmlinearoplayout.hpp>
#include <ql/methods/finitedifferences/meshers/uniform1dmesher.hpp>
#include <ql/methods/finitedifferences/utilities/fdminnervaluecalculator.hpp>
#include <ql/methods/finitedifferences/meshers/fdmmeshercomposite.hpp>
#include <ql/methods/finitedifferences/meshers/fdmblackscholesmesher.hpp>
#include <ql/pricingengines/vanilla/fdsimplebsswingengine.hpp>
#include <ql/methods/finitedifferences/stepconditions/fdmsimpleswingcondition.hpp>
#include <ql/methods/finitedifferences/stepconditions/fdmstepconditioncomposite.hpp>
#include <ql/methods/finitedifferences/solvers/fdmsimple2dbssolver.hpp>

namespace QuantLib {
    
    FdSimpleBSSwingEngine::FdSimpleBSSwingEngine(
            const boost::shared_ptr<GeneralizedBlackScholesProcess>& process,
            Size tGrid, Size xGrid,
            const FdmSchemeDesc& schemeDesc)
    : process_(process),
      tGrid_(tGrid),
      xGrid_(xGrid),
      schemeDesc_(schemeDesc) { 
    }
            
    void FdSimpleBSSwingEngine::calculate() const {

        QL_REQUIRE(arguments_.exercise->type() == Exercise::Bermudan,
                   "Bermudan exercise supported only");

        // 1. Mesher
        const boost::shared_ptr<StrikedTypePayoff> payoff =
            boost::dynamic_pointer_cast<StrikedTypePayoff>(arguments_.payoff);
        const Time maturity = process_->time(arguments_.exercise->lastDate());
        const boost::shared_ptr<Fdm1dMesher> equityMesher(
            new FdmBlackScholesMesher(xGrid_, process_,
                                      maturity, payoff->strike()));
        
        const boost::shared_ptr<Fdm1dMesher> exerciseMesher(
                 new Uniform1dMesher(
                           0, static_cast<Real>(arguments_.maxExerciseRights),
                           arguments_.maxExerciseRights+1));
        
        const boost::shared_ptr<FdmMesher> mesher (
            new FdmMesherComposite(equityMesher, exerciseMesher));
        
        // 2. Calculator
        boost::shared_ptr<FdmInnerValueCalculator> calculator(
                                                    new FdmZeroInnerValue());
        
        // 3. Step conditions
        std::list<boost::shared_ptr<StepCondition<Array> > > stepConditions;
        std::list<std::vector<Time> > stoppingTimes;
        
        // 3.1 Bermudan step conditions
        std::vector<Time> exerciseTimes;
        for (Size i=0; i<arguments_.exercise->dates().size(); ++i) {
            Time t = process_->time(arguments_.exercise->dates()[i]);
            QL_REQUIRE(t >= 0, "exercise dates must not contain past date");
            exerciseTimes.push_back(t);
        }
        stoppingTimes.push_back(exerciseTimes);
        
        boost::shared_ptr<FdmInnerValueCalculator> exerciseCalculator(
                                    new FdmLogInnerValue(payoff, mesher, 0));

        stepConditions.push_back(boost::shared_ptr<StepCondition<Array> >(
            new FdmSimpleSwingCondition(exerciseTimes, mesher,
                                        exerciseCalculator, 1)));
        
        boost::shared_ptr<FdmStepConditionComposite> conditions(
                new FdmStepConditionComposite(stoppingTimes, stepConditions));
        
        // 4. Boundary conditions
        const FdmBoundaryConditionSet boundaries;
        
        // 5. Solver
        FdmSolverDesc solverDesc = { mesher, boundaries, conditions,
                                     calculator, maturity, tGrid_, 0 };
        boost::shared_ptr<FdmSimple2dBSSolver> solver(
                new FdmSimple2dBSSolver(
                               Handle<GeneralizedBlackScholesProcess>(process_),
                               payoff->strike(), solverDesc, schemeDesc_));
    
        const Real spot = process_->x0();
        
        std::vector< std::pair<Real, Real> > exerciseValues;
        for (Size i=arguments_.minExerciseRights;
             i <= arguments_.maxExerciseRights; ++i) {
            const Real y = std::exp(Real(i));
            exerciseValues.push_back(
                           std::pair<Real, Real>(solver->valueAt(spot, y), y));
        }
        const Real y = std::max_element(exerciseValues.begin(),
                                        exerciseValues.end())->second;

        results_.value = solver->valueAt(spot, y);
        results_.delta = solver->deltaAt(spot, y, spot*0.01);
        results_.gamma = solver->gammaAt(spot, y, spot*0.01);
        results_.theta = solver->thetaAt(spot, y);
    }
}
