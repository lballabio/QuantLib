
/* 
Copyright (C) 2000 Ferdinando Ametrano, Luigi Ballabio, Adolfo Benin, Marco Marchioro
See the file LICENSE.TXT for information on usage and distribution
Contact ferdinando@ametrano.net if LICENSE.TXT was not distributed with this file
*/

#ifndef quantlib_stock_h
#define quantlib_stock_h

#include "qldefines.h"
#include "instrument.h"

namespace QuantLib {

	namespace Instruments {
	
		class Stock : public PricedInstrument {
		  public:
			Stock() {}
			Stock(const std::string& isinCode, const std::string& description)
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
	
	}

}


#endif
