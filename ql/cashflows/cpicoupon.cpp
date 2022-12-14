/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2009 Chris Kenyon
 Copyright (C) 2022 Quaternion Risk Management Ltd

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


#include <ql/cashflows/cashflowvectors.hpp>
#include <ql/cashflows/cpicoupon.hpp>
#include <ql/cashflows/cpicouponpricer.hpp>
#include <ql/cashflows/inflationcoupon.hpp>
#include <ql/time/daycounters/thirty360.hpp>
#include <utility>


namespace QuantLib {

    CPICoupon::CPICoupon(Real baseCPI,
                         const Date& paymentDate,
                         Real nominal,
                         const Date& startDate,
                         const Date& endDate,
                         const ext::shared_ptr<ZeroInflationIndex>& zeroIndex,
                         const Period& observationLag,
                         CPI::InterpolationType observationInterpolation,
                         const DayCounter& dayCounter,
                         Real fixedRate,
                         Spread spread,
                         const Date& refPeriodStart,
                         const Date& refPeriodEnd,
                         const Date& exCouponDate)
    : CPICoupon(baseCPI,
                Null<Date>(),
                paymentDate,
                nominal,
                startDate,
                endDate,
                zeroIndex,
                observationLag,
                observationInterpolation,
                dayCounter,
                fixedRate,
                spread,
                refPeriodStart,
                refPeriodEnd,
                exCouponDate) {}

    CPICoupon::CPICoupon(const Date& baseDate,
                         const Date& paymentDate,
                         Real nominal,
                         const Date& startDate,
                         const Date& endDate,
                         const ext::shared_ptr<ZeroInflationIndex>& zeroIndex,
                         const Period& observationLag,
                         CPI::InterpolationType observationInterpolation,
                         const DayCounter& dayCounter,
                         Real fixedRate,
                         Spread spread,
                         const Date& refPeriodStart,
                         const Date& refPeriodEnd,
                         const Date& exCouponDate)
    : CPICoupon(Null<Real>(),
                baseDate,
                paymentDate,
                nominal,
                startDate,
                endDate,
                zeroIndex,
                observationLag,
                observationInterpolation,
                dayCounter,
                fixedRate,
                spread,
                refPeriodStart,
                refPeriodEnd,
                exCouponDate) {}

    CPICoupon::CPICoupon(Real baseCPI,
                         const Date& baseDate,
                         const Date& paymentDate,
                         Real nominal,
                         const Date& startDate,
                         const Date& endDate,
                         const ext::shared_ptr<ZeroInflationIndex>& zeroIndex,
                         const Period& observationLag,
                         CPI::InterpolationType observationInterpolation,
                         const DayCounter& dayCounter,
                         Real fixedRate,
                         Spread spread,
                         const Date& refPeriodStart,
                         const Date& refPeriodEnd,
                         const Date& exCouponDate)
    : InflationCoupon(paymentDate,
                      nominal,
                      startDate,
                      endDate,
                      0,
                      zeroIndex,
                      observationLag,
                      dayCounter,
                      refPeriodStart,
                      refPeriodEnd,
                      exCouponDate),
      baseCPI_(baseCPI), fixedRate_(fixedRate), spread_(spread),
      observationInterpolation_(observationInterpolation) {

        QL_REQUIRE(index_, "no index provided");
        QL_REQUIRE(
            baseCPI_ != Null<Rate>() || baseDate != Null<Date>(),
            "baseCPI and baseDate can not be both null, provide a valid baseCPI or baseDate");
        QL_REQUIRE(baseCPI_ == Null<Rate>() || std::fabs(baseCPI_) > 1e-16,
                   "|baseCPI_| < 1e-16, future divide-by-zero problem");
    }

    CPICoupon::CPICoupon(Real baseCPI,
                         const Date& paymentDate,
                         Real nominal,
                         const Date& startDate,
                         const Date& endDate,
                         Natural fixingDays,
                         const ext::shared_ptr<ZeroInflationIndex>& zeroIndex,
                         const Period& observationLag,
                         CPI::InterpolationType observationInterpolation,
                         const DayCounter& dayCounter,
                         Real fixedRate,
                         Spread spread,
                         const Date& refPeriodStart,
                         const Date& refPeriodEnd,
                         const Date& exCouponDate)
    : InflationCoupon(paymentDate, nominal, startDate, endDate,
                      fixingDays, zeroIndex, observationLag,
                      dayCounter, refPeriodStart, refPeriodEnd, exCouponDate),
      baseCPI_(baseCPI), fixedRate_(fixedRate), spread_(spread),
      observationInterpolation_(observationInterpolation) {

        QL_REQUIRE(zeroIndex, "no index provided");
        QL_REQUIRE(std::fabs(baseCPI_) > 1e-16,
                   "|baseCPI_| < 1e-16, future divide-by-zero problem");
    }


    void CPICoupon::accept(AcyclicVisitor& v) {
        auto* v1 = dynamic_cast<Visitor<CPICoupon>*>(&v);
        if (v1 != nullptr)
            v1->visit(*this);
        else
            InflationCoupon::accept(v);
    }


    bool CPICoupon::checkPricerImpl(
            const ext::shared_ptr<InflationCouponPricer>&pricer) const {
        return static_cast<bool>(
                        ext::dynamic_pointer_cast<CPICouponPricer>(pricer));
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



    CPICashFlow::CPICashFlow(Real notional,
                             const ext::shared_ptr<ZeroInflationIndex>& index,
                             const Date& baseDate,
                             Real baseFixing,
                             const Date& observationDate,
                             const Period& observationLag,
                             CPI::InterpolationType interpolation,
                             const Date& paymentDate,
                             bool growthOnly)
    : IndexedCashFlow(notional, index, baseDate, observationDate - observationLag, paymentDate, growthOnly),
      baseFixing_(baseFixing), observationDate_(observationDate), observationLag_(observationLag),
      interpolation_(interpolation), frequency_(index ? index->frequency() : NoFrequency) {
        QL_REQUIRE(index, "no index provided");
        QL_REQUIRE(
            baseFixing_ != Null<Rate>() || baseDate != Null<Date>(),
            "baseCPI and baseDate can not be both null, provide a valid baseCPI or baseDate");
        QL_REQUIRE(baseFixing_ == Null<Rate>() || std::fabs(baseFixing_) > 1e-16,
                   "|baseCPI_| < 1e-16, future divide-by-zero problem");
    }

    CPICashFlow::CPICashFlow(Real notional,
                             const ext::shared_ptr<ZeroInflationIndex>& index,
                             const Date& baseDate,
                             Real baseFixing,
                             const Date& fixingDate,
                             const Date& paymentDate,
                             bool growthOnly,
                             CPI::InterpolationType interpolation,
                             const Frequency& frequency)
    : IndexedCashFlow(notional, index, baseDate, fixingDate,
                      paymentDate, growthOnly),
      baseFixing_(baseFixing), interpolation_(interpolation),
      frequency_(frequency) {

        QL_REQUIRE(std::fabs(baseFixing_)>1e-16,
                   "|baseFixing|<1e-16, future divide-by-zero error");
        if (interpolation_ != CPI::AsIndex) {
            QL_REQUIRE(frequency_ != QuantLib::NoFrequency,
                       "non-index interpolation w/o frequency");
        }
    }

    Date CPICashFlow::baseDate() const {
        Date base = IndexedCashFlow::baseDate();
        if (base != Date()) {
            return base;
        } else {
            QL_FAIL("no base date specified");
        }
    }

    Real CPICashFlow::baseFixing() const {
        return baseFixing_;
    }

    Real CPICashFlow::indexFixing() const {
        if (observationDate_ != Date()) {
            return CPI::laggedFixing(cpiIndex(), observationDate_, observationLag_, interpolation_);
        } else {
            // we get to this branch when the deprecated constructor was used; it will be phased out
            return CPI::laggedFixing(cpiIndex(), fixingDate() + observationLag_, observationLag_,
                                     interpolation_);
        }
    }

    CPILeg::CPILeg(const Schedule& schedule,
                   ext::shared_ptr<ZeroInflationIndex> index,
                   const Real baseCPI,
                   const Period& observationLag)
    : schedule_(schedule), index_(std::move(index)), baseCPI_(baseCPI),
      observationLag_(observationLag), paymentDayCounter_(Thirty360(Thirty360::BondBasis)),
      paymentCalendar_(schedule.calendar()),

      spreads_(std::vector<Real>(1, 0)), baseDate_(Null<Date>()) {}


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
        return *this;
    }

    CPILeg& CPILeg::withFixingDays(const std::vector<Natural>& fixingDays) {
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

    CPILeg& CPILeg::withBaseDate(const Date& baseDate) {
        baseDate_ = baseDate;
        return *this;
    }


    CPILeg::operator Leg() const {

        QL_REQUIRE(!notionals_.empty(), "no notional given");
        Size n = schedule_.size()-1;
        Leg leg;
        leg.reserve(n+1);   // +1 for notional, we always have some sort ...
        
        Date baseDate = baseDate_;
        // BaseDate and baseCPI are not given, use the first date as startDate and the baseFixingg
        // should be at startDate - observationLag
        
        if (n>0) {
            QL_REQUIRE(!fixedRates_.empty() || !spreads_.empty(),
                       "no fixedRates or spreads given");
            
            if (baseDate_ == Null<Date>() && baseCPI_ == Null<Real>()) {
                baseDate = schedule_.date(0) - observationLag_;
            }

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
                    leg.push_back(ext::make_shared<FixedRateCoupon>
                                   (paymentDate, detail::get(notionals_, i, 0.0),
                                    detail::effectiveFixedRate(spreads_,caps_,floors_,i),
                                    paymentDayCounter_, start, end, refStart, refEnd, exCouponDate));
                } else { // zero inflation coupon
                    if (detail::noOption(caps_, floors_, i)) { // just swaplet
                        leg.push_back(ext::make_shared<CPICoupon>
                                    (baseCPI_,    // all have same base for ratio
                                     baseDate,
                                     paymentDate,
                                     detail::get(notionals_, i, 0.0),
                                     start, end,
                                     index_, observationLag_,
                                     observationInterpolation_,
                                     paymentDayCounter_,
                                     detail::get(fixedRates_, i, 0.0),
                                     detail::get(spreads_, i, 0.0),
                                     refStart, refEnd, exCouponDate));
                    } else  {     // cap/floorlet
                        QL_FAIL("caps/floors on CPI coupons not implemented.");
                    }
                }
            }
        }

        // in CPI legs you always have a notional flow of some sort
        Date paymentDate = paymentCalendar_.adjust(schedule_.date(n), paymentAdjustment_);
        leg.push_back(ext::make_shared<CPICashFlow>
                          (detail::get(notionals_, n, 0.0), index_,
                           baseDate, baseCPI_, 
                           schedule_.date(n), observationLag_, observationInterpolation_,
                           paymentDate, subtractInflationNominal_));

        // no caps and floors here, so this is enough
        setCouponPricer(leg, ext::make_shared<CPICouponPricer>());

        return leg;
    }

}

