
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
/*! \file mcdiscretearithmeticaso.hpp
    \brief Discrete Arithmetic Average Strike Option

    \fullpath
    ql/Pricers/%mcdiscretearithmeticaso.hpp
*/

// $Id$

#ifndef quantlib_pricers_mc_discrete_arithmetic_average_strike_mcpricer_h
#define quantlib_pricers_mc_discrete_arithmetic_average_strike_mcpricer_h

#include <ql/option.hpp>
#include <ql/types.hpp>
#include <ql/Pricers/mcpricer.hpp>
#include <ql/MonteCarlo/mctypedefs.hpp>

namespace QuantLib {

    namespace Pricers {

        //! example of Monte Carlo pricer using a control variate.
        /*! \todo Continous Averaging version     */
        class McDiscreteArithmeticASO : public McPricer<Math::Statistics,
            MonteCarlo::GaussianPathGenerator,
            MonteCarlo::PathPricer<MonteCarlo::Path> > {
          public:
            McDiscreteArithmeticASO(Option::Type type,
                                    double underlying,
                                    Spread dividendYield,
                                    Rate riskFreeRate,
                                    const std::vector<Time>& times,
                                    double volatility,
                                    bool antitheticVariance,
                                    bool controlVariate,
                                    long seed = 0);
        };

    }

}


#endif
