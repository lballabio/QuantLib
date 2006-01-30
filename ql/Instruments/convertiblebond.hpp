/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2005, 2006 Theo Boafo
 Copyright (C) 2006 StatPro Italia srl

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

/*! \file convertiblebond.hpp
    \brief convertible bond class
*/

#ifndef quantlib_convertible_bond_hpp
#define quantlib_convertible_bond_hpp

#include <ql/schedule.hpp>
#include <ql/exercise.hpp>
#include <ql/pricingengine.hpp>
#include <ql/payoff.hpp>
#include <ql/stochasticprocess.hpp>
#include <ql/Instruments/bond.hpp>
#include <ql/Instruments/oneassetstrikedoption.hpp>
#include <ql/Instruments/dividendschedule.hpp>
#include <ql/Instruments/callabilityschedule.hpp>
#include <ql/Indexes/xibor.hpp>

namespace QuantLib {

    class ConvertibleBond : public Bond {
      public:
        class option;
        Real conversionRatio() const { return conversionRatio_; }
        const DividendSchedule& dividends() const { return dividends_; }
        const CallabilitySchedule& callability() const { return callability_; }
        const Handle<Quote>& creditSpread() const { return creditSpread_; }
      protected:
        ConvertibleBond(const boost::shared_ptr<StochasticProcess>& process,
                        const boost::shared_ptr<Exercise>& exercise,
                        const boost::shared_ptr<PricingEngine>& engine,
                        Real conversionRatio,
                        const DividendSchedule&  dividends,
                        const CallabilitySchedule& callability,
                        const Handle<Quote>& creditSpread,
                        const Date& issueDate,
                        Integer settlementDays,
                        const DayCounter& dayCounter,
                        const Schedule& schedule,
                        Real redemption);
        void performCalculations() const;
        Real conversionRatio_;
        CallabilitySchedule callability_;
        DividendSchedule dividends_;
        Handle<Quote> creditSpread_;
        boost::shared_ptr<option> option_;
    };


    //! convertible zero-coupon bond
    /*! \warning At this time, discrete dividends are not managed.

        \warning Most methods inherited from Bond (such as yield or
                 the yield-based dirtyPrice and cleanPrice) refer to
                 the underlying plain-vanilla bond and do not take
                 convertibility and callability into account.
    */
    class ConvertibleZeroCouponBond : public ConvertibleBond {
      public:
        ConvertibleZeroCouponBond(
                          const boost::shared_ptr<StochasticProcess>& process,
                          const boost::shared_ptr<Exercise>& exercise,
                          const boost::shared_ptr<PricingEngine>& engine,
                          Real conversionRatio,
                          const DividendSchedule&  dividends,
                          const CallabilitySchedule& callability,
                          const Handle<Quote>& creditSpread,
                          const Date& issueDate,
                          Integer settlementDays,
                          const DayCounter& dayCounter,
                          const Schedule& schedule,
                          Real redemption = 100);
    };


    //! convertible fixed-coupon bond
    /*! \warning At this time, discrete dividends are not managed.

        \warning Most methods inherited from Bond (such as yield or
                 the yield-based dirtyPrice and cleanPrice) refer to
                 the underlying plain-vanilla bond and do not take
                 convertibility and callability into account.
    */
    class ConvertibleFixedCouponBond : public ConvertibleBond {
      public:
        ConvertibleFixedCouponBond(
                          const boost::shared_ptr<StochasticProcess>& process,
                          const boost::shared_ptr<Exercise>& exercise,
                          const boost::shared_ptr<PricingEngine>& engine,
                          Real conversionRatio,
                          const DividendSchedule&  dividends,
                          const CallabilitySchedule& callability,
                          const Handle<Quote>& creditSpread,
                          const Date& issueDate,
                          Integer settlementDays,
                          const std::vector<Rate>& coupons,
                          const DayCounter& dayCounter,
                          const Schedule& schedule,
                          Real redemption = 100);
    };


    //! convertible floating-rate bond
    /*! \warning At this time, discrete dividends are not managed.

        \warning Most methods inherited from Bond (such as yield or
                 the yield-based dirtyPrice and cleanPrice) refer to
                 the underlying plain-vanilla bond and do not take
                 convertibility and callability into account.
    */
    class ConvertibleFloatingRateBond : public ConvertibleBond {
      public:
        ConvertibleFloatingRateBond(
                          const boost::shared_ptr<StochasticProcess>& process,
                          const boost::shared_ptr<Exercise>& exercise,
                          const boost::shared_ptr<PricingEngine>& engine,
                          Real conversionRatio,
                          const DividendSchedule&  dividends,
                          const CallabilitySchedule& callability,
                          const Handle<Quote>& creditSpread,
                          const Date& issueDate,
                          Integer settlementDays,
                          const boost::shared_ptr<Xibor>& index,
                          Integer fixingDays,
                          const std::vector<Spread>& spreads,
                          const DayCounter& dayCounter,
                          const Schedule& schedule,
                          Real redemption = 100);
    };


    class ConvertibleBond::option : public OneAssetStrikedOption {
      public:
        class arguments;
        class engine;
        option(const ConvertibleBond* bond,
               const boost::shared_ptr<StochasticProcess>& process,
               const boost::shared_ptr<Exercise>& exercise,
               const boost::shared_ptr<PricingEngine>& engine,
               Real conversionRatio,
               const DividendSchedule&  dividends,
               const CallabilitySchedule& callability,
               const Handle<Quote>& creditSpread,
               const std::vector<boost::shared_ptr<CashFlow> >& cashFlows,
               const DayCounter& dayCounter,
               const Schedule& schedule,
               const Date& issueDate,
               Integer settlementDays,
               Real redemption);

        void setupArguments(Arguments*) const;
      private:
        const ConvertibleBond* bond_;
        Real conversionRatio_;
        CallabilitySchedule callability_;
        DividendSchedule  dividends_;
        Handle<Quote> creditSpread_;
        std::vector<boost::shared_ptr<CashFlow> > cashFlows_;
        DayCounter dayCounter_;
        Date issueDate_;
        Schedule schedule_;
        Integer settlementDays_;
        Real redemption_;
    };

    //! %Arguments for Convertible Bond calculation
    class ConvertibleBond::option::arguments
        : public OneAssetStrikedOption::arguments {
      public:
        arguments()
        : conversionRatio(Null<Real>()), settlementDays(Null<Integer>()),
          redemption(Null<Real>()) {}
        Real conversionRatio;
        Handle<Quote> creditSpread;
        DividendSchedule dividends;
        std::vector<Time> callabilityTimes;
        std::vector<Callability::Type> callabilityTypes;
        std::vector<Real> callabilityPrices;
        std::vector<Time> couponTimes;
        std::vector<Real> couponAmounts;
        DayCounter dayCounter;
        Date issueDate;
        Date settlementDate;
        Integer settlementDays;
        Real redemption;
        void validate() const;
    };


    //! convertible bond engine base class
    class ConvertibleBond::option::engine
        : public GenericEngine<ConvertibleBond::option::arguments,
                               ConvertibleBond::option::results> {};

}


#endif

