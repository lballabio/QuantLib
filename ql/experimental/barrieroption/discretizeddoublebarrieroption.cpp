/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2015 Thema Consulting SA

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

#include <ql/experimental/barrieroption/discretizeddoublebarrieroption.hpp>
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
        if (arguments_.barrierType!=DoubleBarrier::KnockOut) {
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
              case DoubleBarrier::KIKO:
                  // low barrier is KI, high is KO
                  if (grid[j] <= arguments_.barrier_lo) {
                     // knocked in dn
                     if (stoppingTime) {
                         optvalues[j] = std::max(vanilla()[j],
                                      (*arguments_.payoff)(grid[j]));
                     }
                     else
                         optvalues[j] = vanilla()[j];
                  }
                  else if (grid[j] >= arguments_.barrier_hi)
                     optvalues[j] = arguments_.rebate; // knocked out hi
                  else if (endTime)
                      optvalues[j] = arguments_.rebate;
                  break;
              case DoubleBarrier::KOKI:
                  // low barrier is KO, high is KI
                  if (grid[j] <= arguments_.barrier_lo)
                      optvalues[j] = arguments_.rebate; // knocked out lo
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
              default:
                  QL_FAIL("invalid barrier type");
            }
        }
    }



    DiscretizedDermanKaniDoubleBarrierOption::DiscretizedDermanKaniDoubleBarrierOption(
                                         const DoubleBarrierOption::arguments& args,
                                         const StochasticProcess& process,
                                         const TimeGrid& grid)
    : unenhanced_(args, process, grid) {
    }

    void DiscretizedDermanKaniDoubleBarrierOption::reset(Size size) {
        unenhanced_.initialize(method(), time());
        values_ = Array(size, 0.0);
        adjustValues();
    }

    void DiscretizedDermanKaniDoubleBarrierOption::postAdjustValuesImpl() {
        unenhanced_.rollback(time());

        Array grid = method()->grid(time());
        unenhanced_.checkBarrier(values_, grid); // compute payoffs
        adjustBarrier(values_, grid);
    }

    void DiscretizedDermanKaniDoubleBarrierOption::adjustBarrier(Array &optvalues, const Array &grid) {
        Real barrier_lo = unenhanced_.arguments().barrier_lo;
        Real barrier_hi = unenhanced_.arguments().barrier_hi;
        Real rebate = unenhanced_.arguments().rebate;
        switch (unenhanced_.arguments().barrierType) {
           case DoubleBarrier::KnockIn:
              for (Size j=0; j<optvalues.size()-1; ++j) {
                  if (grid[j]<=barrier_lo && grid[j+1] > barrier_lo) {
                     // grid[j+1] above barrier_lo, grid[j] under (in),
                     // interpolate optvalues[j+1]
                     Real ltob = (barrier_lo-grid[j]);
                     Real htob = (grid[j+1]-barrier_lo);
                     Real htol = (grid[j+1]-grid[j]);
                     Real u1 = unenhanced_.values()[j+1];
                     Real t1 = unenhanced_.vanilla()[j+1];
                     optvalues[j+1] = std::max(0.0, (ltob*t1+htob*u1)/htol); // derman std
                  }
                  else if (grid[j] < barrier_hi && grid[j+1] >= barrier_hi) {
                     // grid[j+1] above barrier_hi (in), grid[j] under, 
                     // interpolate optvalues[j]
                     Real ltob = (barrier_hi-grid[j]);
                     Real htob = (grid[j+1]-barrier_hi);
                     Real htol = (grid[j+1]-grid[j]);
                     Real u = unenhanced_.values()[j];
                     Real t = unenhanced_.vanilla()[j];
                     optvalues[j] = std::max(0.0, (ltob*u+htob*t)/htol); // derman std
                  }
              }
              break;
           case DoubleBarrier::KnockOut:
              for (Size j=0; j<optvalues.size()-1; ++j) {
                  if (grid[j]<=barrier_lo && grid[j+1] > barrier_lo) {
                     // grid[j+1] above barrier_lo, grid[j] under (out),
                     // interpolate optvalues[j+1]
                     Real a = (barrier_lo-grid[j])*rebate;
                     Real b = (grid[j+1]-barrier_lo)*unenhanced_.values()[j+1];
                     Real c = (grid[j+1]-grid[j]);
                     optvalues[j+1] = std::max(0.0, (a+b)/c);
                  }
                  else if (grid[j] < barrier_hi && grid[j+1] >= barrier_hi) {
                     // grid[j+1] above barrier_hi (out), grid[j] under, 
                     // interpolate optvalues[j]
                     Real a = (barrier_hi-grid[j])*unenhanced_.values()[j];
                     Real b = (grid[j+1]-barrier_hi)*rebate;
                     Real c = (grid[j+1]-grid[j]);
                     optvalues[j] = std::max(0.0, (a+b)/c);
                  }
              }
              break;
           default:
              QL_FAIL("unsupported barrier type");
              break;
        }
    }

}
