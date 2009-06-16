/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2001, 2002, 2003 Sadruddin Rejeb

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/models/calibrationhelper.hpp>
#include <ql/math/solvers1d/brent.hpp>

namespace QuantLib {

    class CalibrationHelper::ImpliedVolatilityHelper {
      public:
        ImpliedVolatilityHelper(const CalibrationHelper& helper,
                                Real value)
        : helper_(helper), value_(value) {}

        Real operator()(Volatility x) const {
            return value_ - helper_.blackPrice(x);
        }
      private:
        const CalibrationHelper& helper_;
        Real value_;
    };

    Volatility CalibrationHelper::impliedVolatility(Real targetValue,
                                                    Real accuracy,
                                                    Size maxEvaluations,
                                                    Volatility minVol,
                                                    Volatility maxVol) const {

        ImpliedVolatilityHelper f(*this,targetValue);
        Brent solver;
        solver.setMaxEvaluations(maxEvaluations);
        return solver.solve(f,accuracy,volatility_->value(),minVol,maxVol);
    }

    Real CalibrationHelper::calibrationError() {
        double error;
        
        switch (calibrationErrorType_) {
          case RelativePriceError:
            error = std::fabs(marketValue() - modelValue())/marketValue();
            break;
          case PriceError:
            error = marketValue() - modelValue();
            break;
          case ImpliedVolError: 
            {
              const Real lowerPrice = blackPrice(0.001);
              const Real upperPrice = blackPrice(10);
              const Real modelPrice = modelValue();

              Volatility implied;
              if (modelPrice <= lowerPrice)
                  implied = 0.001;
              else
                  if (modelPrice >= upperPrice)
                      implied = 10.0;
                  else
                      implied = this->impliedVolatility(
                                          modelPrice, 1e-12, 5000, 0.001, 10);
              error = implied - volatility_->value();
            }
            break;
          default:
            QL_FAIL("unknown Calibration Error Type");
        }
        
        return error;
    }
}
