
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

/*! \file currency.hpp
    \brief Currency base
*/

#ifndef quantlib_currency_hpp
#define quantlib_currency_hpp

#include <ql/types.hpp>
#include <ql/Patterns/bridge.hpp>
#include <ql/Math/rounding.hpp>

namespace QuantLib {

    //! Currency structure
    /*! This abstract class defines the interface of concrete currencies
        which will be derived from it.
    */
    
    class Currency;
    
    class CurrencyImpl {
    public:
	virtual ~CurrencyImpl() {}
	virtual std::string name() const = 0;
	virtual std::string mnemonic() const = 0;
	virtual std::string numeric() const = 0;
	virtual std::string symbol() const = 0;
	virtual bool isRounding() const = 0;
	virtual Rounding prefRounding() const = 0;
	virtual Currency nextCurrency() const = 0;
	virtual Currency lastCurrency() const = 0;
    };
    
    class Currency : public Bridge<Currency,CurrencyImpl> {
    public:
	/*! \name Currency specification

	These methods defines the basic get/set interface to the
	Currency entity.
	*/
	//@{
	//! Currency name. e.g "U.S Dollar"
	std::string name() const {
	    return impl_->name();
	}
	//! Mnemonic e.g "USD"
	std::string mnemonic() const {
	    return impl_->mnemonic();
	}
	//! Numeric code e.g "840" (used for ISO currencies)
	std::string numeric() const {
	    return impl_->numeric();
	}
	//! Symbol e.g "$"
	std::string symbol() const {
	    return impl_->symbol();
	}
	bool isRounding() const {
	    return impl_->isRounding();
	}
	//! Preferred rounding
	Rounding prefRounding() const {
	    return impl_->prefRounding();
	}
	//! Exchange triangulation currencies
	Currency nextCurrency() const {
	    return impl_->nextCurrency();
	}
	Currency lastCurrency() const {
	    return impl_->lastCurrency();
	}
        /*! This default constructor returns a currency with a null 
            implementation, which is therefore unusable except as a 
            placeholder.
        */
	Currency() {}
	//@}
    protected:
	Currency(const boost::shared_ptr<CurrencyImpl>& impl)
	: Bridge<Currency,CurrencyImpl>(impl) {}
    };

    /*! Returns <tt>true</tt> iff the two currencies belong to the same
        derived class.
        \relates Currency
    */
    bool operator==(const Currency&, const Currency&);

    /*! \relates Currency */
    bool operator!=(const Currency&, const Currency&);

    inline bool operator==(const Currency& c1, const Currency& c2) {
        return (c1.isNull() && c2.isNull())
            || (!c1.isNull() && !c2.isNull() && c1.name() == c2.name());
    }

    inline bool operator!=(const Currency& c1, const Currency& c2) {
	return !(c1 == c2);
    }

    //! Known currencies
    enum ECurrency {
    /*! \ingroup currencies */
        ARS,    //!< Argentinian Peso
        ATS,    //!< Austrian Schillings
        AUD,    //!< Australian Dollar
        BDT,    //!< Bangladesh Taka
        BEF,    //!< Belgian Franc
        BGL,    //!< Bulgarian Lev
        BRL,    //!< Brazilian Real
        BYB,    //!< Belarusian Ruble
        CAD,    //!< Canadian Dollar
        CHF,    //!< Swiss Franc
        CLP,    //!< Chilean Peso
        CNY,    //!< Chinese Yuan
        COP,    //!< Colombian Peso
        CYP,    //!< Cyprus Pound
        CZK,    //!< Czech Koruna
        DEM,    //!< German Mark
        DKK,    //!< Danish Krone
        EEK,    //!< Estonian Kroon
        EUR,    //!< Euro
        GBP,    //!< British Pound
        GRD,    //!< Greek Drachma
        HKD,    //!< Hong Kong Dollar
        HUF,    //!< Hungarian Forint
        ILS,    //!< Israeli Shekel
        INR,    //!< Indian Rupee
        IQD,    //!< Iraqi Dinar
        IRR,    //!< Iranian Rial
        ISK,    //!< Iceland Krona
        ITL,    //!< Italian Lira
        JPY,    //!< Japanese Yen
        KRW,    //!< South-Korean Won
        KWD,    //!< Kuwaiti dinar
        LTL,    //!< Lithuanian Litas
        LVL,    //!< Latvian Lats
        MTL,    //!< Maltese Lira
        MXP,    //!< Mexican Peso
        NOK,    //!< Norwegian Kroner
        NPR,    //!< Nepal Rupee
        NZD,    //!< New Zealand Dollar
        PKR,    //!< Pakistani Rupee
        PLN,    //!< New Polish Zloty
        ROL,    //!< Romanian Leu
        SAR,    //!< Saudi Riyal
        SEK,    //!< Swedish Krona
        SGD,    //!< Singapore Dollar
        SIT,    //!< Slovenian Tolar
        SKK,    //!< Slovak Koruna
        THB,    //!< Thai Baht
        TRL,    //!< Turkish Lira
        TTD,    //!< Trinidad & Tobago dollar
        TWD,    //!< Taiwan Dollar
        USD,    //!< US Dollar
        VEB,    //!< Venezuelan Bolivar
        ZAR     //!< South African Rand
    };

    //! Generic Currency structure
    /*! This concrete class defines a generic implementation of the Currency
      class.
    */
    
    class GenericCurrency : public Currency {
    private:
	class Impl : public CurrencyImpl {
	public:
	    Impl(std::string name,
		 std::string mnemonic,
		 std::string numeric,
		 std::string symbol,
		 bool isRounding,
		 Rounding prefRounding,
		 Currency& nextCurrency,
		 Currency& lastCurrency)
	    : name_(name),mnemonic_(mnemonic),numeric_(numeric),
	      symbol_(symbol),isRounding_(isRounding),
	      prefRounding_(prefRounding) {}
	    std::string name() const { return name_; };
	    std::string mnemonic() const { return mnemonic_; };
	    std::string numeric() const { return numeric_; };
	    std::string symbol() const { return symbol_; };
	    bool isRounding() const { return isRounding_; }
	    Rounding prefRounding() const { return prefRounding_; }
	    Currency nextCurrency() const { return nextCurrency_; }
	    Currency lastCurrency() const { return lastCurrency_; }
	private:
	    std::string name_;
	    std::string mnemonic_;
	    std::string numeric_;
	    std::string symbol_;
	    bool isRounding_;
	    Rounding prefRounding_;
	    Currency nextCurrency_;
	    Currency lastCurrency_;
	};
    public:
	GenericCurrency(std::string name,
			std::string mnemonic,
			std::string numeric,
			std::string symbol,
			bool isRounding,
			Rounding prefRounding,
			Currency& nextCurrency,
			Currency& lastCurrency)
	: Currency(boost::shared_ptr<CurrencyImpl>(
		       new Impl(name,mnemonic,numeric,symbol,
				isRounding,prefRounding,
				nextCurrency,lastCurrency))) {}
    };
    
}


#endif
