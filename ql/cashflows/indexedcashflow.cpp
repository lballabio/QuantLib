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

#include <ql/cashflows/indexedcashflow.hpp>
#include <ql/index.hpp>

namespace QuantLib {

    Real IndexedCashFlow::amount() const {
        Real I0 = index_->fixing(baseDate_);
        Real I1 = index_->fixing(fixingDate_);

        if (growthOnly_)
            return notional_ * (I1 / I0 - 1.0);
        else
            return notional_ * (I1 / I0);
    }

}
