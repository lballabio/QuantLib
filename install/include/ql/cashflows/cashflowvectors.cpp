/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl
 Copyright (C) 2003, 2004, 2005 StatPro Italia srl
 Copyright (C) 2006, 2007 Cristina Duminuco
 Copyright (C) 2006, 2007 Giorgio Facchinetti
 Copyright (C) 2006 Mario Pucci
 Copyright (C) 2007 Ferdinando Ametrano

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

#include <ql/cashflows/cashflowvectors.hpp>
#include <ql/cashflows/fixedratecoupon.hpp>
#include <ql/cashflows/capflooredcoupon.hpp>
#include <ql/cashflows/rangeaccrual.hpp>
#include <ql/indexes/iborindex.hpp>
#include <ql/time/schedule.hpp>

namespace QuantLib::detail {

        Rate effectiveFixedRate(const std::vector<Spread>& spreads,
                                const std::vector<Rate>& caps,
                                const std::vector<Rate>& floors,
                                Size i) {
            Rate result = get(spreads, i, 0.0);
            Rate floor = get(floors, i, Null<Rate>());
            if (floor!=Null<Rate>())
                result = std::max(floor, result);
            Rate cap = get(caps, i, Null<Rate>());
            if (cap!=Null<Rate>())
                result = std::min(cap, result);
            return result;
        }

        bool noOption(const std::vector<Rate>& caps,
                      const std::vector<Rate>& floors,
                      Size i) {
            return (get(caps,   i, Null<Rate>()) == Null<Rate>()) &&
                   (get(floors, i, Null<Rate>()) == Null<Rate>());
        }

    }
