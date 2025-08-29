/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008, 2011 Ferdinando Ametrano

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <https://www.quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/indexes/swap/usdliborswap.hpp>
#include <ql/indexes/ibor/usdlibor.hpp>
#include <ql/time/calendars/target.hpp>
#include <ql/time/daycounters/thirty360.hpp>
#include <ql/currencies/america.hpp>

namespace QuantLib {

    UsdLiborSwapIsdaFixAm::UsdLiborSwapIsdaFixAm(
                                const Period& tenor,
                                const Handle<YieldTermStructure>& h)
    : SwapIndex("UsdLiborSwapIsdaFixAm", // familyName
                tenor,
                2, // settlementDays
                USDCurrency(),
                UnitedStates(UnitedStates::GovernmentBond),
                6*Months, // fixedLegTenor
                ModifiedFollowing, // fixedLegConvention
                Thirty360(Thirty360::BondBasis), // fixedLegDaycounter
                ext::shared_ptr<IborIndex>(new USDLibor(3*Months, h))) {}

    UsdLiborSwapIsdaFixAm::UsdLiborSwapIsdaFixAm(
                                const Period& tenor,
                                const Handle<YieldTermStructure>& forwarding,
                                const Handle<YieldTermStructure>& discounting)
    : SwapIndex("UsdLiborSwapIsdaFixAm", // familyName
                tenor,
                2, // settlementDays
                USDCurrency(),
                UnitedStates(UnitedStates::GovernmentBond),
                6*Months, // fixedLegTenor
                ModifiedFollowing, // fixedLegConvention
                Thirty360(Thirty360::BondBasis), // fixedLegDaycounter
                ext::shared_ptr<IborIndex>(new USDLibor(3*Months, forwarding)),
                discounting) {}

    UsdLiborSwapIsdaFixPm::UsdLiborSwapIsdaFixPm(
                                const Period& tenor,
                                const Handle<YieldTermStructure>& h)
    : SwapIndex("UsdLiborSwapIsdaFixPm", // familyName
                tenor,
                2, // settlementDays
                USDCurrency(),
                UnitedStates(UnitedStates::GovernmentBond),
                6*Months, // fixedLegTenor
                ModifiedFollowing, // fixedLegConvention
                Thirty360(Thirty360::BondBasis), // fixedLegDaycounter
                ext::shared_ptr<IborIndex>(new USDLibor(3*Months, h))) {}

    UsdLiborSwapIsdaFixPm::UsdLiborSwapIsdaFixPm(
                                const Period& tenor,
                                const Handle<YieldTermStructure>& forwarding,
                                const Handle<YieldTermStructure>& discounting)
    : SwapIndex("UsdLiborSwapIsdaFixPm", // familyName
                tenor,
                2, // settlementDays
                USDCurrency(),
                UnitedStates(UnitedStates::GovernmentBond),
                6*Months, // fixedLegTenor
                ModifiedFollowing, // fixedLegConvention
                Thirty360(Thirty360::BondBasis), // fixedLegDaycounter
                ext::shared_ptr<IborIndex>(new USDLibor(3*Months, forwarding)),
                discounting) {}

}
