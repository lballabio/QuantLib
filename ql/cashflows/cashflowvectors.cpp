/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Ferdinando Ametrano
 Copyright (C) 2006, 2007 Cristina Duminuco
 Copyright (C) 2006, 2007 Giorgio Facchinetti
 Copyright (C) 2006 Mario Pucci
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl
 Copyright (C) 2003, 2004, 2005 StatPro Italia srl

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
#include <ql/cashflows/fixedratecoupon.hpp>
#include <ql/cashflows/capflooredcoupon.hpp>
#include <ql/cashflows/rangeaccrual.hpp>
#include <ql/indexes/iborindex.hpp>
#include <ql/time/schedule.hpp>

namespace QuantLib {

    namespace {

        Real get(const std::vector<Real>& v,
                 Size i,
                 Real defaultValue) {
            if (v.empty()) {
                return defaultValue;
            } else if (i < v.size()) {
                return v[i];
            } else {
                return v.back();
            }
        }

        Natural get(const std::vector<Natural>& v,
                 Size i,
                 Natural defaultValue) {
            if (v.empty()) {
                return defaultValue;
            } else if (i < v.size()) {
                return v[i];
            } else {
                return v.back();
            }
        }

        Rate effectiveFixedRate(const std::vector<Spread>& spreads,
                                const std::vector<Rate>& caps,
                                const std::vector<Rate>& floors,
                                Size i) {
            Rate result = get(spreads, i, 0.0);
            Rate floor = get(floors, i, Null<Rate>());
            if (floor!=Null<Rate>())
                result = std::max(floor, result);
            Rate cap = get(caps, i, Null<Rate>());
            if (cap!=Null<Rate>())
                result = std::min(cap, result);
            return result;
        }

        bool noOption(const std::vector<Rate>& caps,
                      const std::vector<Rate>& floors,
                      Size i) {
            return (get(caps,   i, Null<Rate>()) == Null<Rate>()) &&
                   (get(floors, i, Null<Rate>()) == Null<Rate>());
        }
    }


    Leg FixedRateLeg(const std::vector<Real>& nominals,
                     const Schedule& schedule,
                     const std::vector<Rate>& couponRates,
                     const DayCounter& paymentDayCounter,
                     BusinessDayConvention paymentAdj,
                     const DayCounter& firstPeriodDayCount) {

        QL_REQUIRE(!couponRates.empty(), "coupon rates not specified");
        QL_REQUIRE(!nominals.empty(), "nominals not specified");

        Leg leg;

        // the following is not always correct
        Calendar calendar = schedule.calendar();

        // first period might be short or long
        Date start = schedule.date(0), end = schedule.date(1);
        Date paymentDate = calendar.adjust(end, paymentAdj);
        Rate rate = couponRates[0];
        Real nominal = nominals[0];
        if (schedule.isRegular(1)) {
            QL_REQUIRE(firstPeriodDayCount.empty() ||
                       firstPeriodDayCount == paymentDayCounter,
                       "regular first coupon "
                       "does not allow a first-period day count");
            leg.push_back(boost::shared_ptr<CashFlow>(new
                FixedRateCoupon(nominal, paymentDate, rate, paymentDayCounter,
                                start, end, start, end)));
        } else {
            Date ref = end - schedule.tenor();
            ref = calendar.adjust(ref,
                                        schedule.businessDayConvention());
            DayCounter dc = firstPeriodDayCount.empty() ?
                            paymentDayCounter :
                            firstPeriodDayCount;
            leg.push_back(boost::shared_ptr<CashFlow>(new
                FixedRateCoupon(nominal, paymentDate, rate,
                                dc, start, end, ref, end)));
        }
        // regular periods
        for (Size i=2; i<schedule.size()-1; ++i) {
            start = end; end = schedule.date(i);
            paymentDate = calendar.adjust(end,paymentAdj);
            if ((i-1) < couponRates.size())
                rate = couponRates[i-1];
            else
                rate = couponRates.back();
            if ((i-1) < nominals.size())
                nominal = nominals[i-1];
            else
                nominal = nominals.back();
            leg.push_back(boost::shared_ptr<CashFlow>(new
                FixedRateCoupon(nominal, paymentDate, rate, paymentDayCounter,
                                start, end, start, end)));
        }
        if (schedule.size() > 2) {
            // last period might be short or long
            Size N = schedule.size();
            start = end; end = schedule.date(N-1);
            paymentDate = calendar.adjust(end,paymentAdj);
            if ((N-2) < couponRates.size())
                rate = couponRates[N-2];
            else
                rate = couponRates.back();
            if ((N-2) < nominals.size())
                nominal = nominals[N-2];
            else
                nominal = nominals.back();
            if (schedule.isRegular(N-1)) {
                leg.push_back(boost::shared_ptr<CashFlow>(new
                    FixedRateCoupon(nominal, paymentDate,
                                    rate, paymentDayCounter,
                                    start, end, start, end)));
            } else {
                Date ref = start + schedule.tenor();
                ref = calendar.adjust(ref,
                                            schedule.businessDayConvention());
                leg.push_back(boost::shared_ptr<CashFlow>(new
                    FixedRateCoupon(nominal, paymentDate,
                                    rate, paymentDayCounter,
                                    start, end, start, ref)));
            }
        }
        return leg;
    }

    template <typename IndexType,
              typename FloatingCouponType,
              typename CappedFlooredCouponType>
    Leg FloatingLeg(const std::vector<Real>& nominals,
                    const Schedule& schedule,
                    const boost::shared_ptr<IndexType>& index,
                    const DayCounter& paymentDayCounter,
                    BusinessDayConvention paymentAdj,
                    const std::vector<Natural>& fixingDays,
                    const std::vector<Real>& gearings,
                    const std::vector<Spread>& spreads,
                    const std::vector<Rate>& caps,
                    const std::vector<Rate>& floors,
                    bool isInArrears) {

        QL_REQUIRE(!nominals.empty(), "no nominal given");

        Size n = schedule.size()-1;
        QL_REQUIRE(nominals.size() <= n,
                   "too many nominals (" << nominals.size() <<
                   "), only " << n << " required");
        QL_REQUIRE(gearings.size()<=n,
                   "too many gearings (" << gearings.size() <<
                   "), only " << n << " required");
        QL_REQUIRE(spreads.size()<=n,
                   "too many spreads (" << spreads.size() <<
                   "), only " << n << " required");
        QL_REQUIRE(caps.size()<=n,
                   "too many caps (" << caps.size() <<
                   "), only " << n << " required");
        QL_REQUIRE(floors.size()<=n,
                   "too many floors (" << floors.size() <<
                   "), only " << n << " required");

        Leg leg; leg.reserve(n);

        // the following is not always correct
        Calendar calendar = schedule.calendar();

        Date refStart, start, refEnd, end;
        Date paymentDate;

        for (Size i=0; i<n; ++i) {
            refStart = start = schedule.date(i);
            refEnd   =   end = schedule.date(i+1);
            paymentDate = calendar.adjust(end, paymentAdj);
            if (i==0   && !schedule.isRegular(i+1))
                refStart = calendar.adjust(end - schedule.tenor(), paymentAdj);
            if (i==n-1 && !schedule.isRegular(i+1))
                refEnd = calendar.adjust(start + schedule.tenor(), paymentAdj);
            if (get(gearings, i, 1.0) == 0.0) { // fixed coupon
                leg.push_back(boost::shared_ptr<CashFlow>(new
                    FixedRateCoupon(get(nominals, i, Null<Real>()), paymentDate,
                                    effectiveFixedRate(spreads, caps, floors, i),
                                    paymentDayCounter,
                                    start, end, refStart, refEnd)));
            } else { // floating coupon
                if (noOption(caps, floors, i))
                    leg.push_back(boost::shared_ptr<CashFlow>(new
                        FloatingCouponType(paymentDate,
                                   get(nominals, i, Null<Real>()),
                                   start, end,
                                   get(fixingDays, i, 2), index,
                                   get(gearings, i, 1.0),
                                   get(spreads, i, 0.0),
                                   refStart, refEnd,
                                   paymentDayCounter, isInArrears)));
                else {
                    leg.push_back(boost::shared_ptr<CashFlow>(new
                        CappedFlooredCouponType(paymentDate,
                                                get(nominals, i, Null<Real>()),
                                                start, end,
                                                get(fixingDays, i, 2), index,
                                                get(gearings, i, 1.0),
                                                get(spreads, i, 0.0),
                                                get(caps,   i, Null<Rate>()),
                                                get(floors, i, Null<Rate>()),
                                                refStart, refEnd,
                                                paymentDayCounter, isInArrears)));
                }
            }
        }
        return leg;
    }

    Leg IborLeg(const std::vector<Real>& nominals,
                const Schedule& schedule,
                const boost::shared_ptr<IborIndex>& index,
                const DayCounter& paymentDayCounter,
                BusinessDayConvention paymentAdj,
                const std::vector<Natural>& fixingDays,
                const std::vector<Real>& gearings,
                const std::vector<Spread>& spreads,
                const std::vector<Rate>& caps,
                const std::vector<Rate>& floors,
                bool isInArrears) {

        Leg cashflows =
            FloatingLeg<IborIndex, IborCoupon, CappedFlooredIborCoupon>(
                nominals,
                schedule,
                index,
                paymentDayCounter,
                paymentAdj,
                fixingDays,
                gearings,
                spreads,
                caps,
                floors,
                isInArrears);

        if (caps.empty() && floors.empty())
            setCouponPricer(cashflows,
                            boost::shared_ptr<FloatingRateCouponPricer>(
                                                  new BlackIborCouponPricer));
        return cashflows;
    }

    Leg CmsLeg(const std::vector<Real>& nominals,
               const Schedule& schedule,
               const boost::shared_ptr<SwapIndex>& index,
               const DayCounter& paymentDayCounter,
               BusinessDayConvention paymentAdj,
               const std::vector<Natural>& fixingDays,
               const std::vector<Real>& gearings,
               const std::vector<Spread>& spreads,
               const std::vector<Rate>& caps,
               const std::vector<Rate>& floors,
               bool isInArrears) {

        return FloatingLeg<SwapIndex, CmsCoupon, CappedFlooredCmsCoupon>(
               nominals,
               schedule,
               index,
               paymentDayCounter,
               paymentAdj,
               fixingDays,
               gearings,
               spreads,
               caps,
               floors,
               isInArrears);
    }

    template <typename IndexType,
              typename FloatingCouponType,
              typename CappedFlooredFloatingCouponType>
    Leg FloatingZeroLeg(const std::vector<Real>& nominals,
                        const Schedule& schedule,
                        const boost::shared_ptr<IndexType>& index,
                        const DayCounter& paymentDayCounter,
                        BusinessDayConvention paymentAdj,
                        const std::vector<Natural>& fixingDays,
                        const std::vector<Real>& gearings,
                        const std::vector<Spread>& spreads,
                        const std::vector<Rate>& caps,
                        const std::vector<Rate>& floors) {

        QL_REQUIRE(!nominals.empty(), "no nominal given");

        Size n = schedule.size()-1;
        QL_REQUIRE(nominals.size() <= n,
                   "too many nominals (" << nominals.size() <<
                   "), only " << n << " required");
        QL_REQUIRE(gearings.size()<=n,
                   "too many gearings (" << gearings.size() <<
                   "), only " << n << " required");
        QL_REQUIRE(spreads.size()<=n,
                   "too many spreads (" << spreads.size() <<
                   "), only " << n << " required");
        QL_REQUIRE(caps.size()<=n,
                   "too many caps (" << caps.size() <<
                   "), only " << n << " required");
        QL_REQUIRE(floors.size()<=n,
                   "too many floors (" << floors.size() <<
                   "), only " << n << " required");

        Leg leg; leg.reserve(n);

        // the following is not always correct
        Calendar calendar = schedule.calendar();

        Date refStart, start, refEnd, end;

        // All payment dates at the end!! in arrears fixing makes no sense
        Date paymentDate = calendar.adjust(schedule.date(n), paymentAdj);
        bool isInArrears = false;

        for (Size i=0; i<n; ++i) {
            refStart = start = schedule.date(i);
            refEnd   =   end = schedule.date(i+1);
            //paymentDate = calendar.adjust(end, paymentAdj);
            if (i==0   && !schedule.isRegular(i+1))
                refStart = calendar.adjust(end - schedule.tenor(), paymentAdj);
            if (i==n-1 && !schedule.isRegular(i+1))
                refEnd = calendar.adjust(start + schedule.tenor(), paymentAdj);
            if (get(gearings, i, 1.0) == 0.0) { // fixed coupon
                leg.push_back(boost::shared_ptr<CashFlow>(new
                    FixedRateCoupon(get(nominals, i, Null<Real>()), paymentDate,
                                    effectiveFixedRate(spreads, caps, floors, i),
                                    paymentDayCounter,
                                    start, end, refStart, refEnd)));
            } else { // floating coupon
                if (noOption(caps, floors, i)) {
                    leg.push_back(boost::shared_ptr<CashFlow>(new
                        FloatingCouponType(paymentDate, // diff
                                           get(nominals, i, Null<Real>()),
                                           start, end,
                                           get(fixingDays, i, 2), index,
                                           get(gearings, i, 1.0), get(spreads, i, 0.0),
                                           refStart, refEnd,
                                           paymentDayCounter, isInArrears)));
                } else {
                    leg.push_back(boost::shared_ptr<CashFlow>(new
                        CappedFlooredFloatingCouponType(paymentDate, // diff
                                           get(nominals, i, Null<Real>()),
                                           start, end,
                                           get(fixingDays, i, 2), index,
                                           get(gearings, i, 1.0), get(spreads, i, 0.0),
                                           get(caps,   i, Null<Rate>()),
                                           get(floors, i, Null<Rate>()),
                                           refStart, refEnd,
                                           paymentDayCounter, isInArrears)));
                }
            }
        }
         return leg;
    }

    Leg IborZeroLeg(const std::vector<Real>& nominals,
                    const Schedule& schedule,
                    const boost::shared_ptr<IborIndex>& index,
                    const DayCounter& paymentDayCounter,
                    BusinessDayConvention paymentAdj,
                    const std::vector<Natural>& fixingDays,
                    const std::vector<Real>& gearings,
                    const std::vector<Spread>& spreads,
                    const std::vector<Rate>& caps,
                    const std::vector<Rate>& floors) {

        return FloatingZeroLeg <IborIndex, IborCoupon, CappedFlooredIborCoupon>(
                   nominals,
                   schedule,
                   index,
                   paymentDayCounter,
                   paymentAdj,
                   fixingDays,
                   gearings,
                   spreads,
                   caps,
                   floors);
    }

    Leg CmsZeroLeg(const std::vector<Real>& nominals,
                   const Schedule& schedule,
                   const boost::shared_ptr<SwapIndex>& index,
                   const DayCounter& paymentDayCounter,
                   BusinessDayConvention paymentAdj,
                   const std::vector<Natural>& fixingDays,
                   const std::vector<Real>& gearings,
                   const std::vector<Spread>& spreads,
                   const std::vector<Rate>& caps,
                   const std::vector<Rate>& floors) {

        return FloatingZeroLeg <SwapIndex, CmsCoupon, CappedFlooredCmsCoupon>(
                   nominals,
                   schedule,
                   index,
                   paymentDayCounter,
                   paymentAdj,
                   fixingDays,
                   gearings,
                   spreads,
                   caps,
                   floors);
    }

    Leg RangeAccrualLeg(const std::vector<Real>& nominals,
                       const Schedule& schedule,
                       const boost::shared_ptr<IborIndex>& index,
                       const DayCounter& paymentDayCounter,
                       BusinessDayConvention paymentConvention,
                       const std::vector<Natural>& fixingDays,
                       const std::vector<Real>& gearings,
                       const std::vector<Spread>& spreads,
                       const std::vector<Rate>& lowerTriggers,
                       const std::vector<Rate>& upperTriggers,
                       const Period& observationTenor,
                       BusinessDayConvention observationConvention) {

        QL_REQUIRE(!nominals.empty(), "no nominal given");

        Size n = schedule.size()-1;
        QL_REQUIRE(nominals.size() <= n,
                   "too many nominals (" << nominals.size() <<
                   "), only " << n << " required");
        QL_REQUIRE(fixingDays.size() <= n,
                   "too many fixingDays (" << fixingDays.size() <<
                   "), only " << n << " required");
        QL_REQUIRE(gearings.size()<=n,
                   "too many gearings (" << gearings.size() <<
                   "), only " << n << " required");
        QL_REQUIRE(spreads.size()<=n,
                   "too many spreads (" << spreads.size() <<
                   "), only " << n << " required");
        QL_REQUIRE(lowerTriggers.size()<=n,
                   "too many lowerTriggers (" << lowerTriggers.size() <<
                   "), only " << n << " required");
        QL_REQUIRE(upperTriggers.size()<=n,
                   "too many upperTriggers (" << upperTriggers.size() <<
                   "), only " << n << " required");

        Leg leg; leg.reserve(n);

        // the following is not always correct
        Calendar calendar = schedule.calendar();

        Date refStart, start, refEnd, end;
        Date paymentDate;
        std::vector<boost::shared_ptr<Schedule> > observationsSchedules;

        for (Size i=0; i<n; ++i) {
            refStart = start = schedule.date(i);
            refEnd   =   end = schedule.date(i+1);
            paymentDate = calendar.adjust(end, paymentConvention);
            if (i==0   && !schedule.isRegular(i+1))
                refStart = calendar.adjust(end - schedule.tenor(), paymentConvention);
            if (i==n-1 && !schedule.isRegular(i+1))
                refEnd = calendar.adjust(start + schedule.tenor(), paymentConvention);
            if (get(gearings, i, 1.0) == 0.0) { // fixed coupon
                leg.push_back(boost::shared_ptr<CashFlow>(new
                    FixedRateCoupon(get(nominals, i, Null<Real>()), paymentDate,
                                    get(spreads, i, 0.0),
                                    paymentDayCounter,
                                    start, end, refStart, refEnd)));
            } else { // floating coupon
                    observationsSchedules.push_back( boost::shared_ptr<Schedule>( new
                          Schedule(start, end,
                                  observationTenor, calendar,
                                  observationConvention,
                                  observationConvention,
                                  false, false)));

                    leg.push_back(boost::shared_ptr<CashFlow>(new
                       RangeAccrualFloatersCoupon(
                            get(nominals, i, Null<Real>()),
                            paymentDate,
                            index,
                            start, end,
                            get(fixingDays, i, 2),
                            paymentDayCounter,
                            get(gearings, i, 1.0),
                            get(spreads, i, 0.0),
                            refStart, refEnd,
                            observationsSchedules.back(),
                            get(lowerTriggers, i, Null<Rate>()),
                            get(upperTriggers, i, Null<Rate>()))));
            }
        }
        return leg;
    }

    template <typename IndexType,
              typename FloatingCouponType,
              typename DigitalCouponType>
    Leg FloatingDigitalLeg(const std::vector<Real>& nominals,
                   const Schedule& schedule,
                   const boost::shared_ptr<IndexType>& index,
                   const DayCounter& paymentDayCounter,
                   BusinessDayConvention paymentAdj,
                   const std::vector<Natural>& fixingDays,
                   const std::vector<Real>& gearings,
                   const std::vector<Spread>& spreads,
                   bool isInArrears,
                   const std::vector<Rate>& callStrikes,
                   Position::Type callPosition,
                   bool isCallATMIncluded,
                   const std::vector<Rate>& callDigitalPayoffs,
                   const std::vector<Rate>& putStrikes,
                   Position::Type putPosition,
                   bool isPutATMIncluded,
                   const std::vector<Rate>& putDigitalPayoffs,
                   Replication::Type replication,
                   Real eps) {

        QL_REQUIRE(!nominals.empty(), "no nominal given");

        Size n = schedule.size()-1;
        QL_REQUIRE(nominals.size() <= n,
                   "too many nominals (" << nominals.size() <<
                   "), only " << n << " required");
        QL_REQUIRE(gearings.size()<=n,
                   "too many gearings (" << gearings.size() <<
                   "), only " << n << " required");
        QL_REQUIRE(spreads.size()<=n,
                   "too many spreads (" << spreads.size() <<
                   "), only " << n << " required");
        QL_REQUIRE(callStrikes.size()<=n,
                   "too many call rates (" << callStrikes.size() <<
                   "), only " << n << " required");
        QL_REQUIRE(putStrikes.size()<=n,
                   "too many put rates (" << putStrikes.size() <<
                   "), only " << n << " required");

        Leg leg; leg.reserve(n);

        // the following is not always correct
        Calendar calendar = schedule.calendar();

        Date refStart, start, refEnd, end;
        Date paymentDate;

        for (Size i=0; i<n; ++i) {
            refStart = start = schedule.date(i);
            refEnd   =   end = schedule.date(i+1);
            paymentDate = calendar.adjust(end, paymentAdj);
            if (i==0   && !schedule.isRegular(i+1))
                refStart = calendar.adjust(end - schedule.tenor(), paymentAdj);
            if (i==n-1 && !schedule.isRegular(i+1))
                refEnd = calendar.adjust(start + schedule.tenor(), paymentAdj);
            if (get(gearings, i, 1.0) == 0.0) { // fixed coupon
                leg.push_back(boost::shared_ptr<CashFlow>(new
                    FixedRateCoupon(get(nominals, i, Null<Real>()), paymentDate,
                                    get(spreads, i, 1.0),
                                    paymentDayCounter,
                                    start, end, refStart, refEnd)));
            } else { // floating digital coupon
                boost::shared_ptr<FloatingCouponType> underlying =
                    (boost::shared_ptr<FloatingCouponType>)(new
                        FloatingCouponType(paymentDate,
                                       get(nominals, i, Null<Real>()),
                                       start, end,
                                       get(fixingDays, i, 2), index,
                                       get(gearings, i, 1.0),
                                       get(spreads, i, 0.0),
                                       refStart, refEnd,
                                       paymentDayCounter, isInArrears));
                leg.push_back(boost::shared_ptr<CashFlow>(new
                    DigitalCouponType( underlying,
                                       get(callStrikes, i, Null<Real>()),
                                       callPosition,
                                       isCallATMIncluded,
                                       get(callDigitalPayoffs, i, Null<Real>()),
                                       get(putStrikes, i, Null<Real>()),
                                       putPosition,
                                       isPutATMIncluded,
                                       get(putDigitalPayoffs, i, Null<Real>()),
                                       replication,
                                       eps)));
            }
        }
        return leg;
    }

    Leg DigitalIborLeg(const std::vector<Real>& nominals,
                const Schedule& schedule,
                const boost::shared_ptr<IborIndex>& index,
                const DayCounter& paymentDayCounter,
                const BusinessDayConvention paymentConvention,
                const std::vector<Natural>& fixingDays,
                const std::vector<Real>& gearings,
                const std::vector<Spread>& spreads,
                bool isInArrears,
                const std::vector<Rate>& callStrikes,
                Position::Type longCallOption,
                bool isCallATMIncluded,
                const std::vector<Rate>& callDigitalPayoffs,
                const std::vector<Rate>& putStrikes,
                Position::Type longPutOption,
                bool isPutATMIncluded,
                const std::vector<Rate>& putDigitalPayoffs,
                Replication::Type replication,
                Real eps) {

        return FloatingDigitalLeg<IborIndex, IborCoupon, DigitalIborCoupon>(
               nominals,
               schedule,
               index,
               paymentDayCounter,
               paymentConvention,
               fixingDays,
               gearings,
               spreads,
               isInArrears,
               callStrikes,
               longCallOption,
               isCallATMIncluded,
               callDigitalPayoffs,
               putStrikes,
               longPutOption,
               isPutATMIncluded,
               putDigitalPayoffs,
               replication,
               eps);    }

    Leg DigitalCmsLeg(const std::vector<Real>& nominals,
                const Schedule& schedule,
                const boost::shared_ptr<SwapIndex>& index,
                const DayCounter& paymentDayCounter,
                const BusinessDayConvention paymentConvention,
                const std::vector<Natural>& fixingDays,
                const std::vector<Real>& gearings,
                const std::vector<Spread>& spreads,
                bool isInArrears,
                const std::vector<Rate>& callStrikes,
                Position::Type callPosition,
                bool isCallATMIncluded,
                const std::vector<Rate>& callDigitalPayoffs,
                const std::vector<Rate>& putStrikes,
                Position::Type putPosition,
                bool isPutATMIncluded,
                const std::vector<Rate>& putDigitalPayoffs,
                Replication::Type replication,
                Real eps) {

        return FloatingDigitalLeg<SwapIndex, CmsCoupon, DigitalCmsCoupon>(
               nominals,
               schedule,
               index,
               paymentDayCounter,
               paymentConvention,
               fixingDays,
               gearings,
               spreads,
               isInArrears,
               callStrikes,
               callPosition,
               isCallATMIncluded,
               callDigitalPayoffs,
               putStrikes,
               putPosition,
               isPutATMIncluded,
               putDigitalPayoffs,
               replication,
               eps);
    }

}
