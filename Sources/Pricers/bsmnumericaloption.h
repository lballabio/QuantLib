
/* 
Copyright (C) 2000 Ferdinando Ametrano, Luigi Ballabio, Adolfo Benin, Marco Marchioro
See the file LICENSE.TXT for information on usage and distribution
Contact ferdinando@ametrano.net if LICENSE.TXT was not distributed with this file
*/

#ifndef BSM_numerical_option_pricer_h
#define BSM_numerical_option_pricer_h

#include "qldefines.h"
#include "bsmoption.h"
#include "array.h"
#include "blackscholesmerton.h"

QL_BEGIN_NAMESPACE(QuantLib)

QL_BEGIN_NAMESPACE(Pricers)

class BSMNumericalOption : public BSMOption {
  public:
	BSMNumericalOption(Type type, double underlying, double strike, Yield underlyingGrowthRate, 
	  Yield riskFreeRate, Time residualTime, double volatility, int gridPoints);
	// accessors
	virtual double value() const = 0;
  protected:
	// methods
	double valueAtCenter(const Array& a) const;
	// input data
	int theGridPoints;
	// results
	Array theGrid;
	QL_ADD_NAMESPACE(Operators,BSMOperator) theOperator;
	mutable Array thePrices;
  private:
	// methods
	void setGridLimits();
	void initializeGrid();
	void initializeInitialCondition();
	void initializeOperator();
	// temporaries
	double sMin, sMax;
	double theGridLogSpacing;
};


QL_END_NAMESPACE(Pricers)

QL_END_NAMESPACE(QuantLib)


#endif
