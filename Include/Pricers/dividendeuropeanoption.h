
/*
 * Copyright (C) 2000
 * Ferdinando Ametrano,	Luigi Ballabio,	Adolfo Benin, Marco	Marchioro
 * 
 * This	file is	part of	QuantLib.
 * QuantLib	is a C++ open source library for financial quantitative
 * analysts	and	developers --- http://quantlib.sourceforge.net/
 *
 * QuantLib	is free	software and you are allowed to	use, copy, modify, merge,
 * publish,	distribute,	and/or sell	copies of it under the conditions stated 
 * in the QuantLib License.
 *
 * This	program	is distributed in the hope that	it will	be useful, but 
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A	PARTICULAR PURPOSE.	See	the	license	for	more details.
 *
 * You should have received	a copy of the license along	with this file;
 * if not, contact ferdinando@ametrano.net
 *
 * QuantLib	license	is also	available at http://quantlib.sourceforge.net/LICENSE.TXT
*/

/*! \file dividendeuropeanoption.h
	\brief european option with discrete dividends
	
	$Source$
	$Name$
	$Log$
	Revision 1.7  2000/12/27 14:05:56  lballabio
	Turned Require and Ensure functions into QL_REQUIRE and QL_ENSURE macros

	Revision 1.6  2000/12/14 12:32:30  lballabio
	Added CVS tags in Doxygen file documentation blocks
	
*/

#ifndef	BSM_dividend_european_option_pricer_h
#define	BSM_dividend_european_option_pricer_h

#include "qldefines.h"
#include "bsmeuropeanoption.h"
#include "dataformatters.h"
#include <vector>

namespace QuantLib {

	namespace Pricers {
	
		class DividendEuropeanOption : public BSMEuropeanOption	{
    		public:
        	  DividendEuropeanOption(Type	type, double underlying, double	strike,	Rate underlyingGrowthRate, 
        			Rate riskFreeRate, Time	residualTime, double volatility, const std::vector<double>&	dividends, 
        			const std::vector<Time>& exdivdates): theDividends(dividends),theExDivDates(exdivdates),
        			BSMEuropeanOption(type, underlying - riskless(riskFreeRate, residualTime,dividends,exdivdates),
        			strike, underlyingGrowthRate,riskFreeRate,residualTime,volatility){
        			    
		    		QL_REQUIRE(theDividends.size()==theExDivDates.size(),"the number of dividends is different from that of dates");
        			for(unsigned int j=0; j<theDividends.size();j++){
    					QL_REQUIRE(theExDivDates[j]>0, "The "	+ IntegerFormatter::toString(j)	+ "-th"	+
    				        "dividend date is not positive"	+ "(" +	DoubleFormatter::toString(theExDivDates[j]) + ")");
    			        QL_REQUIRE(theExDivDates[j]<residualTime,"The " + IntegerFormatter::toString(j) + "-th" +
    				        "dividend date is greater than residual	time" +	"("	+
    				        DoubleFormatter::toString(theExDivDates[j]) + ">" + DoubleFormatter::toString(residualTime)	+ ")");
    					QL_REQUIRE(theDividends[j]>=0,"The	" +	IntegerFormatter::toString(j) +	"-th" +
    				        "dividend is negative" + "(" + DoubleFormatter::toString(theDividends[j]) +	")");
    		        }
        		}
        		Handle<BSMOption> clone() const{
        			return Handle<BSMOption>(new DividendEuropeanOption(*this));
        		}
    			double rho() const{    			    
    			    double tmp_rho = BSMEuropeanOption::rho();
    			    double delta_rho = 0.0;
    				for(unsigned int j=0; j<theDividends.size();j++)
    			        delta_rho += theExDivDates[j]*theDividends[j]*QL_EXP(-theRiskFreeRate*theExDivDates[j]);
    				return tmp_rho + delta_rho*BSMEuropeanOption::delta();
    			}
			private:
			  std::vector<double> theDividends;
			  std::vector<Time> theExDivDates;			
			  double riskless(Rate r,	Time t,	std::vector<double>	divs,std::vector<Time> divDates){
				double tmp_rho = 0.0;
				for(unsigned int j=0; j<divs.size();j++)
					tmp_rho += divs[j]*QL_EXP(-r*divDates[j]);
				return tmp_rho;
			}
		};

	}

}

#endif
