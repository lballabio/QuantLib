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
/*! \file calibrationhelper.cpp
    \brief Calibration helper class

    \fullpath
    ql/InterestRateModelling/CalibrationHelpers/%calibrationhelper.hpp
*/

// $Id$

#include "ql/InterestRateModelling/calibrationhelper.hpp"
#include "ql/Solvers1D/brent.hpp"

namespace QuantLib {

    namespace InterestRateModelling {

        class CalibrationHelper::ImpliedVolatilityHelper
        : public ObjectiveFunction {
          public:
            ImpliedVolatilityHelper(const CalibrationHelper& helper,
                                    double value)
            : helper_(helper), value_(value) {}

            double operator()(double x) const {
                return value_ - helper_.blackPrice(x);
            }
          private:
            const CalibrationHelper& helper_;
            double value_;
        };

        double CalibrationHelper::impliedVolatility(double targetValue,
            double accuracy, Size maxEvaluations,
            double minVol, double maxVol) const {

            ImpliedVolatilityHelper f(*this,targetValue);
            Solvers1D::Brent solver;
            solver.setMaxEvaluations(maxEvaluations);
            return solver.solve(f,accuracy,volatility_->value(),minVol,maxVol);
        }

    }

}
