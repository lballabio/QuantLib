

/*
 Copyright (C) 2000, 2001, 2002 RiskMap srl

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
