/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2009 Roland Lichters

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

/*! \file overnightswap.hpp
    \brief Overnight index swap paying compounded overnight vs. fixed
*/

#ifndef quantlib_overnight_swap_hpp
#define quantlib_overnight_swap_hpp

#include <ql/instruments/swap.hpp>
#include <ql/indexes/ibor/euribor.hpp>
#include <ql/time/schedule.hpp>

namespace QuantLib {

    //! Overnight index swap paying compounded overnight vs. fixed coupons
    class OvernightIndexedSwap : public Swap {
      public:
        enum Type { Receiver = -1, Payer = 1 };
        OvernightIndexedSwap(Type type,
                  Real nominal,
                  // overnight leg
                  const Schedule& overnightSchedule,
                  Rate overnightSpread,
                  const boost::shared_ptr<OvernightIndex>& index,
                  // fixed leg
                  const Schedule& fixedSchedule,
                  Rate rate,
                  const DayCounter& fixedDayCount);

        //! \name Inspectors
        //@{
        Spread overnightSpread() const;
        Rate fixedRate() const;
        Real nominal() const;
        //! "payer" or "receiver" refer to the overnight leg
        Type type() const;
        const Leg& overnightLeg() const;
        const Leg& fixedLeg() const;
        //@}

        //! \name Results
        //@{
        Real overnightLegBPS() const;
        Real overnightLegNPV() const;
        Spread fairSpread() const;

        Real fixedLegBPS() const;
        Real fixedLegNPV() const;
        Real fairRate() const;
        //@}
      private:
        Type type_;
        Real nominal_;
        Rate overnightSpread_;
        Rate fixedRate_;
    };

}

#endif

