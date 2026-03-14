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
 <https://www.quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file ukrpi.hpp
    \brief %UKRPI index
*/

#ifndef quantlib_ukrpi_hpp
#define quantlib_ukrpi_hpp

#include <ql/currencies/europe.hpp>
#include <ql/indexes/inflationindex.hpp>

namespace QuantLib {

    //! UK Retail Price Inflation Index
    class UKRPI : public ZeroInflationIndex {
      public:
        explicit UKRPI(const Handle<ZeroInflationTermStructure>& ts = {})
        : ZeroInflationIndex(
              "RPI", UKRegion(), false, Monthly, Period(1, Months), GBPCurrency(), ts) {}
    };


    //! Quoted year-on-year UK RPI (i.e. not a ratio of UK RPI)
    class YYUKRPI : public YoYInflationIndex {
      public:
        explicit YYUKRPI(const Handle<YoYInflationTermStructure>& ts = {})
        : YoYInflationIndex("YY_RPI",
                            UKRegion(),
                            false,
                            Monthly,
                            Period(1, Months),
                            GBPCurrency(),
                            ts) {}

        QL_DEPRECATED_DISABLE_WARNING

        /*! \deprecated Use the overload without the interpolated parameter.
                        Deprecated in version 1.38.
        */
        [[deprecated("Use the overload without the interpolated parameter")]]
        explicit YYUKRPI(
            bool interpolated,
            const Handle<YoYInflationTermStructure>& ts = {})
        : YoYInflationIndex("YY_RPI",
                            UKRegion(),
                            false,
                            interpolated,
                            Monthly,
                            Period(1, Months),
                            GBPCurrency(),
                            ts) {}

        QL_DEPRECATED_ENABLE_WARNING
    };

}


#endif
