/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Ferdinando Ametrano
 Copyright (C) 2006 Katiuscia Manzoni

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/reference/license.html>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/


#include <ql/Indexes/euriborswapfixa.hpp>

namespace QuantLib {

    #ifndef QL_DISABLE_DEPRECATED
    EuriborSwapFixA::EuriborSwapFixA(Integer years,
                                     const Handle<YieldTermStructure>& h)
    : SwapIndex("EuriborSwapFixA", // familyName
                years,
                2, // settlementDays
                EURCurrency(),
                TARGET(), 
                Annual, // fixedLegFrequency
                Unadjusted, // fixedLegConvention
                Thirty360(Thirty360::BondBasis), // fixedLegDaycounter 
                boost::shared_ptr<IborIndex>(new Euribor6M(h))) {}
    EuriborSwapFixA::EuriborSwapFixA(const Period& tenor,
                                     const Handle<YieldTermStructure>& h)
    : SwapIndex("EuriborSwapFixA", // familyName
                tenor,
                2, // settlementDays
                EURCurrency(),
                TARGET(), 
                Annual, // fixedLegFrequency
                Unadjusted, // fixedLegConvention
                Thirty360(Thirty360::BondBasis), // fixedLegDaycounter 
                boost::shared_ptr<IborIndex>(new Euribor6M(h))) {}
    #endif

    EuriborSwapFixAvs3M::EuriborSwapFixAvs3M(const Period& tenor,
                                         const Handle<YieldTermStructure>& h)
    : SwapIndex("EuriborSwapFixA", // familyName
                tenor,
                2, // settlementDays
                EURCurrency(),
                TARGET(), 
                1*Years, // fixedLegTenor
                Unadjusted, // fixedLegConvention
                Thirty360(Thirty360::BondBasis), // fixedLegDaycounter 
                boost::shared_ptr<IborIndex>(new Euribor3M(h))) {}

    EuriborSwapFixAvs6M::EuriborSwapFixAvs6M(const Period& tenor,
                                         const Handle<YieldTermStructure>& h)
    : SwapIndex("EuriborSwapFixA", // familyName
                tenor,
                2, // settlementDays
                EURCurrency(),
                TARGET(), 
                1*Years, // fixedLegTenor
                Unadjusted, // fixedLegConvention
                Thirty360(Thirty360::BondBasis), // fixedLegDaycounter 
                boost::shared_ptr<IborIndex>(new Euribor6M(h))) {}


}
