/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2014 Master IMAFA - Polytech'Nice Sophia - Université de Nice Sophia Antipolis

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

    struct PartialBarrier : public Barrier {
        enum Range { Start, End, EndB1, EndB2 };
    };

    class PartialTimeBarrierOption : public OneAssetOption {
      public:
        class arguments;
        class engine;
        PartialTimeBarrierOption(PartialBarrier::Type barrierType,
            PartialBarrier::Range barrierRange,
            Real barrier,
            Real rebate,
            Date coverEventDate,
            const ext::shared_ptr<StrikedTypePayoff>& payoff,
            const ext::shared_ptr<Exercise>& exercise);
        void setupArguments(PricingEngine::arguments*) const override;

      protected:
        PartialBarrier::Type barrierType_;
        PartialBarrier::Range barrierRange_;
        Real barrier_;
        Real rebate_;
        Date coverEventDate_;
    };

    //! %Arguments for barrier option calculation
    class PartialTimeBarrierOption::arguments
        : public OneAssetOption::arguments {
      public:
        arguments();
        PartialBarrier::Type barrierType;
        PartialBarrier::Range barrierRange;
        Real barrier;
        Real rebate;
        Date coverEventDate;
        void validate() const override;
    };

    //! %Partial-Time-Barrier-Option %engine base class
    class PartialTimeBarrierOption::engine
        : public GenericEngine<PartialTimeBarrierOption::arguments,
                               PartialTimeBarrierOption::results> {
    };

}


#endif


#ifndef id_46a2b071a5bfcc31b5deaf9ceb9abf5e
#define id_46a2b071a5bfcc31b5deaf9ceb9abf5e
inline bool test_46a2b071a5bfcc31b5deaf9ceb9abf5e(const int* i) {
    return i != nullptr;
}
#endif
