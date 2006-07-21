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

#include <ql/Indexes/swaprate.hpp>
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
    class EuriborSwapFixA : public SwapRate {
      public:
        EuriborSwapFixA(Integer years)
        : SwapRate("EURIBORSWAPFIXA", years, 2, EURCurrency(), TARGET(), 
                   Annual, ModifiedFollowing, Thirty360(Thirty360::BondBasis), 
                   boost::shared_ptr<Xibor>(new Euribor6M())) {}
    };

    //! 1-year %EuriborSwapFixA index
    class EuriborSwapFixA1Y : public EuriborSwapFixA {
      public:
        EuriborSwapFixA1Y()
        : EuriborSwapFixA(1) {}
    };

    //! 2-year %EuriborSwapFixA index
    class EuriborSwapFixA2Y : public EuriborSwapFixA {
      public:
        EuriborSwapFixA2Y()
        : EuriborSwapFixA(2) {}
    };

    //! 3-year %EuriborSwapFixA index
    class EuriborSwapFixA3Y : public EuriborSwapFixA {
      public:
        EuriborSwapFixA3Y()
        : EuriborSwapFixA(3) {}
    };

    //! 4-year %EuriborSwapFixA index
    class EuriborSwapFixA4Y : public EuriborSwapFixA {
      public:
        EuriborSwapFixA4Y()
        : EuriborSwapFixA(4) {}
    };

    //! 5-year %EuriborSwapFixA index
    class EuriborSwapFixA5Y : public EuriborSwapFixA {
      public:
        EuriborSwapFixA5Y()
        : EuriborSwapFixA(5) {}
    };

    //! 6-year %EuriborSwapFixA index
    class EuriborSwapFixA6Y : public EuriborSwapFixA {
      public:
        EuriborSwapFixA6Y()
        : EuriborSwapFixA(6) {}
    };
    
    //! 7-year %EuriborSwapFixA index
    class EuriborSwapFixA7Y : public EuriborSwapFixA {
      public:
        EuriborSwapFixA7Y()
        : EuriborSwapFixA(7) {}
    };

    //! 8-year %EuriborSwapFixA index
    class EuriborSwapFixA8Y : public EuriborSwapFixA {
      public:
        EuriborSwapFixA8Y()
        : EuriborSwapFixA(8) {}
    };
    
    //! 9-year %EuriborSwapFixA index
    class EuriborSwapFixA9Y : public EuriborSwapFixA {
      public:
        EuriborSwapFixA9Y()
        : EuriborSwapFixA(9) {}
    };

    //! 10-year %EuriborSwapFixA index
    class EuriborSwapFixA10Y : public EuriborSwapFixA {
      public:
        EuriborSwapFixA10Y()
        : EuriborSwapFixA(10) {}
    };

    //! 12-year %EuriborSwapFixA index
    class EuriborSwapFixA12Y : public EuriborSwapFixA {
      public:
        EuriborSwapFixA12Y()
        : EuriborSwapFixA(12) {}
    };

    //! 15-year %EuriborSwapFixA index
    class EuriborSwapFixA15Y : public EuriborSwapFixA {
      public:
        EuriborSwapFixA15Y()
        : EuriborSwapFixA(15) {}
    };

    //! 20-year %EuriborSwapFixA index
    class EuriborSwapFixA20Y : public EuriborSwapFixA {
      public:
        EuriborSwapFixA20Y()
        : EuriborSwapFixA(20) {}
    };

    //! 25-year %EuriborSwapFixA index
    class EuriborSwapFixA25Y : public EuriborSwapFixA {
      public:
        EuriborSwapFixA25Y()
        : EuriborSwapFixA(25) {}
    };

    //! 30-year %EuriborSwapFixA index
    class EuriborSwapFixA30Y : public EuriborSwapFixA {
      public:
        EuriborSwapFixA30Y()
        : EuriborSwapFixA(30) {}
    };

}


#endif
