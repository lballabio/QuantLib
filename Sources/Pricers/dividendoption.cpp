
/*
 * Copyright (C) 2000-2001 QuantLib Group
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
 * The license is also available at http://quantlib.sourceforge.net/LICENSE.TXT
 *
 * The members of the QuantLib Group are listed in the Authors.txt file, also
 * available at http://quantlib.sourceforge.net/Authors.txt
*/

/*! \file dividendoption.cpp
    \brief base class for options with dividends

    $Source$
    $Log$
    Revision 1.7  2001/04/09 14:13:34  nando
    all the *.hpp moved below the Include/ql level

    Revision 1.6  2001/04/06 18:46:22  nando
    changed Authors, Contributors, Licence and copyright header

    Revision 1.5  2001/04/06 16:12:18  marmar
    Bug fixed in multi-period option

    Revision 1.4  2001/04/04 12:13:24  nando
    Headers policy part 2:
    The Include directory is added to the compiler's include search path.
    Then both your code and user code specifies the sub-directory in
    #include directives, as in
    #include <Solvers1d/newton.hpp>

    Revision 1.3  2001/04/04 11:07:24  nando
    Headers policy part 1:
    Headers should have a .hpp (lowercase) filename extension
    All *.h renamed to *.hpp

    Revision 1.2  2001/03/21 10:48:08  marmar
    valueAtCenter, firstDerivativeAtCenter, secondDerivativeAtCenter,
    are no longer methods of BSMNumericalOption but separate
    functions

    Revision 1.1  2001/03/20 15:27:38  marmar
    DividendOption and DividendShoutOption are examples of
     MultiPeriodOption's

*/

#include "ql/Pricers/dividendoption.hpp"
#include "ql/Math/cubicspline.hpp"
#include "ql/Pricers/dividendeuropeanoption.hpp"
#include "ql/FiniteDifferences/valueatcenter.hpp"

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
        }

        void DividendOption::initializeControlVariate() const{
            analytic_ = Handle<BSMOption> (new DividendEuropeanOption (type_,
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
