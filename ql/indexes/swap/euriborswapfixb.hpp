/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Ferdinando Ametrano
 Copyright (C) 2007 Katiuscia Manzoni

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

/*! \file euriborswapfixb.hpp
    \brief %EuriborSwapFixB indexes
*/

#ifndef quantlib_euriborswapfixb_hpp
#define quantlib_euriborswapfixb_hpp

#include <ql/indexes/swapindex.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>

namespace QuantLib {

    //! %EuriborSwapFix index base class
    /*! EuriborSwapFixB indexes fixed by ISDA at 12:00AM FRANKFURT.
        Reuters page ISDAFIX2 or EURSFIXB=.

        \warning The 1Y swap's floating leg is based on Euribor3M; the
                 floating legs of longer swaps are based on Euribor6M
    */
    class EuriborSwapFixB : public SwapIndex {
      public:
        EuriborSwapFixB(const Period& tenor,
                        const Handle<YieldTermStructure>& h =
                                    Handle<YieldTermStructure>());
    };

    //! 1-year %EuriborSwapFixB index
    class EuriborSwapFixB1Y : public EuriborSwapFixB {
      public:
        EuriborSwapFixB1Y(const Handle<YieldTermStructure>& h)
        : EuriborSwapFixB(1*Years, h) {}
    };

    //! 2-year %EuriborSwapFixB index
    class EuriborSwapFixB2Y : public EuriborSwapFixB {
      public:
        EuriborSwapFixB2Y(const Handle<YieldTermStructure>& h)
        : EuriborSwapFixB(2*Years, h) {}
    };

    //! 3-year %EuriborSwapFixB index
    class EuriborSwapFixB3Y : public EuriborSwapFixB {
      public:
        EuriborSwapFixB3Y(const Handle<YieldTermStructure>& h)
        : EuriborSwapFixB(3*Years, h) {}
    };

    //! 4-year %EuriborSwapFixB index
    class EuriborSwapFixB4Y : public EuriborSwapFixB {
      public:
        EuriborSwapFixB4Y(const Handle<YieldTermStructure>& h)
        : EuriborSwapFixB(4*Years, h) {}
    };

    //! 5-year %EuriborSwapFixB index
    class EuriborSwapFixB5Y : public EuriborSwapFixB {
      public:
        EuriborSwapFixB5Y(const Handle<YieldTermStructure>& h)
        : EuriborSwapFixB(5*Years, h) {}
    };

    //! 6-year %EuriborSwapFixB index
    class EuriborSwapFixB6Y : public EuriborSwapFixB {
      public:
        EuriborSwapFixB6Y(const Handle<YieldTermStructure>& h)
        : EuriborSwapFixB(6*Years, h) {}
    };

    //! 7-year %EuriborSwapFixB index
    class EuriborSwapFixB7Y : public EuriborSwapFixB {
      public:
        EuriborSwapFixB7Y(const Handle<YieldTermStructure>& h)
        : EuriborSwapFixB(7*Years, h) {}
    };

    //! 8-year %EuriborSwapFixB index
    class EuriborSwapFixB8Y : public EuriborSwapFixB {
      public:
        EuriborSwapFixB8Y(const Handle<YieldTermStructure>& h)
        : EuriborSwapFixB(8*Years, h) {}
    };

    //! 9-year %EuriborSwapFixB index
    class EuriborSwapFixB9Y : public EuriborSwapFixB {
      public:
        EuriborSwapFixB9Y(const Handle<YieldTermStructure>& h)
        : EuriborSwapFixB(9*Years, h) {}
    };

    //! 10-year %EuriborSwapFixB index
    class EuriborSwapFixB10Y : public EuriborSwapFixB {
      public:
        EuriborSwapFixB10Y(const Handle<YieldTermStructure>& h)
        : EuriborSwapFixB(10*Years, h) {}
    };

    //! 12-year %EuriborSwapFixB index
    class EuriborSwapFixB12Y : public EuriborSwapFixB {
      public:
        EuriborSwapFixB12Y(const Handle<YieldTermStructure>& h)
        : EuriborSwapFixB(12*Years, h) {}
    };

    //! 15-year %EuriborSwapFixB index
    class EuriborSwapFixB15Y : public EuriborSwapFixB {
      public:
        EuriborSwapFixB15Y(const Handle<YieldTermStructure>& h)
        : EuriborSwapFixB(15*Years, h) {}
    };

    //! 20-year %EuriborSwapFixB index
    class EuriborSwapFixB20Y : public EuriborSwapFixB {
      public:
        EuriborSwapFixB20Y(const Handle<YieldTermStructure>& h)
        : EuriborSwapFixB(20*Years, h) {}
    };

    //! 25-year %EuriborSwapFixB index
    class EuriborSwapFixB25Y : public EuriborSwapFixB {
      public:
        EuriborSwapFixB25Y(const Handle<YieldTermStructure>& h)
        : EuriborSwapFixB(25*Years, h) {}
    };

    //! 30-year %EuriborSwapFixB index
    class EuriborSwapFixB30Y : public EuriborSwapFixB {
      public:
        EuriborSwapFixB30Y(const Handle<YieldTermStructure>& h)
        : EuriborSwapFixB(30*Years, h) {}
    };

}

#endif
