/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Ferdinando Ametrano
 Copyright (C) 2001, 2002, 2003 Sadruddin Rejeb

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/reference/license.html>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file blackmodel.hpp
    \brief Black formula and associated functions
*/

#ifndef quantlib_black_model_hpp
#define quantlib_black_model_hpp

#include <ql/option.hpp>
#include <ql/Math/normaldistribution.hpp>

namespace QuantLib {

    /*! Black 1976 formula
        \warning instead of volatility it uses standard deviation,
                 i.e. volatility*sqrt(timeToMaturity)
    */
    Real blackFormula(Option::Type optionType,
                      Real strike,
                      Real forward,
                      Real stdDev);

    /*! Approximated Black 1976 implied standard deviation,
        i.e. volatility*sqrt(timeToMaturity).
        
        It is calculated using Brenner and Subrahmanyan (1988) and Feinstein
        (1988) approximation for at-the-money forward option, with the
        extended moneyness approximation by Corrado and Miller (1996)
    */
    Real blackImpliedStdDevApproximation(Option::Type optionType,
                                         Real strike,
                                         Real forward,
                                         Real blackPrice);

    /*! Black 1976 implied  standard deviation,
        i.e. volatility*sqrt(timeToMaturity)
    */
    Real blackImpliedStdDev(Option::Type optionType,
                            Real strike,
                            Real forward,
                            Real blackPrice,
                            Real guess = Null<Real>(),
                            Real accuracy = 1.0e-6);

    inline Real blackItmProbability(Option::Type optionType,
                                    Real strike,
                                    Real forward,
                                    Real stdDev) {
        if (stdDev==0.0)
            return (forward*optionType > strike*optionType ? 1.0 : 0.0);
        if (strike==0.0)
            return (optionType==Option::Call ? 1.0 : 0.0);
        Real d1 = std::log(forward/strike)/stdDev + 0.5*stdDev;
        Real d2 = d1 - stdDev;
        CumulativeNormalDistribution phi;
        return phi(optionType*d2);
    }

}

#endif
