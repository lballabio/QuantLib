/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2014 Master IMAFA - Polytech'Nice Sophia - Université de Nice Sophia Antipolis

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

/*! \file partialtimebarrieroption.hpp
    \brief Partial-time barrier option
*/

#ifndef quantlib_partial_time_barrier_option_hpp
#define quantlib_partial_time_barrier_option_hpp

#include <ql/instruments/oneassetoption.hpp>
#include <ql/instruments/barriertype.hpp>
#include <ql/instruments/payoffs.hpp>

namespace QuantLib {

    class GeneralizedBlackScholesProcess;

    //! choice of time range for partial-time barrier options
    struct PartialBarrier {
        enum Range { Start = 0,  /*!< Monitor the barrier from the
                                      start of the option lifetime
                                      until the so-called cover event. */
                     EndB1 = 2,  /*!< Monitor the barrier from the
                                      cover event to the exercise
                                      date; trigger a knock-out only
                                      if the barrier is hit or crossed
                                      from either side, regardless of
                                      the underlying value when
                                      monitoring starts. */
                     EndB2 = 3   /*!< Monitor the barrier from the
                                      cover event to the exercise
                                      date; immediately trigger a
                                      knock-out if the underlying
                                      value is on the wrong side of
                                      the barrier when monitoring
                                      starts. */
        };
    };

    //! Partial-time %barrier %option
    /*! A particular type of %barrier %option in which the barrier is
        only monitored for a part of the option's lifetime; either
        from start to a so-called cover event, or from the cover event
        to the exercise date.
    */
    class PartialTimeBarrierOption : public OneAssetOption {
      public:
        class arguments;
        class engine;
        PartialTimeBarrierOption(
            Barrier::Type barrierType,
            PartialBarrier::Range barrierRange,
            Real barrier,
            Real rebate,
            Date coverEventDate,
            const ext::shared_ptr<StrikedTypePayoff>& payoff,
            const ext::shared_ptr<Exercise>& exercise);
        void setupArguments(PricingEngine::arguments*) const override;

      protected:
        Barrier::Type barrierType_;
        PartialBarrier::Range barrierRange_;
        Real barrier_;
        Real rebate_;
        Date coverEventDate_;
    };

    //! %Arguments for partial-time %barrier %option calculation
    class PartialTimeBarrierOption::arguments
        : public OneAssetOption::arguments {
      public:
        arguments();
        Barrier::Type barrierType;
        PartialBarrier::Range barrierRange;
        Real barrier;
        Real rebate;
        Date coverEventDate;
        void validate() const override;
    };

    //! Base class for partial-time %barrier %option engines
    class PartialTimeBarrierOption::engine
        : public GenericEngine<PartialTimeBarrierOption::arguments,
                               PartialTimeBarrierOption::results> {
    };

}


#endif
