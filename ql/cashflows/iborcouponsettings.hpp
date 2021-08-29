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

#ifndef quantlib_iborcouponsettings_hpp
#define quantlib_iborcouponsettings_hpp

namespace QuantLib {

    //! Ibor coupon settings for the session
    class IborCouponSettings {
    public:
        IborCouponSettings();
        IborCouponSettings(const IborCouponSettings & other);
        IborCouponSettings & operator=(const IborCouponSettings & other);

        /*! When called, IborCoupons are created as indexed coupons instead of par coupons.
         */
        void createAtParCoupons();

        /*! When called, IborCoupons are created as par coupons instead of indexed coupons.
         */
        void createIndexedCoupons();

        /*! If true the IborCoupons are created as par coupons and vice versa.
         *  The default depends on the compiler flag QL_USE_INDEXED_COUPON and can be overwritten by
         *  createAtParCoupons() and createIndexedCoupons()
        */
        const bool & usingAtParCoupons() const;
        bool & usingAtParCoupons();

    private:
        bool usingAtParCoupons_;
    };

}

#endif
