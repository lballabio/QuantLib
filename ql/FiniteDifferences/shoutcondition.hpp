
/*
 * Copyright (C) 2000-2001 QuantLib Group
 *
 * This file is part of QuantLib.
 * QuantLib is a C++ open source library for financial quantitative
 * analysts and developers --- http://quantlib.org/
 *
 * QuantLib is free software and you are allowed to use, copy, modify, merge,
 * publish, distribute, and/or sell copies of it under the conditions stated
 * in the QuantLib License.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the license for more details.
 *
 * You should have received a copy of the license along with this file;
 * if not, please email quantlib-users@lists.sourceforge.net
 * The license is also available at http://quantlib.org/LICENSE.TXT
 *
 * The members of the QuantLib Group are listed in the Authors.txt file, also
 * available at http://quantlib.org/group.html
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
            for (size_t i = 0; i < a.size(); i++)
                a[i] = QL_MAX(a[i], QL_EXP(-rate_ * (t - resTime_)) *
                                           initialPrices_[i] );
        }

    }

}


#endif
