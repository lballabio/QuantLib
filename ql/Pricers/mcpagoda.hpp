
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

// $Id$

#ifndef quantlib_pagoda_pricer_h
#define quantlib_pagoda_pricer_h

#include <ql/Pricers/mcpricer.hpp>
#include <ql/Math/matrix.hpp>
#include <ql/Math/statistics.hpp>
#include <ql/MonteCarlo/mctypedefs.hpp>

namespace QuantLib {

    namespace Pricers {

        //! roofed Asian option
        /*! Given a certain portfolio of assets at the end of the period
            it is returned the minimum of a given roof and a certain fraction
            of the positive portfolio performance.
            If the performance of the portfolio is below then the payoff
            is null.
        */
        class McPagoda : public McPricer<Math::Statistics,
            MonteCarlo::GaussianMultiPathGenerator,
            MonteCarlo::PathPricer<MonteCarlo::MultiPath> > {
          public:
            McPagoda(const Array& portfolio,
                     double fraction,
                     double roof,
                     const Array& dividendYield,
                     const Math::Matrix& covariance,
                     Rate riskFreeRate,
                     const std::vector<Time>& times,
                     bool antithetic,
                     long seed = 0);
        };

    }

}


#endif
