
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

#include <ql/date.hpp>
#include <ql/Currencies/money.hpp>
#include <ql/Currencies/exchangerate.hpp>
#include <ql/Currencies/ratemanager.hpp>

namespace QuantLib {

    ConversionType CurrencyParams::conversionType_ = AUTOMATED_CONVERSION;
    Currency CurrencyParams::baseCurrency_ = Currency();
    DExchangeRateManager Money::rateManager_ =
	DExchangeRateManager(Date::todaysDate());

    Money Money::operator+(Money other) {
	if (currency_ == other.currency())
	    return Money(other.currency(),value_+other.value());
	else {
	    switch (CurrencyParams::conversionType()) {
		case AUTOMATED_CONVERSION:
		{
		    Money tmp = rateManager_.exchange(*this,other.currency());
		    return Money(other.currency(),tmp.round()+other.value());
		}
		case BASE_CURRENCY_CONVERSION:
		{
		    Currency base = CurrencyParams::baseCurrency();
		    Decimal val_a = value_,
			val_b = other.value();
		    if (!(currency_ == base))
			val_a = rateManager_.exchange(*this,base).round();
		    if (!(other.currency() == base))
			val_b = rateManager_.exchange(other,base).round();
		    return Money(base,val_a+val_b);
		}
		default:
		    break;
	    }
	}
	QL_FAIL("Invalid currency Conversion Type");
	QL_DUMMY_RETURN(Money());
    }
    
    Money Money::operator-(Money other) {
	if (currency_ == other.currency())
	    return Money(other.currency(),value_-other.value());
	else {
	    switch (CurrencyParams::conversionType()) {
		case AUTOMATED_CONVERSION:
		{
		    Money tmp = rateManager_.exchange(*this,other.currency());
		    return Money(other.currency(),tmp.round()-other.value());
		}
		case BASE_CURRENCY_CONVERSION:
		{
		    Currency base = CurrencyParams::baseCurrency();
		    Decimal val_a = value_,
			val_b = other.value();
		    if (!(currency_ == base)) {
			val_a = rateManager_.exchange(*this,base).round();
		    }
		    if (!(other.currency() == base)) {
			val_b = rateManager_.exchange(other,base).round();
		    }
		    return Money(base,val_a-val_b);
		}
		default:
		    break;
	    }
	}
	QL_FAIL("Invalid currency Conversion Type");
	QL_DUMMY_RETURN(Money());
    }
    
    Money Money::operator*(Money other) {
	if (currency_ == other.currency())
	    return Money(other.currency(),value_*other.value());
	else {
	    switch (CurrencyParams::conversionType()) {
		case AUTOMATED_CONVERSION:
		{
		    Money tmp = rateManager_.exchange(*this,other.currency());
		    return Money(other.currency(),tmp.round()*other.value());
		}
		case BASE_CURRENCY_CONVERSION:
		{
		    Currency base = CurrencyParams::baseCurrency();
		    Decimal val_a = value_,
			val_b = other.value();
		    if (!(currency_ == base))
			val_a = rateManager_.exchange(*this,base).round();
		    if (!(other.currency() == base))
			val_b = rateManager_.exchange(other,base).round();
		    return Money(base,val_a*val_b);
		}
		default:
		    break;
	    }
	}
	QL_FAIL("Invalid currency Conversion Type");
	QL_DUMMY_RETURN(Money());
    }
    
    Money Money::operator/(Money other) {
	if (currency_ == other.currency())
	    return Money(other.currency(),value_/other.value());
	else {
	    switch (CurrencyParams::conversionType()) {
		case AUTOMATED_CONVERSION:
		{
		    Money tmp = rateManager_.exchange(*this,other.currency());
		    return Money(other.currency(),tmp.round()/other.value());
		}
		case BASE_CURRENCY_CONVERSION:
		{
		    Currency base = CurrencyParams::baseCurrency();
		    Decimal val_a = value_,
			val_b = other.value();
		    if (!(currency_ == base))
			val_a = rateManager_.exchange(*this,base).round();
		    if (!(other.currency() == base))
			val_b = rateManager_.exchange(other,base).round();
		    return Money(base,val_a/val_b);
		}
		default:
		    break;
	    }
	}
	QL_FAIL("Invalid currency Conversion Type");
	QL_DUMMY_RETURN(Money());
    }
    
    Money& Money::operator+=(Money other) {
	if (currency_ == other.currency()) {
	    value_ += other.value();
	}
	else {
	    switch (CurrencyParams::conversionType()) {
		case AUTOMATED_CONVERSION:
		{
		    Money tmp = rateManager_.exchange(other,currency_);
		    value_ += tmp.value();
		    break;
		}
		case BASE_CURRENCY_CONVERSION:
		{
		    Currency base = CurrencyParams::baseCurrency();
		    QL_REQUIRE(!(base.isNull()), "Null base currency");
		    Decimal ovalue = other.value();
		    if (!(currency_ == base))
			value_ = rateManager_.exchange(*this,base).round();
		    if (!(other.currency() == base))
			ovalue = rateManager_.exchange(other,base).round();
		    value_ += ovalue;
		    break;
		}
		default:
		    break;
	    }
	}
	return *this;
    }
    
    Money& Money::operator-=(Money other) {
	if (currency_ == other.currency())
	    value_ -= other.value();
	else {
	    switch (CurrencyParams::conversionType()) {
		case AUTOMATED_CONVERSION:
		{
		    Money tmp = rateManager_.exchange(other,currency_);
		    value_ -= tmp.value();
		    break;
		}
		case BASE_CURRENCY_CONVERSION:
		{
		    Currency base = CurrencyParams::baseCurrency();
		    QL_REQUIRE(!(base.isNull()), "Null base currency");
		    Decimal ovalue = other.value();
		    if (!(currency_ == base))
			value_ = rateManager_.exchange(*this,base).round();
		    if (!(other.currency() == base))
			ovalue = rateManager_.exchange(other,base).round();
		    value_ -= ovalue;
		    break;
		}
		default:
		    break;
	    }
	}
	return *this;
    }
    
}
