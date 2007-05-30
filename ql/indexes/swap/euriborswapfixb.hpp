/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
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
#include <ql/indexes/ibor/euribor.hpp>
#include <ql/time/calendars/target.hpp>
#include <ql/time/daycounters/thirty360.hpp>
#include <ql/currencies/europe.hpp>

namespace QuantLib {

    //! %EuriborSwapFixB vs 3M index base class
    /*! EuriborSwapFixB rate fixed by ISDA. The swap index is based
        on the Euribor 3M and is fixed at 12:00AM FRANKFURT.
        Reuters page ISDAFIX2 or EURSFIXA=.
    */
    class EuriborSwapFixBvs3M : public SwapIndex {
      public:
        EuriborSwapFixBvs3M(const Period& tenor,
                            const Handle<YieldTermStructure>& h =
                                    Handle<YieldTermStructure>());
    };

    //! %EuriborSwapFixB vs 6M index base class
    /*! EuriborSwapFixB rate fixed by ISDA. The swap index is based
        on the Euribor 6M and is fixed at 12:00AM FRANKFURT.
        Reuters page ISDAFIX2 or EURSFIXB=.
    */
    class EuriborSwapFixBvs6M : public SwapIndex {
      public:
        EuriborSwapFixBvs6M(const Period& tenor,
                            const Handle<YieldTermStructure>& h =
                                      Handle<YieldTermStructure>());
    };

    //! 1-year %EuriborSwapFixBvs3M index
    class EuriborSwapFixB1Y : public EuriborSwapFixBvs3M {
      public:
        EuriborSwapFixB1Y(const Handle<YieldTermStructure>& h)
        : EuriborSwapFixBvs3M(1*Years, h) {}
    };

    //! 2-year %EuriborSwapFixBvs6M index
    class EuriborSwapFixB2Y : public EuriborSwapFixBvs6M {
      public:
        EuriborSwapFixB2Y(const Handle<YieldTermStructure>& h)
        : EuriborSwapFixBvs6M(2*Years, h) {}
    };

    //! 3-year %EuriborSwapFixBvs6M index
    class EuriborSwapFixB3Y : public EuriborSwapFixBvs6M {
      public:
        EuriborSwapFixB3Y(const Handle<YieldTermStructure>& h)
        : EuriborSwapFixBvs6M(3*Years, h) {}
    };

    //! 4-year %EuriborSwapFixBvs6M index
    class EuriborSwapFixB4Y : public EuriborSwapFixBvs6M {
      public:
        EuriborSwapFixB4Y(const Handle<YieldTermStructure>& h)
        : EuriborSwapFixBvs6M(4*Years, h) {}
    };

    //! 5-year %EuriborSwapFixBvs6M index
    class EuriborSwapFixB5Y : public EuriborSwapFixBvs6M {
      public:
        EuriborSwapFixB5Y(const Handle<YieldTermStructure>& h)
        : EuriborSwapFixBvs6M(5*Years, h) {}
    };

    //! 6-year %EuriborSwapFixBvs6M index
    class EuriborSwapFixB6Y : public EuriborSwapFixBvs6M {
      public:
        EuriborSwapFixB6Y(const Handle<YieldTermStructure>& h)
        : EuriborSwapFixBvs6M(6*Years, h) {}
    };

    //! 7-year %EuriborSwapFixBvs6M index
    class EuriborSwapFixB7Y : public EuriborSwapFixBvs6M {
      public:
        EuriborSwapFixB7Y(const Handle<YieldTermStructure>& h)
        : EuriborSwapFixBvs6M(7*Years, h) {}
    };

    //! 8-year %EuriborSwapFixBvs6M index
    class EuriborSwapFixB8Y : public EuriborSwapFixBvs6M {
      public:
        EuriborSwapFixB8Y(const Handle<YieldTermStructure>& h)
        : EuriborSwapFixBvs6M(8*Years, h) {}
    };

    //! 9-year %EuriborSwapFixBvs6M index
    class EuriborSwapFixB9Y : public EuriborSwapFixBvs6M {
      public:
        EuriborSwapFixB9Y(const Handle<YieldTermStructure>& h)
        : EuriborSwapFixBvs6M(9*Years, h) {}
    };

    //! 10-year %EuriborSwapFixBvs6M index
    class EuriborSwapFixB10Y : public EuriborSwapFixBvs6M {
      public:
        EuriborSwapFixB10Y(const Handle<YieldTermStructure>& h)
        : EuriborSwapFixBvs6M(10*Years, h) {}
    };

    //! 12-year %EuriborSwapFixBvs6M index
    class EuriborSwapFixB12Y : public EuriborSwapFixBvs6M {
      public:
        EuriborSwapFixB12Y(const Handle<YieldTermStructure>& h)
        : EuriborSwapFixBvs6M(12*Years, h) {}
    };

    //! 15-year %EuriborSwapFixBvs6M index
    class EuriborSwapFixB15Y : public EuriborSwapFixBvs6M {
      public:
        EuriborSwapFixB15Y(const Handle<YieldTermStructure>& h)
        : EuriborSwapFixBvs6M(15*Years, h) {}
    };

    //! 20-year %EuriborSwapFixBvs6M index
    class EuriborSwapFixB20Y : public EuriborSwapFixBvs6M {
      public:
        EuriborSwapFixB20Y(const Handle<YieldTermStructure>& h)
        : EuriborSwapFixBvs6M(20*Years, h) {}
    };

    //! 25-year %EuriborSwapFixBvs6M index
    class EuriborSwapFixB25Y : public EuriborSwapFixBvs6M {
      public:
        EuriborSwapFixB25Y(const Handle<YieldTermStructure>& h)
        : EuriborSwapFixBvs6M(25*Years, h) {}
    };

    //! 30-year %EuriborSwapFixBvs6M index
    class EuriborSwapFixB30Y : public EuriborSwapFixBvs6M {
      public:
        EuriborSwapFixB30Y(const Handle<YieldTermStructure>& h)
        : EuriborSwapFixBvs6M(30*Years, h) {}
    };

}

#endif
