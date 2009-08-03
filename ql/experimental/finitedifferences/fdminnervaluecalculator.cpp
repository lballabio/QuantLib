/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Andreas Gaida
 Copyright (C) 2008,2009 Ralph Schreyer
 Copyright (C) 2008,2009 Klaus Spanderen

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
#include <ql/experimental/finitedifferences/fdmmesher.hpp>
#include <ql/experimental/finitedifferences/fdminnervaluecalculator.hpp>


namespace QuantLib {

    FdmLogInnerValue::FdmLogInnerValue(const boost::shared_ptr<Payoff>& payoff,
                                       Size direction)
    : payoff_(payoff), direction_(direction) { }

    Real FdmLogInnerValue::innerValue(
             const boost::shared_ptr<FdmMesher>& mesher,
             const FdmLinearOpIterator& iter) {
        
        return payoff_->operator()(std::exp(mesher->location(
                                            iter,direction_)));
    }

    Real FdmLogInnerValue::avgInnerValue(
             const boost::shared_ptr<FdmMesher>& mesher,
             const FdmLinearOpIterator& iter) {
        
        const Size dim = mesher->layout()->dim()[direction_];
        const Size coord = iter.coordinates()[direction_];
        const Real loc = mesher->location(iter,direction_);
        Real a = loc;
        Real b = loc;
        if (coord > 0) {
            a -= mesher->dminus(iter, direction_)/2.0;
        }
        if (coord < dim-1) {
            b += mesher->dplus(iter, direction_)/2.0;
        }
        boost::function1<Real, Real> f = compose(
            std::bind1st(std::mem_fun(&Payoff::operator()), payoff_.get()),
                         std::ptr_fun<Real,Real>(std::exp));
        
        Real retVal;
        try {
            retVal = SimpsonIntegral(1e-4, 10)(f, a, b)/(b-a);
        }
        catch (Error&) {
            // use default value
            retVal = innerValue(mesher, iter);
        }
                    
        return retVal;
    }
}
