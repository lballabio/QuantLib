
/*
 * Copyright (C) 2001
 * Ferdinando Ametrano, Luigi Ballabio, Adolfo Benin, Marco Marchioro
 *
 * This file is part of QuantLib.
 * QuantLib is a C++ open source library for financial quantitative
 * analysts and developers --- http://quantlib.sourceforge.net/
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
 * if not, contact ferdinando@ametrano.net
 *
 * QuantLib license is also available at
 *   http://quantlib.sourceforge.net/LICENSE.TXT
*/

/*! \file shoutcondition.h
    \brief shout option exercise condition

    $Source$
    $Log$
    Revision 1.1  2001/03/02 08:36:44  enri
    Shout options added:
    	* BSMAmericanOption is now AmericanOption, same interface
    	* ShoutOption added
    	* both ShoutOption and AmericanOption inherit from
    	  StepConditionOption
    offline.doxy.linux added.


*/

#ifndef quantlib_pricers_shout_condition_h
#define quantlib_pricers_shout_condition_h

#include "qldefines.h"
#include "standardstepcondition.h"
#include <algorithm>
#include <functional>

namespace QuantLib {
    namespace Pricers {
        class ShoutCondition : 
            public FiniteDifferences::StandardStepCondition {
        public:
            ShoutCondition(const Array& initialPrices, Time resTime,
                           Rate rate);
            void applyTo(Array& a, Time t) const;
        private:
            Rate rate_;
            Time resTime_;
            Array initialPrices_;
        };

        inline ShoutCondition::ShoutCondition(
            const Array& initialPrices, Time resTime, Rate rate) 
            : initialPrices_(initialPrices), resTime_(resTime), rate_(rate) {}
            
        inline void ShoutCondition::applyTo(Array& a, Time t) const {
            for (int i = 0; i < a.size(); i++)
                a[i] = QL_MAX(a[i], QL_EXP(-rate_ * (t - resTime_)) * initialPrices_[i] );
        }
    }
}

#endif
