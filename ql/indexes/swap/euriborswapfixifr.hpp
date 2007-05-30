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

/*! \file euriborswapfixifr.hpp
    \brief %EuriborSwapFixIFR indexes
*/

#ifndef quantlib_euriborswapfixifr_hpp
#define quantlib_euriborswapfixifr_hpp

#include <ql/indexes/swapindex.hpp>
#include <ql/indexes/ibor/euribor.hpp>
#include <ql/time/calendars/target.hpp>
#include <ql/time/daycounters/thirty360.hpp>
#include <ql/currencies/europe.hpp>

namespace QuantLib {

    //! %EuriborSwapFixIFR vs 3M index base class
    /*! EuriborSwapFixIFR index published by IFR Markets and
        distributed by Reuters page TGM42281 and by Telerate.
        For more info see <http://www.ifrmarkets.com>.
    */
    class EuriborSwapFixIFRvs3M : public SwapIndex {
      public:
        EuriborSwapFixIFRvs3M(const Period& tenor,
                              const Handle<YieldTermStructure>& h =
                                    Handle<YieldTermStructure>());
    };

    //! %EuriborSwapFixIFR vs 6M index base class
    /*! EuriborSwapFixIFR index published by IFR Markets and
        distributed by Reuters page TGM42281 and by Telerate.
        For more info see <http://www.ifrmarkets.com>.
    */
    class EuriborSwapFixIFRvs6M : public SwapIndex {
      public:
        EuriborSwapFixIFRvs6M(const Period& tenor,
                              const Handle<YieldTermStructure>& h =
                                    Handle<YieldTermStructure>());
    };

    //! 1-year %EuriborSwapFixIFR3M index
    class EuriborSwapFixIFR1Y : public EuriborSwapFixIFRvs3M {
      public:
        EuriborSwapFixIFR1Y(const Handle<YieldTermStructure>& h)
        : EuriborSwapFixIFRvs3M(1*Years, h) {}
    };

    //! 2-year %EuriborSwapFixIFRvs6M index
    class EuriborSwapFixIFR2Y : public EuriborSwapFixIFRvs6M {
      public:
        EuriborSwapFixIFR2Y(const Handle<YieldTermStructure>& h)
        : EuriborSwapFixIFRvs6M(2*Years, h) {}
    };

    //! 3-year %EuriborSwapFixIFRvs6M index
    class EuriborSwapFixIFR3Y : public EuriborSwapFixIFRvs6M {
      public:
        EuriborSwapFixIFR3Y(const Handle<YieldTermStructure>& h)
        : EuriborSwapFixIFRvs6M(3*Years, h) {}
    };

    //! 4-year %EuriborSwapFixIFRvs6M index
    class EuriborSwapFixIFR4Y : public EuriborSwapFixIFRvs6M {
      public:
        EuriborSwapFixIFR4Y(const Handle<YieldTermStructure>& h)
        : EuriborSwapFixIFRvs6M(4*Years, h) {}
    };

    //! 5-year %EuriborSwapFixIFRvs6M index
    class EuriborSwapFixIFR5Y : public EuriborSwapFixIFRvs6M {
      public:
        EuriborSwapFixIFR5Y(const Handle<YieldTermStructure>& h)
        : EuriborSwapFixIFRvs6M(5*Years, h) {}
    };

    //! 6-year %EuriborSwapFixIFRvs6M index
    class EuriborSwapFixIFR6Y : public EuriborSwapFixIFRvs6M {
      public:
        EuriborSwapFixIFR6Y(const Handle<YieldTermStructure>& h)
        : EuriborSwapFixIFRvs6M(6*Years, h) {}
    };

    //! 7-year %EuriborSwapFixIFRvs6M index
    class EuriborSwapFixIFR7Y : public EuriborSwapFixIFRvs6M {
      public:
        EuriborSwapFixIFR7Y(const Handle<YieldTermStructure>& h)
        : EuriborSwapFixIFRvs6M(7*Years, h) {}
    };

    //! 8-year %EuriborSwapFixIFRvs6M index
    class EuriborSwapFixIFR8Y : public EuriborSwapFixIFRvs6M {
      public:
        EuriborSwapFixIFR8Y(const Handle<YieldTermStructure>& h)
        : EuriborSwapFixIFRvs6M(8*Years, h) {}
    };

    //! 9-year %EuriborSwapFixIFRvs6M index
    class EuriborSwapFixIFR9Y : public EuriborSwapFixIFRvs6M {
      public:
        EuriborSwapFixIFR9Y(const Handle<YieldTermStructure>& h)
        : EuriborSwapFixIFRvs6M(9*Years, h) {}
    };

    //! 10-year %EuriborSwapFixIFRvs6M index
    class EuriborSwapFixIFR10Y : public EuriborSwapFixIFRvs6M {
      public:
        EuriborSwapFixIFR10Y(const Handle<YieldTermStructure>& h)
        : EuriborSwapFixIFRvs6M(10*Years, h) {}
    };

    //! 12-year %EuriborSwapFixIFRvs6M index
    class EuriborSwapFixIFR12Y : public EuriborSwapFixIFRvs6M {
      public:
        EuriborSwapFixIFR12Y(const Handle<YieldTermStructure>& h)
        : EuriborSwapFixIFRvs6M(12*Years, h) {}
    };

    //! 15-year %EuriborSwapFixIFRvs6M index
    class EuriborSwapFixIFR15Y : public EuriborSwapFixIFRvs6M {
      public:
        EuriborSwapFixIFR15Y(const Handle<YieldTermStructure>& h)
        : EuriborSwapFixIFRvs6M(15*Years, h) {}
    };

    //! 20-year %EuriborSwapFixIFRvs6M index
    class EuriborSwapFixIFR20Y : public EuriborSwapFixIFRvs6M {
      public:
        EuriborSwapFixIFR20Y(const Handle<YieldTermStructure>& h)
        : EuriborSwapFixIFRvs6M(20*Years, h) {}
    };

    //! 25-year %EuriborSwapFixIFRvs6M index
    class EuriborSwapFixIFR25Y : public EuriborSwapFixIFRvs6M {
      public:
        EuriborSwapFixIFR25Y(const Handle<YieldTermStructure>& h)
        : EuriborSwapFixIFRvs6M(25*Years, h) {}
    };

    //! 30-year %EuriborSwapFixIFRvs6M index
    class EuriborSwapFixIFR30Y : public EuriborSwapFixIFRvs6M {
      public:
        EuriborSwapFixIFR30Y(const Handle<YieldTermStructure>& h)
        : EuriborSwapFixIFRvs6M(30*Years, h) {}
    };

}

#endif
