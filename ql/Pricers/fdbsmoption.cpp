
/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email quantlib-dev@lists.sf.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/Pricers/fdbsmoption.hpp>
#include <ql/FiniteDifferences/valueatcenter.hpp>

namespace QuantLib {

    FdBsmOption::FdBsmOption(
                         Option::Type type, double underlying, double strike, 
                         Spread dividendYield, Rate riskFreeRate, 
                         Time residualTime, double volatility, Size gridPoints)
    : SingleAssetOption(type, underlying, strike, dividendYield,
                        riskFreeRate, residualTime, volatility),
      gridPoints_(safeGridPoints(gridPoints, residualTime)),
      grid_(gridPoints_), intrinsicValues_(gridPoints_),
      BCs_(2) {
        QL_REQUIRE(volatility > 0.0,
                   "negative or null volatility");
        hasBeenCalculated_ = false;
    }

    double FdBsmOption::value() const {
        if (!hasBeenCalculated_)
            calculate();
        return value_;
    }

    double FdBsmOption::delta() const {
        if (!hasBeenCalculated_)
            calculate();
        return delta_;
    }

    double FdBsmOption::gamma() const {
        if(!hasBeenCalculated_)
            calculate();
        return gamma_;
    }

    void FdBsmOption::setGridLimits(double center,
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
        if(sMin_ > payoff_.strike()/safetyZoneFactor){
            sMin_ = payoff_.strike()/safetyZoneFactor;
            // enforce central placement of the underlying
            sMax_ = center_/(sMin_/center_);
        }
        if(sMax_ < payoff_.strike()*safetyZoneFactor){
            sMax_ = payoff_.strike()*safetyZoneFactor;
            // enforce central placement of the underlying
            sMin_ = center_/(sMax_/center_);
        }
    }

    void FdBsmOption::initializeGrid() const {
        gridLogSpacing_ = (QL_LOG(sMax_)-QL_LOG(sMin_))/(gridPoints_-1);
        double edx = QL_EXP(gridLogSpacing_);
        grid_[0] = sMin_;
        Size j;
        for (j=1; j<gridPoints_; j++)
            grid_[j] = grid_[j-1]*edx;
    }

    void FdBsmOption::initializeInitialCondition() const {
        Size j;
        for(j = 0; j < gridPoints_; j++)
            intrinsicValues_[j] = payoff_(grid_[j]);
    }

    void FdBsmOption::initializeOperator() const {
        finiteDifferenceOperator_ = BSMOperator(gridPoints_, gridLogSpacing_, 
                                                riskFreeRate_, dividendYield_, 
                                                volatility_);

        BCs_[0] = boost::shared_ptr<BoundaryCondition>(new NeumannBC(
                  intrinsicValues_[1]-intrinsicValues_[0], NeumannBC::Lower));
        BCs_[1] = boost::shared_ptr<BoundaryCondition>(new NeumannBC(
            intrinsicValues_[gridPoints_-1] - intrinsicValues_[gridPoints_-2],
            NeumannBC::Upper));
    }

}


