
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

/*! \file grid.hpp
    \brief Grid classes with useful constructors for trees and finite diffs
*/

#ifndef quantlib_time_grid_h
#define quantlib_time_grid_h

#include <ql/dataformatters.hpp>
#include <list>
#include <vector>

namespace QuantLib {

    Disposable<Array> CenteredGrid(Real center, Real dx, Size steps);
    Disposable<Array> BoundedGrid(Real xMin, Real xMax, Size steps);

    //! time grid class
    /*! \todo What was the rationale for limiting the grid to
              positive times? Investigate and see whether we
              can use it for negative ones as well.
    */
    class TimeGrid : public std::vector<Time> {
      public:
        TimeGrid() {}
        //! Regularly spaced time-grid
        TimeGrid(Time end, Size steps);
        //! Time grid with mandatory time points
        /*! Mandatory points are guaranteed to belong to the grid.
            No additional points are added.
        */
        template <class Iterator>
        TimeGrid(Iterator begin, Iterator end)
        #if defined(QL_FULL_ITERATOR_SUPPORT)
        : mandatoryTimes_(begin, end) {
        #else
        {
            while (begin != end)
                mandatoryTimes_.push_back(*(begin++));
        #endif
            std::sort(mandatoryTimes_.begin(),mandatoryTimes_.end());
            // We seem to assume that the grid begins at 0.
            // Let's enforce the assumption for the time being
            // (even though I'm not sure that I agree.)
            QL_REQUIRE(mandatoryTimes_.front() >= 0.0,
                       "negative times not allowed");
            std::vector<Time>::iterator e = 
                std::unique(mandatoryTimes_.begin(),mandatoryTimes_.end());
            mandatoryTimes_.resize(e - mandatoryTimes_.begin());

            if (mandatoryTimes_[0] > 0.0)
                push_back(0.0);

            std::copy(mandatoryTimes_.begin(),mandatoryTimes_.end(),
                      std::back_inserter(*this));

            std::adjacent_difference(this->begin()+1,this->end(),
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
        #if defined(QL_FULL_ITERATOR_SUPPORT)
        : mandatoryTimes_(begin, end) {
        #else
        {
            while (begin != end)
                mandatoryTimes_.push_back(*(begin++));
        #endif
            std::sort(mandatoryTimes_.begin(),mandatoryTimes_.end());
            // We seem to assume that the grid begins at 0.
            // Let's enforce the assumption for the time being
            // (even though I'm not sure that I agree.)
            QL_REQUIRE(mandatoryTimes_.front() >= 0.0,
                       "negative times not allowed");
            std::vector<Time>::iterator e = 
                std::unique(mandatoryTimes_.begin(),mandatoryTimes_.end());
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
            push_back(periodBegin);
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
                    for (Size n=1; n<=nSteps; n++)
                        push_back(periodBegin + n*dt);
                }
                periodBegin = periodEnd;
            }

            std::adjacent_difference(this->begin()+1,this->end(),
                                     std::back_inserter(dt_));
        }
        Size findIndex(Time t) const;
        const std::vector<Time>& mandatoryTimes() const { 
            return mandatoryTimes_; 
        }
        Time dt(Size i) const;
      private:
        std::vector<Time> dt_;
        std::vector<Time> mandatoryTimes_;
    };

    // inline definitions

    inline Disposable<Array> CenteredGrid(Real center, Real dx, 
                                          Size steps) {
        Array result(steps+1);
        for (Size i=0; i<steps+1; i++)
            result[i] = center + (i - steps/2.0)*dx;
        return result;
    }

    inline Disposable<Array> BoundedGrid(Real xMin, Real xMax, 
                                         Size steps) {
        Array result(steps+1);
        Real x=xMin, dx=(xMax-xMin)/steps;
        for (Size i=0; i<steps+1; i++, x+=dx)
            result[i] = x;
        return result;
    }


    inline TimeGrid::TimeGrid(Time end, Size steps) {
        // We seem to assume that the grid begins at 0.
        // Let's enforce the assumption for the time being
        // (even though I'm not sure that I agree.)
        QL_REQUIRE(end > 0.0,
                   "negative times not allowed");
        Time dt = end/steps;
        for (Size i=0; i<=steps; i++)
            push_back(dt*i);

        mandatoryTimes_ = std::vector<Time>(1);
        mandatoryTimes_[0] = end;

        dt_ = std::vector<Time>(steps,dt);
    }

    inline Time TimeGrid::dt(Size i) const {
        return dt_[i];
    }

}


#endif
