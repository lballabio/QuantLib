/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Allen Kuo
 Copyright (C) 2017 BN Algorithms Ltd

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <https://www.quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file callablebond.hpp
    \brief callable bond classes
*/

#ifndef quantlib_callable_bond_hpp
#define quantlib_callable_bond_hpp

#include <ql/instruments/bond.hpp>
#include <ql/pricingengine.hpp>
#include <ql/instruments/callabilityschedule.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>
#include <ql/handle.hpp>
#include <ql/quotes/simplequote.hpp>

namespace QuantLib {

    class Schedule;
    class DayCounter;

    //! Callable bond base class
    /*! Base callable bond class for fixed and zero coupon bonds.
        Defines commonalities between fixed and zero coupon callable
        bonds. At present, only European and Bermudan put/call schedules
        supported (no American optionality), as defined by the Callability
        class.

        \todo models/shortrate/calibrationHelpers
        \todo OAS/OAD
        \todo floating rate callable bonds ?

        \ingroup instruments
    */
    class CallableBond : public Bond {
      public:
        class arguments;
        class results;
        class engine;

        //! \name Inspectors
        //@{
        //! return the bond's put/call schedule
        const CallabilitySchedule& callability() const {
            return putCallSchedule_;
        }
        //@}

        //! \name Calculations
        //@{
        //! returns the Black implied forward yield volatility
        /*! the forward yield volatility, see Hull, Fourth Edition,
            Chapter 20, pg 536). Relevant only to European put/call
            schedules
        */
        Volatility impliedVolatility(
                              const Bond::Price& targetPrice,
                              const Handle<YieldTermStructure>& discountCurve,
                              Real accuracy,
                              Size maxEvaluations,
                              Volatility minVol,
                              Volatility maxVol) const;

        //! Calculate the Option Adjusted Spread (OAS)
        /*! Calculates the spread that needs to be added to the
            reference curve so that the theoretical model value
            matches the marketPrice.

         */
        Spread OAS(Real cleanPrice,
                   const Handle<YieldTermStructure>& engineTS,
                   const DayCounter& dayCounter,
                   Compounding compounding,
                   Frequency frequency,
                   Date settlementDate = Date(),
                   Real accuracy = 1.0e-10,
                   Size maxIterations = 100,
                   Rate guess = 0.0);

        //! Calculate the clean price based on the given
        //! option-adjust-spread (oas) over the given yield term
        //! structure (engineTS)
        Real cleanPriceOAS(Real oas,
                           const Handle<YieldTermStructure>& engineTS,
                           const DayCounter& dayCounter,
                           Compounding compounding,
                           Frequency frequency,
                           Date settlementDate = Date());

        //! Calculate the effective duration, i.e., the first
        //! differential of the dirty price w.r.t. a parallel shift of
        //! the yield term structure divided by current dirty price
        Real effectiveDuration(Real oas,
                               const Handle<YieldTermStructure>& engineTS,
                               const DayCounter& dayCounter,
                               Compounding compounding,
                               Frequency frequency,
                               Real bump=2e-4);

        //! Calculate the effective convexity, i.e., the second
        //! differential of the dirty price w.r.t. a parallel shift of
        //! the yield term structure divided by current dirty price
        Real effectiveConvexity(Real oas,
                                const Handle<YieldTermStructure>& engineTS,
                                const DayCounter& dayCounter,
                                Compounding compounding,
                                Frequency frequency,
                                Real bump=2e-4);
        //@}

        void setupArguments(PricingEngine::arguments* args) const override;

      protected:
        CallableBond(Natural settlementDays,
                     const Date& maturityDate,
                     const Calendar& calendar,
                     DayCounter paymentDayCounter,
                     Real faceAmount,
                     const Date& issueDate = Date(),
                     CallabilitySchedule putCallSchedule = CallabilitySchedule());

        DayCounter paymentDayCounter_;
        Frequency frequency_;
        CallabilitySchedule putCallSchedule_;
        Real faceAmount_;
        // helper class for Black implied volatility calculation
        class ImpliedVolHelper;
        // helper class for option adjusted spread calculations
        class NPVSpreadHelper;

      private:
        /*  Used internally.
            same as Bond::accruedAmount() but with enable early
            payments true.  Forces accrued to be calculated in a
            consistent way for future put/ call dates, which can be
            problematic in lattice engines when option dates are also
            coupon dates.
        */
        Real accrued(Date settlement) const;
    };

    class CallableBond::arguments : public Bond::arguments {
      public:
        arguments() = default;
        std::vector<Date> couponDates;
        std::vector<Real> couponAmounts;
        Real faceAmount;
        //! redemption = face amount * redemption / 100.
        Real redemption;
        Date redemptionDate;
        DayCounter paymentDayCounter;
        Frequency frequency;
        CallabilitySchedule putCallSchedule;
        //! bond full/dirty/cash prices
        std::vector<Real> callabilityPrices;
        std::vector<Date> callabilityDates;
        //! Spread to apply to the valuation. This is a continuously
        //! componded rate added to the model. Currently only applied
        //! by the TreeCallableFixedRateBondEngine
        Real spread;
        void validate() const override;
    };

    //! results for a callable bond calculation
    class CallableBond::results : public Bond::results {
      public:
        // no extra results set yet
    };

    //! base class for callable fixed rate bond engine
    class CallableBond::engine
        : public GenericEngine<CallableBond::arguments,
                               CallableBond::results> {};


    //! callable/puttable fixed rate bond
    /*! Callable fixed rate bond class.

        \ingroup instruments
    */
    class CallableFixedRateBond : public CallableBond {
      public:
        CallableFixedRateBond(Natural settlementDays,
                              Real faceAmount,
                              Schedule schedule,
                              const std::vector<Rate>& coupons,
                              const DayCounter& accrualDayCounter,
                              BusinessDayConvention paymentConvention = Following,
                              Real redemption = 100.0,
                              const Date& issueDate = Date(),
                              const CallabilitySchedule& putCallSchedule = {},
                              const Period& exCouponPeriod = Period(),
                              const Calendar& exCouponCalendar = Calendar(),
                              BusinessDayConvention exCouponConvention = Unadjusted,
                              bool exCouponEndOfMonth = false);
    };

    //! callable/puttable zero coupon bond
    /*! Callable zero coupon bond class.

        \ingroup instruments
    */
    class CallableZeroCouponBond : public CallableBond {
      public:
        CallableZeroCouponBond(Natural settlementDays,
                               Real faceAmount,
                               const Calendar& calendar,
                               const Date& maturityDate,
                               const DayCounter& dayCounter,
                               BusinessDayConvention paymentConvention = Following,
                               Real redemption = 100.0,
                               const Date& issueDate = Date(),
                               const CallabilitySchedule& putCallSchedule = {});
    };

}

#endif
