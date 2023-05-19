/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2009, 2011 Chris Kenyon

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

/*! \file cpicouponpricer.hpp
    \brief zero inflation-coupon pricer
 */

#ifndef quantlib_cpicouponpricer_hpp
#define quantlib_cpicouponpricer_hpp

#include <ql/cashflow.hpp>
#include <ql/option.hpp>
#include <ql/cashflows/inflationcouponpricer.hpp>
#include <ql/cashflows/cpicoupon.hpp>
#include <ql/termstructures/volatility/inflation/cpivolatilitystructure.hpp>

namespace QuantLib {

    //! base pricer for capped/floored CPI coupons N.B. vol-dependent parts are a TODO
    /*! \note this pricer can already do swaplets but to get
              volatility-dependent coupons you need to implement the descendents.
    */
    class CPICouponPricer : public InflationCouponPricer {
      public:
        explicit CPICouponPricer(Handle<YieldTermStructure> nominalTermStructure = Handle<YieldTermStructure>());

        explicit CPICouponPricer(Handle<CPIVolatilitySurface> capletVol,
                                 Handle<YieldTermStructure> nominalTermStructure = Handle<YieldTermStructure>());

        QL_DEPRECATED_DISABLE_WARNING
        ~CPICouponPricer() override = default;
        QL_DEPRECATED_ENABLE_WARNING

        virtual Handle<CPIVolatilitySurface> capletVolatility() const{
            return capletVol_;
        }

        virtual Handle<YieldTermStructure> nominalTermStructure() const{
            return nominalTermStructure_;
        }

        virtual void setCapletVolatility(
            const Handle<CPIVolatilitySurface>& capletVol);


        //! \name InflationCouponPricer interface
        //@{
        Real swapletPrice() const override;
        Rate swapletRate() const override;
        Real capletPrice(Rate effectiveCap) const override;
        Rate capletRate(Rate effectiveCap) const override;
        Real floorletPrice(Rate effectiveFloor) const override;
        Rate floorletRate(Rate effectiveFloor) const override;
        void initialize(const InflationCoupon&) override;
        //@}

        virtual Rate accruedRate(Date settlementDate) const;

      protected:
        virtual Real optionletPrice(Option::Type optionType,
                                    Real effStrike) const;

        virtual Real optionletRate(Option::Type optionType,
                                   Real effStrike) const;

        /*! Derived classes usually only need to implement this.

            The name of the method is misleading.  This actually
            returns the rate of the optionlet (so not discounted and
            not accrued).
        */
        virtual Real optionletPriceImp(Option::Type, Real strike,
                                       Real forward, Real stdDev) const;

        /*! \deprecated Don't use this method.  In derived classes, override accruedRate.
                        Deprecated in version 1.31.
        */
        [[deprecated("Do not use this method. In derived classes, override accruedRate.")]]
        virtual Rate adjustedFixing(Rate fixing = Null<Rate>()) const;

        // data
        Handle<CPIVolatilitySurface> capletVol_;
        Handle<YieldTermStructure> nominalTermStructure_;
        const CPICoupon* coupon_;
        Real gearing_;
        /*! \deprecated Don't use this data member. A spread doesn't make sense for this coupon.
                        Deprecated in version 1.31.
        */
        [[deprecated("Do not use this data member. A spread doesn't make sense for these coupons.")]]
        Spread spread_;
        Real discount_;
    };
    
}

#endif
