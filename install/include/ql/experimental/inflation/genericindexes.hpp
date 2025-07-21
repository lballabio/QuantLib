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
            static ext::shared_ptr<Data> GENERICdata(
                                               new Data("Generic","GENERIC"));
            data_ = GENERICdata;
        }
    };


    //! Generic CPI index
    class GenericCPI : public ZeroInflationIndex {
      public:
        GenericCPI(
            Frequency frequency,
            bool revised,
            const Period& lag,
            const Currency& ccy,
            const Handle<ZeroInflationTermStructure>& ts = {})
        : ZeroInflationIndex("CPI", GenericRegion(), revised, frequency, lag, ccy, ts) {}
    };


    //! Quoted year-on-year Generic CPI (i.e. not a ratio)
    class YYGenericCPI : public YoYInflationIndex {
      public:
        YYGenericCPI(Frequency frequency,
                     bool revised,
                     const Period &lag,
                     const Currency &ccy,
                     const Handle<YoYInflationTermStructure>& ts = {})
        : YoYInflationIndex("YY_CPI",
                            GenericRegion(),
                            revised,
                            frequency,
                            lag,
                            ccy,
                            ts) {}

        QL_DEPRECATED_DISABLE_WARNING

        /*! \deprecated Use the overload without the interpolated parameter.
                        Deprecated in version 1.38.
        */
        [[deprecated("Use the overload without the interpolated parameter")]]
        YYGenericCPI(Frequency frequency,
                     bool revised,
                     bool interpolated,
                     const Period &lag,
                     const Currency &ccy,
                     const Handle<YoYInflationTermStructure>& ts = {})
        : YoYInflationIndex("YY_CPI",
                            GenericRegion(),
                            revised,
                            interpolated,
                            frequency,
                            lag,
                            ccy,
                            ts) {}

        QL_DEPRECATED_ENABLE_WARNING
    };

}

#endif

