		
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

#ifndef shaft_dividend_american_option_pricer_h
#define shaft_dividend_american_option_pricer_h

#include "qldefines.h"
#include "bsmnumericaloption.h"
#include "finitedifferences.h"
#include "americancondition.h"
#include "mathtools.h"
#include <vector>

/*
#include "date.h"
#include "dividendeuropeanoption.h"
#include "finitedifferencemodel.h"
#include "handle.h"

#include "finitedifferences.h"
#include "newcubicspline.h"
#include "cranknicolson.h"
#include "bsmnumericaloption.h"
#include "americancondition.h"
*/
namespace QuantLib {
	namespace Pricers {

		class DividendAmericanOption : public BSMNumericalOption {
		  public:
			// constructor
			DividendAmericanOption(Option::Type type, double underlying, double strike, 
			  Rate underlyingGrowthRate, Rate riskFreeRate, Time residualTime, double volatility,
			   std::vector<double> dividends, std::vector<Time> exdivdates);
			// accessors
			double value() const;
			private:
			mutable bool theOptionIsAmerican;
			std::vector<double> theDividends;
			std::vector<Time> theExDivDates;
			unsigned int  theNumberOfDivs;
			void movePricesBeforeExDiv(double Div, const Array& newGrid, Array& prices, const Array& oldGrid) const;
			Handle<BSMOption> clone() const { return Handle<BSMOption>(new DividendAmericanOption(*this)); }

			double addElements(const std::vector<double>& A) const{
				double sum = 0.0;
				for(unsigned int i=0;i<A.size();i++){ sum += A[i]; }
				return sum;
			}
		};

	}
}
#endif
