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

/*! \file aucpi.hpp
    \brief Australian CPI inflation indexes
*/

#ifndef quantlib_aucpi_hpp
#define quantlib_aucpi_hpp

#include <ql/currencies/oceania.hpp>
#include <ql/indexes/inflationindex.hpp>

namespace QuantLib {

    //! AU CPI index (either quarterly or annual)
    class AUCPI : public ZeroInflationIndex {
      public:
        AUCPI(Frequency frequency,
              bool revised,
              const Handle<ZeroInflationTermStructure>& ts = {})
        : ZeroInflationIndex(
              "CPI", AustraliaRegion(), revised, frequency, Period(2, Months), AUDCurrency(), ts) {}

        /*! \deprecated Use the constructor without the "interpolated" parameter.
                        Deprecated in version 1.29.
        */
        QL_DEPRECATED
        AUCPI(Frequency frequency,
              bool revised,
              bool interpolated,
              const Handle<ZeroInflationTermStructure>& ts = {})
        : AUCPI(frequency, revised, ts) {
            QL_DEPRECATED_DISABLE_WARNING_III_INTERPOLATED_MEMBER
            interpolated_ = interpolated;
            QL_DEPRECATED_ENABLE_WARNING_III_INTERPOLATED_MEMBER
        }
    };


    //! Genuine year-on-year AU CPI (i.e. not a ratio)
    class YYAUCPI : public YoYInflationIndex {
      public:
        YYAUCPI(Frequency frequency,
                bool revised,
                bool interpolated,
                const Handle<YoYInflationTermStructure>& ts = {})
        : YoYInflationIndex("YY_CPI",
                            AustraliaRegion(),
                            revised,
                            interpolated,
                            false,
                            frequency,
                            Period(2, Months),
                            AUDCurrency(),
                            ts) {}
    };


    //! Fake year-on-year AUCPI (i.e. a ratio)
    class YYAUCPIr : public YoYInflationIndex {
      public:
        YYAUCPIr(Frequency frequency,
                 bool revised,
                 bool interpolated,
                 const Handle<YoYInflationTermStructure>& ts = {})
        : YoYInflationIndex("YYR_CPI",
                            AustraliaRegion(),
                            revised,
                            interpolated,
                            true,
                            frequency,
                            Period(2, Months),
                            AUDCurrency(),
                            ts) {}
    };
}

#endif
