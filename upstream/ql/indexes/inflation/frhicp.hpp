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
 <https://www.quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file frhicp.hpp
    \brief French HICP inflation indexes
*/

#ifndef quantlib_frhicp_hpp
#define quantlib_frhicp_hpp

#include <ql/currencies/europe.hpp>
#include <ql/indexes/inflationindex.hpp>

namespace QuantLib {

    //! FR HICP index
    class FRHICP : public ZeroInflationIndex {
      public:
        explicit FRHICP(const Handle<ZeroInflationTermStructure>& ts = {})
        : ZeroInflationIndex(
              "HICP", FranceRegion(), false, Monthly, Period(1, Months), EURCurrency(), ts) {}
    };


    //! Quoted year-on-year FR HICP (i.e. not a ratio)
    class YYFRHICP : public YoYInflationIndex {
      public:
        explicit YYFRHICP(const Handle<YoYInflationTermStructure>& ts = {})
        : YoYInflationIndex("YY_HICP",
                            FranceRegion(),
                            false,
                            Monthly,
                            Period(1, Months),
                            EURCurrency(),
                            ts) {}

        QL_DEPRECATED_DISABLE_WARNING

        /*! \deprecated Use the overload without the interpolated parameter.
                        Deprecated in version 1.38.
        */
        [[deprecated("Use the overload without the interpolated parameter")]]
        explicit YYFRHICP(
            bool interpolated,
            const Handle<YoYInflationTermStructure>& ts = {})
        : YoYInflationIndex("YY_HICP",
                            FranceRegion(),
                            false,
                            interpolated,
                            Monthly,
                            Period(1, Months),
                            EURCurrency(),
                            ts) {}

        QL_DEPRECATED_ENABLE_WARNING
    };

}

#endif
