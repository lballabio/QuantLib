/*
 * Copyright (C) 2001
 * Ferdinando Ametrano, Luigi Ballabio, Adolfo Benin, Marco Marchioro
 *
 * This file is part of QuantLib.
 * QuantLib is a C++ open source library for financial quantitative
 * analysts and developers --- http://quantlib.sourceforge.net/
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
 * if not, contact ferdinando@ametrano.net
 *
 * QuantLib license is also available at
 *   http://quantlib.sourceforge.net/LICENSE.TXT
*/

/*! \file dividendoption.cpp
    \brief base class for options with dividends

    $Source$
    $Log$
    Revision 1.2  2001/03/21 10:48:08  marmar
    valueAtCenter, firstDerivativeAtCenter, secondDerivativeAtCenter,
    are no longer methods of BSMNumericalOption but separate
    functions

    Revision 1.1  2001/03/20 15:27:38  marmar
    DividendOption and DividendShoutOption are examples of
     MultiPeriodOption's

*/

#include "dividendoption.h"
#include "cubicspline.h"
#include "dividendeuropeanoption.h"
#include "valueatcenter.h"

namespace QuantLib {

    namespace Pricers {

        using Math::CubicSpline;
        using FiniteDifferences::valueAtCenter;
        using FiniteDifferences::firstDerivativeAtCenter;
        using FiniteDifferences::secondDerivativeAtCenter;

        DividendOption::DividendOption(Type type, double underlying,
            double strike, Rate dividendYield, Rate riskFreeRate,
            Time residualTime, double volatility,
            const std::vector<double>& dividends,
            const std::vector<Time>& exdivdates, int timeSteps, int gridPoints)
        : dividends_(dividends),
          MultiPeriodOption(type, underlying - addElements(dividends),
          strike, dividendYield, riskFreeRate, residualTime, volatility,
          exdivdates, timeSteps, gridPoints) {

            QL_REQUIRE(dateNumber_ == dividends_.size(),
                "the number of dividends is different "
                "from the number of dates");

            QL_REQUIRE(underlying - addElements(dividends)>0,
                "Dividends cannot exceed underlying");                
                
            analitic_ = Handle<BSMOption> (new DividendEuropeanOption (type_,
                underlying_ + addElements(dividends_), strike_,
                dividendYield_, riskFreeRate_, residualTime_,
                volatility_, dividends_, dates_));
        }

        void DividendOption::executeIntermediateStep(int step) const{
            Array oldGrid = grid_;
            double centre = valueAtCenter(grid_);
            double mltp = centre/grid_[0];
            double newMltp = mltp / (1 + (mltp - 1) *
                dividends_[step] / (centre + dividends_[step]));
            QL_ENSURE(newMltp > 1, "Dividends are to big");
            sMin_ = (centre + dividends_[step])/newMltp;
            sMax_ = (centre + dividends_[step])*newMltp;
            initializeGrid();
            initializeInitialCondition();
            initializeOperator();
            movePricesBeforeExDiv(dividends_[step],
                grid_, prices_, oldGrid);
            movePricesBeforeExDiv(dividends_[step],
                grid_, controlPrices_, oldGrid);
        }

        void DividendOption::movePricesBeforeExDiv(double Div,
            const Array& newGrid, Array& prices, const Array& oldGrid) const {

            Array vOldGrid(oldGrid + Div);
            CubicSpline<Array::iterator, Array::iterator> priceSpline(
                vOldGrid.begin(), vOldGrid.end(), prices.begin());

            for (int j = 0; j < prices.size(); j++)
                prices[j] = QL_MAX(priceSpline(newGrid[j]),
                                   initialPrices_[j]      );
        }

    }

}
