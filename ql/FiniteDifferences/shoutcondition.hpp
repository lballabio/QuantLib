
/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email quantlib-dev@lists.sf.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file shoutcondition.hpp
    \brief shout option exercise condition
*/

#ifndef quantlib_fd_shout_condition_h
#define quantlib_fd_shout_condition_h

#include <ql/FiniteDifferences/fdtypedefs.hpp>
#include <ql/discretizedasset.hpp>

namespace QuantLib {

    //! Shout option condition
    /*! A shout option is an option where the holder has the right to
        lock in a minimum value for the payoff at one (shout) time
        during the option's life. The minimum value is the option's
        intrinsic value at the shout time.

        \todo Unify the intrinsicValues/Payoff thing
    */
    class ShoutCondition : public StandardStepCondition {
      public:
        ShoutCondition(Option::Type type,
                       double strike,
                       Time resTime,
                       Rate rate);
        ShoutCondition(const Array& intrinsicValues,
                       Time resTime,
                       Rate rate);
        void applyTo(Array& a,
                     Time t) const;
        void applyTo(boost::shared_ptr<DiscretizedAsset> asset) const;
      private:
        Array intrinsicValues_;
        boost::shared_ptr<Payoff> payoff_;
        Time resTime_;
        Rate rate_;
    };

    inline ShoutCondition::ShoutCondition(Option::Type type,
                                          double strike, Time resTime, 
                                          Rate rate)
    : payoff_(new PlainVanillaPayoff(type, strike)), 
      resTime_(resTime), rate_(rate) {}

    inline ShoutCondition::ShoutCondition(const Array& intrinsicValues, 
                                          Time resTime, Rate rate)
    : intrinsicValues_(intrinsicValues), resTime_(resTime), rate_(rate) {}

    inline void ShoutCondition::applyTo(Array& a, Time t) const {

        DiscountFactor disc = QL_EXP(-rate_ * (t - resTime_));

        if (intrinsicValues_.size()!=0) {
            QL_REQUIRE(intrinsicValues_.size() == a.size(),
                       "size mismatch");
            for (Size i = 0; i < a.size(); i++)
                a[i] = QL_MAX(a[i],
                              disc * intrinsicValues_[i] );
        } else {
            for (Size i = 0; i < a.size(); i++)
                a[i] = QL_MAX(a[i],
                              (*payoff_)(a[i]) * disc);
        }
    }

    inline void ShoutCondition::applyTo(
                            boost::shared_ptr<DiscretizedAsset> asset) const {
        DiscountFactor disc = QL_EXP(-rate_ * (asset->time() - resTime_));

        if (intrinsicValues_.size()!=0) {
            QL_REQUIRE(intrinsicValues_.size() == asset->values().size(),
                       "size mismatch");
            for (Size i = 0; i < asset->values().size(); i++)
                asset->values()[i] = QL_MAX(asset->values()[i],
                                            disc * intrinsicValues_[i] );
        } else {
            for (Size i = 0; i < asset->values().size(); i++)
                asset->values()[i] = QL_MAX(asset->values()[i],
                                            (*payoff_)(asset->values()[i]) * 
                                            disc);
        }
    }

}


#endif
