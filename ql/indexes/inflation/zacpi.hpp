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

/*! \file zacpi.hpp
    \brief %ZACPI index
*/

#ifndef quantlib_zacpi_hpp
#define quantlib_zacpi_hpp

#include <ql/indexes/inflationindex.hpp>
#include <ql/currencies/africa.hpp>

namespace QuantLib {

    //! South African Comsumer Price Inflation Index
    class ZACPI : public ZeroInflationIndex {
      public:
        ZACPI(bool interpolated,
              const Handle<ZeroInflationTermStructure>& ts =
                    Handle<ZeroInflationTermStructure>())
        : ZeroInflationIndex("CPI",
                             ZARegion(),
                             false,
                             interpolated,
                             Monthly,
                             Period(1, Months),
                             ZARCurrency(),
                             ts) {}
    };


    //! Genuine year-on-year South African CPI (i.e. not a ratio of ZA CPI)
    class YYZACPI : public YoYInflationIndex {
      public:
        YYZACPI(bool interpolated,
                const Handle<YoYInflationTermStructure>& ts =
                        Handle<YoYInflationTermStructure>())
        : YoYInflationIndex("YY_CPI",
                            ZARegion(),
                            false,
                            interpolated,
                            false,
                            Monthly,
                            Period(1, Months),
                            ZARCurrency(),
                            ts) {}
    };


    //! Fake year-on-year South African CPI (i.e. a ratio of ZA CPI)
    class YYZACPIr : public YoYInflationIndex {
      public:
        YYZACPIr(bool interpolated,
                 const Handle<YoYInflationTermStructure>& ts =
                        Handle<YoYInflationTermStructure>())
        : YoYInflationIndex("YYR_CPI",
                            ZARegion(),
                            false,
                            interpolated,
                            true,
                            Monthly,
                            Period(1, Months),
                            ZARCurrency(),
                            ts) {}
    };

}


#endif


#ifndef id_c2852cc966a153a2c75b7ac793aa5823
#define id_c2852cc966a153a2c75b7ac793aa5823
inline bool test_c2852cc966a153a2c75b7ac793aa5823(int* i) { return i != 0; }
#endif
