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

#include <ql/instruments/swap.hpp>
#include <ql/time/daycounter.hpp>
#include <ql/time/businessdayconvention.hpp>
#include <ql/time/calendar.hpp>

namespace QuantLib {

    class Schedule;
    class OvernightIndex;

    //! Overnight indexed swap: fix vs compounded overnight rate
    class OvernightIndexedSwap : public Swap {
    public:
        enum Type { Receiver = -1, Payer = 1 };
        OvernightIndexedSwap(Type type,
                             Real nominal,
                             const Schedule& schedule,
                             Rate fixedRate,
                             const DayCounter& fixedDC,
                             const ext::shared_ptr<OvernightIndex>& overnightIndex,
                             Spread spread = 0.0,
                             Natural paymentLag = 0,
                             BusinessDayConvention paymentAdjustment = Following,
                             const Calendar& paymentCalendar = Calendar(),
                             bool telescopicValueDates = false);

        OvernightIndexedSwap(Type type,
                             const std::vector<Real>& nominals,
                             const Schedule& schedule,
                             Rate fixedRate,
                             const DayCounter& fixedDC,
                             const ext::shared_ptr<OvernightIndex>& overnightIndex,
                             Spread spread = 0.0,
                             Natural paymentLag = 0,
                             BusinessDayConvention paymentAdjustment = Following,
                             const Calendar& paymentCalendar = Calendar(),
                             bool telescopicValueDates = false);

        //! \name Inspectors
        //@{
        Type type() const { return type_; }
        Real nominal() const;
        std::vector<Real> nominals() const { return nominals_; }

        //const Schedule& schedule() { return schedule_; }
        Frequency paymentFrequency() { return paymentFrequency_; }

        Rate fixedRate() const { return fixedRate_; }
        const DayCounter& fixedDayCount() { return fixedDC_; }

        const ext::shared_ptr<OvernightIndex>& overnightIndex() { return overnightIndex_; }
        Spread spread() const { return spread_; }

        const Leg& fixedLeg() const { return legs_[0]; }
        const Leg& overnightLeg() const { return legs_[1]; }
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
        void initialize(const Schedule& schedule);
        Type type_;
        std::vector<Real> nominals_;

        Frequency paymentFrequency_;
        Calendar paymentCalendar_;
        BusinessDayConvention paymentAdjustment_;
        Natural paymentLag_;

        //Schedule schedule_;

        Rate fixedRate_;
        DayCounter fixedDC_;

        ext::shared_ptr<OvernightIndex> overnightIndex_;
        Spread spread_;
        bool telescopicValueDates_;
    };


    // inline

    inline Real OvernightIndexedSwap::nominal() const {
        QL_REQUIRE(nominals_.size()==1, "varying nominals");
        return nominals_[0];
    }

}

#endif
