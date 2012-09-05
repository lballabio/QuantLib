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

#include <ql/methods/finitedifferences/operators/fdmlinearoplayout.hpp>
#include <ql/methods/finitedifferences/stepconditions/fdmsimpleswingcondition.hpp>

namespace QuantLib {

    FdmSimpleSwingCondition::FdmSimpleSwingCondition(
            const std::vector<Time> & exerciseTimes,
            const boost::shared_ptr<FdmMesher>& mesher,
            const boost::shared_ptr<FdmInnerValueCalculator>& calculator,
            Size swingDirection)
    : exerciseTimes_ (exerciseTimes),
      mesher_        (mesher),
      calculator_    (calculator),
      swingDirection_(swingDirection) {
    }
    
    void FdmSimpleSwingCondition::applyTo(Array& a, Time t) const {
        const std::vector<Time>::const_iterator iter
            = std::find(exerciseTimes_.begin(), exerciseTimes_.end(), t);
        
        if (iter != exerciseTimes_.end()) {
            Array retVal= a;

            const Size d = std::distance(iter, exerciseTimes_.end());

            const boost::shared_ptr<FdmLinearOpLayout> layout=mesher_->layout();
            const FdmLinearOpIterator endIter = layout->end();
            
            for (FdmLinearOpIterator iter = layout->begin(); iter != endIter;
                 ++iter) {
                
                const std::vector<Size>& coor = iter.coordinates();
                
                const Size exerciseValue = coor[swingDirection_];
                
                if (exerciseValue > 0) {
                    const Real cashflow = calculator_->innerValue(iter, t);
                    const Real currentValue = a[iter.index()];
                    const Real valueMinusOneExRight 
                         = a[layout->neighbourhood(iter, swingDirection_, -1)];
                    
                    if (   currentValue < cashflow + valueMinusOneExRight
                        || exerciseValue >= d ) {
                        retVal[iter.index()] = cashflow + valueMinusOneExRight;
                    }
                }
            }
            a = retVal;
        }
    }
}
