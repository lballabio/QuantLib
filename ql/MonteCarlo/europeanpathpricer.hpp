
/*
 Copyright (C) 2003 Ferdinando Ametrano
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

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

/*! \file europeanpathpricer.hpp
    \brief path pricer for European options
*/

#ifndef quantlib_montecarlo_european_path_pricer_h
#define quantlib_montecarlo_european_path_pricer_h

#include <ql/MonteCarlo/pathpricer.hpp>
#include <ql/MonteCarlo/path.hpp>
#include <ql/payoff.hpp>

namespace QuantLib {

    //! %path pricer for European options
    class EuropeanPathPricer : public PathPricer<Path> {
      public:
        EuropeanPathPricer(Option::Type type,
                           double underlying,
                           double strike,
                           const RelinkableHandle<TermStructure>& riskFreeTS);
        double operator()(const Path& path) const;
      private:
        double underlying_;
        // it would be easy to generalize to more exotic payoffs
        PlainVanillaPayoff payoff_;
    };

}


#endif
