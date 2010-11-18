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

#include <ql/experimental/finitedifferences/fdmsnapshotcondition.hpp>
#include <ql/experimental/finitedifferences/fdmstepconditioncomposite.hpp>

namespace QuantLib {

    FdmStepConditionComposite::FdmStepConditionComposite(
        const std::list<std::vector<Time> > & stoppingTimes,
        const Conditions & conditions)
    : conditions_(conditions) {

        std::set<Real> allStoppingTimes;
        for (std::list<std::vector<Time> >::const_iterator
             iter = stoppingTimes.begin(); iter != stoppingTimes.end();
             ++iter) {
            allStoppingTimes.insert(iter->begin(), iter->end());
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
        for (Conditions::const_iterator iter = conditions_.begin();
             iter != conditions_.end(); ++iter) {
            (*iter)->applyTo(a, t);
        }
    }
    
    boost::shared_ptr<FdmStepConditionComposite> 
    FdmStepConditionComposite::joinConditions(
                const boost::shared_ptr<FdmSnapshotCondition>& c1,
                const boost::shared_ptr<FdmStepConditionComposite>& c2) {

        std::list<std::vector<Time> > stoppingTimes;
        stoppingTimes.push_back(std::vector<Time>(1, c1->getTime()));
        stoppingTimes.push_back(c2->stoppingTimes());

        FdmStepConditionComposite::Conditions conditions;
        conditions.push_back(c1);
        conditions.push_back(c2);

        return boost::shared_ptr<FdmStepConditionComposite>(
            new FdmStepConditionComposite(stoppingTimes, conditions));
    }
}
