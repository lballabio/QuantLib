
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

/*! \file grid.hpp
    \brief Grid classes with useful constructors for trees and finite diffs

    \fullpath
    ql/%grid.hpp
*/

// $Id$

#ifndef quantlib_time_grid_h
#define quantlib_time_grid_h

#include <ql/array.hpp>
#include <ql/dataformatters.hpp>
#include <iterator>
#include <list>
#include <vector>

namespace QuantLib {

    //! spatial grid class
    class Grid : public Array {
      public:
        Grid(double center, double dx, Size steps);
    };

    inline Grid::Grid(double center, double dx, Size steps) : Array(steps) {
        for (Size i=0; i<steps; i++)
            (*this)[i] = center + (i - steps/2.0)*dx;
    }

    
    //! time grid class
    class TimeGrid : public std::vector<Time> {
      public:
        TimeGrid() {}
        //! Regularly spaced time-grid
        TimeGrid(Time end, Size steps);
        //! Time grid with mandatory time-points (regularly spaced between them)
        template <class Iterator>
        TimeGrid(Iterator begin, Iterator end, Size steps)
        : mandatoryTimeIndex_(1, 0)
            #if defined(QL_FULL_ITERATOR_SUPPORT)
            , mandatoryTimes_(begin, end) {
            #else
            {
            while (begin != end)
                mandatoryTimes_.push_back(*(begin++));
            #endif

            // sort unique
            std::sort(mandatoryTimes_.begin(), mandatoryTimes_.end());
            std::vector<Time>::iterator uniqueEnd = std::unique(
                mandatoryTimes_.begin(), mandatoryTimes_.end());
            mandatoryTimes_.resize(uniqueEnd-mandatoryTimes_.begin());

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
            std::vector<Time>::const_iterator t;
            for (t=mandatoryTimes_.begin(); t<mandatoryTimes_.end(); t++) {
                // mandatory time's position (index) in the time grid
                mandatoryTimeIndex_.push_back(size());
                Time periodEnd = *t;
                Size nSteps = (Size)((periodEnd - periodBegin)/dtMax + 1.0);
                double dt = (periodEnd - periodBegin)/nSteps;
                for (Size n=0; n<nSteps; n++) {
                    push_back(periodBegin + n*dt);
                }
                periodBegin = periodEnd;
            }
            mandatoryTimeIndex_.push_back(size());
            push_back(periodBegin); // Note periodBegin = periodEnd

            std::adjacent_difference(this->begin(), this->end(), std::back_inserter(dt_));
        }
        Size findIndex(Time t) const;
        Time dt(Size i) const {return dt_[i];}
      private:
          std::vector<Time> dt_;
          std::vector<Time> mandatoryTimes_;
          std::vector<Size> mandatoryTimeIndex_;
    };


    inline TimeGrid::TimeGrid(Time end, Size steps) {
        Time dt = end/steps;
        for (Size i=0; i<=steps; i++)
            push_back(dt*i);

        dt_ = std::vector<Time>(steps, dt);

        mandatoryTimes_ = std::vector<Time>(2);
        mandatoryTimes_[0] = 0;
        mandatoryTimes_[1] = end;

        mandatoryTimeIndex_ = std::vector<Size>(2);
        mandatoryTimeIndex_[0] = 0;
        mandatoryTimeIndex_[1] = steps+1;
    }

}


#endif
