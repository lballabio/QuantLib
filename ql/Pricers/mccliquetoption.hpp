

/*
 Copyright (C) 2002, 2003 Ferdinando Ametrano

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
/*! \file mccliquetoption.hpp
    \brief Cliquet option priced with Monte Carlo simulation

    \fullpath
    ql/Pricers/%mccliquetoption.hpp
*/

// $Id$

#ifndef quantlib_montecarlo_cliquetoption_pricer_h
#define quantlib_montecarlo_cliquetoption_pricer_h

#include <ql/option.hpp>
#include <ql/types.hpp>
#include <ql/Pricers/mcpricer.hpp>
#include <ql/MonteCarlo/mctypedefs.hpp>

namespace QuantLib {

    namespace Pricers {

        //! simple example of Monte Carlo pricer
        class McCliquetOption : public McPricer<Math::Statistics,
            MonteCarlo::GaussianPathGenerator,
            MonteCarlo::PathPricer<MonteCarlo::Path> > {
          public:
            McCliquetOption(Option::Type type,
                       double underlying,
                       double moneyness,
                       const std::vector<Spread>& dividendYield,
                       const std::vector<Rate>& riskFreeRate,
                       const std::vector<Time>& times,
                       const std::vector<double>& volatility,
                       bool antitheticVariance,
                       long seed=0);
        };

    }

}


#endif
