
/* 
 * QuantLib -- A C++ library for financial quantitative analysts and developers
 * http://quantlib.sourceforge.net/
 *
 * This program is free software; you can redistribute it and/or
 * See the file LICENSE.TXT for complete information on usage and distribution
 * Contact ferdinando@ametrano.net if LICENSE.TXT was not distributed with this file
 *
 * This file:
 * Copyright (C) 2000 Ferdinando Ametrano, Luigi Ballabio, Adolfo Benin, Marco Marchioro
*/

#ifndef BSM_option_pricer_h
#define BSM_option_pricer_h

#include "qldefines.h"
#include "options.h"
#include "date.h"
#include "yield.h"
#include "handle.h"
#include "solver1d.h"

QL_BEGIN_NAMESPACE(QuantLib)

QL_BEGIN_NAMESPACE(Pricers)

class BSMOption : public Option {
  public:
	  BSMOption(Type type, double underlying, double strike, Rate underlyingGrowthRate, 
	    Rate riskFreeRate, Time residualTime, double volatility) 
	  : theType(type), theUnderlying(underlying), theStrike(strike), theUnderlyingGrowthRate(underlyingGrowthRate),
	    theRiskFreeRate(riskFreeRate), theResidualTime(residualTime), theVolatility(volatility),
      hasBeenCalculated(false) {

      Require(strike > 0.0, "BSMOption::BSMOption : strike must be positive");
      Require(underlying > 0.0, "BSMOption::BSMOption : underlying must be positive");
      Require(residualTime > 0.0, "BSMOption::BSMOption : residual time must be positive");
      Require(volatility > 0.0, "BSMOption::BSMOption : volatility must be positive");

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
    class BSMFunction : public Function {
	    public:
		  BSMFunction(Handle<BSMOption> tempBSM, double price) {
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
	Require(volatility>=0.0,"BSMOption::setVolatility : Volatility must be positive");
	theVolatility = volatility;
	hasBeenCalculated=false;
}

inline void BSMOption::setRiskFreeRate(Rate newRiskFreeRate) {
	theRiskFreeRate = newRiskFreeRate;
	hasBeenCalculated=false;
}


QL_END_NAMESPACE(Pricers)

QL_END_NAMESPACE(QuantLib)


#endif
