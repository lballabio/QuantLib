/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007, 2009 Chris Kenyon
 Copyright (C) 2010 StatPro Italia srl
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
        explicit EUHICP(const Handle<ZeroInflationTermStructure>& ts = {})
        : ZeroInflationIndex("HICP",
                             EURegion(),
                             false,
                             Monthly,
                             Period(1, Months), // availability
                             EURCurrency(),
                             ts) {}

        /*! \deprecated Use the constructor without the "interpolated" parameter.
                        Deprecated in version 1.29.
        */
        QL_DEPRECATED
        explicit EUHICP(
            bool interpolated,
            const Handle<ZeroInflationTermStructure>& ts = {})
        : EUHICP(ts) {
            QL_DEPRECATED_DISABLE_WARNING
            interpolated_ = interpolated;
            QL_DEPRECATED_ENABLE_WARNING
        }
    };

    //! EU HICPXT index
    class EUHICPXT : public ZeroInflationIndex {
      public:
        explicit EUHICPXT(const Handle<ZeroInflationTermStructure>& ts = {})
        : ZeroInflationIndex("HICPXT",
                             EURegion(),
                             false,
                             Monthly,
                             Period(1, Months), // availability
                             EURCurrency(),
                             ts) {}

        /*! \deprecated Use the constructor without the "interpolated" parameter.
                        Deprecated in version 1.29.
        */
        QL_DEPRECATED
        explicit EUHICPXT(
            bool interpolated,
            const Handle<ZeroInflationTermStructure>& ts = {})
        : EUHICPXT(ts) {
            QL_DEPRECATED_DISABLE_WARNING
            interpolated_ = interpolated;
            QL_DEPRECATED_ENABLE_WARNING
        }
    };


    //! Genuine year-on-year EU HICP (i.e. not a ratio of EU HICP)
    class YYEUHICP : public YoYInflationIndex {
      public:
        explicit YYEUHICP(
            bool interpolated,
            const Handle<YoYInflationTermStructure>& ts = {})
        : YoYInflationIndex("YY_HICP",
                            EURegion(),
                            false,
                            interpolated,
                            false,
                            Monthly,
                            Period(1, Months),
                            EURCurrency(),
                            ts) {}
    };

    //! Genuine year-on-year EU HICPXT
    class YYEUHICPXT : public YoYInflationIndex {
      public:
        explicit YYEUHICPXT(
            bool interpolated,
            const Handle<YoYInflationTermStructure>& ts = {})
        : YoYInflationIndex("YY_HICPXT",
                            EURegion(),
                            false,
                            interpolated,
                            false,
                            Monthly,
                            Period(1, Months),
                            EURCurrency(),
                            ts) {}
    };


    //! Fake year-on-year EU HICP (i.e. a ratio of EU HICP)
    class YYEUHICPr : public YoYInflationIndex {
      public:
        explicit YYEUHICPr(
            bool interpolated,
            const Handle<YoYInflationTermStructure>& ts = {})
        : YoYInflationIndex("YYR_HICP",
                            EURegion(),
                            false,
                            interpolated,
                            true,
                            Monthly,
                            Period(1, Months),
                            EURCurrency(),
                            ts) {}
    };

}


#endif
