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

/*! \file eurliborswapfixb.hpp
    \brief %EurliborSwapFixB indexes
*/

#ifndef quantlib_eurliborswapfixb_hpp
#define quantlib_eurliborswapfixb_hpp

#include <ql/Indexes/swapindex.hpp>
#include <ql/Indexes/eurlibor.hpp>
#include <ql/Indexes/libor.hpp>
#include <ql/Calendars/target.hpp>
#include <ql/DayCounters/thirty360.hpp>
#include <ql/Currencies/europe.hpp>

namespace QuantLib {

    //! %EurliborSwapFixB vs 3M index base class
    /*! EurliborSwapFixB rate fixed by ISDA in cooperation with
        Reuters and Intercapital Brokers.  The swap index is based on
        the EuroLibor 3M and is fixed at 11:00AM London.  Reuters page
        ISDAFIX2 or EURSFIXLB= Further info can be found at:
        <http://www.isda.org/fix/isdafix.html>.
    */
    class EurliborSwapFixBvs3M : public SwapIndex {
      public:
        EurliborSwapFixBvs3M(const Period& tenor,
                             const Handle<YieldTermStructure>& h =
                                    Handle<YieldTermStructure>());
    };

    //! %EurliborSwapFixB vs 6M index base class
    /*! EurliborSwapFixB rate fixed by ISDA in cooperation with
        Reuters and Intercapital Brokers.  The swap index is based on
        the EuroLibor 6M and is fixed at 11:00AM London.  Reuters page
        ISDAFIX2 or EURSFIXLB= Further info can be found at:
        <http://www.isda.org/fix/isdafix.html>.
    */
    class EurliborSwapFixBvs6M : public SwapIndex {
      public:
        EurliborSwapFixBvs6M(const Period& tenor,
                             const Handle<YieldTermStructure>& h =
                                    Handle<YieldTermStructure>());
    };

    //! 1-year %EurliborSwapFixBvs3M index
    class EurliborSwapFixB1Y : public EurliborSwapFixBvs3M {
      public:
        EurliborSwapFixB1Y(const Handle<YieldTermStructure>& h)
        : EurliborSwapFixBvs3M(1*Years, h) {}
    };

    //! 2-year %EurliborSwapFixBvs6M index
    class EurliborSwapFixB2Y : public EurliborSwapFixBvs6M {
      public:
        EurliborSwapFixB2Y(const Handle<YieldTermStructure>& h)
        : EurliborSwapFixBvs6M(2*Years, h) {}
    };

    //! 3-year %EurliborSwapFixBvs6M index
    class EurliborSwapFixB3Y : public EurliborSwapFixBvs6M {
      public:
        EurliborSwapFixB3Y(const Handle<YieldTermStructure>& h)
        : EurliborSwapFixBvs6M(3*Years, h) {}
    };

    //! 4-year %EurliborSwapFixBvs6M index
    class EurliborSwapFixB4Y : public EurliborSwapFixBvs6M {
      public:
        EurliborSwapFixB4Y(const Handle<YieldTermStructure>& h)
        : EurliborSwapFixBvs6M(4*Years, h) {}
    };

    //! 5-year %EurliborSwapFixBvs6M index
    class EurliborSwapFixB5Y : public EurliborSwapFixBvs6M {
      public:
        EurliborSwapFixB5Y(const Handle<YieldTermStructure>& h)
        : EurliborSwapFixBvs6M(5*Years, h) {}
    };

    //! 6-year %EurliborSwapFixBvs6M index
    class EurliborSwapFixB6Y : public EurliborSwapFixBvs6M {
      public:
        EurliborSwapFixB6Y(const Handle<YieldTermStructure>& h)
        : EurliborSwapFixBvs6M(6*Years, h) {}
    };

    //! 7-year %EurliborSwapFixBvs6M index
    class EurliborSwapFixB7Y : public EurliborSwapFixBvs6M {
      public:
        EurliborSwapFixB7Y(const Handle<YieldTermStructure>& h)
        : EurliborSwapFixBvs6M(7*Years, h) {}
    };

    //! 8-year %EurliborSwapFixBvs6M index
    class EurliborSwapFixB8Y : public EurliborSwapFixBvs6M {
      public:
        EurliborSwapFixB8Y(const Handle<YieldTermStructure>& h)
        : EurliborSwapFixBvs6M(8*Years, h) {}
    };

    //! 9-year %EurliborSwapFixBvs6M index
    class EurliborSwapFixB9Y : public EurliborSwapFixBvs6M {
      public:
        EurliborSwapFixB9Y(const Handle<YieldTermStructure>& h)
        : EurliborSwapFixBvs6M(9*Years, h) {}
    };

    //! 10-year %EurliborSwapFixBvs6M index
    class EurliborSwapFixB10Y : public EurliborSwapFixBvs6M {
      public:
        EurliborSwapFixB10Y(const Handle<YieldTermStructure>& h)
        : EurliborSwapFixBvs6M(10*Years, h) {}
    };

    //! 12-year %EurliborSwapFixBvs6M index
    class EurliborSwapFixB12Y : public EurliborSwapFixBvs6M {
      public:
        EurliborSwapFixB12Y(const Handle<YieldTermStructure>& h)
        : EurliborSwapFixBvs6M(12*Years, h) {}
    };

    //! 15-year %EurliborSwapFixBvs6M index
    class EurliborSwapFixB15Y : public EurliborSwapFixBvs6M {
      public:
        EurliborSwapFixB15Y(const Handle<YieldTermStructure>& h)
        : EurliborSwapFixBvs6M(15*Years, h) {}
    };

    //! 20-year %EurliborSwapFixBvs6M index
    class EurliborSwapFixB20Y : public EurliborSwapFixBvs6M {
      public:
        EurliborSwapFixB20Y(const Handle<YieldTermStructure>& h)
        : EurliborSwapFixBvs6M(20*Years, h) {}
    };

    //! 25-year %EurliborSwapFixBvs6M index
    class EurliborSwapFixB25Y : public EurliborSwapFixBvs6M {
      public:
        EurliborSwapFixB25Y(const Handle<YieldTermStructure>& h)
        : EurliborSwapFixBvs6M(25*Years, h) {}
    };

    //! 30-year %EurliborSwapFixBvs6M index
    class EurliborSwapFixB30Y : public EurliborSwapFixBvs6M {
      public:
        EurliborSwapFixB30Y(const Handle<YieldTermStructure>& h)
        : EurliborSwapFixBvs6M(30*Years, h) {}
    };

}

#endif
