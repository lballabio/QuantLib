/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2005, 2006 Theo Boafo
 Copyright (C) 2006, 2007 StatPro Italia srl

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

/*! \file convertiblebonds.hpp
    \brief convertible bond class
*/

#ifndef quantlib_convertible_bonds_hpp
#define quantlib_convertible_bonds_hpp

#include <ql/instruments/bond.hpp>
#include <ql/instruments/callabilityschedule.hpp>
#include <ql/instruments/dividendschedule.hpp>
#include <ql/instruments/oneassetoption.hpp>
#include <ql/quote.hpp>
#include <ql/time/daycounter.hpp>
#include <ql/time/schedule.hpp>

namespace QuantLib {

    class IborIndex;
    class PricingEngine;

    //! %callability leaving to the holder the possibility to convert
    class SoftCallability : public Callability {
      public:
        SoftCallability(const Bond::Price& price, const Date& date, Real trigger)
        : Callability(price, Callability::Call, date), trigger_(trigger) {}
        Real trigger() const { return trigger_; }

      private:
        Real trigger_;
    };


    //! base class for convertible bonds
    class ConvertibleBond : public Bond {
      public:
        class arguments;
        class engine;
        Real conversionRatio() const { return conversionRatio_; }
        const CallabilitySchedule& callability() const { return callability_; }

      protected:
        ConvertibleBond(ext::shared_ptr<Exercise> exercise,
                        Real conversionRatio,
                        const CallabilitySchedule& callability,
                        const Date& issueDate,
                        Natural settlementDays,
                        const Schedule& schedule,
                        Real redemption);
        void setupArguments(PricingEngine::arguments*) const override;

      private:
        ext::shared_ptr<Exercise> exercise_;
        Real conversionRatio_;
        CallabilitySchedule callability_;
        Real redemption_;
    };


    //! convertible zero-coupon bond
    /*! \warning Most methods inherited from Bond (such as yield or
                 the yield-based dirtyPrice and cleanPrice) refer to
                 the underlying plain-vanilla bond and do not take
                 convertibility and callability into account.
    */
    class ConvertibleZeroCouponBond : public ConvertibleBond {
      public:
        ConvertibleZeroCouponBond(const ext::shared_ptr<Exercise>& exercise,
                                  Real conversionRatio,
                                  const CallabilitySchedule& callability,
                                  const Date& issueDate,
                                  Natural settlementDays,
                                  const DayCounter& dayCounter,
                                  const Schedule& schedule,
                                  Real redemption = 100);
    };


    //! convertible fixed-coupon bond
    /*! \warning Most methods inherited from Bond (such as yield or
                 the yield-based dirtyPrice and cleanPrice) refer to
                 the underlying plain-vanilla bond and do not take
                 convertibility and callability into account.
    */
    class ConvertibleFixedCouponBond : public ConvertibleBond {
      public:
        ConvertibleFixedCouponBond(const ext::shared_ptr<Exercise>& exercise,
                                   Real conversionRatio,
                                   const CallabilitySchedule& callability,
                                   const Date& issueDate,
                                   Natural settlementDays,
                                   const std::vector<Rate>& coupons,
                                   const DayCounter& dayCounter,
                                   const Schedule& schedule,
                                   Real redemption = 100,
                                   const Period& exCouponPeriod = Period(),
                                   const Calendar& exCouponCalendar = Calendar(),
                                   BusinessDayConvention exCouponConvention = Unadjusted,
                                   bool exCouponEndOfMonth = false);
    };


    //! convertible floating-rate bond
    /*! \warning Most methods inherited from Bond (such as yield or
                 the yield-based dirtyPrice and cleanPrice) refer to
                 the underlying plain-vanilla bond and do not take
                 convertibility and callability into account.
    */
    class ConvertibleFloatingRateBond : public ConvertibleBond {
      public:
        ConvertibleFloatingRateBond(const ext::shared_ptr<Exercise>& exercise,
                                    Real conversionRatio,
                                    const CallabilitySchedule& callability,
                                    const Date& issueDate,
                                    Natural settlementDays,
                                    const ext::shared_ptr<IborIndex>& index,
                                    Natural fixingDays,
                                    const std::vector<Spread>& spreads,
                                    const DayCounter& dayCounter,
                                    const Schedule& schedule,
                                    Real redemption = 100,
                                    const Period& exCouponPeriod = Period(),
                                    const Calendar& exCouponCalendar = Calendar(),
                                    BusinessDayConvention exCouponConvention = Unadjusted,
                                    bool exCouponEndOfMonth = false);
    };


    class ConvertibleBond::arguments : public PricingEngine::arguments {
      public:
        arguments()
        : conversionRatio(Null<Real>()), settlementDays(Null<Natural>()), redemption(Null<Real>()) {}

        ext::shared_ptr<Exercise> exercise;
        Real conversionRatio;
        std::vector<Date> callabilityDates; 
        std::vector<Callability::Type> callabilityTypes;
        std::vector<Real> callabilityPrices;
        std::vector<Real> callabilityTriggers;
        Leg cashflows;
        Date issueDate;
        Date settlementDate;

        Natural settlementDays;
        Real redemption;
        void validate() const override;
    };

    class ConvertibleBond::engine
    : public GenericEngine<ConvertibleBond::arguments, ConvertibleBond::results> {};

}

#endif


#ifndef id_7d154010bbadf415693660c4fc0cd677
#define id_7d154010bbadf415693660c4fc0cd677
inline bool test_7d154010bbadf415693660c4fc0cd677(const int* i) {
    return i != nullptr;
}
#endif
