/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007, 2009 Chris Kenyon
 Copyright (C) 2010 StatPro Italia srl
 Copyright (C) 2007 Ralf Konrad Eckel

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

/*! \file euhicp.hpp
    \brief EU HICP index
*/

#ifndef quantlib_euhicp_hpp
#define quantlib_euhicp_hpp

#include <ql/currencies/europe.hpp>
#include <ql/indexes/inflationindex.hpp>

namespace QuantLib {

    //! EU HICP index
    class EUHICP : public ZeroInflationIndex {
      public:
        QL_DEPRECATED_INTERPOLATED_INFLATION_INDEXES
        explicit EUHICP(
            bool interpolated,
            const Handle<ZeroInflationTermStructure>& ts = Handle<ZeroInflationTermStructure>())
        : EUHICP(ts) {
            interpolated_ = interpolated;
        }

        explicit EUHICP(
            const Handle<ZeroInflationTermStructure>& ts = Handle<ZeroInflationTermStructure>())
        : ZeroInflationIndex("HICP",
                             EURegion(),
                             false,
                             Monthly,
                             Period(1, Months), // availability
                             EURCurrency(),
                             ts) {}
    };

    //! EU HICPXT index
    class EUHICPXT : public ZeroInflationIndex {
      public:
        QL_DEPRECATED_INTERPOLATED_INFLATION_INDEXES
        explicit EUHICPXT(
            bool interpolated,
            const Handle<ZeroInflationTermStructure>& ts = Handle<ZeroInflationTermStructure>())
        : EUHICPXT(ts) {
            interpolated_ = interpolated;
        }

        explicit EUHICPXT(
            const Handle<ZeroInflationTermStructure>& ts = Handle<ZeroInflationTermStructure>())
        : ZeroInflationIndex("HICPXT",
                             EURegion(),
                             false,
                             Monthly,
                             Period(1, Months), // availability
                             EURCurrency(),
                             ts) {}
    };


    //! Genuine year-on-year EU HICP (i.e. not a ratio of EU HICP)
    class YYEUHICP : public YoYInflationIndex {
      public:
        QL_DEPRECATED_INTERPOLATED_INFLATION_INDEXES
        explicit YYEUHICP(
            bool interpolated,
            const Handle<YoYInflationTermStructure>& ts = Handle<YoYInflationTermStructure>())
        : YYEUHICP(ts) {
            interpolated_ = interpolated;
        }

        explicit YYEUHICP(
            const Handle<YoYInflationTermStructure>& ts = Handle<YoYInflationTermStructure>())
        : YoYInflationIndex(
              "YY_HICP", EURegion(), false, false, Monthly, Period(1, Months), EURCurrency(), ts) {}
    };

    //! Genuine year-on-year EU HICPXT
    class YYEUHICPXT : public YoYInflationIndex {
      public:
        QL_DEPRECATED_INTERPOLATED_INFLATION_INDEXES
        explicit YYEUHICPXT(
            bool interpolated,
            const Handle<YoYInflationTermStructure>& ts = Handle<YoYInflationTermStructure>())
        : YYEUHICPXT(ts) {
            interpolated_ = interpolated;
        }

        explicit YYEUHICPXT(
            const Handle<YoYInflationTermStructure>& ts = Handle<YoYInflationTermStructure>())
        : YoYInflationIndex("YY_HICPXT",
                            EURegion(),
                            false,
                            false,
                            Monthly,
                            Period(1, Months),
                            EURCurrency(),
                            ts) {}
    };


    //! Fake year-on-year EU HICP (i.e. a ratio of EU HICP)
    class YYEUHICPr : public YoYInflationIndex {
      public:
        QL_DEPRECATED_INTERPOLATED_INFLATION_INDEXES
        explicit YYEUHICPr(
            bool interpolated,
            const Handle<YoYInflationTermStructure>& ts = Handle<YoYInflationTermStructure>())
        : YYEUHICPr(ts) {
            interpolated_ = interpolated;
        }

        explicit YYEUHICPr(
            const Handle<YoYInflationTermStructure>& ts = Handle<YoYInflationTermStructure>())
        : YoYInflationIndex(
              "YYR_HICP", EURegion(), false, true, Monthly, Period(1, Months), EURCurrency(), ts) {}
    };

}


#endif
