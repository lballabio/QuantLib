/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Ferdinando Ametrano
 Copyright (C) 2006 Chiara Fornarola

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/reference/license.html>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file eurliborswapfixifr.hpp
    \brief %EurliborSwapFixIFR indexes
*/

#ifndef quantlib_eurliborswapfixifr_hpp
#define quantlib_eurliborswapfixifr_hpp

#include <ql/indexes/swapindex.hpp>
#include <ql/indexes/eurlibor.hpp>
#include <ql/indexes/libor.hpp>
#include <ql/time/calendars/target.hpp>
#include <ql/time/daycounters/thirty360.hpp>
#include <ql/currencies/europe.hpp>

namespace QuantLib {

    //! %EurliborSwapFixIFR vs 3M index base class
    /*! EuriborSwapFix index published by IFR Markets and distributed
        by Reuters page TGM42281 and by Telerate. For more info see
        <http://www.ifrmarkets.com>.
    */
    class EurliborSwapFixIFRvs3M : public SwapIndex {
      public:
        EurliborSwapFixIFRvs3M(const Period& tenor,
                               const Handle<YieldTermStructure>& h =
                                    Handle<YieldTermStructure>());
    };

    //! %EurliborSwapFixIFR vs 6M index base class
    /*! EuriborSwapFix index published by IFR Markets and distributed
        by Reuters page TGM42281 and by Telerate. For more info see
        <http://www.ifrmarkets.com>.
    */
    class EurliborSwapFixIFRvs6M : public SwapIndex {
      public:
        EurliborSwapFixIFRvs6M(const Period& tenor,
                               const Handle<YieldTermStructure>& h =
                                    Handle<YieldTermStructure>());
    };

    //! 1-year %EurliborSwapFixIFRvs3M index
    class EurliborSwapFixIFR1Y : public EurliborSwapFixIFRvs3M {
      public:
        EurliborSwapFixIFR1Y(const Handle<YieldTermStructure>& h)
        : EurliborSwapFixIFRvs3M(1*Years, h) {}
    };

    //! 2-year %EurliborSwapFixIFRvs6M index
    class EurliborSwapFixIFR2Y : public EurliborSwapFixIFRvs6M {
      public:
        EurliborSwapFixIFR2Y(const Handle<YieldTermStructure>& h)
        : EurliborSwapFixIFRvs6M(2*Years, h) {}
    };

    //! 3-year %EurliborSwapFixIFRvs6M index
    class EurliborSwapFixIFR3Y : public EurliborSwapFixIFRvs6M {
      public:
        EurliborSwapFixIFR3Y(const Handle<YieldTermStructure>& h)
        : EurliborSwapFixIFRvs6M(3*Years, h) {}
    };

    //! 4-year %EurliborSwapFixIFRvs6M index
    class EurliborSwapFixIFR4Y : public EurliborSwapFixIFRvs6M {
      public:
        EurliborSwapFixIFR4Y(const Handle<YieldTermStructure>& h)
        : EurliborSwapFixIFRvs6M(4*Years, h) {}
    };

    //! 5-year %EurliborSwapFixIFRvs6M index
    class EurliborSwapFixIFR5Y : public EurliborSwapFixIFRvs6M {
      public:
        EurliborSwapFixIFR5Y(const Handle<YieldTermStructure>& h)
        : EurliborSwapFixIFRvs6M(5*Years, h) {}
    };

    //! 6-year %EurliborSwapFixIFRvs6M index
    class EurliborSwapFixIFR6Y : public EurliborSwapFixIFRvs6M {
      public:
        EurliborSwapFixIFR6Y(const Handle<YieldTermStructure>& h)
        : EurliborSwapFixIFRvs6M(6*Years, h) {}
    };

    //! 7-year %EurliborSwapFixIFRvs6M index
    class EurliborSwapFixIFR7Y : public EurliborSwapFixIFRvs6M {
      public:
        EurliborSwapFixIFR7Y(const Handle<YieldTermStructure>& h)
        : EurliborSwapFixIFRvs6M(7*Years, h) {}
    };

    //! 8-year %EurliborSwapFixIFRvs6M index
    class EurliborSwapFixIFR8Y : public EurliborSwapFixIFRvs6M {
      public:
        EurliborSwapFixIFR8Y(const Handle<YieldTermStructure>& h)
        : EurliborSwapFixIFRvs6M(8*Years, h) {}
    };

    //! 9-year %EurliborSwapFixIFRvs6M index
    class EurliborSwapFixIFR9Y : public EurliborSwapFixIFRvs6M {
      public:
        EurliborSwapFixIFR9Y(const Handle<YieldTermStructure>& h)
        : EurliborSwapFixIFRvs6M(9*Years, h) {}
    };

    //! 10-year %EurliborSwapFixIFRvs6M index
    class EurliborSwapFixIFR10Y : public EurliborSwapFixIFRvs6M {
      public:
        EurliborSwapFixIFR10Y(const Handle<YieldTermStructure>& h)
        : EurliborSwapFixIFRvs6M(10*Years, h) {}
    };

    //! 12-year %EurliborSwapFixIFRvs6M index
    class EurliborSwapFixIFR12Y : public EurliborSwapFixIFRvs6M {
      public:
        EurliborSwapFixIFR12Y(const Handle<YieldTermStructure>& h)
        : EurliborSwapFixIFRvs6M(12*Years, h) {}
    };

    //! 15-year %EurliborSwapFixIFRvs6M index
    class EurliborSwapFixIFR15Y : public EurliborSwapFixIFRvs6M {
      public:
        EurliborSwapFixIFR15Y(const Handle<YieldTermStructure>& h)
        : EurliborSwapFixIFRvs6M(15*Years, h) {}
    };

    //! 20-year %EurliborSwapFixIFRvs6M index
    class EurliborSwapFixIFR20Y : public EurliborSwapFixIFRvs6M {
      public:
        EurliborSwapFixIFR20Y(const Handle<YieldTermStructure>& h)
        : EurliborSwapFixIFRvs6M(20*Years, h) {}
    };

    //! 25-year %EurliborSwapFixIFRvs6M index
    class EurliborSwapFixIFR25Y : public EurliborSwapFixIFRvs6M {
      public:
        EurliborSwapFixIFR25Y(const Handle<YieldTermStructure>& h)
        : EurliborSwapFixIFRvs6M(25*Years, h) {}
    };

    //! 30-year %EurliborSwapFixIFRvs6M index
    class EurliborSwapFixIFR30Y : public EurliborSwapFixIFRvs6M {
      public:
        EurliborSwapFixIFR30Y(const Handle<YieldTermStructure>& h)
        : EurliborSwapFixIFRvs6M(30*Years, h) {}
    };

}

#endif
