
/*
 Copyright (C) 2000, 2001, 2002 RiskMap srl

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
/*! \file bsmfdoption.hpp
    \brief common code for numerical option evaluation

    \fullpath
    ql/Pricers/%bsmfdoption.hpp
*/

// $Id$

#ifndef BSM_numerical_option_pricer_h
#define BSM_numerical_option_pricer_h

#include <ql/Pricers/singleassetoption.hpp>
#include <ql/FiniteDifferences/bsmoperator.hpp>

namespace QuantLib {

    namespace Pricers {

        //! Black-Scholes-Merton option priced numerically
        class FdBsmOption : public SingleAssetOption {
          public:
            FdBsmOption(Option::Type type, double underlying,
                double strike, Spread dividendYield, Rate riskFreeRate,
                Time residualTime, double volatility, Size gridPoints);
            // accessors
            virtual void calculate() const = 0;
            double value() const;
            double delta() const;
            double gamma() const;
            double theta() const;
            Array getGrid() const{return grid_;}

          protected:
            // methods
            virtual void setGridLimits(double center,
                                       double timeDelay) const;
            virtual void initializeGrid() const;
            virtual void initializeInitialCondition() const;
            virtual void initializeOperator() const;
            // input data
            Size gridPoints_;
            // results
            mutable double value_, delta_, gamma_, theta_;

            mutable Array grid_;
            mutable FiniteDifferences::BSMOperator finiteDifferenceOperator_;
            mutable Array initialPrices_;
            // temporaries
            mutable double sMin_, center_, sMax_;
          private:
            // temporaries
            mutable double gridLogSpacing_;
            Size safeGridPoints(Size gridPoints,
            Time residualTime);
        };

        //! This is a safety check to be sure we have enough grid points.
        #define QL_NUM_OPT_MIN_GRID_POINTS            100
        //! This is a safety check to be sure we have enough grid points.
        #define QL_NUM_OPT_GRID_POINTS_PER_YEAR        50

        // The following is a safety check to be sure we have enough grid
        // points.
        inline Size FdBsmOption::safeGridPoints(
            Size gridPoints, Time residualTime) {
            return QL_MAX(gridPoints, residualTime>1.0 ?
                static_cast<Size>(
                    (QL_NUM_OPT_MIN_GRID_POINTS +
                    (residualTime-1.0) *
                    QL_NUM_OPT_GRID_POINTS_PER_YEAR))
                : QL_NUM_OPT_MIN_GRID_POINTS);
        }

    }

}


#endif
