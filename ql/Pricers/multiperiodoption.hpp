
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

/*! \file multiperiodoption.hpp
    \brief base class for option with events happening at different periods

    \fullpath
    ql/Pricers/%multiperiodoption.hpp
*/

// $Id$

#ifndef quantlib_multi_period_option_pricer_h
#define quantlib_multi_period_option_pricer_h

#include "ql/Pricers/bsmnumericaloption.hpp"
#include "ql/FiniteDifferences/fdtypedefs.hpp"
#include <vector>

namespace QuantLib {

    namespace Pricers {

        class MultiPeriodOption : public BSMNumericalOption {
          public:
            double controlVariateCorrection() const;
          protected:
            // constructor
            MultiPeriodOption(Option::Type type, double underlying,
                double strike, Spread dividendYield, Rate riskFreeRate,
                Time residualTime, double volatility, int gridPoints,
                const std::vector<Time>& dates,
                int timeSteps);
            // Protected attributes
            std::vector<Time> dates_;
            size_t dateNumber_;
            int timeStepPerPeriod_;
            bool lastDateIsResTime_;
            int lastIndex_;
            bool firstDateIsZero_;
            double firstNonZeroDate_;
            int firstIndex_;
            mutable Handle<SingleAssetOption> analytic_;
            mutable Array prices_, controlPrices_;
            mutable Handle<FiniteDifferences::StandardStepCondition>
                                                            stepCondition_;
            mutable Handle<FiniteDifferences::StandardFiniteDifferenceModel>
                                                            model_;
            // Methods
            void calculate() const;
            virtual void initializeControlVariate() const;
            virtual void initializeModel() const;
            virtual void initializeStepCondition() const;
            virtual void executeIntermediateStep(int step) const = 0;
          private:
            mutable double controlVariateCorrection_;
        };

    }

}


#endif
