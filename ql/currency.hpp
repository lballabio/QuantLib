

/*
 Copyright (C) 2000, 2001, 2002 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email ferdinando@ametrano.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/
/*! \file currency.hpp
    \brief Known currencies

    \fullpath
    ql/%currency.hpp
*/

// $Id$

#ifndef quantlib_currency_h
#define quantlib_currency_h

#include <ql/qldefines.hpp>

namespace QuantLib {

    //! Known currencies
    enum Currency {
        AUD,    //!< Australian Dollar
        BGL,    //!< Bulgarian Lev
        CAD,    //!< Canadian Dollar
        CHF,    //!< Swiss Franc
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
        ISK,    //!< Iceland Krona
        ITL,    //!< Italian Lira
        JPY,    //!< Japanese Yen
        KRW,    //!< South-Korean Won
        LTL,    //!< Lithuanian Litas
        LVL,    //!< Latvian Lats
        MTL,    //!< Maltese Lira
        NOK,    //!< Norwegian Kroner
        NZD,    //!< New Zealand Dollar
        PLZ,    //!< Polish Zloty
        ROL,    //!< Romanian Leu
        SEK,    //!< Swedish Krona
        SGD,    //!< Singapore Dollar
        SIT,    //!< Slovenian Tolar
        SKK,    //!< Slovak Koruna
        TRL,    //!< Turkish Lira
        USD,    //!< US Dollar
        ZAR     //!< South African Rand
    };

}


#endif
