/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2009 Chris Kenyon
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

/*! \file uscpi.hpp
 \brief US CPI index
 */

#ifndef quantlib_uscpi_hpp
#define quantlib_uscpi_hpp

#include <ql/currencies/america.hpp>
#include <ql/indexes/inflationindex.hpp>

namespace QuantLib {

    //! US CPI index
    class USCPI : public ZeroInflationIndex {
      public:
        explicit USCPI(const Handle<ZeroInflationTermStructure>& ts = {})
        : ZeroInflationIndex("CPI",
                             USRegion(),
                             false,
                             Monthly,
                             Period(1, Months), // availability
                             USDCurrency(),
                             ts) {}

        /*! \deprecated Use the constructor without the "interpolated" parameter.
                        Deprecated in version 1.29.
        */
        QL_DEPRECATED
        explicit USCPI(
            bool interpolated,
            const Handle<ZeroInflationTermStructure>& ts = {})
        : USCPI(ts) {
            QL_DEPRECATED_DISABLE_WARNING
            interpolated_ = interpolated;
            QL_DEPRECATED_ENABLE_WARNING
        }
    };


    //! Genuine year-on-year US CPI (i.e. not a ratio of US CPI)
    class YYUSCPI : public YoYInflationIndex {
      public:
        explicit YYUSCPI(
            bool interpolated,
            const Handle<YoYInflationTermStructure>& ts = {})
        : YoYInflationIndex("YY_CPI",
                            USRegion(),
                            false,
                            interpolated,
                            false,
                            Monthly,
                            Period(1, Months),
                            USDCurrency(),
                            ts) {}
    };


    //! Fake year-on-year US CPI (i.e. a ratio of US CPI)
    class YYUSCPIr : public YoYInflationIndex {
      public:
        explicit YYUSCPIr(
            bool interpolated,
            const Handle<YoYInflationTermStructure>& ts = {})
        : YoYInflationIndex("YYR_CPI",
                            USRegion(),
                            false,
                            interpolated,
                            true,
                            Monthly,
                            Period(1, Months),
                            USDCurrency(),
                            ts) {}
    };

}


#endif
