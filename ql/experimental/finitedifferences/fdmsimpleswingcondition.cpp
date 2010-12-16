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

#include <ql/experimental/finitedifferences/fdmsimpleswingcondition.hpp>

namespace QuantLib {

    FdmSimpleSwingCondition::FdmSimpleSwingCondition(
            const std::vector<Time> & exerciseTimes,
            const boost::shared_ptr<FdmMesher> & mesher,
            const boost::shared_ptr<Payoff> & payoff,
            Size equityDirection,
            Size swingDirection)
    : x_(mesher->layout()->dim()[0]),
      exerciseTimes_(exerciseTimes), 
      mesher_(mesher),
      payoff_(payoff),
      equityDirection_(equityDirection),
      swingDirection_(swingDirection) {
                
        const Size xSpacing = mesher_->layout()->spacing()[equityDirection_];
        Array tmp = mesher_->locations(equityDirection_);
        for (Size i = 0; i < x_.size(); ++i) {
            x_[i] = std::exp(tmp[i*xSpacing]);
        }
    }
    
    void FdmSimpleSwingCondition::applyTo(Array& a, Time t) const {
        Array retVal = a;
        
        const std::vector<Time>::const_iterator iter
            = std::find(exerciseTimes_.begin(), exerciseTimes_.end(), t);
        
        if (iter != exerciseTimes_.end()) {    
            const boost::shared_ptr<FdmLinearOpLayout> layout=mesher_->layout();
            const FdmLinearOpIterator endIter = layout->end();
            
            for (FdmLinearOpIterator iter = layout->begin(); iter != endIter;
                 ++iter) {
                
                const std::vector<Size>& coor = iter.coordinates();
                
                const Real x = x_[coor[equityDirection_]];
                const Size exerciseValue = coor[swingDirection_];
                
                if (exerciseValue > 0) {
                    const Real cashflow = (*payoff_)(x);
                    const Real currentValue = a[iter.index()];
                    const Real valueMinusOneExRight 
                         = a[layout->neighbourhood(iter, swingDirection_, -1)];
                    
                    if (currentValue < cashflow + valueMinusOneExRight) {
                        retVal[iter.index()] = cashflow + valueMinusOneExRight;
                    }
                }
            }
        }
            
        a = retVal;
    }
}
