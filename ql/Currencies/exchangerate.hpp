
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

#ifndef quantlib_exchangerate_hpp
#define quantlib_exchangerate_hpp

#include <vector>
#include <map>
#include <ql/types.hpp>
#include <ql/date.hpp>
#include <ql/marketelement.hpp>
#include <ql/currency.hpp>

namespace QuantLib {

    //! Exchange Rate
    /*! This concrete class defines the interface to translation between
      money instances of different currencies.
    */
    class Money;

    class ExchangeRate {
    public:
	/*! \name Exchange Rate interface

	Defines the basic interface to specifying exchange rates between
	different currencies.
	*/

	ExchangeRate() {}
	ExchangeRate(const Currency& source,
		     const Currency& target,
		     std::string type)
	: source_(source),target_(target),type_(type) {}
	ExchangeRate(const Currency& source,
		     const Currency& target,
		     RelinkableHandle<Quote> factor)
	: source_(source),target_(target),factor_(factor),
	  type_("direct") {}
	//@{
	//! The source currency.
	const Currency& source() const { return source_; }
	//! The target currency.
	const Currency& target() const { return target_; }
	//! The type, free format, special processing for 'direct' or 'derived'
	const std::string type() { return type_; }
	/*! The translation factor of source vs. target (ie 1 of source = x of
	  target.
	*/
	RelinkableHandle<Quote>& factor() { return factor_; }
	//! Add to the chain of exchange rates, used in derived types.
	void addLink(boost::shared_ptr<ExchangeRate> exchRate) {
	    rateChain_.push_back(exchRate);
	}
	std::vector<boost::shared_ptr<ExchangeRate> > rateChain() { return rateChain_; }
	//! Exchange amount.
	const Money exchange(const Money& amount) const;
	bool operator==(ExchangeRate& other) const {
	    return (source_ == other.source() &&
		    target_ == other.target() &&
		    factor_->value() == other.factor()->value());
	}
	const std::string hashKey() const {
	    return ExchangeRate::hashKey(source_,target_);
	}
	static std::string hashKey(const Currency& curr1,
				   const Currency& curr2) {
	    return curr1.mnemonic()+"::"+curr2.mnemonic();
	}
	//@}
    private:
	Currency source_;
	Currency target_;
	RelinkableHandle<Quote> factor_;
	std::vector<boost::shared_ptr<ExchangeRate> > rateChain_;
	std::string type_;
    };

    //! Dated Exchange Rate
    /*! This concrete class defines the interface to translation between
      money instances of different currencies.
    */
    class DExchangeRate : public ExchangeRate {
    public:
	/*! \name Dated Exchange Rate interface

	Defines the basic interface to specifying exchange rates between
	different currencies, valid for a range of dates.
	*/

	DExchangeRate() {}
	DExchangeRate(const Currency& source,
		      const Currency& target,
		      const Date& startDate,
		      const Date& endDate,
		      std::string type)
	: ExchangeRate(source,target,type) {
	    QL_REQUIRE(startDate <= endDate,
		       "Invalid startDate vs endDate");
	    startDate_ = startDate;
	    endDate_ = endDate;
	}
	DExchangeRate(const Currency& source,
		      const Currency& target,
		      RelinkableHandle<Quote> factor,
		      const Date& startDate,
		      const Date& endDate)
	: ExchangeRate(source,target,factor) {
	    QL_REQUIRE(startDate <= endDate,
		       "Invalid startDate vs endDate");
	    startDate_ = startDate;
	    endDate_ = endDate;
	}
	//@{
	//! The start of the valid date range.
	const Date& startDate() const { return startDate_; }
	//! The end of the valid date range.
	const Date& endDate() const { return endDate_; }
	bool operator==(DExchangeRate& other) const {
	    return (ExchangeRate::operator==(other) &&
		    startDate_ == other.startDate() &&
		    endDate_ == other.endDate());
	}
    private:
	Date startDate_;
	Date endDate_;
    };
    
}

#endif
