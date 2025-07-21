/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006, 2007, 2008, 2011 Ferdinando Ametrano
 Copyright (C) 2006 Chiara Fornarola

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

#include <ql/indexes/swap/chfliborswap.hpp>
#include <ql/indexes/ibor/chflibor.hpp>
#include <ql/time/calendars/target.hpp>
#include <ql/time/daycounters/thirty360.hpp>
#include <ql/currencies/europe.hpp>

namespace QuantLib {

    ChfLiborSwapIsdaFix::ChfLiborSwapIsdaFix(
                                const Period& tenor,
                                const Handle<YieldTermStructure>& h)
    : SwapIndex("ChfLiborSwapIsdaFix", // familyName
                tenor,
                2, // settlementDays
                CHFCurrency(),
                TARGET(),
                1*Years, // fixedLegTenor
                ModifiedFollowing, // fixedLegConvention
                Thirty360(Thirty360::BondBasis), // fixedLegDaycounter
                tenor > 1*Years ?
                    ext::shared_ptr<IborIndex>(new CHFLibor(6*Months, h)) :
                    ext::shared_ptr<IborIndex>(new CHFLibor(3*Months, h))) {}

    ChfLiborSwapIsdaFix::ChfLiborSwapIsdaFix(
                                const Period& tenor,
                                const Handle<YieldTermStructure>& forwarding,
                                const Handle<YieldTermStructure>& discounting)
    : SwapIndex("ChfLiborSwapIsdaFix", // familyName
                tenor,
                2, // settlementDays
                CHFCurrency(),
                TARGET(),
                1*Years, // fixedLegTenor
                ModifiedFollowing, // fixedLegConvention
                Thirty360(Thirty360::BondBasis), // fixedLegDaycounter
                tenor > 1*Years ?
                    ext::shared_ptr<IborIndex>(new CHFLibor(6*Months, forwarding)) :
                    ext::shared_ptr<IborIndex>(new CHFLibor(3*Months, forwarding)),
                discounting) {}

}
