
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

/*! \file bsmnumericaloption.cpp
    \brief common code for numerical option evaluation

    \fullpath
    ql/Pricers/%bsmnumericaloption.cpp
*/

// $Id$

#include <ql/Pricers/bsmnumericaloption.hpp>
#include <ql/FiniteDifferences/valueatcenter.hpp>

namespace QuantLib {

    namespace Pricers {

        using FiniteDifferences::BoundaryCondition;
        using FiniteDifferences::BSMOperator;
        using FiniteDifferences::valueAtCenter;
        using FiniteDifferences::firstDerivativeAtCenter;
        using FiniteDifferences::secondDerivativeAtCenter;

        BSMNumericalOption::BSMNumericalOption(Option::Type type,
            double underlying, double strike, Spread dividendYield,
            Rate riskFreeRate, Time residualTime, double volatility,
            size_t gridPoints)
        : SingleAssetOption(type, underlying, strike, dividendYield,
            riskFreeRate, residualTime, volatility),
            gridPoints_(safeGridPoints(gridPoints, residualTime)),
            grid_(gridPoints_), initialPrices_(gridPoints_){
                hasBeenCalculated_ = false;
        }

        double BSMNumericalOption::value() const {
            if (!hasBeenCalculated_)
                calculate();
            return value_;
        }

        double BSMNumericalOption::delta() const {
            if (!hasBeenCalculated_)
                calculate();
            return delta_;
        }

        double BSMNumericalOption::gamma() const {
            if(!hasBeenCalculated_)
                calculate();
            return gamma_;
        }

        double BSMNumericalOption::theta() const {
            if(!hasBeenCalculated_)
                calculate();
            return theta_;
        }

        void BSMNumericalOption::setGridLimits(double center,
                                               double timeDelay) const {

            center_ = center;
            double volSqrtTime = volatility_*QL_SQRT(timeDelay);
            // the prefactor fine tunes performance at small volatilities
            double prefactor = 1.0 + 0.02/volSqrtTime;
            double minMaxFactor = QL_EXP(4.0 * prefactor * volSqrtTime);
            sMin_ = center_/minMaxFactor;  // underlying grid min value
            sMax_ = center_*minMaxFactor;  // underlying grid max value
            // insure strike is included in the grid
            double safetyZoneFactor = 1.1;
            if(sMin_ > strike_/safetyZoneFactor){
                sMin_ = strike_/safetyZoneFactor;
                // enforce central placement of the underlying
                sMax_ = center_/(sMin_/center_);
            }
            if(sMax_ < strike_*safetyZoneFactor){
                sMax_ = strike_*safetyZoneFactor;
                // enforce central placement of the underlying
                sMin_ = center_/(sMax_/center_);
            }
        }

        void BSMNumericalOption::initializeGrid() const {
            gridLogSpacing_ = (QL_LOG(sMax_)-QL_LOG(sMin_))/(gridPoints_-1);
            double edx = QL_EXP(gridLogSpacing_);
            grid_[0] = sMin_;
            size_t j;
            for (j=1; j<gridPoints_; j++)
                grid_[j] = grid_[j-1]*edx;
        }

        void BSMNumericalOption::initializeInitialCondition() const {
            size_t j;
            switch (type_) {
              case Option::Call:
                for(j = 0; j < gridPoints_; j++)
                    initialPrices_[j] = QL_MAX(grid_[j]-strike_,0.0);
                break;
              case Option::Put:
                for(j = 0; j < gridPoints_; j++)
                    initialPrices_[j] = QL_MAX(strike_-grid_[j],0.0);
                break;
              case Option::Straddle:
                for(j = 0; j < gridPoints_; j++)
                    initialPrices_[j] = QL_FABS(strike_-grid_[j]);
                break;
              default:
                throw Error("BSMNumericalOption: invalid option type");
            }
        }

        void BSMNumericalOption::initializeOperator() const {
            finiteDifferenceOperator_ = BSMOperator(gridPoints_,
                gridLogSpacing_, riskFreeRate_, dividendYield_, volatility_);

            finiteDifferenceOperator_.setLowerBC(
                BoundaryCondition(BoundaryCondition::Neumann,
                    initialPrices_[1]-initialPrices_[0]));

            finiteDifferenceOperator_.setUpperBC(
                BoundaryCondition(BoundaryCondition::Neumann,
                    initialPrices_[gridPoints_-1] -
                        initialPrices_[gridPoints_-2]));
        }

    }

}


