
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

/*! \file money.hpp
    \brief Money implementation
*/

#ifndef quantlib_money_hpp
#define quantlib_money_hpp

#include <ql/types.hpp>
#include <ql/errors.hpp>
#include <ql/currency.hpp>
#include <ql/Currencies/exchangerate.hpp>
#include <ql/Currencies/ratemanager.hpp>
#include <ql/Math/rounding.hpp>

namespace QuantLib {

    //! Currency Parameters
    /*! This concrete class acts as the static setup for rules when doing
      maths on Money objects.
    */

    enum ConversionType {
	NO_CONVERSION,
	BASE_CURRENCY_CONVERSION,
	AUTOMATED_CONVERSION
    };
    
    class CurrencyParams {
    public:
	/*! \name Currency Rules
	  
	Determines the rules of conversion to be applied when adding or
	subtracting money objects.
	*/

	CurrencyParams() {}
	//@{
	//! Conversion type get/set
	static ConversionType conversionType() { return conversionType_; }
	static void setConversionType(const ConversionType& type) {
	    conversionType_ = type;
	}
	//! Base Currency get/set
	static Currency& baseCurrency() { return baseCurrency_; }
	static void setBaseCurrency(const Currency& base) {
	    baseCurrency_ = base;
	}
        //@}
    private:
	static ConversionType conversionType_;
	static Currency baseCurrency_;
    };

    //! Money structure
    /*! This concrete class defines the interface of money objects, which
        acts like an instance of a currency
    */

    class Money {
    public:
	Money() {}
	Money(const Currency& currency, const Decimal value)
	    : currency_(currency),value_(value),isRounding_(false) {}
	/*! \name Money interface
	  
	Defines basic interface to Money objects.
	*/

	//@{
	//! Set Rounding to be applied
	void setRounding(const Rounding& rounding) {
	    isRounding_ = true;
	    rounding_ = rounding;
	}
	//! Is there local rounding to be applied?
	const bool isRounding() const { return isRounding_; }
	//! The currency of the value
	const Currency& currency() const { return currency_; }
	//! The amount of money in specified currency.
	const Decimal value() const { return value_; }
	//! Round the value, first check for local, else apply currency's.
	const Decimal round() const {
	    if (isRounding_)
		return rounding_(value_);
	    if (currency_.isRounding())
		return currency_.prefRounding()(value_);
	    return value_;
	}
	/*! Overloaded add and subtract. Use CurrencyParams to determine the
	  resulting currency:
	  'AUTOMATED_CONVERSION': result will be in other currency (Money
	  object on the right hand side of operator).
	  'BASE_CURRENCY_CONVERSION': result will be converted to base
	  currency.
	*/
	Money operator+(Money other);
	Money operator-(Money other);
	Money operator*(Money other);
	Money operator/(Money other);
	Money& operator+=(Money other);
	Money& operator-=(Money other);

	Money operator+(double value);
	Money& operator+=(double value);
	Money operator-(double value);
	Money& operator-=(double value);
	Money operator*(double value);
	Money& operator*=(double value);
	Money operator/(double value);
	Money& operator/=(double value);
	
	//! Get/set exchange rate manager that will do exchange calculations.
	static DExchangeRateManager exchangeRateManager() {
	    return rateManager_;
	}
	static void setExchangeRateManager(DExchangeRateManager m){
	    rateManager_ = m;
	}

    private:
	Currency currency_;
	Decimal value_;
	mutable bool isRounding_;
	mutable Rounding rounding_;
	static DExchangeRateManager rateManager_;
    };

    /*! Returns <tt>true</tt> if the two moneys belong to the same
        derived class.
        \relates Money
    */
    bool operator==(const Money&, const Money&);

    /*! \relates Currency */
    bool operator!=(const Money&, const Money&);

    inline bool operator==(const Money& m1, const Money& m2) {
        return (m1.value() == m2.value() && m1.currency() == m2.currency());
    }

    inline bool operator!=(const Money& m1, const Money& m2) {
	return !(m1 == m2);
    }

    inline Money Money::operator+(double value) {
	return Money(currency_,value_+value);
    }

    inline Money& Money::operator+=(double value) {
	value_ += value;
	return *this;
    }

    inline Money Money::operator-(double value) {
	return Money(currency_,value_-value);
    }

    inline Money& Money::operator-=(double value) {
	value_ -= value;
	return *this;
    }

    inline Money Money::operator*(double value) {
	return Money(currency_,value_*value);
    }

    inline Money& Money::operator*=(double value) {
	value_ *= value;
	return *this;
    }

    inline Money Money::operator/(double value) {
	return Money(currency_,value_/value);
    }

    inline Money& Money::operator/=(double value) {
	value_ /= value;
	return *this;
    }

}

#endif
