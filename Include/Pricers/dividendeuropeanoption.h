
     
/*
 * Copyright (C) 2000, 2001
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

/*! \file dividendeuropeanoption.h
    \brief european option with discrete deterministic dividends

    $Source$
    $Name$
    $Log$
    Revision 1.11  2001/02/14 13:53:19  marmar
    default  constructor has been moved to the cpp file

    Revision 1.10  2001/02/13 10:43:55  marmar
    Efficency improved. Also, dividends do not have to be positive
    to allow for negative cash flows

    Revision 1.9  2001/02/13 10:02:17  marmar
    Ambiguous variable name underlyingGrowthRate changed in
    unambiguos dividendYield

    Revision 1.8  2001/01/17 14:37:56  nando
    tabs removed

    Revision 1.7  2000/12/27 14:05:56  lballabio
    Turned Require and Ensure functions into QL_REQUIRE and QL_ENSURE macros

    Revision 1.6  2000/12/14 12:32:30  lballabio
    Added CVS tags in Doxygen file documentation blocks

*/

#ifndef BSM_dividend_european_option_pricer_h
#define BSM_dividend_european_option_pricer_h

#include "qldefines.h"
#include "bsmeuropeanoption.h"
#include "dataformatters.h"
#include <vector>

namespace QuantLib {

    namespace Pricers {

        class DividendEuropeanOption : public BSMEuropeanOption    {
        public:
            DividendEuropeanOption(Type type, double underlying, double strike, 
                Rate dividendYield, Rate riskFreeRate, Time residualTime, 
                double volatility, const std::vector<double>& dividends,
                const std::vector<Time>& exdivdates);
            double rho() const;
            Handle<BSMOption> clone() const{
                return Handle<BSMOption>(new DividendEuropeanOption(*this));
            }
            private:
              std::vector<double> theDividends;
              std::vector<Time> theExDivDates;
              double riskless(Rate r, std::vector<double> divs, 
                                      std::vector<Time> divDates) const;
        };
        
        inline double DividendEuropeanOption::riskless(Rate r,
            std::vector<double> divs, std::vector<Time> divDates) const{
                
            double tmp_riskless = 0.0;
            for(int j = 0; j < divs.size(); j++)
                tmp_riskless += divs[j]*QL_EXP(-r*divDates[j]);
            return tmp_riskless;
        }

    }

}

#endif
