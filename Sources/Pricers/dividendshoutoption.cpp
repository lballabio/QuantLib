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

/*! \file dividendshoutoption.cpp
    \brief base class for shout options with dividends

    $Source$
    $Log$
    Revision 1.1  2001/03/20 15:27:38  marmar
    DividendOption and DividendShoutOption are examples of
     MultiPeriodOption's

*/

#include "dividendshoutoption.h"

namespace QuantLib {

    namespace Pricers {
        
        DividendShoutOption::DividendShoutOption(Type type, 
                double underlying, double strike, Rate dividendYield, 
                Rate riskFreeRate, Time residualTime, double volatility, 
                const std::vector<double>& dividends, 
                const std::vector<Time>& exdivdates,
                int timeSteps, int gridPoints)
       : DividendOption(type, underlying, strike, dividendYield, 
                        riskFreeRate, residualTime, volatility, 
                        dividends, exdivdates, timeSteps, gridPoints){}

        void DividendShoutOption::initializeStepCondition() const {
            stepCondition_ = Handle<FiniteDifferences::StandardStepCondition>(
                             new ShoutCondition(initialPrices_, residualTime_,
                                                riskFreeRate_));   
        }

    }

}
