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
 <http://quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file eurliborswapfixb.hpp
    \brief %EurliborSwapFixB indexes
*/

#ifndef quantlib_eurliborswapfixb_hpp
#define quantlib_eurliborswapfixb_hpp

#include <ql/indexes/swapindex.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>

namespace QuantLib {

    //! %EurliborSwapFixB index base class
    /*! EurliborSwapFixB indexes fixed by ISDA in cooperation with
        Reuters and Intercapital Brokers at 11:00AM London.
        Reuters page ISDAFIX2 or EURSFIXLB=.
        Further info can be found at: <http://www.isda.org/fix/isdafix.html>.

        \warning The 1Y swap's floating leg is based on Eurlibor3M; the
                 floating legs of longer swaps are based on Eurlibor6M
    */
    class EurliborSwapFixB : public SwapIndex {
      public:
        EurliborSwapFixB(const Period& tenor,
                         const Handle<YieldTermStructure>& h =
                                    Handle<YieldTermStructure>());
    };

    //! 1-year %EurliborSwapFixB index
    class EurliborSwapFixB1Y : public EurliborSwapFixB {
      public:
        EurliborSwapFixB1Y(const Handle<YieldTermStructure>& h)
        : EurliborSwapFixB(1*Years, h) {}
    };

    //! 2-year %EurliborSwapFixB index
    class EurliborSwapFixB2Y : public EurliborSwapFixB {
      public:
        EurliborSwapFixB2Y(const Handle<YieldTermStructure>& h)
        : EurliborSwapFixB(2*Years, h) {}
    };

    //! 3-year %EurliborSwapFixB index
    class EurliborSwapFixB3Y : public EurliborSwapFixB {
      public:
        EurliborSwapFixB3Y(const Handle<YieldTermStructure>& h)
        : EurliborSwapFixB(3*Years, h) {}
    };

    //! 4-year %EurliborSwapFixB index
    class EurliborSwapFixB4Y : public EurliborSwapFixB {
      public:
        EurliborSwapFixB4Y(const Handle<YieldTermStructure>& h)
        : EurliborSwapFixB(4*Years, h) {}
    };

    //! 5-year %EurliborSwapFixB index
    class EurliborSwapFixB5Y : public EurliborSwapFixB {
      public:
        EurliborSwapFixB5Y(const Handle<YieldTermStructure>& h)
        : EurliborSwapFixB(5*Years, h) {}
    };

    //! 6-year %EurliborSwapFixB index
    class EurliborSwapFixB6Y : public EurliborSwapFixB {
      public:
        EurliborSwapFixB6Y(const Handle<YieldTermStructure>& h)
        : EurliborSwapFixB(6*Years, h) {}
    };

    //! 7-year %EurliborSwapFixB index
    class EurliborSwapFixB7Y : public EurliborSwapFixB {
      public:
        EurliborSwapFixB7Y(const Handle<YieldTermStructure>& h)
        : EurliborSwapFixB(7*Years, h) {}
    };

    //! 8-year %EurliborSwapFixB index
    class EurliborSwapFixB8Y : public EurliborSwapFixB {
      public:
        EurliborSwapFixB8Y(const Handle<YieldTermStructure>& h)
        : EurliborSwapFixB(8*Years, h) {}
    };

    //! 9-year %EurliborSwapFixB index
    class EurliborSwapFixB9Y : public EurliborSwapFixB {
      public:
        EurliborSwapFixB9Y(const Handle<YieldTermStructure>& h)
        : EurliborSwapFixB(9*Years, h) {}
    };

    //! 10-year %EurliborSwapFixB index
    class EurliborSwapFixB10Y : public EurliborSwapFixB {
      public:
        EurliborSwapFixB10Y(const Handle<YieldTermStructure>& h)
        : EurliborSwapFixB(10*Years, h) {}
    };

    //! 12-year %EurliborSwapFixB index
    class EurliborSwapFixB12Y : public EurliborSwapFixB {
      public:
        EurliborSwapFixB12Y(const Handle<YieldTermStructure>& h)
        : EurliborSwapFixB(12*Years, h) {}
    };

    //! 15-year %EurliborSwapFixB index
    class EurliborSwapFixB15Y : public EurliborSwapFixB {
      public:
        EurliborSwapFixB15Y(const Handle<YieldTermStructure>& h)
        : EurliborSwapFixB(15*Years, h) {}
    };

    //! 20-year %EurliborSwapFixB index
    class EurliborSwapFixB20Y : public EurliborSwapFixB {
      public:
        EurliborSwapFixB20Y(const Handle<YieldTermStructure>& h)
        : EurliborSwapFixB(20*Years, h) {}
    };

    //! 25-year %EurliborSwapFixB index
    class EurliborSwapFixB25Y : public EurliborSwapFixB {
      public:
        EurliborSwapFixB25Y(const Handle<YieldTermStructure>& h)
        : EurliborSwapFixB(25*Years, h) {}
    };

    //! 30-year %EurliborSwapFixB index
    class EurliborSwapFixB30Y : public EurliborSwapFixB {
      public:
        EurliborSwapFixB30Y(const Handle<YieldTermStructure>& h)
        : EurliborSwapFixB(30*Years, h) {}
    };

}

#endif
