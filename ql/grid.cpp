
/*
 Copyright (C) 2001, 2002, 2003 Sadruddin Rejeb

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email ferdinando@ametrano.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file grid.cpp
    \brief Grid classes with useful constructors for trees and finite diffs

    \fullpath
    ql/%grid.cpp
*/

// $Id$

#include <ql/grid.hpp>

namespace {

    class CloseEnoughTo : public std::unary_function<double,bool> {
      public:
        CloseEnoughTo(double target, double tolerance = 1.0e-10)
        : target_(target), tolerance_(tolerance) {}
        bool operator()(double x) const {
            return QL_FABS(x-target_) <= tolerance_;
        }
      private:
        double target_, tolerance_;
    };

}

namespace QuantLib {

    TimeGrid::TimeGrid(const std::list<Time>& times, Size steps)
    : std::vector<Time>(0) {
        Time last = times.back();
        Time dtMax;
        // The resulting timegrid have points at times listed in the input
        // list. Between these points, there are inner-points which are
        // regularly spaced.
        if (steps == 0) {
            std::vector<Time> diff;
            std::back_insert_iterator<std::vector<Time> > ii(diff);
            std::adjacent_difference(times.begin(), times.end(), ii);
            if (diff.front()==0.0)
                diff.erase(diff.begin());
            dtMax = *(std::min_element(diff.begin(), diff.end()));
        } else {
            dtMax = last/steps;
        }

        Time periodBegin = 0.0;
        std::list<Time>::const_iterator t;
        for (t = times.begin(); t != times.end(); t++) {
            Time periodEnd = *t;
            if (periodBegin >= periodEnd) // Should we use a QL_REQUIRE?
                continue;
            Size nSteps = (Size)((periodEnd - periodBegin)/dtMax + 1.0);
            double dt = (periodEnd - periodBegin)/nSteps;
            for (Size n=0; n<nSteps; n++)
                push_back(periodBegin + n*dt);
            periodBegin = periodEnd;
        }
        push_back(periodBegin); // Note periodBegin = periodEnd
    }

    Size TimeGrid::findIndex(Time t) const {
        const_iterator result = std::find_if(begin(), end(), 
                                             CloseEnoughTo(t));
        if (result == end()) {
            int i;
            for (i=0; i<size(); i++) {
                if ((*this)[i] > t)
                    break;
            }
            if (i == 0) {
                throw Error("Using inadequate tree: all nodes "
                            "are later than the required time t = "
                            + DoubleFormatter::toString(t,12) +
                            " (earliest node is t1 = "
                            + DoubleFormatter::toString((*this)[0],12) +
                            ")");
            } else if (i == size()) {
                throw Error("Using inadequate tree: all nodes "
                            "are earlier than the required time t = "
                            + DoubleFormatter::toString(t,12) +
                            " (latest node is t1 = "
                            + DoubleFormatter::toString((*this)[size()-1],12) +
                            ")");
            } else {
                throw Error("Using inadequate tree: the nodes closest "
                            "to the required time t = "
                            + DoubleFormatter::toString(t,12) +
                            " are t1 = " 
                            + DoubleFormatter::toString((*this)[i-1],12) +
                            " and t2 = "
                            + DoubleFormatter::toString((*this)[i],12));
            }
        }
        return result - begin();
    }

}

