/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Katiuscia Manzoni

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

/*! \file euriborswapfixa.hpp
    \brief %euriborswapfixa index
*/

#ifndef quantlib_euriborswapfixa_hpp
#define quantlib_euriborswapfixa_hpp

#include <ql/Indexes/swapindex.hpp>
#include <ql/Indexes/euribor.hpp>
#include <ql/Calendars/target.hpp>
#include <ql/DayCounters/thirty360.hpp>
#include <ql/Currencies/europe.hpp>

namespace QuantLib {

    //! %EuriborSwapFixA index
    /*! EuriborSwapFixA rate fixed by ISDA. The swap index is based 
        on the Euribor 6M and is fixed at 11:00AM FRANKFURT.
        Reuters page ISDAFIX2 or EURSFIXA=
    */
    class EuriborSwapFixA : public SwapIndex {
      public:
        EuriborSwapFixA(Integer years,
                        const Handle<YieldTermStructure>& h =
                                    Handle<YieldTermStructure>())
        : SwapIndex("EuriborSwapFixA", // familyName
                    years,
                    2, // settlementDays
                    EURCurrency(),
                    TARGET(), 
                    Annual, // fixedLegFrequency
                    Unadjusted, // fixedLegConvention
                    Thirty360(Thirty360::BondBasis), // fixedLegDaycounter 
                    boost::shared_ptr<Xibor>(new Euribor6M(h))) {}
    };



    //! 1-year %EuriborSwapFixA index
    class EuriborSwapFixA1Y : public EuriborSwapFixA {
      public:
        EuriborSwapFixA1Y(const Handle<YieldTermStructure>& h)
        : EuriborSwapFixA(1,h) {}
    };

    //! 2-year %EuriborSwapFixA index
    class EuriborSwapFixA2Y : public EuriborSwapFixA {
      public:
        EuriborSwapFixA2Y(const Handle<YieldTermStructure>& h)
        : EuriborSwapFixA(2,h) {}
    };

    //! 3-year %EuriborSwapFixA index
    class EuriborSwapFixA3Y : public EuriborSwapFixA {
      public:
        EuriborSwapFixA3Y(const Handle<YieldTermStructure>& h)
        : EuriborSwapFixA(3,h) {}
    };

    //! 4-year %EuriborSwapFixA index
    class EuriborSwapFixA4Y : public EuriborSwapFixA {
      public:
        EuriborSwapFixA4Y(const Handle<YieldTermStructure>& h)
        : EuriborSwapFixA(4,h) {}
    };

    //! 5-year %EuriborSwapFixA index
    class EuriborSwapFixA5Y : public EuriborSwapFixA {
      public:
        EuriborSwapFixA5Y(const Handle<YieldTermStructure>& h)
        : EuriborSwapFixA(5,h) {}
    };

    //! 6-year %EuriborSwapFixA index
    class EuriborSwapFixA6Y : public EuriborSwapFixA {
      public:
        EuriborSwapFixA6Y(const Handle<YieldTermStructure>& h)
        : EuriborSwapFixA(6,h) {}
    };
    
    //! 7-year %EuriborSwapFixA index
    class EuriborSwapFixA7Y : public EuriborSwapFixA {
      public:
        EuriborSwapFixA7Y(const Handle<YieldTermStructure>& h)
        : EuriborSwapFixA(7,h) {}
    };

    //! 8-year %EuriborSwapFixA index
    class EuriborSwapFixA8Y : public EuriborSwapFixA {
      public:
        EuriborSwapFixA8Y(const Handle<YieldTermStructure>& h)
        : EuriborSwapFixA(8,h) {}
    };
    
    //! 9-year %EuriborSwapFixA index
    class EuriborSwapFixA9Y : public EuriborSwapFixA {
      public:
        EuriborSwapFixA9Y(const Handle<YieldTermStructure>& h)
        : EuriborSwapFixA(9,h) {}
    };

    //! 10-year %EuriborSwapFixA index
    class EuriborSwapFixA10Y : public EuriborSwapFixA {
      public:
        EuriborSwapFixA10Y(const Handle<YieldTermStructure>& h)
        : EuriborSwapFixA(10,h) {}
    };

    //! 12-year %EuriborSwapFixA index
    class EuriborSwapFixA12Y : public EuriborSwapFixA {
      public:
        EuriborSwapFixA12Y(const Handle<YieldTermStructure>& h)
        : EuriborSwapFixA(12,h) {}
    };

    //! 15-year %EuriborSwapFixA index
    class EuriborSwapFixA15Y : public EuriborSwapFixA {
      public:
        EuriborSwapFixA15Y(const Handle<YieldTermStructure>& h)
        : EuriborSwapFixA(15,h) {}
    };

    //! 20-year %EuriborSwapFixA index
    class EuriborSwapFixA20Y : public EuriborSwapFixA {
      public:
        EuriborSwapFixA20Y(const Handle<YieldTermStructure>& h)
        : EuriborSwapFixA(20,h) {}
    };

    //! 25-year %EuriborSwapFixA index
    class EuriborSwapFixA25Y : public EuriborSwapFixA {
      public:
        EuriborSwapFixA25Y(const Handle<YieldTermStructure>& h)
        : EuriborSwapFixA(25,h) {}
    };

    //! 30-year %EuriborSwapFixA index
    class EuriborSwapFixA30Y : public EuriborSwapFixA {
      public:
        EuriborSwapFixA30Y(const Handle<YieldTermStructure>& h)
        : EuriborSwapFixA(30,h) {}
    };
   
}


#endif
