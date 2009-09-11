/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2009 Chris Kenyon

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

/*! \file frhicp.hpp
    \brief French HICP inflation indexes
*/

#ifndef quantlib_frhicp_hpp
#define quantlib_frhicp_hpp

#include <ql/indexes/inflationindex.hpp>
#include <ql/currencies/europe.hpp>

namespace QuantLib {

    //! FR HICP index
    class FRHICP : public ZeroInflationIndex {
      public:
        FRHICP(Frequency frequency,
               bool revised,
               bool interpolated,
               const Handle<ZeroInflationTermStructure>& ts =
                                         Handle<ZeroInflationTermStructure>())
        : ZeroInflationIndex("HICP",
                             FranceRegion(),
                             revised,
                             interpolated,
                             frequency,
                             Period(3, Months),
                             EURCurrency(),
                             ts) {}
    };


    //! Genuine year-on-year FR HICP (i.e. not a ratio)
    class YYFRHICP : public YoYInflationIndex {
      public:
        YYFRHICP(Frequency frequency,
                 bool revised,
                 bool interpolated,
                 const Handle<YoYInflationTermStructure>& ts =
                                          Handle<YoYInflationTermStructure>())
        : YoYInflationIndex("YY_HICP",
                            FranceRegion(),
                            revised,
                            interpolated,
                            false,
                            frequency,
                            Period(3, Months),
                            EURCurrency(),
                            ts) {}
    };


    //! Fake year-on-year FR HICP (i.e. a ratio)
    class YYFRHICPr : public YoYInflationIndex {
      public:
        YYFRHICPr(Frequency frequency,
                  bool revised,
                  bool interpolated,
                  const Handle<YoYInflationTermStructure>& ts =
                                          Handle<YoYInflationTermStructure>())
        : YoYInflationIndex("YYR_HICP",
                            FranceRegion(),
                            revised,
                            interpolated,
                            true,
                            frequency,
                            Period(3, Months),
                            EURCurrency(),
                            ts) {}
    };

}

#endif

