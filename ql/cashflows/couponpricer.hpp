/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Giorgio Facchinetti
 Copyright (C) 2007 Cristina Duminuco
 Copyright (C) 2011 Ferdinando Ametrano
 Copyright (C) 2015 Peter Caspers

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

/*! \file couponpricer.hpp
    \brief Coupon pricers
*/

#ifndef quantlib_coupon_pricer_hpp
#define quantlib_coupon_pricer_hpp

#include <ql/cashflow.hpp>
#include <ql/indexes/iborindex.hpp>
#include <ql/option.hpp>
#include <ql/quotes/simplequote.hpp>
#include <ql/termstructures/volatility/optionlet/optionletvolatilitystructure.hpp>
#include <ql/termstructures/volatility/swaption/swaptionvolstructure.hpp>
#include <utility>

namespace QuantLib {

    class FloatingRateCoupon;
    class IborCoupon;

    //! generic pricer for floating-rate coupons
    class FloatingRateCouponPricer: public virtual Observer,
                                    public virtual Observable {
      public:
        ~FloatingRateCouponPricer() override = default;
        //! \name required interface
        //@{
        virtual Real swapletPrice() const = 0;
        virtual Rate swapletRate() const = 0;
        virtual Real capletPrice(Rate effectiveCap) const = 0;
        virtual Rate capletRate(Rate effectiveCap) const = 0;
        virtual Real floorletPrice(Rate effectiveFloor) const = 0;
        virtual Rate floorletRate(Rate effectiveFloor) const = 0;
        virtual void initialize(const FloatingRateCoupon& coupon) = 0;
        //@}
        //! \name Observer interface
        //@{
        void update() override { notifyObservers(); }
        //@}
    };

    //! base pricer for capped/floored Ibor coupons
    class IborCouponPricer : public FloatingRateCouponPricer {
      public:
        explicit IborCouponPricer(
            Handle<OptionletVolatilityStructure> v = Handle<OptionletVolatilityStructure>(),
            const bool useIndexedCoupon =
#ifdef QL_USE_IDNDEXED_COUPON
            true
#else
            false
#endif
        )
        : capletVol_(std::move(v)), useIndexedCoupon_(useIndexedCoupon) {
            registerWith(capletVol_);
        }

        bool useIndexedCoupon() const { return useIndexedCoupon_; }

        Handle<OptionletVolatilityStructure> capletVolatility() const{
            return capletVol_;
        }
        void setCapletVolatility(
                            const Handle<OptionletVolatilityStructure>& v =
                                    Handle<OptionletVolatilityStructure>()) {
            unregisterWith(capletVol_);
            capletVol_ = v;
            registerWith(capletVol_);
            update();
        }

      protected:
        void initialize(const FloatingRateCoupon& coupon) override;
        void computeCachedData() const;

        const IborCoupon* coupon_;
        ext::shared_ptr<IborIndex> index_;
        Real gearing_;
        Spread spread_;
        Time accrualPeriod_;
        Date fixingDate_;
        Date fixingValueDate_;
        Date fixingMaturityDate_;
        Real spanningTime_;
        Real spanningTimeIndexMaturity_;

        Handle<OptionletVolatilityStructure> capletVol_;
        bool useIndexedCoupon_;
    };

    /*! Black-formula pricer for capped/floored Ibor coupons
        References for timing adjustments
        Black76             Hull, Options, Futures and other
                            derivatives, 4th ed., page 550
        BivariateLognormal  http://ssrn.com/abstract=2170721 */
    class BlackIborCouponPricer : public IborCouponPricer {
      public:
        enum TimingAdjustment { Black76, BivariateLognormal };
        BlackIborCouponPricer(
            const Handle<OptionletVolatilityStructure>& v = Handle<OptionletVolatilityStructure>(),
            const TimingAdjustment timingAdjustment = Black76,
            Handle<Quote> correlation = Handle<Quote>(ext::shared_ptr<Quote>(new SimpleQuote(1.0))),
            const bool useIndexedCoupon =
#ifdef QL_USE_IDNDEXED_COUPON
            true
#else
            false
#endif
            )
        : IborCouponPricer(v, useIndexedCoupon), timingAdjustment_(timingAdjustment),
          correlation_(std::move(correlation)) {
            { // this additional scope seems required to avoid a misleading-indentation warning
                QL_REQUIRE(timingAdjustment_ == Black76 || timingAdjustment_ == BivariateLognormal,
                           "unknown timing adjustment (code " << timingAdjustment_ << ")");
            }
            registerWith(correlation_);
        };
        void initialize(const FloatingRateCoupon& coupon) override;
        Real swapletPrice() const override;
        Rate swapletRate() const override;
        Real capletPrice(Rate effectiveCap) const override;
        Rate capletRate(Rate effectiveCap) const override;
        Real floorletPrice(Rate effectiveFloor) const override;
        Rate floorletRate(Rate effectiveFloor) const override;

      protected:
        Real optionletPrice(Option::Type optionType,
                            Real effStrike) const;

        virtual Rate adjustedFixing(Rate fixing = Null<Rate>()) const;

        Real discount_;
        Real spreadLegValue_;

      private:
        const TimingAdjustment timingAdjustment_;
        const Handle<Quote> correlation_;
    };

    //! base pricer for vanilla CMS coupons
    class CmsCouponPricer : public FloatingRateCouponPricer {
      public:
        explicit CmsCouponPricer(
            Handle<SwaptionVolatilityStructure> v = Handle<SwaptionVolatilityStructure>())
        : swaptionVol_(std::move(v)) {
            registerWith(swaptionVol_);
        }

        Handle<SwaptionVolatilityStructure> swaptionVolatility() const{
            return swaptionVol_;
        }
        void setSwaptionVolatility(
                            const Handle<SwaptionVolatilityStructure>& v=
                                    Handle<SwaptionVolatilityStructure>()) {
            unregisterWith(swaptionVol_);
            swaptionVol_ = v;
            registerWith(swaptionVol_);
            update();
        }
      private:
        Handle<SwaptionVolatilityStructure> swaptionVol_;
    };

    /*! (CMS) coupon pricer that has a mean reversion parameter which can be
      used to calibrate to cms market quotes */
    class MeanRevertingPricer {
    public:
        virtual Real meanReversion() const = 0;
        virtual void setMeanReversion(const Handle<Quote>&) = 0;
        virtual ~MeanRevertingPricer() = default;
    };

    void setCouponPricer(const Leg& leg,
                         const ext::shared_ptr<FloatingRateCouponPricer>&);

    void setCouponPricers(
            const Leg& leg,
            const std::vector<ext::shared_ptr<FloatingRateCouponPricer> >&);

    /*! set the first matching pricer (if any) to each coupon of the leg */
    void setCouponPricers(
            const Leg& leg,
            const ext::shared_ptr<FloatingRateCouponPricer>&,
            const ext::shared_ptr<FloatingRateCouponPricer>&);

    void setCouponPricers(
            const Leg& leg,
            const ext::shared_ptr<FloatingRateCouponPricer>&,
            const ext::shared_ptr<FloatingRateCouponPricer>&,
            const ext::shared_ptr<FloatingRateCouponPricer>&);

    void setCouponPricers(
            const Leg& leg,
            const ext::shared_ptr<FloatingRateCouponPricer>&,
            const ext::shared_ptr<FloatingRateCouponPricer>&,
            const ext::shared_ptr<FloatingRateCouponPricer>&,
            const ext::shared_ptr<FloatingRateCouponPricer>&);

    // inline

    inline Real BlackIborCouponPricer::swapletPrice() const {
        // past or future fixing is managed in InterestRateIndex::fixing()

        Real swapletPrice = adjustedFixing() * accrualPeriod_ * discount_;
        return gearing_ * swapletPrice + spreadLegValue_;
    }

    inline Rate BlackIborCouponPricer::swapletRate() const {
        return gearing_ * adjustedFixing() + spread_;
    }

    inline Real BlackIborCouponPricer::capletPrice(Rate effectiveCap) const {
        Real capletPrice = optionletPrice(Option::Call, effectiveCap);
        return gearing_ * capletPrice;
    }

    inline Rate BlackIborCouponPricer::capletRate(Rate effectiveCap) const {
        return capletPrice(effectiveCap) / (accrualPeriod_*discount_);
    }

    inline
    Real BlackIborCouponPricer::floorletPrice(Rate effectiveFloor) const {
        Real floorletPrice = optionletPrice(Option::Put, effectiveFloor);
        return gearing_ * floorletPrice;
    }

    inline
    Rate BlackIborCouponPricer::floorletRate(Rate effectiveFloor) const {
        return floorletPrice(effectiveFloor) / (accrualPeriod_*discount_);
    }

}

#endif
