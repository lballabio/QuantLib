
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
#include <vector>

namespace QuantLib {

	namespace Pricers {
		double DividendEuropeanOption_riskless(Rate r, Time t, std::vector<double> divs,std::vector<Time> divDates);
	
		class DividendEuropeanOption : public BSMEuropeanOption {
		  public:
				DividendEuropeanOption(Type type, double underlying, double strike, Rate underlyingGrowthRate, 
				  Rate riskFreeRate, Time residualTime, double volatility, const std::vector<double>& dividends, 
				  const std::vector<Time>& exdivdates):
					BSMEuropeanOption(type, underlying - DividendEuropeanOption_riskless(riskFreeRate,residualTime,dividends,exdivdates), strike, underlyingGrowthRate, riskFreeRate, residualTime, volatility){} 
				Handle<BSMOption> clone() const{ 
						return Handle<BSMOption>(new DividendEuropeanOption(*this)); 
				}
		  	double rho() const{
		  			Require(1==0,"method rho() not defined for DividendEuropeanOption"); 
		  			return 1234567890.0;
		  	}
		};
		namespace DividendEuropeanOptionFunction {
  
				double riskless(Rate r, Time t, std::vector<double> divs,std::vector<Time> divDates){
					Require(divs.size()==divDates.size(),"the number of dividends is diffrent from	that of	dates");
					Require(divs.size()>=1,"the	number of dividends must be at least one");
					Require(divDates[0]>0,"The	ex dividend dates must be positive");
					Require(divDates[divDates.size()-1]<t,"The	x dividend dates must be within the residual time");
					Require(divs[0]>=0,"Dividends cannot be	negative");
					unsigned int	j;
					for(j=1; j<divs.size();j++){
						Require(double(divDates[j-1])<double(divDates[j]),"Dividend dates must be in increasing order");
						Require(divs[j]>=0,"DividendEuropeanOption: Dividends cannot be negative");
					}
					double z = 0.0;
					for(j=0;j<divs.size();j++)
						z += divs[j]*QL_EXP(-r*divDates[j]);
					return z;
				}
			}

	}

}

#endif
