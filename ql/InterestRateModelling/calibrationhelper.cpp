
/*
 Copyright (C) 2000, 2001, 2002 Sadruddin Rejeb

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software developed by the QuantLib Group; you can
 redistribute it and/or modify it under the terms of the QuantLib License;
 either version 1.0, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 QuantLib License for more details.

 You should have received a copy of the QuantLib License along with this
 program; if not, please email ferdinando@ametrano.net

 The QuantLib License is also available at http://quantlib.org/license.html
 The members of the QuantLib Group are listed in the QuantLib License
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
            double accuracy, size_t maxEvaluations,
            double minVol, double maxVol) const {

            ImpliedVolatilityHelper f(*this,targetValue);
            Solvers1D::Brent solver;
            solver.setMaxEvaluations(maxEvaluations);
            return solver.solve(f,accuracy,volatility_,minVol,maxVol);
        }

    }

}
