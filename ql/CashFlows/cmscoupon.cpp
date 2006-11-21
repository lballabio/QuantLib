/*
 Copyright (C) 2006 Giorgio Facchinetti
 Copyright (C) 2006 Mario Pucci
 Copyright (C) 2006 StatPro Italia srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/reference/license.html>.


 This program is distributed in the hope that it will be useful, but
 WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 or FITNESS FOR A PARTICULAR PURPOSE. See the license for more details. */

#include <ql/CashFlows/cmscoupon.hpp>
#include <ql/Math/normaldistribution.hpp>
#include <ql/PricingEngines/blackformula.hpp>
#include <ql/option.hpp>

namespace QuantLib {

    CMSCoupon::CMSCoupon(
                    const Real nominal,
                    const Date& paymentDate,
                    const boost::shared_ptr<SwapIndex>& index,
                    const Date& startDate, const Date& endDate,
                    Integer fixingDays,
                    const DayCounter& dayCounter,
                    const boost::shared_ptr<VanillaCMSCouponPricer>& pricer,
                    Real gearing,
                    Spread spread,
                    Rate cap,
                    Rate floor,
                    const Date& refPeriodStart,
                    const Date& refPeriodEnd,
                    bool isInArrears)
    : FloatingRateCoupon(paymentDate, nominal, startDate, endDate,
                         fixingDays, index, gearing, spread,
                         refPeriodStart, refPeriodEnd, dayCounter),
      swapIndex_(index), cap_(cap), floor_(floor), isInArrears_(isInArrears),
      pricer_(pricer) {}

    namespace {

        DiscountFactor discount(Rate R, const Date& tp,
                                DiscountFactor D_s0,
                                const Schedule& s, const DayCounter& dc) {
            Time alpha = dc.yearFraction(s[0],s[1]);
            Time beta = dc.yearFraction(s[0],tp);
            return D_s0/std::pow(1+alpha*R, beta/alpha);
        }

        Real level(Rate R, DiscountFactor D_s0,
                   const Schedule& s, const DayCounter& dc) {
            Real sum = 0.0;
            DiscountFactor discount = D_s0;
            for (Size j=1; j<s.size(); j++) {
                Time alpha = dc.yearFraction(s[j-1],s[j]);
                discount *= 1.0/(1.0+alpha*R);
                sum += alpha*discount;
            }
            return sum;
        }

        Real G(Rate R, const Date& tp,
               DiscountFactor D_s0,
               const Schedule& s, const DayCounter& dc) {
            return discount(R,tp,D_s0,s,dc)/level(R,D_s0,s,dc);
        }

        Real Gprime(Rate R, const Date& tp,
                    DiscountFactor D_s0,
                    const Schedule& s, const DayCounter& dc) {
            static const Spread dR = 1.0e-5;
            return (G(R+dR,tp,D_s0,s,dc)-G(R-dR,tp,D_s0,s,dc))/(2.0*dR);
        }

        Real d_lambda(Real lambda, Rate R, Rate K,
                      Volatility sigma, Time tau) {
            QL_REQUIRE(R > 0.0, "invalid forward rate:" << R);
            QL_REQUIRE(K > 0.0, "invalid strike:" << K);
            QL_REQUIRE(sigma > 0.0, "invalid volatility:" << sigma);
            QL_REQUIRE(tau > 0.0, "invalid residual time:" << tau);
            return (std::log(R/K)+lambda*sigma*sigma*tau)
                /(sigma*std::sqrt(tau));
        }

    }

    Real CMSCoupon::price(const Handle<YieldTermStructure>& discountingCurve) const {
        return amount()*discountingCurve->discount(date());
    }

    Rate CMSCoupon::rate() const
    {
        Date d = fixingDate();
        const Rate Rs = index_->fixing(d);
        Date today = Settings::instance().evaluationDate();
        if (d <= today) {
            // the fixing is determined
            Rate r = gearing_*Rs + spread_;
            if (cap_ != Null<Rate>())
                r = std::min(r, cap_);
            if (floor_ != Null<Rate>())
                r = std::max(r, floor_);
            return r;
        } else {
            pricer_->initialize(*this);
            return pricer_->rate();
        }
    }

    //Rate CMSCoupon::rate1() const {
    //    Date d = fixingDate();
    //    const Rate Rs = index_->fixing(d);
    //    Date today = Settings::instance().evaluationDate();
    //    if (d <= today || gearing_ == 0.0) {
    //        // the fixing is determined
    //        Rate r = gearing_*Rs + spread_;
    //        if (cap_ != Null<Rate>())
    //            r = std::min(r, cap_);
    //        if (floor_ != Null<Rate>())
    //            r = std::max(r, floor_);
    //        return r;
    //    } else {
    //        Rate rate = Rs;
    //        const DayCounter& dc = dayCounter_;
    //        Volatility sigma =
    //            swaptionVol_->volatility(d, index_->tenor(), Rs);
    //        QL_REQUIRE(sigma > 0.0, "internal error: corrupted volatility");
    //        Time tau = dc.yearFraction(today,d);
    //        Schedule s(d, d+index_->tenor(),
    //                   swapIndex_->tenor(),
    //                   index_->calendar(),
    //                   swapIndex_->fixedLegConvention(),
    //                   swapIndex_->fixedLegConvention(),
    //                   true, true); //FIXME
    //        Date tp = date();
    //        DiscountFactor D_s0 =
    //            index_->termStructure()->discount(d);
    //        Real g = G(Rs,tp,D_s0,s,dc), g1 = Gprime(Rs,tp,D_s0,s,dc);
    //        Spread adjustment = (g1/g)*Rs*Rs*(std::exp(sigma*sigma*tau)-1.0);
    //        rate += adjustment;

    //        // translate cap and floor for the coupon to those for the rate.
    //        // also ensure that both strikes are positive as we use BS.
    //        Rate capStrike = Null<Rate>(), floorStrike = Null<Rate>();
    //        if (cap_ != Null<Rate>()) {
    //            if (gearing_ > 0.0)
    //                capStrike = std::max((cap_-spread_)/gearing_,
    //                                     QL_EPSILON);
    //            else
    //                floorStrike = std::max((cap_-spread_)/gearing_,
    //                                       QL_EPSILON);
    //        }
    //        if (floor_ != Null<Rate>()) {
    //            if (gearing_ > 0.0)
    //                floorStrike = std::max((floor_-spread_)/gearing_,
    //                                       QL_EPSILON);
    //            else
    //                capStrike = std::max((floor_-spread_)/gearing_,
    //                                     QL_EPSILON);
    //        }

    //        if (capStrike != Null<Rate>()) {
    //            Rate caplet = blackFormula(Option::Call, capStrike, Rs,
    //                sigma*std::sqrt(tau));
    //            CumulativeNormalDistribution N;
    //            Real N32 = N(d_lambda(1.5,Rs,capStrike,sigma,tau));
    //            Real N12 = N(d_lambda(0.5,Rs,capStrike,sigma,tau));
    //            Real Nm12 = N(d_lambda(-0.5,Rs,capStrike,sigma,tau));

    //            Spread adjustment =
    //                (g1/g)*(Rs*Rs*std::exp(sigma*sigma*tau)*N32
    //                        - Rs*(Rs+capStrike)*N12
    //                        + Rs*capStrike*Nm12);

    //            caplet += adjustment;
    //            rate -= caplet;
    //        }

    //        if (floorStrike != Null<Rate>()) {
    //            Rate floorlet = blackFormula(Option::Put, floorStrike, Rs,
    //                sigma*std::sqrt(tau));
    //            CumulativeNormalDistribution N;
    //            Real N32 = N(-d_lambda(1.5,Rs,floorStrike,sigma,tau));
    //            Real N12 = N(-d_lambda(0.5,Rs,floorStrike,sigma,tau));
    //            Real Nm12 = N(-d_lambda(-0.5,Rs,floorStrike,sigma,tau));

    //            Spread adjustment =
    //                -(g1/g)*(Rs*Rs*std::exp(sigma*sigma*tau)*N32
    //                         - Rs*(Rs+floorStrike)*N12
    //                         + Rs*floorStrike*Nm12);

    //            floorlet += adjustment;
    //            rate += floorlet;
    //        }
    //        return gearing_*rate + spread_;
    //    }
    //}

    void CMSCoupon::accept(AcyclicVisitor& v) {
        Visitor<CMSCoupon>* v1 = dynamic_cast<Visitor<CMSCoupon>*>(&v);
        if (v1 != 0)
            v1->visit(*this);
        else
            FloatingRateCoupon::accept(v);
    }

    Date CMSCoupon::fixingDate() const {
        Date refDate = isInArrears_ ? accrualEndDate_ : accrualStartDate_;
        return index_->calendar().advance(refDate, -fixingDays_*Days,
                                          Preceding);
    }

}
