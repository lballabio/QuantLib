
/*
 * Copyright (C) 2000-2001 QuantLib Group
 *
 * This file is part of QuantLib.
 * QuantLib is a C++ open source library for financial quantitative
 * analysts and developers --- http://quantlib.org/
 *
 * QuantLib is free software and you are allowed to use, copy, modify, merge,
 * publish, distribute, and/or sell copies of it under the conditions stated
 * in the QuantLib License.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the license for more details.
 *
 * You should have received a copy of the license along with this file;
 * if not, please email quantlib-users@lists.sourceforge.net
 * The license is also available at http://quantlib.org/LICENSE.TXT
 *
 * The members of the QuantLib Group are listed in the Authors.txt file, also
 * available at http://quantlib.org/group.html
*/

/*! \file timegrid.hpp
    \brief Time grid class with useful constructors

    \fullpath
    ql/InterestRateModelling/%timegrid.hpp
*/

// $Id$

#ifndef quantlib_lattices_time_grid_h
#define quantlib_lattices_time_grid_h

#include <ql/qldefines.hpp>

namespace QuantLib {

    namespace Lattices {

        class TimeGrid : public std::vector<Time> {
          public:
            TimeGrid() : std::vector<Time>(0) {}

            TimeGrid(const std::list<Time>& times, size_t steps) 
            : std::vector<Time>(0) {
                Time last = times.back();
                double dtMax = last/steps;

                Time begin = 0.0;
                std::list<Time>::const_iterator t;
                for (t = times.begin(); t != times.end(); t++) {
                    Time end = *t;
                    if (begin == end) continue;
                    unsigned nSteps = (unsigned int)((end - begin)/dtMax + 1.0);
                    double dt = (end - begin)/nSteps;
                    for (unsigned int n=0; n<nSteps; n++)
                        push_back(begin + n*dt);
                    begin = end;
                }
                push_back(begin);
            }

            unsigned int findIndex(Time t) const {
                for (size_t i=0; i<size(); i++) {
                    if ((*this)[i] == t)
                        return i;
                }
                throw Error("Using inadequate tree");
                QL_DUMMY_RETURN(0);
            }

            Time dt(unsigned int i) {
                return (*this)[i+1]  - (*this)[i];
            }

        };

    }

}

#endif
