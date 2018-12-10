/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
  Copyright (C) 2014, 2015, 2018 Peter Caspers

  This file is part of QuantLib, a free-software/open-source library
  for financial quantitative analysts and developers - http://quantlib.org/

  QuantLib is free software: you can redistribute it and/or modify it
  under the terms of the QuantLib license.  You should have received a
  copy of the license along with this program; if not, please email
  <quantlib-dev@lists.sf.net>. The license is also available online at
  <http://quantlib.org/license.shtml>.


  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
  or FITNESS FOR A PARTICULAR PURPOSE. See the license for more details. */

/*! \file lognormalcmsspreadpricer.hpp
    \brief cms spread coupon pricer as in Brigo, Mercurio, 13.6.2, with
           extensions for shifted lognormal and normal dynamics as
           described in http://ssrn.com/abstract=2686998
*/

#ifndef quantlib_lognormal_cmsspread_pricer_hpp
#define quantlib_lognormal_cmsspread_pricer_hpp

#include <ql/cashflows/cmscoupon.hpp>
#include <ql/experimental/coupons/cmsspreadcoupon.hpp>
#include <ql/experimental/coupons/swapspreadindex.hpp>
#include <ql/math/integrals/gaussianquadratures.hpp>
#include <ql/math/distributions/normaldistribution.hpp>

namespace QuantLib {

    class CmsSpreadCoupon;
    class YieldTermStructure;

    //! CMS spread - coupon pricer
    /*! The swap rate adjustments are computed using the given
        volatility structures for the underlyings in every case
        (w.r.t. volatility type and shift).

        For the bivariate spread model, the volatility type and
        the shifts can be inherited (default), or explicitly
        specified. In the latter case the type, and (if lognormal)
        the shifts must be given (or are defaulted to zero, if not
        given).

        References:

        Brigo, Mercurio: Interst Rate Models - Theory and Practice,
        2nd Edition, Springer, 2006, chapter 13.6.2

        http://ssrn.com/abstract=2686998
    */

    class LognormalCmsSpreadPricer : public CmsSpreadCouponPricer {

      public:
        LognormalCmsSpreadPricer(
            const ext::shared_ptr<CmsCouponPricer> cmsPricer,
            const Handle<Quote> &correlation,
            const Handle<YieldTermStructure> &couponDiscountCurve =
                Handle<YieldTermStructure>(),
            const Size IntegrationPoints = 16,
            const boost::optional<VolatilityType> volatilityType= boost::none,
            const Real shift1 = Null<Real>(), const Real shift2 = Null<Real>());

        /* */
        virtual Real swapletPrice() const;
        virtual Rate swapletRate() const;
        virtual Real capletPrice(Rate effectiveCap) const;
        virtual Rate capletRate(Rate effectiveCap) const;
        virtual Real floorletPrice(Rate effectiveFloor) const;
        virtual Rate floorletRate(Rate effectiveFloor) const;

      private:
        void initialize(const FloatingRateCoupon &coupon);
        Real optionletPrice(Option::Type optionType, Real strike) const;

        Real integrand(const Real) const;
        Real integrand_normal(const Real) const;

        class integrand_f;
        friend class integrand_f;

        ext::shared_ptr<CmsCouponPricer> cmsPricer_;

        Handle<YieldTermStructure> couponDiscountCurve_;

        const CmsSpreadCoupon *coupon_;

        Date today_, fixingDate_, paymentDate_;

        Real fixingTime_;

        Real gearing_, spread_;
        Real spreadLegValue_;
        Real discount_;

        ext::shared_ptr<SwapSpreadIndex> index_;

        ext::shared_ptr<CumulativeNormalDistribution> cnd_;
        ext::shared_ptr<GaussianQuadrature> integrator_;

        Real swapRate1_, swapRate2_, gearing1_, gearing2_;
        Real adjustedRate1_, adjustedRate2_;
        Real vol1_, vol2_;
        Real mu1_, mu2_;
        Real rho_;

        bool inheritedVolatilityType_;
        VolatilityType volType_;
        Real shift1_, shift2_;

        mutable Real phi_, a_, b_, s1_, s2_, m1_, m2_, v1_, v2_, k_;
        mutable Real alpha_, psi_;
        mutable Option::Type optionType_;

        ext::shared_ptr<CmsCoupon> c1_, c2_;
    };
}

#endif
