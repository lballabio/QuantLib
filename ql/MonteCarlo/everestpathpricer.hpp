
/*
 Copyright (C) 2000, 2001, 2002 RiskMap srl

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
/*! \file everestpathpricer.hpp
    \brief path pricer for European-type Everest option

    \fullpath
    ql/MonteCarlo/%everestpathpricer.hpp

*/

// $Id$

#ifndef quantlib_everest_path_pricer_h
#define quantlib_everest_path_pricer_h

#include <ql/MonteCarlo/pathpricer.hpp>
#include <ql/MonteCarlo/multipath.hpp>

namespace QuantLib {

    namespace MonteCarlo {

        //! %path pricer for European-type Everest option
        /*! The payoff of an Everest option is given by the
            final-price initial-price ratio of the worst performer.
        */
        class EverestPathPricer : public PathPricer<MultiPath> {
          public:
            EverestPathPricer(DiscountFactor discount,
                              bool useAntitheticVariance);
            double operator()(const MultiPath& multiPath) const;
        };

    }

}


#endif
