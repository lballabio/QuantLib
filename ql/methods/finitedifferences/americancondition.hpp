/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl
 Copyright (C) 2003, 2004, 2005 StatPro Italia srl

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

/*! \file americancondition.hpp
    \brief american option exercise condition
*/

#ifndef quantlib_fd_american_condition_h
#define quantlib_fd_american_condition_h

#include <ql/methods/finitedifferences/fdtypedefs.hpp>
#include <ql/discretizedasset.hpp>
#include <ql/instruments/payoffs.hpp>

namespace QuantLib {

    //! American exercise condition.
    /*! \todo unify the intrinsicValues/Payoff thing */
    class AmericanCondition :
        public StandardCurveDependentStepCondition {
    public:
        AmericanCondition(Option::Type type,
                          Real strike)
            : StandardCurveDependentStepCondition(type, strike) {};
        AmericanCondition(const Array& intrinsicValues)
            : StandardCurveDependentStepCondition(intrinsicValues) {};
    private:
        Real applyToValue(Real current, Real intrinsic) const {
            return std::max(current, intrinsic);
        }
    };
}


#endif
