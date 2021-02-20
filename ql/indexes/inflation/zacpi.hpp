/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007, 2009 Chris Kenyon
 Copyright (C) 2021 Ralf Konrad Eckel

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

#include <ql/currencies/africa.hpp>
#include <ql/indexes/inflationindex.hpp>

namespace QuantLib {

    //! South African Comsumer Price Inflation Index
    class ZACPI : public ZeroInflationIndex {
      public:
        QL_TO_BE_DEPRECATED_INTERPOLATED_INFLATION_INDEXES
        explicit ZACPI(
            bool interpolated,
            const Handle<ZeroInflationTermStructure>& ts = Handle<ZeroInflationTermStructure>())
        : ZACPI(ts) {
            interpolated_ = interpolated;
        }

        explicit ZACPI(
            const Handle<ZeroInflationTermStructure>& ts = Handle<ZeroInflationTermStructure>())
        : ZeroInflationIndex(
              "CPI", ZARegion(), false, Monthly, Period(1, Months), ZARCurrency(), ts) {}
    };


    //! Genuine year-on-year South African CPI (i.e. not a ratio of ZA CPI)
    class YYZACPI : public YoYInflationIndex {
      public:
        QL_TO_BE_DEPRECATED_INTERPOLATED_YOY_INDEXES
        explicit YYZACPI(
            bool interpolated,
            const Handle<YoYInflationTermStructure>& ts = Handle<YoYInflationTermStructure>())
        : YYZACPI(ts) {
            interpolated_ = interpolated;
        }

        explicit YYZACPI(
            const Handle<YoYInflationTermStructure>& ts = Handle<YoYInflationTermStructure>())
        : YoYInflationIndex(
              "YY_CPI", ZARegion(), false, false, Monthly, Period(1, Months), ZARCurrency(), ts) {}
    };


    //! Fake year-on-year South African CPI (i.e. a ratio of ZA CPI)
    class YYZACPIr : public YoYInflationIndex {
      public:
        QL_TO_BE_DEPRECATED_INTERPOLATED_YOY_INDEXES
        explicit YYZACPIr(
            bool interpolated,
            const Handle<YoYInflationTermStructure>& ts = Handle<YoYInflationTermStructure>())
        : YYZACPIr(ts) {
            interpolated_ = interpolated;
        }

        explicit YYZACPIr(
            const Handle<YoYInflationTermStructure>& ts = Handle<YoYInflationTermStructure>())
        : YoYInflationIndex(
              "YYR_CPI", ZARegion(), false, true, Monthly, Period(1, Months), ZARCurrency(), ts) {}
    };

}


#endif
