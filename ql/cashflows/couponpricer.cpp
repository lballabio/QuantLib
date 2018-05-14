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

#include <ql/cashflows/couponpricer.hpp>
#include <ql/cashflows/capflooredcoupon.hpp>
#include <ql/cashflows/digitalcoupon.hpp>
#include <ql/cashflows/digitalcmscoupon.hpp>
#include <ql/cashflows/digitaliborcoupon.hpp>
#include <ql/cashflows/rangeaccrual.hpp>
#include <ql/experimental/coupons/subperiodcoupons.hpp> /* internal */
#include <ql/experimental/coupons/cmsspreadcoupon.hpp>  /* internal */
#include <ql/experimental/coupons/digitalcmsspreadcoupon.hpp>  /* internal */
#include <ql/pricingengines/blackformula.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>

using boost::dynamic_pointer_cast;

namespace QuantLib {

//===========================================================================//
//                              BlackIborCouponPricer                        //
//===========================================================================//

    void BlackIborCouponPricer::initialize(const FloatingRateCoupon& coupon) {

        gearing_ = coupon.gearing();
        spread_ = coupon.spread();
        accrualPeriod_ = coupon.accrualPeriod();
        QL_REQUIRE(accrualPeriod_ != 0.0, "null accrual period");

        index_ = dynamic_pointer_cast<IborIndex>(coupon.index());
        if (!index_) {
            // check if the coupon was right
            const IborCoupon* c = dynamic_cast<const IborCoupon*>(&coupon);
            QL_REQUIRE(c, "IborCoupon required");
            // coupon was right, index is not
            QL_FAIL("IborIndex required");
        }
        Handle<YieldTermStructure> rateCurve =
                                            index_->forwardingTermStructure();

        Date paymentDate = coupon.date();
        if (paymentDate > rateCurve->referenceDate())
            discount_ = rateCurve->discount(paymentDate);
        else
            discount_ = 1.0;

        spreadLegValue_ = spread_ * accrualPeriod_ * discount_;

        coupon_ = &coupon;
    }

    Real BlackIborCouponPricer::optionletPrice(Option::Type optionType,
                                               Real effStrike) const {
        Date fixingDate = coupon_->fixingDate();
        if (fixingDate <= Settings::instance().evaluationDate()) {
            // the amount is determined
            Real a, b;
            if (optionType==Option::Call) {
                a = coupon_->indexFixing();
                b = effStrike;
            } else {
                a = effStrike;
                b = coupon_->indexFixing();
            }
            return std::max(a - b, 0.0)* accrualPeriod_*discount_;
        } else {
            // not yet determined, use Black model
            QL_REQUIRE(!capletVolatility().empty(),
                       "missing optionlet volatility");
            Real stdDev =
                std::sqrt(capletVolatility()->blackVariance(fixingDate,
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
            return fixing * accrualPeriod_ * discount_;
        }
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
        Date d1 = coupon_->fixingDate();
        Date d2 = index_->valueDate(d1);
        Date d3 = index_->maturityDate(d2);
        if (coupon_->date() == d3)
            return fixing;

        QL_REQUIRE(!capletVolatility().empty(),
                   "missing optionlet volatility");
        Date referenceDate = capletVolatility()->referenceDate();
        // no variance has accumulated, so the convexity is zero
        if (d1 <= referenceDate)
            return fixing;
        Time tau = index_->dayCounter().yearFraction(d2, d3);
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
            Date d4 = coupon_->date();
            Date d5 = d4 >= d3 ? d3 : d2;
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
            boost::shared_ptr<FloatingRateCouponPricer> pricer_;
          public:
            explicit PricerSetter(
                    const boost::shared_ptr<FloatingRateCouponPricer>& pricer)
            : pricer_(pricer) {}

            void visit(CashFlow& c);
            void visit(Coupon& c);
            void visit(FloatingRateCoupon& c);
            void visit(CappedFlooredCoupon& c);
            void visit(IborCoupon& c);
            void visit(CappedFlooredIborCoupon& c);
            void visit(DigitalIborCoupon& c);
            void visit(CmsCoupon& c);
            void visit(CmsSpreadCoupon& c);
            void visit(CappedFlooredCmsCoupon& c);
            void visit(CappedFlooredCmsSpreadCoupon& c);
            void visit(DigitalCmsCoupon& c);
            void visit(DigitalCmsSpreadCoupon& c);
            void visit(RangeAccrualFloatersCoupon& c);
            void visit(SubPeriodsCoupon& c);
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
            if (boost::dynamic_pointer_cast<IborCoupon>(c.underlying())) {
                QL_REQUIRE(boost::dynamic_pointer_cast<IborCouponPricer>(pricer_),
                           "pricer not compatible with Ibor Coupon");
            } else if (boost::dynamic_pointer_cast<CmsCoupon>(c.underlying())) {
                QL_REQUIRE(boost::dynamic_pointer_cast<CmsCouponPricer>(pricer_),
                           "pricer not compatible with CMS Coupon");
            } else if (boost::dynamic_pointer_cast<CmsSpreadCoupon>(c.underlying())) {
                QL_REQUIRE(boost::dynamic_pointer_cast<CmsSpreadCouponPricer>(pricer_),
                           "pricer not compatible with CMS spread Coupon");
            }
            c.setPricer(pricer_);
        }

        void PricerSetter::visit(IborCoupon& c) {
            const boost::shared_ptr<IborCouponPricer> iborCouponPricer =
                boost::dynamic_pointer_cast<IborCouponPricer>(pricer_);
            QL_REQUIRE(iborCouponPricer,
                       "pricer not compatible with Ibor coupon");
            c.setPricer(iborCouponPricer);
        }

        void PricerSetter::visit(DigitalIborCoupon& c) {
            const boost::shared_ptr<IborCouponPricer> iborCouponPricer =
                boost::dynamic_pointer_cast<IborCouponPricer>(pricer_);
            QL_REQUIRE(iborCouponPricer,
                       "pricer not compatible with Ibor coupon");
            c.setPricer(iborCouponPricer);
        }

        void PricerSetter::visit(CappedFlooredIborCoupon& c) {
            const boost::shared_ptr<IborCouponPricer> iborCouponPricer =
                boost::dynamic_pointer_cast<IborCouponPricer>(pricer_);
            QL_REQUIRE(iborCouponPricer,
                       "pricer not compatible with Ibor coupon");
            c.setPricer(iborCouponPricer);
        }

        void PricerSetter::visit(CmsCoupon& c) {
            const boost::shared_ptr<CmsCouponPricer> cmsCouponPricer =
                boost::dynamic_pointer_cast<CmsCouponPricer>(pricer_);
            QL_REQUIRE(cmsCouponPricer,
                       "pricer not compatible with CMS coupon");
            c.setPricer(cmsCouponPricer);
        }

        void PricerSetter::visit(CmsSpreadCoupon& c) {
            const boost::shared_ptr<CmsSpreadCouponPricer> cmsSpreadCouponPricer =
                boost::dynamic_pointer_cast<CmsSpreadCouponPricer>(pricer_);
            QL_REQUIRE(cmsSpreadCouponPricer,
                       "pricer not compatible with CMS spread coupon");
            c.setPricer(cmsSpreadCouponPricer);
        }

        void PricerSetter::visit(CappedFlooredCmsCoupon& c) {
            const boost::shared_ptr<CmsCouponPricer> cmsCouponPricer =
                boost::dynamic_pointer_cast<CmsCouponPricer>(pricer_);
            QL_REQUIRE(cmsCouponPricer,
                       "pricer not compatible with CMS coupon");
            c.setPricer(cmsCouponPricer);
        }

        void PricerSetter::visit(CappedFlooredCmsSpreadCoupon& c) {
            const boost::shared_ptr<CmsSpreadCouponPricer> cmsSpreadCouponPricer =
                boost::dynamic_pointer_cast<CmsSpreadCouponPricer>(pricer_);
            QL_REQUIRE(cmsSpreadCouponPricer,
                       "pricer not compatible with CMS spread coupon");
            c.setPricer(cmsSpreadCouponPricer);
        }

        void PricerSetter::visit(DigitalCmsCoupon& c) {
            const boost::shared_ptr<CmsCouponPricer> cmsCouponPricer =
                boost::dynamic_pointer_cast<CmsCouponPricer>(pricer_);
            QL_REQUIRE(cmsCouponPricer,
                       "pricer not compatible with CMS coupon");
            c.setPricer(cmsCouponPricer);
        }

        void PricerSetter::visit(DigitalCmsSpreadCoupon& c) {
            const boost::shared_ptr<CmsSpreadCouponPricer> cmsSpreadCouponPricer =
                boost::dynamic_pointer_cast<CmsSpreadCouponPricer>(pricer_);
            QL_REQUIRE(cmsSpreadCouponPricer,
                       "pricer not compatible with CMS spread coupon");
            c.setPricer(cmsSpreadCouponPricer);
        }

        void PricerSetter::visit(RangeAccrualFloatersCoupon& c) {
            const boost::shared_ptr<RangeAccrualPricer> rangeAccrualPricer =
                boost::dynamic_pointer_cast<RangeAccrualPricer>(pricer_);
            QL_REQUIRE(rangeAccrualPricer,
                       "pricer not compatible with range-accrual coupon");
            c.setPricer(rangeAccrualPricer);
        }

        void PricerSetter::visit(SubPeriodsCoupon& c) {
            const boost::shared_ptr<SubPeriodsPricer> subPeriodsPricer =
                boost::dynamic_pointer_cast<SubPeriodsPricer>(pricer_);
            QL_REQUIRE(subPeriodsPricer,
                       "pricer not compatible with sub-period coupon");
            c.setPricer(subPeriodsPricer);
        }

        void setCouponPricersFirstMatching(const Leg& leg,
                                           const std::vector<boost::shared_ptr<FloatingRateCouponPricer> >& p) {
            std::vector<PricerSetter> setter;
            for (Size i = 0; i < p.size(); ++i) {
                setter.push_back(PricerSetter(p[i]));
            }
            for (Size i = 0; i < leg.size(); ++i) {
                Size j = 0;
                do {
                    try {
                        leg[i]->accept(setter[j]);
                        j = p.size();
                    } catch (...) {
                        ++j;
                    }
                } while (j < p.size());
            }
        }

    } // anonymous namespace

    void setCouponPricer(const Leg& leg, const boost::shared_ptr<FloatingRateCouponPricer>& pricer) {
            PricerSetter setter(pricer);
            for (Size i = 0; i < leg.size(); ++i) {
                leg[i]->accept(setter);
            }
    }

    void setCouponPricers(
            const Leg& leg,
            const std::vector<boost::shared_ptr<FloatingRateCouponPricer> >&
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
            const boost::shared_ptr<FloatingRateCouponPricer>& p1,
            const boost::shared_ptr<FloatingRateCouponPricer>& p2) {
        std::vector<boost::shared_ptr<FloatingRateCouponPricer> > p;
        p.push_back(p1);
        p.push_back(p2);
        setCouponPricersFirstMatching(leg, p);
    }

    void setCouponPricers(
            const Leg& leg,
            const boost::shared_ptr<FloatingRateCouponPricer>& p1,
            const boost::shared_ptr<FloatingRateCouponPricer>& p2,
            const boost::shared_ptr<FloatingRateCouponPricer>& p3) {
        std::vector<boost::shared_ptr<FloatingRateCouponPricer> > p;
        p.push_back(p1);
        p.push_back(p2);
        p.push_back(p3);
        setCouponPricersFirstMatching(leg, p);
    }

    void setCouponPricers(
            const Leg& leg,
            const boost::shared_ptr<FloatingRateCouponPricer>& p1,
            const boost::shared_ptr<FloatingRateCouponPricer>& p2,
            const boost::shared_ptr<FloatingRateCouponPricer>& p3,
            const boost::shared_ptr<FloatingRateCouponPricer>& p4) {
        std::vector<boost::shared_ptr<FloatingRateCouponPricer> > p;
        p.push_back(p1);
        p.push_back(p2);
        p.push_back(p3);
        p.push_back(p4);
        setCouponPricersFirstMatching(leg, p);
    }


}
