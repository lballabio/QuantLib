
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
    /*! \todo Unify the intrinsicValues/Payoff thing */
    class AmericanCondition : public StandardStepCondition {
      public:
        AmericanCondition(Option::Type type,
                          double strike);
        AmericanCondition(const Array& intrinsicValues);
        void applyTo(Array& a,
                     Time t) const;
        void applyTo(boost::shared_ptr<DiscretizedAsset> asset) const;
      private:
        Array intrinsicValues_;
        boost::shared_ptr<Payoff> payoff_;
    };


    // inline definitions

    inline AmericanCondition::AmericanCondition(Option::Type type,
                                                double strike)
    : payoff_(new PlainVanillaPayoff(type, strike)) {}

    inline AmericanCondition::AmericanCondition(const Array& intrinsicValues)
    : intrinsicValues_(intrinsicValues) {}

    inline void AmericanCondition::applyTo(Array& a, Time t) const {

        if (intrinsicValues_.size()!=0) {
            QL_REQUIRE(intrinsicValues_.size() == a.size(),
                       "AmericanCondition::applyTo : "
                       " size mismatch");
            for (Size i = 0; i < a.size(); i++)
                a[i] = QL_MAX(a[i], intrinsicValues_[i]);
        } else {
            for (Size i = 0; i < a.size(); i++)
                a[i] = QL_MAX(a[i], (*payoff_)(a[i]));
        }

    }

    inline void AmericanCondition::applyTo(
                            boost::shared_ptr<DiscretizedAsset> asset) const {
        if (intrinsicValues_.size()!=0) {
            QL_REQUIRE(intrinsicValues_.size() == asset->values().size(),
                       "AmericanCondition::applyTo : "
                       " size mismatch");
            for (Size i = 0; i < asset->values().size(); i++)
                asset->values()[i] = QL_MAX(asset->values()[i],
                                            intrinsicValues_[i]);
        } else {
            for (Size i = 0; i < asset->values().size(); i++)
                asset->values()[i] = QL_MAX(asset->values()[i],
                                            (*payoff_)(asset->values()[i]));
        }
    }

}


#endif
