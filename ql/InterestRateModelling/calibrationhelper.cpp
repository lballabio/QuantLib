
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
