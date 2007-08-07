/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006, 2007 Ferdinando Ametrano
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

/*! \file eurliborswapfixifr.hpp
    \brief %EurliborSwapFixIFR indexes
*/

#ifndef quantlib_eurliborswapfixifr_hpp
#define quantlib_eurliborswapfixifr_hpp

#include <ql/indexes/swapindex.hpp>

namespace QuantLib {

    //! %EurliborSwapFixIFR index base class
    /*! EuriborSwapFix indexes published by IFR Markets and distributed
        by Reuters page TGM42281 and by Telerate. For more info see
        <http://www.ifrmarkets.com>.

        \warning The 1Y swap's floating leg is based on Eurlibor3M; the
                 floating legs of longer swaps are based on Eurlibor6M
    */
    class EurliborSwapFixIFR : public SwapIndex {
      public:
        EurliborSwapFixIFR(const Period& tenor,
                           const Handle<YieldTermStructure>& h =
                                    Handle<YieldTermStructure>());
    };

    //! 1-year %EurliborSwapFixIFR index
    class EurliborSwapFixIFR1Y : public EurliborSwapFixIFR {
      public:
        EurliborSwapFixIFR1Y(const Handle<YieldTermStructure>& h)
        : EurliborSwapFixIFR(1*Years, h) {}
    };

    //! 2-year %EurliborSwapFixIFR index
    class EurliborSwapFixIFR2Y : public EurliborSwapFixIFR {
      public:
        EurliborSwapFixIFR2Y(const Handle<YieldTermStructure>& h)
        : EurliborSwapFixIFR(2*Years, h) {}
    };

    //! 3-year %EurliborSwapFixIFR index
    class EurliborSwapFixIFR3Y : public EurliborSwapFixIFR {
      public:
        EurliborSwapFixIFR3Y(const Handle<YieldTermStructure>& h)
        : EurliborSwapFixIFR(3*Years, h) {}
    };

    //! 4-year %EurliborSwapFixIFR index
    class EurliborSwapFixIFR4Y : public EurliborSwapFixIFR {
      public:
        EurliborSwapFixIFR4Y(const Handle<YieldTermStructure>& h)
        : EurliborSwapFixIFR(4*Years, h) {}
    };

    //! 5-year %EurliborSwapFixIFR index
    class EurliborSwapFixIFR5Y : public EurliborSwapFixIFR {
      public:
        EurliborSwapFixIFR5Y(const Handle<YieldTermStructure>& h)
        : EurliborSwapFixIFR(5*Years, h) {}
    };

    //! 6-year %EurliborSwapFixIFR index
    class EurliborSwapFixIFR6Y : public EurliborSwapFixIFR {
      public:
        EurliborSwapFixIFR6Y(const Handle<YieldTermStructure>& h)
        : EurliborSwapFixIFR(6*Years, h) {}
    };

    //! 7-year %EurliborSwapFixIFR index
    class EurliborSwapFixIFR7Y : public EurliborSwapFixIFR {
      public:
        EurliborSwapFixIFR7Y(const Handle<YieldTermStructure>& h)
        : EurliborSwapFixIFR(7*Years, h) {}
    };

    //! 8-year %EurliborSwapFixIFR index
    class EurliborSwapFixIFR8Y : public EurliborSwapFixIFR {
      public:
        EurliborSwapFixIFR8Y(const Handle<YieldTermStructure>& h)
        : EurliborSwapFixIFR(8*Years, h) {}
    };

    //! 9-year %EurliborSwapFixIFR index
    class EurliborSwapFixIFR9Y : public EurliborSwapFixIFR {
      public:
        EurliborSwapFixIFR9Y(const Handle<YieldTermStructure>& h)
        : EurliborSwapFixIFR(9*Years, h) {}
    };

    //! 10-year %EurliborSwapFixIFR index
    class EurliborSwapFixIFR10Y : public EurliborSwapFixIFR {
      public:
        EurliborSwapFixIFR10Y(const Handle<YieldTermStructure>& h)
        : EurliborSwapFixIFR(10*Years, h) {}
    };

    //! 12-year %EurliborSwapFixIFR index
    class EurliborSwapFixIFR12Y : public EurliborSwapFixIFR {
      public:
        EurliborSwapFixIFR12Y(const Handle<YieldTermStructure>& h)
        : EurliborSwapFixIFR(12*Years, h) {}
    };

    //! 15-year %EurliborSwapFixIFR index
    class EurliborSwapFixIFR15Y : public EurliborSwapFixIFR {
      public:
        EurliborSwapFixIFR15Y(const Handle<YieldTermStructure>& h)
        : EurliborSwapFixIFR(15*Years, h) {}
    };

    //! 20-year %EurliborSwapFixIFR index
    class EurliborSwapFixIFR20Y : public EurliborSwapFixIFR {
      public:
        EurliborSwapFixIFR20Y(const Handle<YieldTermStructure>& h)
        : EurliborSwapFixIFR(20*Years, h) {}
    };

    //! 25-year %EurliborSwapFixIFR index
    class EurliborSwapFixIFR25Y : public EurliborSwapFixIFR {
      public:
        EurliborSwapFixIFR25Y(const Handle<YieldTermStructure>& h)
        : EurliborSwapFixIFR(25*Years, h) {}
    };

    //! 30-year %EurliborSwapFixIFR index
    class EurliborSwapFixIFR30Y : public EurliborSwapFixIFR {
      public:
        EurliborSwapFixIFR30Y(const Handle<YieldTermStructure>& h)
        : EurliborSwapFixIFR(30*Years, h) {}
    };

}

#endif
