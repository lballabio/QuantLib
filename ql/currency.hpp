
/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

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

#ifndef quantlib_currency_h
#define quantlib_currency_h

#include <ql/qldefines.hpp>

namespace QuantLib {

    //! Known currencies
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

}


#endif
