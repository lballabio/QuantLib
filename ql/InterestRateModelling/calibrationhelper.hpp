

/*
 Copyright (C) 2001, 2002 Sadruddin Rejeb

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
                                     Size maxEvaluations,
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
