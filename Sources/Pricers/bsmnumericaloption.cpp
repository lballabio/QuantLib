
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
    Sources/Pricers/%bsmnumericaloption.cpp
*/

// $Id$
// $Log$
// Revision 1.51  2001/08/31 15:23:47  sigmud
// refining fullpath entries for doxygen documentation
//
// Revision 1.50  2001/08/28 14:47:46  nando
// unsigned int instead of int
//
// Revision 1.49  2001/08/09 14:59:48  sigmud
// header modification
//
// Revision 1.48  2001/08/07 11:25:55  sigmud
// copyright header maintenance
//
// Revision 1.47  2001/08/06 15:43:34  nando
// BSMOption now is SingleAssetOption
// BSMEuropeanOption now is EuropeanOption
//
// Revision 1.46  2001/07/27 07:46:01  nando
// pruned warnings
//
// Revision 1.45  2001/07/25 15:47:29  sigmud
// Change from quantlib.sourceforge.net to quantlib.org
//
// Revision 1.44  2001/05/24 15:40:10  nando
// smoothing #include xx.hpp and cutting old Log messages
//

#include "ql/Pricers/bsmnumericaloption.hpp"
#include "ql/FiniteDifferences/valueatcenter.hpp"

namespace QuantLib {

    namespace Pricers {

        using FiniteDifferences::BoundaryCondition;
        using FiniteDifferences::BSMOperator;
        using FiniteDifferences::valueAtCenter;
        using FiniteDifferences::firstDerivativeAtCenter;
        using FiniteDifferences::secondDerivativeAtCenter;

        BSMNumericalOption::BSMNumericalOption(BSMNumericalOption::Type type,
            double underlying, double strike, Rate dividendYield,
            Rate riskFreeRate, Time residualTime, double volatility,
            unsigned int gridPoints)
        : SingleAssetOption(type, underlying, strike, dividendYield, riskFreeRate,
            residualTime, volatility),
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
            unsigned int j;
            for (j=1; j<gridPoints_; j++)
                grid_[j] = grid_[j-1]*edx;
        }

        void BSMNumericalOption::initializeInitialCondition() const {
            unsigned int j;
            switch (type_) {
              case Call:
                for(j = 0; j < gridPoints_; j++)
                    initialPrices_[j] = QL_MAX(grid_[j]-strike_,0.0);
                break;
              case Put:
                for(j = 0; j < gridPoints_; j++)
                    initialPrices_[j] = QL_MAX(strike_-grid_[j],0.0);
                break;
              case Straddle:
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

            finiteDifferenceOperator_.setHigherBC(
                BoundaryCondition(BoundaryCondition::Neumann,
                    initialPrices_[gridPoints_-1] -
                        initialPrices_[gridPoints_-2]));
        }

    }

}


