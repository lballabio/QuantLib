/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2009 Chris Kenyon

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
 */



#include <ql/cashflows/baseindexedcashflow.hpp>
#include <ql/index.hpp>
#include <ql/indexes/inflationindex.hpp>

 
namespace QuantLib {

	
	Date BaseIndexedCashFlow::baseDate() const {
		// you may not have a valid date
		exit(-1);
	}
	
	Real BaseIndexedCashFlow::baseFixing() const {
		return baseFixing_;
	}
	
	Real BaseIndexedCashFlow::amount() const {
        Real I0 = baseFixing();
        Real I1;
		
		// what interpolation do we use? Index / flat / linear
		if (interpolation() == iiINDEX ) {
			I1 = index()->fixing(fixingDate()); 
		} else {
			// work out what it should be
			//std::cout << fixingDate() << " and " << frequency() << std::endl;
			//std::pair<Date,Date> dd = inflationPeriod(fixingDate(), frequency());
			//std::cout << fixingDate() << " and " << dd.first << " " << dd.second << std::endl;
			// work out what it should be
			std::pair<Date,Date> dd = inflationPeriod(fixingDate(), frequency());
			Real indexStart = index()->fixing(dd.first);
			if (interpolation() == iiLINEAR) {
				Real indexEnd = index()->fixing(dd.second+Period(1,Days));
				// linear interpolation 
				//std::cout << indexStart << " and " << indexEnd << std::endl;
				I1 = indexStart + (indexEnd - indexStart) * (fixingDate() - dd.first) 
				/ ( (dd.second+Period(1,Days)) - dd.first); // can't get to next period's value within current period
			} else {
				// no interpolation, i.e. flat = constant, so use start-of-period value
				I1 = indexStart;
			}
			
		}
		
		
        if (growthOnly())
            return notional() * (I1 / I0 - 1.0);
        else
            return notional() * (I1 / I0);
    }
	

}




