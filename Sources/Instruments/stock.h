
/* 
Copyright (C) 2000 F.Ametrano, L.Ballabio, A.Benin, M.Marchioro
See the file LICENSE.TXT for information on usage and distribution
Contact ferdinando@ametrano.net if LICENSE.TXT was not distributed with this file
*/

#ifndef quantlib_stock_h
#define quantlib_stock_h

#include "qldefines.h"
#include "instrument.h"

QL_BEGIN_NAMESPACE(QuantLib)

QL_BEGIN_NAMESPACE(Instruments)

class Stock : public PricedInstrument {
  public:
	Stock() {}
	Stock(std::string isinCode, std::string description)
	: PricedInstrument(isinCode,description) {}
	// modifiers
	void setPrice(double price) { PricedInstrument::setPrice(price); theNPV = price; }
	// inspectors
	bool useTermStructure() const { return false; }
	bool useSwaptionVolatility() const { return false; }
	bool useForwardVolatility() const { return false; }
  private:
	// methods
	bool needsFinalCalculations() const { return true; }
	void performFinalCalculations() const { Require(priceIsSet,"stock price not set"); }
};

QL_END_NAMESPACE(Instruments)

QL_END_NAMESPACE(QuantLib)


#endif
