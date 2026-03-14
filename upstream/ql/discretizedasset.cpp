/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2001, 2002, 2003 Sadruddin Rejeb
 Copyright (C) 2004 StatPro Italia srl

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

#include <ql/discretizedasset.hpp>

namespace QuantLib {

    void DiscretizedOption::postAdjustValuesImpl() {
        /* In the real world, with time flowing forward, first
           any payment is settled and only after options can be
           exercised. Here, with time flowing backward, options
           must be exercised before performing the adjustment.
        */
        underlying_->partialRollback(time());
        underlying_->preAdjustValues();
        Size i;
        switch (exerciseType_) {
          case Exercise::American:
            if (time_ >= exerciseTimes_[0] && time_ <= exerciseTimes_[1])
                applyExerciseCondition();
            break;
          case Exercise::Bermudan:
          case Exercise::European:
            for (i=0; i<exerciseTimes_.size(); i++) {
                Time t = exerciseTimes_[i];
                if (t >= 0.0 && isOnTime(t))
                    applyExerciseCondition();
            }
            break;
          default:
            QL_FAIL("invalid exercise type");
        }
        underlying_->postAdjustValues();
    }

}

