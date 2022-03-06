/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2001, 2002, 2003 Sadruddin Rejeb
 Copyright (C) 2003, 2004, 2005, 2006, 2008 Ferdinando Ametrano
 Copyright (C) 2006 Mark Joshi
 Copyright (C) 2006 StatPro Italia srl
 Copyright (C) 2007 Cristina Duminuco
 Copyright (C) 2007 Chiara Fornarola
 Copyright (C) 2013 Gary Kennedy
 Copyright (C) 2015 Peter Caspers
 Copyright (C) 2017 Klaus Spanderen
 Copyright (C) 2019 Wojciech Ślusarski
 Copyright (C) 2020 Marcin Rybacki

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file blackformula.hpp
    \brief Black formula
*/

#ifndef quantlib_blackformula_hpp
#define quantlib_blackformula_hpp

#include <ql/instruments/payoffs.hpp>
#include <ql/option.hpp>

namespace QuantLib {

    /*! Black 1976 formula
        \warning instead of volatility it uses standard deviation,
                 i.e. volatility*sqrt(timeToMaturity)
    */
    Real blackFormula(Option::Type optionType,
                      Real strike,
                      Real forward,
                      Real stdDev,
                      Real discount = 1.0,
                      Real displacement = 0.0);

    /*! Black 1976 formula
        \warning instead of volatility it uses standard deviation,
                 i.e. volatility*sqrt(timeToMaturity)
    */
    Real blackFormula(const ext::shared_ptr<PlainVanillaPayoff>& payoff,
                      Real forward,
                      Real stdDev,
                      Real discount = 1.0,
                      Real displacement = 0.0);

    /*! Black 1976 model forward derivative
        \warning instead of volatility it uses standard deviation,
                 i.e. volatility*sqrt(timeToMaturity)
    */
    Real blackFormulaForwardDerivative(Option::Type optionType,
                                       Real strike,
                                       Real forward,
                                       Real stdDev,
                                       Real discount = 1.0,
                                       Real displacement = 0.0);

    /*! Black 1976 model forward derivative
        \warning instead of volatility it uses standard deviation,
                 i.e. volatility*sqrt(timeToMaturity)
    */
    Real blackFormulaForwardDerivative(const ext::shared_ptr<PlainVanillaPayoff>& payoff,
                                       Real forward,
                                       Real stdDev,
                                       Real discount = 1.0,
                                       Real displacement = 0.0);

    /*! Approximated Black 1976 implied standard deviation,
        i.e. volatility*sqrt(timeToMaturity).

        It is calculated using Brenner and Subrahmanyan (1988) and Feinstein
        (1988) approximation for at-the-money forward option, with the
        extended moneyness approximation by Corrado and Miller (1996)
    */
    Real blackFormulaImpliedStdDevApproximation(Option::Type optionType,
                                                Real strike,
                                                Real forward,
                                                Real blackPrice,
                                                Real discount = 1.0,
                                                Real displacement = 0.0);

    /*! Approximated Black 1976 implied standard deviation,
        i.e. volatility*sqrt(timeToMaturity).

        It is calculated using Brenner and Subrahmanyan (1988) and Feinstein
        (1988) approximation for at-the-money forward option, with the
        extended moneyness approximation by Corrado and Miller (1996)
    */
    Real blackFormulaImpliedStdDevApproximation(const ext::shared_ptr<PlainVanillaPayoff>& payoff,
                                                Real forward,
                                                Real blackPrice,
                                                Real discount = 1.0,
                                                Real displacement = 0.0);

    /*! Approximated Black 1976 implied standard deviation,
        i.e. volatility*sqrt(timeToMaturity).

        It is calculated following "An improved approach to computing
        implied volatility", Chambers, Nawalkha, The Financial Review,
        2001, 89-100. The atm option price must be known to use this
        method.
    */
    Real blackFormulaImpliedStdDevChambers(Option::Type optionType,
                                           Real strike,
                                           Real forward,
                                           Real blackPrice,
                                           Real blackAtmPrice,
                                           Real discount = 1.0,
                                           Real displacement = 0.0);

    /*! Approximated Black 1976 implied standard deviation,
        i.e. volatility*sqrt(timeToMaturity).

        It is calculated following "An improved approach to computing
        implied volatility", Chambers, Nawalkha, The Financial Review,
        2001, 89-100. The atm option price must be known to use this
        method.
    */
    Real blackFormulaImpliedStdDevChambers(const ext::shared_ptr<PlainVanillaPayoff>& payoff,
                                           Real forward,
                                           Real blackPrice,
                                           Real blackAtmPrice,
                                           Real discount = 1.0,
                                           Real displacement = 0.0);

    /*! Approximated Black 1976 implied standard deviation,
        i.e. volatility*sqrt(timeToMaturity).

        It is calculated using

        "An Explicit Implicit Volatility Formula"
        R. Radoicic, D. Stefanica,
        https://papers.ssrn.com/sol3/papers.cfm?abstract_id=2908494

        "Tighter Bounds for Implied Volatility",
        J. Gatheral, I. Matic, R. Radoicic, D. Stefanica
        https://papers.ssrn.com/sol3/papers.cfm?abstract_id=2922742
    */
    Real blackFormulaImpliedStdDevApproximationRS(Option::Type optionType,
                                                  Real strike,
                                                  Real forward,
                                                  Real blackPrice,
                                                  Real discount = 1.0,
                                                  Real displacement = 0.0);

    Real blackFormulaImpliedStdDevApproximationRS(const ext::shared_ptr<PlainVanillaPayoff>& payoff,
                                                  Real forward,
                                                  Real blackPrice,
                                                  Real discount = 1.0,
                                                  Real displacement = 0.0);


    /*! Black 1976 implied standard deviation,
        i.e. volatility*sqrt(timeToMaturity)
    */
    Real blackFormulaImpliedStdDev(Option::Type optionType,
                                   Real strike,
                                   Real forward,
                                   Real blackPrice,
                                   Real discount = 1.0,
                                   Real displacement = 0.0,
                                   Real guess = Null<Real>(),
                                   Real accuracy = 1.0e-6,
                                   Natural maxIterations = 100);

    /*! Black 1976 implied standard deviation,
        i.e. volatility*sqrt(timeToMaturity)
    */
    Real blackFormulaImpliedStdDev(const ext::shared_ptr<PlainVanillaPayoff>& payoff,
                                   Real forward,
                                   Real blackPrice,
                                   Real discount = 1.0,
                                   Real displacement = 0.0,
                                   Real guess = Null<Real>(),
                                   Real accuracy = 1.0e-6,
                                   Natural maxIterations = 100);

    /*! Black 1976 implied standard deviation,
         i.e. volatility*sqrt(timeToMaturity)

        "An Adaptive Successive Over-relaxation Method for Computing the
        Black-Scholes Implied Volatility"
        M. Li, http://mpra.ub.uni-muenchen.de/6867/


        Starting point of the iteration is calculated based on

        "An Explicit Implicit Volatility Formula"
        R. Radoicic, D. Stefanica,
        https://papers.ssrn.com/sol3/papers.cfm?abstract_id=2908494
    */
    Real blackFormulaImpliedStdDevLiRS(Option::Type optionType,
                                       Real strike,
                                       Real forward,
                                       Real blackPrice,
                                       Real discount = 1.0,
                                       Real displacement = 0.0,
                                       Real guess = Null<Real>(),
                                       Real omega = 1.0,
                                       Real accuracy = 1.0e-6,
                                       Natural maxIterations = 100);

    Real blackFormulaImpliedStdDevLiRS(const ext::shared_ptr<PlainVanillaPayoff>& payoff,
                                       Real forward,
                                       Real blackPrice,
                                       Real discount = 1.0,
                                       Real displacement = 0.0,
                                       Real guess = Null<Real>(),
                                       Real omega = 1.0,
                                       Real accuracy = 1.0e-6,
                                       Natural maxIterations = 100);

    /*! Black 1976 probability of being in the money (in the bond martingale
        measure), i.e. N(d2).
        It is a risk-neutral probability, not the real world one.
        \warning instead of volatility it uses standard deviation,
                 i.e. volatility*sqrt(timeToMaturity)
    */
    Real blackFormulaCashItmProbability(
        Option::Type optionType, Real strike, Real forward, Real stdDev, Real displacement = 0.0);

    /*! Black 1976 probability of being in the money (in the bond martingale
        measure), i.e. N(d2).
        It is a risk-neutral probability, not the real world one.
        \warning instead of volatility it uses standard deviation,
                 i.e. volatility*sqrt(timeToMaturity)
    */
    Real blackFormulaCashItmProbability(const ext::shared_ptr<PlainVanillaPayoff>& payoff,
                                        Real forward,
                                        Real stdDev,
                                        Real displacement = 0.0);

    /*! Black 1976 probability of being in the money in the asset martingale
        measure, i.e. N(d1).
        It is a risk-neutral probability, not the real world one.
    */
    Real blackFormulaAssetItmProbability(
        Option::Type optionType, Real strike, Real forward, Real stdDev, Real displacement = 0.0);

    /*! Black 1976 probability of being in the money in the asset martingale
        measure, i.e. N(d1).
        It is a risk-neutral probability, not the real world one.
    */
    Real blackFormulaAssetItmProbability(const ext::shared_ptr<PlainVanillaPayoff>& payoff,
                                         Real forward,
                                         Real stdDev,
                                         Real displacement = 0.0);

    /*! Black 1976 formula for standard deviation derivative
        \warning instead of volatility it uses standard deviation, i.e.
                 volatility*sqrt(timeToMaturity), and it returns the
                 derivative with respect to the standard deviation.
                 If T is the time to maturity Black vega would be
                 blackStdDevDerivative(strike, forward, stdDev)*sqrt(T)
    */
    Real blackFormulaStdDevDerivative(
        Real strike, Real forward, Real stdDev, Real discount = 1.0, Real displacement = 0.0);

    /*! Black 1976 formula for  derivative with respect to implied vol, this
        is basically the vega, but if you want 1% change multiply by 1%
   */
    Real blackFormulaVolDerivative(Real strike,
                                   Real forward,
                                   Real stdDev,
                                   Real expiry,
                                   Real discount = 1.0,
                                   Real displacement = 0.0);


    /*! Black 1976 formula for standard deviation derivative
        \warning instead of volatility it uses standard deviation, i.e.
                 volatility*sqrt(timeToMaturity), and it returns the
                 derivative with respect to the standard deviation.
                 If T is the time to maturity Black vega would be
                 blackStdDevDerivative(strike, forward, stdDev)*sqrt(T)
    */
    Real blackFormulaStdDevDerivative(const ext::shared_ptr<PlainVanillaPayoff>& payoff,
                                      Real forward,
                                      Real stdDev,
                                      Real discount = 1.0,
                                      Real displacement = 0.0);

    /*! Black 1976 formula for second derivative by standard deviation
        \warning instead of volatility it uses standard deviation, i.e.
                 volatility*sqrt(timeToMaturity), and it returns the
                 derivative with respect to the standard deviation.
    */
    Real blackFormulaStdDevSecondDerivative(
        Rate strike, Rate forward, Real stdDev, Real discount, Real displacement);

    /*! Black 1976 formula for second derivative by standard deviation
        \warning instead of volatility it uses standard deviation, i.e.
                 volatility*sqrt(timeToMaturity), and it returns the
                 derivative with respect to the standard deviation.
    */
    Real blackFormulaStdDevSecondDerivative(const ext::shared_ptr<PlainVanillaPayoff>& payoff,
                                            Real forward,
                                            Real stdDev,
                                            Real discount = 1.0,
                                            Real displacement = 0.0);

    /*! Black style formula when forward is normal rather than
        log-normal. This is essentially the model of Bachelier.

        \warning Bachelier model needs absolute volatility, not
                 percentage volatility. Standard deviation is
                 absoluteVolatility*sqrt(timeToMaturity)
    */
    Real bachelierBlackFormula(
        Option::Type optionType, Real strike, Real forward, Real stdDev, Real discount = 1.0);

    /*! Black style formula when forward is normal rather than
        log-normal. This is essentially the model of Bachelier.

        \warning Bachelier model needs absolute volatility, not
                 percentage volatility. Standard deviation is
                 absoluteVolatility*sqrt(timeToMaturity)
    */
    Real bachelierBlackFormula(const ext::shared_ptr<PlainVanillaPayoff>& payoff,
                               Real forward,
                               Real stdDev,
                               Real discount = 1.0);

    /*! Bachelier Black model forward derivative.

        \warning Bachelier model needs absolute volatility, not
                 percentage volatility. Standard deviation is
                 absoluteVolatility*sqrt(timeToMaturity)
    */
    Real bachelierBlackFormulaForwardDerivative(
        Option::Type optionType, Real strike, Real forward, Real stdDev, Real discount = 1.0);

    /*! Bachelier Black model forward derivative.

        \warning Bachelier model needs absolute volatility, not
                 percentage volatility. Standard deviation is
                 absoluteVolatility*sqrt(timeToMaturity)
    */
    Real bachelierBlackFormulaForwardDerivative(
        const ext::shared_ptr<PlainVanillaPayoff>& payoff,
        Real forward,
        Real stdDev,
        Real discount = 1.0);

    /*! Approximated Bachelier implied volatility

        It is calculated using  the analytic implied volatility approximation
        of J. Choi, K Kim and M. Kwak (2009), “Numerical Approximation of the
        Implied Volatility Under Arithmetic Brownian Motion”,
        Applied Math. Finance, 16(3), pp. 261-268.
    */
    Real bachelierBlackFormulaImpliedVol(Option::Type optionType,
                                         Real strike,
                                         Real forward,
                                         Real tte,
                                         Real bachelierPrice,
                                         Real discount = 1.0);

    /*! Bachelier formula for standard deviation derivative
        \warning instead of volatility it uses standard deviation, i.e.
                 volatility*sqrt(timeToMaturity), and it returns the
                 derivative with respect to the standard deviation.
                 If T is the time to maturity Black vega would be
                 blackStdDevDerivative(strike, forward, stdDev)*sqrt(T)
    */

    Real bachelierBlackFormulaStdDevDerivative(Real strike,
                                               Real forward,
                                               Real stdDev,
                                               Real discount = 1.0);

    Real bachelierBlackFormulaStdDevDerivative(const ext::shared_ptr<PlainVanillaPayoff>& payoff,
                                               Real forward,
                                               Real stdDev,
                                               Real discount = 1.0);

    /*! Bachelier formula for probability of being in the money in the asset martingale
        measure, i.e. N(d).
        It is a risk-neutral probability, not the real world one.
    */
    Real bachelierBlackFormulaAssetItmProbability(Option::Type optionType,
                                                  Real strike,
                                                  Real forward,
                                                  Real stdDev);

    /*! Bachelier formula for of being in the money in the asset martingale
        measure, i.e. N(d).
        It is a risk-neutral probability, not the real world one.
    */
    Real bachelierBlackFormulaAssetItmProbability(const ext::shared_ptr<PlainVanillaPayoff>& payoff,
                                                  Real forward,
                                                  Real stdDev);

}

#endif


#ifndef id_f059e42e094b6257b92bb7e2ff1e974a
#define id_f059e42e094b6257b92bb7e2ff1e974a
inline bool test_f059e42e094b6257b92bb7e2ff1e974a(int* i) { return i != 0; }
#endif
