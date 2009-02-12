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

/*! \file inflationcappedcouponpricer.hpp
    \brief Pricers for coupons depending on inflation and nominal
*/

#ifndef quantlib_inflation_capped_coupon_pricer_hpp
#define quantlib_inflation_capped_coupon_pricer_hpp

#include <ql/experimental/inflation/nominalyoyinflationcoupon.hpp>
#include <ql/math/distributions/normaldistribution.hpp>

namespace QuantLib {

    //! e.g. for French Inflation product BSM...
    class InflationCappedCouponPricer : public virtual Observer,
                                        public virtual Observable {
      public:
        InflationCappedCouponPricer(
                          const Real tau,
                          const Real Nstart, const Real Nmu, const Real Nsd,
                          const Real DDstart, const Real DDs, const Real DDsd,
                          const Real correlation)
        : tau_(tau),
          Nstart_(Nstart), Nmu_(Nmu), Nsd_(Nsd),
          DDstart_(DDstart), DDs_(DDs), DDsd_(DDsd),
          correlation_(correlation) {}
        virtual ~InflationCappedCouponPricer() {}

        //! \name Interface
        //@{
        Real price() const;
        Rate rate() const;
        Rate adjustedFixing() const;
        //@}

        //! \name Observer interface
        //@{
        virtual void update(){notifyObservers();}
        //@}

        // they can all use this, it pulls out of the coupon
        // the details required for pricing it
        virtual void initialize(const InflationCappedCoupon& coupon);

      protected:
        // descendents only need implement this
        virtual Real optionletPriceImp() const = 0;

        //! data
        //@{
        boost::shared_ptr<YoYInflationIndex> inflationIndex_;
        boost::shared_ptr<IborIndex> nominalIndex_;
        Date today_;
        const InflationCappedCoupon* coupon_;
        Real tau_, Nstart_, Nmu_, Nsd_, DDstart_, DDs_, DDsd_, correlation_;
        Real discount_;
        Real nominalSpread_, capGearing_, inflationSpread_;
        //@}
    };


    //! Prices a Normal (inflation) versus a Displaced Diffusion (nominal)
    class AnalyticInflationCappedCouponPricer
        : public InflationCappedCouponPricer {
      public:
        AnalyticInflationCappedCouponPricer(
                          const Real tau,
                          const Real Nstart, const Real Nmu, const Real Nsd,
                          const Real DDstart, const Real DDs, const Real DDsd,
                          const Real correlation,
                          const Size nPoints) :
        InflationCappedCouponPricer(tau, Nstart, Nmu, Nsd,
                                    DDstart, DDs, DDsd, correlation),
        nPoints_(nPoints) {}

      protected:
        Size nPoints_;
        CumulativeNormalDistribution Phi;
        virtual Real payoffContribution(const Real sd) const;
        virtual Real optionletPriceImp() const;
    };


    //! Prices a Normal (inflation) versus a Displaced Diffusion (nominal)
    class MCInflationCappedCouponPricer : public InflationCappedCouponPricer {
      public:
        MCInflationCappedCouponPricer(
                          const Real tau,
                          const Real Nstart, const Real Nmu, const Real Nsd,
                          const Real DDstart, const Real DDs, const Real DDsd,
                          const Real correlation,
                          const Size nSamples, const long seed = 0) :
        InflationCappedCouponPricer(tau, Nstart, Nmu, Nsd,
                                    DDstart, DDs, DDsd, correlation),
        nSamples_(nSamples), seed_(seed) {}

      protected:
        Size nSamples_;
        long seed_;
        virtual Real optionletPriceImp() const;
    };

}

#endif

