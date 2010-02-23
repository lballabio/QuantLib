/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2009 Roland Lichters
 Copyright (C) 2009 Ferdinando Ametrano

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

namespace QuantLib {

    class Schedule;
    class OvernightIndex;

    //! Overnight indexed swap: fix vs compounded overnight rate
    class OvernightIndexedSwap : public Swap {
      public:
        enum Type { Receiver = -1, Payer = 1 };
        OvernightIndexedSwap(
                    Type type,
                    Real nominal,
                    const Schedule& schedule,
                    Rate fixedRate,
                    const DayCounter& fixedDC,
                    const boost::shared_ptr<OvernightIndex>& overnightIndex,
                    Spread spread = 0.0);
        //! \name Inspectors
        //@{
        Type type() const { return type_; }
        Real nominal() const { return nominal_; }

        //const Schedule& schedule() { return schedule_; }
        Frequency paymentFrequency() { return paymentFrequency_; }

        Rate fixedRate() const { return fixedRate_; }
        const DayCounter& fixedDayCount() { return fixedDC_; }

        const boost::shared_ptr<OvernightIndex>& overnightIndex();
        Spread spread() { return spread_; }

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
        Type type_;
        Real nominal_;

        Frequency paymentFrequency_;
        //Schedule schedule_;

        Rate fixedRate_;
        DayCounter fixedDC_;

        boost::shared_ptr<OvernightIndex> overnightIndex_;
        Spread spread_;
    };

}

#endif
