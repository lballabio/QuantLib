/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Ferdinando Ametrano
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
#include <ql/indexes/ibor/euribor.hpp>
#include <ql/time/calendars/target.hpp>
#include <ql/time/daycounters/thirty360.hpp>
#include <ql/currencies/europe.hpp>

namespace QuantLib {

    //! %EuriborSwapFixA vs 3M index base class
    /*! EuriborSwapFixA rate fixed by ISDA. The swap index is based
        on the Euribor 3M and is fixed at 11:00AM FRANKFURT.
        Reuters page ISDAFIX2 or EURSFIXA=.
    */
    class EuriborSwapFixAvs3M : public SwapIndex {
      public:
        EuriborSwapFixAvs3M(const Period& tenor,
                            const Handle<YieldTermStructure>& h =
                                    Handle<YieldTermStructure>());
    };

    //! %EuriborSwapFixA vs 6M index base class
    /*! EuriborSwapFixA rate fixed by ISDA. The swap index is based
        on the Euribor 6M and is fixed at 11:00AM FRANKFURT.
        Reuters page ISDAFIX2 or EURSFIXA=.
    */
    class EuriborSwapFixAvs6M : public SwapIndex {
      public:
        EuriborSwapFixAvs6M(const Period& tenor,
                            const Handle<YieldTermStructure>& h =
                                    Handle<YieldTermStructure>());
    };

    //! 1-year %EuriborSwapFixAvs3M index
    class EuriborSwapFixA1Y : public EuriborSwapFixAvs3M {
      public:
        EuriborSwapFixA1Y(const Handle<YieldTermStructure>& h)
        : EuriborSwapFixAvs3M(1*Years, h) {}
    };

    //! 2-year %EuriborSwapFixAvs6M index
    class EuriborSwapFixA2Y : public EuriborSwapFixAvs6M {
      public:
        EuriborSwapFixA2Y(const Handle<YieldTermStructure>& h)
        : EuriborSwapFixAvs6M(2*Years, h) {}
    };

    //! 3-year %EuriborSwapFixAvs6M index
    class EuriborSwapFixA3Y : public EuriborSwapFixAvs6M {
      public:
        EuriborSwapFixA3Y(const Handle<YieldTermStructure>& h)
        : EuriborSwapFixAvs6M(3*Years, h) {}
    };

    //! 4-year %EuriborSwapFixAvs6M index
    class EuriborSwapFixA4Y : public EuriborSwapFixAvs6M {
      public:
        EuriborSwapFixA4Y(const Handle<YieldTermStructure>& h)
        : EuriborSwapFixAvs6M(4*Years, h) {}
    };

    //! 5-year %EuriborSwapFixAvs6M index
    class EuriborSwapFixA5Y : public EuriborSwapFixAvs6M {
      public:
        EuriborSwapFixA5Y(const Handle<YieldTermStructure>& h)
        : EuriborSwapFixAvs6M(5*Years, h) {}
    };

    //! 6-year %EuriborSwapFixAvs6M index
    class EuriborSwapFixA6Y : public EuriborSwapFixAvs6M {
      public:
        EuriborSwapFixA6Y(const Handle<YieldTermStructure>& h)
        : EuriborSwapFixAvs6M(6*Years, h) {}
    };

    //! 7-year %EuriborSwapFixAvs6M index
    class EuriborSwapFixA7Y : public EuriborSwapFixAvs6M {
      public:
        EuriborSwapFixA7Y(const Handle<YieldTermStructure>& h)
        : EuriborSwapFixAvs6M(7*Years, h) {}
    };

    //! 8-year %EuriborSwapFixAvs6M index
    class EuriborSwapFixA8Y : public EuriborSwapFixAvs6M {
      public:
        EuriborSwapFixA8Y(const Handle<YieldTermStructure>& h)
        : EuriborSwapFixAvs6M(8*Years, h) {}
    };

    //! 9-year %EuriborSwapFixAvs6M index
    class EuriborSwapFixA9Y : public EuriborSwapFixAvs6M {
      public:
        EuriborSwapFixA9Y(const Handle<YieldTermStructure>& h)
        : EuriborSwapFixAvs6M(9*Years, h) {}
    };

    //! 10-year %EuriborSwapFixAvs6M index
    class EuriborSwapFixA10Y : public EuriborSwapFixAvs6M {
      public:
        EuriborSwapFixA10Y(const Handle<YieldTermStructure>& h)
        : EuriborSwapFixAvs6M(10*Years, h) {}
    };

    //! 12-year %EuriborSwapFixAvs6M index
    class EuriborSwapFixA12Y : public EuriborSwapFixAvs6M {
      public:
        EuriborSwapFixA12Y(const Handle<YieldTermStructure>& h)
        : EuriborSwapFixAvs6M(12*Years, h) {}
    };

    //! 15-year %EuriborSwapFixAvs6M index
    class EuriborSwapFixA15Y : public EuriborSwapFixAvs6M {
      public:
        EuriborSwapFixA15Y(const Handle<YieldTermStructure>& h)
        : EuriborSwapFixAvs6M(15*Years, h) {}
    };

    //! 20-year %EuriborSwapFixAvs6M index
    class EuriborSwapFixA20Y : public EuriborSwapFixAvs6M {
      public:
        EuriborSwapFixA20Y(const Handle<YieldTermStructure>& h)
        : EuriborSwapFixAvs6M(20*Years, h) {}
    };

    //! 25-year %EuriborSwapFixAvs6M index
    class EuriborSwapFixA25Y : public EuriborSwapFixAvs6M {
      public:
        EuriborSwapFixA25Y(const Handle<YieldTermStructure>& h)
        : EuriborSwapFixAvs6M(25*Years, h) {}
    };

    //! 30-year %EuriborSwapFixAvs6M index
    class EuriborSwapFixA30Y : public EuriborSwapFixAvs6M {
      public:
        EuriborSwapFixA30Y(const Handle<YieldTermStructure>& h)
        : EuriborSwapFixAvs6M(30*Years, h) {}
    };

}

#endif
