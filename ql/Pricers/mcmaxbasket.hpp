

/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

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

// $Id$

#ifndef quantlib_max_basket_pricer_h
#define quantlib_max_basket_pricer_h

#include <ql/Pricers/mcpricer.hpp>
#include <ql/Math/matrix.hpp>

namespace QuantLib {

    namespace Pricers {

        //! simple example of multi-factor Monte Carlo pricer
        class McMaxBasket : public McPricer<Math::Statistics,
            MonteCarlo::GaussianMultiPathGenerator,
            MonteCarlo::PathPricer_old<MonteCarlo::MultiPath> > {
          public:
            McMaxBasket(const Array& underlying,
                        const Array& dividendYield,
                        const Math::Matrix& covariance,
                        Rate riskFreeRate,
                        double residualTime,
                        bool antitheticVariance,
                        long seed=0);
        };

    }

}


#endif
