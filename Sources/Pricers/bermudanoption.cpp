
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

/*! \file bermudanoption.cpp
    \brief  class for finite-difference evaluation of bermudan option

    $Source$
    $Log$
    Revision 1.5  2001/04/05 07:57:46  marmar
    One bug fixed in bermudan option, theta, rho, and vega  still not working

    Revision 1.4  2001/04/04 17:13:37  nando
    bat file was not updated with Luigi's shuffling

    Revision 1.3  2001/04/04 12:13:24  nando
    Headers policy part 2:
    The Include directory is added to the compiler's include search path.
    Then both your code and user code specifies the sub-directory in
    #include directives, as in
    #include <Solvers1d/newton.hpp>

    Revision 1.2  2001/04/04 11:07:24  nando
    Headers policy part 1:
    Headers should have a .hpp (lowercase) filename extension
    All *.h renamed to *.hpp

    Revision 1.1  2001/03/21 09:52:19  marmar
    simple BermudanOption pricer added

*/

#include "Pricers/bermudanoption.hpp"
#include "Pricers/bsmeuropeanoption.hpp"

namespace QuantLib {

    namespace Pricers {

        BermudanOption::BermudanOption(Type type, double underlying,
            double strike, Rate dividendYield, Rate riskFreeRate,
            Time residualTime, double volatility,
            const std::vector<Time>& dates,
            int timeSteps, int gridPoints)
        : MultiPeriodOption(type, underlying, strike, dividendYield,
          riskFreeRate, residualTime, volatility, dates, timeSteps,
          gridPoints) {

            analitic_ = Handle<BSMOption> (new BSMEuropeanOption (
                            type_, underlying_, strike_, dividendYield_,
                            riskFreeRate_, residualTime_, volatility_));
        }

        using FiniteDifferences::StandardStepCondition;

        void BermudanOption::initializeStepCondition() const{
            stepCondition_ = Handle<StandardStepCondition> ();
        }

        void BermudanOption::executeIntermediateStep(int step) const{

            // to avoid warning
            step;
            int size = initialPrices_.size();
            for(int j = 0; j < size; j++)
                prices_[j] = QL_MAX(prices_[j], initialPrices_[j]);            
        }

    }

}
