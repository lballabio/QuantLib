
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

#include <ql/dataformatters.hpp>
#include <ql/Currencies/ratemanager.hpp>
#include <ql/Currencies/money.hpp>

namespace QuantLib {

    void ExchangeRateManager::add(ExchangeRate& exchRate) {
	map_[exchRate.hashKey()] = exchRate;
    }

    void ExchangeRateManager::add(std::vector<ExchangeRate>& list) {
	for (Size i=0; i<list.size(); i++)
	    add(list[i]);
    }

    ExchangeRate ExchangeRateManager::directRate(
	const Currency& source,
	const Currency& target) {
	std::string key = ExchangeRate::hashKey(source,target);
	if (map_.find(key) != map_.end())
	    return map_[key];
	key = ExchangeRate::hashKey(target,source);
	if (map_.find(key) != map_.end())
	    return map_[key];
	QL_FAIL("No direct exchange possible from "+
		source.mnemonic()+" to "+target.mnemonic());
	QL_DUMMY_RETURN(ExchangeRate());
    }

    ExchangeRate ExchangeRateManager::derivedRate(
	const Currency& source,
	const Currency& target) {
	try {
	    return directRate(source,target);
	} catch (...) { ; }
	ExchangeRate exchRate = ExchangeRate(source,target,"derived");
	if (!(target == target.nextCurrency())) {
	    exchRate.addLink(boost::shared_ptr<ExchangeRate>(
				 new ExchangeRate(
				     lookupRate(source,
						target.nextCurrency()))));
	    exchRate.addLink(boost::shared_ptr<ExchangeRate>(
				 new ExchangeRate(
				     lookupRate(target.lastCurrency(),
						target))));
	} else if (!(source == source.nextCurrency())) {
	    exchRate.addLink(boost::shared_ptr<ExchangeRate>(
				 new ExchangeRate(
				     lookupRate(target,
						source.nextCurrency()))));
	    exchRate.addLink(boost::shared_ptr<ExchangeRate>(
				 new ExchangeRate(
				     lookupRate(source.lastCurrency(),
						source))));
	} else
	    QL_FAIL("No derived exchange rate from "+
		    source.mnemonic()+" to "+target.mnemonic());
	return exchRate;
    }
    
    ExchangeRate ExchangeRateManager::lookupRate(
	const Currency& source,
	const Currency& target,
	std::string type) {
	if (type == "direct") return directRate(source,target);
	if (type == "derived") return derivedRate(source,target);
	if (type == "any") {
	    try {
		return directRate(source,target);
            } catch (...) {}
	    try {
		return derivedRate(source,target);
            } catch (...) {}
	    QL_FAIL("No exchange rate from "+
		    source.mnemonic()+" to "+target.mnemonic());
	}
	QL_DUMMY_RETURN(Exchangerate);
    }

    Money ExchangeRateManager::exchange(const Money& value,
					const Currency& target) {
	if (value.currency() == target)
	    return value;
	return lookupRate(value.currency(),target).exchange(value);
    }
    
    void DExchangeRateManager::add(DExchangeRate& exchRate) {
	Date valid = exchRate.startDate();
	do {
	    map_[valid][exchRate.hashKey()] = exchRate;
	    valid += 1;
	} while (valid <= exchRate.endDate());
    }

    void DExchangeRateManager::add(std::vector<DExchangeRate>& list) {
	for (Size i=0; i<list.size(); i++)
	    add(list[i]);
    }

    DExchangeRate DExchangeRateManager::directRate(
	const Currency& source,
	const Currency& target) {
	QL_REQUIRE(!(map_.find(exchDate_) == map_.end()),
		   "No direct exchange rate valid for "+
		   DateFormatter::toString(exchDate_)+"from "+
		   source.mnemonic()+" to "+target.mnemonic());
	TargetLevel tlev = map_[exchDate_];
	std::string key = DExchangeRate::hashKey(source,target);
	if (tlev.find(key) != tlev.end())
	    return tlev[key];
	key = DExchangeRate::hashKey(target,source);
	if (tlev.find(key) != tlev.end())
	    return tlev[key];
	QL_FAIL("No direct exchange possible from "+
		source.mnemonic()+" to "+target.mnemonic());
	QL_DUMMY_RETURN(DExchangeRate);
    }

    DExchangeRate DExchangeRateManager::derivedRate(
	const Currency& source,
	const Currency& target) {
	try {
	    return directRate(source,target);
	} catch (...) { ; }
	DExchangeRate exchRate = DExchangeRate(source,target,
					       exchDate_,exchDate_,
					       "derived");
	if (!(target == target.nextCurrency())) {
	    exchRate.addLink(boost::shared_ptr<DExchangeRate>(
				 new DExchangeRate(
				     lookupRate(source,
						target.nextCurrency()))));
	    exchRate.addLink(boost::shared_ptr<DExchangeRate>(
				 new DExchangeRate(
				     lookupRate(target.lastCurrency(),
						target))));
	} else if (!(source == source.nextCurrency())) {
	    exchRate.addLink(boost::shared_ptr<DExchangeRate>(
				 new DExchangeRate(
				     lookupRate(target,
						source.nextCurrency()))));
	    exchRate.addLink(boost::shared_ptr<DExchangeRate>(
				 new DExchangeRate(
				     lookupRate(source.lastCurrency(),
						source))));
	} else
	    QL_FAIL("No derived exchange rate from "+
		    source.mnemonic()+" to "+target.mnemonic());
	return exchRate;
    }
    
    DExchangeRate DExchangeRateManager::lookupRate(
	const Currency& source,
	const Currency& target,
	std::string type) {
	if (type == "direct") return directRate(source,target);
	if (type == "derived") return derivedRate(source,target);
	if (type == "any") {
	    try {
		return directRate(source,target);
            } catch (...) {}
	    try {
		return derivedRate(source,target);
            } catch (...) {}
	    QL_FAIL("No exchange rate from "+
		    source.mnemonic()+" to "+target.mnemonic());
	}
	QL_DUMMY_RETURN(DExchangeRate);
    }

    Money DExchangeRateManager::exchange(const Money& value,
					 const Currency& target) {
	if (value.currency() == target)
	    return value;
	return lookupRate(value.currency(),target).exchange(value);
    }
    
}
