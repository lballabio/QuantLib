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

/*! \file fdminnervaluecalculator.cpp
    \brief layer of abstraction to calculate the inner value
*/

#include <ql/payoff.hpp>
#include <ql/math/functional.hpp>
#include <ql/math/integrals/simpsonintegral.hpp>
#include <ql/instruments/basketoption.hpp>
#include <ql/methods/finitedifferences/meshers/fdmmesher.hpp>
#include <ql/methods/finitedifferences/operators/fdmlinearoplayout.hpp>
#include <ql/methods/finitedifferences/utilities/fdminnervaluecalculator.hpp>

#include <deque>

namespace QuantLib {

    FdmLogInnerValue::FdmLogInnerValue(
        const boost::shared_ptr<Payoff>& payoff,
        const boost::shared_ptr<FdmMesher>& mesher,
        Size direction)
    : payoff_(payoff), 
      mesher_(mesher),
      direction_ (direction) {
    }

    Real FdmLogInnerValue::innerValue(const FdmLinearOpIterator& iter, Time) {
        const Real s = std::exp(mesher_->location(iter, direction_));
        return payoff_->operator()(s);
    }

    Real FdmLogInnerValue::avgInnerValue(
                                    const FdmLinearOpIterator& iter, Time t) {
        if (avgInnerValues_.empty()) {
            // calculate caching values
            avgInnerValues_.resize(mesher_->layout()->dim()[direction_]);
            std::deque<bool> initialized(avgInnerValues_.size(), false);

            const boost::shared_ptr<FdmLinearOpLayout> layout=mesher_->layout();
            const FdmLinearOpIterator endIter = layout->end();
            for (FdmLinearOpIterator iter = layout->begin(); iter != endIter;
                 ++iter) {
                const Size xn = iter.coordinates()[direction_];
                if (!initialized[xn]) {
                    initialized[xn]     = true;
                    avgInnerValues_[xn] = avgInnerValueCalc(iter, t);
                }
            }
        }
        
        return avgInnerValues_[iter.coordinates()[direction_]];
    }
    
    Real FdmLogInnerValue::avgInnerValueCalc(
                                    const FdmLinearOpIterator& iter, Time t) {
        const Size dim = mesher_->layout()->dim()[direction_];
        const Size coord = iter.coordinates()[direction_];
        const Real loc = mesher_->location(iter,direction_);
        Real a = loc;
        Real b = loc;
        if (coord > 0) {
            a -= mesher_->dminus(iter, direction_)/2.0;
        }
        if (coord < dim-1) {
            b += mesher_->dplus(iter, direction_)/2.0;
        }
        boost::function1<Real, Real> f = compose(
            std::bind1st(std::mem_fun(&Payoff::operator()), payoff_.get()),
                         std::ptr_fun<Real,Real>(std::exp));
        
        Real retVal;
        try {
            const Real acc 
                = ((f(a) != 0.0 || f(b) != 0.0) ? (f(a)+f(b))*5e-5 : 1e-4);
            retVal = SimpsonIntegral(acc, 8)(f, a, b)/(b-a);
        }
        catch (Error&) {
            // use default value
            retVal = innerValue(iter, t);
        }
                    
        return retVal;
    }
    
    FdmLogBasketInnerValue::FdmLogBasketInnerValue(
                                const boost::shared_ptr<BasketPayoff>& payoff,
                                const boost::shared_ptr<FdmMesher>& mesher)
    : payoff_(payoff),
      mesher_(mesher) { }

    Real FdmLogBasketInnerValue::innerValue(
                                    const FdmLinearOpIterator& iter, Time) {
        Array x(mesher_->layout()->dim().size());
        for (Size i=0; i < x.size(); ++i) {
            x[i] = std::exp(mesher_->location(iter, i));
        }
        
        return payoff_->operator()(x);
    }
    
    Real 
    FdmLogBasketInnerValue::avgInnerValue(
                                    const FdmLinearOpIterator& iter, Time t) {
        return innerValue(iter, t);
    }
}
