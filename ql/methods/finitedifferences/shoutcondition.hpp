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

/*! \file shoutcondition.hpp
    \brief shout option exercise condition
*/

#ifndef quantlib_fd_shout_condition_hpp
#define quantlib_fd_shout_condition_hpp

#include <ql/methods/finitedifferences/fdtypedefs.hpp>
#include <ql/discretizedasset.hpp>
#include <ql/instruments/payoffs.hpp>

namespace QuantLib {

    //! Shout option condition
    /*! A shout option is an option where the holder has the right to
        lock in a minimum value for the payoff at one (shout) time
        during the option's life. The minimum value is the option's
        intrinsic value at the shout time.
    */
    class ShoutCondition : public StandardCurveDependentStepCondition {
      public:
        ShoutCondition(Option::Type type,
                       Real strike,
                       Time resTime,
                       Rate rate);
        ShoutCondition(const Array& intrinsicValues,
                       Time resTime,
                       Rate rate);
        void applyTo(Array& a,
                     Time t) const;
      private:
        virtual Real applyToValue(Real current,
                                  Real intrinsic) const {
            return std::max(current, disc_ * intrinsic );
        }
        Time resTime_;
        Rate rate_;
        mutable DiscountFactor disc_;
    };

    inline ShoutCondition::ShoutCondition(Option::Type type,
                                          Real strike, Time resTime,
                                          Rate rate)
    : StandardCurveDependentStepCondition(type, strike),
      resTime_(resTime), rate_(rate) {}

    inline ShoutCondition::ShoutCondition(const Array& intrinsicValues,
                                          Time resTime, Rate rate)
        : StandardCurveDependentStepCondition(intrinsicValues),
          resTime_(resTime), rate_(rate) {}

    inline void ShoutCondition::applyTo(Array& a, Time t) const {
        disc_ = std::exp(-rate_ * (t - resTime_));
        StandardCurveDependentStepCondition::applyTo(a, t);
    }
}


#endif
