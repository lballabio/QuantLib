

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
/*! \file mceverest.hpp
    \brief %Everest-type option pricer

    \fullpath
    ql/Pricers/%mceverest.hpp
*/

// $Id$

#ifndef quantlib_pricers_everest_pricer_h
#define quantlib_pricers_everest_pricer_h

#include <ql/Pricers/mcpricer.hpp>
#include <ql/Math/matrix.hpp>

namespace QuantLib {

    namespace Pricers {

        //! Everest-type option pricer
        /*! The payoff of an Everest option is simply given by the
            final price / initial price ratio of the worst performer
        */
        class McEverest : public McPricer<Math::Statistics,
            MonteCarlo::GaussianMultiPathGenerator,
            MonteCarlo::PathPricer<MonteCarlo::MultiPath> > {
          public:
            McEverest(const Array& dividendYield,
                      const Math::Matrix& covariance,
                      Rate riskFreeRate,
                      Time residualTime,
                      bool antitheticVariance,
                      long seed = 0);
        };

    }

}


#endif
