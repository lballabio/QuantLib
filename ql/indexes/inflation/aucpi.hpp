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

/*! \file aucpi.hpp
    \brief Australian CPI inflation indexes
*/

#ifndef quantlib_aucpi_hpp
#define quantlib_aucpi_hpp

#include <ql/indexes/inflationindex.hpp>
#include <ql/currencies/oceania.hpp>

namespace QuantLib {

    //! AU CPI index (either quarterly or annual)
    class AUCPI : public ZeroInflationIndex {
      public:
        AUCPI(Frequency frequency,
              bool revised,
              bool interpolated,
              const Handle<ZeroInflationTermStructure>& ts =
                                         Handle<ZeroInflationTermStructure>())
        : ZeroInflationIndex("CPI",
                             AustraliaRegion(),
                             revised,
                             interpolated,
                             frequency,
                             Period(2, Months),
                             AUDCurrency(),
                             ts) {}
    };


    //! Genuine year-on-year AU CPI (i.e. not a ratio)
    class YYAUCPI : public YoYInflationIndex {
      public:
        YYAUCPI(Frequency frequency,
                bool revised,
                bool interpolated,
                const Handle<YoYInflationTermStructure>& ts =
                                          Handle<YoYInflationTermStructure>())
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
                 const Handle<YoYInflationTermStructure>& ts =
                                          Handle<YoYInflationTermStructure>())
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



#ifndef id_59d636161ecdf5e26fd0da6eddea8eba
#define id_59d636161ecdf5e26fd0da6eddea8eba
inline bool test_59d636161ecdf5e26fd0da6eddea8eba(int* i) { return i != 0; }
#endif
