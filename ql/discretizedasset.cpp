
/*
 Copyright (C) 2001, 2002, 2003 Sadruddin Rejeb

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email quantlib-dev@lists.sf.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/discretizedasset.hpp>

namespace QuantLib {

    void DiscretizedOption::postAdjustValues() {
        /* In the real world, with time flowing forward, first
           any payment is settled and only after options can be
           exercised. Here, with time flowing backward, options
           must be exercised before performing the adjustment.
        */
        method()->rollAlmostBack(underlying_, time());
        switch (exerciseType_) {
          case Exercise::American:
            if (time_ >= exerciseTimes_[0] && time_ <= exerciseTimes_[1])
                applyExerciseCondition();
            break;
          default:
            for (Size i=0; i<exerciseTimes_.size(); i++) {
                Time t = exerciseTimes_[i];
                if (t >= 0.0 && isOnTime(t))
                    applyExerciseCondition();
            }
        }
        underlying_->postAdjustValues();
    }

}

