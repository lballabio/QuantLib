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


#include <ql/exercise.hpp>
#include <ql/methods/finitedifferences/meshers/fdmmesher.hpp>
#include <ql/methods/finitedifferences/stepconditions/fdmamericanstepcondition.hpp>
#include <ql/methods/finitedifferences/stepconditions/fdmbermudanstepcondition.hpp>
#include <ql/methods/finitedifferences/stepconditions/fdmsnapshotcondition.hpp>
#include <ql/methods/finitedifferences/stepconditions/fdmstepconditioncomposite.hpp>
#include <ql/methods/finitedifferences/utilities/fdmdividendhandler.hpp>
#include <ql/methods/finitedifferences/utilities/fdminnervaluecalculator.hpp>
#include <set>
#include <utility>

namespace QuantLib {

    FdmStepConditionComposite::FdmStepConditionComposite(
        const std::list<std::vector<Time> >& stoppingTimes, Conditions conditions)
    : conditions_(std::move(conditions)) {

        std::set<Real> allStoppingTimes;
        for (const auto& stoppingTime : stoppingTimes) {
            allStoppingTimes.insert(stoppingTime.begin(), stoppingTime.end());
        }
        stoppingTimes_ = std::vector<Time>(allStoppingTimes.begin(),
                                           allStoppingTimes.end());
    }

    const FdmStepConditionComposite::Conditions&
    FdmStepConditionComposite::conditions() const {
        return conditions_;
    }

    const std::vector<Time>& FdmStepConditionComposite::stoppingTimes() const {
        return stoppingTimes_;
    }

    void FdmStepConditionComposite::applyTo(Array& a, Time t) const {
        for (const auto& condition : conditions_) {
            condition->applyTo(a, t);
        }
    }
    
    ext::shared_ptr<FdmStepConditionComposite> 
    FdmStepConditionComposite::joinConditions(
                const ext::shared_ptr<FdmSnapshotCondition>& c1,
                const ext::shared_ptr<FdmStepConditionComposite>& c2) {

        std::list<std::vector<Time> > stoppingTimes;
        stoppingTimes.push_back(c2->stoppingTimes());
        stoppingTimes.emplace_back(1, c1->getTime());

        FdmStepConditionComposite::Conditions conditions;
        conditions.push_back(c2);
        conditions.push_back(c1);

        return ext::make_shared<FdmStepConditionComposite>(
            stoppingTimes, conditions);
    }

    ext::shared_ptr<FdmStepConditionComposite> 
    FdmStepConditionComposite::vanillaComposite(
                 const DividendSchedule& cashFlow,
                 const ext::shared_ptr<Exercise>& exercise,
                 const ext::shared_ptr<FdmMesher>& mesher,
                 const ext::shared_ptr<FdmInnerValueCalculator>& calculator,
                 const Date& refDate,
                 const DayCounter& dayCounter) {
        
        std::list<std::vector<Time> > stoppingTimes;
        std::list<ext::shared_ptr<StepCondition<Array> > > stepConditions;

        if(!cashFlow.empty()) {
            ext::shared_ptr<FdmDividendHandler> dividendCondition(
                new FdmDividendHandler(cashFlow, mesher,
                                       refDate, dayCounter, 0));
            stepConditions.push_back(dividendCondition);

            std::vector<Time> dividendTimes = dividendCondition->dividendTimes();
            stoppingTimes.push_back(dividendTimes);

            // smoother convergence behavior with number of time steps
            const Time maturityTime = dayCounter.yearFraction(
                refDate,exercise->lastDate());

            for (auto& t: dividendTimes)
                t = std::min(maturityTime, t+1e-5);
            stoppingTimes.push_back(dividendTimes);
        }

        QL_REQUIRE(   exercise->type() == Exercise::American
                   || exercise->type() == Exercise::European
                   || exercise->type() == Exercise::Bermudan,
                   "exercise type is not supported");
        if (exercise->type() == Exercise::American) {
            stepConditions.push_back(ext::shared_ptr<StepCondition<Array> >(
                          new FdmAmericanStepCondition(mesher,calculator)));
        }
        else if (exercise->type() == Exercise::Bermudan) {
            ext::shared_ptr<FdmBermudanStepCondition> bermudanCondition(
                new FdmBermudanStepCondition(exercise->dates(),
                                             refDate, dayCounter,
                                             mesher, calculator));
            stepConditions.push_back(bermudanCondition);
            stoppingTimes.push_back(bermudanCondition->exerciseTimes());
        }
        
        return ext::make_shared<FdmStepConditionComposite>(
            stoppingTimes, stepConditions);

    }

}
