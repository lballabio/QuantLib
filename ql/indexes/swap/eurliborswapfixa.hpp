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

/*! \file eurliborswapfixa.hpp
    \brief %EurliborSwapFixA indexes
*/

#ifndef quantlib_eurliborswapfixa_hpp
#define quantlib_eurliborswapfixa_hpp

#include <ql/indexes/swapindex.hpp>
#include <ql/indexes/ibor/eurlibor.hpp>
#include <ql/indexes/ibor/libor.hpp>
#include <ql/time/calendars/target.hpp>
#include <ql/time/daycounters/thirty360.hpp>
#include <ql/currencies/europe.hpp>

namespace QuantLib {

    //! %EurliborSwapFixA vs 3M index base class
    /*! EurliborSwapFixA rate fixed by ISDA in cooperation with
        Reuters and Intercapital Brokers.  The swap index is based on
        the EuroLibor 3M and is fixed at 10:00 AM London.  Reuters page
        ISDAFIX2 or EURSFIXLA=.  Further info can be found at:
        <http://www.isda.org/fix/isdafix.html>.
    */
    class EurliborSwapFixAvs3M : public SwapIndex {
      public:
        EurliborSwapFixAvs3M(const Period& tenor,
                           const Handle<YieldTermStructure>& h =
                                    Handle<YieldTermStructure>());
    };

    //! %EurliborSwapFixA vs 6M index base class
    /*! EurliborSwapFixA rate fixed by ISDA in cooperation with
        Reuters and Intercapital Brokers.  The swap index is based on
        the EuroLibor 6M and is fixed at 10:00 AM London.  Reuters page
        ISDAFIX2 or EURSFIXLA=.  Further info can be found at:
        <http://www.isda.org/fix/isdafix.html>.
    */
    class EurliborSwapFixAvs6M : public SwapIndex {
      public:
        EurliborSwapFixAvs6M(const Period& tenor,
                           const Handle<YieldTermStructure>& h =
                                    Handle<YieldTermStructure>());
    };

    //! 1-year %EurliborSwapFixAvs3M index
    class EurliborSwapFixA1Y : public EurliborSwapFixAvs3M {
      public:
        EurliborSwapFixA1Y(const Handle<YieldTermStructure>& h)
        : EurliborSwapFixAvs3M(1*Years, h) {}
    };

    //! 2-year %EurliborSwapFixAvs6M index
    class EurliborSwapFixA2Y : public EurliborSwapFixAvs6M {
      public:
        EurliborSwapFixA2Y(const Handle<YieldTermStructure>& h)
        : EurliborSwapFixAvs6M(2*Years, h) {}
    };

    //! 3-year %EurliborSwapFixAvs6M index
    class EurliborSwapFixA3Y : public EurliborSwapFixAvs6M {
      public:
        EurliborSwapFixA3Y(const Handle<YieldTermStructure>& h)
        : EurliborSwapFixAvs6M(3*Years, h) {}
    };

    //! 4-year %EurliborSwapFixAvs6M index
    class EurliborSwapFixA4Y : public EurliborSwapFixAvs6M {
      public:
        EurliborSwapFixA4Y(const Handle<YieldTermStructure>& h)
        : EurliborSwapFixAvs6M(4*Years, h) {}
    };

    //! 5-year %EurliborSwapFixAvs6M index
    class EurliborSwapFixA5Y : public EurliborSwapFixAvs6M {
      public:
        EurliborSwapFixA5Y(const Handle<YieldTermStructure>& h)
        : EurliborSwapFixAvs6M(5*Years, h) {}
    };

    //! 6-year %EurliborSwapFixAvs6M index
    class EurliborSwapFixA6Y : public EurliborSwapFixAvs6M {
      public:
        EurliborSwapFixA6Y(const Handle<YieldTermStructure>& h)
        : EurliborSwapFixAvs6M(6*Years, h) {}
    };

    //! 7-year %EurliborSwapFixAvs6M index
    class EurliborSwapFixA7Y : public EurliborSwapFixAvs6M {
      public:
        EurliborSwapFixA7Y(const Handle<YieldTermStructure>& h)
        : EurliborSwapFixAvs6M(7*Years, h) {}
    };

    //! 8-year %EurliborSwapFixAvs6M index
    class EurliborSwapFixA8Y : public EurliborSwapFixAvs6M {
      public:
        EurliborSwapFixA8Y(const Handle<YieldTermStructure>& h)
        : EurliborSwapFixAvs6M(8*Years, h) {}
    };

    //! 9-year %EurliborSwapFixAvs6M index
    class EurliborSwapFixA9Y : public EurliborSwapFixAvs6M {
      public:
        EurliborSwapFixA9Y(const Handle<YieldTermStructure>& h)
        : EurliborSwapFixAvs6M(9*Years, h) {}
    };

    //! 10-year %EurliborSwapFixAvs6M index
    class EurliborSwapFixA10Y : public EurliborSwapFixAvs6M {
      public:
        EurliborSwapFixA10Y(const Handle<YieldTermStructure>& h)
        : EurliborSwapFixAvs6M(10*Years, h) {}
    };

    //! 12-year %EurliborSwapFixAvs6M index
    class EurliborSwapFixA12Y : public EurliborSwapFixAvs6M {
      public:
        EurliborSwapFixA12Y(const Handle<YieldTermStructure>& h)
        : EurliborSwapFixAvs6M(12*Years, h) {}
    };

    //! 15-year %EurliborSwapFixAvs6M index
    class EurliborSwapFixA15Y : public EurliborSwapFixAvs6M {
      public:
        EurliborSwapFixA15Y(const Handle<YieldTermStructure>& h)
        : EurliborSwapFixAvs6M(15*Years, h) {}
    };

    //! 20-year %EurliborSwapFixAvs6M index
    class EurliborSwapFixA20Y : public EurliborSwapFixAvs6M {
      public:
        EurliborSwapFixA20Y(const Handle<YieldTermStructure>& h)
        : EurliborSwapFixAvs6M(20*Years, h) {}
    };

    //! 25-year %EurliborSwapFixAvs6M index
    class EurliborSwapFixA25Y : public EurliborSwapFixAvs6M {
      public:
        EurliborSwapFixA25Y(const Handle<YieldTermStructure>& h)
        : EurliborSwapFixAvs6M(25*Years, h) {}
    };

    //! 30-year %EurliborSwapFixAvs6M index
    class EurliborSwapFixA30Y : public EurliborSwapFixAvs6M {
      public:
        EurliborSwapFixA30Y(const Handle<YieldTermStructure>& h)
        : EurliborSwapFixAvs6M(30*Years, h) {}
    };

}

#endif
