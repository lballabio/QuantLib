
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

namespace QuantLib {

    FdDividendOption::FdDividendOption(
                       Option::Type type, Real underlying,
                       Real strike, Spread dividendYield, Rate riskFreeRate,
                       Time residualTime, Volatility volatility,
                       const std::vector<Real>& dividends,
                       const std::vector<Time>& exdivdates,
                       Size timeSteps, Size gridPoints)
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
                   DecimalFormatter::toString(underlying - underlying_) +
                   ") cannot exceed underlying(" +
                   DecimalFormatter::toString(underlying) +
                   ")");
    }

    void FdDividendOption::initializeControlVariate() const{
        Real riskless = 0.0;
        for (Size i=0; i<dividends_.size(); i++)
            riskless += dividends_[i]*std::exp(-riskFreeRate_*dates_[i]);
        Real spot = underlying_ + addElements(dividends_) - riskless;
        DiscountFactor discount = std::exp(-riskFreeRate_*residualTime_);
        DiscountFactor qDiscount = std::exp(-dividendYield_*residualTime_);
        Real forward = spot*qDiscount/discount;
        Real variance = volatility_*volatility_*residualTime_;
        boost::shared_ptr<StrikedTypePayoff> payoff(
                                             new PlainVanillaPayoff(payoff_));
        // theta, rho, and dividend rho should be corrected. However,
        // the control variate machinery won't use them.
        analytic_ = boost::shared_ptr<BlackFormula>(
                       new BlackFormula(forward, discount, variance, payoff));
    }

    void FdDividendOption::executeIntermediateStep(Size step) const{

        Real newSMin = sMin_ + dividends_[step];
        Real newSMax = sMax_ + dividends_[step];

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

        std::vector<Real> logOldGrid(0);
        std::vector<Real> tmpPrices(0);

        for(j = 0; j<gridSize; j++){
            Real p = prices[j];
            Real g = oldGrid[j];
            if (g > 0){
                logOldGrid.push_back(std::log(g));
                tmpPrices.push_back(p);
            }
        }

        NaturalCubicSpline priceSpline(logOldGrid.begin(), logOldGrid.end(),
                                       tmpPrices.begin());
        for (j = 0; j < gridSize; j++)
            prices[j] = priceSpline(std::log(newGrid[j]), true);

    }

}
