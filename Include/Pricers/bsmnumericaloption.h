
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

#ifndef BSM_numerical_option_pricer_h
#define BSM_numerical_option_pricer_h

#include "qldefines.h"
#include "bsmoption.h"
#include "array.h"
#include "handle.h"
#include "finitedifferences.h"

namespace QuantLib {

	namespace Pricers {
	
		class BSMNumericalOption : public BSMOption {
		  public:
			BSMNumericalOption(Type type, double underlying, double strike, Rate underlyingGrowthRate, 
			  Rate riskFreeRate, Time residualTime, double volatility, int gridPoints);
			// accessors
			double delta() const;
			double gamma() const;
			double theta() const;
			double rho()   const;
			double vega()  const;
		
		  protected:
			// methods
			double valueAtCenter(const Array& a) const;
			double firstDerivativeAtCenter(const Array& a, const Array& g) const;
			double secondDerivativeAtCenter(const Array& a, const Array& g) const;
			void setGridLimits() const;
			void initializeGrid() const;
			void initializeInitialCondition() const;
			void initializeOperator() const;
			// input data
			int theGridPoints;
			// results
			mutable bool rhoComputed, vegaComputed;
			mutable double theDelta, theGamma, theTheta;
			mutable double  theRho, theVega;
		
			mutable Array theGrid;
			mutable FiniteDifferences::BSMOperator theOperator;
			mutable Array theInitialPrices;
			// temporaries
			mutable double sMin, sMax;
		  private:
			// temporaries
			mutable double theGridLogSpacing;
			static double dVolMultiplier, dRMultiplier; 
		};

	}

}


#endif
