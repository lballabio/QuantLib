
/*
 Copyright (C) 2004 Decillion Pty(Ltd)
 Copyright (C) 2004 StatPro Italia srl

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
    \brief Known currencies
*/

#ifndef quantlib_currency_hpp
#define quantlib_currency_hpp

#include <ql/Math/rounding.hpp>
#include <ql/errors.hpp>
#include <boost/shared_ptr.hpp>
#include <string>

namespace QuantLib {

    //! Currency specification
    class Currency {
      public:
        //! default constructor
        /*! Instances built via this constructor have undefined
            behavior. Such instances can only act as placeholders
            and must be reassigned to a valid currency before being
            used.
        */
        Currency() {}
        //! \name Inspectors
        //@{
        //! currency name, e.g, "U.S. Dollar"
        const std::string& name() const { return name_; }
        //! ISO 4217 three-letter code, e.g, "USD"
        const std::string& code() const { return code_; }
        //! ISO 4217 numeric code, e.g, "840"
        Integer numericCode() const { return numeric_; }
        //! symbol, e.g, "$"
        const std::string& symbol() const { return symbol_; }
        //! fraction symbol, e.g, "¢"
        const std::string& fractionSymbol() const { return fractionSymbol_; }
        //! number of fractionary parts in a unit, e.g, 100
        Integer fractionsPerUnit() const { return fractionsPerUnit_; }
        //! rounding convention
        const Rounding& rounding() const { return rounding_; }
        //@}
        //! \name other info
        //@{
        //! is this a usable instance?
        bool isValid() const { return !name_.empty(); }
        //! currency used for triangulated exchange when required
        const Currency& triangulationCurrency() const {
            return *triangulated_;
        }
        //@}
      protected:
        Currency(const std::string& name,
                 const std::string& code,
                 Integer numericCode,
                 const std::string& symbol,
                 const std::string& fractionSymbol,
                 Integer fractionsPerUnit,
                 const Rounding& rounding,
                 const Currency& triangulationCurrency = Currency())
	    : name_(name), code_(code), numeric_(numericCode),
	      symbol_(symbol), fractionsPerUnit_(fractionsPerUnit),
          rounding_(rounding) {
            QL_REQUIRE(!name.empty(), "no currency name given");
            triangulated_ = boost::shared_ptr<Currency>(
                                         new Currency(triangulationCurrency));
        }
	    std::string name_, code_;
        Integer numeric_;
        std::string symbol_, fractionSymbol_;
        Integer fractionsPerUnit_;
	    Rounding rounding_;
        boost::shared_ptr<Currency> triangulated_;
    };


    /*! \brief comparison based on name
        \relates Currency
    */
    bool operator==(const Currency&, const Currency&);

    /*! \brief comparison based on name
        \relates Currency
    */
    bool operator!=(const Currency&, const Currency&);


    //! Tags for known currencies
    /*! \ingroup currencies */
    enum CurrencyTag {
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


    //! format currencies for output
    class CurrencyFormatter {
      public:
        static std::string toString(CurrencyTag c);
        static std::string toString(const Currency& c);
    };


    // inline definitions

    inline bool operator==(const Currency& c1, const Currency& c2) {
        return c1.name() == c2.name();
    }

    inline bool operator!=(const Currency& c1, const Currency& c2) {
        return !(c1 == c2);
    }

}


#endif
