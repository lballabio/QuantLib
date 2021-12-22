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

#include <ql/cashflows/capflooredcoupon.hpp>
#include <ql/cashflows/couponpricer.hpp>
#include <ql/cashflows/digitalcmscoupon.hpp>
#include <ql/cashflows/digitalcoupon.hpp>
#include <ql/cashflows/digitaliborcoupon.hpp>
#include <ql/cashflows/rangeaccrual.hpp>
#include <ql/cashflows/subperiodcoupon.hpp>
#include <ql/experimental/coupons/cmsspreadcoupon.hpp>        /* internal */
#include <ql/experimental/coupons/digitalcmsspreadcoupon.hpp> /* internal */
#include <ql/pricingengines/blackformula.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>
#include <utility>

namespace QuantLib {

//===========================================================================//
//                              IborCouponPricer                             //
//===========================================================================//

    IborCouponPricer::IborCouponPricer(
            Handle<OptionletVolatilityStructure> v,
            boost::optional<bool> useIndexedCoupon)
        : capletVol_(std::move(v)),
          useIndexedCoupon_(useIndexedCoupon ?
                            *useIndexedCoupon :
                            !IborCoupon::Settings::instance().usingAtParCoupons()) {
        registerWith(capletVol_);
    }

    void IborCouponPricer::initializeCachedData(const IborCoupon& coupon) const {

        if(coupon.cachedDataIsInitialized_)
            return;

        coupon.fixingValueDate_ = coupon.iborIndex()->fixingCalendar().advance(
            coupon.fixingDate_, coupon.iborIndex()->fixingDays(), Days);
        coupon.fixingMaturityDate_ = coupon.iborIndex()->maturityDate(coupon.fixingValueDate_);

        if (useIndexedCoupon_) {
            coupon.fixingEndDate_ = coupon.fixingMaturityDate_;
        } else {
            if (coupon.isInArrears_)
                coupon.fixingEndDate_ = coupon.fixingMaturityDate_;
            else { // par coupon approximation
                Date nextFixingDate = coupon.iborIndex()->fixingCalendar().advance(
                    coupon.accrualEndDate(), -static_cast<Integer>(coupon.fixingDays_), Days);
                coupon.fixingEndDate_ = coupon.iborIndex()->fixingCalendar().advance(
                    nextFixingDate, coupon.iborIndex()->fixingDays(), Days);
                // make sure the estimation period contains at least one day
                coupon.fixingEndDate_ =
                    std::max(coupon.fixingEndDate_, coupon.fixingValueDate_ + 1);
            }
        }

        coupon.spanningTime_ = coupon.iborIndex()->dayCounter().yearFraction(
            coupon.fixingValueDate_, coupon.fixingEndDate_);

        QL_REQUIRE(coupon.spanningTime_ > 0.0,
                   "\n cannot calculate forward rate between "
                       << coupon.fixingValueDate_ << " and " << coupon.fixingEndDate_
                       << ":\n non positive time (" << coupon.spanningTime_ << ") using "
                       << coupon.iborIndex()->dayCounter().name() << " daycounter");

        coupon.spanningTimeIndexMaturity_ = coupon.iborIndex()->dayCounter().yearFraction(
            coupon.fixingValueDate_, coupon.fixingMaturityDate_);

        coupon.cachedDataIsInitialized_ = true;
    }

    void IborCouponPricer::initialize(const FloatingRateCoupon& coupon) {
        coupon_ = dynamic_cast<const IborCoupon *>(&coupon);
        QL_REQUIRE(coupon_, "IborCouponPricer: expected IborCoupon");

        initializeCachedData(*coupon_);

        index_ = coupon_->iborIndex();
        gearing_ = coupon_->gearing();
        spread_ = coupon_->spread();
        accrualPeriod_ = coupon_->accrualPeriod();
        QL_REQUIRE(accrualPeriod_ != 0.0, "null accrual period");

        fixingDate_ = coupon_->fixingDate_;
        fixingValueDate_ = coupon_->fixingValueDate_;
        fixingMaturityDate_ = coupon_->fixingMaturityDate_;
        spanningTime_ = coupon_->spanningTime_;
        spanningTimeIndexMaturity_ = coupon_->spanningTimeIndexMaturity_;
    }


//===========================================================================//
//                              BlackIborCouponPricer                        //
//===========================================================================//

    void BlackIborCouponPricer::initialize(const FloatingRateCoupon& coupon) {

        IborCouponPricer::initialize(coupon);

        Handle<YieldTermStructure> rateCurve = index_->forwardingTermStructure();

        if (rateCurve.empty()) {
            discount_ = Null<Real>(); // might not be needed, will be checked later
            QL_DEPRECATED_DISABLE_WARNING
            spreadLegValue_ = Null<Real>();
            QL_DEPRECATED_ENABLE_WARNING
        } else {
            Date paymentDate = coupon_->date();
            if (paymentDate > rateCurve->referenceDate())
                discount_ = rateCurve->discount(paymentDate);
            else
                discount_ = 1.0;
            QL_DEPRECATED_DISABLE_WARNING
            spreadLegValue_ = spread_ * accrualPeriod_ * discount_;
            QL_DEPRECATED_ENABLE_WARNING
        }

    }

    Real BlackIborCouponPricer::optionletRate(Option::Type optionType, Real effStrike) const {
        if (fixingDate_ <= Settings::instance().evaluationDate()) {
            // the amount is determined
            Real a, b;
            if (optionType==Option::Call) {
                a = coupon_->indexFixing();
                b = effStrike;
            } else {
                a = effStrike;
                b = coupon_->indexFixing();
            }
            return std::max(a - b, 0.0);
        } else {
            // not yet determined, use Black model
            QL_REQUIRE(!capletVolatility().empty(),
                       "missing optionlet volatility");
            Real stdDev =
                std::sqrt(capletVolatility()->blackVariance(fixingDate_,
                                                            effStrike));
            Real shift = capletVolatility()->displacement();
            bool shiftedLn =
                capletVolatility()->volatilityType() == ShiftedLognormal;
            Rate fixing =
                shiftedLn
                    ? blackFormula(optionType, effStrike, adjustedFixing(),
                                   stdDev, 1.0, shift)
                    : bachelierBlackFormula(optionType, effStrike,
                                            adjustedFixing(), stdDev, 1.0);
            return fixing;
        }
    }

    Real BlackIborCouponPricer::optionletPrice(Option::Type optionType,
                                               Real effStrike) const {
        QL_REQUIRE(discount_ != Null<Rate>(), "no forecast curve provided");
        return optionletRate(optionType, effStrike) * accrualPeriod_ * discount_;
    }

    Rate BlackIborCouponPricer::adjustedFixing(Rate fixing) const {

        if (fixing == Null<Rate>())
            fixing = coupon_->indexFixing();

        // if the pay date is equal to the index estimation end date
        // there is no convexity; in all other cases in principle an
        // adjustment has to be applied, but the Black76 method only
        // applies the standard in arrears adjustment; the bivariate
        // lognormal method is more accurate in this regard.
        if ((!coupon_->isInArrears() && timingAdjustment_ == Black76))
            return fixing;
        const Date& d1 = fixingDate_;
        const Date& d2 = fixingValueDate_;
        const Date& d3 = fixingMaturityDate_;
        if (coupon_->date() == d3)
            return fixing;

        QL_REQUIRE(!capletVolatility().empty(),
                   "missing optionlet volatility");
        Date referenceDate = capletVolatility()->referenceDate();
        // no variance has accumulated, so the convexity is zero
        if (d1 <= referenceDate)
            return fixing;
        const Time& tau = spanningTimeIndexMaturity_;
        Real variance = capletVolatility()->blackVariance(d1, fixing);

        Real shift = capletVolatility()->displacement();
        bool shiftedLn =
            capletVolatility()->volatilityType() == ShiftedLognormal;

        Spread adjustment = shiftedLn
                                ? (fixing + shift) * (fixing + shift) *
                                      variance * tau / (1.0 + fixing * tau)
                                : variance * tau / (1.0 + fixing * tau);

        if (timingAdjustment_ == BivariateLognormal) {
            QL_REQUIRE(!correlation_.empty(), "no correlation given");
            const Date& d4 = coupon_->date();
            const Date& d5 = d4 >= d3 ? d3 : d2;
            Time tau2 = index_->dayCounter().yearFraction(d5, d4);
            if (d4 >= d3)
                adjustment = 0.0;
            // if d4 < d2 (payment before index start) we just apply the
            // Black76 in arrears adjustment
            if (tau2 > 0.0) {
                Real fixing2 =
                    (index_->forwardingTermStructure()->discount(d5) /
                         index_->forwardingTermStructure()->discount(d4) -
                     1.0) /
                    tau2;
                adjustment -= shiftedLn
                                  ? correlation_->value() * tau2 * variance *
                                        (fixing + shift) * (fixing2 + shift) /
                                        (1.0 + fixing2 * tau2)
                                  : correlation_->value() * tau2 * variance /
                                        (1.0 + fixing2 * tau2);
            }
        }
        return fixing + adjustment;
    }

//===========================================================================//
//                         CouponSelectorToSetPricer                         //
//===========================================================================//

    namespace {

        class PricerSetter : public AcyclicVisitor,
                             public Visitor<CashFlow>,
                             public Visitor<Coupon>,
                             public Visitor<FloatingRateCoupon>,
                             public Visitor<CappedFlooredCoupon>,
                             public Visitor<IborCoupon>,
                             public Visitor<CmsCoupon>,
                             public Visitor<CmsSpreadCoupon>,
                             public Visitor<CappedFlooredIborCoupon>,
                             public Visitor<CappedFlooredCmsCoupon>,
                             public Visitor<CappedFlooredCmsSpreadCoupon>,
                             public Visitor<DigitalIborCoupon>,
                             public Visitor<DigitalCmsCoupon>,
                             public Visitor<DigitalCmsSpreadCoupon>,
                             public Visitor<RangeAccrualFloatersCoupon>,
                             public Visitor<SubPeriodsCoupon> {
          private:
            ext::shared_ptr<FloatingRateCouponPricer> pricer_;
          public:
            explicit PricerSetter(ext::shared_ptr<FloatingRateCouponPricer> pricer)
            : pricer_(std::move(pricer)) {}

            void visit(CashFlow& c) override;
            void visit(Coupon& c) override;
            void visit(FloatingRateCoupon& c) override;
            void visit(CappedFlooredCoupon& c) override;
            void visit(IborCoupon& c) override;
            void visit(CappedFlooredIborCoupon& c) override;
            void visit(DigitalIborCoupon& c) override;
            void visit(CmsCoupon& c) override;
            void visit(CmsSpreadCoupon& c) override;
            void visit(CappedFlooredCmsCoupon& c) override;
            void visit(CappedFlooredCmsSpreadCoupon& c) override;
            void visit(DigitalCmsCoupon& c) override;
            void visit(DigitalCmsSpreadCoupon& c) override;
            void visit(RangeAccrualFloatersCoupon& c) override;
            void visit(SubPeriodsCoupon& c) override;
        };

        void PricerSetter::visit(CashFlow&) {
            // nothing to do
        }

        void PricerSetter::visit(Coupon&) {
            // nothing to do
        }

        void PricerSetter::visit(FloatingRateCoupon& c) {
            c.setPricer(pricer_);
        }

        void PricerSetter::visit(CappedFlooredCoupon& c) {
            // we might end up here because a CappedFlooredCoupon
            // was directly constructed; we should then check
            // the underlying for consistency with the pricer
            if (ext::dynamic_pointer_cast<IborCoupon>(c.underlying()) != nullptr) {
                QL_REQUIRE(ext::dynamic_pointer_cast<IborCouponPricer>(pricer_),
                           "pricer not compatible with Ibor Coupon");
            } else if (ext::dynamic_pointer_cast<CmsCoupon>(c.underlying()) != nullptr) {
                QL_REQUIRE(ext::dynamic_pointer_cast<CmsCouponPricer>(pricer_),
                           "pricer not compatible with CMS Coupon");
            } else if (ext::dynamic_pointer_cast<CmsSpreadCoupon>(c.underlying()) != nullptr) {
                QL_REQUIRE(ext::dynamic_pointer_cast<CmsSpreadCouponPricer>(pricer_),
                           "pricer not compatible with CMS spread Coupon");
            }
            c.setPricer(pricer_);
        }

        void PricerSetter::visit(IborCoupon& c) {
            const ext::shared_ptr<IborCouponPricer> iborCouponPricer =
                ext::dynamic_pointer_cast<IborCouponPricer>(pricer_);
            QL_REQUIRE(iborCouponPricer,
                       "pricer not compatible with Ibor coupon");
            c.setPricer(iborCouponPricer);
        }

        void PricerSetter::visit(DigitalIborCoupon& c) {
            const ext::shared_ptr<IborCouponPricer> iborCouponPricer =
                ext::dynamic_pointer_cast<IborCouponPricer>(pricer_);
            QL_REQUIRE(iborCouponPricer,
                       "pricer not compatible with Ibor coupon");
            c.setPricer(iborCouponPricer);
        }

        void PricerSetter::visit(CappedFlooredIborCoupon& c) {
            const ext::shared_ptr<IborCouponPricer> iborCouponPricer =
                ext::dynamic_pointer_cast<IborCouponPricer>(pricer_);
            QL_REQUIRE(iborCouponPricer,
                       "pricer not compatible with Ibor coupon");
            c.setPricer(iborCouponPricer);
        }

        void PricerSetter::visit(CmsCoupon& c) {
            const ext::shared_ptr<CmsCouponPricer> cmsCouponPricer =
                ext::dynamic_pointer_cast<CmsCouponPricer>(pricer_);
            QL_REQUIRE(cmsCouponPricer,
                       "pricer not compatible with CMS coupon");
            c.setPricer(cmsCouponPricer);
        }

        void PricerSetter::visit(CmsSpreadCoupon& c) {
            const ext::shared_ptr<CmsSpreadCouponPricer> cmsSpreadCouponPricer =
                ext::dynamic_pointer_cast<CmsSpreadCouponPricer>(pricer_);
            QL_REQUIRE(cmsSpreadCouponPricer,
                       "pricer not compatible with CMS spread coupon");
            c.setPricer(cmsSpreadCouponPricer);
        }

        void PricerSetter::visit(CappedFlooredCmsCoupon& c) {
            const ext::shared_ptr<CmsCouponPricer> cmsCouponPricer =
                ext::dynamic_pointer_cast<CmsCouponPricer>(pricer_);
            QL_REQUIRE(cmsCouponPricer,
                       "pricer not compatible with CMS coupon");
            c.setPricer(cmsCouponPricer);
        }

        void PricerSetter::visit(CappedFlooredCmsSpreadCoupon& c) {
            const ext::shared_ptr<CmsSpreadCouponPricer> cmsSpreadCouponPricer =
                ext::dynamic_pointer_cast<CmsSpreadCouponPricer>(pricer_);
            QL_REQUIRE(cmsSpreadCouponPricer,
                       "pricer not compatible with CMS spread coupon");
            c.setPricer(cmsSpreadCouponPricer);
        }

        void PricerSetter::visit(DigitalCmsCoupon& c) {
            const ext::shared_ptr<CmsCouponPricer> cmsCouponPricer =
                ext::dynamic_pointer_cast<CmsCouponPricer>(pricer_);
            QL_REQUIRE(cmsCouponPricer,
                       "pricer not compatible with CMS coupon");
            c.setPricer(cmsCouponPricer);
        }

        void PricerSetter::visit(DigitalCmsSpreadCoupon& c) {
            const ext::shared_ptr<CmsSpreadCouponPricer> cmsSpreadCouponPricer =
                ext::dynamic_pointer_cast<CmsSpreadCouponPricer>(pricer_);
            QL_REQUIRE(cmsSpreadCouponPricer,
                       "pricer not compatible with CMS spread coupon");
            c.setPricer(cmsSpreadCouponPricer);
        }

        void PricerSetter::visit(RangeAccrualFloatersCoupon& c) {
            const ext::shared_ptr<RangeAccrualPricer> rangeAccrualPricer =
                ext::dynamic_pointer_cast<RangeAccrualPricer>(pricer_);
            QL_REQUIRE(rangeAccrualPricer,
                       "pricer not compatible with range-accrual coupon");
            c.setPricer(rangeAccrualPricer);
        }

        void PricerSetter::visit(SubPeriodsCoupon& c) {
            const ext::shared_ptr<SubPeriodsPricer> subPeriodsPricer =
                ext::dynamic_pointer_cast<SubPeriodsPricer>(pricer_);
            QL_REQUIRE(subPeriodsPricer,
                       "pricer not compatible with sub-period coupon");
            c.setPricer(subPeriodsPricer);
        }

        void setCouponPricersFirstMatching(const Leg& leg,
                                           const std::vector<ext::shared_ptr<FloatingRateCouponPricer> >& p) {
            std::vector<PricerSetter> setter;
            setter.reserve(p.size());
            for (const auto& i : p) {
                setter.emplace_back(i);
            }
            for (const auto& i : leg) {
                Size j = 0;
                do {
                    try {
                        i->accept(setter[j]);
                        j = p.size();
                    } catch (...) {
                        ++j;
                    }
                } while (j < p.size());
            }
        }

    } // anonymous namespace

    void setCouponPricer(const Leg& leg, const ext::shared_ptr<FloatingRateCouponPricer>& pricer) {
            PricerSetter setter(pricer);
            for (const auto& i : leg) {
                i->accept(setter);
            }
    }

    void setCouponPricers(
            const Leg& leg,
            const std::vector<ext::shared_ptr<FloatingRateCouponPricer> >&
                                                                    pricers) {
        Size nCashFlows = leg.size();
        QL_REQUIRE(nCashFlows>0, "no cashflows");

        Size nPricers = pricers.size();
        QL_REQUIRE(nCashFlows >= nPricers,
                   "mismatch between leg size (" << nCashFlows <<
                   ") and number of pricers (" << nPricers << ")");

        for (Size i=0; i<nCashFlows; ++i) {
            PricerSetter setter(i<nPricers ? pricers[i] : pricers[nPricers-1]);
            leg[i]->accept(setter);
        }
    }

    void setCouponPricers(
            const Leg& leg,
            const ext::shared_ptr<FloatingRateCouponPricer>& p1,
            const ext::shared_ptr<FloatingRateCouponPricer>& p2) {
        std::vector<ext::shared_ptr<FloatingRateCouponPricer> > p;
        p.push_back(p1);
        p.push_back(p2);
        setCouponPricersFirstMatching(leg, p);
    }

    void setCouponPricers(
            const Leg& leg,
            const ext::shared_ptr<FloatingRateCouponPricer>& p1,
            const ext::shared_ptr<FloatingRateCouponPricer>& p2,
            const ext::shared_ptr<FloatingRateCouponPricer>& p3) {
        std::vector<ext::shared_ptr<FloatingRateCouponPricer> > p;
        p.push_back(p1);
        p.push_back(p2);
        p.push_back(p3);
        setCouponPricersFirstMatching(leg, p);
    }

    void setCouponPricers(
            const Leg& leg,
            const ext::shared_ptr<FloatingRateCouponPricer>& p1,
            const ext::shared_ptr<FloatingRateCouponPricer>& p2,
            const ext::shared_ptr<FloatingRateCouponPricer>& p3,
            const ext::shared_ptr<FloatingRateCouponPricer>& p4) {
        std::vector<ext::shared_ptr<FloatingRateCouponPricer> > p;
        p.push_back(p1);
        p.push_back(p2);
        p.push_back(p3);
        p.push_back(p4);
        setCouponPricersFirstMatching(leg, p);
    }


}
