
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
#include <list>
#include <vector>

namespace QuantLib {

    //! spatial grid class
    class Grid : public Array {
      public:
        Grid(double center, double dx, Size steps);
    };

    //! time grid class
    class TimeGrid : public std::vector<Time> {
      public:
        TimeGrid() {}
        //! Regularly spaced time-grid
        TimeGrid(Time end, Size steps);
        //! Time grid with mandatory time-points (regularly spaced between them)
        TimeGrid(const std::list<Time>& times, Size steps);

        Size findIndex(Time t) const;

        Time dt(Size i) const;
    };

    //Inline definitions

    inline Grid::Grid(double center, double dx, Size steps) : Array(steps) {
        for (Size i=0; i<steps; i++)
            (*this)[i] = center + (i - steps/2.0)*dx;
    }

    inline TimeGrid::TimeGrid(Time end, Size steps) {
        Time dt = end/steps;
        for (Size i=0; i<=steps; i++)
            push_back(dt*i);
    }

    inline Time TimeGrid::dt(Size i) const {
        return (*this)[i+1]  - (*this)[i];
    }

}


#endif
