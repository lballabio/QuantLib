/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2004, 2005, 2008 StatPro Italia srl
 Copyright (C) 2016 Quaternion Risk Management Ltd

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*
    Data from http://fx.sauder.ubc.ca/currency_table.html
    and http://www.thefinancials.com/vortex/CurrencyFormats.html
*/

#include <ql/currencies/america.hpp>

namespace QuantLib {

    QL_DEPRECATED_DISABLE_WARNING

    // Argentinian peso
    /* The ISO three-letter code is ARS; the numeric code is 32.
       It is divided in 100 centavos.
    */
     ARSCurrency::ARSCurrency() {
        static ext::shared_ptr<Data> arsData(
                                   new Data("Argentinian peso", "ARS", 32,
                                            "", "", 100,
                                            Rounding(),
                                            "%2% %1$.2f"));
        data_ = arsData;
    }

    // Brazilian real
    /* The ISO three-letter code is BRL; the numeric code is 986.
       It is divided in 100 centavos.
    */
    BRLCurrency::BRLCurrency() {
        static ext::shared_ptr<Data> brlData(
                                    new Data("Brazilian real", "BRL", 986,
                                             "R$", "", 100,
                                             Rounding(),
                                             "%3% %1$.2f"));
        data_ = brlData;
    }

    // Canadian dollar
    /* The ISO three-letter code is CAD; the numeric code is 124.
       It is divided into 100 cents.
    */
    CADCurrency::CADCurrency() {
        static ext::shared_ptr<Data> cadData(
                                   new Data("Canadian dollar", "CAD", 124,
                                            "Can$", "", 100,
                                            Rounding(),
                                            "%3% %1$.2f"));
        data_ = cadData;
    }

    // Chilean peso
    /* The ISO three-letter code is CLP; the numeric code is 152.
      It is divided in 100 centavos.
    */
    CLPCurrency::CLPCurrency() {
        static ext::shared_ptr<Data> clpData(
                                      new Data("Chilean peso", "CLP", 152,
                                               "Ch$", "", 100,
                                               Rounding(),
                                               "%3% %1$.0f"));
        data_ = clpData;
    }

    // Colombian peso
    /* The ISO three-letter code is COP; the numeric code is 170.
       It is divided in 100 centavos.
    */
    COPCurrency::COPCurrency() {
        static ext::shared_ptr<Data> copData(
                                    new Data("Colombian peso", "COP", 170,
                                             "Col$", "", 100,
                                             Rounding(),
                                             "%3% %1$.2f"));
        data_ = copData;
    }

    // Mexican peso
    /* The ISO three-letter code is MXN; the numeric code is 484.
       It is divided in 100 centavos.
    */
    MXNCurrency::MXNCurrency() {
        static ext::shared_ptr<Data> mxnData(
                                      new Data("Mexican peso", "MXN", 484,
                                               "Mex$", "", 100,
                                               Rounding(),
                                               "%3% %1$.2f"));
        data_ = mxnData;
    }

    // Peruvian nuevo sol
    /* The ISO three-letter code is PEN; the numeric code is 604.
       It is divided in 100 centimos.
    */
    PENCurrency::PENCurrency() {
        static ext::shared_ptr<Data> penData(
                                new Data("Peruvian nuevo sol", "PEN", 604,
                                         "S/.", "", 100,
                                         Rounding(),
                                         "%3% %1$.2f"));
        data_ = penData;
    }

    // Peruvian inti
    /* The ISO three-letter code was PEI.
       It was divided in 100 centimos. A numeric code is not available;
       as per ISO 3166-1, we assign 998 as a user-defined code.
    */
    PEICurrency::PEICurrency() {
        static ext::shared_ptr<Data> peiData(
                                new Data("Peruvian inti", "PEI", 998,
                                         "I/.", "", 100,
                                         Rounding(),
                                         "%3% %1$.2f"));
        data_ = peiData;
    }

    // Peruvian sol
    /* The ISO three-letter code was PEH. A numeric code is not available;
       as per ISO 3166-1, we assign 999 as a user-defined code.
       It was divided in 100 centavos.
    */
    PEHCurrency::PEHCurrency() {
        static ext::shared_ptr<Data> pehData(
                                new Data("Peruvian sol", "PEH", 999,
                                         "S./", "", 100,
                                         Rounding(),
                                         "%3% %1$.2f"));
        data_ = pehData;
    }

    // Trinidad & Tobago dollar
    /* The ISO three-letter code is TTD; the numeric code is 780.
       It is divided in 100 cents.
    */
    TTDCurrency::TTDCurrency() {
        static ext::shared_ptr<Data> ttdData(
                          new Data("Trinidad & Tobago dollar", "TTD", 780,
                                   "TT$", "", 100,
                                   Rounding(),
                                   "%3% %1$.2f"));
        data_ = ttdData;
    }

    // U.S. dollar
    /* The ISO three-letter code is USD; the numeric code is 840.
       It is divided in 100 cents.
    */
    USDCurrency::USDCurrency() {
        static ext::shared_ptr<Data> usdData(
                                       new Data("U.S. dollar", "USD", 840,
                                                "$", "\xA2", 100,
                                                Rounding(),
                                                "%3% %1$.2f"));
        data_ = usdData;
    }

    // Venezuelan bolivar
    /* The ISO three-letter code is VEB; the numeric code is 862.
       It is divided in 100 centimos.
    */
    VEBCurrency::VEBCurrency() {
        static ext::shared_ptr<Data> vebData(
                                new Data("Venezuelan bolivar", "VEB", 862,
                                         "Bs", "", 100,
                                         Rounding(),
                                         "%3% %1$.2f"));
        data_ = vebData;
    }

    // Mexican Unidad de Inversion
    MXVCurrency::MXVCurrency() {
        static ext::shared_ptr<Data> mxvData(new Data("Mexican Unidad de Inversion", "MXV", 979,
                                                        "MXV", "", 1, Rounding(), "1$.2f %3%"));
        data_ = mxvData;
    }

    // Unidad de Valor Real
    COUCurrency::COUCurrency() {
        static ext::shared_ptr<Data> couData(new Data("Unidad de Valor Real (UVR) (funds code)",
                                                        "COU", 970, "COU", "", 100, Rounding(),
                                                        "1$.2f %3%"));
        data_ = couData;
    }

    // Unidad de Fomento
    CLFCurrency::CLFCurrency() {
        static ext::shared_ptr<Data> clfData(new Data(
            "Unidad de Fomento (funds code)", "CLF", 990, "CLF", "", 1, Rounding(), "1$.2f %3%"));
        data_ = clfData;
    }

    // Uruguayan peso
    UYUCurrency::UYUCurrency() {
        static ext::shared_ptr<Data> uyuData(
            new Data("Uruguayan peso", "UYU", 858, "UYU", "", 1, Rounding(), "1$.2f %3%"));
        data_ = uyuData;
    }

    QL_DEPRECATED_ENABLE_WARNING

}

