
/*
 Copyright (C) 2004 Decillion Pty(Ltd)

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email quantlib-dev@lists.sf.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/Currencies/exchangerate.hpp>
#include <ql/Currencies/money.hpp>

namespace QuantLib {

    const Money ExchangeRate::exchange(const Money& amount) const {
	if (type_ == "direct") {
	    if (amount.currency() == source_)
		return Money(target_,amount.value()*factor_->value());
	    QL_REQUIRE(amount.currency() == target_,
		       "Direct exchange not possible");
	    return Money(source_,amount.value()/factor_->value());
	} else if (type_ == "derived") {
	    if (rateChain_[0]->source_ == amount.currency() ||
		rateChain_[0]->target_ == amount.currency())
		return rateChain_[1]->exchange(
		    rateChain_[0]->exchange(amount));
	    if (rateChain_[1]->source_ == amount.currency() ||
		rateChain_[1]->target_ == amount.currency())
		return rateChain_[0]->exchange(
		    rateChain_[1]->exchange(amount));
	    QL_FAIL("Invalid derived exchange rate");
	}
	QL_DUMMY_RETURN(Money());
    }
    
}
