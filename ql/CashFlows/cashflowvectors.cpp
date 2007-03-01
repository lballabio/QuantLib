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
 <http://quantlib.org/reference/license.html>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/
#include <ql/CashFlows/cashflowvectors.hpp>
#include <ql/CashFlows/fixedratecoupon.hpp>
#include <ql/CashFlows/shortfloatingcoupon.hpp>
#include <ql/CashFlows/capflooredcoupon.hpp>


namespace QuantLib {

    namespace {

        Real get(const std::vector<Real>& v, Size i,
                 Real defaultValue) {
            if (v.empty()) {
                return defaultValue;
            } else if (i < v.size()) {
                return v[i];
            } else {
                return v.back();
            }
        }

    }


    Leg FixedRateLeg(const Schedule& schedule,
                     const std::vector<Real>& nominals,
                     const std::vector<Rate>& couponRates,
                     const DayCounter& paymentDayCounter,
                     BusinessDayConvention paymentAdjustment,
                     const DayCounter& firstPeriodDayCount) {

        QL_REQUIRE(!couponRates.empty(), "coupon rates not specified");
        QL_REQUIRE(!nominals.empty(), "nominals not specified");

        Leg leg;
        Calendar calendar = schedule.calendar();

        // first period might be short or long
        Date start = schedule.date(0), end = schedule.date(1);
        Date paymentDate = calendar.adjust(end,paymentAdjustment);
        Rate rate = couponRates[0];
        Real nominal = nominals[0];
        if (schedule.isRegular(1)) {
            QL_REQUIRE(firstPeriodDayCount.empty() ||
                       firstPeriodDayCount == paymentDayCounter,
                       "regular first coupon "
                       "does not allow a first-period day count");
            leg.push_back(boost::shared_ptr<CashFlow>(
                new FixedRateCoupon(nominal, paymentDate, rate, paymentDayCounter,
                                    start, end, start, end)));
        } else {
            Date reference = end - schedule.tenor();
            reference = calendar.adjust(reference,
                                        schedule.businessDayConvention());
            DayCounter dc = firstPeriodDayCount.empty() ?
                            paymentDayCounter :
                            firstPeriodDayCount;
            leg.push_back(boost::shared_ptr<CashFlow>(
                new FixedRateCoupon(nominal, paymentDate, rate,
                                    dc, start, end, reference, end)));
        }
        // regular periods
        for (Size i=2; i<schedule.size()-1; ++i) {
            start = end; end = schedule.date(i);
            paymentDate = calendar.adjust(end,paymentAdjustment);
            if ((i-1) < couponRates.size())
                rate = couponRates[i-1];
            else
                rate = couponRates.back();
            if ((i-1) < nominals.size())
                nominal = nominals[i-1];
            else
                nominal = nominals.back();
            leg.push_back(boost::shared_ptr<CashFlow>(
                new FixedRateCoupon(nominal, paymentDate, rate, paymentDayCounter,
                                    start, end, start, end)));
        }
        if (schedule.size() > 2) {
            // last period might be short or long
            Size N = schedule.size();
            start = end; end = schedule.date(N-1);
            paymentDate = calendar.adjust(end,paymentAdjustment);
            if ((N-2) < couponRates.size())
                rate = couponRates[N-2];
            else
                rate = couponRates.back();
            if ((N-2) < nominals.size())
                nominal = nominals[N-2];
            else
                nominal = nominals.back();
            if (schedule.isRegular(N-1)) {
                leg.push_back(boost::shared_ptr<CashFlow>(
                    new FixedRateCoupon(nominal, paymentDate,
                                        rate, paymentDayCounter,
                                        start, end, start, end)));
            } else {
                Date reference = start + schedule.tenor();
                reference = calendar.adjust(reference,
                                            schedule.businessDayConvention());
                leg.push_back(boost::shared_ptr<CashFlow>(
                    new FixedRateCoupon(nominal, paymentDate,
                                        rate, paymentDayCounter,
                                        start, end, start, reference)));
            }
        }
        return leg;
    }


    Leg IborLeg(const Schedule& schedule,
                const std::vector<Real>& nominals,
                const boost::shared_ptr<IborIndex>& index,
                const DayCounter& paymentDayCounter,
                Integer fixingDays,
                BusinessDayConvention paymentAdjustment,
                const std::vector<Real>& gearings,
                const std::vector<Spread>& spreads,
                const std::vector<Rate>& caps,
                const std::vector<Rate>& floors) {

            QL_REQUIRE(!nominals.empty(), "no nominal given");
            bool hasEmbeddedOption = ( caps.empty() && floors.empty() ) ? false : true ;

            Leg leg;
            Calendar calendar = schedule.calendar();
            Size N = schedule.size();

            if (!hasEmbeddedOption) {

                // first period might be short or long
                Date start = schedule.date(0), end = schedule.date(1);
                Date paymentDate = calendar.adjust(end,paymentAdjustment);
                if (schedule.isRegular(1)) {
                    if (get(gearings,0,1.0)!=0.0) {
                        leg.push_back(boost::shared_ptr<CashFlow>(new
                            IborCoupon(paymentDate,
                                       get(nominals, 0, Null<Real>()),
                                       start, end, fixingDays, index,
                                       get(gearings, 0, 1.0),
                                       get(spreads, 0, 0.0),
                                       Date(), Date(), paymentDayCounter)));
                    } else {
                        // if gearing is null a fixed rate coupon with rate equal to
                        // margin is constructed
                        leg.push_back(boost::shared_ptr<CashFlow>(new
                            FixedRateCoupon(get(nominals,0, Null<Real>()), paymentDate,
                                        get(spreads,0,0.0), paymentDayCounter,
                                        start, end, start, end)));
                    }
                } else {
                    Date reference = end - schedule.tenor();
                    reference = calendar.adjust(reference,paymentAdjustment);
                    if (get(gearings,0,1.0)!=0.0) {
                        leg.push_back(boost::shared_ptr<CashFlow>(new
                            IborCoupon(paymentDate, get(nominals,0, Null<Real>()),
                                       start, end, fixingDays, index,
                                       get(gearings,0,1.0),
                                       get(spreads,0,0.0),
                                       reference, end, paymentDayCounter)));
                    } else {
                        // if gearing is null a fixed rate coupon with rate equal to
                        // margin is constructed
                        leg.push_back(boost::shared_ptr<CashFlow>(new
                            FixedRateCoupon(get(nominals,0, Null<Real>()), paymentDate,
                                        get(spreads,0,0.0), paymentDayCounter,
                                        start, end, reference, end)));
                    }
                }
                // regular periods
                for (Size i=2; i<schedule.size()-1; ++i) {
                    start = end; end = schedule.date(i);
                    paymentDate = calendar.adjust(end,paymentAdjustment);
                    if (get(gearings,i-1,1.0)!=0.0) {
                        leg.push_back(boost::shared_ptr<CashFlow>(new
                            IborCoupon(paymentDate, get(nominals,i-1, Null<Real>()),
                                       start, end, fixingDays, index,
                                       get(gearings,i-1,1.0),
                                       get(spreads,i-1,0.0),
                                       Date(),Date(), paymentDayCounter)));
                    } else {
                        // if gearing is null a fixed rate coupon with rate equal to
                        // margin is constructed
                        leg.push_back(boost::shared_ptr<CashFlow>(new
                            FixedRateCoupon(get(nominals,i-1, Null<Real>()), paymentDate,
                                        get(spreads,i-1,0.0), paymentDayCounter,
                                        start, end, start, end)));
                    }
                }
                if (schedule.size() > 2) {
                    // last period might be short or long
                    start = end; end = schedule.date(N-1);
                    paymentDate = calendar.adjust(end,paymentAdjustment);
                    if (schedule.isRegular(N-1)) {
                        if (get(gearings,N-2,1.0)!=0.0) {
                            leg.push_back(boost::shared_ptr<CashFlow>(new
                                IborCoupon(paymentDate, get(nominals,N-2, Null<Real>()),
                                           start, end, fixingDays, index,
                                           get(gearings,N-2,1.0),
                                           get(spreads,N-2,0.0),
                                           Date(),Date(),
                                           paymentDayCounter)));
                        } else {
                            // if gearing is null a fixed rate coupon with rate equal to
                            // margin is constructed
                            leg.push_back(boost::shared_ptr<CashFlow>(new
                                FixedRateCoupon(get(nominals,N-2, Null<Real>()), paymentDate,
                                                get(spreads,N-2,0.0), paymentDayCounter,
                                                start, end, start, end)));
                        }
                    } else {
                        Date reference = start + schedule.tenor();
                        reference = calendar.adjust(reference,paymentAdjustment);
                        if (get(gearings,N-2,1.0)!=0.0) {
                            leg.push_back(boost::shared_ptr<CashFlow>(new
                                IborCoupon(paymentDate, get(nominals,N-2, Null<Real>()),
                                           start, end, fixingDays, index,
                                           get(gearings,N-2,1.0),
                                           get(spreads,N-2,0.0),
                                           start, reference,
                                           paymentDayCounter)));
                        } else {
                            // if gearing is null a fixed rate coupon with rate equal to
                            // margin is constructed
                            leg.push_back(boost::shared_ptr<CashFlow>(new
                                FixedRateCoupon(get(nominals,N-2, Null<Real>()), paymentDate,
                                                get(spreads,N-2,0.0), paymentDayCounter,
                                                start, end, start, reference)));
                        }
                    }
                }
            } else {
                // Not all coupons have embedded options

                // first period might be short or long
                Date start = schedule.date(0), end = schedule.date(1);
                Date paymentDate = calendar.adjust(end,paymentAdjustment);
                if (schedule.isRegular(1)) {
                    if (get(gearings,0,1.0)!=0.0) {
                            leg.push_back(boost::shared_ptr<CashFlow>(new
                                CappedFlooredIborCoupon(paymentDate, get(nominals,0, Null<Real>()),
                                        start, end, fixingDays, index,
                                        get(gearings,0,1.0),
                                        get(spreads,0,0.0),
                                        get(caps,0,Null<Rate>()),
                                        get(floors,0,Null<Rate>()),
                                        Date(),Date(), paymentDayCounter)));
                    } else {
                        // if gearing is null a fixed rate coupon with rate equal to
                        // Min [caprate, Max[floorrate, spread] ] is constructed
                        Rate effectiveRate = get(spreads, 0, 0.0);
                        if (get(floors, 0, Null<Rate>())!=Null<Rate>())
                            effectiveRate = std::max(get(floors, 0, Null<Rate>()), effectiveRate);
                        if (get(caps, 0, Null<Rate>())!=Null<Rate>())
                            effectiveRate = std::min(get(caps, 0, Null<Rate>()), effectiveRate);
                        leg.push_back(boost::shared_ptr<CashFlow>(new
                            FixedRateCoupon(get(nominals,0, Null<Real>()), paymentDate,
                                            effectiveRate, paymentDayCounter,
                                            start, end, start, end)));
                    }
                } else {
                    Date reference = end - schedule.tenor();
                    reference = calendar.adjust(reference,paymentAdjustment);
                    if (get(gearings,0,1.0)!=0.0) {
                        leg.push_back(boost::shared_ptr<CashFlow>(new
                            CappedFlooredIborCoupon(paymentDate, get(nominals,0, Null<Real>()),
                                        start, end, fixingDays, index,
                                        get(gearings,0,1.0),
                                        get(spreads,0,0.0),
                                        get(caps,0,Null<Rate>()),
                                        get(floors,0,Null<Rate>()),
                                        reference, end, paymentDayCounter)));
                    } else {
                        // if gearing is null a fixed rate coupon with rate equal to
                        // Min [caprate, Max[floorrate, spread] ] is constructed
                        Rate effectiveRate = get(spreads, 0, 0.0);
                        if (get(floors, 0, Null<Rate>())!=Null<Rate>())
                            effectiveRate = std::max(get(floors, 0, Null<Rate>()), effectiveRate);
                        if (get(caps, 0, Null<Rate>())!=Null<Rate>())
                            effectiveRate = std::min(get(caps, 0, Null<Rate>()), effectiveRate);
                        leg.push_back(boost::shared_ptr<CashFlow>(new
                            FixedRateCoupon(get(nominals, 0, Null<Real>()), paymentDate,
                                            effectiveRate, paymentDayCounter,
                                            start, end, reference, end)));
                    }
                }
                // regular periods
                for (Size i=2; i<schedule.size()-1; ++i) {
                    start = end; end = schedule.date(i);
                    paymentDate = calendar.adjust(end,paymentAdjustment);
                    if (get(gearings,i-1,1.0)!=0.0) {
                        leg.push_back(boost::shared_ptr<CashFlow>(new
                            CappedFlooredIborCoupon(paymentDate, get(nominals,i-1, Null<Real>()),
                                        start, end, fixingDays, index,
                                        get(gearings,i-1,1.0),
                                        get(spreads,i-1,0.0),
                                        get(caps,i-1,Null<Rate>()),
                                        get(floors,i-1,Null<Rate>()),
                                        Date(),Date(), paymentDayCounter)));
                    } else {
                        // if gearing is null a fixed rate coupon with rate equal to
                        // Min [caprate, Max[floorrate, spread] ] is constructed
                        Rate effectiveRate = get(spreads, i-1, 0.0);
                        if (get(floors, i-1, Null<Rate>())!=Null<Rate>())
                            effectiveRate = std::max(get(floors, i-1, Null<Rate>()), effectiveRate);
                        if (get(caps, i-1, Null<Rate>())!=Null<Rate>())
                            effectiveRate = std::min(get(caps, i-1, Null<Rate>()), effectiveRate);
                        leg.push_back(boost::shared_ptr<CashFlow>(new
                            FixedRateCoupon(get(nominals,i-1, Null<Real>()), paymentDate,
                                            effectiveRate, paymentDayCounter,
                                            start, end, start, end)));
                    }
                }
                if (schedule.size() > 2) {
                    // last period might be short or long
                    start = end; end = schedule.date(N-1);
                    paymentDate = calendar.adjust(end,paymentAdjustment);
                    if (schedule.isRegular(N-1)) {
                        if (get(gearings,N-2,1.0)!=0.0) {
                            leg.push_back(boost::shared_ptr<CashFlow>(new
                                CappedFlooredIborCoupon(paymentDate, get(nominals,N-2, Null<Real>()),
                                            start, end, fixingDays, index,
                                            get(gearings,N-2,1.0),
                                            get(spreads,N-2,0.0),
                                            get(caps,N-2,Null<Rate>()),
                                            get(floors,N-2,Null<Rate>()),
                                            Date(),Date(), paymentDayCounter)));
                        } else {
                            // if gearing is null a fixed rate coupon with rate equal to
                            // Min [caprate, Max[floorrate, spread] ] is constructed
                            Rate effectiveRate = get(spreads, N-2, 0.0);
                            if (get(floors, N-2, Null<Rate>())!=Null<Rate>())
                                effectiveRate = std::max(get(floors, N-2, Null<Rate>()), effectiveRate);
                            if (get(caps, N-2, Null<Rate>())!=Null<Rate>())
                                effectiveRate = std::min(get(caps, N-2, Null<Rate>()), effectiveRate);
                            leg.push_back(boost::shared_ptr<CashFlow>(new
                                FixedRateCoupon(get(nominals,N-2, Null<Real>()), paymentDate,
                                                effectiveRate, paymentDayCounter,
                                                start, end, start, end)));
                        }
                    } else {
                        Date reference = start + schedule.tenor();
                        reference = calendar.adjust(reference,paymentAdjustment);
                        if (get(gearings,N-2,1.0)!=0.0) {
                            leg.push_back(boost::shared_ptr<CashFlow>(new
                                CappedFlooredIborCoupon(paymentDate, get(nominals,N-2, Null<Real>()),
                                            start, end, fixingDays, index,
                                            get(gearings,N-2,1.0),
                                            get(spreads,N-2,0.0),
                                            get(caps,N-2,Null<Rate>()),
                                            get(floors,N-2,Null<Rate>()),
                                            start, reference, paymentDayCounter)));
                        } else {
                            // if gearing is null a fixed rate coupon with rate equal to
                            // Min [caprate, Max[floorrate, spread] ] is constructed
                            Rate effectiveRate = get(spreads, N-2, 0.0);
                            if (get(floors, N-2, Null<Rate>())!=Null<Rate>())
                                effectiveRate = std::max(get(floors, N-2, Null<Rate>()), effectiveRate);
                            if (get(caps, N-2, Null<Rate>())!=Null<Rate>())
                                effectiveRate = std::min(get(caps, N-2, Null<Rate>()), effectiveRate);
                            leg.push_back(boost::shared_ptr<CashFlow>(new
                                FixedRateCoupon(get(nominals,N-2, Null<Real>()), paymentDate,
                                                effectiveRate, paymentDayCounter,
                                                start, end, start, reference)));
                        }
                    }
                }
            }
            return leg;
        }


    Leg IborInArrearsLeg(const Schedule& schedule,
                         const std::vector<Real>& nominals,
                         const boost::shared_ptr<IborIndex>& index,
                         const DayCounter& paymentDayCounter,
                         Integer fixingDays,
                         BusinessDayConvention paymentAdjustment,
                         const std::vector<Real>& gearings,
                         const std::vector<Spread>& spreads,
                         const std::vector<Rate>& caps,
                         const std::vector<Rate>& floors) {

            QL_REQUIRE(!nominals.empty(), "no nominal given");
            bool hasEmbeddedOption = ( caps.empty() && floors.empty() ) ? false : true ;

            Leg leg;
            Calendar calendar = schedule.calendar();
            Size N = schedule.size();

            if (!hasEmbeddedOption) {

                // first period might be short or long
                Date start = schedule.date(0), end = schedule.date(1);
                Date paymentDate = calendar.adjust(end,paymentAdjustment);
                if (schedule.isRegular(1)) {
                    leg.push_back(boost::shared_ptr<CashFlow>(
                        new IborCoupon(paymentDate, get(nominals,0, Null<Real>()), start, end,
                                                    fixingDays, index, get(gearings,0,1.0),
                                                    get(spreads,0,0.0),
                                                    Date(),Date(),paymentDayCounter,true)));
                } else {
                    Date reference = end - schedule.tenor();
                    reference = calendar.adjust(reference,paymentAdjustment);
                    leg.push_back(boost::shared_ptr<CashFlow>(
                        new IborCoupon(paymentDate, get(nominals,0, Null<Real>()), start, end,
                                                    fixingDays, index, get(gearings,0,1.0),
                                                    get(spreads,0,0.0),
                                                    Date(),Date(),paymentDayCounter,true)));
                }
                // regular periods
                for (Size i=2; i<schedule.size()-1; ++i) {
                    start = end; end = schedule.date(i);
                    paymentDate = calendar.adjust(end,paymentAdjustment);
                    leg.push_back(boost::shared_ptr<CashFlow>(
                        new IborCoupon(paymentDate, get(nominals,i-1, Null<Real>()), start, end,
                                                    fixingDays, index, get(gearings,i-1,1.0),
                                                    get(spreads,i-1,0.0),
                                                    Date(),Date(),paymentDayCounter,true)));
                }
                if (schedule.size() > 2) {
                    // last period might be short or long
                    start = end; end = schedule.date(N-1);
                    paymentDate = calendar.adjust(end,paymentAdjustment);
                    if (schedule.isRegular(N-1)) {
                       leg.push_back(boost::shared_ptr<CashFlow>(
                            new IborCoupon(paymentDate, get(nominals,N-2, Null<Real>()), start, end,
                                                        fixingDays, index, get(gearings,N-2,1.0),
                                                        get(spreads,N-2,0.0),
                                                        Date(),Date(),paymentDayCounter,true)));
                    } else {
                        Date reference = start + schedule.tenor();
                        reference =
                            calendar.adjust(reference,paymentAdjustment);
                           leg.push_back(boost::shared_ptr<CashFlow>(
                                new IborCoupon(paymentDate, get(nominals,N-2, Null<Real>()), start, end,
                                                            fixingDays, index, get(gearings,N-2,1.0),
                                                            get(spreads,N-2,0.0),
                                                            start,reference,paymentDayCounter,true)));
                    }
                }
            } else {

                // first period might be short or long
                Date start = schedule.date(0), end = schedule.date(1);
                Date paymentDate = calendar.adjust(end,paymentAdjustment);
                if (schedule.isRegular(1)) {
                    leg.push_back(boost::shared_ptr<CashFlow>(
                        new CappedFlooredIborCoupon(paymentDate, get(nominals,0, Null<Real>()), start, end,
                                                    fixingDays, index, get(gearings,0,1.0),
                                                    get(spreads,0,0.0),
                                                    get(caps,0,Null<Rate>()),
                                                    get(floors,0,Null<Rate>()),
                                                    Date(),Date(),paymentDayCounter,true)));
                } else {
                    Date reference = end - schedule.tenor();
                    reference = calendar.adjust(reference,paymentAdjustment);
                    leg.push_back(boost::shared_ptr<CashFlow>(
                        new CappedFlooredIborCoupon(paymentDate, get(nominals,0, Null<Real>()), start, end,
                                                    fixingDays, index, get(gearings,0,1.0),
                                                    get(spreads,0,0.0),
                                                    get(caps,0,Null<Rate>()),
                                                    get(floors,0,Null<Rate>()),
                                                    Date(),Date(),paymentDayCounter,true)));
                }
                // regular periods
                for (Size i=2; i<schedule.size()-1; ++i) {
                    start = end; end = schedule.date(i);
                    paymentDate = calendar.adjust(end,paymentAdjustment);
                    leg.push_back(boost::shared_ptr<CashFlow>(
                        new CappedFlooredIborCoupon(paymentDate, get(nominals,i-1, Null<Real>()), start, end,
                                                    fixingDays, index, get(gearings,i-1,1.0),
                                                    get(spreads,i-1,0.0),
                                                    get(caps,i-1,Null<Rate>()),
                                                    get(floors,i-1,Null<Rate>()),
                                                    Date(),Date(),paymentDayCounter,true)));
                }
                if (schedule.size() > 2) {
                    // last period might be short or long
                    start = end; end = schedule.date(N-1);
                    paymentDate = calendar.adjust(end,paymentAdjustment);
                    if (schedule.isRegular(N-1)) {
                       leg.push_back(boost::shared_ptr<CashFlow>(
                            new CappedFlooredIborCoupon(paymentDate, get(nominals,N-2, Null<Real>()), start, end,
                                                        fixingDays, index, get(gearings,N-2,1.0),
                                                        get(spreads,N-2,0.0),
                                                        get(caps,N-2,Null<Rate>()),
                                                        get(floors,N-2,Null<Rate>()),
                                                        Date(),Date(),paymentDayCounter,true)));
                    } else {
                        Date reference = start + schedule.tenor();
                        reference =
                            calendar.adjust(reference,paymentAdjustment);
                           leg.push_back(boost::shared_ptr<CashFlow>(
                                new CappedFlooredIborCoupon(paymentDate, get(nominals,N-2, Null<Real>()), start, end,
                                                            fixingDays, index, get(gearings,N-2,1.0),
                                                            get(spreads,N-2,0.0),
                                                            get(caps,N-2,Null<Rate>()),
                                                            get(floors,N-2,Null<Rate>()),
                                                            start,reference,paymentDayCounter,true)));
                    }
                }

            }

            return leg;
        }


    Leg CmsLeg(const Schedule& schedule,
                    const std::vector<Real>& nominals,
                    const boost::shared_ptr<SwapIndex>& index,
                    const DayCounter& paymentDayCounter,
                    Integer fixingDays,
                    BusinessDayConvention paymentAdjustment,
                    const std::vector<Real>& gearings,
                    const std::vector<Spread>& spreads,
                    const std::vector<Rate>& caps,
                    const std::vector<Rate>& floors) {

        Leg leg;
        Calendar calendar = schedule.calendar();
        Size N = schedule.size();

        QL_REQUIRE(!nominals.empty(), "no nominal given");

        // first period might be short or long
        Date start = schedule.date(0), end = schedule.date(1);
        Date paymentDate = calendar.adjust(end,paymentAdjustment);
        if (schedule.isRegular(1)) {
            if (get(gearings,0,1.0)!=0.0) {
                leg.push_back(boost::shared_ptr<CashFlow>(new
                    CappedFlooredCmsCoupon(paymentDate, get(nominals,0, Null<Real>()),
                                           start, end, fixingDays, index,
                                           get(gearings,0,1.0),
                                           get(spreads,0,0.0),
                                           get(caps,0,Null<Rate>()),
                                           get(floors,0,Null<Rate>()),
                                           start, end, paymentDayCounter)));
            } else {
                // if gearing is null a fixed rate coupon with rate equal to
                // Min [caprate, Max[floorrate, spread] ] is constructed
                Rate effectiveRate = get(spreads, 0, 0.0);
                if (get(floors, 0, Null<Rate>())!=Null<Rate>())
                    effectiveRate = std::max(get(floors, 0, Null<Rate>()), effectiveRate);
                if (get(caps, 0, Null<Rate>())!=Null<Rate>())
                    effectiveRate = std::min(get(caps, 0, Null<Rate>()), effectiveRate);
                leg.push_back(boost::shared_ptr<CashFlow>(new
                    FixedRateCoupon(get(nominals,0, Null<Real>()), paymentDate,
                                    effectiveRate, paymentDayCounter,
                                    start, end, start, end)));
            }
        } else {
            Date reference = end - schedule.tenor();
            reference = calendar.adjust(reference,paymentAdjustment);
            if (get(gearings,0,1.0)!=0.0) {
               leg.push_back(boost::shared_ptr<CashFlow>(new
                   CappedFlooredCmsCoupon(paymentDate, get(nominals,0, Null<Real>()),
                                          start, end, fixingDays, index,
                                          get(gearings,0,1.0),
                                          get(spreads,0,0.0),
                                          get(caps,0,Null<Rate>()),
                                          get(floors,0,Null<Rate>()),
                                          reference, end, paymentDayCounter)));
            } else {
                // if gearing is null a fixed rate coupon with rate equal to
                // Min [caprate, Max[floorrate, spread] ] is constructed
                Rate effectiveRate = get(spreads, 0, 0.0);
                if (get(floors, 0, Null<Rate>())!=Null<Rate>())
                    effectiveRate = std::max(get(floors, 0, Null<Rate>()), effectiveRate);
                if (get(caps, 0, Null<Rate>())!=Null<Rate>())
                    effectiveRate = std::min(get(caps, 0, Null<Rate>()), effectiveRate);
                leg.push_back(boost::shared_ptr<CashFlow>(new
                    FixedRateCoupon(get(nominals, 0, Null<Real>()), paymentDate,
                                    effectiveRate, paymentDayCounter,
                                    start, end, reference, end)));
            }
        }
        // regular periods
        for (Size i=2; i<schedule.size()-1; ++i) {
            start = end; end = schedule.date(i);
            paymentDate = calendar.adjust(end,paymentAdjustment);
            if (get(gearings,i-1,1.0)!=0.0) {
                leg.push_back(boost::shared_ptr<CashFlow>(new
                    CappedFlooredCmsCoupon(paymentDate, get(nominals, i-1, Null<Real>()),
                                           start, end, fixingDays, index,
                                           get(gearings,i-1,1.0),
                                           get(spreads,i-1,0.0),
                                           get(caps,i-1,Null<Rate>()),
                                           get(floors,i-1,Null<Rate>()),
                                           start, end, paymentDayCounter)));
            } else {
                // if gearing is null a fixed rate coupon with rate equal to
                // Min [caprate, Max[floorrate, spread] ] is constructed
                Rate effectiveRate = get(spreads, i-1, 0.0);
                if (get(floors, i-1, Null<Rate>())!=Null<Rate>())
                    effectiveRate = std::max(get(floors, i-1, Null<Rate>()), effectiveRate);
                if (get(caps, i-1, Null<Rate>())!=Null<Rate>())
                    effectiveRate = std::min(get(caps, i-1, Null<Rate>()), effectiveRate);
                leg.push_back(boost::shared_ptr<CashFlow>(new
                    FixedRateCoupon(get(nominals, i-1, Null<Real>()), paymentDate,
                                    effectiveRate, paymentDayCounter,
                                    start, end, start, end)));
            }
        }
        if (schedule.size() > 2) {
            // last period might be short or long
            start = end; end = schedule.date(N-1);
            paymentDate = calendar.adjust(end,paymentAdjustment);
            if (schedule.isRegular(N-1)) {
                if (get(gearings,N-2,1.0)!=0.0) {
                    leg.push_back(boost::shared_ptr<CashFlow>(new
                        CappedFlooredCmsCoupon(paymentDate, get(nominals, N-2, Null<Real>()),
                                               start, end, fixingDays, index,
                                               get(gearings,N-2,1.0),
                                               get(spreads,N-2,0.0),
                                               get(caps,N-2,Null<Rate>()),
                                               get(floors,N-2,Null<Rate>()),
                                               start, end, paymentDayCounter)));
                } else {
                    // if gearing is null a fixed rate coupon with rate equal to
                    // Min [caprate, Max[floorrate, spread] ] is constructed
                    Rate effectiveRate = get(spreads, N-2, 0.0);
                    if (get(floors, N-2, Null<Rate>())!=Null<Rate>())
                        effectiveRate = std::max(get(floors, N-2, Null<Rate>()), effectiveRate);
                    if (get(caps, N-2, Null<Rate>())!=Null<Rate>())
                        effectiveRate = std::min(get(caps, N-2, Null<Rate>()), effectiveRate);
                    leg.push_back(boost::shared_ptr<CashFlow>(new
                        FixedRateCoupon(get(nominals, N-2, Null<Real>()), paymentDate,
                                        effectiveRate, paymentDayCounter,
                                        start, end, start, end)));
                }
            } else {
                Date reference = start + schedule.tenor();
                reference = calendar.adjust(reference,paymentAdjustment);
                if (get(gearings,N-2,1.0)!=0.0) {
                    leg.push_back(boost::shared_ptr<CashFlow>(new
                        CappedFlooredCmsCoupon(paymentDate, get(nominals, N-2, Null<Real>()),
                                               start, end, fixingDays, index,
                                               get(gearings,N-2,1.0),
                                               get(spreads,N-2,0.0),
                                               get(caps,N-2,Null<Rate>()),
                                               get(floors,N-2,Null<Rate>()),
                                               start, reference, paymentDayCounter)));
                } else {
                    // if gearing is null a fixed rate coupon with rate equal to
                    // Min [caprate, Max[floorrate, spread] ] is constructed
                    Rate effectiveRate = get(spreads, N-2, 0.0);
                    if (get(floors, N-2, Null<Rate>())!=Null<Rate>())
                        effectiveRate = std::max(get(floors, N-2, Null<Rate>()), effectiveRate);
                    if (get(caps, N-2, Null<Rate>())!=Null<Rate>())
                        effectiveRate = std::min(get(caps, N-2, Null<Rate>()), effectiveRate);
                    leg.push_back(boost::shared_ptr<CashFlow>(new
                        FixedRateCoupon(get(nominals, N-2, Null<Real>()), paymentDate,
                                        effectiveRate, paymentDayCounter,
                                        start, end, start, reference)));
                }
            }
        }
        return leg;
    }

    Leg CmsInArrearsLeg(const Schedule& schedule,
                        const std::vector<Real>& nominals,
                        const boost::shared_ptr<SwapIndex>& index,
                        const DayCounter& paymentDayCounter,
                        Integer fixingDays,
                        BusinessDayConvention paymentAdjustment,
                        const std::vector<Real>& gearings,
                        const std::vector<Spread>& spreads,
                        const std::vector<Rate>& caps,
                        const std::vector<Rate>& floors) {

        Leg leg;
        Calendar calendar = schedule.calendar();
        Size N = schedule.size();

        QL_REQUIRE(!nominals.empty(), "no nominal given");

        // first period might be short or long
        Date start = schedule.date(0), end = schedule.date(1);
        Date paymentDate = calendar.adjust(end,paymentAdjustment);
        if (schedule.isRegular(1)) {
            leg.push_back(boost::shared_ptr<CashFlow>(
                new CappedFlooredCmsCoupon(paymentDate, get(nominals, 0, Null<Real>()),
                              start, end, fixingDays, index,
                              get(gearings,0,1.0),
                              get(spreads,0,0.0),
                              get(caps,0,Null<Rate>()),
                              get(floors,0,Null<Rate>()),
                              start, end, paymentDayCounter,true)));

        } else {
            Date reference = end - schedule.tenor();
            reference =
                calendar.adjust(reference,paymentAdjustment);
            leg.push_back(boost::shared_ptr<CashFlow>(
                new CappedFlooredCmsCoupon(paymentDate, get(nominals, 0, Null<Real>()),
                              start, end, fixingDays, index,
                              get(gearings,0,1.0),
                              get(spreads,0,0.0),
                              get(caps,0,Null<Rate>()),
                              get(floors,0,Null<Rate>()),
                              reference, end, paymentDayCounter,true)));
        }
        // regular periods
        for (Size i=2; i<schedule.size()-1; ++i) {
            start = end; end = schedule.date(i);
            paymentDate = calendar.adjust(end,paymentAdjustment);
            leg.push_back(boost::shared_ptr<CashFlow>(
                new CappedFlooredCmsCoupon(paymentDate, get(nominals, i-1, Null<Real>()),
                              start, end, fixingDays, index,
                              get(gearings,i-1,1.0),
                              get(spreads,i-1,0.0),
                              get(caps,i-1,Null<Rate>()),
                              get(floors,i-1,Null<Rate>()),
                              start, end,paymentDayCounter, true)));
        }
        if (schedule.size() > 2) {
            // last period might be short or long
            start = end; end = schedule.date(N-1);
            paymentDate = calendar.adjust(end,paymentAdjustment);
            if (schedule.isRegular(N-1)) {
                leg.push_back(boost::shared_ptr<CashFlow>(
                    new CappedFlooredCmsCoupon(paymentDate, get(nominals, N-2, Null<Real>()),
                                  start, end, fixingDays, index,
                                  get(gearings,N-2,1.0),
                                  get(spreads,N-2,0.0),
                                  get(caps,N-2,Null<Rate>()),
                                  get(floors,N-2,Null<Rate>()),
                                  start, end, paymentDayCounter, true)));
            } else {
                Date reference = start + schedule.tenor();
                reference =
                    calendar.adjust(reference,paymentAdjustment);
                leg.push_back(boost::shared_ptr<CashFlow>(
                    new CappedFlooredCmsCoupon(paymentDate, get(nominals, N-2, Null<Real>()),
                                  start, end, fixingDays, index,
                                  get(gearings,N-2,1.0),
                                  get(spreads,N-2,0.0),
                                  get(caps,N-2,Null<Rate>()),
                                  get(floors,N-2,Null<Rate>()),
                                  start, reference, paymentDayCounter,true)));
            }
        }
        return leg;
    }


    Leg CmsZeroLeg(const Schedule& schedule,
                   const std::vector<Real>& nominals,
                   const boost::shared_ptr<SwapIndex>& index,
                   const DayCounter& paymentDayCounter,
                   Integer fixingDays,
                   BusinessDayConvention paymentAdjustment,
                   const std::vector<Real>& gearings,
                   const std::vector<Spread>& spreads,
                   const std::vector<Rate>& caps,
                   const std::vector<Rate>& floors) {

        Leg leg;
        Calendar calendar = schedule.calendar();
        Size N = schedule.size();

        QL_REQUIRE(!nominals.empty(), "no nominal given");

        // All payment dates at the end
        Date paymentDate = calendar.adjust(schedule.date(N-1),paymentAdjustment);

        // first period might be short or long
        Date start = schedule.date(0), end = schedule.date(1);
        if (schedule.isRegular(1)) {
            leg.push_back(boost::shared_ptr<CashFlow>(
                new CappedFlooredCmsCoupon(paymentDate, get(nominals, 0, Null<Real>()),
                              start, end, fixingDays, index,
                              get(gearings,0,1.0),
                              get(spreads,0,0.0),
                              get(caps,0,Null<Rate>()),
                              get(floors,0,Null<Rate>()),
                              start, end, paymentDayCounter)));

        } else {
            Date reference = end - schedule.tenor();
            reference =
                calendar.adjust(reference,paymentAdjustment);
            leg.push_back(boost::shared_ptr<CashFlow>(
                new CappedFlooredCmsCoupon(paymentDate, get(nominals, 0, Null<Real>()),
                              start, end, fixingDays, index,
                              get(gearings,0,1.0),
                              get(spreads,0,0.0),
                              get(caps,0,Null<Rate>()),
                              get(floors,0,Null<Rate>()),
                              reference, end, paymentDayCounter)));
        }
        // regular periods
        for (Size i=2; i<schedule.size()-1; ++i) {
            start = end; end = schedule.date(i);
            leg.push_back(boost::shared_ptr<CashFlow>(
                new CappedFlooredCmsCoupon(paymentDate, get(nominals, i-1, Null<Real>()),
                              start, end, fixingDays, index,
                              get(gearings,i-1,1.0),
                              get(spreads,i-1,0.0),
                              get(caps,i-1,Null<Rate>()),
                              get(floors,i-1,Null<Rate>()),
                              start, end, paymentDayCounter)));
        }
        if (schedule.size() > 2) {
            // last period might be short or long
            start = end; end = schedule.date(N-1);
            if (schedule.isRegular(N-1)) {
                leg.push_back(boost::shared_ptr<CashFlow>(
                    new CappedFlooredCmsCoupon(paymentDate, get(nominals, N-2, Null<Real>()),
                                  start, end, fixingDays, index,
                                  get(gearings,N-2,1.0),
                                  get(spreads,N-2,0.0),
                                  get(caps,N-2,Null<Rate>()),
                                  get(floors,N-2,Null<Rate>()),
                                  start, end, paymentDayCounter)));
            } else {
                Date reference = start + schedule.tenor();
                reference =
                    calendar.adjust(reference,paymentAdjustment);
                leg.push_back(boost::shared_ptr<CashFlow>(
                    new CappedFlooredCmsCoupon(paymentDate, get(nominals, N-2, Null<Real>()),
                                  start, end, fixingDays, index,
                                  get(gearings,N-2,1.0),
                                  get(spreads,N-2,0.0),
                                  get(caps,N-2,Null<Rate>()),
                                  get(floors,N-2,Null<Rate>()),
                                  start, reference, paymentDayCounter)));
            }
        }
         return leg;
    }

//===========================================================================//
//                 setPricers methods by CouponSelectorToSetPricer           //
//===========================================================================//

    void CashFlows::setPricer(
               const Leg& leg,
               const boost::shared_ptr<FloatingRateCouponPricer>& pricer){
         for(Size i=0; i<leg.size(); ++i){
            CouponSelectorToSetPricer selector(pricer);
            leg[i]->accept(selector);
       }
    }
    
    void CashFlows::setPricers(
            const Leg& leg,
            const std::vector<boost::shared_ptr<FloatingRateCouponPricer> >& pricers){
        QL_REQUIRE(leg.size() == pricers.size(), "mismatch between leg and pricers");
        for(QuantLib::Size i=0; i<leg.size(); ++i){
            CouponSelectorToSetPricer selector(pricers[i]);
            leg[i]->accept(selector);
       }
    }

}
