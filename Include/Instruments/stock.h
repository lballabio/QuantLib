
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

/*! \file stock.h
	\brief concrete stock class
	
	$Source$
	$Name$
	$Log$
	Revision 1.3  2000/12/14 12:32:30  lballabio
	Added CVS tags in Doxygen file documentation blocks

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
			void performFinalCalculations() const { price(); } // just check that it works
		};
	
	}

}


#endif
