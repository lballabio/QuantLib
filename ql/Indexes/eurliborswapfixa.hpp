/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
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
    \brief %eurliborswapfixa index
*/

#ifndef quantlib_eurliborswapfixa_hpp
#define quantlib_eurliborswapfixa_hpp

#include <ql/Indexes/swapindex.hpp>
#include <ql/Indexes/eurlibor.hpp>
#include <ql/Indexes/libor.hpp>
#include <ql/Calendars/target.hpp>
#include <ql/DayCounters/thirty360.hpp>
#include <ql/Currencies/europe.hpp>

namespace QuantLib {

    //! %EurliborSwapFixA index
    /*! EurliborSwapFixA rate fixed by ISDA in cooperation with Reuters and Intercapital Brokers.
        The swap index is based on the EuroLibor 6M and is fixed at 10:00AM London.
        Reuters page ISDAFIX2 or EURSFIXLA=
        Further info can be found at: http://www.isda.org/fix/isdafix.html
    */
    class EurliborSwapFixA : public SwapIndex {
      public:
        EurliborSwapFixA(Integer years,
                        const Handle<YieldTermStructure>& h =
                                    Handle<YieldTermStructure>())
        : SwapIndex("EurliborSwapFixA", // familyName
                    years,
                    2, // settlementDays
                    EURCurrency(),
                    TARGET(), 
                    Annual, // fixedLegFrequency
                    Unadjusted, // fixedLegConvention
                    Thirty360(Thirty360::BondBasis), // fixedLegDaycounter 
                    boost::shared_ptr<Xibor>(new EURLibor6M(h))) {}
    };



    //! 1-year %EurliborSwapFixA index
    class EurliborSwapFixA1Y : public EurliborSwapFixA {
      public:
        EurliborSwapFixA1Y(const Handle<YieldTermStructure>& h)
        : EurliborSwapFixA(1,h) {}
    };

    //! 2-year %EurliborSwapFixA index
    class EurliborSwapFixA2Y : public EurliborSwapFixA {
      public:
        EurliborSwapFixA2Y(const Handle<YieldTermStructure>& h)
        : EurliborSwapFixA(2,h) {}
    };

    //! 3-year %EurliborSwapFixA index
    class EurliborSwapFixA3Y : public EurliborSwapFixA {
      public:
        EurliborSwapFixA3Y(const Handle<YieldTermStructure>& h)
        : EurliborSwapFixA(3,h) {}
    };

    //! 4-year %EurliborSwapFixA index
    class EurliborSwapFixA4Y : public EurliborSwapFixA {
      public:
        EurliborSwapFixA4Y(const Handle<YieldTermStructure>& h)
        : EurliborSwapFixA(4,h) {}
    };

    //! 5-year %EurliborSwapFixA index
    class EurliborSwapFixA5Y : public EurliborSwapFixA {
      public:
        EurliborSwapFixA5Y(const Handle<YieldTermStructure>& h)
        : EurliborSwapFixA(5,h) {}
    };

    //! 6-year %EurliborSwapFixA index
    class EurliborSwapFixA6Y : public EurliborSwapFixA {
      public:
        EurliborSwapFixA6Y(const Handle<YieldTermStructure>& h)
        : EurliborSwapFixA(6,h) {}
    };
    
    //! 7-year %EurliborSwapFixA index
    class EurliborSwapFixA7Y : public EurliborSwapFixA {
      public:
        EurliborSwapFixA7Y(const Handle<YieldTermStructure>& h)
        : EurliborSwapFixA(7,h) {}
    };

    //! 8-year %EurliborSwapFixA index
    class EurliborSwapFixA8Y : public EurliborSwapFixA {
      public:
        EurliborSwapFixA8Y(const Handle<YieldTermStructure>& h)
        : EurliborSwapFixA(8,h) {}
    };
    
    //! 9-year %EurliborSwapFixA index
    class EurliborSwapFixA9Y : public EurliborSwapFixA {
      public:
        EurliborSwapFixA9Y(const Handle<YieldTermStructure>& h)
        : EurliborSwapFixA(9,h) {}
    };

    //! 10-year %EurliborSwapFixA index
    class EurliborSwapFixA10Y : public EurliborSwapFixA {
      public:
        EurliborSwapFixA10Y(const Handle<YieldTermStructure>& h)
        : EurliborSwapFixA(10,h) {}
    };

    //! 12-year %EurliborSwapFixA index
    class EurliborSwapFixA12Y : public EurliborSwapFixA {
      public:
        EurliborSwapFixA12Y(const Handle<YieldTermStructure>& h)
        : EurliborSwapFixA(12,h) {}
    };

    //! 15-year %EurliborSwapFixA index
    class EurliborSwapFixA15Y : public EurliborSwapFixA {
      public:
        EurliborSwapFixA15Y(const Handle<YieldTermStructure>& h)
        : EurliborSwapFixA(15,h) {}
    };

    //! 20-year %EurliborSwapFixA index
    class EurliborSwapFixA20Y : public EurliborSwapFixA {
      public:
        EurliborSwapFixA20Y(const Handle<YieldTermStructure>& h)
        : EurliborSwapFixA(20,h) {}
    };

    //! 25-year %EurliborSwapFixA index
    class EurliborSwapFixA25Y : public EurliborSwapFixA {
      public:
        EurliborSwapFixA25Y(const Handle<YieldTermStructure>& h)
        : EurliborSwapFixA(25,h) {}
    };

    //! 30-year %EurliborSwapFixA index
    class EurliborSwapFixA30Y : public EurliborSwapFixA {
      public:
        EurliborSwapFixA30Y(const Handle<YieldTermStructure>& h)
        : EurliborSwapFixA(30,h) {}
    };
   
}


#endif
