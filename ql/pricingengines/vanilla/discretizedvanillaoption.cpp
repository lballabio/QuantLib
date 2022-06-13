/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2002, 2003 Sadruddin Rejeb
 Copyright (C) 2004 StatPro Italia srl

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

#include <ql/pricingengines/vanilla/discretizedvanillaoption.hpp>
#include <vector>

namespace QuantLib {

    DiscretizedVanillaOption::DiscretizedVanillaOption(
                                         const VanillaOption::arguments& args,
                                         const StochasticProcess& process,
                                         const TimeGrid& grid)
    : arguments_(args) {
        stoppingTimes_.resize(args.exercise->dates().size());
        for (Size i=0; i<stoppingTimes_.size(); ++i) {
            stoppingTimes_[i] =
                process.time(args.exercise->date(i));
            if (!grid.empty()) {
                // adjust to the given grid
                stoppingTimes_[i] = grid.closestTime(stoppingTimes_[i]);
            }
        }
    }

    void DiscretizedVanillaOption::reset(Size size) {
        values_ = Array(size, 0.0);
        adjustValues();
    }

    void DiscretizedVanillaOption::postAdjustValuesImpl() {

        Time now = time();
        switch (arguments_.exercise->type()) {
          case Exercise::American:
            if (now <= stoppingTimes_[1] &&
                now >= stoppingTimes_[0])
                applySpecificCondition();
            break;
          case Exercise::European:
            if (isOnTime(stoppingTimes_[0]))
                applySpecificCondition();
            break;
          case Exercise::Bermudan:
              for (Real stoppingTime : stoppingTimes_) {
                  if (isOnTime(stoppingTime))
                      applySpecificCondition();
              }
            break;
          default:
            QL_FAIL("invalid option type");
        }
    }

    void DiscretizedVanillaOption::applySpecificCondition() {
        Array grid = method()->grid(time());
        for (Size j=0; j<values_.size(); j++) {
            values_[j] = std::max(values_[j],
                                  (*arguments_.payoff)(grid[j]));
        }
    }

}

