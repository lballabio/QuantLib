

/*
 Copyright (C) 2000, 2001, 2002 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email ferdinando@ametrano.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/
/*! \file shoutcondition.hpp
    \brief shout option exercise condition

    \fullpath
    ql/FiniteDifferences/%shoutcondition.hpp
*/

// $Id$

#ifndef quantlib_fd_shout_condition_h
#define quantlib_fd_shout_condition_h

#include <ql/FiniteDifferences/fdtypedefs.hpp>

namespace QuantLib {

    namespace FiniteDifferences {

        class ShoutCondition
        : public FiniteDifferences::StandardStepCondition {
          public:
            ShoutCondition(const Array& initialPrices,
                           Time resTime,
                           Rate rate);
            void applyTo(Array& a,
                         Time t) const;
          private:
            Array initialPrices_;
            Time resTime_;
            Rate rate_;
        };

        inline ShoutCondition::ShoutCondition(
            const Array& initialPrices, Time resTime, Rate rate)
            : initialPrices_(initialPrices), resTime_(resTime), rate_(rate) {}

        inline void ShoutCondition::applyTo(Array& a, Time t) const {
            for (Size i = 0; i < a.size(); i++)
                a[i] = QL_MAX(a[i], QL_EXP(-rate_ * (t - resTime_)) *
                                           initialPrices_[i] );
        }

    }

}


#endif
