
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

/*! \file exchangerate.hpp
    \brief ExchangeRate implementation
*/

#ifndef quantlib_ratemanager_hpp
#define quantlib_ratemanager_hpp

#include <ql/Currencies/exchangerate.hpp>

namespace QuantLib {

    //! ExchangeRateManager structure
    /*! This concrete class defines the interface to the manager of exchange
      rates.
    */
    class ExchangeRateManager {
    public:
	virtual ~ExchangeRateManager() {}
	ExchangeRateManager() {}
	/*! \name Exchange Rate Manager

	Defines the basic interface to exchanging of Money objects.
	*/

	//@{
	//! Add an exchange rate.
	void add(ExchangeRate& exchRate);
	//! Add a list of exchange rates.
	void add(std::vector<ExchangeRate>& list);
	/*! Lookup the exchange rate between two currencies, type can be
	  'direct' - only direct exchange rates will be returned if availble,
	  'derived' - if direct available return, else use 'next' and 'last'
	  in Currency to determine exchange rate chain.
	  'any' - first try for direct else derived if available.
	*/
	ExchangeRate lookupRate(const Currency& source,
				const Currency& target,
				std::string type = "any");
	//! Apply exchange rate to convert from Money to target currency.
	Money exchange(const Money& value, const Currency& target);
	//@}
    private:
	ExchangeRate directRate(const Currency& source,
				const Currency& target);
	ExchangeRate derivedRate(const Currency& source,
				 const Currency& target);
	mutable std::map<std::string,ExchangeRate> map_;
    };

    //! DExchangeRateManager structure
    /*! This concrete class handles specialisation of exchangeRateManager to
      handle dated exchange rates.
    */
    class DExchangeRateManager : public ExchangeRateManager {
    public:
	/*! \name Dated Exchange Rate Manager

	Defines the basic interface to exchanging of Money objects on a
	specific date.
	*/
	
	//@{
	DExchangeRateManager() {}
	DExchangeRateManager(const Date& exchDate) : exchDate_(exchDate) {}
	//! Add a dated exchange rate.
	void add(DExchangeRate& exchRate);
	//! Add a list of dated exchange rates.
	void add(std::vector<DExchangeRate>& list);
	//! Get/set the exchange date to be used as lookup date.
	const Date& exchDate() const { return exchDate_; }
	void setExchDate(const Date& exchDate) {
	    exchDate_ = exchDate;
	}
	/*! Lookup the exchange rate between two currencies, type can be
	  'direct' - only direct exchange rates will be returned if availble,
	  'derived' - if direct available return, else use 'next' and 'last'
	  in Currency to determine exchange rate chain.
	  'any' - first try for direct else derived if available.
	*/
	DExchangeRate lookupRate(const Currency& source,
				 const Currency& target,
				 std::string type = "any");
	//! Apply exchange rate to convert from Money to target currency.
	Money exchange(const Money& value, const Currency& target);
	//@}
    private:
	DExchangeRate directRate(const Currency& source,
				 const Currency& target);
	DExchangeRate derivedRate(const Currency& source,
				  const Currency& target);
	typedef std::map<std::string,DExchangeRate> TargetLevel;
	mutable std::map<Date,TargetLevel> map_;
	mutable Date exchDate_;
    };

}

#endif
