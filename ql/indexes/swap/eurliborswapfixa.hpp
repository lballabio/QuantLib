/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006, 2007 Ferdinando Ametrano
 Copyright (C) 2006 Chiara Fornarola

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

/*! \file eurliborswapfixa.hpp
    \brief %EurliborSwapFixA indexes
*/

#ifndef quantlib_eurliborswapfixa_hpp
#define quantlib_eurliborswapfixa_hpp

#include <ql/indexes/swapindex.hpp>

namespace QuantLib {

    //! %EurliborSwapFixA index base class
    /*! EurliborSwapFixA indexes fixed by ISDA in cooperation with
        Reuters and Intercapital Brokers at 10:00 AM London. 
        Reuters page ISDAFIX2 or EURSFIXLA=.
        Further info can be found at: <http://www.isda.org/fix/isdafix.html>.
        
        \warning The 1Y swap's floating leg is based on Euribor3M; the
                 floating legs of longer swaps are based on Euribor6M
    */
    class EurliborSwapFixA : public SwapIndex {
      public:
        EurliborSwapFixA(const Period& tenor,
                         const Handle<YieldTermStructure>& h =
                                    Handle<YieldTermStructure>());
    };

    //! 1-year %EurliborSwapFixA index
    class EurliborSwapFixA1Y : public EurliborSwapFixA {
      public:
        EurliborSwapFixA1Y(const Handle<YieldTermStructure>& h)
        : EurliborSwapFixA(1*Years, h) {}
    };

    //! 2-year %EurliborSwapFixA index
    class EurliborSwapFixA2Y : public EurliborSwapFixA {
      public:
        EurliborSwapFixA2Y(const Handle<YieldTermStructure>& h)
        : EurliborSwapFixA(2*Years, h) {}
    };

    //! 3-year %EurliborSwapFixA index
    class EurliborSwapFixA3Y : public EurliborSwapFixA {
      public:
        EurliborSwapFixA3Y(const Handle<YieldTermStructure>& h)
        : EurliborSwapFixA(3*Years, h) {}
    };

    //! 4-year %EurliborSwapFixA index
    class EurliborSwapFixA4Y : public EurliborSwapFixA {
      public:
        EurliborSwapFixA4Y(const Handle<YieldTermStructure>& h)
        : EurliborSwapFixA(4*Years, h) {}
    };

    //! 5-year %EurliborSwapFixA index
    class EurliborSwapFixA5Y : public EurliborSwapFixA {
      public:
        EurliborSwapFixA5Y(const Handle<YieldTermStructure>& h)
        : EurliborSwapFixA(5*Years, h) {}
    };

    //! 6-year %EurliborSwapFixA index
    class EurliborSwapFixA6Y : public EurliborSwapFixA {
      public:
        EurliborSwapFixA6Y(const Handle<YieldTermStructure>& h)
        : EurliborSwapFixA(6*Years, h) {}
    };

    //! 7-year %EurliborSwapFixA index
    class EurliborSwapFixA7Y : public EurliborSwapFixA {
      public:
        EurliborSwapFixA7Y(const Handle<YieldTermStructure>& h)
        : EurliborSwapFixA(7*Years, h) {}
    };

    //! 8-year %EurliborSwapFixA index
    class EurliborSwapFixA8Y : public EurliborSwapFixA {
      public:
        EurliborSwapFixA8Y(const Handle<YieldTermStructure>& h)
        : EurliborSwapFixA(8*Years, h) {}
    };

    //! 9-year %EurliborSwapFixA index
    class EurliborSwapFixA9Y : public EurliborSwapFixA {
      public:
        EurliborSwapFixA9Y(const Handle<YieldTermStructure>& h)
        : EurliborSwapFixA(9*Years, h) {}
    };

    //! 10-year %EurliborSwapFixA index
    class EurliborSwapFixA10Y : public EurliborSwapFixA {
      public:
        EurliborSwapFixA10Y(const Handle<YieldTermStructure>& h)
        : EurliborSwapFixA(10*Years, h) {}
    };

    //! 12-year %EurliborSwapFixA index
    class EurliborSwapFixA12Y : public EurliborSwapFixA {
      public:
        EurliborSwapFixA12Y(const Handle<YieldTermStructure>& h)
        : EurliborSwapFixA(12*Years, h) {}
    };

    //! 15-year %EurliborSwapFixA index
    class EurliborSwapFixA15Y : public EurliborSwapFixA {
      public:
        EurliborSwapFixA15Y(const Handle<YieldTermStructure>& h)
        : EurliborSwapFixA(15*Years, h) {}
    };

    //! 20-year %EurliborSwapFixA index
    class EurliborSwapFixA20Y : public EurliborSwapFixA {
      public:
        EurliborSwapFixA20Y(const Handle<YieldTermStructure>& h)
        : EurliborSwapFixA(20*Years, h) {}
    };

    //! 25-year %EurliborSwapFixA index
    class EurliborSwapFixA25Y : public EurliborSwapFixA {
      public:
        EurliborSwapFixA25Y(const Handle<YieldTermStructure>& h)
        : EurliborSwapFixA(25*Years, h) {}
    };

    //! 30-year %EurliborSwapFixA index
    class EurliborSwapFixA30Y : public EurliborSwapFixA {
      public:
        EurliborSwapFixA30Y(const Handle<YieldTermStructure>& h)
        : EurliborSwapFixA(30*Years, h) {}
    };

}

#endif
