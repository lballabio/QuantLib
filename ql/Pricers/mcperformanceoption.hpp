
/*
 Copyright (C) 2002, 2003 Ferdinando Ametrano

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email quantlib-dev@lists.sf.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file mcperformanceoption.hpp
    \brief Performance option priced with Monte Carlo simulation
*/

#ifndef quantlib_montecarlo_performanceoption_pricer_h
#define quantlib_montecarlo_performanceoption_pricer_h

#include <ql/option.hpp>
#include <ql/types.hpp>
#include <ql/Pricers/mcpricer.hpp>
#include <ql/MonteCarlo/montecarlomodel.hpp>

namespace QuantLib {

    //! Performance option computed using Monte Carlo simulation
    /*! A performance option is a variant of a cliquet option:
        the payoff of each forward-starting (a.k.a. deferred
        strike) options is \$ max(S/X- 1) \$.
    */
    class McPerformanceOption 
        : public McPricer<SingleAsset<PseudoRandom> >{
      public:
        McPerformanceOption(
                    Option::Type type,
                    double underlying,
                    double moneyness,
                    const RelinkableHandle<TermStructure>& dividendYield,
                    const RelinkableHandle<TermStructure>& riskFreeRate,
                    const RelinkableHandle<BlackVolTermStructure>& volatility,
                    const std::vector<Time>& times,
                    long seed = 0);
    };

}


#endif
