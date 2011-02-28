/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Roland Lichters
 Copyright (C) 2007 StatPro Italia srl

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

/*! \file bmaswap.hpp
    \brief swap paying Libor against BMA coupons
*/

#ifndef quantlib_bma_swap_hpp
#define quantlib_bma_swap_hpp

#include <ql/instruments/swap.hpp>
#include <ql/indexes/iborindex.hpp>
#include <ql/indexes/bmaindex.hpp>

namespace QuantLib {

    //! swap paying Libor against BMA coupons
    class BMASwap : public Swap {
      public:
        enum Type { Receiver = -1, Payer = 1 };
        BMASwap(Type type,
                Real nominal,
                // Libor leg
                const Schedule& liborSchedule,
                Rate liborFraction,
                Rate liborSpread,
                const boost::shared_ptr<IborIndex>& liborIndex,
                const DayCounter& liborDayCount,
                // BMA leg
                const Schedule& bmaSchedule,
                const boost::shared_ptr<BMAIndex>& bmaIndex,
                const DayCounter& bmaDayCount);

        //! \name Inspectors
        //@{
        Real liborFraction() const;
        Spread liborSpread() const;
        Real nominal() const;
        //! "payer" or "receiver" refer to the BMA leg
        Type type() const;
        const Leg& bmaLeg() const;
        const Leg& liborLeg() const;
        //@}

        //! \name Results
        //@{
        Real liborLegBPS() const;
        Real liborLegNPV() const;
        Rate fairLiborFraction() const;
        Spread fairLiborSpread() const;

        Real bmaLegBPS() const;
        Real bmaLegNPV() const;
        //@}

      private:
        Type type_;
        Real nominal_;
        Rate liborFraction_;
        Rate liborSpread_;
    };

}

#endif

