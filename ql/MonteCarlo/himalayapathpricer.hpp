
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
/*! \file himalayapathpricer.hpp
    \brief multipath pricer for European-type Himalaya option

    \fullpath
    ql/MonteCarlo/%himalayapathpricer.hpp

*/

// $Id$

#ifndef quantlib_himalaya_path_pricer_h
#define quantlib_himalaya_path_pricer_h


#include <ql/MonteCarlo/pathpricer.hpp>
#include <ql/MonteCarlo/multipath.hpp>

namespace QuantLib {

    namespace MonteCarlo {

        //! multipath pricer for European-type Himalaya option
        /*! The payoff of an himalaya option is computed in the following way:
            given a basket of N assets, and M time periods, at end of
            each period the option who performed the best is added to the
            average and then discarded from the basket. At the end of the
            M periods the option pays the max between the strike and the
            average of the best performers.
        */
        class HimalayaPathPricer : public PathPricer<MultiPath> {
          public:
            HimalayaPathPricer(const Array& underlying,
                               double strike,
                               DiscountFactor discount,
                               bool useAntitheticVariance);
            double operator()(const MultiPath& multiPath) const;
          private:
            Option::Type type_;
            Array underlying_;
            double strike_;
        };

    }

}

#endif
