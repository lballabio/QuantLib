
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

/*! \file swaptioncondition.hpp
    \brief swaption option exercise condition

    \fullpath
    ql/Pricers/%swaptioncondition.hpp
*/

// $Id$

#ifndef quantlib_pricers_swaption_condition_h
#define quantlib_pricers_swaption_condition_h

#include "ql/FiniteDifferences/fdtypedefs.hpp"
#include "ql/InterestRateModelling/swapfuturevalue.hpp"

namespace QuantLib {

    namespace Pricers {

        class SwaptionCondition
        : public FiniteDifferences::StandardStepCondition {
          public:
            SwaptionCondition(const Handle<InterestRateModelling::Model>& model,
                const Handle<Instruments::SimpleSwap>& swap,
                const std::vector<double>& rates);
            void applyTo(Array& a, Time t) const;
          private:
            const Handle<InterestRateModelling::Model>& model_;
            const Handle<Instruments::SimpleSwap>& swap_;
            std::vector<double> rates_;
        };


        // inline definitions

        inline SwaptionCondition::SwaptionCondition(
            const Handle<InterestRateModelling::Model>& model,
            const Handle<Instruments::SimpleSwap>& swap,
            const std::vector<double>& rates)
        : model_(model), swap_(swap), rates_(rates) {}

        inline void SwaptionCondition::applyTo(Array& a, Time t) const {
            for (unsigned int i = 0; i < a.size(); i++)
                a[i] = QL_MAX(a[i],
                    QL_MAX(0.0,
                    InterestRateModelling::swapFutureValue(swap_, model_, rates_[i], t)));
        }

    }

}


#endif
