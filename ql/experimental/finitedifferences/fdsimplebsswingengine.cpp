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
#include <ql/experimental/finitedifferences/fdmlinearoplayout.hpp>
#include <ql/experimental/finitedifferences/uniform1dmesher.hpp>
#include <ql/experimental/finitedifferences/fdminnervaluecalculator.hpp>
#include <ql/experimental/finitedifferences/fdmmeshercomposite.hpp>
#include <ql/experimental/finitedifferences/fdmblackscholesmesher.hpp>
#include <ql/experimental/finitedifferences/fdsimplebsswingengine.hpp>
#include <ql/experimental/finitedifferences/fdmsimpleswingcondition.hpp>
#include <ql/experimental/finitedifferences/fdmstepconditioncomposite.hpp>
#include <ql/experimental/finitedifferences/fdmsimple2dbssolver.hpp>

namespace QuantLib {

    namespace {
        class FdmZeroInnerValue : public FdmInnerValueCalculator {
          public:
            Real innerValue(const FdmLinearOpIterator&)    { return 0.0; }
            Real avgInnerValue(const FdmLinearOpIterator&) { return 0.0; }
        };
    }
    
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
        
        // 1. Layout
        std::vector<Size> dim;
        dim.push_back(xGrid_);
        dim.push_back(arguments_.exerciseRights+1);
        const boost::shared_ptr<FdmLinearOpLayout> layout(
                                              new FdmLinearOpLayout(dim));
        
        // 2. Mesher
        const boost::shared_ptr<StrikedTypePayoff> payoff =
            boost::dynamic_pointer_cast<StrikedTypePayoff>(arguments_.payoff);
        const Time maturity = process_->time(arguments_.exercise->lastDate());
        const boost::shared_ptr<Fdm1dMesher> equityMesher(
            new FdmBlackScholesMesher(xGrid_, process_,
                                      maturity, payoff->strike()));
        
        const boost::shared_ptr<Fdm1dMesher> exerciseMesher(
                 new Uniform1dMesher(0, arguments_.exerciseRights, 
                                        arguments_.exerciseRights+1));
        
        std::vector<boost::shared_ptr<Fdm1dMesher> > meshers;
        meshers.push_back(equityMesher);
        meshers.push_back(exerciseMesher);
        boost::shared_ptr<FdmMesher> mesher (
                                     new FdmMesherComposite(layout, meshers));
        
        // 3. Calculator
        boost::shared_ptr<FdmInnerValueCalculator> calculator(
                                                    new FdmZeroInnerValue());
        
        // 4. Step conditions
        std::list<boost::shared_ptr<StepCondition<Array> > > stepConditions;
        std::list<std::vector<Time> > stoppingTimes;
        
        // 4.1 Bermudan step conditions
        std::vector<Time> exerciseTimes;
        for (Size i=0; i<arguments_.exercise->dates().size(); ++i) {
            Time t = process_->time(arguments_.exercise->dates()[i]);
            QL_REQUIRE(t >= 0, "exercise dates must not contain past date");
            exerciseTimes.push_back(t);
        }
        stoppingTimes.push_back(exerciseTimes);
        
        stepConditions.push_back(boost::shared_ptr<StepCondition<Array> >(
            new FdmSimpleSwingCondition(exerciseTimes, mesher, payoff, 0, 1)));
        
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
        const Real y = std::exp(arguments_.exerciseRights);
        
        results_.value = solver->valueAt(spot, y);
        results_.delta = solver->deltaAt(spot, y, spot*0.01);
        results_.gamma = solver->gammaAt(spot, y, spot*0.01);
        results_.theta = solver->thetaAt(spot, y);
    }
}
