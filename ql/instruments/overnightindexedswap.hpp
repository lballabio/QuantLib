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
#include <ql/instruments/swap.hpp>
#include <ql/time/businessdayconvention.hpp>
#include <ql/time/calendar.hpp>
#include <ql/time/daycounter.hpp>
#include <ql/time/schedule.hpp>

namespace QuantLib {

    class Schedule;
    class OvernightIndex;

    //! Overnight indexed swap: fix vs compounded overnight rate
    class OvernightIndexedSwap : public Swap {
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
                             std::vector<Real> nominals,
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
                             Real nominal,
                             const Schedule& fixedSchedule,
                             Rate fixedRate,
                             DayCounter fixedDC,
                             const Schedule& overnightSchedule,
                             ext::shared_ptr<OvernightIndex> overnightIndex,
                             Spread spread = 0.0,
                             Natural paymentLag = 0,
                             BusinessDayConvention paymentAdjustment = Following,
                             const Calendar& paymentCalendar = Calendar(),
                             bool telescopicValueDates = false,
                             RateAveraging::Type averagingMethod = RateAveraging::Compound);

        OvernightIndexedSwap(Type type,
                             std::vector<Real> nominals,
                             Schedule fixedSchedule,
                             Rate fixedRate,
                             DayCounter fixedDC,
                             Schedule overnightSchedule,
                             ext::shared_ptr<OvernightIndex> overnightIndex,
                             Spread spread = 0.0,
                             Natural paymentLag = 0,
                             BusinessDayConvention paymentAdjustment = Following,
                             const Calendar& paymentCalendar = Calendar(),
                             bool telescopicValueDates = false,
                             RateAveraging::Type averagingMethod = RateAveraging::Compound);

        //! \name Inspectors
        //@{
        Type type() const { return type_; }
        Real nominal() const;
        std::vector<Real> nominals() const { return nominals_; }

        Frequency paymentFrequency() const {
            return std::max(fixedSchedule_.tenor().frequency(),
                            overnightSchedule_.tenor().frequency());
        }

        const Schedule& fixedSchedule() const { return fixedSchedule_; }
        Rate fixedRate() const { return fixedRate_; }
        const DayCounter& fixedDayCount() const { return fixedDC_; }

        const Schedule& overnightSchedule() const { return overnightSchedule_; }
        const ext::shared_ptr<OvernightIndex>& overnightIndex() const { return overnightIndex_; }
        Spread spread() const { return spread_; }

        const Leg& fixedLeg() const { return legs_[0]; }
        const Leg& overnightLeg() const { return legs_[1]; }

        RateAveraging::Type averagingMethod() const { return averagingMethod_; }
        //@}

        //! \name Results
        //@{
        Real fixedLegBPS() const;
        Real fixedLegNPV() const;
        Real fairRate() const;

        Real overnightLegBPS() const;
        Real overnightLegNPV() const;
        Spread fairSpread() const;
        //@}
      private:
        Type type_;
        std::vector<Real> nominals_;

        Schedule fixedSchedule_;
        Rate fixedRate_;
        DayCounter fixedDC_;

        Schedule overnightSchedule_;
        ext::shared_ptr<OvernightIndex> overnightIndex_;
        Spread spread_;
        RateAveraging::Type averagingMethod_;
    };


    // inline

    inline Real OvernightIndexedSwap::nominal() const {
        QL_REQUIRE(nominals_.size() == 1, "varying nominals");
        return nominals_[0];
    }

}

#endif
