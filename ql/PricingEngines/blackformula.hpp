/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2003, 2004, 2005, 2006 Ferdinando Ametrano
 Copyright (C) 2006 Mark Joshi
 Copyright (C) 2001, 2002, 2003 Sadruddin Rejeb
 Copyright (C) 2006 StatPro Italia srl

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

/*! \file blackformula.hpp
    \brief Black formula
*/

#ifndef quantlib_blackformula_hpp
#define quantlib_blackformula_hpp

#include <ql/option.hpp>
#include <ql/Instruments/payoffs.hpp>

namespace QuantLib {

    #ifndef QL_DISABLE_DEPRECATED
    class BlackCalculator;
    //! \deprecated use BlackCalculator instead
    typedef BlackCalculator BlackFormula;
    #endif

    /*! Black 1976 formula
        \warning instead of volatility it uses standard deviation,
                 i.e. volatility*sqrt(timeToMaturity)
    */
    Real blackFormula(Option::Type optionType,
                      Real strike,
                      Real forward,
                      Real stdDev,
                      Real discount = 1.0);

    /*! Black 1976 formula
        \warning instead of volatility it uses standard deviation,
                 i.e. volatility*sqrt(timeToMaturity)
    */
    Real blackFormula(const boost::shared_ptr<PlainVanillaPayoff>& payoff,
                      Real forward,
                      Real stdDev,
                      Real discount = 1.0);


    /*! Approximated Black 1976 implied standard deviation,
        i.e. volatility*sqrt(timeToMaturity).

        It is calculated using Brenner and Subrahmanyan (1988) and Feinstein
        (1988) approximation for at-the-money forward option, with the
        extended moneyness approximation by Corrado and Miller (1996)
    */
    Real blackImpliedStdDevApproximation(Option::Type optionType,
                                         Real strike,
                                         Real forward,
                                         Real blackPrice,
                                         Real discount = 1.0);

    /*! Approximated Black 1976 implied standard deviation,
        i.e. volatility*sqrt(timeToMaturity).

        It is calculated using Brenner and Subrahmanyan (1988) and Feinstein
        (1988) approximation for at-the-money forward option, with the
        extended moneyness approximation by Corrado and Miller (1996)
    */
    Real blackImpliedStdDevApproximation(
                        const boost::shared_ptr<PlainVanillaPayoff>& payoff,
                        Real forward,
                        Real blackPrice,
                        Real discount = 1.0);


    /*! Black 1976 implied standard deviation,
        i.e. volatility*sqrt(timeToMaturity)
    */
    Real blackImpliedStdDev(Option::Type optionType,
                            Real strike,
                            Real forward,
                            Real blackPrice,
                            Real discount = 1.0,
                            Real guess = Null<Real>(),
                            Real accuracy = 1.0e-6);

    /*! Black 1976 implied standard deviation,
        i.e. volatility*sqrt(timeToMaturity)
    */
    Real blackImpliedStdDev(
                        const boost::shared_ptr<PlainVanillaPayoff>& payoff,
                        Real forward,
                        Real blackPrice,
                        Real discount = 1.0,
                        Real guess = Null<Real>(),
                        Real accuracy = 1.0e-6);


    /*! Black 1976 "in the money probability" formula
        \warning instead of volatility it uses standard deviation,
                 i.e. volatility*sqrt(timeToMaturity)
    */
    Real blackItmProbability(Option::Type optionType,
                             Real strike,
                             Real forward,
                             Real stdDev);

    /*! Black 1976 "in the money probability" formula
        \warning instead of volatility it uses standard deviation,
                 i.e. volatility*sqrt(timeToMaturity)
    */
    Real blackItmProbability(
                        const boost::shared_ptr<PlainVanillaPayoff>& payoff,
                        Real forward,
                        Real stdDev);


    /*! Black 1976 formula for standard deviation derivative
        \warning instead of volatility it uses standard deviation, i.e.
                 volatility*sqrt(timeToMaturity), and it returns the
                 derivative with respect to the standard deviation.
                 If T is the time to maturity Black vega would be
                 blackStdDevDerivative(strike, forward, stdDev)*sqrt(T)
    */
    Real blackStdDevDerivative(Rate strike,
                               Rate forward,
                               Real stdDev,
                               Real discount = 1.0);

    /*! Black 1976 formula for standard deviation derivative
        \warning instead of volatility it uses standard deviation, i.e.
                 volatility*sqrt(timeToMaturity), and it returns the
                 derivative with respect to the standard deviation.
                 If T is the time to maturity Black vega would be
                 blackStdDevDerivative(strike, forward, stdDev)*sqrt(T)
    */
    Real blackStdDevDerivative(
                        const boost::shared_ptr<PlainVanillaPayoff>& payoff,
                        Real forward,
                        Real stdDev,
                        Real discount = 1.0);


    /*! Black style formula when forward is normal rather than
        log-normal. This is essentially the model of Bachelier.

        \warning Bachelier model needs absolute volatility, not percentage
                 volatility. Standard deviation is
                 absoluteVolatility*sqrt(timeToMaturity)
    */
    Real bachelierBlackFormula(Option::Type optionType,
                               Real strike,
                               Real forward,
                               Real stdDev,
                               Real discount = 1.0);

    /*! Black style formula when forward is normal rather than
        log-normal. This is essentially the model of Bachelier.

        \warning Bachelier model needs absolute volatility, not percentage
                 volatility. Standard deviation is
                 absoluteVolatility*sqrt(timeToMaturity)
    */
    Real bachelierBlackFormula(
                        const boost::shared_ptr<PlainVanillaPayoff>& payoff,
                        Real forward,
                        Real stdDev,
                        Real discount = 1.0);

}

#endif
