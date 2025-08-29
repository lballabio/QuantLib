/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 StatPro Italia srl

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

#include <ql/instruments/claim.hpp>

namespace QuantLib {

    Real FaceValueClaim::amount(const Date&,
                                Real notional,
                                Real recoveryRate) const {
        return notional * (1.0-recoveryRate);
    }



    FaceValueAccrualClaim::FaceValueAccrualClaim(
                          const ext::shared_ptr<Bond>& referenceSecurity)
    : referenceSecurity_(referenceSecurity) {
        registerWith(referenceSecurity);
    }

    Real FaceValueAccrualClaim::amount(const Date& d,
                                       Real notional,
                                       Real recoveryRate) const {
        Real accrual = referenceSecurity_->accruedAmount(d)
                     / referenceSecurity_->notional(d);
        return notional * (1.0-recoveryRate-accrual);
    }

}

