/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2009 Roland Lichters
 Copyright (C) 2009 Ferdinando Ametrano
 Copyright (C) 2017 Joseph Jeisman
 Copyright (C) 2017 Fabrice Lecuyer
 Copyright (C) 2026 Sergio Araujo
 Copyright (C) 2026 Thiago Rodrigues

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

/*! \file overnightindexedswap.hpp
    \brief Overnight index swap paying compounded overnight vs. fixed
*/

#ifndef quantlib_overnight_indexed_swap_hpp
#define quantlib_overnight_indexed_swap_hpp

#include <ql/cashflows/rateaveraging.hpp>
#include <ql/compounding.hpp>
#include <ql/instruments/fixedvsfloatingswap.hpp>
#include <ql/optional.hpp>
#include <ql/time/businessdayconvention.hpp>
#include <ql/time/calendar.hpp>
#include <ql/time/daycounter.hpp>
#include <ql/time/frequency.hpp>
#include <ql/time/schedule.hpp>

namespace QuantLib {

    class OvernightIndex;

    //! Overnight indexed swap: fix vs compounded overnight rate
    /*! By default, fixed coupons accrue simple interest, i.e., they pay
        \f$ N K \tau \f$.  The optional \c fixedCompounding and
        \c fixedCompoundingFrequency parameters allow the fixed rate to
        be quoted with a different convention; the fixed coupons then
        pay \f$ N (c(K, \tau) - 1) \f$, where \f$ c \f$ is the compound
        factor of an InterestRate with the given day counter,
        compounding and frequency.

        For instance, Brazilian DI x Pré swaps quote the fixed rate as
        exponential on business days, i.e., \f$ (1+K)^{n/252} - 1 \f$
        where \f$ n \f$ is the number of business days in the period;
        they can be modeled by passing Business252, Compounded and
        Annual, usually with a single (zero-coupon) fixed payment.

        \warning When the fixed compounding is not Simple, the fixed-leg
                 NPV is not linear in the fixed rate.  The fair rate is
                 then obtained in closed form for a single fixed coupon
                 and by a one-dimensional root search otherwise; the
                 latter requires a DiscountingSwapEngine (with other
                 engines, fairRate() reports that the result is not
                 available.)  The value of
                 fixedLegBPS() is still the sum of the discounted
                 \f$ N \tau \f$ terms (times one basis point) and is no
                 longer the sensitivity of the fixed-leg NPV to the rate.
    */
    class OvernightIndexedSwap : public FixedVsFloatingSwap {
      public:
        OvernightIndexedSwap(Type type,
                             Real nominal,
                             const Schedule& schedule,
                             Rate fixedRate,
                             DayCounter fixedDC,
                             const ext::shared_ptr<OvernightIndex>& overnightIndex,
                             Spread spread = 0.0,
                             Integer paymentLag = 0,
                             BusinessDayConvention paymentAdjustment = Following,
                             const Calendar& paymentCalendar = Calendar(),
                             bool telescopicValueDates = false,
                             RateAveraging::Type averagingMethod = RateAveraging::Compound,
                             Natural lookbackDays = Null<Natural>(),
                             Natural lockoutDays = 0,
                             bool applyObservationShift = false,
                             const std::optional<Integer>& roundingPrecision = std::nullopt,
                             Compounding fixedCompounding = Simple,
                             Frequency fixedCompoundingFrequency = Annual);

        OvernightIndexedSwap(Type type,
                             const std::vector<Real>& nominals,
                             const Schedule& schedule,
                             Rate fixedRate,
                             DayCounter fixedDC,
                             const ext::shared_ptr<OvernightIndex>& overnightIndex,
                             Spread spread = 0.0,
                             Integer paymentLag = 0,
                             BusinessDayConvention paymentAdjustment = Following,
                             const Calendar& paymentCalendar = Calendar(),
                             bool telescopicValueDates = false,
                             RateAveraging::Type averagingMethod = RateAveraging::Compound,
                             Natural lookbackDays = Null<Natural>(),
                             Natural lockoutDays = 0,
                             bool applyObservationShift = false,
                             const std::optional<Integer>& roundingPrecision = std::nullopt,
                             Compounding fixedCompounding = Simple,
                             Frequency fixedCompoundingFrequency = Annual);

        OvernightIndexedSwap(Type type,
                             Real nominal,
                             Schedule fixedSchedule,
                             Rate fixedRate,
                             DayCounter fixedDC,
                             Schedule overnightSchedule,
                             const ext::shared_ptr<OvernightIndex>& overnightIndex,
                             Spread spread = 0.0,
                             Integer paymentLag = 0,
                             BusinessDayConvention paymentAdjustment = Following,
                             const Calendar& paymentCalendar = Calendar(),
                             bool telescopicValueDates = false,
                             RateAveraging::Type averagingMethod = RateAveraging::Compound,
                             Natural lookbackDays = Null<Natural>(),
                             Natural lockoutDays = 0,
                             bool applyObservationShift = false,
                             const std::optional<Integer>& roundingPrecision = std::nullopt,
                             Compounding fixedCompounding = Simple,
                             Frequency fixedCompoundingFrequency = Annual);

        OvernightIndexedSwap(Type type,
                             std::vector<Real> fixedNominals,
                             Schedule fixedSchedule,
                             Rate fixedRate,
                             DayCounter fixedDC,
                             const std::vector<Real>& overnightNominals,
                             Schedule overnightSchedule,
                             const ext::shared_ptr<OvernightIndex>& overnightIndex,
                             Spread spread = 0.0,
                             Integer paymentLag = 0,
                             BusinessDayConvention paymentAdjustment = Following,
                             const Calendar& paymentCalendar = Calendar(),
                             bool telescopicValueDates = false,
                             RateAveraging::Type averagingMethod = RateAveraging::Compound,
                             Natural lookbackDays = Null<Natural>(),
                             Natural lockoutDays = 0,
                             bool applyObservationShift = false,
                             const std::optional<Integer>& roundingPrecision = std::nullopt,
                             Compounding fixedCompounding = Simple,
                             Frequency fixedCompoundingFrequency = Annual);

        //! \name Inspectors
        //@{
        Frequency paymentFrequency() const {
            return std::max(fixedSchedule().tenor().frequency(),
                            floatingSchedule().tenor().frequency());
        }

        const std::vector<Real>& overnightNominals() const { return floatingNominals(); }
        const Schedule& overnightSchedule() const { return floatingSchedule(); }
        const ext::shared_ptr<OvernightIndex>& overnightIndex() const { return overnightIndex_; }
        const Leg& overnightLeg() const { return floatingLeg(); }

        Integer paymentLag() const { return paymentLag_; }
        const Calendar& paymentCalendar() const { return paymentCalendar_; }
        bool telescopicValueDates() const { return telescopicValueDates_; }

        RateAveraging::Type averagingMethod() const { return averagingMethod_; }
        Natural lookbackDays() const { return lookbackDays_; }
        Natural lockoutDays() const { return lockoutDays_; }
        bool applyObservationShift() const { return applyObservationShift_; }
        std::optional<Integer> roundingPrecision() const { return roundingPrecision_; }
        Compounding fixedCompounding() const { return fixedCompounding_; }
        Frequency fixedCompoundingFrequency() const { return fixedCompoundingFrequency_; }
        //@}

        //! \name Results
        //@{
        Real overnightLegBPS() const { return floatingLegBPS(); }
        Real overnightLegNPV() const { return floatingLegNPV(); }
        /*! Returns the fixed rate for which the fixed leg, priced as
            by the current engine, would have the given NPV (with the
            sign convention of fixedLegNPV().)  It takes the fixed-leg
            compounding into account; see the class documentation.
        */
        Rate impliedFixedRate(Real fixedLegNPV) const;
        //@}
        // other
        void fetchResults(const PricingEngine::results*) const override;

      private:
        void setupFloatingArguments(arguments* args) const override;
        Rate fixedRateForNPV(Real fixedLegNPV) const;

        ext::shared_ptr<OvernightIndex> overnightIndex_;
        Integer paymentLag_;
        Calendar paymentCalendar_;
        bool telescopicValueDates_;
        RateAveraging::Type averagingMethod_;
        Natural lookbackDays_;
        Natural lockoutDays_;
        bool applyObservationShift_;
        std::optional<Integer> roundingPrecision_;
        Compounding fixedCompounding_;
        Frequency fixedCompoundingFrequency_;
    };

}

#endif
