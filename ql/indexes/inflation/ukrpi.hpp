/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007, 2009 Chris Kenyon

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

/*! \file ukrpi.hpp
    \brief %UKRPI index
*/

#ifndef quantlib_ukrpi_hpp
#define quantlib_ukrpi_hpp

#include <ql/indexes/inflationindex.hpp>
#include <ql/currencies/europe.hpp>

namespace QuantLib {

    //! UK Retail Price Inflation Index
    class UKRPI : public ZeroInflationIndex {
      public:
        UKRPI(bool interpolated,
              const Handle<ZeroInflationTermStructure>& ts =
                    Handle<ZeroInflationTermStructure>())
        : ZeroInflationIndex("RPI",
                             UKRegion(),
                             false,
                             interpolated,
                             Monthly,
                             Period(1, Months),
                             GBPCurrency(),
                             ts) {}
    };


    //! Genuine year-on-year UK RPI (i.e. not a ratio of UK RPI)
    class YYUKRPI : public YoYInflationIndex {
      public:
        YYUKRPI(bool interpolated,
                const Handle<YoYInflationTermStructure>& ts =
                        Handle<YoYInflationTermStructure>())
        : YoYInflationIndex("YY_RPI",
                            UKRegion(),
                            false,
                            interpolated,
                            false,
                            Monthly,
                            Period(1, Months),
                            GBPCurrency(),
                            ts) {}
    };


    //! Fake year-on-year UK RPI (i.e. a ratio of UK RPI)
    class YYUKRPIr : public YoYInflationIndex {
      public:
        YYUKRPIr(bool interpolated,
                 const Handle<YoYInflationTermStructure>& ts =
                        Handle<YoYInflationTermStructure>())
        : YoYInflationIndex("YYR_RPI",
                            UKRegion(),
                            false,
                            interpolated,
                            true,
                            Monthly,
                            Period(1, Months),
                            GBPCurrency(),
                            ts) {}
    };

}


#endif


#ifndef id_291a6704153b4a3fe5bcdeccdfb7b947
#define id_291a6704153b4a3fe5bcdeccdfb7b947
inline bool test_291a6704153b4a3fe5bcdeccdfb7b947(const int* i) {
    return i != nullptr;
}
#endif
