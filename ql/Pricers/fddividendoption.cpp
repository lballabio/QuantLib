
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

/*! \file dividendoption.cpp
    \brief base class for options with dividends

    \fullpath
    ql/Pricers/%dividendoption.cpp
*/

// $Id$

#include <ql/Pricers/fddividendoption.hpp>
#include <ql/Math/cubicspline.hpp>
#include <ql/Pricers/fddividendeuropeanoption.hpp>
#include <ql/FiniteDifferences/valueatcenter.hpp>
#include <iterator>

namespace QuantLib {

    namespace Pricers {

        using Math::CubicSpline;
        using FiniteDifferences::valueAtCenter;

        FdDividendOption::FdDividendOption(Option::Type type, double underlying,
            double strike, Spread dividendYield, Rate riskFreeRate,
            Time residualTime, double volatility,
            const std::vector<double>& dividends,
            const std::vector<Time>& exdivdates,
            int timeSteps, int gridPoints)
        : FdMultiPeriodOption(type, underlying - addElements(dividends),
          strike, dividendYield, riskFreeRate, residualTime, volatility,
          gridPoints, exdivdates, timeSteps), dividends_(dividends) {

            QL_REQUIRE(dateNumber_ == dividends.size(),
                       "the number of dividends(" +
                       IntegerFormatter::toString(dividends.size()) +
                       ") is different from the number of dates(" +
                       IntegerFormatter::toString(dateNumber_) +
                       ")");

            QL_REQUIRE(underlying > addElements(dividends),
                       "Dividends(" +
                       DoubleFormatter::toString(underlying - underlying_) +
                       ") cannot exceed underlying(" +
                       DoubleFormatter::toString(underlying) +
                       ")");
        }

        void FdDividendOption::initializeControlVariate() const{
            analytic_ = Handle<SingleAssetOption> (new
                            FdDividendEuropeanOption (type_,
                                underlying_ + addElements(dividends_),
                                strike_,
                                dividendYield_,
                                riskFreeRate_,
                                residualTime_,
                                volatility_,
                                dividends_,
                                dates_));
        }

        void FdDividendOption::executeIntermediateStep(int step) const{

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

        void FdDividendOption::movePricesBeforeExDiv(
                Array& prices, const Array& newGrid,
                               const Array& oldGrid) const {

            int j, gridSize = oldGrid.size();

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

            CubicSpline<std::vector<double>::iterator,
              std::vector<double>::iterator> priceSpline(
                logOldGrid.begin(), logOldGrid.end(), tmpPrices.begin());

            for (j = 0; j < gridSize; j++)
                prices[j] = priceSpline(QL_LOG(newGrid[j]));

        }

    }

}
