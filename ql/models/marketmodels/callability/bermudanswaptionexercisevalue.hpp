/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Mark Joshi

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


#ifndef quantlib_bermudan_swaption_exercise_value_hpp
#define quantlib_bermudan_swaption_exercise_value_hpp

#include <ql/models/marketmodels/callability/exercisevalue.hpp>
#include <ql/models/marketmodels/evolutiondescription.hpp>
#include <boost/shared_ptr.hpp>
#include <valarray>

namespace QuantLib {

    class Payoff;

    class BermudanSwaptionExerciseValue : public MarketModelExerciseValue {
      public:
        BermudanSwaptionExerciseValue(const std::vector<Time>& rateTimes,
                                      const std::vector<boost::shared_ptr<Payoff> >&);
        Size numberOfExercises() const;
        // including any time at which state should be updated
        const EvolutionDescription& evolution() const;
        std::vector<Time> possibleCashFlowTimes() const;
        void nextStep(const CurveState&);
        void reset();
        // whether or not evolution times are exercise times
        std::valarray<bool> isExerciseTime() const;
        MarketModelMultiProduct::CashFlow value(const CurveState&) const;
        std::auto_ptr<MarketModelExerciseValue> clone() const;
      private:
        Size numberOfExercises_;
        std::vector<Time> rateTimes_;
        std::vector<boost::shared_ptr<Payoff> > payoffs_;
        EvolutionDescription evolution_;
        // evolving
        Size currentIndex_;
        MarketModelMultiProduct::CashFlow cf_;
    };

}


#endif
