/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2012 Thema Consulting SA (programmer: Riccardo Ghetta)

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

#include <ql/pricingengines/barrier/discretizeddoublebarrieroption.hpp>
#include <vector>

namespace QuantLib {



    DiscretizedDoubleBarrierOption::DiscretizedDoubleBarrierOption(
                                         const DoubleBarrierOption::arguments& args,
                                         const StochasticProcess& process,
                                         const TimeGrid& grid)
    : arguments_(args), vanilla_(arguments_, process, grid)  {
        QL_REQUIRE(args.exercise->dates().size(), "specify at least one stopping date");

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

    void DiscretizedDoubleBarrierOption::reset(Size size) {
        vanilla_.initialize(method(), time());
        values_ = Array(size, 0.0);
        adjustValues();
    }

    void DiscretizedDoubleBarrierOption::postAdjustValuesImpl() {
        if (arguments_.barrierType==DoubleBarrier::KnockIn) {
            vanilla_.rollback(time());
        }
        Array grid = method()->grid(time());
        checkBarrier(values_, grid);
    }

    void DiscretizedDoubleBarrierOption::checkBarrier(Array &optvalues, const Array &grid) const {

        Time now = time();
        bool endTime = isOnTime(stoppingTimes_.back());
        bool stoppingTime = false;         
        switch (arguments_.exercise->type()) {
          case Exercise::American:
            if (now <= stoppingTimes_[1] &&
                now >= stoppingTimes_[0])
                stoppingTime = true;
            break;
          case Exercise::European:
            if (isOnTime(stoppingTimes_[0]))
                stoppingTime = true;
            break;
          case Exercise::Bermudan:
            for (Size i=0; i<stoppingTimes_.size(); i++) {
                if (isOnTime(stoppingTimes_[i])) {
                    stoppingTime = true;
                    break;
                }
            }
            break;
          default:
            QL_FAIL("invalid option type");
        }
        for (Size j=0; j<optvalues.size(); j++) {
            switch (arguments_.barrierType) {
              case DoubleBarrier::KnockIn:
                  if (grid[j] <= arguments_.barrier_lo) {
                     // knocked in dn
                     if (stoppingTime) {
                         optvalues[j] = std::max(vanilla()[j],
                                      (*arguments_.payoff)(grid[j]));
                     }
                     else
                         optvalues[j] = vanilla()[j]; 
                  }
                  else if (grid[j] >= arguments_.barrier_hi) {
                     // knocked in up
                     if (stoppingTime) {
                         optvalues[j] = std::max(vanilla()[j],
                                      (*arguments_.payoff)(grid[j]));
                     }
                     else
                         optvalues[j] = vanilla()[j]; 
                  }
                  else if (endTime)
                      optvalues[j] = arguments_.rebate;
                  break;
              case DoubleBarrier::KnockOut:
                  if (grid[j] <= arguments_.barrier_lo)
                      optvalues[j] = arguments_.rebate; // knocked out lo
                  else if (grid[j] >= arguments_.barrier_hi)
                     optvalues[j] = arguments_.rebate; // knocked out hi
                  else if (stoppingTime)
                      optvalues[j] = std::max(optvalues[j],
                                     (*arguments_.payoff)(grid[j]));
                  break;
              default:
                  QL_FAIL("invalid barrier type");
            }
        }
    }

}
