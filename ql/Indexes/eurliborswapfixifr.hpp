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

/*! \file eurliborswapfixifr.hpp
    \brief %eurliborswapfixifr index
*/

#ifndef quantlib_eurliborswapfixifr_hpp
#define quantlib_eurliborswapfixifr_hpp

#include <ql/Indexes/swapindex.hpp>
#include <ql/Indexes/eurlibor.hpp>
#include <ql/Indexes/libor.hpp>
#include <ql/Calendars/target.hpp>
#include <ql/DayCounters/thirty360.hpp>
#include <ql/Currencies/europe.hpp>

namespace QuantLib {

    //! %EurliborSwapFixIFR index
    /*! EuriborSwapFix index published by IFR Markets and distributed by Reuters page TGM42281 and
  //       by Telerate. For more info see http://www.ifrmarkets.com
    */
    class EurliborSwapFixIFR : public SwapIndex {
      public:
        EurliborSwapFixIFR(Integer years,
                        const Handle<YieldTermStructure>& h =
                                    Handle<YieldTermStructure>())
        : SwapIndex("EurliborSwapFixIFR", // familyName
                    years,
                    2, // settlementDays
                    EURCurrency(),
                    TARGET(), 
                    Annual, // fixedLegFrequency
                    Unadjusted, // fixedLegConvention
                    Thirty360(Thirty360::BondBasis), // fixedLegDaycounter 
                    boost::shared_ptr<Xibor>(new EURLibor6M(h))) {}
    };



    //! 1-year %EurliborSwapFixIFR index
    class EurliborSwapFixIFR1Y : public EurliborSwapFixIFR {
      public:
        EurliborSwapFixIFR1Y(const Handle<YieldTermStructure>& h)
        : EurliborSwapFixIFR(1,h) {}
    };

    //! 2-year %EurliborSwapFixIFR index
    class EurliborSwapFixIFR2Y : public EurliborSwapFixIFR {
      public:
        EurliborSwapFixIFR2Y(const Handle<YieldTermStructure>& h)
        : EurliborSwapFixIFR(2,h) {}
    };

    //! 3-year %EurliborSwapFixIFR index
    class EurliborSwapFixIFR3Y : public EurliborSwapFixIFR {
      public:
        EurliborSwapFixIFR3Y(const Handle<YieldTermStructure>& h)
        : EurliborSwapFixIFR(3,h) {}
    };

    //! 4-year %EurliborSwapFixIFR index
    class EurliborSwapFixIFR4Y : public EurliborSwapFixIFR {
      public:
        EurliborSwapFixIFR4Y(const Handle<YieldTermStructure>& h)
        : EurliborSwapFixIFR(4,h) {}
    };

    //! 5-year %EurliborSwapFixIFR index
    class EurliborSwapFixIFR5Y : public EurliborSwapFixIFR {
      public:
        EurliborSwapFixIFR5Y(const Handle<YieldTermStructure>& h)
        : EurliborSwapFixIFR(5,h) {}
    };

    //! 6-year %EurliborSwapFixIFR index
    class EurliborSwapFixIFR6Y : public EurliborSwapFixIFR {
      public:
        EurliborSwapFixIFR6Y(const Handle<YieldTermStructure>& h)
        : EurliborSwapFixIFR(6,h) {}
    };
    
    //! 7-year %EurliborSwapFixIFR index
    class EurliborSwapFixIFR7Y : public EurliborSwapFixIFR {
      public:
        EurliborSwapFixIFR7Y(const Handle<YieldTermStructure>& h)
        : EurliborSwapFixIFR(7,h) {}
    };

    //! 8-year %EurliborSwapFixIFR index
    class EurliborSwapFixIFR8Y : public EurliborSwapFixIFR {
      public:
        EurliborSwapFixIFR8Y(const Handle<YieldTermStructure>& h)
        : EurliborSwapFixIFR(8,h) {}
    };
    
    //! 9-year %EurliborSwapFixIFR index
    class EurliborSwapFixIFR9Y : public EurliborSwapFixIFR {
      public:
        EurliborSwapFixIFR9Y(const Handle<YieldTermStructure>& h)
        : EurliborSwapFixIFR(9,h) {}
    };

    //! 10-year %EurliborSwapFixIFR index
    class EurliborSwapFixIFR10Y : public EurliborSwapFixIFR {
      public:
        EurliborSwapFixIFR10Y(const Handle<YieldTermStructure>& h)
        : EurliborSwapFixIFR(10,h) {}
    };

    //! 12-year %EurliborSwapFixIFR index
    class EurliborSwapFixIFR12Y : public EurliborSwapFixIFR {
      public:
        EurliborSwapFixIFR12Y(const Handle<YieldTermStructure>& h)
        : EurliborSwapFixIFR(12,h) {}
    };

    //! 15-year %EurliborSwapFixIFR index
    class EurliborSwapFixIFR15Y : public EurliborSwapFixIFR {
      public:
        EurliborSwapFixIFR15Y(const Handle<YieldTermStructure>& h)
        : EurliborSwapFixIFR(15,h) {}
    };

    //! 20-year %EurliborSwapFixIFR index
    class EurliborSwapFixIFR20Y : public EurliborSwapFixIFR {
      public:
        EurliborSwapFixIFR20Y(const Handle<YieldTermStructure>& h)
        : EurliborSwapFixIFR(20,h) {}
    };

    //! 25-year %EurliborSwapFixIFR index
    class EurliborSwapFixIFR25Y : public EurliborSwapFixIFR {
      public:
        EurliborSwapFixIFR25Y(const Handle<YieldTermStructure>& h)
        : EurliborSwapFixIFR(25,h) {}
    };

    //! 30-year %EurliborSwapFixIFR index
    class EurliborSwapFixIFR30Y : public EurliborSwapFixIFR {
      public:
        EurliborSwapFixIFR30Y(const Handle<YieldTermStructure>& h)
        : EurliborSwapFixIFR(30,h) {}
    };
   
}


#endif
