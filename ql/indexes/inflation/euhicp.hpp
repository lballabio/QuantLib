/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Chris Kenyon

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

#include <ql/indexes/inflationindex.hpp>
#include <ql/currencies/europe.hpp>

namespace QuantLib {

    //! EU HICP index
    class EUHICP : public ZeroInflationIndex {
      public:
        EUHICP(Frequency frequency,
               bool revised,
               bool interpolated,
               const Handle<ZeroInflationTermStructure>& ts =
                                        Handle<ZeroInflationTermStructure>())
        : ZeroInflationIndex("HICP",
                             EURegion(),
                             revised,
                             interpolated,
                             frequency,
                             Period(3, Months),
                             EURCurrency(),
                             ts) {}
    };


    //! Genuine year-on-year EU HICP (i.e. not a ratio of EU HICP)
    class YYEUHICP : public YoYInflationIndex {
      public:
        YYEUHICP(Frequency frequency,
                 bool revised,
                 bool interpolated,
                 const Handle<YoYInflationTermStructure>& ts =
                                         Handle<YoYInflationTermStructure>())
        : YoYInflationIndex("YY_HICP",
                            EURegion(),
                            revised,
                            interpolated,
                            false,
                            frequency,
                            Period(3, Months),
                            EURCurrency(),
                            ts) {}
    };


    //! Fake year-on-year EU HICP (i.e. a ratio of EU HICP)
    class YYEUHICPr : public YoYInflationIndex {
      public:
        YYEUHICPr(Frequency frequency,
                  bool revised,
                  bool interpolated,
                  const Handle<YoYInflationTermStructure>& ts =
                                         Handle<YoYInflationTermStructure>())
        : YoYInflationIndex("YYR_HICP",
                            EURegion(),
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
