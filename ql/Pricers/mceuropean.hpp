

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
/*! \file mceuropean.hpp
    \brief simple example of Monte Carlo pricer

    \fullpath
    ql/Pricers/%mceuropean.hpp
*/

// $Id$

#ifndef quantlib_montecarlo_european_pricer_h
#define quantlib_montecarlo_european_pricer_h

#include <ql/option.hpp>
#include <ql/types.hpp>
#include <ql/Pricers/mcpricer.hpp>
#include <ql/MonteCarlo/mctypedefs.hpp>

namespace QuantLib {

    namespace Pricers {

        //! simple example of Monte Carlo pricer
        /*! \deprecated use VanillaOption with McEuropeanEngine */
        class McEuropean : public McPricer<Math::Statistic,
            MonteCarlo::GaussianPathGenerator_old,
            MonteCarlo::PathPricer_old<MonteCarlo::Path> > {
          public:
            McEuropean(Option::Type type,
                       double underlying,
                       double strike,
                       Spread dividendYield,
                       Rate riskFreeRate,
                       double residualTime,
                       double volatility,
                       bool antitheticVariance,
                       long seed=0);
        };

    }

}


#endif
