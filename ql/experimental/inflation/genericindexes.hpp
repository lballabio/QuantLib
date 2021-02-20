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

/*! \file genericindexes.hpp
    \brief Generic inflation indexes
*/

#ifndef quantlib_generic_inflation_indexes_hpp
#define quantlib_generic_inflation_indexes_hpp

#include <ql/indexes/inflationindex.hpp>

namespace QuantLib {

    //! Generic geographical/economic region
    class GenericRegion : public Region {
      public:
        GenericRegion() {
            static ext::shared_ptr<Data> GENERICdata(new Data("Generic", "GENERIC"));
            data_ = GENERICdata;
        }
    };


    //! Generic CPI index
    class GenericCPI : public ZeroInflationIndex {
      public:
        QL_TO_BE_DEPRECATED_INTERPOLATED_INFLATION_INDEXES
        GenericCPI(
            Frequency frequency,
            bool revised,
            bool interpolated,
            const Period& lag,
            const Currency& ccy,
            const Handle<ZeroInflationTermStructure>& ts = Handle<ZeroInflationTermStructure>())
        : GenericCPI(frequency, revised, lag, ccy, ts) {
            interpolated_ = interpolated;
        }

        GenericCPI(
            Frequency frequency,
            bool revised,
            const Period& lag,
            const Currency& ccy,
            const Handle<ZeroInflationTermStructure>& ts = Handle<ZeroInflationTermStructure>())
        : ZeroInflationIndex("CPI", GenericRegion(), revised, frequency, lag, ccy, ts) {}
    };


    //! Genuine year-on-year Generic CPI (i.e. not a ratio)
    class YYGenericCPI : public YoYInflationIndex {
      public:
        QL_TO_BE_DEPRECATED_INTERPOLATED_INFLATION_INDEXES
        YYGenericCPI(
            Frequency frequency,
            bool revised,
            bool interpolated,
            const Period& lag,
            const Currency& ccy,
            const Handle<YoYInflationTermStructure>& ts = Handle<YoYInflationTermStructure>())
        : YYGenericCPI(frequency, revised, lag, ccy, ts) {
            interpolated_ = interpolated;
        }

        YYGenericCPI(
            Frequency frequency,
            bool revised,
            const Period& lag,
            const Currency& ccy,
            const Handle<YoYInflationTermStructure>& ts = Handle<YoYInflationTermStructure>())
        : YoYInflationIndex("YY_CPI", GenericRegion(), revised, false, frequency, lag, ccy, ts) {}
    };

    //! Fake year-on-year GenericCPI (i.e. a ratio)
    class YYGenericCPIr : public YoYInflationIndex {
      public:
        QL_TO_BE_DEPRECATED_INTERPOLATED_INFLATION_INDEXES
        YYGenericCPIr(
            Frequency frequency,
            bool revised,
            bool interpolated,
            const Period& lag,
            const Currency& ccy,
            const Handle<YoYInflationTermStructure>& ts = Handle<YoYInflationTermStructure>())
        : YYGenericCPIr(frequency, revised, lag, ccy, ts) {
            interpolated_ = interpolated;
        }

        YYGenericCPIr(
            Frequency frequency,
            bool revised,
            const Period& lag,
            const Currency& ccy,
            const Handle<YoYInflationTermStructure>& ts = Handle<YoYInflationTermStructure>())
        : YoYInflationIndex("YYR_CPI", GenericRegion(), revised, true, frequency, lag, ccy, ts) {}
    };

}

#endif
