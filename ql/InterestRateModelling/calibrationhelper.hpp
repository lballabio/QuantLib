
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

/*! \file calibrationhelper.hpp
    \brief Calibration helper class

    \fullpath
    ql/InterestRateModelling/%calibrationhelper.hpp
*/

// $Id$

#ifndef quantlib_interest_rate_modelling_calibration_helper_h
#define quantlib_interest_rate_modelling_calibration_helper_h

#include <ql/InterestRateModelling/model.hpp>

namespace QuantLib {

    namespace InterestRateModelling {

        class CalibrationHelper {

          public:
            double marketValue() { return marketValue_; }
            virtual double modelValue(const Handle<Model>& model) = 0;

            void setVolatility(double volatility) {
                volatility_ = volatility;
                marketValue_ = blackPrice(volatility);
            }

            double impliedVolatility(double targetValue,
                                     double accuracy, 
                                     size_t maxEvaluations,
                                     double minVol, 
                                     double maxVol) const;

            virtual double blackPrice(double volatility) const = 0;

          private:
            class ImpliedVolatilityHelper;

            double volatility_;
            double marketValue_;
        };

    }

}
#endif
