
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

/*! \file bsmoption.h
	\brief common code for option evaluation
	
	$Source$
	$Name$
	$Log$
	Revision 1.4  2000/12/27 14:05:56  lballabio
	Turned Require and Ensure functions into QL_REQUIRE and QL_ENSURE macros

	Revision 1.3  2000/12/14 12:32:30  lballabio
	Added CVS tags in Doxygen file documentation blocks
	
*/

#ifndef BSM_option_pricer_h
#define BSM_option_pricer_h

#include "qldefines.h"
#include "options.h"
#include "date.h"
#include "rate.h"
#include "handle.h"
#include "solver1d.h"

namespace QuantLib {

	namespace Pricers {
	
		class BSMOption : public Option {
		  public:
			BSMOption(Type type, double underlying, double strike, Rate underlyingGrowthRate, 
			  Rate riskFreeRate, Time residualTime, double volatility) 
			: theType(type), theUnderlying(underlying), theStrike(strike), theUnderlyingGrowthRate(underlyingGrowthRate),
			  theRiskFreeRate(riskFreeRate), theResidualTime(residualTime), theVolatility(volatility),
			  hasBeenCalculated(false) {
				QL_REQUIRE(strike > 0.0, "BSMOption::BSMOption : strike must be positive");
				QL_REQUIRE(underlying > 0.0, "BSMOption::BSMOption : underlying must be positive");
				QL_REQUIRE(residualTime > 0.0, "BSMOption::BSMOption : residual time must be positive");
				QL_REQUIRE(volatility > 0.0, "BSMOption::BSMOption : volatility must be positive");
			}
			virtual ~BSMOption() {}	// just in case
			// modifiers
			void setVolatility(double newVolatility) ;
			void setRiskFreeRate(Rate newRate) ;
			// accessors
			virtual double value() const = 0;
			virtual double delta() const = 0;
			virtual double gamma() const = 0;
			virtual double theta() const = 0;
			virtual double vega() const = 0;
			virtual double rho() const = 0;
			double impliedVolatility(double targetValue, double accuracy = 1e-4, int maxEvaluations = 100) const ;
			virtual Handle<BSMOption> clone() const = 0;
		  protected:
			// input data
			Type theType;
			double theUnderlying, theStrike;
			Rate theUnderlyingGrowthRate, theRiskFreeRate;
			Time theResidualTime;
			double theVolatility;
			// results
			mutable bool hasBeenCalculated;	// declared as mutable to preserve the logical
			mutable double theValue;				// constness (does this word exist?) of value()
		  private:
			class BSMFunction;
			friend class BSMFunction;
			class BSMFunction : public ObjectiveFunction {
			  public:
				BSMFunction(const Handle<BSMOption>& tempBSM, double price) {
					bsm = tempBSM;
					thePrice = price;
				}
				double value(double x) const {
					bsm->setVolatility(x);
					return (bsm->value()-thePrice);
				}
				double derivative(double x) const {
					// assuming that derivative(x) is always called after value(x)
					// so that setVolatility unnecessary
					return bsm->vega();
				}
			  private:
				mutable Handle<BSMOption> bsm;
				double thePrice;
			};
		};
		
		inline void BSMOption::setVolatility(double volatility) {
			QL_REQUIRE(volatility>=0.0,"BSMOption::setVolatility : Volatility must be positive");
			theVolatility = volatility;
			hasBeenCalculated=false;
		}
		
		inline void BSMOption::setRiskFreeRate(Rate newRiskFreeRate) {
			theRiskFreeRate = newRiskFreeRate;
			hasBeenCalculated=false;
		}

	}

}


#endif
