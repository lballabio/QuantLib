
/*
 * Copyright (C) 2000
 * Ferdinando Ametrano, Luigi Ballabio, Adolfo Benin, Marco Marchioro
 * 
 * This file is part of QuantLib
 * QuantLib is a C++ open source library for financial quantitative
 * analysts and developers --- http://quantlib.sourceforge.net/
 *
 * QuantLib is free software and you are allowed to use, copy, modify, merge,
 * publish, distribute, and/or sell copies of it under the conditions stated 
 * in the QuantLib License: see the file LICENSE.TXT for details.
 * Contact ferdinando@ametrano.net if LICENSE.TXT was not distributed with this file.
 * LICENCE.TXT is also available at http://quantlib.sourceforge.net/LICENSE.TXT
*/

#ifndef BSM_american_condition_h
#define BSM_american_condition_h

#include "qldefines.h"
#include "bsmnumericaloption.h"
#include "stepcondition.h"
#include <algorithm>
#include <functional>

namespace QuantLib {

	namespace Pricers {
	
		class BSMAmericanCondition : public PDE::StepCondition<Array> {
		  public:
			BSMAmericanCondition(const Array& initialPrices) : initialPrices(initialPrices) {}
			void applyTo(Array& a, Time t) const {
				for (int i=0; i<a.size(); i++)
					a[i] = QL_MAX(a[i],initialPrices[i]);
			}
		  private:
			Array initialPrices;
		};
	
	}

}

#endif
