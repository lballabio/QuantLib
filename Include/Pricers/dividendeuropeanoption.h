
/*
 * Copyright (C) 2000
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
 * QuantLib license is also available at http://quantlib.sourceforge.net/LICENSE.TXT
*/

#ifndef BSM_dividend_european_option_pricer_h
#define BSM_dividend_european_option_pricer_h

#include "qldefines.h"
#include "bsmeuropeanoption.h"
#include "dataformatters.h"
#include <vector>

namespace QuantLib {

	namespace Pricers {
	
		class DividendEuropeanOption : public BSMEuropeanOption {
            public:
            DividendEuropeanOption(Type type, double underlying, double strike, Rate underlyingGrowthRate, 
                Rate riskFreeRate, Time residualTime, double volatility, const std::vector<double>& dividends, 
                const std::vector<Time>& exdivdates)
                : BSMEuropeanOption(type, underlying - riskless(riskFreeRate,
                residualTime,dividends,exdivdates),strike, underlyingGrowthRate,
                riskFreeRate, residualTime, volatility) {}
            Handle<BSMOption> clone() const{
                return Handle<BSMOption>(new DividendEuropeanOption(*this));
            }
		  	double rho() const{
		  		Require(1==0,"method rho() not yet defined for DividendEuropeanOption"); 
		  		return 1234567890.0;
		  	}
            private:
            double riskless(Rate r, Time t, std::vector<double> divs,std::vector<Time> divDates){
                QuantLib::Require(divs.size()==divDates.size(),"the number of dividends is different from	that of	dates");
				unsigned int j;
				double z = 0.0;
				for(j=0; j<divs.size();j++){
                    QuantLib::Require(divDates[j]>0, "The " + IntegerFormatter::toString(j) + "-th" +
                        "dividend date is not positive" + "(" + DoubleFormatter::toString(divDates[j]) + ")");
                    QuantLib::Require(divDates[j]<t,"The " + IntegerFormatter::toString(j) + "-th" +
                        "dividend date is greater than residual time" + "(" +
                        DoubleFormatter::toString(divDates[j]) +
                        ">" + DoubleFormatter::toString(t) + ")");
					QuantLib::Require(divs[j]>=0,"The " + IntegerFormatter::toString(j) + "-th" +
                        "dividend is negative" + "(" + DoubleFormatter::toString(divs[j]) + ")");
                    z += divs[j]*QL_EXP(-r*divDates[j]);
                }
                return z;
            }
        };

    }

}

#endif
