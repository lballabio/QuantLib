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



#include <ql/cashflows/inflationcoupon.hpp>
#include <ql/cashflows/cashflowvectors.hpp>
#include <ql/time/daycounters/thirty360.hpp>

#include <ql/cashflows/cpicoupon.hpp>
#include <ql/cashflows/cpicouponpricer.hpp>


namespace QuantLib {

    CPICoupon::
    CPICoupon(
              Real baseCPI, // user provided
              const Date& paymentDate,
              Real nominal,
              const Date& startDate,
              const Date& endDate,
              Natural fixingDays,
              const boost::shared_ptr<ZeroInflationIndex>& zeroIndex,
              const Period& observationLag,
              CPI::InterpolationType observationInterpolation,
              const DayCounter& dayCounter,
              Real fixedRate, // aka gearing
              Spread spread,
              const Date& refPeriodStart,
              const Date& refPeriodEnd,
              const Date& exCouponDate)
    : InflationCoupon(paymentDate, nominal, startDate, endDate,
                      fixingDays, zeroIndex, observationLag,
                      dayCounter, refPeriodStart, refPeriodEnd, exCouponDate),
        baseCPI_(baseCPI), fixedRate_(fixedRate), spread_(spread),
        observationInterpolation_(observationInterpolation) {

            QL_REQUIRE(std::fabs(baseCPI_) > 1e-16,
                       "|baseCPI_| < 1e-16, future divide-by-zero problem");
        }


    void CPICoupon::accept(AcyclicVisitor& v) {
        Visitor<CPICoupon>* v1 =
        dynamic_cast<Visitor<CPICoupon>*>(&v);
        if (v1 != 0)
            v1->visit(*this);
        else
            InflationCoupon::accept(v);
    }


    bool CPICoupon::checkPricerImpl(
            const boost::shared_ptr<InflationCouponPricer>&pricer) const {
        return static_cast<bool>(
                        boost::dynamic_pointer_cast<CPICouponPricer>(pricer));
    }


    Rate CPICoupon::indexFixing(const Date &d) const {
        // you may want to modify the interpolation of the index
        // this gives you the chance

        Rate I1;
        // what interpolation do we use? Index / flat / linear
        if (observationInterpolation() == CPI::AsIndex) {
            I1 = cpiIndex()->fixing(d);

        } else {
            // work out what it should be
            std::pair<Date,Date> dd = inflationPeriod(d, cpiIndex()->frequency());
            Real indexStart = cpiIndex()->fixing(dd.first);
            if (observationInterpolation() == CPI::Linear) {
                Real indexEnd = cpiIndex()->fixing(dd.second+Period(1,Days));
                // linear interpolation
                I1 = indexStart + (indexEnd - indexStart) * (d - dd.first)
                / (Real)( (dd.second+Period(1,Days)) - dd.first); // can't get to next period's value within current period
            } else {
                // no interpolation, i.e. flat = constant, so use start-of-period value
                I1 = indexStart;
            }

        }
        return I1;
    }




    Date CPICashFlow::baseDate() const {
        // you may not have a valid date
        QL_FAIL("no base date specified");
    }

    Real CPICashFlow::baseFixing() const {
        return baseFixing_;
    }

    Real CPICashFlow::amount() const {
        Real I0 = baseFixing();
        Real I1;

        // what interpolation do we use? Index / flat / linear
        if (interpolation() == CPI::AsIndex ) {
            I1 = index()->fixing(fixingDate());
        } else {
            // work out what it should be
            //std::cout << fixingDate() << " and " << frequency() << std::endl;
            //std::pair<Date,Date> dd = inflationPeriod(fixingDate(), frequency());
            //std::cout << fixingDate() << " and " << dd.first << " " << dd.second << std::endl;
            // work out what it should be
            std::pair<Date,Date> dd = inflationPeriod(fixingDate(), frequency());
            Real indexStart = index()->fixing(dd.first);
            if (interpolation() == CPI::Linear) {
                Real indexEnd = index()->fixing(dd.second+Period(1,Days));
                // linear interpolation
                //std::cout << indexStart << " and " << indexEnd << std::endl;
                I1 = indexStart + (indexEnd - indexStart) * (fixingDate() - dd.first)
                / ( (dd.second+Period(1,Days)) - dd.first); // can't get to next period's value within current period
            } else {
                // no interpolation, i.e. flat = constant, so use start-of-period value
                I1 = indexStart;
            }

        }


        if (growthOnly())
            return notional() * (I1 / I0 - 1.0);
        else
            return notional() * (I1 / I0);
    }


    CPILeg::CPILeg(const Schedule& schedule, const boost::shared_ptr<ZeroInflationIndex>& index,
                   const Real baseCPI, const Period& observationLag) :
    schedule_(schedule), index_(index),
    baseCPI_(baseCPI), observationLag_(observationLag),
    paymentDayCounter_(Thirty360()),
    paymentAdjustment_(ModifiedFollowing),
    paymentCalendar_(schedule.calendar()),
    fixingDays_(std::vector<Natural>(1,0)),
    observationInterpolation_(CPI::AsIndex),
    subtractInflationNominal_(true),
    spreads_(std::vector<Real>(1,0))
    {}


    CPILeg& CPILeg::withObservationInterpolation(CPI::InterpolationType interp) {
        observationInterpolation_ = interp;
        return *this;
    }


    CPILeg& CPILeg::withFixedRates(Real fixedRate) {
        fixedRates_ = std::vector<Real>(1,fixedRate);
        return *this;
    }

    CPILeg& CPILeg::withFixedRates(const std::vector<Real>& fixedRates) {
        fixedRates_ =   fixedRates;
        return *this;
    }

    CPILeg& CPILeg::withNotionals(Real notional) {
        notionals_ = std::vector<Real>(1,notional);
        return *this;
    }

    CPILeg& CPILeg::withNotionals(const std::vector<Real>& notionals) {
        notionals_ = notionals;
        return *this;
    }

    CPILeg& CPILeg::withSubtractInflationNominal(bool growthOnly) {
        subtractInflationNominal_ = growthOnly;
        return *this;
    }

    CPILeg& CPILeg::withPaymentDayCounter(const DayCounter& dayCounter) {
        paymentDayCounter_ = dayCounter;
        return *this;
    }

    CPILeg& CPILeg::withPaymentAdjustment(BusinessDayConvention convention) {
        paymentAdjustment_ = convention;
        return *this;
    }

    CPILeg& CPILeg::withPaymentCalendar(const Calendar& cal) {
        paymentCalendar_ = cal;
        return *this;
    }

    CPILeg& CPILeg::withFixingDays(Natural fixingDays) {
        fixingDays_ = std::vector<Natural>(1,fixingDays);
        return *this;
    }

    CPILeg& CPILeg::withFixingDays(const std::vector<Natural>& fixingDays) {
        fixingDays_ = fixingDays;
        return *this;
    }

    CPILeg& CPILeg::withSpreads(Spread spread) {
        spreads_ = std::vector<Spread>(1,spread);
        return *this;
    }

    CPILeg& CPILeg::withSpreads(const std::vector<Spread>& spreads) {
        spreads_ = spreads;
        return *this;
    }

    CPILeg& CPILeg::withCaps(Rate cap) {
        caps_ = std::vector<Rate>(1,cap);
        return *this;
    }

    CPILeg& CPILeg::withCaps(const std::vector<Rate>& caps) {
        caps_ = caps;
        return *this;
    }

    CPILeg& CPILeg::withFloors(Rate floor) {
        floors_ = std::vector<Rate>(1,floor);
        return *this;
    }

    CPILeg& CPILeg::withFloors(const std::vector<Rate>& floors) {
        floors_ = floors;
        return *this;
    }

    CPILeg& CPILeg::withExCouponPeriod(
                        const Period& period,
                        const Calendar& cal,
                        BusinessDayConvention convention,
                        bool endOfMonth) {
        exCouponPeriod_ = period;
        exCouponCalendar_ = cal;
        exCouponAdjustment_ = convention;
        exCouponEndOfMonth_ = endOfMonth;
        return *this;
    }


    CPILeg::operator Leg() const {

        QL_REQUIRE(!notionals_.empty(), "no notional given");
        Size n = schedule_.size()-1;
        Leg leg;
        leg.reserve(n+1);   // +1 for notional, we always have some sort ...
        if (n>0) {
            QL_REQUIRE(!fixedRates_.empty() || !spreads_.empty(),
                       "no fixedRates or spreads given");

            Date refStart, start, refEnd, end;

            for (Size i=0; i<n; ++i) {
                refStart = start = schedule_.date(i);
                refEnd   =   end = schedule_.date(i+1);
                Date paymentDate = paymentCalendar_.adjust(end, paymentAdjustment_);

                Date exCouponDate;
                if (exCouponPeriod_ != Period())
                {
                    exCouponDate = exCouponCalendar_.advance(paymentDate,
                                                                -exCouponPeriod_,
                                                                exCouponAdjustment_,
                                                                exCouponEndOfMonth_);
                }

                if (i==0   && schedule_.hasIsRegular() && !schedule_.isRegular(i+1)) {
                    BusinessDayConvention bdc = schedule_.businessDayConvention();
                    refStart = schedule_.calendar().adjust(end - schedule_.tenor(), bdc);
                }
                if (i==n-1 && schedule_.hasIsRegular() && !schedule_.isRegular(i+1)) {
                    BusinessDayConvention bdc = schedule_.businessDayConvention();
                    refEnd = schedule_.calendar().adjust(start + schedule_.tenor(), bdc);
                }
                if (detail::get(fixedRates_, i, 1.0) == 0.0) { // fixed coupon
                    leg.push_back(boost::shared_ptr<CashFlow>
                                  (new FixedRateCoupon
                                   (paymentDate, detail::get(notionals_, i, 0.0),
                                    detail::effectiveFixedRate(spreads_,caps_,floors_,i),
                                    paymentDayCounter_, start, end, refStart, refEnd, exCouponDate)));
                } else { // zero inflation coupon
                    if (detail::noOption(caps_, floors_, i)) { // just swaplet
                        boost::shared_ptr<CPICoupon> coup;

                        coup = boost::shared_ptr<CPICoupon>
                            (new CPICoupon(baseCPI_,    // all have same base for ratio
                                     paymentDate,
                                     detail::get(notionals_, i, 0.0),
                                     start, end,
                                     detail::get(fixingDays_, i, 0.0),
                                     index_, observationLag_,
                                     observationInterpolation_,
                                     paymentDayCounter_,
                                     detail::get(fixedRates_, i, 0.0),
                                     detail::get(spreads_, i, 0.0),
                                     refStart, refEnd, exCouponDate));

                        // in this case you can set a pricer
                        // straight away because it only provides computation - not data
                        boost::shared_ptr<CPICouponPricer> pricer
                            (new CPICouponPricer);
                        coup->setPricer(pricer);
                        leg.push_back(boost::dynamic_pointer_cast<CashFlow>(coup));

                    } else  {     // cap/floorlet
                        QL_FAIL("caps/floors on CPI coupons not implemented.");
                    }
                }
            }
        }

        // in CPI legs you always have a notional flow of some sort
        Date paymentDate = paymentCalendar_.adjust(schedule_.date(n), paymentAdjustment_);
        Date fixingDate = paymentDate - observationLag_;
        boost::shared_ptr<CashFlow> xnl(new CPICashFlow
                          (detail::get(notionals_, n, 0.0), index_,
                           Date(), // is fake, i.e. you do not have one
                           baseCPI_, fixingDate, paymentDate,
                           subtractInflationNominal_, observationInterpolation_,
                           index_->frequency())
                         );
        leg.push_back(xnl);


        return leg;
    }






} // namespace RiskLib

