/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/reference/license.html>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file americancondition.hpp
    \brief american option exercise condition
*/

#ifndef quantlib_fd_american_condition_h
#define quantlib_fd_american_condition_h

#include <ql/FiniteDifferences/fdtypedefs.hpp>
#include <ql/discretizedasset.hpp>
#include <ql/Instruments/payoffs.hpp>

namespace QuantLib {

    //! American exercise condition.
    /*! \todo unify the intrinsicValues/Payoff thing */
    class AmericanCondition : public StandardStepCondition {
      public:
        AmericanCondition(Option::Type type,
                          Real strike);
        AmericanCondition(const Array& intrinsicValues);
        void applyTo(Array& a, Time t) const;
      private:
        Array intrinsicValues_;
        boost::shared_ptr<Payoff> payoff_;
    };


    // inline definitions

    inline AmericanCondition::AmericanCondition(Option::Type type,
                                                Real strike)
    : payoff_(new PlainVanillaPayoff(type, strike)) {}

    inline AmericanCondition::AmericanCondition(const Array& intrinsicValues)
    : intrinsicValues_(intrinsicValues) {}

    inline void AmericanCondition::applyTo(Array& a, Time) const {

        if (intrinsicValues_.size()!=0) {
            QL_REQUIRE(intrinsicValues_.size() == a.size(),
                       "size mismatch");
            for (Size i = 0; i < a.size(); i++)
                a[i] = std::max(a[i], intrinsicValues_[i]);
        } else {
            for (Size i = 0; i < a.size(); i++)
                a[i] = std::max(a[i], (*payoff_)(a[i]));
        }

    }

}


#endif
