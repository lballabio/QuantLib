
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

/*! \file bsmnumericaloption.hpp
    \brief common code for numerical option evaluation

    \fullpath
    ql/Pricers/%bsmnumericaloption.hpp
*/

// $Id$

#ifndef BSM_numerical_option_pricer_h
#define BSM_numerical_option_pricer_h

#include "ql/Pricers/singleassetoption.hpp"
#include "ql/FiniteDifferences/bsmoperator.hpp"

namespace QuantLib {

    namespace Pricers {

        //! Black-Scholes-Merton option priced numerically
        class BSMNumericalOption : public SingleAssetOption {
          public:
            BSMNumericalOption(Option::Type type, double underlying, 
                double strike, Rate dividendYield, Rate riskFreeRate, 
                Time residualTime, double volatility, size_t gridPoints);
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
            size_t gridPoints_;
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
            size_t safeGridPoints(size_t gridPoints, 
            Time residualTime);
        };

        //! This is a safety check to be sure we have enough grid points.
        #define QL_NUM_OPT_MIN_GRID_POINTS            100
        //! This is a safety check to be sure we have enough grid points.
        #define QL_NUM_OPT_GRID_POINTS_PER_YEAR        50

        // The following is a safety check to be sure we have enough grid
        // points.
        inline size_t BSMNumericalOption::safeGridPoints(
            size_t gridPoints, Time residualTime) {
            return QL_MAX(gridPoints, residualTime>1.0 ?
                static_cast<size_t>(
                    (QL_NUM_OPT_MIN_GRID_POINTS + 
                    (residualTime-1.0) * 
                    QL_NUM_OPT_GRID_POINTS_PER_YEAR))
                : QL_NUM_OPT_MIN_GRID_POINTS);
        }

    }

}


#endif
