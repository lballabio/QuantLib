
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
 *      http://quantlib.sourceforge.net/LICENSE.TXT
*/

/*! \file dividendeuropeanoption.cpp
    \brief european option with discrete deterministic dividends

    $Source$
    $Log$
    Revision 1.1  2001/02/14 13:51:38  marmar
    default  constructor has been moved to the cpp file

*/

#include "dividendeuropeanoption.h"

namespace QuantLib {

    namespace Pricers {

        DividendEuropeanOption::DividendEuropeanOption(
            Type type, double underlying, double strike, Rate dividendYield, 
            Rate riskFreeRate, Time residualTime, double volatility, 
            const std::vector<double>& dividends,
            const std::vector<Time>& exdivdates): 
            theDividends(dividends),theExDivDates(exdivdates),
            BSMEuropeanOption(type, underlying - riskless(riskFreeRate, 
                dividends, exdivdates), strike, dividendYield, 
                riskFreeRate, residualTime, volatility){
                
                QL_REQUIRE(theDividends.size()==theExDivDates.size(),
                    "the number of dividends is different from that of dates");
                for(int j = 0; j < theDividends.size(); j++){

                    QL_REQUIRE(theExDivDates[j] > 0, "The "+
                         IntegerFormatter::toString(j)+ "-th" +
                        "dividend date is not positive"    + "(" + 
                        DoubleFormatter::toString(theExDivDates[j]) + ")");
                        
                    QL_REQUIRE(theExDivDates[j] < residualTime,"The " + 
                        IntegerFormatter::toString(j) + "-th" +
                        "dividend date is greater than residual time" + "(" +
                        DoubleFormatter::toString(theExDivDates[j]) + ">" + 
                        DoubleFormatter::toString(residualTime)    + ")");
                }
            }

        double DividendEuropeanOption::rho() const{
        
            double tmp_rho = BSMEuropeanOption::rho();
            double delta_rho = 0.0;
            for(int j = 0; j < theDividends.size();j++)
                delta_rho += theExDivDates[j]*theDividends[j]*
                            QL_EXP(-theRiskFreeRate*theExDivDates[j]);
            return tmp_rho + delta_rho*BSMEuropeanOption::delta();
        }
        
    }

}

