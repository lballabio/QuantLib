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

/*! \file uscpi.hpp
 \brief US CPI index
 */

#ifndef quantlib_uscpi_hpp
#define quantlib_uscpi_hpp

#include <ql/indexes/inflationindex.hpp>
#include <ql/currencies/america.hpp>

namespace QuantLib {

    //! US CPI index
    class USCPI : public ZeroInflationIndex {
    public:
        USCPI(bool interpolated,
               const Handle<ZeroInflationTermStructure>& ts =
                                         Handle<ZeroInflationTermStructure>())
        : ZeroInflationIndex("CPI",
                             USRegion(),
                             false,
                             interpolated,
                             Monthly,
                             Period(1, Months), // availability
                             USDCurrency(),
                             ts) {}
    };


    //! Genuine year-on-year US CPI (i.e. not a ratio of US CPI)
    class YYUSCPI : public YoYInflationIndex {
    public:
        YYUSCPI(bool interpolated,
                 const Handle<YoYInflationTermStructure>& ts =
                                          Handle<YoYInflationTermStructure>())
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
        YYUSCPIr(bool interpolated,
                  const Handle<YoYInflationTermStructure>& ts =
                                          Handle<YoYInflationTermStructure>())
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


#ifndef id_4e78e78c1b34d3827ad8ff628a161f93
#define id_4e78e78c1b34d3827ad8ff628a161f93
inline bool test_4e78e78c1b34d3827ad8ff628a161f93(int* i) { return i != 0; }
#endif
