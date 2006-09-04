/*
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
#include <ql/PricingEngines/blackmodel.hpp>

namespace QuantLib {

    CMSCoupon::CMSCoupon(const Real nominal,
                  const Date& paymentDate,
                  const boost::shared_ptr<SwapIndex>& index,
                  const Date& startDate, const Date& endDate,
                  Integer fixingDays,
                  const DayCounter& dayCounter,
                  const boost::shared_ptr<VanillaCMSCouponPricer>& Pricer,
                  Real gearing,
                  Rate spread,
                  Rate cap,
                  Rate floor,
                  Real meanReversion,
                  const Date& refPeriodStart,
                  const Date& refPeriodEnd)
    : FloatingRateCoupon(paymentDate, nominal, startDate, endDate,
                         fixingDays, index, gearing, spread,
                         refPeriodStart, refPeriodEnd, dayCounter),
      swapIndex_(index), cap_(cap), floor_(floor), meanReversion_(meanReversion), Pricer_(Pricer) {}

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
            Pricer_->initialize(*this);
            return Pricer_->rate();
    }

   Rate CMSCoupon::rate1() const {
        Date d = fixingDate();
        const Rate Rs = index_->fixing(d);
        Date today = Settings::instance().evaluationDate();
        if (d <= today || gearing_ == 0.0) {
            // the fixing is determined
            Rate r = gearing_*Rs + spread_;
            if (cap_ != Null<Rate>())
                r = std::min(r, cap_);
            if (floor_ != Null<Rate>())
                r = std::max(r, floor_);
            return r;
        } else {
            // a convexity adjustment is required
            QL_REQUIRE(!swaptionVol_.empty(), "missing swaption volatility");

            Rate rate = Rs;
            const DayCounter& dc = dayCounter_;
            Volatility sigma =
                swaptionVol_->volatility(d, index_->tenor(), Rs);
            QL_REQUIRE(sigma > 0.0, "internal error: corrupted volatility");
            Time tau = dc.yearFraction(today,d);
            Schedule s(index_->calendar(),
                       d, d+index_->tenor(),
                       swapIndex_->fixedLegFrequency(),
                       swapIndex_->fixedLegConvention());
            Date tp = date();
            DiscountFactor D_s0 =
                index_->termStructure()->discount(d);
            Real g = G(Rs,tp,D_s0,s,dc), g1 = Gprime(Rs,tp,D_s0,s,dc);
            Spread adjustment = (g1/g)*Rs*Rs*(std::exp(sigma*sigma*tau)-1.0);
            rate += adjustment;

            // translate cap and floor for the coupon to those for the rate.
            // also ensure that both strikes are positive as we use BS.
            Rate capStrike = Null<Rate>(), floorStrike = Null<Rate>();
            if (cap_ != Null<Rate>()) {
                if (gearing_ > 0.0)
                    capStrike = std::max((cap_-spread_)/gearing_,
                                         QL_EPSILON);
                else
                    floorStrike = std::max((cap_-spread_)/gearing_,
                                           QL_EPSILON);
            }
            if (floor_ != Null<Rate>()) {
                if (gearing_ > 0.0)
                    floorStrike = std::max((floor_-spread_)/gearing_,
                                           QL_EPSILON);
                else
                    capStrike = std::max((floor_-spread_)/gearing_,
                                         QL_EPSILON);
            }

            if (capStrike != Null<Rate>()) {
                Rate caplet = detail::blackFormula(Rs, capStrike,
                                                   sigma*std::sqrt(tau), 1.0);
                CumulativeNormalDistribution N;
                Real N32 = N(d_lambda(1.5,Rs,capStrike,sigma,tau));
                Real N12 = N(d_lambda(0.5,Rs,capStrike,sigma,tau));
                Real Nm12 = N(d_lambda(-0.5,Rs,capStrike,sigma,tau));

                Spread adjustment =
                    (g1/g)*(Rs*Rs*std::exp(sigma*sigma*tau)*N32
                            - Rs*(Rs+capStrike)*N12
                            + Rs*capStrike*Nm12);

                caplet += adjustment;
                rate -= caplet;
            }

            if (floorStrike != Null<Rate>()) {
                Rate floorlet = detail::blackFormula(Rs, floorStrike,
                                                     sigma*std::sqrt(tau),
                                                     -1.0);
                CumulativeNormalDistribution N;
                Real N32 = N(-d_lambda(1.5,Rs,floorStrike,sigma,tau));
                Real N12 = N(-d_lambda(0.5,Rs,floorStrike,sigma,tau));
                Real Nm12 = N(-d_lambda(-0.5,Rs,floorStrike,sigma,tau));

                Spread adjustment =
                    -(g1/g)*(Rs*Rs*std::exp(sigma*sigma*tau)*N32
                             - Rs*(Rs+floorStrike)*N12
                             + Rs*floorStrike*Nm12);

                floorlet += adjustment;
                rate += floorlet;
            }
            return gearing_*rate + spread_;
        }
    }

    void CMSCoupon::setSwaptionVolatility(
                   const Handle<SwaptionVolatilityStructure>& vol) {
        if (!swaptionVol_.empty())
            unregisterWith(swaptionVol_);
        swaptionVol_ = vol;
        if (!swaptionVol_.empty())
            registerWith(swaptionVol_);
        notifyObservers();
    }

   Handle<SwaptionVolatilityStructure> CMSCoupon::swaptionVolatility( ) const {
        return swaptionVol_;
   }

    void CMSCoupon::accept(AcyclicVisitor& v) {
        Visitor<CMSCoupon>* v1 = dynamic_cast<Visitor<CMSCoupon>*>(&v);
        if (v1 != 0)
            v1->visit(*this);
        else
            FloatingRateCoupon::accept(v);
    }


    namespace {

        Real get(const std::vector<Real>& v, Size i,
                 Real defaultValue = Null<Real>()) {
            if (v.empty()) {
                return defaultValue;
            } else if (i < v.size()) {
                return v[i];
            } else {
                return v.back();
            }
        }

    }

    std::vector<boost::shared_ptr<CashFlow> >
    CMSCouponVector(const Schedule& schedule,
                    BusinessDayConvention paymentAdjustment,
                    const std::vector<Real>& nominals,
                    const boost::shared_ptr<SwapIndex>& index,
                    Integer fixingDays,
                    const DayCounter& dayCounter,
                    const std::vector<Real>& baseRates,
                    const std::vector<Real>& fractions,
                    const std::vector<Real>& caps,
                    const std::vector<Real>& floors,
                    const std::vector<Real>& meanReversions,
                    const boost::shared_ptr<VanillaCMSCouponPricer>& Pricer,
                    const Handle<SwaptionVolatilityStructure>& vol) {

        //std::vector<CMSCoupon> leg;
        std::vector<boost::shared_ptr<CashFlow> > leg;
        //std::vector<boost::shared_ptr<CashFlow> > legCashFlow;
        Calendar calendar = schedule.calendar();
        Size N = schedule.size();

        QL_REQUIRE(!nominals.empty(), "no nominal given");

        // first period might be short or long
        Date start = schedule.date(0), end = schedule.date(1);
        Date paymentDate = calendar.adjust(end,paymentAdjustment);
        if (schedule.isRegular(1)) {
            leg.push_back(boost::shared_ptr<CashFlow>(
                new CMSCoupon(get(nominals,0), paymentDate, index,
                              start, end, fixingDays, dayCounter, Pricer,
                              get(fractions,0,1.0), get(baseRates,0,0.0),
                              get(caps,0,Null<Rate>()),
                              get(floors,0,Null<Rate>()),
                              get(meanReversions,0,Null<Rate>()),
                              start, end)));

        } else {
            Date reference = end - schedule.tenor();
            reference =
                calendar.adjust(reference,paymentAdjustment);
            leg.push_back(boost::shared_ptr<CashFlow>(
                new CMSCoupon(get(nominals,0), paymentDate, index,
                              start, end, fixingDays, dayCounter, Pricer,
                              get(fractions,0,1.0), get(baseRates,0,0.0),
                              get(caps,0,Null<Rate>()),
                              get(floors,0,Null<Rate>()),
                              get(meanReversions,0,Null<Rate>()),
                              reference, end)));
        }
        // regular periods
        for (Size i=2; i<schedule.size()-1; i++) {
            start = end; end = schedule.date(i);
            paymentDate = calendar.adjust(end,paymentAdjustment);
            leg.push_back(boost::shared_ptr<CashFlow>(
                new CMSCoupon(get(nominals,i-1), paymentDate, index,
                              start, end, fixingDays, dayCounter, Pricer,
                              get(fractions,i-1,1.0), get(baseRates,i-1,0.0),
                              get(caps,i-1,Null<Rate>()),
                              get(floors,i-1,Null<Rate>()),
                              get(meanReversions,i-1,Null<Rate>()),
                              start, end)));
        }
        if (schedule.size() > 2) {
            // last period might be short or long
            start = end; end = schedule.date(N-1);
            paymentDate = calendar.adjust(end,paymentAdjustment);
            if (schedule.isRegular(N-1)) {
                leg.push_back(boost::shared_ptr<CashFlow>(
                    new CMSCoupon(get(nominals,N-2), paymentDate, index,
                                  start, end, fixingDays, dayCounter, Pricer,
                                  get(fractions,N-2,1.0),
                                  get(baseRates,N-2,0.0),
                                  get(caps,N-2,Null<Rate>()),
                                  get(floors,N-2,Null<Rate>()),
                                  get(meanReversions,N-2,Null<Rate>()),
                                  start, end)));
            } else {
                Date reference = start + schedule.tenor();
                reference =
                    calendar.adjust(reference,paymentAdjustment);
                leg.push_back(boost::shared_ptr<CashFlow>(
                    new CMSCoupon(get(nominals,N-2), paymentDate, index,
                                  start, end, fixingDays, dayCounter, Pricer,
                                  get(fractions,N-2,1.0),
                                  get(baseRates,N-2,0.0),
                                  get(caps,N-2,Null<Rate>()),
                                  get(floors,N-2,Null<Rate>()),
                                  get(meanReversions,N-2,Null<Rate>()),
                                  start, reference)));
            }
        }

        for (Size i=0; i<leg.size(); i++) {
            const boost::shared_ptr<CMSCoupon> cmsCoupon =
               boost::dynamic_pointer_cast<CMSCoupon>(leg[i]);
            if (cmsCoupon)
                cmsCoupon->setSwaptionVolatility(vol);
            else
                QL_FAIL("unexpected error when casting to CMSCoupon");
        }

        return leg;
    }

}
