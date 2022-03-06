/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008, 2009 Ferdinando Ametrano

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

/*! \file eurliborswap.hpp
    \brief %EUR %Libor %Swap indexes
*/

#ifndef quantlib_eurliborswap_hpp
#define quantlib_eurliborswap_hpp

#include <ql/indexes/swapindex.hpp>

namespace QuantLib {

    //! %EurLiborSwapIsdaFixA index base class
    /*! %EUR %Libor %Swap indexes fixed by ISDA in cooperation with
        Reuters and Intercapital Brokers at 10am London.
        Annual 30/360 vs 6M Libor, 1Y vs 3M Libor.
        Reuters page ISDAFIX2 or EURSFIXLA=.

        Further info can be found at <http://www.isda.org/fix/isdafix.html> or
        Reuters page ISDAFIX.

    */
    class EurLiborSwapIsdaFixA : public SwapIndex {
      public:
        EurLiborSwapIsdaFixA(const Period& tenor,
                             const Handle<YieldTermStructure>& h =
                                    Handle<YieldTermStructure>());
        EurLiborSwapIsdaFixA(const Period& tenor,
                             const Handle<YieldTermStructure>& forwarding,
                             const Handle<YieldTermStructure>& discounting);
    };

    //! %EurLiborSwapIsdaFixB index base class
    /*! %EUR %Libor %Swap indexes fixed by ISDA in cooperation with
        Reuters and Intercapital Brokers at 11am London.
        Annual 30/360 vs 6M Libor, 1Y vs 3M Libor.
        Reuters page ISDAFIX2 or EURSFIXLB=.

        Further info can be found at <http://www.isda.org/fix/isdafix.html> or
        Reuters page ISDAFIX.

    */
    class EurLiborSwapIsdaFixB : public SwapIndex {
      public:
        EurLiborSwapIsdaFixB(const Period& tenor,
                             const Handle<YieldTermStructure>& h =
                                    Handle<YieldTermStructure>());
        EurLiborSwapIsdaFixB(const Period& tenor,
                             const Handle<YieldTermStructure>& forwarding,
                             const Handle<YieldTermStructure>& discounting);
    };


    //! %EurLiborSwapIfrFix index base class
    /*! %EUR %Libor %Swap indexes published by IFR Markets and
        distributed by Reuters page TGM42281 and by Telerate.
        Annual 30/360 vs 6M Libor, 1Y vs 3M Libor.
        For more info see <http://www.ifrmarkets.com>.

    */
    class EurLiborSwapIfrFix : public SwapIndex {
      public:
        EurLiborSwapIfrFix(const Period& tenor,
                           const Handle<YieldTermStructure>& h =
                                    Handle<YieldTermStructure>());
        EurLiborSwapIfrFix(const Period& tenor,
                           const Handle<YieldTermStructure>& forwarding,
                           const Handle<YieldTermStructure>& discounting);
    };

}

#endif


#ifndef id_4861e81c301909409894c77735b310b8
#define id_4861e81c301909409894c77735b310b8
inline bool test_4861e81c301909409894c77735b310b8(const int* i) {
    return i != nullptr;
}
#endif
