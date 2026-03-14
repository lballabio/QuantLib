/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2001, 2002, 2003 Sadruddin Rejeb
 Copyright (C) 2015 Peter Caspers

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <https://www.quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/models/calibrationhelper.hpp>
#include <ql/math/solvers1d/brent.hpp>

namespace QuantLib {

    Volatility BlackCalibrationHelper::impliedVolatility(Real targetValue,
                                                         Real accuracy,
                                                         Size maxEvaluations,
                                                         Volatility minVol,
                                                         Volatility maxVol) const {

        auto error = [&](Volatility x) { return targetValue - blackPrice(x); };
        Brent solver;
        solver.setMaxEvaluations(maxEvaluations);
        return solver.solve(error, accuracy, volatility_->value(), minVol, maxVol);
    }

    Real BlackCalibrationHelper::calibrationError() {
        Real error;
        
        switch (calibrationErrorType_) {
          case RelativePriceError:
            error = std::fabs(marketValue() - modelValue())/marketValue();
            break;
          case PriceError:
            error = marketValue() - modelValue();
            break;
          case ImpliedVolError: 
            {
              Real minVol = volatilityType_ == ShiftedLognormal ? 0.0010 : 0.00005;
              Real maxVol = volatilityType_ == ShiftedLognormal ? 10.0 : 0.50;
              const Real lowerPrice = blackPrice(minVol);
              const Real upperPrice = blackPrice(maxVol);
              const Real modelPrice = modelValue();

              Volatility implied;
              if (modelPrice <= lowerPrice)
                  implied = minVol;
              else if (modelPrice >= upperPrice)
                  implied = maxVol;
              else
                  implied = this->impliedVolatility(
                                          modelPrice, 1e-12, 5000, minVol, maxVol);
              error = implied - volatility_->value();
            }
            break;
          default:
            QL_FAIL("unknown Calibration Error Type");
        }
        
        return error;
    }
}
