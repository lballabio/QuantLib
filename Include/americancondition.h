/* 
Copyright (C) 2000 Ferdinando Ametrano, Luigi Ballabio, Adolfo Benin, Marco Marchioro
See the file LICENSE.TXT for information on usage and distribution
Contact ferdinando@ametrano.net if LICENSE.TXT was not distributed with this file
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
