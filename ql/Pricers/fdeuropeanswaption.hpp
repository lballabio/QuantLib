
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

/*! \file fdeuropeanswaption.hpp
    \brief European swaption calculated using finite differences

    \fullpath
    ql/Pricers/%fdeuropeanswaption.hpp
*/

// $Id$

#ifndef quantlib_pricers_european_swaption_h
#define quantlib_pricers_european_swaption_h

#include <vector>
#include "ql/option.hpp"
#include "ql/InterestRateModelling/onefactormodel.hpp"
#include "ql/Pricers/swaptioncondition.hpp"
#include "ql/FiniteDifferences/onefactoroperator.hpp"

namespace QuantLib {

    namespace Pricers {

        //! Discount Bond calculated using finite differences
        class FDEuropeanSwaption {
          public:
            FDEuropeanSwaption(
              const Handle<Instruments::SimpleSwap>& swap,
              Time maturity,
              const Handle<InterestRateModelling::Model>& model);
            double value(Rate rate,
              unsigned int timeSteps, unsigned int gridPoints);
          private:
            void fixInitialCondition(Array& prices);

            const Handle<Instruments::SimpleSwap>& swap_;
            Time maturity_;
            const Handle<InterestRateModelling::Model>& model_;
            mutable Handle<FiniteDifferences::StandardStepCondition > stepCondition_;
        };
    }
}

#endif
