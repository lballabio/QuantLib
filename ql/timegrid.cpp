/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2001, 2002, 2003 Sadruddin Rejeb
 Copyright (C) 2005 StatPro Italia srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/reference/license.html>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/timegrid.hpp>
#include <ql/Math/comparison.hpp>
#include <ql/errors.hpp>
#include <iomanip>

namespace QuantLib {

    namespace {

        class CloseEnoughTo : public std::unary_function<Real,bool> {
          public:
            CloseEnoughTo(Real target) : target_(target) {}
            bool operator()(Real x) const {
                return close_enough(x,target_);
            }
          private:
            Real target_;
        };

    }


    TimeGrid::TimeGrid(Time end, Size steps) {
        // We seem to assume that the grid begins at 0.
        // Let's enforce the assumption for the time being
        // (even though I'm not sure that I agree.)
        QL_REQUIRE(end > 0.0,
                   "negative times not allowed");
        Time dt = end/steps;
        for (Size i=0; i<=steps; i++)
            times_.push_back(dt*i);

        mandatoryTimes_ = std::vector<Time>(1);
        mandatoryTimes_[0] = end;

        dt_ = std::vector<Time>(steps,dt);
    }


    Size TimeGrid::findIndex(Time t) const {
        const_iterator result = std::find_if(times_.begin(), times_.end(),
                                             CloseEnoughTo(t));
        if (result == end()) {
            Size i;
            for (i=0; i<size(); i++) {
                if (times_[i] > t)
                    break;
            }
            if (i == 0) {
                QL_FAIL("using inadequate time grid: all nodes "
                        "are later than the required time t = "
                        << std::setprecision(12) << t
                        << " (earliest node is t1 = "
                        << std::setprecision(12) << times_.front() << ")");
            } else if (i == size()) {
                QL_FAIL("using inadequate time grid: all nodes "
                        "are earlier than the required time t = "
                        << std::setprecision(12) << t
                        << " (latest node is t1 = "
                        << std::setprecision(12) << times_.back() << ")");
            } else {
                QL_FAIL("using inadequate time grid: the nodes closest "
                        "to the required time t = "
                        << std::setprecision(12) << t
                        << " are t1 = "
                        << std::setprecision(12) << times_[i-1]
                        << " and t2 = "
                        << std::setprecision(12) << times_[i]);
            }
        }
        return result - times_.begin();
    }

}

