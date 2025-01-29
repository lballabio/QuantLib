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

    //! South African Consumer Price Inflation Index
    class ZACPI : public ZeroInflationIndex {
      public:
        explicit ZACPI(const Handle<ZeroInflationTermStructure>& ts = {})
        : ZeroInflationIndex(
              "CPI", ZARegion(), false, Monthly, Period(1, Months), ZARCurrency(), ts) {}
    };


    //! Quoted year-on-year South African CPI (i.e. not a ratio of ZA CPI)
    class YYZACPI : public YoYInflationIndex {
      public:
        explicit YYZACPI(
            const Handle<YoYInflationTermStructure>& ts = {})
        : YoYInflationIndex("YY_CPI",
                            ZARegion(),
                            false,
                            Monthly,
                            Period(1, Months),
                            ZARCurrency(),
                            ts) {}

        QL_DEPRECATED_DISABLE_WARNING

        /*! \deprecated Use the overload without the interpolated parameter.
                        Deprecated in version 1.38.
        */
        [[deprecated("Use the overload without the interpolated parameter")]]
        explicit YYZACPI(
            bool interpolated,
            const Handle<YoYInflationTermStructure>& ts = {})
        : YoYInflationIndex("YY_CPI",
                            ZARegion(),
                            false,
                            interpolated,
                            Monthly,
                            Period(1, Months),
                            ZARCurrency(),
                            ts) {}

        QL_DEPRECATED_ENABLE_WARNING
    };

}


#endif
