
/*
 * Copyright (C) 2000-2001 QuantLib Group
 *
 * This file is part of QuantLib.
 * QuantLib is a C++ open source library for financial quantitative
 * analysts and developers --- http://quantlib.org/
 *
 * QuantLib is free software and you are allowed to use, copy, modify, merge,
 * publish, distribute, and/or sell copies of it under the conditions stated
 * in the QuantLib License.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the license for more details.
 *
 * You should have received a copy of the license along with this file;
 * if not, please email quantlib-users@lists.sourceforge.net
 * The license is also available at http://quantlib.org/LICENSE.TXT
 *
 * The members of the QuantLib Group are listed in the Authors.txt file, also
 * available at http://quantlib.org/group.html
*/

/*! \file currency.hpp
    \fullpath Include/ql/%currency.hpp
    \brief Known currencies

*/

// $Id$
// $Log$
// Revision 1.10  2001/08/09 14:59:45  sigmud
// header modification
//
// Revision 1.9  2001/08/08 11:07:48  sigmud
// inserting \fullpath for doxygen
//
// Revision 1.8  2001/08/07 11:25:53  sigmud
// copyright header maintenance
//
// Revision 1.7  2001/07/25 15:47:27  sigmud
// Change from quantlib.sourceforge.net to quantlib.org
//
// Revision 1.6  2001/06/04 10:47:47  lballabio
// Added a truckload of currencies
//
// Revision 1.5  2001/05/24 15:38:07  nando
// smoothing #include xx.hpp and cutting old Log messages
//

#ifndef quantlib_currency_h
#define quantlib_currency_h

#include "ql/qldefines.hpp"

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
