/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008, 2009 Ferdinando Ametrano

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <https://www.quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file euriborswap.hpp
    \brief Euribor %Swap indexes
*/

#ifndef quantlib_euriborswap_hpp
#define quantlib_euriborswap_hpp

#include <ql/indexes/swapindex.hpp>

namespace QuantLib {

    //! %EuriborSwapIsdaFixA index base class
    /*! %Euribor %Swap indexes fixed by ISDA in cooperation with
        Reuters and Intercapital Brokers at 11am Frankfurt.
        Annual 30/360 vs 6M Euribor, 1Y vs 3M Euribor.
        Reuters page ISDAFIX2 or EURSFIXA=.

        Further info can be found at <http://www.isda.org/fix/isdafix.html> or
        Reuters page ISDAFIX.

    */
    class EuriborSwapIsdaFixA : public SwapIndex {
      public:
        EuriborSwapIsdaFixA(const Period& tenor,
                            const Handle<YieldTermStructure>& h = {});
        EuriborSwapIsdaFixA(const Period& tenor,
                            const Handle<YieldTermStructure>& forwarding,
                            const Handle<YieldTermStructure>& discounting);
    };

    //! %EuriborSwapIsdaFixB index base class
    /*! %Euribor %Swap indexes fixed by ISDA in cooperation with
        Reuters and Intercapital Brokers at 12am Frankfurt.
        Annual 30/360 vs 6M Euribor, 1Y vs 3M Euribor.
        Reuters page ISDAFIX2 or EURSFIXB=.

        Further info can be found at <http://www.isda.org/fix/isdafix.html> or
        Reuters page ISDAFIX.

    */
    class EuriborSwapIsdaFixB : public SwapIndex {
      public:
        EuriborSwapIsdaFixB(const Period& tenor,
                            const Handle<YieldTermStructure>& h = {});
        EuriborSwapIsdaFixB(const Period& tenor,
                            const Handle<YieldTermStructure>& forwarding,
                            const Handle<YieldTermStructure>& discounting);
    };

    //! %EuriborSwapIfrFix index base class
    /*! %Euribor %Swap indexes published by IFR Markets and
        distributed by Reuters page TGM42281 and by Telerate.
        Annual 30/360 vs 6M Euribor, 1Y vs 3M Euribor.
        For more info see <http://www.ifrmarkets.com>.

    */
    class EuriborSwapIfrFix : public SwapIndex {
      public:
        EuriborSwapIfrFix(const Period& tenor,
                          const Handle<YieldTermStructure>& h = {});
        EuriborSwapIfrFix(const Period& tenor,
                          const Handle<YieldTermStructure>& forwarding,
                          const Handle<YieldTermStructure>& discounting);
    };

}

#endif
