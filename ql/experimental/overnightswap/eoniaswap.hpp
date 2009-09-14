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

/*! \file eoniaswap.hpp
    \brief Overnight index swap paying compounded Eonia vs. fixed
*/

#ifndef quantlib_eonia_swap_hpp
#define quantlib_eonia_swap_hpp

#include <ql/instruments/swap.hpp>
#include <ql/indexes/ibor/euribor.hpp>
#include <ql/time/schedule.hpp>

namespace QuantLib {

    //! Overnight index swap paying compounded Eonia vs. fixed coupons
    class EoniaSwap : public Swap {
      public:
        enum Type { Receiver = -1, Payer = 1 };
        EoniaSwap(Type type,
                  Real nominal,
                  // Eonia leg
                  const Schedule& eoniaSchedule,
                  Rate eoniaSpread,
                  const boost::shared_ptr<Eonia>& index,
                  // fixed leg
                  const Schedule& fixedSchedule,
                  Rate rate,
                  const DayCounter& fixedDayCount);

        //! \name Inspectors
        //@{
        Spread eoniaSpread() const;
        Rate fixedRate() const;
        Real nominal() const;
        //! "payer" or "receiver" refer to the Eonia leg
        Type type() const;
        const Leg& eoniaLeg() const;
        const Leg& fixedLeg() const;
        //@}

        //! \name Results
        //@{
        Real eoniaLegBPS() const;
        Real eoniaLegNPV() const;
        Spread fairSpread() const;

        Real fixedLegBPS() const;
        Real fixedLegNPV() const;
        Real fairRate() const;
        //@}
      private:
        Type type_;
        Real nominal_;
        Rate eoniaSpread_;
        Rate fixedRate_;
    };

}

#endif

