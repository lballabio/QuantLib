/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006, 2007 Ferdinando Ametrano
 Copyright (C) 2006 Katiuscia Manzoni

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

/*! \file euriborswapfixa.hpp
    \brief %EuriborSwapFixA indexes
*/

#ifndef quantlib_euriborswapfixa_hpp
#define quantlib_euriborswapfixa_hpp

#include <ql/indexes/swapindex.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>

namespace QuantLib {

    //! %EuriborSwapFixA index base class
    /*! EuriborSwapFixA indexes fixed by ISDA at 11:00AM FRANKFURT.
        Reuters page ISDAFIX2 or EURSFIXA=.

        \warning The 1Y swap's floating leg is based on Euribor3M; the
                 floating legs of longer swaps are based on Euribor6M
    */
    class EuriborSwapFixA : public SwapIndex {
      public:
        EuriborSwapFixA(const Period& tenor,
                        const Handle<YieldTermStructure>& h =
                                    Handle<YieldTermStructure>());
    };

    //! 1-year %EuriborSwapFixA index
    class EuriborSwapFixA1Y : public EuriborSwapFixA {
      public:
        EuriborSwapFixA1Y(const Handle<YieldTermStructure>& h)
        : EuriborSwapFixA(1*Years, h) {}
    };

    //! 2-year %EuriborSwapFixA index
    class EuriborSwapFixA2Y : public EuriborSwapFixA {
      public:
        EuriborSwapFixA2Y(const Handle<YieldTermStructure>& h)
        : EuriborSwapFixA(2*Years, h) {}
    };

    //! 3-year %EuriborSwapFixA index
    class EuriborSwapFixA3Y : public EuriborSwapFixA {
      public:
        EuriborSwapFixA3Y(const Handle<YieldTermStructure>& h)
        : EuriborSwapFixA(3*Years, h) {}
    };

    //! 4-year %EuriborSwapFixA index
    class EuriborSwapFixA4Y : public EuriborSwapFixA {
      public:
        EuriborSwapFixA4Y(const Handle<YieldTermStructure>& h)
        : EuriborSwapFixA(4*Years, h) {}
    };

    //! 5-year %EuriborSwapFixA index
    class EuriborSwapFixA5Y : public EuriborSwapFixA {
      public:
        EuriborSwapFixA5Y(const Handle<YieldTermStructure>& h)
        : EuriborSwapFixA(5*Years, h) {}
    };

    //! 6-year %EuriborSwapFixA index
    class EuriborSwapFixA6Y : public EuriborSwapFixA {
      public:
        EuriborSwapFixA6Y(const Handle<YieldTermStructure>& h)
        : EuriborSwapFixA(6*Years, h) {}
    };

    //! 7-year %EuriborSwapFixA index
    class EuriborSwapFixA7Y : public EuriborSwapFixA {
      public:
        EuriborSwapFixA7Y(const Handle<YieldTermStructure>& h)
        : EuriborSwapFixA(7*Years, h) {}
    };

    //! 8-year %EuriborSwapFixA index
    class EuriborSwapFixA8Y : public EuriborSwapFixA {
      public:
        EuriborSwapFixA8Y(const Handle<YieldTermStructure>& h)
        : EuriborSwapFixA(8*Years, h) {}
    };

    //! 9-year %EuriborSwapFixA index
    class EuriborSwapFixA9Y : public EuriborSwapFixA {
      public:
        EuriborSwapFixA9Y(const Handle<YieldTermStructure>& h)
        : EuriborSwapFixA(9*Years, h) {}
    };

    //! 10-year %EuriborSwapFixA index
    class EuriborSwapFixA10Y : public EuriborSwapFixA {
      public:
        EuriborSwapFixA10Y(const Handle<YieldTermStructure>& h)
        : EuriborSwapFixA(10*Years, h) {}
    };

    //! 12-year %EuriborSwapFixA index
    class EuriborSwapFixA12Y : public EuriborSwapFixA {
      public:
        EuriborSwapFixA12Y(const Handle<YieldTermStructure>& h)
        : EuriborSwapFixA(12*Years, h) {}
    };

    //! 15-year %EuriborSwapFixA index
    class EuriborSwapFixA15Y : public EuriborSwapFixA {
      public:
        EuriborSwapFixA15Y(const Handle<YieldTermStructure>& h)
        : EuriborSwapFixA(15*Years, h) {}
    };

    //! 20-year %EuriborSwapFixA index
    class EuriborSwapFixA20Y : public EuriborSwapFixA {
      public:
        EuriborSwapFixA20Y(const Handle<YieldTermStructure>& h)
        : EuriborSwapFixA(20*Years, h) {}
    };

    //! 25-year %EuriborSwapFixA index
    class EuriborSwapFixA25Y : public EuriborSwapFixA {
      public:
        EuriborSwapFixA25Y(const Handle<YieldTermStructure>& h)
        : EuriborSwapFixA(25*Years, h) {}
    };

    //! 30-year %EuriborSwapFixA index
    class EuriborSwapFixA30Y : public EuriborSwapFixA {
      public:
        EuriborSwapFixA30Y(const Handle<YieldTermStructure>& h)
        : EuriborSwapFixA(30*Years, h) {}
    };

}

#endif
