
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

#include <ql/Pricers/fddividendoption.hpp>
#include <ql/Math/cubicspline.hpp>
#include <ql/FiniteDifferences/valueatcenter.hpp>
#include <iterator>

namespace QuantLib {

    FdDividendOption::FdDividendOption(
                       Option::Type type, double underlying,
                       double strike, Spread dividendYield, Rate riskFreeRate,
                       Time residualTime, double volatility,
                       const std::vector<double>& dividends,
                       const std::vector<Time>& exdivdates,
                       int timeSteps, int gridPoints)
    : FdMultiPeriodOption(type, underlying - addElements(dividends),
                          strike, dividendYield, riskFreeRate,
                          residualTime, volatility,
                          gridPoints, exdivdates, timeSteps),
      dividends_(dividends) {

        QL_REQUIRE(dateNumber_ == dividends.size(),
                   "the number of dividends(" +
                   SizeFormatter::toString(dividends.size()) +
                   ") is different from the number of dates(" +
                   SizeFormatter::toString(dateNumber_) +
                   ")");

        QL_REQUIRE(underlying > addElements(dividends),
                   "dividends(" +
                   DoubleFormatter::toString(underlying - underlying_) +
                   ") cannot exceed underlying(" +
                   DoubleFormatter::toString(underlying) +
                   ")");
    }

    void FdDividendOption::initializeControlVariate() const{
        double riskless = 0.0;
        for (Size i=0; i<dividends_.size(); i++)
            riskless += dividends_[i]*QL_EXP(-riskFreeRate_*dates_[i]);
        double spot = underlying_ + addElements(dividends_) - riskless;
        double discount = QL_EXP(-riskFreeRate_*residualTime_);
        double qDiscount = QL_EXP(-dividendYield_*residualTime_);
        double forward = spot*qDiscount/discount;
        double variance = volatility_*volatility_*residualTime_;
        boost::shared_ptr<StrikedTypePayoff> payoff(
                                             new PlainVanillaPayoff(payoff_));
        // theta, rho, and dividend rho should be corrected. However,
        // the control variate machinery won't use them.
        analytic_ = boost::shared_ptr<BlackFormula>(
                       new BlackFormula(forward, discount, variance, payoff));
    }

    void FdDividendOption::executeIntermediateStep(Size step) const{

        double newSMin = sMin_ + dividends_[step];
        double newSMax = sMax_ + dividends_[step];

        setGridLimits(center_ + dividends_[step], dates_[step]);
        if (sMin_ < newSMin){
            sMin_ = newSMin;
            sMax_ = center_/(sMin_/center_);
        }
        if (sMax_ > newSMax){
            sMax_ = newSMax;
            sMin_ = center_/(sMax_/center_);
        }
        Array oldGrid = grid_ + dividends_[step];

        initializeGrid();
        initializeInitialCondition();
        // This operation was faster than the obvious:
        //     movePricesBeforeExDiv(initialPrices_, grid_, oldGrid);

        movePricesBeforeExDiv(prices_,        grid_, oldGrid);
        movePricesBeforeExDiv(controlPrices_, grid_, oldGrid);
        initializeOperator();
        initializeModel();
        initializeStepCondition();
        stepCondition_ -> applyTo(prices_, dates_[step]);
    }

    void FdDividendOption::movePricesBeforeExDiv(Array& prices,
                                                 const Array& newGrid,
                                                 const Array& oldGrid) const {
        Size j, gridSize = oldGrid.size();

        std::vector<double> logOldGrid(0);
        std::vector<double> tmpPrices(0);

        for(j = 0; j<gridSize; j++){
            double p = prices[j];
            double g = oldGrid[j];
            if (g > 0){
                logOldGrid.push_back(QL_LOG(g));
                tmpPrices.push_back(p);
            }
        }

        NaturalCubicSpline priceSpline(logOldGrid.begin(), logOldGrid.end(),
                                       tmpPrices.begin());
        for (j = 0; j < gridSize; j++)
            prices[j] = priceSpline(QL_LOG(newGrid[j]), true);

    }

}
