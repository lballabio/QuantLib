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

/*! \file fdminnervaluecalculator.hpp
    \brief layer of abstraction to calculate the inner value
*/

#ifndef quantlib_fdm_inner_value_calculator_hpp
#define quantlib_fdm_inner_value_calculator_hpp

namespace QuantLib {

    class FdmInnerValueCalculator {
      public:
    	virtual ~FdmInnerValueCalculator() {}
        virtual double innerValue(const Array& location) = 0; 
    };
    
    class FdmLogInnerValue : public FdmInnerValueCalculator {
    
      public:
        FdmLogInnerValue(const boost::shared_ptr<Payoff>& payoff,
                         Size direction)
        : payoff_(payoff), direction_(direction) {};
    
        Real innerValue(const Array& location) {
            return payoff_->operator()(std::exp(location[direction_]));
        };
    
      private:
        const boost::shared_ptr<Payoff> payoff_;
        const Size direction_;
    };
    
}

#endif
