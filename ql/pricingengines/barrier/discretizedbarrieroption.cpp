/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2014 Thema Consulting SA

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
        QL_REQUIRE(!args.exercise->dates().empty(), "specify at least one stopping date");

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
        if (arguments_.barrierType==Barrier::DownIn ||
                     arguments_.barrierType==Barrier::UpIn) {
            vanilla_.rollback(time());
        }
        Array grid = method()->grid(time());
        checkBarrier(values_, grid);
    }

    void DiscretizedBarrierOption::checkBarrier(Array &optvalues, const Array &grid) const {

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
              for (Real i : stoppingTimes_) {
                  if (isOnTime(i)) {
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
              case Barrier::DownIn:
                  if (grid[j] <= arguments_.barrier) {
                     // knocked in
                     if (stoppingTime) {
                        optvalues[j] = std::max(vanilla_.values()[j],
                                      (*arguments_.payoff)(grid[j]));
                     }
                     else
                         optvalues[j] = vanilla_.values()[j]; 
                  }
                  else if (endTime)
                      optvalues[j] = arguments_.rebate;
                  break;
              case Barrier::DownOut:
                  if (grid[j] <= arguments_.barrier)
                      optvalues[j] = arguments_.rebate; // knocked out
                  else if (stoppingTime) {
                      optvalues[j] = std::max(optvalues[j],
                                     (*arguments_.payoff)(grid[j]));
                  }
                  break;
              case Barrier::UpIn:
                  if (grid[j] >= arguments_.barrier) {
                     // knocked in
                     if (stoppingTime) {
                         optvalues[j] = std::max(vanilla_.values()[j],
                                      (*arguments_.payoff)(grid[j]));
                     }
                     else
                         optvalues[j] = vanilla_.values()[j]; 
                  }
                  else if (endTime)
                      optvalues[j] = arguments_.rebate;
                  break;
              case Barrier::UpOut:
                  if (grid[j] >= arguments_.barrier)
                     optvalues[j] = arguments_.rebate; // knocked out
                  else if (stoppingTime)
                      optvalues[j] = std::max(optvalues[j],
                                     (*arguments_.payoff)(grid[j]));
                  break;
              default:
                  QL_FAIL("invalid barrier type");
            }
        }
    }



    DiscretizedDermanKaniBarrierOption::DiscretizedDermanKaniBarrierOption(
                                         const BarrierOption::arguments& args,
                                         const StochasticProcess& process,
                                         const TimeGrid& grid)
    : unenhanced_(args, process, grid) {
    }

    void DiscretizedDermanKaniBarrierOption::reset(Size size) {
        unenhanced_.initialize(method(), time());
        values_ = Array(size, 0.0);
        adjustValues();
    }

    void DiscretizedDermanKaniBarrierOption::postAdjustValuesImpl() {
        unenhanced_.rollback(time());

        Array grid = method()->grid(time());
        adjustBarrier(values_, grid);
        unenhanced_.checkBarrier(values_, grid); // compute payoffs
    }

    void DiscretizedDermanKaniBarrierOption::adjustBarrier(Array &optvalues, const Array &grid) {
        Real barrier = unenhanced_.arguments().barrier;
        Real rebate = unenhanced_.arguments().rebate;
        switch (unenhanced_.arguments().barrierType) {
           case Barrier::DownIn:
              for (Size j=0; j<optvalues.size()-1; ++j) {
                  if (grid[j]<=barrier && grid[j+1] > barrier) {
                      // grid[j+1] above barrier, grid[j] under (in),
                      // interpolate optvalues[j+1]
                      Real ltob = (barrier-grid[j]);
                      Real htob = (grid[j+1]-barrier);
                      Real htol = (grid[j+1]-grid[j]);
                      Real u1 = unenhanced_.values()[j+1];
                      Real t1 = unenhanced_.vanilla()[j+1];
                      optvalues[j+1] = std::max(0.0, (ltob*t1+htob*u1)/htol);
                  }
              }
              break;
           case Barrier::DownOut:
              for (Size j=0; j<optvalues.size()-1; ++j) {
                  if (grid[j]<=barrier && grid[j+1] > barrier) {
                      // grid[j+1] above barrier, grid[j] under (out),
                      // interpolate optvalues[j+1]
                      Real a = (barrier-grid[j])*rebate;
                      Real b = (grid[j+1]-barrier)*unenhanced_.values()[j+1];
                      Real c = (grid[j+1]-grid[j]);
                      optvalues[j+1] = std::max(0.0, (a+b)/c);
                  }
              }
              break;
           case Barrier::UpIn:
              for (Size j=0; j<optvalues.size()-1; ++j) {
                  if (grid[j] < barrier && grid[j+1] >= barrier) {
                      // grid[j+1] above barrier (in), grid[j] under, 
                      // interpolate optvalues[j]
                      Real ltob = (barrier-grid[j]);
                      Real htob = (grid[j+1]-barrier);
                      Real htol = (grid[j+1]-grid[j]);
                      Real u = unenhanced_.values()[j];
                      Real t = unenhanced_.vanilla()[j];
                      optvalues[j] = std::max(0.0, (ltob*u+htob*t)/htol); // derman std
                  }
               }
              break;
           case Barrier::UpOut:
              for (Size j=0; j<optvalues.size()-1; ++j) {
                  if (grid[j] < barrier && grid[j+1] >= barrier) {
                      // grid[j+1] above barrier (out), grid[j] under, 
                      // interpolate optvalues[j]
                      Real a = (barrier-grid[j])*unenhanced_.values()[j];
                      Real b = (grid[j+1]-barrier)*rebate;
                      Real c = (grid[j+1]-grid[j]);
                      optvalues[j] = std::max(0.0, (a+b)/c);
                  }
              }
              break;
        }
    }

}

