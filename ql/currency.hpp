
/*
 Copyright (C) 2000, 2001, 2002 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software developed by the QuantLib Group; you can
 redistribute it and/or modify it under the terms of the QuantLib License;
 either version 1.0, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 QuantLib License for more details.

 You should have received a copy of the QuantLib License along with this
 program; if not, please email ferdinando@ametrano.net

 The QuantLib License is also available at http://quantlib.org/license.html
 The members of the QuantLib Group are listed in the QuantLib License
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
