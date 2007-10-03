/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2001, 2002, 2003 Sadruddin Rejeb
 Copyright (C) 2005, 2006 StatPro Italia srl

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

/*! \file timegrid.hpp
    \brief discrete time grid
*/

#ifndef quantlib_time_grid_hpp
#define quantlib_time_grid_hpp

#include <ql/errors.hpp>
#include <ql/math/comparison.hpp>
#include <vector>
#include <numeric>

namespace QuantLib {

    //! time grid class
    /*! \todo what was the rationale for limiting the grid to
              positive times? Investigate and see whether we
              can use it for negative ones as well.
    */
    class TimeGrid {
      public:
        //! \name Constructors
        //@{
        TimeGrid() {}
        //! Regularly spaced time-grid
        TimeGrid(Time end, Size steps);
        //! Time grid with mandatory time points
        /*! Mandatory points are guaranteed to belong to the grid.
            No additional points are added.
        */
        template <class Iterator>
        TimeGrid(Iterator begin, Iterator end)
        : mandatoryTimes_(begin, end) {
            std::sort(mandatoryTimes_.begin(),mandatoryTimes_.end());
            // We seem to assume that the grid begins at 0.
            // Let's enforce the assumption for the time being
            // (even though I'm not sure that I agree.)
            QL_REQUIRE(mandatoryTimes_.front() >= 0.0,
                       "negative times not allowed");
            std::vector<Time>::iterator e =
                std::unique(mandatoryTimes_.begin(),mandatoryTimes_.end(),
                            std::ptr_fun(close_enough));
            mandatoryTimes_.resize(e - mandatoryTimes_.begin());

            if (mandatoryTimes_[0] > 0.0)
                times_.push_back(0.0);

            times_.insert(times_.end(),
                          mandatoryTimes_.begin(), mandatoryTimes_.end());

            std::adjacent_difference(times_.begin()+1,times_.end(),
                                     std::back_inserter(dt_));

        }
        //! Time grid with mandatory time points
        /*! Mandatory points are guaranteed to belong to the grid.
            Additional points are then added with regular spacing
            between pairs of mandatory times in order to reach the
            desired number of steps.
        */
        template <class Iterator>
        TimeGrid(Iterator begin, Iterator end, Size steps)
        : mandatoryTimes_(begin, end) {
            std::sort(mandatoryTimes_.begin(),mandatoryTimes_.end());
            // We seem to assume that the grid begins at 0.
            // Let's enforce the assumption for the time being
            // (even though I'm not sure that I agree.)
            QL_REQUIRE(mandatoryTimes_.front() >= 0.0,
                       "negative times not allowed");
            std::vector<Time>::iterator e =
                std::unique(mandatoryTimes_.begin(),mandatoryTimes_.end(),
                            std::ptr_fun(close_enough));
            mandatoryTimes_.resize(e - mandatoryTimes_.begin());

            Time last = mandatoryTimes_.back();
            Time dtMax;
            // The resulting timegrid have points at times listed in the input
            // list. Between these points, there are inner-points which are
            // regularly spaced.
            if (steps == 0) {
                std::vector<Time> diff;
                std::adjacent_difference(mandatoryTimes_.begin(),
                                         mandatoryTimes_.end(),
                                         std::back_inserter(diff));
                if (diff.front()==0.0)
                    diff.erase(diff.begin());
                dtMax = *(std::min_element(diff.begin(), diff.end()));
            } else {
                dtMax = last/steps;
            }

            Time periodBegin = 0.0;
            times_.push_back(periodBegin);
            for (std::vector<Time>::const_iterator t=mandatoryTimes_.begin();
                                                   t<mandatoryTimes_.end();
                                                   t++) {
                Time periodEnd = *t;
                if (periodEnd != 0.0) {
                    // the nearest integer
                    Size nSteps = Size((periodEnd - periodBegin)/dtMax+0.5);
                    // at least one time step!
                    nSteps = (nSteps!=0 ? nSteps : 1);
                    Time dt = (periodEnd - periodBegin)/nSteps;
                    times_.reserve(nSteps);
                    for (Size n=1; n<=nSteps; ++n)
                        times_.push_back(periodBegin + n*dt);
                }
                periodBegin = periodEnd;
            }

            std::adjacent_difference(times_.begin()+1,times_.end(),
                                     std::back_inserter(dt_));
        }
        //@}
        //! \name Time grid interface
        //@{
        //! returns the index i such that grid[i] = t
        Size index(Time t) const;
        //! returns the index i such that grid[i] is closest to t
        Size closestIndex(Time t) const;
        //! returns the time on the grid closest to the given t
        Time closestTime(Time t) const {
            return times_[closestIndex(t)];
        }
        const std::vector<Time>& mandatoryTimes() const {
            return mandatoryTimes_;
        }
        Time dt(Size i) const { return dt_[i]; }
        //@}
        //! \name sequence interface
        //@{
        typedef std::vector<Time>::const_iterator const_iterator;
        typedef std::vector<Time>::const_reverse_iterator
                                          const_reverse_iterator;

        Time operator[](Size i) const { return times_[i]; }
        Time at(Size i) const { return times_.at(i); }
        Size size() const { return times_.size(); }
        bool empty() const { return times_.empty(); }
        const_iterator begin() const { return times_.begin(); }
        const_iterator end() const { return times_.end(); }
        const_reverse_iterator rbegin() const { return times_.rbegin(); }
        const_reverse_iterator rend() const { return times_.rend(); }
        Time front() const { return times_.front(); }
        Time back() const { return times_.back(); }
        //@}
      private:
        std::vector<Time> times_;
        std::vector<Time> dt_;
        std::vector<Time> mandatoryTimes_;
    };

}


#endif
