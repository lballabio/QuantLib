/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2004, 2005 StatPro Italia srl

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

/*! \file crypto.hpp
    \brief crypto currencies

    Data from https://coinmarketcap.com
*/

#ifndef quantlib_crypto_currencies_hpp
#define quantlib_crypto_currencies_hpp

#include <ql/currency.hpp>

#if defined(QL_PATCH_MSVC)
#pragma warning(push)
#pragma warning(disable:4819)
#endif

namespace QuantLib {

    //! Bitcoin
    /*! https://bitcoin.org/

        \ingroup currencies
    */
    class BTCCurrency : public Currency {
      public:
        BTCCurrency();
    };

    //! Ethereum
    /*! https://www.ethereum.org/

        \ingroup currencies
    */
    class ETHCurrency : public Currency {
      public:
        ETHCurrency();
    };

    //! Ethereum Classic
    /*! https://ethereumclassic.github.io/

        \ingroup currencies
    */
    class ETCCurrency : public Currency {
      public:
        ETCCurrency();
    };

    //! Bitcoin Cash
    /*! https://www.bitcoincash.org/

        \ingroup currencies
    */
    class BCHCurrency : public Currency {
      public:
        BCHCurrency();
    };

    //! Ripple
    /*! https://ripple.com/

        \ingroup currencies
    */
    class XRPCurrency : public Currency {
      public:
        XRPCurrency();
    };

    //! Litecoin
    /*! https://litecoin.com/

        \ingroup currencies
    */
    class LTCCurrency : public Currency {
      public:
        LTCCurrency();
    };

    //! Dash coin
    /*! https://www.dash.org/

        \ingroup currencies
    */
    class DASHCurrency : public Currency {
      public:
        DASHCurrency();
    };

    //! Zcash
    /*! https://z.cash/

        \ingroup currencies
    */
    class ZECCurrency : public Currency {
      public:
        ZECCurrency();
    };
}

#if defined(QL_PATCH_MSVC)
#pragma warning(pop)
#endif

#endif


#ifndef id_b4a0e2dd8281fa7e20bfd1c02a387007
#define id_b4a0e2dd8281fa7e20bfd1c02a387007
inline bool test_b4a0e2dd8281fa7e20bfd1c02a387007(int* i) { return i != 0; }
#endif
