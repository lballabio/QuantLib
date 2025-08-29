/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008, 2011 Ferdinando Ametrano

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

/*! \file jpyliborswap.hpp
    \brief %JPY %Libor %Swap indexes
*/

#ifndef quantlib_jpyliborswap_hpp
#define quantlib_jpyliborswap_hpp

#include <ql/indexes/swapindex.hpp>

namespace QuantLib {

    //! %JpyLiborSwapIsdaFixAm index base class
    /*! %JPY %Libor %Swap indexes fixed by ISDA in cooperation with
        Reuters and Intercapital Brokers at 10am Tokyo.
        Semiannual Act/365 vs 6M Libor. Reuters page ISDAFIX1 or JPYSFIXA=.

        Further info can be found at <http://www.isda.org/fix/isdafix.html> or
        Reuters page ISDAFIX.

    */
    class JpyLiborSwapIsdaFixAm : public SwapIndex {
      public:
        JpyLiborSwapIsdaFixAm(const Period& tenor,
                              const Handle<YieldTermStructure>& h = {});
        JpyLiborSwapIsdaFixAm(const Period& tenor,
                              const Handle<YieldTermStructure>& forwarding,
                              const Handle<YieldTermStructure>& discounting);
    };

    //! %JpyLiborSwapIsdaFixPm index base class
    /*! %JPY %Libor %Swap indexes fixed by ISDA in cooperation with
        Reuters and Intercapital Brokers at 3pm Tokyo.
        Semiannual Act/365 vs 6M Libor. Reuters page ISDAFIX1 or JPYSFIXP=.

        Further info can be found at <http://www.isda.org/fix/isdafix.html> or
        Reuters page ISDAFIX.

    */
    class JpyLiborSwapIsdaFixPm : public SwapIndex {
      public:
        JpyLiborSwapIsdaFixPm(const Period& tenor,
                              const Handle<YieldTermStructure>& h = {});
        JpyLiborSwapIsdaFixPm(const Period& tenor,
                              const Handle<YieldTermStructure>& forwarding,
                              const Handle<YieldTermStructure>& discounting);
    };

}

#endif
