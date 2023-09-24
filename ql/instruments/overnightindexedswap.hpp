/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2009 Roland Lichters
 Copyright (C) 2009 Ferdinando Ametrano
 Copyright (C) 2017 Joseph Jeisman
 Copyright (C) 2017 Fabrice Lecuyer

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

/*! \file overnightindexedswap.hpp
    \brief Overnight index swap paying compounded overnight vs. fixed
*/

#ifndef quantlib_overnight_indexed_swap_hpp
#define quantlib_overnight_indexed_swap_hpp

#include <ql/cashflows/rateaveraging.hpp>
#include <ql/instruments/fixedvsfloatingswap.hpp>
#include <ql/time/businessdayconvention.hpp>
#include <ql/time/calendar.hpp>
#include <ql/time/daycounter.hpp>
#include <ql/time/schedule.hpp>

namespace QuantLib {

    class OvernightIndex;

    //! Overnight indexed swap: fix vs compounded overnight rate
    class OvernightIndexedSwap : public FixedVsFloatingSwap {
      public:
        OvernightIndexedSwap(Type type,
                             Real nominal,
                             const Schedule& schedule,
                             Rate fixedRate,
                             DayCounter fixedDC,
                             ext::shared_ptr<OvernightIndex> overnightIndex,
                             Spread spread = 0.0,
                             Natural paymentLag = 0,
                             BusinessDayConvention paymentAdjustment = Following,
                             const Calendar& paymentCalendar = Calendar(),
                             bool telescopicValueDates = false,
                             RateAveraging::Type averagingMethod = RateAveraging::Compound);

        OvernightIndexedSwap(Type type,
                             const std::vector<Real>& nominals,
                             const Schedule& schedule,
                             Rate fixedRate,
                             DayCounter fixedDC,
                             const ext::shared_ptr<OvernightIndex>& overnightIndex,
                             Spread spread = 0.0,
                             Natural paymentLag = 0,
                             BusinessDayConvention paymentAdjustment = Following,
                             const Calendar& paymentCalendar = Calendar(),
                             bool telescopicValueDates = false,
                             RateAveraging::Type averagingMethod = RateAveraging::Compound);

        OvernightIndexedSwap(Type type,
                             Real nominal,
                             const Schedule& fixedSchedule,
                             Rate fixedRate,
                             DayCounter fixedDC,
                             const Schedule& overnightSchedule,
                             const ext::shared_ptr<OvernightIndex>& overnightIndex,
                             Spread spread = 0.0,
                             Natural paymentLag = 0,
                             BusinessDayConvention paymentAdjustment = Following,
                             const Calendar& paymentCalendar = Calendar(),
                             bool telescopicValueDates = false,
                             RateAveraging::Type averagingMethod = RateAveraging::Compound);

        OvernightIndexedSwap(Type type,
                             std::vector<Real> fixedNominals,
                             Schedule fixedSchedule,
                             Rate fixedRate,
                             DayCounter fixedDC,
                             const std::vector<Real>& overnightNominals,
                             const Schedule& overnightSchedule,
                             const ext::shared_ptr<OvernightIndex>& overnightIndex,
                             Spread spread = 0.0,
                             Natural paymentLag = 0,
                             BusinessDayConvention paymentAdjustment = Following,
                             const Calendar& paymentCalendar = Calendar(),
                             bool telescopicValueDates = false,
                             RateAveraging::Type averagingMethod = RateAveraging::Compound);

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

        RateAveraging::Type averagingMethod() const { return averagingMethod_; }
        //@}

        //! \name Results
        //@{
        Real overnightLegBPS() const { return floatingLegBPS(); }
        Real overnightLegNPV() const { return floatingLegNPV(); }
        //@}
      private:
        void setupFloatingArguments(arguments* args) const override;

        ext::shared_ptr<OvernightIndex> overnightIndex_;
        RateAveraging::Type averagingMethod_;
    };

}

#endif
