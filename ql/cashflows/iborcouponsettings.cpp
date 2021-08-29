/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
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

/*! \file ibor coupon settings.hpp
    \brief ibor coupon settings for the session
*/

#include <ql/cashflows/iborcouponsettings.hpp>
#include <ql/qldefines.hpp>

namespace QuantLib {

    IborCouponSettings::IborCouponSettings() :
#ifndef QL_USE_INDEXED_COUPON
        usingAtParCoupons_(true)
#else
        usingAtParCoupons_(false)
#endif
    {
    }

    IborCouponSettings::IborCouponSettings(const IborCouponSettings & other) :
        usingAtParCoupons_(other.usingAtParCoupons_)
    {
    }

    IborCouponSettings & IborCouponSettings::operator=(const IborCouponSettings & other)
    {
        usingAtParCoupons_ = other.usingAtParCoupons_;
        return *this;
    }

    void IborCouponSettings::createAtParCoupons() {
        usingAtParCoupons_ = true;
    }

    void IborCouponSettings::createIndexedCoupons() {
        usingAtParCoupons_ = false;
    }

    const bool & IborCouponSettings::usingAtParCoupons() const {
        return usingAtParCoupons_;
    }

    bool & IborCouponSettings::usingAtParCoupons() {
        return usingAtParCoupons_;
    }

}
