/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2012 Riccardo Ghetta, Thema Consulting SA

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

#include <ql/pricingengines/barrier/discretizedbarrieroption.hpp>
#include <vector>

namespace QuantLib {

    DiscretizedBarrierOption::DiscretizedBarrierOption(
                                         const BarrierOption::arguments& args,
                                         const StochasticProcess& process,
                                         const TimeGrid& grid)
    : arguments_(args), vanilla_(arguments_, process, grid) {
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

    void DiscretizedBarrierOption::reset(Size size) {
        vanilla_.initialize(method(), time());
        values_ = Array(size, 0.0);
        adjustValues();
    }

    void DiscretizedBarrierOption::postAdjustValuesImpl() {
        Array grid = method()->grid(time());
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
        if (arguments_.barrierType==Barrier::DownIn ||
                     arguments_.barrierType==Barrier::UpIn) {
            vanilla_.rollback(now);  
        }
        checkBarrier(grid, endTime, stoppingTime);
    }

    void DiscretizedBarrierOption::checkBarrier(const Array &grid, bool endTime, bool stoppingTime) {

        for (Size j=0; j<values_.size(); j++) {
            switch (arguments_.barrierType) {
              case Barrier::DownIn:
                  if (grid[j] <= arguments_.barrier) {
                     // knocked in
                     if (stoppingTime) {
                         values_[j] = std::max(vanilla_.values()[j],
                                      (*arguments_.payoff)(grid[j]));
                     }
                     else
                         values_[j] = vanilla_.values()[j]; 
                  }
                  else if (endTime)
                      values_[j] = arguments_.rebate;
                  break;
              case Barrier::DownOut:
                  if (grid[j] <= arguments_.barrier)
                      values_[j] = arguments_.rebate; // knocked out
                  else if (stoppingTime)
                      values_[j] = std::max(values()[j],
                                     (*arguments_.payoff)(grid[j]));
                  break;
              case Barrier::UpIn:
                  if (grid[j] >= arguments_.barrier) {
                     // knocked in
                     if (stoppingTime) {
                         values_[j] = std::max(vanilla_.values()[j],
                                      (*arguments_.payoff)(grid[j]));
                     }
                     else
                         values_[j] = vanilla_.values()[j]; 
                  }
                  else if (endTime)
                      values_[j] = arguments_.rebate;
                  break;
              case Barrier::UpOut:
                  if (grid[j] >= arguments_.barrier)
                     values_[j] = arguments_.rebate; // knocked out
                  else if (stoppingTime)
                      values_[j] = std::max(values()[j],
                                     (*arguments_.payoff)(grid[j]));
                  break;
              default:
                  QL_FAIL("invalid barrier type");
            }
        }
    }

}

