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
/*! \file singlepathaveragepriceasianpricer.cpp
	
	$Source$
	$Name$
	$Log$
	Revision 1.1  2001/01/04 17:31:23  marmar
	Alpha version of the Monte Carlo tools.

*/

#include "singlepathaveragepriceasianpricer.h"

namespace QuantLib {

	namespace MonteCarlo {

		SinglePathAveragePriceAsianPricer::SinglePathAveragePriceAsianPricer(Option::Type optiontype, 
									double underlying, double strike, double discount):
									SinglePathEuropeanPricer(optiontype, underlying, strike, discount){
			isInitialized_=true;
		}

		double SinglePathAveragePriceAsianPricer::value(const Path & path) const{

			int n = path.size();
			QL_REQUIRE(n>0,"the path cannot be empty");
			QL_REQUIRE(isInitialized_,"SinglePathAveragePriceAsianPricer: pricer not initialized");

			double price = underlying_;
			double averagePrice = 0.0;

			for(unsigned int i=0;i<n;i++){
				price *= QL_EXP(path[i]);
				averagePrice += price;
			}
			averagePrice = averagePrice/n;

			return computePlainVanilla(optionType_, averagePrice, strike_, discount_);
		}
		
	}

}