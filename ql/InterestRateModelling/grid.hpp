
/*
 Copyright (C) 2000, 2001, 2002 Sadruddin Rejeb

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software developed by the QuantLib Group; you can
 redistribute it and/or modify it under the terms of the QuantLib License;
 either version 1.0, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 QuantLib License for more details.

 You should have received a copy of the QuantLib License along with this
 program; if not, please email ferdinando@ametrano.net

 The QuantLib License is also available at http://quantlib.org/license.html
 The members of the QuantLib Group are listed in the QuantLib License
*/
/*! \file grid.hpp
    \brief Grid constructor for finite differences pricers

    \fullpath
    ql/InterestRateModelling/%grid.hpp
*/

// $Id$


#ifndef quantlib_interest_rate_modelling_grid_h
#define quantlib_interest_rate_modelling_grid_h

#include "ql/InterestRateModelling/onefactormodel.hpp"

namespace QuantLib {

    namespace InterestRateModelling {

        class Grid : public Array {
          public:
            Grid(Size gridPoints,
                 double initialCenter,
                 double strikeCenter,
                 Time residualTime,
                 Time timeDelay,
                 const OneFactorModel& model);
            double dx() { return dx_;}
            Size index() const {return index_;}
            Size safeGridPoints(Size gridPoints, Time residualTime) const;
          private:
            void initialize(Size gridPoints,
                            double initialCenter,
                            double strikeCenter,
                            Time residualTime,
                            Time timeDelay,
                            const OneFactorModel& model);
            double dx_;
            Size index_;

        };
    }

}

#endif
